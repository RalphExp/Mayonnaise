%{
#include "may.h"

int yy_get_char(const char*);
char* yy_get_string(const char*);
%}

%option yylineno
%x COMMENT

%%
void            { return VOID; }
char            { return CHAR; }
short           { return SHORT; }
int             { return INT; }
long            { return LONG; }
struct          { return STRUCT; }
union           { return UNION; }
enum            { return ENUM; }
extern          { return EXTERN; }
static          { return STATIC; }
const           { return CONST; }
signed          { return SIGNED; }
unsigned        { return UNSIGNED; }
if              { return IF; }
else            { return ELSE; }
switch          { return SWITCH; }
case            { return CASE; }
default         { return DEFAULT; }
while           { return WHILE; }
do              { return DO; }
for             { return FOR; }
"return"        { return RETURN; }
break           { return BREAK; }
continue        { return CONTINUE; }
goto            { return GOTO; }
typedef         { return TYPEDEF; }
import          { return IMPORT; }

[ \t\n\r\f]+
[a-zA-Z_][a-zA-Z0-9_]* { yylval.s_value = strdup(yytext); return IDENTIFIER; }
[1-9][0-9]*U?L?|0[xX][0-9a-fA-F]+U?L?|0[0-7]*U?L? { yylval.s_value = strdup(yytext); return INTEGER; } 

"/*"            { BEGIN(COMMENT); }
<COMMENT>"*/"    { BEGIN(INITIAL); }
<COMMENT>([^*]|\n)+|.
<COMMENT><<EOF>>    { printf("%s: %d: Unterminated comment\n", "current file", yylineno); return 0; }
"//".*

"\'"[^\"]*"\'" { /* will check the content later */
				 yylval.s_value = strdup(yytext + 1); 
				 yylval.s_value[strlen(yytext)-2] = 0; 
				 return CHARACTER; 
				}

"\""[^\"]*"\"" { /* will check the content later */
				 yylval.s_value = strdup(yytext + 1); 
				 yylval.s_value[strlen(yytext)-2] = 0; 
				 return STRING; 
				}

%%

/* TODO: utf-8 */
int yy_get_char(const char *s)
{
	int buf[4] = {0};
	char c = input();
	char r; /* character to return */
	if (c == 0) {
		printf("end of file ");
		return 0;
	} 
	if (c == '\\') {
		c = input();
		if (c == 0) {
			printf("end of file ");
			return 0;
		}
		switch (c) {
		case 'r': r = '\r'; break;
		case 'n': r = '\n'; break;
		case 'b': r = '\b'; break;
		case 't': r = '\t'; break;
		case '0'...'7': {
			int i = 0;
			do {
				buf[i] = c - '0';
				++i;
				if (i == 3) 
				    break;
				c = input();
				if (c < '0' || c > '7') {
					printf("character format error ");
					return 0;
				}
			} while (1);
			r = (buf[0] << 16) + (buf[1] << 8) + buf[2];
		}
		default:
			r = c;
		}
	} else {
		r = c;
	}

	c = input();
	if (c != '\'') {
		printf("character fmt error");
		return 0;
	}
	return r;
}

int yywrap() {
    return 0;
}
