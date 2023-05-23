#ifndef UTIL_H_
#define UTIL_H_

#include <string>

#include "token.h"

using namespace std;

class Location {
public:
    Location() {}

    Location(const string& src, Token tok) : 
        src_name(src), token(tok) {}
   
    int lineno() { return token.begin_line; }
    int column() { return token.begin_column; }
    string source_name() { return src_name; }

    string to_string() { 
        return src_name + ":" + std::to_string(token.begin_line); 
    }

protected:
    string src_name;
    Token token;
};

class Dumper {
public:
    Dumper() {}
};

#endif