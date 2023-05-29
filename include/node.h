#ifndef AST_NODE_H_
#define AST_NODE_H_

#include <iostream>
#include <memory>

#include "entry.h"
#include "util.h"
#include "type.h"

using namespace std;

namespace ast {

class Node : public Dumpable {
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
    virtual Type* type() const = 0;
    virtual long alloc_size() const { return type()->alloc_size(); }
    virtual bool is_const() const { return false; }
    virtual bool is_parameter() const { return false; }
    virtual bool is_lvalue() const { return false; }
    virtual bool is_assignable() const { return false; }
    virtual bool is_loadable() { return false; }
    virtual bool is_callable() const ;
    virtual bool is_pointer() const;
};

class TypeNode : public Node {
public:
    TypeNode(Type* tp) : type_(tp), ref_(nullptr) {}
    TypeNode(TypeRef* ref) : type_(nullptr), ref_(ref) {}
    TypeNode(Type* tp, TypeRef* ref) : type_(tp), ref_(ref) {}
    ~TypeNode();
    Location location() { return ref_->location(); }
    Type* type() const;    
    bool is_resolved() { return type() != nullptr; } 
    void setType(Type* tp);
    void dump_node(Dumper& dumper);

protected:
    TypeRef* ref_;
    Type* type_;
};

class LiteralNode : public ExprNode {
public:
    LiteralNode(const Location& loc, TypeRef* ref) : 
        loc_(loc), type_node_(ref) {}
    
    Location location() { return loc_; }
    TypeNode* type_node() { return &type_node_; }
    Type* type() const { return type_node_.type(); }
    bool is_constant() { return true; }

protected:
    Location loc_;
    TypeNode type_node_;
};

class IntegerLiteralNode : public LiteralNode {
public:
    IntegerLiteralNode(const Location& loc, TypeRef* ref, long value) :
        LiteralNode(loc, ref), value_(value) {}

    long value() { return value_; }

protected:
    void dump_node(Dumper& dumper);

protected:
    long value_;
};

class StringLiteralNode : public LiteralNode {
public:
    StringLiteralNode(const Location& loc, TypeRef* ref, const string& value);
    string value() { return value_; }
    ConstantEntry* entry() { return entry_; }
    
protected:
    void dump_node(Dumper& dumper);

protected:
    string value_;
    ConstantEntry* entry_;
};

class LHSNode : public ExprNode {
public:
    LHSNode();
    Type* type() const;

protected:
    Type* type_;
    Type* orig_type_;
};

class VariableNode : public LHSNode {
public:
    VariableNode(const Location& loc, const string& name);
    // VariableNode(DefinedVariable* var);
    ~VariableNode();
    string name() const { return name_; }
    Location location() const { return loc_; };
    // Entity* entity();
    // void set_entity();
    // bool is_resolved() const;
    // bool is_lvalue() const;
    // bool is_assignable() const;
    // bool is_parameter() const;
    // Type* orig_type() const;
    // TypeNode* type_node() const;
    
protected:
    void dump_node(Dumper& dumper) {}

protected:
    Location loc_;
    string name_;
    // Entity* entity_;
};

}

#endif