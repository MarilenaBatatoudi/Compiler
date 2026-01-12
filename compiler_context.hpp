#ifndef COMPILER_CONTEXT_HPP
#define COMPILER_CONTEXT_HPP

#include <string>
#include "astnode.hpp"

struct CompilerContext {
    std::string inputFile;
    std::string outputFile;

    ASTNode* ast = nullptr;
};

#endif /* COMPILER_CONTEXT_HPP */