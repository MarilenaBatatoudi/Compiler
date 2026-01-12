#ifndef VISITOR_HPP
#define VISITOR_HPP

class ProgramNode;
class VarDeclNode;
class LetDeclNode;
class FuncDeclNode;
class BlockNode;
class AssignStmtNode;
class PrintStmtNode;
class ReturnStmtNode;
class IfStmtNode;
class WhileStmtNode;
class IntLitNode;
class FloatLitNode;
class BoolLitNode;
class IdNode;
class UnaryOpNode;
class BinaryOpNode;
class CallNode;
class TypeNode;
class ParamNode;

class Visitor {
 public:
    virtual ~Visitor() = default;
    
    // visit methods for all node types
    virtual void visit(ProgramNode* node) = 0;
    virtual void visit(VarDeclNode* node) = 0;
    virtual void visit(LetDeclNode* node) = 0;
    virtual void visit(FuncDeclNode* node) = 0;
    virtual void visit(BlockNode* node) = 0;
    virtual void visit(AssignStmtNode* node) = 0;
    virtual void visit(PrintStmtNode* node) = 0;
    virtual void visit(ReturnStmtNode* node) = 0;
    virtual void visit(IfStmtNode* node) = 0;
    virtual void visit(WhileStmtNode* node) = 0;
    virtual void visit(IntLitNode* node) = 0;
    virtual void visit(FloatLitNode* node) = 0;
    virtual void visit(BoolLitNode* node) = 0;
    virtual void visit(IdNode* node) = 0;
    virtual void visit(UnaryOpNode* node) = 0;
    virtual void visit(BinaryOpNode* node) = 0;
    virtual void visit(CallNode* node) = 0;
    virtual void visit(TypeNode* node) = 0;
    virtual void visit(ParamNode* node) = 0;
};

#endif /* VISITOR_HPP */