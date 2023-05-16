%{
#include <stdio.h>
#include <stdarg.h>

void yyerror(const char*, ...);
extern int yylineno;
%}

%union {
    int i_value;
    char c_value;
    char *s_value;
    void *v_value;
}

%token <i_value> VOID CHAR SHORT INT LONG 
%token <i_value> TYPEDEF STRUCT UNION ENUM 
%token <i_value> STATIC EXTERN 
%token <i_value> SIGNED UNSIGNED CONST
%token <i_value> IF ELSE SWITCH CASE DEFAULT WHILE DO FOR RETURN BREAK CONTINUE GOTO
%token <i_value> IMPORT SIZEOF 
%token <s_value> IDENTIFIER INTEGER CHARACTER STRING

%type <v_value> compilation_unit impdecls decls
%start compilation_unit

%%
compilation_unit : impdecls decls {}
            ;

impdecls    : {}
            ;

decls   : {}
        ;
%%

void yyerror(const char* str, ...)
{
    va_list va;
    va_start(va, str);
    fprintf(stderr, "%d: error: ", yylineno);
    vfprintf(stderr, str, va);
    fprintf(stderr, "\n");
    va_end(va);
}
