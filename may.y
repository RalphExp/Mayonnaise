%{
#include <stdio.h>
#include <stdarg.h>

extern "C" {
    int yylex();
    void yyerror(const char*);
}


%}

%union {
    int i_value;
}

%token <i_value> VOID CHAR SHORT INT LONG 
%token <i_value> TYPEDEF STRUCT UNION ENUM 
%token <i_value> STATIC EXTERN 
%token <i_value> SIGNED UNSIGNED
%token <i_value> IF ELSE SWITCH CASE DEFAULT WHILE DO FOR RETURN BREAK CONTINUE GOTO
%token <i_value> IMPORT SIZEOF

%start compile_unit

%%
compile_unit: {}

%%

int yylex()
{
    return 0;
}

void yyerror(const char* str)
{
    fprintf(stderr, str);
}
