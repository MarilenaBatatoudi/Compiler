#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>

#include "stageprocessor.hpp"
#include "parser.tab.hpp"
#include "astnode.hpp"
#include "semantic_analyzer.hpp"
#include "exception.hpp"
#include "data_type.hpp"
#include "visitor.hpp"

extern FILE* yyin;

bool LexingParsingStageProcessor::process(CompilerContext& ctx) {
    FILE* inputFile = fopen(ctx.inputFile.c_str(), "r");
    if (!inputFile) {
        std::cerr << "Cannot open source file: " << ctx.inputFile << std::endl;
        return false;
    }

    yyin = inputFile;
    ASTNode* root = nullptr;

    try {
        yyparse(&root);
    } catch (const LexerException& e) {
        std::cerr << "Lexer error" << e.what() << std::endl;
        fclose(inputFile);
        return false;
    } catch (const ParserException& e) {
        std::cerr << "Parser error" << e.what() << std::endl;
        fclose(inputFile);
        return false;
    } catch (...) {
        std::cerr << "Unknown error during lexing/parsing" << std::endl;
        fclose(inputFile);
        return false;
    }

    ctx.ast = root;
    fclose(inputFile);
    return true;
}

bool SemanticAnalysisStageProcessor::process(CompilerContext& ctx) {
    if (!ctx.ast) {
        std::cerr << "Semantic error: missing AST" << std::endl;
        return false;
    }

    SemanticAnalyzer semanticAnalyzer(ctx.ast);
    try {
        semanticAnalyzer.analyze();
    } catch (const SemanticException& e) {
        std::cerr << "Semantic error: " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown error during semantic analysis" << std::endl;
        return false;
    }
    return true;
}

bool OptimizationStageProcessor::process(CompilerContext& ctx) {
    // Optional: You can add optimizations here later.
    (void)ctx;
    return true;
}

// ===============================
// MIPS Code Generator (Visitor)
// ===============================

namespace {

struct VariableInfo {
    int offset;  // Offset from $fp
};

struct FunctionContext {
    FuncDeclNode* func = nullptr;
    int nextLocalOffset = 0;  // Negative offsets for locals: -4, -8, ...
    int numParams = 0;
    std::vector<std::map<std::string, VariableInfo>> envStack;
    std::string endLabel;
};

class CodeGenVisitor : public Visitor {
 public:
    std::ostringstream dataSection;
    std::ostringstream textSection;
    int labelCounter = 0;

    // Current function context
    FunctionContext* currentFunc = nullptr;
    std::map<std::string, FunctionContext> functionContexts;

    // Track whether we saw a main function
    bool hasMainFunction = false;

    CodeGenVisitor() {
        // Initialize data and text sections
        dataSection << ".data\n";
        dataSection << "newline_str:\n"
                    << "    .asciiz \"\\n\"\n";
        dataSection << "div_zero_msg:\n"
                    << "    .asciiz \"Runtime Error: Division by zero\\n\"\n";
        dataSection << "missing_main_msg:\n"
                    << "    .asciiz \"Runtime Error: Missing main function\\n\"\n";

        textSection << ".text\n";
    }

    std::string newLabel(const std::string& base) {
        std::ostringstream oss;
        oss << base << "_" << labelCounter++;
        return oss.str();
    }

    void pushEnv() {
        if (!currentFunc) {
            return;
        }
        currentFunc->envStack.emplace_back();
    }

    void popEnv() {
        if (!currentFunc) {
            return;
        }
        if (!currentFunc->envStack.empty()) {
            currentFunc->envStack.pop_back();
        }
    }

    // Look up a variable in the current function's environment stack
    bool lookupVariable(const std::string& name, int& offsetOut) {
        if (!currentFunc) {
            return false;
        }
        for (auto it = currentFunc->envStack.rbegin();
             it != currentFunc->envStack.rend();
             ++it) {
            auto vIt = it->find(name);
            if (vIt != it->end()) {
                offsetOut = vIt->second.offset;
                return true;
            }
        }
        return false;
    }

