#ifndef AST_NODE_H_
#define AST_NODE_H_

#include <cassert>
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
    string name() { return name_; }

    virtual ~Node() {};
    virtual void dump_node(Dumper& dumper) = 0;
    virtual Location location() = 0;
protected:
    string name_;
};

class ExprNode : public Node {
public:
    ExprNode() {}
    virtual ~ExprNode() {}
    virtual Type* type() = 0;
    virtual Type* orig_type() { return type(); }
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
    TypeNode(Type* tp) : type_(tp), ref_(nullptr) {}
    TypeNode(TypeRef* ref) : type_(nullptr), ref_(ref) {}
    TypeNode(Type* tp, TypeRef* ref) : type_(tp), ref_(ref) {}
    ~TypeNode();
    Location location() { return ref_->location(); }
    Type* type();
    TypeRef* type_ref() { return ref_; }  
    bool is_resolved() { return type() != nullptr; } 
    void set_type(Type* tp);
    void dump_node(Dumper& dumper);

protected:
    TypeRef* ref_;
    Type* type_;
};

class LiteralNode : public ExprNode {
public:
    LiteralNode(const Location& loc, TypeRef* ref);
    ~LiteralNode();

    Location location() { return loc_; }
    TypeNode* type_node() { return tnode_; }
    Type* type() { return tnode_->type(); }
    bool is_constant() { return true; }

protected:
    Location loc_;
    TypeNode* tnode_;
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
    Type* type() { return type_; }
    void set_type(Type* type) { delete type_; type_ = type; }
    long alloc_size() { return orig_type()->alloc_size(); }
    bool is_lvalue() { return true; }
    bool is_assignable() { return is_loadable(); }
    bool is_loadable();

protected:
    Type* orig_type() { return orig_type_; }

protected:
    Type* type_;
    Type* orig_type_;
};

class VariableNode : public LHSNode {
public:
    VariableNode(const Location& loc, const string& name);
    // VariableNode(DefinedVariable* var);
    ~VariableNode();
    string name() { return name_; }
    Location location() { return loc_; };
    // Entity* entity();
    // void set_entity();
    // bool is_resolved();
    // bool is_lvalue();
    // bool is_assignable();
    // bool is_parameter();
    // Type* orig_type();
    // TypeNode* type_node();
    
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
    string op() { return op_; }
    Type* type() { return expr_->type(); }
    ExprNode* expr() { return expr_; }
    Location location() { return expr_->location(); }
    void set_op_type(Type* type) { delete op_type_; op_type_ = type; }
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
    void set_expr(ExprNode* expr) { delete expr_; expr_ = expr; }
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
    ExprNode* expr() { return expr_; }
    ExprNode* index() { return index_; }
    ExprNode* base_expr();
    Type* orig_type();
    bool is_multi_dimension();
    long element_size() { return orig_type()->alloc_size(); }
    long length();
    void dump_node(Dumper& dumper);
    Location location() { return expr_->location(); }

protected:
    ExprNode* expr_;
    ExprNode* index_;
};

class Slot : public Node {
public:
    Slot();
    Slot(TypeNode* t, const string& n);
    TypeNode* type_node() { return tnode_; }
    TypeRef* type_ref() { assert(tnode_); return tnode_->type_ref(); }
    Type* type() { assert(tnode_); return tnode_->type(); }
    string name() { return name_; }
    Location location() { return tnode_->location(); }
    long size() { return type()->size(); }
    long alloc_size() { return type()->alloc_size(); }
    long allignment() { return type()->alignment(); }
    long offset() { return offset_; }
    void set_offset(long offset) { offset_ = offset; }
    void dump_node(Dumper& dumper);

protected:
    string name_;
    TypeNode* tnode_;
    long offset_;
};

class MemberNode : public LHSNode {
public:
    MemberNode(ExprNode* expr, const string& member);
    CompositeType* base_type();
    Location location() { return expr()->location(); }
    ExprNode* expr() { return expr_; }
    string member() { return member_; }
    long offset() { return base_type()->member_offset(member_); }

protected:
    Type* orig_type() { return base_type()->member_type(member_); }
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
    string member_;
};

class PtrMemberNode : public LHSNode {
public:
    PtrMemberNode(ExprNode* expr, const string& member);
    CompositeType* derefered_composite_type();
    Type* derefered_type();
    ExprNode* expr() { return expr_; }
    string member() { return member_; }
    long offset() { return derefered_composite_type()->member_offset(member_); }
    Location location() { return expr_->location(); }
    
protected:
    Type* orig_type() { return derefered_composite_type()->member_type(member_); }
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
    string member_;
};

class FuncallNode : public ExprNode {
public:
    FuncallNode(ExprNode* expr, const vector<ExprNode*>& args);
    FuncallNode(ExprNode* expr, vector<ExprNode*>&& args);
    ~FuncallNode();

    ExprNode* expr() { return expr_; }
    Type* type();
    // FunctionType* functionType();
    long num_args() { return args_.size(); }
    vector<ExprNode*> args() { return args_; }
    void replaceArgs(const vector<ExprNode*>& args) { args_ = args; }
    void replaceArgs(vector<ExprNode*>&& args) { args_ = move(args); }
    Location location() { return expr_->location(); }
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
    vector<ExprNode*> args_;
};

class SizeofExprNode : public ExprNode {
public:
    SizeofExprNode(ExprNode* expr, TypeRef* ref);
    ExprNode* expr() { return expr_; }
    void set_expr(ExprNode* expr) { delete expr_; expr_ = expr; }
    Type* type() { return tnode_->type(); }
    TypeNode* typeNode() { return tnode_; }
    Location location() { return expr_->location(); }
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
    TypeNode* tnode_;
};

class SizeofTypeNode : public ExprNode {
public:
    SizeofTypeNode(TypeNode* operand, TypeRef* type);
    Type* operand() { return op_->type(); }
    TypeNode* operand_type_node() { return op_; }
    Type* type() { return tnode_->type(); }
    TypeNode* type_node() { return tnode_; }
    Location location() { return op_->location(); }
    void dump_node(Dumper& dumper);

protected:
    TypeNode* op_;
    TypeNode* tnode_;
};

class AddressNode : public ExprNode {
public:
    AddressNode(ExprNode* expr);
    ExprNode* expr() { return expr_; }
    Type* type();
    void set_type(Type* type);
    Location location() { return expr_->location(); }
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
    Type* type_;
};

class DereferenceNode : public LHSNode {
public:
    DereferenceNode(ExprNode* expr);
    Type* orig_type();
    ExprNode* expr() { return expr_; };
    void set_expr(ExprNode* expr) { delete expr_; expr_ = expr; }
    Location location() { return expr_->location(); }
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
};

class PrefixOpNode : public UnaryArithmeticOpNode {
public:
    PrefixOpNode(const string& op, ExprNode* expr);
};

} // namespace ast

#endif