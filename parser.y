%{
#include <stdio.h>
#include <stdarg.h>
#include "scanner.h"

using namespace std;
%}

// version of bison
%require "3.8.1"

// use c++ language
%language "c++"

// parser's class name
%define api.parser.class {Parser}

// use variant instead of the old union format
%define api.value.type variant

// parser constructor's parameter, it is much easier 
// to use yyscan_t than c++ class
%param {yyscan_t scanner}

// tracking location
// %locations

%code provides
{
    #define YY_DECL \
        int yylex(yy::Parser::semantic_type *yylval, yyscan_t yyscanner)
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
%type <int> define_func_or_var
%type <int> import_stmt
%type <int> storage type 
%type <int> expr
%start compilation_unit

%%
compilation_unit : import_stmts {} top_defs <<EOF>> {}
                ;

import_stmts : %empty /* new ... */
        | import_stmt { /* new ...*/ }
        | import_stmts import_stmt {}
        ;

import_stmt : IMPORT {}

top_defs : storage type define_func_or_var { /* define function or variables*/ }
        | def_const {}
        | def_struct {}
        | def_union {}
        | def_typedef {}
        | top_defs storage type define_func_or_var {}
        | top_defs def_const {}
        | top_defs def_struct {}
        | top_defs def_union {}
        | top_defs def_typedef {}
        ;

define_func_or_var: {}

def_const : CONST type name '=' expr ';'  { /* define const variable */ }

def_struct : STRUCT name member_list ';'  { /* define struct */ }

def_union : UNION name member_list ';'  { /* define union */ }

def_typedef : TYPEDEF typeref IDENTIFIER ';'  { /* define typedef */ }

storage : {}

type : {}

typeref : {}

name : {}

member_list : {}

expr : {}
%%

void yy::Parser::error(const std::string& msg) {
    cerr << msg << endl;
}