    // Declare a new local variable in the current environment frame
    int declareLocalVariable(const std::string& name) {
        if (!currentFunc) {
            return 0;
        }
        // Move stack pointer down by 4 bytes for this local
        currentFunc->nextLocalOffset -= 4;
        int offset = currentFunc->nextLocalOffset;

        textSection << "    addi $sp, $sp, -4\n";
        if (currentFunc->envStack.empty()) {
            currentFunc->envStack.emplace_back();
        }
        currentFunc->envStack.back()[name] = {offset};
        return offset;
    }

    // ===========================
    // Visitor implementations
    // ===========================

    void visit(ProgramNode* node) override {
        // First, generate code for all function declarations.
        // (Top-level var/let declarations are ignored here for simplicity.)
        for (DeclNode* decl : node->declarations) {
            if (auto* func = dynamic_cast<FuncDeclNode*>(decl)) {
                if (func->name == "main") {
                    hasMainFunction = true;
                }
                func->accept(*this);
            }
        }

        // Division-by-zero handler
        textSection << "\n# Division-by-zero runtime handler\n";
        textSection << "div_by_zero:\n";
        textSection << "    la $a0, div_zero_msg\n";
        textSection << "    li $v0, 4\n";
        textSection << "    syscall\n";
        textSection << "    li $v0, 10\n";
        textSection << "    syscall\n";

        // If there is no main function, emit a stub main
        if (!hasMainFunction) {
            textSection << "\n# Stub main for missing main function\n";
            textSection << ".globl main\n";
            textSection << "main:\n";
            textSection << "    la $a0, missing_main_msg\n";
            textSection << "    li $v0, 4\n";
            textSection << "    syscall\n";
            textSection << "    li $v0, 10\n";
            textSection << "    syscall\n";
        }
    }

    void visit(FuncDeclNode* node) override {
        // Set up function context
        FunctionContext& ctx = functionContexts[node->name];
        ctx.func = node;
        ctx.nextLocalOffset = 0;
        ctx.numParams = static_cast<int>(node->params.size());
        ctx.envStack.clear();
        ctx.endLabel = newLabel(node->name + "_end");

        FunctionContext* savedFunc = currentFunc;
        currentFunc = &ctx;

        // Push initial environment (for parameters and top-level block vars)
        pushEnv();

        // Compute parameter offsets.
        // Caller pushes arguments left-to-right.
        // After prologue, arguments are at positive offsets from $fp starting at 8.
        int numParams = ctx.numParams;
        for (int i = 0; i < numParams; ++i) {
            ParamNode* param = node->params[i];
            int offset = 8 + 4 * (numParams - 1 - i);
            currentFunc->envStack.back()[param->name] = {offset};
        }

        // Emit function prologue
        textSection << "\n# Function " << node->name << "\n";
        if (node->name == "main") {
            textSection << ".globl main\n";
        }
        textSection << node->name << ":\n";
        textSection << "    addi $sp, $sp, -8\n";
        textSection << "    sw $fp, 4($sp)\n";
        textSection << "    sw $ra, 0($sp)\n";
        textSection << "    move $fp, $sp\n";

        // Visit the body
        if (node->body) {
            node->body->accept(*this);
        }

        // Function epilogue label (for returns to jump to)
        textSection << ctx.endLabel << ":\n";
        textSection << "    move $sp, $fp\n";
        textSection << "    lw $ra, 0($sp)\n";
        textSection << "    lw $fp, 4($sp)\n";
        textSection << "    addi $sp, $sp, 8\n";

        if (node->name == "main") {
            textSection << "    li $v0, 10\n";
            textSection << "    syscall\n";
        } else {
            textSection << "    jr $ra\n";
        }

        // Restore previous function context
        currentFunc = savedFunc;
    }

    void visit(BlockNode* node) override {
        if (!currentFunc) {
            // Ignore global-level blocks for codegen.
            return;
        }

        pushEnv();

        // Use orderedItems so declarations and statements stay in source order
        for (ASTNode* item : node->orderedItems) {
            if (item) {
                item->accept(*this);
            }
        }

        popEnv();
    }

    void visit(VarDeclNode* node) override {
        if (!currentFunc) {
            // Top-level global vars are not handled in this simple codegen.
            return;
        }

        // Evaluate initializer into $t0
        if (node->init) {
            node->init->accept(*this);
        } else {
            // Default init to 0 if none
            textSection << "    li $t0, 0\n";
        }

        int offset = declareLocalVariable(node->name);
        textSection << "    sw $t0, " << offset << "($fp)\n";
    }

