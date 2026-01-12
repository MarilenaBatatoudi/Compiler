#include "semantic_analyzer.hpp"
#include "exception.hpp"
#include "scope.hpp"
#include "data_type.hpp"
#include <memory>
#include <vector>
#include <string>

// Helper function to convert BaseType to DataType
DataType baseTypeToDataType(BaseType bt) {
    switch (bt) {
        case BaseType::Int: return DataType::INT;
        case BaseType::Float: return DataType::FLOAT;
        case BaseType::Bool: return DataType::BOOL;
    }
    return DataType::IOTA;
}

// Helper function to check type compatibility for assignments
bool isAssignmentCompatible(DataType target, DataType source) {
    if (target == source) { 
        return true;
    }
    
    if (source == DataType::INT && (target == DataType::FLOAT || target == DataType::BOOL)) {
        return true;
    }
    
    if (source == DataType::BOOL && target == DataType::INT) {
        return true;
    }
    return false;
}

// Helper function to check if type is numeric
bool isNumeric(DataType type) {
    return type == DataType::INT || type == DataType::FLOAT;
}

// SCOPE AND TYPE CHECKING VISITOR
class ScopeAndTypeChecker : public Visitor {
 private:
    std::shared_ptr<Scope> currentScope;
    FuncDeclNode* currentFunction = nullptr;  // Track which function we're in
    
 public:
    void visit(ProgramNode* node) override {
        // Create global scope
        node->scope = std::make_shared<Scope>(nullptr);
        currentScope = node->scope;
        
        // Visit all declarations in order
        for (auto* decl : node->declarations) {
            decl->accept(*this);
        }
    }
    
    void visit(VarDeclNode* node) override {
        if (currentScope->existsInCurrentScope(node->name)) {
            SymbolInfo* existing = currentScope->lookup(node->name);
            if (existing && existing->kind == SymbolKind::Function) {
                throw SemanticException(SemanticErrorType::FUNCTION_USED_AS_VARIABLE, SemanticErrorContext::Function(node->name));
            }
            throw SemanticException(SemanticErrorType::REDECLARED_IDENTIFIER, SemanticErrorContext::Identifier(node->name));
        }
        
        DataType declaredType = baseTypeToDataType(node->type->kind);
        node->init->accept(*this);
        DataType initType = node->init->dataType;
        
        // checking type compatibility
        if (!isAssignmentCompatible(declaredType, initType)) {
            throw SemanticException(SemanticErrorType::VAR_DECL_TYPE_MISMATCH, SemanticErrorContext::IdentifierTypeMismatch(node->name, declaredType, initType));
        }
        
        // adding the symbol table
        currentScope->addSymbol(node->name, SymbolKind::Variable, declaredType);
    }
    
    void visit(LetDeclNode* node) override {
        if (currentScope->existsInCurrentScope(node->name)) {
            SymbolInfo* existing = currentScope->lookup(node->name);
            if (existing && existing->kind == SymbolKind::Function) {
                throw SemanticException(SemanticErrorType::FUNCTION_USED_AS_VARIABLE, SemanticErrorContext::Function(node->name));
            }
            throw SemanticException(SemanticErrorType::REDECLARED_IDENTIFIER, SemanticErrorContext::Identifier(node->name));
        }
        

        DataType declaredType = baseTypeToDataType(node->type->kind);
        node->init->accept(*this);
        DataType initType = node->init->dataType;
        
        // checking type compatibility
        if (!isAssignmentCompatible(declaredType, initType)) {
            throw SemanticException(SemanticErrorType::VAR_DECL_TYPE_MISMATCH, SemanticErrorContext::IdentifierTypeMismatch(node->name, declaredType, initType));
        }
        
        // adding a constant to symbol table
        currentScope->addSymbol(node->name, SymbolKind::Constant, declaredType);
    }
    
