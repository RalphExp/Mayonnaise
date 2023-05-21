%{
#include "token.h"
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
void        { Token tok(yy::Parser::token::VOID, "void"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
char        { Token tok(yy::Parser::token::CHAR, "char"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
short       { Token tok(yy::Parser::token::SHORT, "short"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
int         { Token tok(yy::Parser::token::INT, "int"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
long        { Token tok(yy::Parser::token::LONG, "long"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
struct      { Token tok(yy::Parser::token::STRUCT, "struct"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
union       { Token tok(yy::Parser::token::UNION, "union"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
enum        { Token tok(yy::Parser::token::ENUM, "enum"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
extern      { Token tok(yy::Parser::token::EXTERN, "extern"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
static      { Token tok(yy::Parser::token::STATIC, "static"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
const       { Token tok(yy::Parser::token::CONST, "const"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
signed      { Token tok(yy::Parser::token::SIGNED, "signed"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
unsigned    { Token tok(yy::Parser::token::UNSIGNED, "unsigned"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
if          { Token tok(yy::Parser::token::IF, "if"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
else        { Token tok(yy::Parser::token::ELSE, "else"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
switch      { Token tok(yy::Parser::token::SWITCH, "switch"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
case        { Token tok(yy::Parser::token::CASE, "case"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
default     { Token tok(yy::Parser::token::DEFAULT, "default"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
while       { Token tok(yy::Parser::token::WHILE, "while"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
do          { Token tok(yy::Parser::token::DO, "do"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
for         { Token tok(yy::Parser::token::FOR, "for"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
"return"    { Token tok(yy::Parser::token::RETURN, "return"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
break       { Token tok(yy::Parser::token::BREAK, "break"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
continue    { Token tok(yy::Parser::token::CONTINUE, "continue"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
goto        { Token tok(yy::Parser::token::GOTO, "goto"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
typedef     { Token tok(yy::Parser::token::TYPEDEF, "typedef"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }
import      { Token tok(yy::Parser::token::IMPORT, "import"); YY_SET_LOCATION; yylval->emplace<Token>(tok); return tok.kind; }

[ \t\n\r\f]+    { /* skip */ }
[a-zA-Z_][a-zA-Z0-9_]* { 
            Token tok(yy::Parser::token::IDENTIFIER, yytext); 
            YY_SET_LOCATION; 
            yylval->emplace<Token>(tok); 
            return tok.kind; 
        }
[1-9][0-9]*U?L?|0[xX][0-9a-fA-F]+U?L?|0[0-7]*U?L? { 
            Token tok(yy::Parser::token::INTEGER, yytext);
            YY_SET_LOCATION; 
            yylval->emplace<Token>(tok); 
            return tok.kind; 
        }

"/*"            { BEGIN(COMMENT); }
<COMMENT>"*/"   { BEGIN(INITIAL); }
<COMMENT>([^*]|\n)+|.
<COMMENT><<EOF>>    { printf("Unterminated comment\n"); return 0; }
"//".*

"\'"[^\"]*"\'" { /* will check the content later */
                 Token tok(yy::Parser::token::CHARACTER, string(yytext + 1, strlen(yytext)-2));
                 YY_SET_LOCATION;
                 yylval->emplace<Token>(tok); 
                 return tok.kind;
               }

"\""[^\"]*"\"" { /* will check the content later */
                 Token tok(yy::Parser::token::CHARACTER, string(yytext + 1, strlen(yytext)-2));
                 YY_SET_LOCATION;
                 yylval->emplace<Token>(tok); 
                 return tok.kind;
               }

.|\n           { Token tok((int)yytext[0], yytext); 
                 YY_SET_LOCATION;
                 yylval->emplace<Token>(tok); 
                 return tok.kind; 
               }
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