    void visit(LetDeclNode* node) override {
        // Same behavior as VarDeclNode, but semantically constant.
        if (!currentFunc) {
            return;
        }

        if (node->init) {
            node->init->accept(*this);
        } else {
            textSection << "    li $t0, 0\n";
        }

        int offset = declareLocalVariable(node->name);
        textSection << "    sw $t0, " << offset << "($fp)\n";
    }

    void visit(AssignStmtNode* node) override {
        if (!currentFunc) {
            return;
        }

        // Compute RHS into $t0
        if (node->rhs) {
            node->rhs->accept(*this);
        }

        int offset = 0;
        if (lookupVariable(node->name, offset)) {
            textSection << "    sw $t0, " << offset << "($fp)\n";
        } else {
            // Fallback: do nothing but leave a comment
            textSection << "    # Warning: assignment to unknown variable "
                        << node->name << "\n";
        }
    }

    void visit(PrintStmtNode* node) override {
        if (!currentFunc) {
            return;
        }

        if (!node->expr) {
            return;
        }

        node->expr->accept(*this);  // result in $t0

        // For simplicity, print all values with print_int (syscall 1)
        textSection << "    move $a0, $t0\n";
        textSection << "    li $v0, 1\n";
        textSection << "    syscall\n";

        // Print newline
        textSection << "    la $a0, newline_str\n";
        textSection << "    li $v0, 4\n";
        textSection << "    syscall\n";
    }

    void visit(ReturnStmtNode* node) override {
        if (!currentFunc) {
            return;
        }

        if (node->expr) {
            node->expr->accept(*this);  // result in $t0
            textSection << "    move $v0, $t0\n";
        }

        // Jump to the function epilogue
        textSection << "    j " << currentFunc->endLabel << "\n";
    }

    void visit(IfStmtNode* node) override {
        if (!currentFunc) {
            return;
        }

        std::string elseLabel = newLabel("if_else");
        std::string endLabel  = newLabel("if_end");

        // Condition into $t0
        if (node->cond) {
            node->cond->accept(*this);
        }

        if (node->elseBlk) {
            // If false, jump to else
            textSection << "    beq $t0, $zero, " << elseLabel << "\n";
            // Then block
            if (node->thenBlk) {
                node->thenBlk->accept(*this);
            }
            textSection << "    j " << endLabel << "\n";
            // Else block
            textSection << elseLabel << ":\n";
            node->elseBlk->accept(*this);
            textSection << endLabel << ":\n";
        } else {
            // No else block
            textSection << "    beq $t0, $zero, " << endLabel << "\n";
            if (node->thenBlk) {
                node->thenBlk->accept(*this);
            }
            textSection << endLabel << ":\n";
        }
    }

    void visit(WhileStmtNode* node) override {
        if (!currentFunc) {
            return;
        }

        std::string startLabel = newLabel("while_start");
        std::string endLabel   = newLabel("while_end");

        textSection << startLabel << ":\n";

        if (node->cond) {
            node->cond->accept(*this);
        }
        // If condition is false, exit loop
        textSection << "    beq $t0, $zero, " << endLabel << "\n";

        if (node->body) {
            node->body->accept(*this);
        }

        textSection << "    j " << startLabel << "\n";
        textSection << endLabel << ":\n";
    }

    // ===== Expressions: result is always in $t0 =====

    void visit(IntLitNode* node) override {
        textSection << "    li $t0, " << node->value << "\n";
    }

    void visit(FloatLitNode* node) override {
        // Simple placeholder: treat float as int bits via cast.
        // For full float support you would use $f registers and float ops.
        textSection << "    li $t0, " << static_cast<int>(node->value) << "\n";
    }

    void visit(BoolLitNode* node) override {
        textSection << "    li $t0, " << (node->value ? 1 : 0) << "\n";
    }

    void visit(IdNode* node) override {
        if (!currentFunc) {
            // No function context, treat as 0
            textSection << "    li $t0, 0\n";
            return;
        }

        int offset = 0;
        if (lookupVariable(node->name, offset)) {
            textSection << "    lw $t0, " << offset << "($fp)\n";
        } else {
            textSection << "    # Unknown variable " << node->name
                        << ", default to 0\n";
            textSection << "    li $t0, 0\n";
        }
    }