    void visit(FuncDeclNode* node) override {
        if (currentScope->existsInCurrentScope(node->name)) {
            throw SemanticException(SemanticErrorType::REDECLARED_FUNCTION, SemanticErrorContext::Function(node->name));
        }
        
        DataType returnType = baseTypeToDataType(node->retType->kind);
        std::vector<DataType> paramTypes;
        for (auto* param : node->params) {
            paramTypes.push_back(baseTypeToDataType(param->type->kind));
        }
        
        currentScope->addFunction(node->name, returnType, paramTypes);
        node->scope = std::make_shared<Scope>(currentScope);
        currentScope = node->scope;
        
        // adding the parameters to the function scope
        for (auto* param : node->params) {
            if (currentScope->existsInCurrentScope(param->name)) {
                throw SemanticException(SemanticErrorType::REDECLARED_IDENTIFIER, SemanticErrorContext::Identifier(param->name));
            }
            DataType paramType = baseTypeToDataType(param->type->kind);
            currentScope->addSymbol(param->name, SymbolKind::Variable, paramType);
        }
        
        FuncDeclNode* savedFunction = currentFunction;
        currentFunction = node;
        
        node->body->accept(*this);
        currentFunction = savedFunction; // restorign function body
        currentScope = currentScope->parent; // exiting function scope
    }
    
    void visit(BlockNode* node) override {
        node->scope = std::make_shared<Scope>(currentScope);
        currentScope = node->scope;
        
        for (auto* decl : node->decls) {
            decl->accept(*this);
        }
        for (auto* stmt : node->stmts) {
            stmt->accept(*this);
        }
    
        currentScope = currentScope->parent; // exiting block scope
    }
    
    void visit(AssignStmtNode* node) override {
        SymbolInfo* symbol = currentScope->lookup(node->name);
        if (!symbol) {
            throw SemanticException(SemanticErrorType::UNDECLARED_IDENTIFIER, SemanticErrorContext::Identifier(node->name));
        }
    
        if (symbol->kind == SymbolKind::Function) { 
            throw SemanticException(SemanticErrorType::FUNCTION_USED_AS_VARIABLE, SemanticErrorContext::Function(node->name));
        }
        
        // checking if it's a constant
        if (symbol->kind == SymbolKind::Constant) {
            throw SemanticException(SemanticErrorType::VAR_ASSIGN_TO_CONSTANT, SemanticErrorContext::Identifier(node->name));
        }
        
        node->rhs->accept(*this);
        DataType rhsType = node->rhs->dataType;
        
        if (!isAssignmentCompatible(symbol->type, rhsType)) {
            throw SemanticException(SemanticErrorType::VAR_ASSIGN_TYPE_MISMATCH, SemanticErrorContext::IdentifierTypeMismatch(node->name, symbol->type, rhsType));
        }
    }
    
    void visit(PrintStmtNode* node) override {
        node->expr->accept(*this);
    }
    
    void visit(ReturnStmtNode* node) override {
        node->expr->accept(*this);
        DataType returnType = node->expr->dataType;
        DataType expectedType = baseTypeToDataType(currentFunction->retType->kind);
        
        // checking type compatibility
        if (!isAssignmentCompatible(expectedType, returnType)) {
            throw SemanticException(SemanticErrorType::RETURN_TYPE_MISMATCH, SemanticErrorContext::ReturnTypeMismatch(currentFunction->name, expectedType, returnType));
        }
    }
    
    void visit(IfStmtNode* node) override {
        node->cond->accept(*this);
        if (node->cond->dataType != DataType::BOOL) {
            throw SemanticException(SemanticErrorType::CONDITION_NOT_BOOL, SemanticErrorContext());
        }
        
        node->thenBlk->accept(*this);

        if (node->elseBlk) {
            node->elseBlk->accept(*this);
        }
    }
    
    void visit(WhileStmtNode* node) override {
        node->cond->accept(*this);
        if (node->cond->dataType != DataType::BOOL) {
            throw SemanticException(SemanticErrorType::CONDITION_NOT_BOOL, SemanticErrorContext());
        }
        
        node->body->accept(*this);
    }
    
