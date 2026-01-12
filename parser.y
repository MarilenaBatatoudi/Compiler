%code requires {
    #include "astnode.hpp"
} 
%{
#include <vector>
#include <string>
#include "astnode.hpp"
#include "exception.hpp"

#include <cstdio>
#include <cstdlib>

#define YYDEBUG 1

void yyerror(ASTNode** root, const char* s);
extern int yylex();
static TypeNode* makeTypeFromToken(int tok);

%}
%parse-param { ASTNode** root }
%locations

%union {
    // Modify or add more types as needed
    ProgramNode* program;

    DeclNode* decl;
    class StmtNode* stmt;
    class ExpNode* exp;
    TypeNode* type;
    class BlockNode* block;
    
    class ParamNode* param;
    std::vector<ParamNode*>* param_list;
    std::vector<DeclNode*>* decl_list;
    std::vector<class ExpNode*>* exp_list;
    
    std::string* text;
    int intval;
    double floatval;
    bool boolval;
}

// If you make changes here, remember to update the corresponding parts in lex file as well
%token <floatval> FLOAT_LITERAL
%token <intval> INTEGER_LITERAL 
%token <boolval> BOOL_LITERAL
%token <text> IDENTIFIER
%token FUNC_KEYWORD VAR_KEYWORD LET_KEYWORD IF_KEYWORD ELSE_KEYWORD WHILE_KEYWORD PRINT_KEYWORD RETURN_KEYWORD INT_KEYWORD FLOAT_KEYWORD BOOL_KEYWORD
%token ASSIGN_OP EQUAL_OP NEQ_OP LT_OP GT_OP LEQ_OP GEQ_OP
%token PLUS_OP MINUS_OP MULTIPLY_OP DIVIDE_OP
%token LPAREN_DELIMITER RPAREN_DELIMITER LBRACE_DELIMITER RBRACE_DELIMITER SEMI_DELIMITER COMMA_DELIMITER COLON_DELIMITER
%token END_OF_FILE

// adding precedence
%left EQUAL_OP NEQ_OP
%left LT_OP GT_OP LEQ_OP GEQ_OP
%left PLUS_OP MINUS_OP
%left MULTIPLY_OP DIVIDE_OP
%precedence UMINUS


// Modify or add more %type declarations as needed
%type <program> program
%type <decl> decl var_decl let_decl func_decl
%type <decl_list> decl_list
%type <block> block block_items
%type <stmt> stmt
%type <type> type
%type <param> param
%type <param_list> opt_params param_list
%type <exp> exp primary call
%type <exp_list> opt_args arg_list

%start program

%%
// Modify or add other grammar rules here...
program
    : decl_list END_OF_FILE {
        ProgramNode* ast = new ProgramNode();
        for (auto* decl : *$1)
            ast->addDecl(decl);
        *root = ast;
        $$ = ast;
    }
    ;
decl_list
    : decl_list decl {
        $$ = $1;
        $$->push_back($2);
    }
    | decl {
        $$ = new std::vector<DeclNode*>;
        $$->push_back($1);
    }
    ;
    
//DECLARATIONS 

decl
    : var_decl {
        $$ = $1;
    }
    | let_decl { 
        $$ = $1; 
    }
    | func_decl { 
        $$ = $1; 
    }
    ;
var_decl
    : VAR_KEYWORD IDENTIFIER COLON_DELIMITER type ASSIGN_OP exp SEMI_DELIMITER {
        std::string name = *$2; delete $2;
        $$ = new VarDeclNode(name, $4, $6);
    }
    ;
let_decl
    : LET_KEYWORD IDENTIFIER COLON_DELIMITER type ASSIGN_OP exp SEMI_DELIMITER {
        std::string name = *$2; 
        delete $2;
        $$ = new LetDeclNode(name, $4, $6);
    }
    ;
func_decl
    : FUNC_KEYWORD IDENTIFIER LPAREN_DELIMITER opt_params RPAREN_DELIMITER COLON_DELIMITER type LBRACE_DELIMITER block RBRACE_DELIMITER {
        std::string name = *$2;
        delete $2;
        $$ = new FuncDeclNode(name, *$4, $7, $9);
        delete $4;
    }
    ;
    
//PARAMETERS 

opt_params
    : %empty { 
        $$ = new std::vector<ParamNode*>(); 
    }
    | param_list { 
        $$ = $1; 
    }
    ;

param_list
    : param { 
        $$ = new std::vector<ParamNode*>(); 
        $$->push_back($1); 
    }
    | param_list COMMA_DELIMITER param { 
        $$ = $1; 
        $$->push_back($3);
    }
    ;

param
    : IDENTIFIER COLON_DELIMITER type {
        std::string name = *$1; 
        delete $1;
        $$ = new ParamNode(name, $3);
    }
    ;

//TYPES 

type
    : INT_KEYWORD {
        $$ = makeTypeFromToken(INT_KEYWORD);
    }
    | FLOAT_KEYWORD {
        $$ = makeTypeFromToken(FLOAT_KEYWORD);
    }
    | BOOL_KEYWORD {
        $$ = makeTypeFromToken(BOOL_KEYWORD);
    }
    ;

//BLOCKS

block
    : %empty {
        $$ = new BlockNode();
    }
    | block_items {
        $$ = $1;
    }
    ;

