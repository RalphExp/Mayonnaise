#ifndef AST_NODE_H_
#define AST_NODE_H_

#include <iostream>
#include <memory>

#include "util.h"
#include "ast.h"
#include "type.h"

using namespace std;

class Type;

class Node {
public:
    Node() {}
    virtual void dump_node(Dumper& dumper) = 0;
    void dump(ostream& os=cout);
    void dump(Dumper& dumper);
    string name(void) const { return name_; }

protected:
    string name_;
    Location loc_;    
};

class ExprNode : public Node {
public:
    ExprNode() {}
    virtual ~ExprNode() {}
    virtual Type* type() = 0;
    virtual long alloc_size() { return type()->alloc_size(); }
    virtual bool is_const() { return false; }
    virtual bool is_parameter() { return false; }
    virtual bool is_lvalue() { return false; }
    virtual bool is_assignable() { return false; }
    virtual bool is_loadable() { return false; }
    virtual bool is_callable();
    virtual bool is_pointer();
};

class TypeNode : public Node {
public:
    TypeNode(Type* tp) { type_.reset(tp); }
    TypeNode(const TypeRef& ref) : ref_(ref) {}
    Location location() { return ref_.location(); }
    Type* type();    
    bool is_resolved() { return type() != nullptr; } 
    void setType(Type* tp);
    void dump_node(Dumper& dumper);

protected:
    TypeRef ref_;
    shared_ptr<Type> type_;
};

class LiteralNode : public ExprNode {
public:
    LiteralNode(const Location& loc, const TypeRef& ref) : 
        loc_(loc), type_node_(ref) {}
    
    Location location() { return loc_; }
    TypeNode* type_node() { return &type_node_; }
    Type* type() { return type_node_.type(); }
    bool is_constant() { return true; }

protected:
    Location loc_;
    TypeNode type_node_;
};

class IntegerLiteralNode : public LiteralNode {
public:
    IntegerLiteralNode(const Location& loc, const TypeRef& ref, long value) :
        LiteralNode(loc, ref), value_(value) {}

    long value() { return value_; }

protected:
    void dump_node(Dumper& dumper) {
        dumper.print_member("typeNode", &type_node_);
        dumper.print_member("value", value_);
    }

protected:
    long value_;
};

#endif