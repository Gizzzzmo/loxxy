#define USING_FAMILY(Payload, Indirection, ptr_variant)\
    using ExprPointer = ExprPointer<Payload, Indirection, ptr_variant>; \
    using StmtPointer = StmtPointer<Payload, Indirection, ptr_variant>; \
    using BinaryExpr = BinaryExpr<Payload, Indirection, ptr_variant>; \
    using UnaryExpr = UnaryExpr<Payload, Indirection, ptr_variant>; \
    using GroupingExpr = GroupingExpr<Payload, Indirection, ptr_variant>; \
    using StringExpr = StringExpr<Payload, Indirection, ptr_variant>; \
    using NumberExpr = NumberExpr<Payload, Indirection, ptr_variant>; \
    using BoolExpr = BoolExpr<Payload, Indirection, ptr_variant>; \
    using NilExpr = NilExpr<Payload, Indirection, ptr_variant>; \
    using VarExpr = VarExpr<Payload, Indirection, ptr_variant>;\
    using AssignExpr = AssignExpr<Payload, Indirection, ptr_variant>;\
    using PrintStmt = PrintStmt<Payload, Indirection, ptr_variant>; \
    using ExpressionStmt = ExpressionStmt<Payload, Indirection, ptr_variant>; \
    using VarDecl = VarDecl<Payload, Indirection, ptr_variant>;\
    using TURoot = TURoot<Payload, Indirection, ptr_variant>
