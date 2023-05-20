%{
#include <stdio.h>
#include <stdarg.h>
#include "scanner.hh"

using namespace std;
%}

// version of bison
%require "3.8"

// use c++ language
%language "c++"

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

%code provides
{
    #define YY_USER_ACTION \
        loc->begin.line = loc->end.line = yylineno; \
        loc->begin.column = yycolumn; \
        loc->end.column = yycolumn + yyleng-1; \
        yycolumn += yyleng;
}

%code provides
{
    #define YY_DECL \
        int yylex(yy::Parser::semantic_type *yylval, \
            yy::Parser::location_type *loc, yyscan_t yyscanner)
    YY_DECL;
}

// %locations

%token <int> VOID CHAR SHORT INT LONG 
%token <int> TYPEDEF STRUCT UNION ENUM 
%token <int> STATIC EXTERN 
%token <int> SIGNED UNSIGNED CONST
%token <int> IF ELSE SWITCH CASE DEFAULT WHILE DO FOR RETURN BREAK CONTINUE GOTO
%token <int> IMPORT SIZEOF 
%token <std::string> IDENTIFIER INTEGER CHARACTER STRING

%type <int> compilation_unit import_stmts top_defs
%type <int> def_func def_vars def_const def_union def_typedef
%type <int> import_stmt
%type <int> storage type 
%type <int> expr
%start compilation_unit

%%
compilation_unit : top_defs {}
        | import_stmts top_defs {}
        ;

import_stmts : import_stmt {}
        | import_stmts import_stmt {}
        ;

import_stmt : IMPORT ';' {}


top_defs : def_func { printf("find function\n"); }
        | def_vars {}
        | def_const {}
        | def_struct {}
        | def_union {}
        | def_typedef {}
        | top_defs def_func {}
        | top_defs def_vars {}
        | top_defs def_const {}
        | top_defs def_struct {}
        | top_defs def_union {}
        | top_defs def_typedef {}
        ;

/* XXX: typeref and type is different? */
def_func : storage type name '(' params ')' block {}
        ;

def_var_list : def_vars {}
        | def_var_list ',' def_vars {}
        ;

def_vars : storage type name '=' expr ';' {}
        | storage type name ';' {}
        | def_vars ',' storage type name '=' expr ';' {}
        | def_vars ',' storage type name ';' {}
        ;

def_const : CONST type name '=' expr ';'  {}

 /* TODO: need to check null struct */
def_struct : STRUCT name member_list ';'  {}
    
/* TODO: need to check null union */
def_union : UNION name member_list ';'  {} 


def_typedef : TYPEDEF typeref IDENTIFIER ';'  {}

param_typerefs : VOID {}
        | fixed_param_typerefs
        | fixed_param_typerefs ',' "..." {}

params : VOID {}
        | fixed_params
        | fixed_params ',' "..." {} 
        ;

fixed_param_typerefs : typeref {}
        | fixed_param_typerefs ',' typeref {}
        ;

fixed_params : param {}
        | fixed_params ',' param {}
        ;

param : type name {}
        ;

block : '{' '}' {}
        | '{' stmts '}' {}
        | '{' def_var_list stmts '}' {}
        ;

storage : %empty {}
        | STATIC {}
        ;

type : typeref { return 0; }
        ;

typeref : typeref_base 
        | typeref_base '[' ']' {}
        | typeref_base '[' INTEGER ']' {}
        | typeref_base '*' {}
        | typeref_base '(' param_typerefs ')' {}
        | typeref typeref_base '[' ']' {}
        | typeref typeref_base '[' INTEGER ']' {}
        | typeref typeref_base '*' {}
        | typeref typeref_base '(' param_typerefs ')' {}
        ;

stmts : stmt {}
        | stmts stmt {}
        ;

stmt : ';' {}
        | label_stmt {}
        | expr ';' {}
        | block {}
        | if_stmt {}
        | while_stmt {}
        | dowhile_stmt {}
        | for_stmt {}
        | switch_stmt {}
        | break_stmt {}
        | continue_stmt {}
        | goto_stmt {}
        | return_stmt {}
        ;

label_stmt : IDENTIFIER ':' stmt {}
        ;

if_stmt : IF '(' expr ')' stmt ELSE stmt {}
        | IF '(' expr ')' stmt {}
        ;

while_stmt : WHILE '(' expr ')' stmt {}

dowhile_stmt : DO stmt WHILE '(' expr ')' ';' {}

for_stmt : FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' stmt {}

goto_stmt : GOTO IDENTIFIER ';'

switch_stmt : SWITCH '(' expr ')' '{' case_clauses '}' {}

case_clauses : default_clause
        | case_clause
        | case_clauses default_clause
        | case_clauses case_clause
        ;

case_clause : cases {}
        | cases case_body {}
        ;

cases : CASE primary ':' {}

case_body : stmt {}

default_clause : DEFAULT ':' {}
        | DEFAULT ':' case_body {}
        ;

return_stmt : RETURN ';' {}
        | RETURN expr ';' {}
        ;

continue_stmt : CONTINUE ';' {}

break_stmt : BREAK ';'

member_list : '{' '}' {}
        | '{' slots '}' 
        ;

slots : type name ';'
        | slots type name ';'
        ;

opt_expr : %empty {}
        | expr {}
        ;

typeref_base : VOID {}
        | CHAR {}
        | SHORT {}
        | INT {}
        | LONG {}
        | UNSIGNED CHAR {}
        | UNSIGNED SHORT {}
        | UNSIGNED INT {}
        | UNSIGNED LONG {}
        | STRUCT IDENTIFIER {}
        | UNION IDENTIFIER {}
        | IDENTIFIER
        ;

expr : expr '=' term {}
    | expr assign_op term {}
    | expr10 {}
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

expr10 : expr10 '?' expr ':' expr9 {}
        | expr9 {}
        ;

expr9 : expr8 {}
        | expr8 "||" expr8 {}
        ;

expr8 : expr7 {}
        | expr7 "&&" expr7 {}
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

term : '(' type ')' term {}
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

postfix : primary 
        | postfix "++"
        | postfix "--"
        | postfix '[' expr ']'
        | postfix '.' name
        | postfix "->" name
        | postfix '(' ')'
        | postfix '(' args ')'
        ;

name : IDENTIFIER {}

args : expr
        | args ',' expr
        ;

primary : INTEGER 
        | CHARACTER
        | STRING
        | IDENTIFIER
        | '(' expr ')'
        ;
%%

void yy::Parser::error(const location_type& loc, const std::string& msg) {
    printf("%s at (line %d, column: %d)\n", msg.c_str(),
        loc.begin.line, loc.begin.column);
}
