#ifndef SEMANTIC_ANALYZER_HPP
#define SEMANTIC_ANALYZER_HPP

#include "astnode.hpp"

class SemanticAnalyzer {
 private:
    [[maybe_unused]] ASTNode* root;
 public:
    explicit SemanticAnalyzer(ASTNode* root) : root(root) {}
    void analyze();
};


#endif // SEMANTIC_ANALYZER_HPP
