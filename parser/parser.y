%{
#include <stdio.h>
#include "parser/scanner.hh"
%}

// version of bison
%require "3.8"

// use c++ language
%language "c++"

// c++ namespace
%define api.namespace {parser}

// parser's class name
%define api.parser.class {Parser}

// use variant instead of the old union format
%define api.value.type variant

// get more detailed information when error occurs
%define parse.error detailed

// parser constructor's parameter, it is much easier 
// to use yyscan_t than c++ class
%param {yyscan_t scanner}

// tracking location
%locations

%code requires
{
    #include <vector>
    #include <string>
    #include <sstream>

    #include "token.h"
    #include "node.h"
    #include "type.h"

    using namespace ast;

    string string_value(const string& image);
    char character_code(const string& image);
    IntegerLiteralNode* integer_node(const Location &loc, const string& image);
}

// inefficient but works
%code provides
{
    #define YY_USER_ACTION \
        loc->begin.line = loc->end.line = yylineno; \
        loc->begin.column = yycolumn + 1; \
        for (int i = 0; yytext[i] != 0; i++) { \
            if (yytext[i] == '\n') { \
                loc->end.line++; \
                loc->end.column = 0; \
                yycolumn = 0; \
            } else { \
                loc->end.column++; \
                ++yycolumn; \
            } \
        }
}

%code provides
{
    #define YY_FINISH_TOKEN \
        tok.begin_line_ = loc->begin.line; \
        tok.begin_column_ = loc->begin.column; \
        tok.end_line_ = loc->end.line; \
        tok.end_column_ = loc->end.column; \
        yylval->emplace<Token>(tok); \
        return tok.kind_;
}

// out yylex version
%code provides
{
    #define YY_DECL \
        int yylex(parser::Parser::semantic_type *yylval, \
            parser::Parser::location_type *loc, yyscan_t yyscanner) \

    YY_DECL;
}

%token <Token> VOID CHAR SHORT INT LONG 
%token <Token> TYPEDEF STRUCT UNION ENUM 
%token <Token> STATIC EXTERN 
%token <Token> SIGNED UNSIGNED CONST
%token <Token> IF ELSE SWITCH CASE DEFAULT WHILE DO FOR RETURN BREAK CONTINUE GOTO
%token <Token> IMPORT SIZEOF
%token <Token> IDENTIFIER INTEGER CHARACTER STRING

%type <int> compilation_unit import_stmts top_defs
%type <int> def_func def_vars def_const def_union def_typedef
%type <int> import_stmt
%type <int> storage
%type <int> type typeref_base
%type <vector<ExprNode*>> args
%type <ExprNode*> expr
%type <ExprNode*> postfix
%type <ExprNode*> primary
%type <string> name

%start compilation_unit

%destructor { delete $$; } <ExprNode*> 

%%
compilation_unit : top_defs 
        | import_stmts top_defs
        ;

import_stmts : import_stmt 
        | import_stmts import_stmt 
        ;

import_stmt : IMPORT ';' 


top_defs : def_func
        | def_vars
        | def_const 
        | def_struct 
        | def_union 
        | def_typedef 
        | top_defs def_func 
        | top_defs def_vars 
        | top_defs def_const 
        | top_defs def_struct 
        | top_defs def_union 
        | top_defs def_typedef 
        ;

/* XXX: typeref and type is different? */
def_func : storage type name '(' params ')' block 
        ;

def_var_list : def_vars 
        | def_var_list def_vars 
        ;

def_vars : storage type name '=' expr ';' 
        | storage type name ';' 
        | def_vars ',' storage type name '=' expr ';' 
        | def_vars ',' storage type name ';' 
        ;

def_const : CONST type name '=' expr ';'  

 /* TODO: need to check null struct */
def_struct : STRUCT name member_list ';'
    
/* TODO: need to check null union */
def_union : UNION name member_list ';'


def_typedef : TYPEDEF typeref IDENTIFIER ';'

params : VOID 
        | fixed_params 
        | fixed_params ',' "..."
        ;

fixed_params : param
        | fixed_params ',' param 
        ;

param_typerefs : VOID
        | fixed_param_typerefs
        | fixed_param_typerefs ',' "..."
        ;

fixed_param_typerefs : typeref
        | fixed_param_typerefs ',' typeref
        ;

param : type name
        ;

block : '{' '}' 
        | '{' stmts '}'
        | '{' def_var_list stmts '}'
        ;

storage : %empty
        | STATIC
        ;

type : typeref
        ;

typeref : typeref_base 
        | typeref_base '[' ']'
        | typeref_base '[' INTEGER ']'
        | typeref_base '*'
        | typeref_base '(' param_typerefs ')'
        | typeref '[' ']'
        | typeref '[' INTEGER ']'
        | typeref '*'
        | typeref '(' param_typerefs ')'
        ;

stmts : stmt
        | stmts stmt
        ;

stmt : ';'
        | label_stmt
        | expr ';'
        | block
        | if_stmt
        | while_stmt
        | dowhile_stmt
        | for_stmt
        | switch_stmt
        | break_stmt
        | continue_stmt
        | goto_stmt
        | return_stmt
        ;

