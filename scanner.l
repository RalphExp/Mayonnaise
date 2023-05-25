%{
#include "token.h"
#include "parser.hh"
#include <string>

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
void        { Token tok(yy::Parser::token::VOID, "void"); YY_FINISH_TOKEN }
char        { Token tok(yy::Parser::token::CHAR, "char"); YY_FINISH_TOKEN }
short       { Token tok(yy::Parser::token::SHORT, "short"); YY_FINISH_TOKEN }
int         { Token tok(yy::Parser::token::INT, "int"); YY_FINISH_TOKEN }
long        { Token tok(yy::Parser::token::LONG, "long"); YY_FINISH_TOKEN }
struct      { Token tok(yy::Parser::token::STRUCT, "struct"); YY_FINISH_TOKEN }
union       { Token tok(yy::Parser::token::UNION, "union"); YY_FINISH_TOKEN }
enum        { Token tok(yy::Parser::token::ENUM, "enum"); YY_FINISH_TOKEN }
extern      { Token tok(yy::Parser::token::EXTERN, "extern"); YY_FINISH_TOKEN }
static      { Token tok(yy::Parser::token::STATIC, "static"); YY_FINISH_TOKEN }
const       { Token tok(yy::Parser::token::CONST, "const"); YY_FINISH_TOKEN }
signed      { Token tok(yy::Parser::token::SIGNED, "signed"); YY_FINISH_TOKEN }
unsigned    { Token tok(yy::Parser::token::UNSIGNED, "unsigned"); YY_FINISH_TOKEN }
if          { Token tok(yy::Parser::token::IF, "if"); YY_FINISH_TOKEN }
else        { Token tok(yy::Parser::token::ELSE, "else"); YY_FINISH_TOKEN }
switch      { Token tok(yy::Parser::token::SWITCH, "switch"); YY_FINISH_TOKEN }
case        { Token tok(yy::Parser::token::CASE, "case"); YY_FINISH_TOKEN }
default     { Token tok(yy::Parser::token::DEFAULT, "default"); YY_FINISH_TOKEN }
while       { Token tok(yy::Parser::token::WHILE, "while"); YY_FINISH_TOKEN }
do          { Token tok(yy::Parser::token::DO, "do"); YY_FINISH_TOKEN }
for         { Token tok(yy::Parser::token::FOR, "for"); YY_FINISH_TOKEN }
"return"    { Token tok(yy::Parser::token::RETURN, "return"); YY_FINISH_TOKEN }
break       { Token tok(yy::Parser::token::BREAK, "break"); YY_FINISH_TOKEN }
continue    { Token tok(yy::Parser::token::CONTINUE, "continue"); YY_FINISH_TOKEN }
goto        { Token tok(yy::Parser::token::GOTO, "goto"); YY_FINISH_TOKEN }
typedef     { Token tok(yy::Parser::token::TYPEDEF, "typedef"); YY_FINISH_TOKEN }
import      { Token tok(yy::Parser::token::IMPORT, "import"); YY_FINISH_TOKEN }

[ \t\n\r\f]+    { /* skip */ }
[a-zA-Z_][a-zA-Z0-9_]* { Token tok(yy::Parser::token::IDENTIFIER, yytext); YY_FINISH_TOKEN }
[1-9][0-9]*U?L?|0[xX][0-9a-fA-F]+U?L?|0[0-7]*U?L? { 
            Token tok(yy::Parser::token::INTEGER, yytext); YY_FINISH_TOKEN }

"/*"            { BEGIN(COMMENT); }
<COMMENT>"*/"   { BEGIN(INITIAL); }
<COMMENT>([^*]|\n)+|.
<COMMENT><<EOF>>    { printf("Unterminated comment\n"); return 0; }
"//".*

"\'"[^\"]*"\'" { /* will check the content later */
                 Token tok(yy::Parser::token::CHARACTER, string(yytext + 1, strlen(yytext)-2)); 
                 YY_FINISH_TOKEN 
               }

"\""[^\"]*"\"" { /* will check the content later */
                 Token tok(yy::Parser::token::STRING, string(yytext + 1, strlen(yytext)-2)); YY_FINISH_TOKEN
               }

.|\n           { Token tok((int)yytext[0], yytext); YY_FINISH_TOKEN }
%%
