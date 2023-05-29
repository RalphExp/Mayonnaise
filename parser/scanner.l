%{
#include "token.h"
#include "parser.hh"
#include <string>

using namespace std;
char unescape_char(char c);
char character_code(const string& image);
string string_value(const string& image);
long integer_value(const string& image);

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
                 try {
                   string s = string_value(string(yytext+1, strlen(yytext)-2));
                   char c = character_code(s);
                   Token tok(yy::Parser::token::CHARACTER, to_string(c)); 
                   YY_FINISH_TOKEN
                 } catch (string &e) {
                    printf("%s at line %d, col %d\n", e.c_str(), loc->begin.line, loc->begin.column);
                    return 0;
                 }
               }

"\""[^\"]*"\"" { /* will check the content later */
                 try {
                   string s = string_value(string(yytext+1, strlen(yytext)-2));
                   Token tok(yy::Parser::token::STRING, s); 
                   YY_FINISH_TOKEN
                 } catch (string &e) {
                    printf("%s at line %d, col %d\n", e.c_str(), loc->begin.line, loc->begin.column);
                    return 0;
                 }
               }

.|\n           { Token tok((int)yytext[0], yytext); YY_FINISH_TOKEN }
%%

char unescape_char(char c) {
    switch (c) {
    case '0': return 0;
    case '"': return '"';
    case '\'': return '\'';
    case 'a': return 7;
    case 'b': return 8;
    case 'e': return 27;
    case 'f': return '\f';
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    case 'v': return 11;
    default:
        throw string("unknown escape character: '") + c + "'"; 
    }
}

char character_code(const string& image) {
    auto s = string_value(image);
    if (s.size() != 1) {
        throw string("character size must be 1");
    }
    return s[0];
}

long integer_value(const string& image) {
    size_t idx;
    int base = 10;
    const char* s;
    if (image.size() >= 2 && image[0] == '0') {
        if (image[1] == 'X' || image[1] == 'x') {
            base = 16;
            s = &image[2];
        } else {
            base = 8;
            s = &image[1];
        }
    }
    long value = strtol(s, NULL, base);
    return value;
}

string string_value(const string& image) {
    stringstream ss;
    size_t idx = 0;
    while (idx < image.size()) {
        if (image[idx] != '\\') {
            ss << image[idx];
            idx += 1;
        } else {
            if ((image.size() > idx + 3) && isdigit(image[idx+1]) &&
                        isdigit(image[idx+2]) && isdigit(image[idx+3])) {
                ss << integer_value(image.substr(idx, 4));
                idx += 4;
            } else {
                if (idx == image.size() - 1) {
                    throw string("invalid escape character");
                }
                ss << unescape_char(image[idx+1]);
                idx += 2;
            }
        }
    }
    return ss.str();
}
