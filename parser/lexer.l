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
%x STR
%x CH

%%
void        { Token tok(parser::Parser::token::VOID, "void"); YY_SET_LOCATION }
char        { Token tok(parser::Parser::token::CHAR, "char"); YY_SET_LOCATION }
short       { Token tok(parser::Parser::token::SHORT, "short"); YY_SET_LOCATION }
int         { Token tok(parser::Parser::token::INT, "int"); YY_SET_LOCATION }
long        { Token tok(parser::Parser::token::LONG, "long"); YY_SET_LOCATION }
struct      { Token tok(parser::Parser::token::STRUCT, "struct"); YY_SET_LOCATION }
union       { Token tok(parser::Parser::token::UNION, "union"); YY_SET_LOCATION }
enum        { Token tok(parser::Parser::token::ENUM, "enum"); YY_SET_LOCATION }
extern      { Token tok(parser::Parser::token::EXTERN, "extern"); YY_SET_LOCATION }
static      { Token tok(parser::Parser::token::STATIC, "static"); YY_SET_LOCATION }
const       { Token tok(parser::Parser::token::CONST, "const"); YY_SET_LOCATION }
sizeof      { Token tok(parser::Parser::token::SIZEOF, "sizeof"); YY_SET_LOCATION }
signed      { Token tok(parser::Parser::token::SIGNED, "signed"); YY_SET_LOCATION }
unsigned    { Token tok(parser::Parser::token::UNSIGNED, "unsigned"); YY_SET_LOCATION }
if          { Token tok(parser::Parser::token::IF, "if"); YY_SET_LOCATION }
else        { Token tok(parser::Parser::token::ELSE, "else"); YY_SET_LOCATION }
switch      { Token tok(parser::Parser::token::SWITCH, "switch"); YY_SET_LOCATION }
case        { Token tok(parser::Parser::token::CASE, "case"); YY_SET_LOCATION }
default     { Token tok(parser::Parser::token::DEFAULT, "default"); YY_SET_LOCATION }
while       { Token tok(parser::Parser::token::WHILE, "while"); YY_SET_LOCATION }
do          { Token tok(parser::Parser::token::DO, "do"); YY_SET_LOCATION }
for         { Token tok(parser::Parser::token::FOR, "for"); YY_SET_LOCATION }
return      { Token tok(parser::Parser::token::RETURN, "return"); YY_SET_LOCATION }
break       { Token tok(parser::Parser::token::BREAK, "break"); YY_SET_LOCATION }
continue    { Token tok(parser::Parser::token::CONTINUE, "continue"); YY_SET_LOCATION }
goto        { Token tok(parser::Parser::token::GOTO, "goto"); YY_SET_LOCATION }
typedef     { Token tok(parser::Parser::token::TYPEDEF, "typedef"); YY_SET_LOCATION }
import      { Token tok(parser::Parser::token::IMPORT, "import"); YY_SET_LOCATION }

"++"        { Token tok(parser::Parser::token::PLUS_PLUS, "++"); YY_SET_LOCATION }
"--"        { Token tok(parser::Parser::token::MINUS_MINUS, "--"); YY_SET_LOCATION }
"&&"        { Token tok(parser::Parser::token::AND_AND, "&&"); YY_SET_LOCATION }
"||"        { Token tok(parser::Parser::token::OR_OR, "||"); YY_SET_LOCATION }
"<<"        { Token tok(parser::Parser::token::LSHIFT, "<<"); YY_SET_LOCATION }
">>"        { Token tok(parser::Parser::token::RSHIFT, ">>"); YY_SET_LOCATION }
"->"        { Token tok(parser::Parser::token::POINT_TO, "->"); YY_SET_LOCATION }
"=="        { Token tok(parser::Parser::token::EQ, "=="); YY_SET_LOCATION }
"!="        { Token tok(parser::Parser::token::NE, "!="); YY_SET_LOCATION }
"<="        { Token tok(parser::Parser::token::LE, "!="); YY_SET_LOCATION }
">="        { Token tok(parser::Parser::token::GE, "!="); YY_SET_LOCATION }
"+="        { Token tok(parser::Parser::token::PLUS_ASSIGN, "+="); YY_SET_LOCATION }
"-="        { Token tok(parser::Parser::token::MINUS_ASSIGN, "-="); YY_SET_LOCATION }
"*="        { Token tok(parser::Parser::token::MULTIPLY_ASSIGN, "&="); YY_SET_LOCATION }
"/="        { Token tok(parser::Parser::token::DIVIDE_ASSIGN, "|="); YY_SET_LOCATION }
"%="        { Token tok(parser::Parser::token::MOD_ASSIGN, "&="); YY_SET_LOCATION }
"^="        { Token tok(parser::Parser::token::XOR_ASSIGN, "|="); YY_SET_LOCATION }
"&="        { Token tok(parser::Parser::token::AND_ASSIGN, "&="); YY_SET_LOCATION }
"|="        { Token tok(parser::Parser::token::OR_ASSIGN, "|="); YY_SET_LOCATION }
"<<="       { Token tok(parser::Parser::token::LSHIFT_ASSIGN, "<<="); YY_SET_LOCATION }
">>="       { Token tok(parser::Parser::token::RSHIFT_ASSIGN, ">>="); YY_SET_LOCATION }
"..."       { Token tok(parser::Parser::token::ELLIPSIS, "..."); YY_SET_LOCATION }

