# Loxxy

C++ implementation of [Crafting Interpreters](https://craftinginterpreters.com/)' toy programming language lox.
The project has turned more into an exploration of:
- modern C++ language features
- extensive use of dependency injection driven design to facilitate 
- different memory-layout and -allocation strategies for the syntax tree
- benchmarking and comparing parsers that use these different strategies 

Due to extensive use of templates and C++20's new modules compile times are kind of abysmally slow :P

## Building

To build the project (currently consisting of a lexer repl, a parser repl, an interpreter and a few tests) you need a relatively new version of cmake and ninja (to support C++ modules), as well as a compiler that supports C++23.

If you have the nix package manager you can use the [flake](./flake.nix) to get a development environment with all necessary dependencies (although I haven't properly packaged things yet; anything other than `nix develop` isn't going to work).

I'm using a few git submodules so clone recursively, and build with cmake as usual:
``` bash
git clone --recurse-submodules https://github.com/Gizzzzmo/loxxy
cd loxxy
mkdir build && cd build
cmake .. -GNinja
```
Executables end up in the bin-, tests in the test-directory.

I've tested builds with clang-18, clang-19, and gcc-14 on linux.


## Architecture overview

### Lexer

The [lexer implementation](./lib/lexer.cpp) is kind of prematurely optimized without ever having been benchmarked against a simpler version.
I do still want to do this.
Similar to how the parser can work with several different memory layouts of the syntax tree, the lexer should be able to use different string allocation strategies.


### Syntax Tree

The structure of the syntax tree is entirely defined in [./lib/ast/nodes.cpp](./lib/ast/nodes.cpp).
All nodes are standard-layout structs templated over three arguments:
- a `Payload` type
- an `Indirection` type
- a bool value `ptr_variant` 

With the Payload type one can specify an extra piece of data that every node carries in addition to the stuff that is relevant for the AST structure.

The Indirection type specifies how (as in using what type) an inner node points to its children and how a child can be retrieved from this pointer.
More precisely for any type `T` `Indirection::template type<T>` has to be some kind of pointer-like type, and `Indirection::template get<T>` has to be a function that takes an object of this type and returns a reference to the pointed-to object.
For example, providing the following struct to this template parameter would make all syntax tree nodes use std::unique_ptr objects as the pointer-like object: 
```c++
struct UniquePtrIndirection {
    template<typename T>
    using type = std::unique_ptr<T>;
    template <typename T>
    static constexpr T& get(const std::unique_ptr<T>& ptr) {
        return *ptr;
    }
}
```
The `ptr_variant` flag specifies where in memory we save the information what kind of syntax tree node we are looking at.
If `ptr_variant` is true then every pointer to a node within the tree consists not only of the pointer-like object determined by `Indirection`, but also of an identifier what kind of node is being pointed to.
If `ptr_variant` is false, this information is instead stored with the object itself.
This is achieved by having the pointer be either a `std::variant<typename Indirection::template type<NodeType1>, typename Indirection::template type<NodeType2> ...>`, or a `typename Indirection::template type<std::variant<NodeType1, NodeType2, ...>>`.
In the first case the pointers themselves are (wrapped) std::variants, while in the latter they point to a (wrapped) std::variant of all the node types hence the name `ptr_variant`.

### Parser

The [parser class](./lib/parser/rd.cpp) is a recursive descent parser that takes a Builder, and an istream object.
The types of both objects are template parameters.
The Builder type has to implement a templated call operator that is overloaded to construct every possible syntax tree node.
The first argument has to be a marker<NodeType> object (that contains no data).
This exists only to enable template deduction.
For example [the node builder that uses std::unique_ptr](./lib/ast/builders/boxed.cpp) simply implements a call operator that forwards the arguments to std::make_unique, and returns the created object.
The input stream type has to implement functions peek, and get, to process a stream of tokens.

This form of dependency injection for the node construction means that with a single parser implementation, we can benchmark how the different memory allocation strategies affect the parse performance.
For the input stream it means that lexing and parsing can easily be paralellized by sticking a [single-producer-single-consumer queue](./SPSCQueue/) between the lexer output callback and the input stream's get and peek methods (realized via [./lib/utils/tqstream.cpp](./lib/utils/tqstream.cpp)).

### Interpreter





