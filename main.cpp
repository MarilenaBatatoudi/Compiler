#ifndef YYDEBUG
#define YYDEBUG 0
#endif

#include <iostream>
#include <string>
#include "compiler.hpp"

extern int yydebug;

int main(int argc, char** argv) {
    yydebug = YYDEBUG;  // Set to 1 to enable parser debug output

    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <source-file> <output-file>" << std::endl;
        return 1;
    }

    std::string sourceFile = argv[1];
    std::string outputFile = argv[2];

    try {
        Compiler compiler(sourceFile, outputFile);
        compiler.compile();
    } catch (const std::exception& e) {
        std::cerr << "Unexpected exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