    void visit(IntLitNode* node) override {
        node->dataType = DataType::INT;
    }
    
    void visit(FloatLitNode* node) override {
        node->dataType = DataType::FLOAT;
    }
    
    void visit(BoolLitNode* node) override {
        node->dataType = DataType::BOOL;
    }
    
    void visit(IdNode* node) override {
        SymbolInfo* symbol = currentScope->lookup(node->name);
        if (!symbol) {
            throw SemanticException(SemanticErrorType::UNDECLARED_IDENTIFIER, SemanticErrorContext::Identifier(node->name));
        }
        
        if (symbol->kind == SymbolKind::Function) {
            throw SemanticException(SemanticErrorType::FUNCTION_USED_AS_VARIABLE, SemanticErrorContext::Function(node->name));
        }
        
        node->dataType = symbol->type;
    }
    
    void visit(UnaryOpNode* node) override {
        node->expr->accept(*this);
        DataType operandType = node->expr->dataType;
        
        if (node->op == UnOp::Neg) {
            if (!isNumeric(operandType)) {
                throw SemanticException(SemanticErrorType::INVALID_UNARY_OPERATION, SemanticErrorContext::ActualType(operandType));
            }
            node->dataType = operandType;
        }
    }
    
    void visit(BinaryOpNode* node) override {
        // type checking both operands
        node->left->accept(*this);
        node->right->accept(*this);
        
        DataType leftType = node->left->dataType;
        DataType rightType = node->right->dataType;
        
        if (node->op == BinOp::Add || node->op == BinOp::Sub || node->op == BinOp::Mul || node->op == BinOp::Div) {
             // both operands must be numeric
            if (!isNumeric(leftType) || !isNumeric(rightType)) {
                std::string opStr;
                if (node->op == BinOp::Add) { 
                    opStr = "+";
                }
                else if (node->op == BinOp::Sub) { 
                    opStr = "-";
                }
                else if (node->op == BinOp::Mul) { 
                    opStr = "*";
                }
                else if (node->op == BinOp::Div) { 
                    opStr = "/";
                }
                
                throw SemanticException(SemanticErrorType::INVALID_BINARY_OPERATION, SemanticErrorContext::InvalidOperationBetweenTypes(opStr, leftType, rightType));
            }
            
            if (leftType == DataType::FLOAT || rightType == DataType::FLOAT) {
                node->dataType = DataType::FLOAT;
            } else {
                node->dataType = DataType::INT;
            }
        }
        else {
            // both operants must be numeric or same type
            bool bothNumeric = isNumeric(leftType) && isNumeric(rightType);
            bool sameType = (leftType == rightType);
            
            if (!bothNumeric && !sameType) {
                std::string opStr;
                if (node->op == BinOp::Lt) { 
                    opStr = "<";
                }
                else if (node->op == BinOp::Gt) { 
                    opStr = ">";
                }
                else if (node->op == BinOp::Le) { 
                    opStr = "<=";
                }
                else if (node->op == BinOp::Ge) { 
                    opStr = ">=";
                }
                else if (node->op == BinOp::Eq) { 
                    opStr = "==";
                }
                else if (node->op == BinOp::Neq) { 
                    opStr = "!=";
                }
                
                throw SemanticException(SemanticErrorType::INVALID_BINARY_OPERATION, SemanticErrorContext::InvalidOperationBetweenTypes(opStr, leftType, rightType));
            }
            
            node->dataType = DataType::BOOL;
        }
    }
    
