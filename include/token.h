#ifndef TOKEN_H_
#define TOKEN_H_

#include <string>

using namespace std;

namespace cbc {

class Token {
public:
    Token() : 
        kind_(0), image_(""), 
        begin_line_(0),
        begin_column_(0), 
        end_line_(0), 
        end_column_(0) {}

    Token(int kind, const string& image) : 
        kind_(kind), image_(image),
        begin_line_(0), 
        begin_column_(0), 
        end_line_(0), end_column_(0) {}

    Token(const Token& token) : 
        kind_(token.kind_), image_(token.image_),
        begin_line_(token.begin_line_), 
        begin_column_(token.begin_column_), 
        end_line_(token.end_line_), 
        end_column_(token.end_column_) {}
       
    Token(Token&& token) : 
        kind_(token.kind_), image_(token.image_),
        begin_line_(token.begin_line_), 
        begin_column_(token.begin_column_), 
        end_line_(token.end_line_), 
        end_column_(token.end_column_) {}

public:
    int kind_;
    int begin_line_;
    int begin_column_;
    int end_line_;
    int end_column_;
    string image_;
};

} // namespace cbc

#endif