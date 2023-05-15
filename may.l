%{
#include "may.h"
%}

%%
void        { return VOID; }
char        { return CHAR; }
short       { return SHORT; }
int         { return INT; }
long        { return LONG; }
struct      { return STRUCT; }
union       { return UNION; }
enum        { return ENUM; }
extern      { return EXTERN; }
static      { return STATIC; }
const       { return CONST; }
signed      { return SIGNED; }
unsigned    { return UNSIGNED; }
if          { return IF; }
else        { return ELSE; }
switch      { return SWITCH; }
case        { return CASE; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval.s_value = strdup(yytext); return IDENTIFIER; }
%%

int yywrap() {
	return 0;
}