[ \t\n\r\f]+    { /* skip */ }
[a-zA-Z_][a-zA-Z0-9_]* { 
            Option* option = (Option*)yyget_extra(yyscanner);
            string s = yytext;
            if (option->typename_.count(s) == 0) {
                Token tok(parser::Parser::token::IDENTIFIER, s); 
                YY_SET_LOCATION 
            } else {
                Token tok(parser::Parser::token::TYPENAME, s); 
                YY_SET_LOCATION     
            }
        }
[1-9][0-9]*U?L?|0[xX][0-9a-fA-F]+U?L?|0[0-7]*U?L? { 
            Token tok(parser::Parser::token::INTEGER, yytext); YY_SET_LOCATION }

"/*"            { BEGIN(COMMENT); }
<COMMENT>"*/"   { BEGIN(INITIAL); }
<COMMENT>([^*]|\n)+|.
<COMMENT><<EOF>>    { printf("Unterminated comment\n"); return 0; }
"//".*

"\'"        { BEGIN(CH); }
<CH>([^"\\]|\\(.|\n))*\' {
                BEGIN(INITIAL);
                try {
                    string s = string_value(string(yytext, strlen(yytext)-1));
                    Token tok(parser::Parser::token::CHARACTER, s);
                    YY_SET_LOCATION
                } catch (string &e) {
                    printf("%s at line %d, col %d\n", e.c_str(), loc->begin.line, loc->begin.column);
                    return parser::Parser::token::ERROR;
                }
            }
<CH><<EOF>> { printf("unterminated character\n");
               return parser::Parser::token::ERROR;
            }

"\""        { BEGIN(STR); }
<STR>([^\"\\]|\\(.|\n))*\" {
                BEGIN(INITIAL);
                try {
                    string s = string_value(string(yytext, strlen(yytext)-1));
                    Token tok(parser::Parser::token::STRING, s);
                    YY_SET_LOCATION
                } catch (string &e) {
                    printf("%s at line %d, col %d\n", e.c_str(), loc->begin.line, loc->begin.column);
                    return parser::Parser::token::ERROR;
                }
            }
<STR><<EOF>>  { printf("unterminated string\n");
                return parser::Parser::token::ERROR;
            }

.|\n        { Token tok((int)yytext[0], yytext); YY_SET_LOCATION }
%%

char unescape_char(char c) {
    switch (c) {
    case '0':  return 0;
    case '"':  return '"';
    case '\'': return '\'';
    case 'a':  return 7;
    case 'b':  return 8;
    case 'e':  return 27;
    case 'f':  return '\f';
    case 'n':  return '\n';
    case 'r':  return '\r';
    case 't':  return '\t';
    case 'v':  return 11;
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
    const char* s = &image[0];
    if (image.size() >= 2 && image[0] == '0') {
        if (image[1] == 'X' || image[1] == 'x') {
            base = 16;
            s = &image[2];
        } else {
            base = 8;
            s = &image[1];
        }
    }
    long value = strtol(s, 0, base);
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
