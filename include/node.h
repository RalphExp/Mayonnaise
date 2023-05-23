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

    void dump(ostream& os=cout) {
        Dumper dumper(os);
        dump(dumper);
    }
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

    template<typename S, typename E>
    E accept(const ASTVisitor<S,E> &visitor);
};

class TypeNode : public Node {
public:
    TypeNode(Type* tp) { tp_.reset(tp); }
    TypeNode(const TypeRef& ref) : ref_(ref) {}
    virtual Type* type() { return tp_.get(); }
    void dump_node(Dumper& dumper) {};

protected:
    TypeRef ref_;
    shared_ptr<Type> tp_;
};

class LiteralNode : public ExprNode {
public:
    LiteralNode(const Location& loc, const TypeRef& ref) : loc_(loc), tn_(ref) {}
    Location& location() { return loc_; }
    TypeNode& type_node() { return tn_; }
    Type* type() { return tn_.type(); }
    bool is_constant() { return true; }

protected:
    Location loc_;
    TypeNode tn_;
};

class IntegerLiteralNode : public LiteralNode {
public:
    IntegerLiteralNode(const Location& loc, const TypeRef& ref, long value) :
        LiteralNode(loc, ref), val_(value) {}

    long value() { return val_; }


protected:
    long val_;
};

#endif