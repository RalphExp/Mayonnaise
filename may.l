%{
#include "may.h"
%}

%option yylineno nodefault
%x COMMENT
%x IN_CHARACTER
%x IN_STRING
%x CHARACTER_TERM

%%
void        	{ return VOID; }
char        	{ return CHAR; }
short       	{ return SHORT; }
int         	{ return INT; }
long        	{ return LONG; }
struct      	{ return STRUCT; }
union       	{ return UNION; }
enum        	{ return ENUM; }
extern      	{ return EXTERN; }
static      	{ return STATIC; }
const       	{ return CONST; }
signed      	{ return SIGNED; }
unsigned    	{ return UNSIGNED; }
if          	{ return IF; }
else        	{ return ELSE; }
switch      	{ return SWITCH; }
case        	{ return CASE; }
default         { return DEFAULT; }
while           { return WHILE; }
do              { return DO; }
for				{ return FOR; }
"return"          { return RETURN; }
break           { return BREAK; }
continue        { return CONTINUE; }
goto            { return GOTO; }
typedef         { return TYPEDEF; }
import          { return IMPORT; }

[ \t\n\r\f]+
[a-zA-Z_][a-zA-Z0-9_]* { yylval.s_value = strdup(yytext); return IDENTIFIER; }
[1-9][0-9]*U?L?|0[xX][0-9a-fA-F]+U?L?|0[0-7]*U?L? { yylval.s_value = strdup(yytext); return INTEGER; } 

"/*"			{ BEGIN(COMMENT); }
<COMMENT>"*/"	{ BEGIN(INITIAL); }
<COMMENT>([^*]|\n)+|.
<COMMENT><<EOF>>	{ printf("%s: %d: Unterminated comment\n", "current file", yylineno); return 0; }
"//".*

"'"				{ BEGIN(IN_CHARACTER); }
<IN_CHARACTER>[^\'\\\n\r] 	{ BEGIN(CHARACTER_TERM); }
<IN_CHARACTER>"\\"[0-7]{3} 	{ BEGIN(CHARACTER_TERM); }
<IN_CHARACTER>"\\". 		{ BEGIN(CHARACTER_TERM); }
<CHARACTER_TERM>"'" { yylval.s_value = strdup(yytext); return CHARACTER; BEGIN(INITIAL); }

"\""			{ BEGIN(IN_STRING); }
<IN_STRING>[^\"\\\n\r]+		
<IN_STRING>"\""	{ BEGIN(INITIAL); }

%%

int yywrap() {
	return 0;
}
