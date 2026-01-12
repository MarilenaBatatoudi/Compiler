#ifndef STAGEPROCESSOR_HPP
#define STAGEPROCESSOR_HPP

#include <fstream>
#include <string>
#include "compiler_context.hpp"

enum class Stage {
    LEXING_AND_PARSING,
    SEMANTIC_ANALYSIS,
    OPTIMIZATION,
    CODE_GENERATION
};

class StageProcessor {
 public:
    virtual ~StageProcessor() = default;
    virtual bool process(CompilerContext& ctx) = 0;
};

class LexingParsingStageProcessor : public StageProcessor {
 public:
    bool process(CompilerContext& ctx) override;
};

class SemanticAnalysisStageProcessor : public StageProcessor {
 public:
    bool process(CompilerContext& ctx) override;
};

class OptimizationStageProcessor : public StageProcessor {
 public:
    bool process(CompilerContext& ctx) override;
};

class CodeGenerationStageProcessor : public StageProcessor {
 private:
    ASTNode* astRoot = nullptr;
    std::string generateCode();

 public:
    bool process(CompilerContext& ctx) override;
};

#endif /* STAGEPROCESSOR_HPP */