    void visit(CallNode* node) override {
        SymbolInfo* symbol = currentScope->lookup(node->callee);
        if (!symbol) {
            throw SemanticException(SemanticErrorType::UNDECLARED_FUNCTION, SemanticErrorContext::Function(node->callee));
        }
        
        if (symbol->kind != SymbolKind::Function) {
            throw SemanticException(SemanticErrorType::NOT_A_FUNCTION, SemanticErrorContext::Identifier(node->callee));
        }
        
        // checking argument count
        if (node->args.size() != symbol->paramTypes.size()) {
            throw SemanticException(SemanticErrorType::WRONG_NUMBER_OF_ARGUMENTS, SemanticErrorContext::ArgCount(node->callee, symbol->paramTypes.size(), node->args.size()));
        }
        
        std::vector<DataType> argTypes;
        for (size_t i = 0; i < node->args.size(); i++) {
            node->args[i]->accept(*this);
            DataType argType = node->args[i]->dataType;
            argTypes.push_back(argType);
            
            if (!isAssignmentCompatible(symbol->paramTypes[i], argType)) {
                throw SemanticException(SemanticErrorType::INVALID_SIGNATURE, SemanticErrorContext::Signature(node->callee, symbol->paramTypes, argTypes));
            }
        }
        node->dataType = symbol->type;
    }
    
    void visit(TypeNode*) override {}
    
    void visit(ParamNode*) override {}
};

// ControlFlowChecker
class ControlFlowChecker {
public:
    static void checkProgram(ProgramNode* prog) {
        if (!prog) return;
        
        for (DeclNode* decl : prog->declarations) {
            if (auto* func = dynamic_cast<FuncDeclNode*>(decl)) {
                checkFunction(func);
            }
        }
    }

private:
    static void checkFunction(FuncDeclNode* func) {
        if (!func || !func->body) return;

        bool alwaysReturns = analyzeBlock(func->body);

        DataType retType = typeFromTypeNode(func->retType);
        if (retType != DataType::IOTA && !alwaysReturns) {
            throw SemanticException(
                SemanticErrorType::MISSING_RETURN,
                SemanticErrorContext::Function(func->name));
        }
    }

    static bool analyzeBlock(BlockNode* block) {
        if (!block) { 
            return false;
        }
        return analyzeItems(block->orderedItems);
    }

    static bool analyzeItems(const std::vector<ASTNode*>& items) {
        bool terminated = false;

        for (ASTNode* node : items) {
            if (!node) { 
                continue;
            }

            if (terminated) {
                throw SemanticException(SemanticErrorType::UNREACHABLE_CODE, SemanticErrorContext());
            }

            bool stmtReturns = false;

            if (auto* ret = dynamic_cast<ReturnStmtNode*>(node)) {
                (void)ret;
                stmtReturns = true;
            } else if (auto* ifs = dynamic_cast<IfStmtNode*>(node)) {
                bool thenRet = analyzeBlock(ifs->thenBlk);
                bool elseRet = false;
                if (ifs->elseBlk) {
                    elseRet = analyzeBlock(ifs->elseBlk);
                }
                stmtReturns = (ifs->elseBlk != nullptr) && thenRet && elseRet;
            } else if (auto* w = dynamic_cast<WhileStmtNode*>(node)) {
                analyzeBlock(w->body);
                stmtReturns = false;
            } else if (auto* b = dynamic_cast<BlockNode*>(node)) {
                stmtReturns = analyzeBlock(b);
            }

            if (stmtReturns) {
                terminated = true;
            }
        }

        return terminated;
    }

    static DataType typeFromTypeNode(TypeNode* type) {
        if (!type) { 
            return DataType::IOTA;
        }
        switch (type->kind) {
            case BaseType::Int: return DataType::INT;
            case BaseType::Float: return DataType::FLOAT;
            case BaseType::Bool: return DataType::BOOL;
        }
        return DataType::IOTA;
    }
};

void SemanticAnalyzer::analyze() {
    if (!root) { 
        return;
    }

    // first pass: scope and type checks
    ScopeAndTypeChecker scopeChecker;
    root->accept(scopeChecker);

    // second pass: control-flow analysis (unreachable and missing return)
    if (auto* prog = dynamic_cast<ProgramNode*>(root)) {
        ControlFlowChecker::checkProgram(prog);
    }
}
