#include <memory>
#include "compiler.hpp"
#include "compiler_context.hpp"
#include "stageprocessor.hpp"

Compiler::Compiler(const std::string& sourceFile,
                   const std::string& outputFile,
                   bool /*debug*/) {
    ctx = {
        .inputFile = sourceFile,
        .outputFile = outputFile,
        .ast = nullptr,
    };

    stageOrder = {
        Stage::LEXING_AND_PARSING,
        Stage::SEMANTIC_ANALYSIS,
        Stage::OPTIMIZATION,
        Stage::CODE_GENERATION
    };
}

std::unique_ptr<StageProcessor> Compiler::getStageProcessor(Stage stage) {
    switch (stage) {
        case Stage::LEXING_AND_PARSING:
            return std::make_unique<LexingParsingStageProcessor>();
        case Stage::SEMANTIC_ANALYSIS:
            return std::make_unique<SemanticAnalysisStageProcessor>();
        case Stage::OPTIMIZATION:
            return std::make_unique<OptimizationStageProcessor>();
        case Stage::CODE_GENERATION:
            return std::make_unique<CodeGenerationStageProcessor>();
    }
    return nullptr;
}

void Compiler::compile() {
    for (const auto& stage : stageOrder) {
        std::unique_ptr<StageProcessor> processor = getStageProcessor(stage);
        if (processor) {
            bool ok = processor->process(this->ctx);
            if (!ok) {
                // Stop compilation on the first failing stage.
                break;
            }
        }
    }
}
