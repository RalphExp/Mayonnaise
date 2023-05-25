#ifndef UTIL_H_
#define UTIL_H_

#include <string>

#include "token.h"

using namespace std;

class Node;
class Type;
class TypeRef;

class Location {
public:
    Location() {}
    Location(const Token& tok);
    Location(const string& src, const Token& tok);
   
    int lineno() { return tok_.begin_line_; }
    int column() { return tok_.begin_column_; }
    string source_name() { return src_; }
    string to_string() const { return src_ + ":" + std::to_string(tok_.begin_line_); }

protected:
    string src_;
    Token tok_;
};

class Dumper {
public:
    Dumper(ostream &os);

    void print_indent(void);
    void print_class(const Node& node, const Location &loc);
    void print_pair(const string& name, const string& val);
    void print_member(const string& name, int n);
    void print_member(const string& name, long l);
    void print_member(const string& name, bool b);
    void print_member(const string& name, const TypeRef& ref);
    void print_member(const string& name, Type* type);
    void print_member(const string& name, Node* node);

protected:
    void indent() { ++indent_; }
    void dedent() { --indent_; }

protected:
    ostream& os_; // output stream
    int indent_; // indent
};

#endif