    void visit(UnaryOpNode* node) override {
        if (!node->expr) {
            textSection << "    li $t0, 0\n";
            return;
        }

        node->expr->accept(*this);  // result in $t0

        if (node->op == UnOp::Neg) {
            textSection << "    subu $t0, $zero, $t0\n";
        }
    }

    void visit(BinaryOpNode* node) override {
        if (!node->left || !node->right) {
            textSection << "    li $t0, 0\n";
            return;
        }

        // Evaluate left -> $t0, push on stack
        node->left->accept(*this);
        textSection << "    addi $sp, $sp, -4\n";
        textSection << "    sw $t0, 0($sp)\n";

        // Evaluate right -> $t0
        node->right->accept(*this);

        // Pop left into $t1
        textSection << "    lw $t1, 0($sp)\n";
        textSection << "    addi $sp, $sp, 4\n";

        switch (node->op) {
            case BinOp::Add:
                textSection << "    add $t0, $t1, $t0\n";
                break;
            case BinOp::Sub:
                textSection << "    sub $t0, $t1, $t0\n";
                break;
            case BinOp::Mul:
                textSection << "    mul $t0, $t1, $t0\n";
                break;
            case BinOp::Div:
                // Check division by zero: right operand in $t0
                textSection << "    beq $t0, $zero, div_by_zero\n";
                textSection << "    div $t1, $t0\n";
                textSection << "    mflo $t0\n";
                break;
            case BinOp::Eq:
                textSection << "    seq $t0, $t1, $t0\n";
                break;
            case BinOp::Neq:
                textSection << "    sne $t0, $t1, $t0\n";
                break;
            case BinOp::Lt:
                textSection << "    slt $t0, $t1, $t0\n";
                break;
            case BinOp::Gt:
                textSection << "    sgt $t0, $t1, $t0\n";
                break;
            case BinOp::Le:
                textSection << "    sle $t0, $t1, $t0\n";
                break;
            case BinOp::Ge:
                textSection << "    sge $t0, $t1, $t0\n";
                break;
        }
    }

    void visit(CallNode* node) override {
        if (!currentFunc) {
            // Calls only make sense inside a function
            textSection << "    li $t0, 0\n";
            return;
        }

        // Push arguments left-to-right
        for (ExpNode* arg : node->args) {
            if (!arg) {
                continue;
            }
            arg->accept(*this);  // result in $t0
            textSection << "    addi $sp, $sp, -4\n";
            textSection << "    sw $t0, 0($sp)\n";
        }

        // Call function
        textSection << "    jal " << node->callee << "\n";

        // Pop arguments
        if (!node->args.empty()) {
            int totalBytes = static_cast<int>(node->args.size()) * 4;
            textSection << "    addi $sp, $sp, " << totalBytes << "\n";
        }

        // Get return value from $v0 into $t0
        textSection << "    move $t0, $v0\n";
    }

    void visit(TypeNode*) override {
        // No code needed
    }

    void visit(ParamNode*) override {
        // Parameters handled in FuncDeclNode
    }
};

}  // anonymous namespace

// ===============================
// CodeGenerationStageProcessor
// ===============================

std::string CodeGenerationStageProcessor::generateCode() {
    if (!astRoot) {
        return "";
    }

    ProgramNode* program = dynamic_cast<ProgramNode*>(astRoot);
    if (!program) {
        return "";
    }

    CodeGenVisitor gen;
    program->accept(gen);

    std::ostringstream full;
    full << gen.dataSection.str() << "\n" << gen.textSection.str();
    return full.str();
}

bool CodeGenerationStageProcessor::process(CompilerContext& ctx) {
    if (!ctx.ast) {
        std::cerr << "Code generation error: missing AST" << std::endl;
        return false;
    }

    astRoot = ctx.ast;

    std::ofstream out(ctx.outputFile);
    if (!out) {
        std::cerr << "Error opening output file: " << ctx.outputFile << std::endl;
        return false;
    }

    std::string code = generateCode();
    out << code;
    out.close();

    return true;
}
