module;
#include "KaleidoscopeJIT.h"
#include "loxxy/ast.hpp"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassInstrumentation.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>

export module ast.llvm;
import ast;
import utils.stupid_type_traits;
import utils.string_store;
import utils.variant;

using std::nullptr_t;
using std::same_as;
using std::string;
using utils::any_ref;
using utils::IndirectVisitor;

export namespace loxxy {

template <typename Payload, typename Indirection, bool ptr_variant, typename Resolver = void>
struct IRGenerator : IndirectVisitor<IRGenerator<Payload, Indirection, ptr_variant, Resolver>, Resolver, Indirection> {

    USING_FAMILY(Payload, Indirection, ptr_variant);
    using Self = IRGenerator<Payload, Indirection, ptr_variant, Resolver>;
    using Parent = IndirectVisitor<Self, Resolver, Indirection>;
    using Parent::operator();
    using Parent::Parent;

    using Adhoc = utils::Adhoc<Resolver, Indirection>;

    template <typename... Args>
    IRGenerator(Args&&... args) : adhoc(std::forward<Args>(args)...) {
        fpm.addPass(llvm::InstCombinePass());
        fpm.addPass(llvm::ReassociatePass());
        fpm.addPass(llvm::GVNPass());
        fpm.addPass(llvm::SimplifyCFGPass());

        llvm::PassBuilder pass_builder;
        pass_builder.registerModuleAnalyses(mam);
        pass_builder.registerFunctionAnalyses(fam);
        pass_builder.crossRegisterProxies(lam, fam, cgam, mam);

        si.registerCallbacks(pic, &mam);
    }

    void operator()(const ExpressionStmt& node) {
        utils::visit(*this, node.expr);
        return;
    }

    void operator()(const PrintStmt& node) {}

    void operator()(const VarDecl& node) {
        if (!node.expr.has_value())
            named_values[node.identifier] = llvm::ConstantFP::get(context, llvm::APFloat(0.0));
        else
            named_values[node.identifier] = utils::visit(*this, node.expr.value());
    }

    void operator()(const FunDecl& node) {
        std::vector<llvm::Type*> doubles(node.args.size(), llvm::Type::getDoubleTy(context));

        llvm::FunctionType* ft = llvm::FunctionType::get(llvm::Type::getDoubleTy(context), doubles, false);
        llvm::Function* func = llvm::Function::Create(
            ft, llvm::Function::ExternalLinkage, std::string(std::string_view(*node.identifier)), &module
        );

        uint8_t idx = 0;
        for (auto& arg : func->args()) {
            arg.setName(std::string_view(*node.args[idx++]));
        }

        llvm::BasicBlock* basic_block = llvm::BasicBlock::Create(context, "entry", func);
        builder.SetInsertPoint(basic_block);

        named_values.clear();

        idx = 0;
        for (auto& arg : func->args()) {
            named_values[node.args[idx++]] = &arg;
        }
        return_value = nullptr;

        for (auto& stmt : node.body) {
            utils::visit(*this, stmt);
            if (return_value)
                break;
        }

        if (return_value)
            builder.CreateRet(return_value);
        else
            builder.CreateRetVoid();

        llvm::verifyFunction(*func);

        fpm.run(*func, fam);

        functions.push_back(func);
    }

    void operator()(const BlockStmt& node) {}

    void operator()(const IfStmt& node) {}

    void operator()(const WhileStmt& node) {}

    void operator()(const ReturnStmt& node) { return_value = utils::visit(*this, node.expr); }

    template <typename T>
    void operator()(const T&) {}

