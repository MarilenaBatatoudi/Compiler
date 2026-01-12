#include <iostream>
#include "astnode.hpp"
#include "visitor.hpp"
#include "data_type.hpp"

#include <string>
#include <vector>

void printIndent(int indent) {
    for (int i = 0; i < indent; ++i) std::cout << " ";
}

static const char* typeToStr(BaseType t) {
    switch (t) {
        case BaseType::Int: return "int";
        case BaseType::Float: return "float";
        case BaseType::Bool: return "bool";
    }
    return "?";
}

static const char* unOpToStr(UnOp o) {
    switch (o) {
        case UnOp::Neg: return "-";
    }
    return "?";
}

static const char* binOpToStr(BinOp o) {
    switch (o) {
        case BinOp::Add: return "+";
        case BinOp::Sub: return "-";
        case BinOp::Mul: return "*";
        case BinOp::Div: return "/";
        case BinOp::Eq: return "==";
        case BinOp::Neq: return "!=";
        case BinOp::Lt: return "<";
        case BinOp::Gt: return ">";
        case BinOp::Le: return "<=";
        case BinOp::Ge: return ">=";
    }
    return "?";
}


// ProgramNode implementation
ProgramNode::~ProgramNode() {
    for (auto& decl : declarations) delete decl;
}

void ProgramNode::addDecl(DeclNode* decl) {
    declarations.push_back(decl);
}

void ProgramNode::print(int indent) const {
    printIndent(indent);
    std::cout << "ProgramNode:\n";
    for (auto& decl : declarations) decl->print(indent + 2);
}

// Can be removed if not using the Visitor pattern
void ProgramNode::accept(Visitor& v) {
    v.visit(this);
}


// TypeNode
void TypeNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Type: " << typeToStr(kind) << "\n";
}
void TypeNode::accept(Visitor& v) {
    v.visit(this);
}

// ParamNode
ParamNode::~ParamNode() {
    delete type;
}

void ParamNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Param: " << name << "\n";
    if (type) { 
        type->print(indent + 2);
    }
}
void ParamNode::accept(Visitor& v) {
    v.visit(this);
}

// BlockNode
BlockNode::~BlockNode() {
    for (auto* decl : decls) { 
        delete decl;
    }
    for (auto* s : stmts) { 
        delete s;
    }
}

void BlockNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Block\n";

    if (!decls.empty()) {
        printIndent(indent + 2);
        std::cout << "Decls:\n";
        for (const auto* d : decls) { 
            if (d) { 
                d->print(indent + 4);
            }
        }
    }

    if (!stmts.empty()) {
        printIndent(indent + 2);
        std::cout << "Stmts:\n";
        for (const auto* s : stmts) { 
            if (s) { 
                s->print(indent + 4);
            }
        }
    }
}
void BlockNode::accept(Visitor& v) {
    v.visit(this);
}

// VarDeclNode
VarDeclNode::~VarDeclNode() {
    delete type;
    delete init;
}

void VarDeclNode::print(int indent) const {
    printIndent(indent);
    std::cout << "VarDecl: " << name << "\n";
    if (type) {
        printIndent(indent + 2);
        std::cout << "Type:\n";
        type->print(indent + 4);
    }
    if (init) {
        printIndent(indent + 2);
        std::cout << "Init:\n";
        init->print(indent + 4);
    }
}
void VarDeclNode::accept(Visitor& v) {
    v.visit(this);
}

// LetDeclNode
LetDeclNode::~LetDeclNode() {
    delete type;
    delete init;
}

void LetDeclNode::print(int indent) const {
    printIndent(indent);
    std::cout << "LetDecl: " << name << "\n";
    if (type) {
        printIndent(indent + 2);
        std::cout << "Type:\n";
        type->print(indent + 4);
    }
    if (init) {
        printIndent(indent + 2);
        std::cout << "Init:\n";
        init->print(indent + 4);
    }
}
void LetDeclNode::accept(Visitor& v) {
    v.visit(this);
}

// FuncDeclNode
FuncDeclNode::~FuncDeclNode() {
    for (auto* p : params) { 
        delete p;
    }
    delete retType;
    delete body;
}

void FuncDeclNode::print(int indent) const {
    printIndent(indent);
    std::cout << "FuncDecl: " << name << "\n";

    if (!params.empty()) {
        printIndent(indent + 2);
        std::cout << "Params:\n";
        for (const auto* p : params) { 
            if (p) { 
                p->print(indent + 4);
            }
        }
    }

    if (retType) {
        printIndent(indent + 2);
        std::cout << "ReturnType:\n";
        retType->print(indent + 4);
    }

    if (body) {
        printIndent(indent + 2);
        std::cout << "Body:\n";
        body->print(indent + 4);
    }
}
void FuncDeclNode::accept(Visitor& v) {
    v.visit(this);
}

