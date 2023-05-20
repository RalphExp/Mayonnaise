%{
#include "parser.hh"
int yy_get_char(const char*);
char* yy_get_string(const char*);

using namespace std;
%}

%option reentrant
%option interactive
%option noyywrap
%option nodefault
%option outfile="scanner.cc" header="scanner.hh"
%option yylineno
%x COMMENT

%%
void            { return yy::Parser::token::VOID; }
char            { return yy::Parser::token::CHAR; }
short           { return yy::Parser::token::SHORT; }
int             { return yy::Parser::token::INT; }
long            { return yy::Parser::token::LONG; }
struct          { return yy::Parser::token::STRUCT; }
union           { return yy::Parser::token::UNION; }
enum            { return yy::Parser::token::ENUM; }
extern          { return yy::Parser::token::EXTERN; }
static          { return yy::Parser::token::STATIC; }
const           { return yy::Parser::token::CONST; }
signed          { return yy::Parser::token::SIGNED; }
unsigned        { return yy::Parser::token::UNSIGNED; }
if              { return yy::Parser::token::IF; }
else            { return yy::Parser::token::ELSE; }
switch          { return yy::Parser::token::SWITCH; }
case            { return yy::Parser::token::CASE; }
default         { return yy::Parser::token::DEFAULT; }
while           { return yy::Parser::token::WHILE; }
do              { return yy::Parser::token::DO; }
for             { return yy::Parser::token::FOR; }
"return"        { return yy::Parser::token::RETURN; }
break           { return yy::Parser::token::BREAK; }
continue        { return yy::Parser::token::CONTINUE; }
goto            { return yy::Parser::token::GOTO; }
typedef         { return yy::Parser::token::TYPEDEF; }
import          { return yy::Parser::token::IMPORT; }

[ \t\n\r\f]+
[a-zA-Z_][a-zA-Z0-9_]* { yylval->emplace<string>(yytext); return yy::Parser::token::IDENTIFIER; }
[1-9][0-9]*U?L?|0[xX][0-9a-fA-F]+U?L?|0[0-7]*U?L? { yylval->emplace<string>(yytext);
                return yy::Parser::token::INTEGER; }

"/*"            { BEGIN(COMMENT); }
<COMMENT>"*/"   { BEGIN(INITIAL); }
<COMMENT>([^*]|\n)+|.
<COMMENT><<EOF>>    { printf("Unterminated comment\n"); return 0; }
"//".*

"\'"[^\"]*"\'" { /* will check the content later */
                 yylval->emplace<string>(yytext + 1, strlen(yytext)-2);
                 return yy::Parser::token::CHARACTER;
               }

"\""[^\"]*"\"" { /* will check the content later */
                 yylval->emplace<string>(yytext + 1, strlen(yytext)-2);
                 return yy::Parser::token::STRING; 
               }

.|\n           { return yytext[0]; }
%%

/* TODO: utf-8 */

int yy_get_char(const char *s, yyscan_t yyscanner)
{
    int buf[4] = {0};
    char c = yyinput(yyscanner);
    char r; 
    if (c == 0) {
        printf("end of file ");
        return 0;
    } 
    if (c == '\\') {
        c = yyinput(yyscanner);
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
                c = yyinput(yyscanner);
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

    c = yyinput(yyscanner);
    if (c != '\'') {
        printf("character fmt error");
        return 0;
    }
    return r;
}