    auto operator()(const BinaryExpr& node) -> llvm::Value* {
        llvm::Value* lhs = utils::visit(*this, node.lhs);
        llvm::Value* rhs = utils::visit(*this, node.rhs);

        if (!lhs || !rhs)
            return nullptr;

        switch (node.op.getType()) {
        case GREATER:
            return builder.CreateFCmpUGT(lhs, rhs, "cmptmp");
        case GREATER_EQUAL:
            return builder.CreateFCmpUGE(lhs, rhs, "cmptmp");
        case LESS:
            return builder.CreateFCmpULT(lhs, rhs, "cmptmp");
        case LESS_EQUAL:
            return builder.CreateFCmpULE(lhs, rhs, "cmptmp");
        case EQUAL_EQUAL:
            return builder.CreateFCmpUEQ(lhs, rhs, "cmptmp");
        case BANG_EQUAL:
            return builder.CreateFCmpUNE(lhs, rhs, "cmptmp");
        case PLUS:
            return builder.CreateFAdd(lhs, rhs, "addtmp");
        case MINUS:
            return builder.CreateFSub(lhs, rhs, "subtmp");
        case STAR:
            return builder.CreateFMul(lhs, rhs, "multmp");
        case SLASH:
            return builder.CreateFDiv(lhs, rhs, "divtmp");
        case AND:
            return builder.CreateAnd(lhs, rhs, "conjtmp");
        case OR:
            return builder.CreateOr(lhs, rhs, "disjtmp");
        default:
            throw std::runtime_error("malformed binary node");
        }
    }

    auto operator()(const GroupingExpr& node) -> llvm::Value* { return utils::visit(*this, node.expr); }

    auto operator()(const UnaryExpr& node) -> llvm::Value* {
        llvm::Value* value = utils::visit(*this, node.expr);
        switch (node.op.getType()) {
        case MINUS:
            return builder.CreateFSub(llvm::ConstantFP::get(context, llvm::APFloat(0.0)), value);
        case BANG:
            return builder.CreateNot(value);
        default:
            throw std::runtime_error("malformed unary node");
        }
    }

    auto operator()(const NumberExpr& node) -> llvm::Value* {
        return llvm::ConstantFP::get(context, llvm::APFloat(node.x));
    }

    auto operator()(const StringExpr& node) -> llvm::Value* {
        return llvm::ConstantFP::get(context, llvm::APFloat(0.0));
    }

    auto operator()(const NilExpr& node) -> llvm::Value* { return llvm::ConstantFP::get(context, llvm::APFloat(0.0)); }

    auto operator()(const BoolExpr& node) -> llvm::Value* { return llvm::ConstantFP::get(context, llvm::APFloat(0.0)); }

    auto operator()(const VarExpr& node) -> llvm::Value* {
        llvm::Value* value = named_values[node.identifier];
        return value;
    }

    auto operator()(const AssignExpr& node) -> llvm::Value* {
        named_values[node.identifier] = utils::visit(*this, node.expr);
        return named_values[node.identifier];
    }

    auto operator()(const CallExpr& node) -> llvm::Value* {
        auto get_callee = adhoc.make_visitor(
            [](const VarExpr& var) -> const persistent_string<>* { return var.identifier; },
            []<typename T>(const T&) -> const persistent_string<>* { throw 1; }
        );

        llvm::Function* callee = module.getFunction(std::string_view(*utils::visit(get_callee, node.callee)));
        std::vector<llvm::Value*> args;
        for (const auto& arg : node.arguments) {
            args.push_back(utils::visit(*this, arg));
        }

        return builder.CreateCall(callee, args, "calltmp");
    }

    void printIR() { module.print(llvm::errs(), nullptr); }

private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> builder{context};
    llvm::Module module{"loxxy", context};
    std::map<const utils::persistent_string<>*, llvm::Value*> named_values;
    std::vector<llvm::Function*> functions;
    llvm::Value* return_value = nullptr;
    llvm::FunctionPassManager fpm;
    llvm::LoopAnalysisManager lam;
    llvm::FunctionAnalysisManager fam;
    llvm::CGSCCAnalysisManager cgam;
    llvm::ModuleAnalysisManager mam;
    llvm::PassInstrumentationCallbacks pic;
    llvm::StandardInstrumentations si{context, true};

    Adhoc adhoc;
};

} // namespace loxxy
