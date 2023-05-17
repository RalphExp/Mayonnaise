%{
#include <stdio.h>
#include <stdarg.h>
#include <FlexLexer.h>
%}

// new version of bison
%require "3.8.1"

// using c++ language
%language "c++"

// parser's class name
%define api.parser.class {Parser}

// use variant instead of union
%define api.value.type variant

// parser constructor's parameter
%parse-param {Lexer* lexer}

// tracking location
// %locations

%code requires
{
    // forward declaration
    namespace yy {
        class Lexer;
    }
}

%code
{
    #include "lexer.h"
    #define yylex(x) lexer->lex(x)
}


// %locations

%token <int> VOID CHAR SHORT INT LONG 
%token <int> TYPEDEF STRUCT UNION ENUM 
%token <int> STATIC EXTERN 
%token <int> SIGNED UNSIGNED CONST
%token <int> IF ELSE SWITCH CASE DEFAULT WHILE DO FOR RETURN BREAK CONTINUE GOTO
%token <int> IMPORT SIZEOF 
%token <std::string> IDENTIFIER INTEGER CHARACTER STRING

%type <int> compilation_unit impdecls decls
%start compilation_unit

%%
compilation_unit : impdecls decls {}
            ;

impdecls    : {}
            ;

decls   : {}
        ;
%%

