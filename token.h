#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>

using namespace std;

class Token {
public:
    Token() : 
        kind(0), image(""), 
        begin_line(0),
        begin_column(0), 
        end_line(0), 
        end_column(0) {}

    Token(int kind, const string& image) : 
        kind(kind), image(image),
        begin_line(0), begin_column(0), 
        end_line(0), end_column(0) {}

    Token(const Token& token) : 
        kind(token.kind), image(token.image),
        begin_line(token.begin_line), 
        begin_column(token.begin_column), 
        end_line(token.end_line), 
        end_column(token.end_column) {}
       
    Token(Token&& token) : 
        kind(token.kind), image(token.image),
        begin_line(token.begin_line), 
        begin_column(token.begin_column), 
        end_line(token.end_line), 
        end_column(token.end_column) {}

public:
    int kind;
    int begin_line;
    int begin_column;
    int end_line;
    int end_column;
    string image;
};

#endif