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
    void dump(ostream& os=cout);
    void dump(Dumper& dumper);
    string name(void) const { return name_; }

    virtual ~Node() {};
    virtual void dump_node(Dumper& dumper) = 0;
    virtual Location location() const = 0;
protected:
    string name_;
};

class ExprNode : public Node {
public:
    ExprNode() {}
    virtual Type* type() const = 0;
    virtual Type* orig_type() { return type(); }
    virtual long alloc_size() const { return type()->alloc_size(); }
    virtual bool is_const() const { return false; }
    virtual bool is_parameter() const { return false; }
    virtual bool is_lvalue() const { return false; }
    virtual bool is_assignable() const { return false; }
    virtual bool is_loadable() const { return false; }
    virtual bool is_callable() const ;
    virtual bool is_pointer() const;
};

class TypeNode : public Node {
public:
    TypeNode(Type* tp) : type_(tp), ref_(nullptr) {}
    TypeNode(TypeRef* ref) : type_(nullptr), ref_(ref) {}
    TypeNode(Type* tp, TypeRef* ref) : type_(tp), ref_(ref) {}
    ~TypeNode();
    Location location() const { return ref_->location(); }
    Type* type() const;
    TypeRef* type_ref() { return ref_; }  
    bool is_resolved() { return type() != nullptr; } 
    void setType(Type* tp);
    void dump_node(Dumper& dumper);

protected:
    TypeRef* ref_;
    Type* type_;
};

class LiteralNode : public ExprNode {
public:
    LiteralNode(const Location& loc, TypeRef* ref);
    ~LiteralNode();

    Location location() const { return loc_; }
    TypeNode* type_node() { return type_node_; }
    Type* type() const { return type_node_->type(); }
    bool is_constant() const { return true; }

protected:
    Location loc_;
    TypeNode* type_node_;
};

class IntegerLiteralNode : public LiteralNode {
public:
    IntegerLiteralNode(const Location& loc, TypeRef* ref, long value);
    long value() { return value_; }

protected:
    void dump_node(Dumper& dumper);

protected:
    long value_;
};

class StringLiteralNode : public LiteralNode {
public:
    StringLiteralNode(const Location& loc, TypeRef* ref, const string& value);
    ~StringLiteralNode() { delete entry_; }
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
    ~LHSNode();
    Type* type() const { return type_; }
    void set_type(Type* type) { type_ = type; }
    long alloc_size() const { return orig_type()->alloc_size(); }
    bool is_lvalue() const { return true; }
    bool is_assignable() const { return is_loadable(); }
    bool is_loadable() const;

protected:
    Type* orig_type() const { return orig_type_; }

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

class UnaryOpNode : public ExprNode {
public:
    UnaryOpNode(const string& op, ExprNode* node);
    ~UnaryOpNode();
    string op() const { return op_; }
    Type* type() const { return expr_->type(); }
    ExprNode* expr() const { return expr_; }
    Location location() const { return expr_->location(); }
    void set_op_type(Type* type) { op_type_ = type; }
    void set_expr(ExprNode* expr) { expr_ = expr;}
    void dump_node(Dumper& dumper);

protected:
    string op_;
    Type* op_type_;
    ExprNode* expr_;
};

class UnaryArithmeticOpNode : public UnaryOpNode {
public:
    ~UnaryArithmeticOpNode() {}
    UnaryArithmeticOpNode(const string& op, ExprNode* expr);
    long amount() const { return amount_; }
    void set_expr(ExprNode* expr) { expr_ = expr; }
    void set_amount(long amount) { amount_ = amount; }

protected:
    long amount_;
};

class SuffixOpNode : public UnaryArithmeticOpNode {
public:
    SuffixOpNode(const string& op, ExprNode* expr);
};


// Array Reference Node
class ArefNode : public LHSNode {
public:
    ArefNode(ExprNode* expr, ExprNode* index);
    ~ArefNode();
    ExprNode* expr() const { return expr_; }
    ExprNode* index() const { return index_; }
    ExprNode* base_expr() const;
    Type* orig_type() const;
    bool is_multi_dimension() const;
    long element_size() const { return orig_type()->alloc_size(); }
    long length() const;
    void dump_node(Dumper& dumper);
    Location location() const { return expr_->location(); }

protected:
    ExprNode* expr_;
    ExprNode* index_;
};

class Slot : public Node {
public:
    Slot(TypeNode* t, const string& n);
    TypeNode* type_node() const { return type_node_; }
    TypeRef* type_ref() const { return type_node_->type_ref(); }
    Type* type() const { return type_node_->type(); }
    string name() const { return name_; }
    Location location() { return type_node_->location(); }
    long size() const { return type()->size(); }
    long alloc_size() const { return type()->alloc_size(); }
    long allignment() const { return type()->alignment(); }
    long offset() const { return offset_; }
    void set_offset(long offset) { offset_ = offset; }
    void dump_node(Dumper& dumper);

protected:
    string name_;
    TypeNode* type_node_;
    long offset_;
};

class MemberNode : public LHSNode {
public:
    MemberNode(ExprNode* expr, const string& member);
    CompositeType* base_type();
    // Type* orig_type() const { base_type()->member_type(member_); }
    Location location() const { return expr()->location(); }
    ExprNode* expr() const { return expr_; }
    string member() const { return member_; }
    // long offset() const { return base_type()->member_offset(member_); }
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
    string member_;
};

class PtrMemberNode : public LHSNode {
public:
};

class FuncallNode : public ExprNode {
public:
};


} // namespace ast

#endif