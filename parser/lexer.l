%{
#include "token.h"
#include "parser.hh"
#include <string>
#include <set>

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
%option outfile="lexer.cc" header="lexer.hh"
%option yylineno

%x COMMENT

%%
void        { Token tok(parser::Parser::token::VOID, "void"); YY_FINISH_TOKEN }
char        { Token tok(parser::Parser::token::CHAR, "char"); YY_FINISH_TOKEN }
short       { Token tok(parser::Parser::token::SHORT, "short"); YY_FINISH_TOKEN }
int         { Token tok(parser::Parser::token::INT, "int"); YY_FINISH_TOKEN }
long        { Token tok(parser::Parser::token::LONG, "long"); YY_FINISH_TOKEN }
struct      { Token tok(parser::Parser::token::STRUCT, "struct"); YY_FINISH_TOKEN }
union       { Token tok(parser::Parser::token::UNION, "union"); YY_FINISH_TOKEN }
enum        { Token tok(parser::Parser::token::ENUM, "enum"); YY_FINISH_TOKEN }
extern      { Token tok(parser::Parser::token::EXTERN, "extern"); YY_FINISH_TOKEN }
static      { Token tok(parser::Parser::token::STATIC, "static"); YY_FINISH_TOKEN }
const       { Token tok(parser::Parser::token::CONST, "const"); YY_FINISH_TOKEN }
signed      { Token tok(parser::Parser::token::SIGNED, "signed"); YY_FINISH_TOKEN }
unsigned    { Token tok(parser::Parser::token::UNSIGNED, "unsigned"); YY_FINISH_TOKEN }
if          { Token tok(parser::Parser::token::IF, "if"); YY_FINISH_TOKEN }
else        { Token tok(parser::Parser::token::ELSE, "else"); YY_FINISH_TOKEN }
switch      { Token tok(parser::Parser::token::SWITCH, "switch"); YY_FINISH_TOKEN }
case        { Token tok(parser::Parser::token::CASE, "case"); YY_FINISH_TOKEN }
default     { Token tok(parser::Parser::token::DEFAULT, "default"); YY_FINISH_TOKEN }
while       { Token tok(parser::Parser::token::WHILE, "while"); YY_FINISH_TOKEN }
do          { Token tok(parser::Parser::token::DO, "do"); YY_FINISH_TOKEN }
for         { Token tok(parser::Parser::token::FOR, "for"); YY_FINISH_TOKEN }
"return"    { Token tok(parser::Parser::token::RETURN, "return"); YY_FINISH_TOKEN }
break       { Token tok(parser::Parser::token::BREAK, "break"); YY_FINISH_TOKEN }
continue    { Token tok(parser::Parser::token::CONTINUE, "continue"); YY_FINISH_TOKEN }
goto        { Token tok(parser::Parser::token::GOTO, "goto"); YY_FINISH_TOKEN }
typedef     { Token tok(parser::Parser::token::TYPEDEF, "typedef"); YY_FINISH_TOKEN }
import      { Token tok(parser::Parser::token::IMPORT, "import"); YY_FINISH_TOKEN }

[ \t\n\r\f]+    { /* skip */ }
[a-zA-Z_][a-zA-Z0-9_]* { 
            set<string>* typenames = (set<string>*)yyget_extra(yyscanner);
            string s = yytext;
            if (typenames->count(s) != 0) {
                Token tok(parser::Parser::token::IDENTIFIER, s); 
                YY_FINISH_TOKEN 
            } else {
                Token tok(parser::Parser::token::TYPENAME, s); 
                YY_FINISH_TOKEN     
            }
        }
[1-9][0-9]*U?L?|0[xX][0-9a-fA-F]+U?L?|0[0-7]*U?L? { 
            Token tok(parser::Parser::token::INTEGER, yytext); YY_FINISH_TOKEN }

"/*"            { BEGIN(COMMENT); }
<COMMENT>"*/"   { BEGIN(INITIAL); }
<COMMENT>([^*]|\n)+|.
<COMMENT><<EOF>>    { printf("Unterminated comment\n"); return 0; }
"//".*

"\'"[^\"]*"\'" { /* will check the content later */
                 try {
                   string s = string_value(string(yytext+1, strlen(yytext)-2));
                   char c = character_code(s);
                   Token tok(parser::Parser::token::CHARACTER, to_string(c)); 
                   YY_FINISH_TOKEN
                 } catch (string &e) {
                    printf("%s at line %d, col %d\n", e.c_str(), loc->begin.line, loc->begin.column);
                    return 0;
                 }
               }

"\""[^\"]*"\"" { /* will check the content later */
                 try {
                   string s = string_value(string(yytext+1, strlen(yytext)-2));
                   Token tok(parser::Parser::token::STRING, s); 
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
