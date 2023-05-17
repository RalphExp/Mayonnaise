

#ifndef LEXER_H_
#define LEXER_H_

namespace yy {

class Lexer : public yyFlexLexer {

public:
    Lexer(std::istream& yyin, std::ostream& yyout) : yyFlexLexer(yyin, yyout) {}
    Lexer(std::istream* yyin=nullptr, std::ostream* yyout=nullptr) : yyFlexLexer(yyin, yyout) {}

    int lex(yy::Parser::value_type *yylval);
};

}

#endif // LEXER_H_