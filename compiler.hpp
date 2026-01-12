#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <string>
#include <vector>
#include <memory>
#include "stageprocessor.hpp"
#include "compiler_context.hpp"

class Compiler {
 private:
    CompilerContext ctx;
    std::vector<Stage> stageOrder;
    std::unique_ptr<StageProcessor> getStageProcessor(Stage stage);

 public:
    Compiler(const std::string& sourceFile,
             const std::string& outputFile,
             bool debug = false);

    void compile();
};

#endif /* COMPILER_HPP */