// AssignStmtNode
AssignStmtNode::~AssignStmtNode() {
    delete rhs;
}

void AssignStmtNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Assign: " << name << "\n";
    if (rhs) { 
        rhs->print(indent + 2);
    }
}
void AssignStmtNode::accept(Visitor& v) {
    v.visit(this);
}

// PrintStmtNode
PrintStmtNode::~PrintStmtNode() {
    delete expr;
}

void PrintStmtNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Print\n";
    if (expr) { 
        expr->print(indent + 2);
    }
}
void PrintStmtNode::accept(Visitor& v) {
    v.visit(this);
}

// ReturnStmtNode
ReturnStmtNode::~ReturnStmtNode() {
    delete expr;
}

void ReturnStmtNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Return\n";
    if (expr) { 
        expr->print(indent + 2);
    }
}
void ReturnStmtNode::accept(Visitor& v) {
    v.visit(this);
}

// IfStmtNode
IfStmtNode::~IfStmtNode() {
    delete cond;
    delete thenBlk;
    delete elseBlk;
}

void IfStmtNode::print(int indent) const {
    printIndent(indent);
    std::cout << "If\n";
    if (cond) {
        printIndent(indent + 2);
        std::cout << "Cond:\n";
        cond->print(indent + 4);
    }
    if (thenBlk) {
        printIndent(indent + 2);
        std::cout << "Then:\n";
        thenBlk->print(indent + 4);
    }
    if (elseBlk) {
        printIndent(indent + 2);
        std::cout << "Else:\n";
        elseBlk->print(indent + 4);
    }
}
void IfStmtNode::accept(Visitor& v) {
    v.visit(this);
}

// WhileStmtNode
WhileStmtNode::~WhileStmtNode() {
    delete cond;
    delete body;
}

void WhileStmtNode::print(int indent) const {
    printIndent(indent);
    std::cout << "While\n";
    if (cond) {
        printIndent(indent + 2);
        std::cout << "Cond:\n";
        cond->print(indent + 4);
    }
    if (body) {
        printIndent(indent + 2);
        std::cout << "Body:\n";
        body->print(indent + 4);
    }
}
void WhileStmtNode::accept(Visitor& v) {
    v.visit(this);
}

// LitNodes
void IntLitNode::print(int indent) const {
    printIndent(indent);
    std::cout << "IntLit: " << value << "\n";
}
void IntLitNode::accept(Visitor& v) {
    v.visit(this);
}

void FloatLitNode::print(int indent) const {
    printIndent(indent);
    std::cout << "FloatLit: " << value << "\n";
}
void FloatLitNode::accept(Visitor& v) {
    v.visit(this);
}

void BoolLitNode::print(int indent) const {
    printIndent(indent);
    std::cout << "BoolLit: ";
    if (value) {
        std::cout << "true\n";
    }
    else {
        std::cout << "false\n";
    }
}
void BoolLitNode::accept(Visitor& v) {
    v.visit(this);
}

// IdNode
void IdNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Id: " << name << "\n";
}
void IdNode::accept(Visitor& v) {
    v.visit(this);
}

// UnaryOpNode
UnaryOpNode::~UnaryOpNode() {
    delete expr;
}

void UnaryOpNode::print(int indent) const {
    printIndent(indent);
    std::cout << "UnaryOp: " << unOpToStr(op) << "\n";
    if (expr) { 
        expr->print(indent + 2);
    }
}
void UnaryOpNode::accept(Visitor& v) {
    v.visit(this);
}

// BinaryOpNode
BinaryOpNode::~BinaryOpNode() {
    delete left;
    delete right;
}

void BinaryOpNode::print(int indent) const {
    printIndent(indent);
    std::cout << "BinaryOp: " << binOpToStr(op) << "\n";
    if (left) {
        printIndent(indent + 2);
        std::cout << "LHS:\n";
        left->print(indent + 4);
    }
    if (right) {
        printIndent(indent + 2);
        std::cout << "RHS:\n";
        right->print(indent + 4);
    }
}
void BinaryOpNode::accept(Visitor& v) {
    v.visit(this);
}

// CallNode
CallNode::~CallNode() {
    for (auto* p : args) {
        delete p;
    }
}

void CallNode::print(int indent) const {
    printIndent(indent);
    std::cout << "Call: " << callee << "\n";
    
    if (args.empty()) {
        return;
    }
    printIndent(indent + 2);
    std::cout << "Args:\n";
    
    for (std::size_t i=0; i < args.size(); ++i) { 
        if (args[i]) { 
            args[i]->print(indent + 4);
        }
    }
}
void CallNode::accept(Visitor& v) {
    v.visit(this);
}