block_items
    : block_items decl {
        $$ = $1;
        $$->decls.push_back($2);
        $$->orderedItems.push_back($2);   // NEW: preserve order
    }
    | block_items stmt {
        $$ = $1;
        $$->stmts.push_back($2);
        $$->orderedItems.push_back($2);   // NEW: preserve order
    }
    | decl {
        $$ = new BlockNode();
        $$->decls.push_back($1);
        $$->orderedItems.push_back($1);   // NEW: first item
    }
    | stmt {
        $$ = new BlockNode();
        $$->stmts.push_back($1);
        $$->orderedItems.push_back($1);   // NEW: first item
    }
    ;

//STATEMENTS 

stmt
    : PRINT_KEYWORD LPAREN_DELIMITER exp RPAREN_DELIMITER SEMI_DELIMITER {
        $$ = new PrintStmtNode($3);
    }
    | IF_KEYWORD LPAREN_DELIMITER exp RPAREN_DELIMITER LBRACE_DELIMITER block RBRACE_DELIMITER {
      $$ = new IfStmtNode($3, $6, nullptr);
    }
    | IF_KEYWORD LPAREN_DELIMITER exp RPAREN_DELIMITER LBRACE_DELIMITER block RBRACE_DELIMITER ELSE_KEYWORD LBRACE_DELIMITER block RBRACE_DELIMITER {
        $$ = new IfStmtNode($3, $6, $10);
    }
    | WHILE_KEYWORD LPAREN_DELIMITER exp RPAREN_DELIMITER LBRACE_DELIMITER block RBRACE_DELIMITER {
        $$ = new WhileStmtNode($3, $6);
    }
    | IDENTIFIER ASSIGN_OP exp SEMI_DELIMITER {
        std::string name = *$1; delete $1;
        $$ = new AssignStmtNode(name, $3);
    }
    | RETURN_KEYWORD exp SEMI_DELIMITER {
        $$ = new ReturnStmtNode($2);
    }
    ;
    
//EXPRESSIONS

exp
    : exp PLUS_OP exp {
        $$ = new BinaryOpNode(BinOp::Add, $1, $3);
    }
    | exp MINUS_OP exp {
        $$ = new BinaryOpNode(BinOp::Sub, $1, $3);
    }
    | exp MULTIPLY_OP exp {
        $$ = new BinaryOpNode(BinOp::Mul, $1, $3);
    }
    | exp DIVIDE_OP exp {
        $$ = new BinaryOpNode(BinOp::Div, $1, $3);
    }
    | exp EQUAL_OP exp {
        $$ = new BinaryOpNode(BinOp::Eq, $1, $3);
    }
    | exp NEQ_OP exp {
        $$ = new BinaryOpNode(BinOp::Neq, $1, $3); 
    }
    | exp LT_OP exp {
        $$ = new BinaryOpNode(BinOp::Lt, $1, $3);
    }
    | exp GT_OP exp {
        $$ = new BinaryOpNode(BinOp::Gt, $1, $3);
    }
    | exp LEQ_OP exp {
        $$ = new BinaryOpNode(BinOp::Le, $1, $3);
    }
    | exp GEQ_OP exp {
        $$ = new BinaryOpNode(BinOp::Ge, $1, $3);
    }
    | MINUS_OP exp %prec UMINUS {
        $$ = new UnaryOpNode(UnOp::Neg, $2);
    }
    | call {
        $$ = $1;
    }
    | primary {
        $$ = $1;
    }
    ;
    
call
    : IDENTIFIER LPAREN_DELIMITER opt_args RPAREN_DELIMITER {
        std::string callee = *$1;
        delete $1;
        $$ = new CallNode(callee, *$3);
        delete $3;
    }
    ;

primary
    : INTEGER_LITERAL {
        $$ = new IntLitNode($1);
    }
    | FLOAT_LITERAL {
        $$ = new FloatLitNode($1);
    }
    | BOOL_LITERAL {
        $$ = new BoolLitNode($1);
    }
    | IDENTIFIER {
        std::string name = *$1; delete $1;
        $$ = new IdNode(name);
    }
    | LPAREN_DELIMITER exp RPAREN_DELIMITER {
        $$ = $2;
    }
    ;

opt_args
    : %empty {
        $$ = new std::vector<ExpNode*>();
    }
    | arg_list {
        $$ = $1;
    }
    ;

arg_list
    : exp {
        $$ = new std::vector<ExpNode*>();
        $$->push_back($1);
    }
    | arg_list COMMA_DELIMITER exp {
        $$ = $1; 
        $$->push_back($3);
    }
    ;
    
%%

static TypeNode* makeTypeFromToken(int token) {
    if (token == INT_KEYWORD) {
        return new TypeNode(BaseType::Int);
    }
    if (token == FLOAT_KEYWORD) {
        return new TypeNode(BaseType::Float);
    }
    if (token == BOOL_KEYWORD) {
        return new TypeNode(BaseType::Bool);
    }
    return nullptr;
}

extern int yychar;
void yyerror(ASTNode**, const char*) {
    int col;
    if ((yychar == END_OF_FILE) || (yychar == 0)) {
        col = yylloc.last_column;
    }
    else {
        col = yylloc.first_column;
    }
    
    const int line = yylloc.first_line;
    
    throw ParserException(line, col);

}
