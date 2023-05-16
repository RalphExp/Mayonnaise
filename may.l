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
[ \t\n\r\f]+ {}
[a-zA-Z_][a-zA-Z0-9_]* { yylval.s_value = strdup(yytext); return IDENTIFIER; }
[1-9][0-9]*U?L?|0[xX][0-9a-fA-F]+U?L?|0[0-7]*U?L? { yylval.s_value = strdup(yytext); return INTEGER; } 
%%

int yywrap() {
	return 0;
}
