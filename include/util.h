#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <vector>
#include <type_traits>
#include <ostream>

#include "token.h"

using namespace std;

namespace may {
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
    string to_string() const { return (src_.size() ? src_ : "(unknown)") + \
        ": at line " + std::to_string(tok_.begin_line_) + \
        ", column " + std::to_string(tok_.begin_column_); }

protected:
    string src_;
    Token tok_;
};

class Dumper {
public:
    Dumper(ostream &os);
    void print_indent(void);
    void print_class(Node* node, const Location &loc);
    void print_pair(const string& name, const string& val);
    void print_member(const string& name, int n);
    void print_member(const string& name, long l);
    void print_member(const string& name, bool b);
    void print_member(const string& name, const string& str);
    void print_member(const string& name, const string& str, bool is_resolved);
    void print_member(const string& name, TypeRef* ref);
    void print_member(const string& name, Type* type);
    void print_member(const string& name, Node* node);

    template<typename T>
    void print_node_list(const string& name, const vector<T*>& nodes) {
        print_indent();
        os_ << name << ":" << endl;
        indent();
        for (T* n : nodes) {
            n->dump(*this);
        }
        dedent();
    }

    template<typename T>
    void print_class(T* node, const Location& loc) {
        print_indent();
        os_ << "<<" << node->class_name() << ">>"
        << "(" << loc.to_string() << ")" 
        << endl;
    }

protected:
    void indent() { ++indent_; }
    void dedent() { --indent_; }

protected:
    ostream& os_; // output stream
    int indent_; // indent
};

class Dumpable {
public:
    virtual void dump(Dumper& dumper) = 0;
};

} // namespace may

#endif