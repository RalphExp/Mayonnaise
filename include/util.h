#ifndef UTIL_H_
#define UTIL_H_

#include <string>

#include "token.h"

using namespace std;

class Node;

class Location {
public:
    Location() {}

    Location(const string& src, Token tok) : 
        src_(src), tok_(tok) {}
   
    int lineno() { return tok_.begin_line_; }
    int column() { return tok_.begin_column_; }
    
    string source_name() { return src_; }
    string to_string() const { 
        return src_ + ":" + std::to_string(tok_.begin_line_); 
    }

protected:
    string src_;
    Token tok_;
};

class Dumper {
public:
    Dumper(ostream &os);

    void print_indent(void);
    void print_class(const Node& node, const Location &loc);

protected:
    void indent() { ++indent_; }
    void dedent() { --indent_; }

protected:
    ostream& os_; // output stream
    int indent_; // indent
};

#endif