label_stmt : IDENTIFIER ':' stmt
        ;

if_stmt : IF '(' expr ')' stmt ELSE stmt
        | IF '(' expr ')' stmt
        ;

while_stmt : WHILE '(' expr ')' stmt

dowhile_stmt : DO stmt WHILE '(' expr ')' ';'

for_stmt : FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' stmt

goto_stmt : GOTO IDENTIFIER ';'

switch_stmt : SWITCH '(' expr ')' '{' case_clauses '}'

case_clauses : default_clause
        | case_clause
        | case_clauses default_clause
        | case_clauses case_clause
        ;

case_clause : cases
        | cases case_body
        ;

cases : CASE primary ':'

case_body : stmt

default_clause : DEFAULT ':'
        | DEFAULT ':' case_body
        ;

return_stmt : RETURN ';'
        | RETURN expr ';'
        ;

continue_stmt : CONTINUE ';'

break_stmt : BREAK ';'

member_list : '{' '}'
        | '{' slots '}' 
        ;

slots : type name ';'
        | slots type name ';'
        ;

opt_expr : %empty
        | expr
        ;

typeref_base : VOID
        | CHAR
        | SHORT
        | INT
        | LONG
        | UNSIGNED CHAR
        | UNSIGNED SHORT
        | UNSIGNED INT
        | UNSIGNED LONG
        | STRUCT IDENTIFIER
        | UNION IDENTIFIER
        | IDENTIFIER
        ;

expr : term '=' expr
    | term assign_op expr
    | expr10
    ;

assign_op : "+=" 
        | "-="
        | "*="
        | "/="
        | "%="
        | "&="
        | "|="
        | "^="
        | "<<="
        | ">>="
        ;

expr10 : expr10 '?' expr ':' expr9
        | expr9
        ;

expr9 : expr8
        | expr8 "||" expr8
        ;

expr8 : expr7
        | expr7 "&&" expr7
        ;

expr7 : expr6
        | expr6 '>' expr6
        | expr6 '<' expr6
        | expr6 ">=" expr6
        | expr6 "<=" expr6
        | expr6 "==" expr6
        | expr6 "!=" expr6
        ;

expr6 : expr5 
        | expr5 '|' expr5
        ;

expr5 : expr4 
        | expr4 '^' expr4
        ;
        
expr4 : expr3 
        | expr3 '&' expr3
        ;

expr3 : expr2 
        | expr2 ">>" expr2
        | expr2 "<<" expr2
        ;

expr2 : expr1 
        | expr1 '+' expr1
        | expr1 '-' expr1
        ;

expr1 : term
        | term '*' term
        | term '/' term
        | term '%' term
        ;

term : '(' type ')' term
        | unary
        ;

unary : "++" unary
        | "--" unary
        | '+' unary
        | '-' term
        | '!' term
        | '~' term
        | '*' term
        | '&' term
        | SIZEOF '(' type ')'
        | SIZEOF unary
        | postfix
        ;

postfix : primary { $$ = $1; }
        | postfix "++" { $$ = new SuffixOpNode("++", $1); }
        | postfix "--" { $$ = new SuffixOpNode("--", $1); }
        | postfix '[' expr ']' { $$ = new ArefNode($1, $3); }
        | postfix '.' name { $$ = new MemberNode($1, $3); }
        | postfix "->" name
        | postfix '(' ')'
        | postfix '(' args ')'
        ;

name : IDENTIFIER {  $$ = $1.image_; } 


args : expr { $$ = vector<ExprNode*> {$1}; }
        | args ',' expr { $$.push_back($3); }
        ;

primary : INTEGER       { $$ = integer_node(Location($1), $1.image_); }
        | CHARACTER     { $$ = new IntegerLiteralNode(Location($1), 
                              IntegerTypeRef::char_ref(), $1.image_[0]);
                        }
        | STRING        { $$ = new StringLiteralNode(Location($1),
                              new PointerTypeRef(IntegerTypeRef::char_ref()), $1.image_);
                        }
        | IDENTIFIER    { $$ = new VariableNode(Location($1), $1.image_);}
        | '(' expr ')'  { $$ = $2; } /* XXX: this rule will cause 4 conflicts-rr */
        ;

%%

IntegerLiteralNode* integer_node(const Location &loc, const string& image)
{
    long i = stol(image);
    if (image.size() >= 3 && image.substr(image.size()-2,2) == "UL")
        return new IntegerLiteralNode(loc, IntegerTypeRef::ulong_ref(), i);
    if (image.size() >= 2 && image.substr(image.size()-1,1) == "L")
        return new IntegerLiteralNode(loc, IntegerTypeRef::long_ref(), i);
    if (image.size() >= 2 && image.substr(image.size()-1,1) == "U")
        return new IntegerLiteralNode(loc, IntegerTypeRef::uint_ref(), i);
    return new IntegerLiteralNode(loc, IntegerTypeRef::int_ref(), i);
} 

void parser::Parser::error(const location_type& loc, const std::string& msg) 
{
    printf("%s at (line %d, column: %d)\n", msg.c_str(),
        loc.begin.line, loc.begin.column);
}