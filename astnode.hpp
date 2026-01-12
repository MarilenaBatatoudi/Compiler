#ifndef ASTNODE_HPP
#define ASTNODE_HPP

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <utility>
#include "visitor.hpp"
#include "data_type.hpp"
#include "scope.hpp"

// Forward declaration
class Scope;

// Base ASTNode class
class ASTNode {
 public:
    virtual ~ASTNode() = default;
    virtual void print(int indent = 0) const = 0;
    virtual void accept(Visitor& v) = 0;
};

class DeclNode;
class StmtNode;
class ExpNode;
class TypeNode;
class BlockNode;
class ParamNode;

class CodeItemNode : public ASTNode {};
class DeclNode : public CodeItemNode {};

class ProgramNode : public ASTNode {
 public:
    std::vector<DeclNode*> declarations;
    std::shared_ptr<Scope> scope;
    
    ~ProgramNode();
    void addDecl(DeclNode* decl);
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

enum class BaseType {
    Int, Float, Bool
};

class TypeNode : public ASTNode {
 public:
    BaseType kind;
    explicit TypeNode(BaseType k) : kind(k) {}
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class ParamNode : public ASTNode {
 public:
    std::string name;
    TypeNode* type;
    ParamNode(const std::string& n, TypeNode* t) : name(n), type(t) {}
    ~ParamNode();
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class BlockNode : public ASTNode {
public:
    std::vector<DeclNode*> decls;
    std::vector<StmtNode*> stmts;

    // NEW: items in textual order (decls and stmts mixed)
    std::vector<ASTNode*> orderedItems;

    std::shared_ptr<Scope> scope;

    ~BlockNode();
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class VarDeclNode : public DeclNode {
 public:
    std::string name;
    TypeNode* type;
    ExpNode* init;
    VarDeclNode(const std::string& n, TypeNode* t, ExpNode* e) : name(n), type(t), init(e) {}
    ~VarDeclNode();
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class LetDeclNode : public DeclNode {
 public:
    std::string name;
    TypeNode* type;
    ExpNode* init;

    LetDeclNode(const std::string& n, TypeNode* t, ExpNode* e) : name(n), type(t), init(e) {}
    ~LetDeclNode();
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class FuncDeclNode : public DeclNode {
 public:
    std::string name;
    std::vector<ParamNode*> params;
    TypeNode* retType;
    BlockNode* body;
    std::shared_ptr<Scope> scope;
    
    FuncDeclNode(const std::string& n, const std::vector<ParamNode*>& p, TypeNode* r, BlockNode* b) : name(n), params(p), retType(r), body(b) {}
    ~FuncDeclNode();
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class StmtNode : public ASTNode {};

class AssignStmtNode : public StmtNode {
 public:
    std::string name;
    ExpNode* rhs;
    AssignStmtNode(const std::string& n, ExpNode* e) : name(n), rhs(e) {}
    ~AssignStmtNode();
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class PrintStmtNode : public StmtNode {
 public:
    ExpNode* expr;
    explicit PrintStmtNode(ExpNode* e) : expr(e) {}
    ~PrintStmtNode();
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class ReturnStmtNode : public StmtNode {
 public:
    ExpNode* expr;
    explicit ReturnStmtNode(ExpNode* e) : expr(e) {}
    ~ReturnStmtNode();
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class IfStmtNode : public StmtNode {
 public:
    ExpNode* cond;
    BlockNode* thenBlk;
    BlockNode* elseBlk;
    IfStmtNode(ExpNode* c, BlockNode* t, BlockNode* e) : cond(c), thenBlk(t), elseBlk(e) {}
    ~IfStmtNode();
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class WhileStmtNode : public StmtNode {
 public:
    ExpNode* cond;
    BlockNode* body;
    WhileStmtNode(ExpNode* c, BlockNode* b) : cond(c), body(b) {}
    ~WhileStmtNode();
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class ExpNode : public ASTNode {
 public:
    DataType dataType = DataType::IOTA;
};

class IntLitNode : public ExpNode {
 public:
    int value;
    explicit IntLitNode(int v) : value(v) {}
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class FloatLitNode : public ExpNode {
 public:
    double value;
    explicit FloatLitNode(double v) : value(v) {}
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class BoolLitNode : public ExpNode {
 public:
    bool value;
    explicit BoolLitNode(bool v) : value(v) {}
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class IdNode : public ExpNode {
 public:
    std::string name;
    explicit IdNode(const std::string& n) : name(n) {}
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

enum class UnOp {
    Neg
};

class UnaryOpNode : public ExpNode {
 public:
    UnOp op;
    ExpNode* expr;
    UnaryOpNode(UnOp o, ExpNode* e) : op(o), expr(e) {}
    ~UnaryOpNode();
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

enum class BinOp {
    Add, Sub, Mul, Div, Eq, Neq, Lt, Gt, Le, Ge
};

class BinaryOpNode : public ExpNode {
 public:
    BinOp op;
    ExpNode* left;
    ExpNode* right;
    BinaryOpNode(BinOp o, ExpNode* l, ExpNode* r) : op(o), left(l), right(r) {}
    ~BinaryOpNode();
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

class CallNode : public ExpNode {
 public:
    std::string callee;
    std::vector<ExpNode*> args;
    CallNode(const std::string& c, const std::vector<ExpNode*>& a) : callee(c), args(a) {}
    ~CallNode();
    void print(int indent = 0) const override;
    void accept(Visitor& v) override;
};

#endif // ASTNODE_HPP