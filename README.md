# MiniLang compiler in C++
# Maria Eleni Batatoudi, mb5017

### Overal Structure of the Project ###
Stage 1: Lexical Analyzer
Stage 2: Parser (Parser Generator, Header for AST)
Stage 3: semantic analyzer with symbol tables and control-flow checks
Stage 4: MIPS code generator producing SPIM assembly

### Design for the Code Generator ###

I followed the stage-based pipeline as mentioned in the PA4 Assignment requirements. Compiler::compile() iterates through 4 stages in order:
    1. LexingParsingStageProcessor
    2. SemanticAnalysisStageProcessor
    3. OptimizationStageProcessor
    4. CodeGenerationStageProcessor
The code generation is the final stage and it only runs after lexing, parsing, and semantic analysis succeed, just like mentioned in the assignment instructions. The CodeGenerationStageProcessor created a CodeGenerator, which is a Visitor over the AST, and returns a combined MIPS string. I maitnain separate enviornemnts for globals and locals using VarLocation and a scope stack, as well as FuncInfo to track function labels, lexical levels, and parameter and return types.
Each function follows the following stack format from the runtime specification manual:
saved $fp at 0($fp), return address at 4($fp), static link at 8($fp) for nested functions, arguments at positive offsets, and locals at negative offsets from $fp.
Calls push arguments right to left, then a static link, then I use jal. The callee binds each formal by copying from the appropriate positive offset into a local slot. Integers and booleans are in temporary $t registers and return through $v0, floats are in $f registers and return through $f0.
Expressions are emitted via emitExpr, which handles literals, identifiers, unary minus, binary operators, function calls, and explicit int and float operations. For print statements, I used SPIM syscalls: print_int for ints and booleans, print_float for floats, and I always print a newline after using the print_char. I also emit a small runtime library in assembly for error handling: global strings for different main() errors and division by zero, a _runtime_error, and a _diz_zero that loads the appropriate messgae and jumps to _runtime_error.
The global entry point main first calls _init_globals, then checks that top-level main exists and is well structured before calling it or printing a runtime error.
