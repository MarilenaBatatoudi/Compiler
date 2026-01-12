%option nounput noinput
%option yylineno

%{
    #include "parser.tab.hpp"
    #include "exception.hpp"
    #define YY_DECL int yylex()

    static bool eof_seen = false;

    // Track column manually
    static int yycolumn = 1;

    // Update location for each token
    #define YY_USER_ACTION \
        yylloc.first_line   = yylineno; \
        yylloc.last_line    = yylineno; \
        yylloc.first_column = yycolumn; \
        yylloc.last_column  = yycolumn + yyleng - 1; \
        yycolumn += yyleng;
%}

%%

[ \t\r]+  { /* skip whitespace */ }
\n        { yycolumn = 1; }
"#".*     { /* skip comments */ }

"func"    { return FUNC_KEYWORD; }
"var"     { return VAR_KEYWORD; }
"let"     { return LET_KEYWORD; }
"if"      { return IF_KEYWORD; }
"else"    { return ELSE_KEYWORD; }
"while"   { return WHILE_KEYWORD; }
"print"   { return PRINT_KEYWORD; }
"return"  { return RETURN_KEYWORD; }
"int"     { return INT_KEYWORD; }
"float"   { return FLOAT_KEYWORD; }
"bool"    { return BOOL_KEYWORD; }

[0-9]+                    {
                            yylval.intval = atoi(yytext);
                            return INTEGER_LITERAL;
                         }

[0-9]+"."[0-9]*           {
                            yylval.floatval = atof(yytext);
                            return FLOAT_LITERAL;
                         }

"true"|"false"          {
                            yylval.boolval = (strcmp(yytext,"true")==0);
                            return BOOL_LITERAL;
                         }

[a-zA-Z_][a-zA-Z0-9_]*    {
                            
                            yylval.text = new std::string(yytext);
                            return IDENTIFIER;
                         }


":="  { return ASSIGN_OP; }
"=="  { return EQUAL_OP; }
"!="  { return NEQ_OP; }
"<="  { return LEQ_OP; }
">="  { return GEQ_OP; }
"<"   { return LT_OP; }
">"   { return GT_OP; }
"+"   { return PLUS_OP; }
"-"   { return MINUS_OP; }
"*"   { return MULTIPLY_OP; }
"/"   { return DIVIDE_OP; }

"(" { return LPAREN_DELIMITER; }
")" { return RPAREN_DELIMITER; }
"{" { return LBRACE_DELIMITER; }
"}" { return RBRACE_DELIMITER; }
";" { return SEMI_DELIMITER; }
"," { return COMMA_DELIMITER; }
":" { return COLON_DELIMITER; }
. { throw LexerException(yylloc.first_line, yylloc.first_column); }

<<EOF>> {
    if (!eof_seen) {
        eof_seen = true;
        yylloc.first_line = yylineno;
        yylloc.last_line = yylineno;
        yylloc.first_column = yycolumn;
        yylloc.last_column = yycolumn;
        return END_OF_FILE;
    }
    return 0; // signal end-of-input to Bison
    }
%%
int yywrap() {
    return 1;
}