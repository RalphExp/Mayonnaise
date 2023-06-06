#ifndef AST_NODE_H_
#define AST_NODE_H_

#include <cassert>
#include <iostream>
#include <memory>

#include "entity.h"
#include "util.h"
#include "type.h"

using namespace std;

namespace may {

// namespace ast

class Node : public Dumpable {
public:
    Node() {}
    void dump(ostream& os=cout);
    void dump(Dumper& dumper);
    virtual string class_name() = 0;
    virtual ~Node() {};
    virtual void dump_node(Dumper& dumper) = 0;
    virtual Location location() = 0;
};

class ExprNode : public Node {
public:
    ExprNode() {}
    virtual ~ExprNode() {}
    virtual shared_ptr<Type> type() = 0;
    virtual shared_ptr<Type> orig_type() { return type(); }
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
    TypeNode(shared_ptr<Type> tp);
    TypeNode(shared_ptr<TypeRef> ref);
    TypeNode(shared_ptr<Type> tp, shared_ptr<TypeRef> ref);
    ~TypeNode();
    Location location() { return ref_->location(); }
    shared_ptr<Type> type();
    shared_ptr<TypeRef> type_ref() { return ref_; }  
    bool is_resolved() { return !!type_; } 
    void set_type(shared_ptr<Type> tp);
    void dump_node(Dumper& dumper);
    string class_name() { return "TypeNode"; }

protected:
    shared_ptr<TypeRef> ref_;
    shared_ptr<Type> type_;
};

class LiteralNode : public ExprNode {
public:
    LiteralNode(const Location& loc, shared_ptr<TypeRef> ref);
    ~LiteralNode();

    Location location() { return loc_; }
    shared_ptr<TypeNode> type_node() { return tnode_; }
    shared_ptr<Type> type() { return tnode_->type(); }
    bool is_constant() { return true; }
    string class_name() { return "LiteralNode"; }

protected:
    Location loc_;
    shared_ptr<TypeNode> tnode_;
};

class IntegerLiteralNode : public LiteralNode {
public:
    IntegerLiteralNode(const Location& loc, shared_ptr<TypeRef> ref, long value);
    long value() { return value_; }
    string class_name() { return "IntegerLiteralNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    long value_;
};

class StringLiteralNode : public LiteralNode {
public:
    StringLiteralNode(const Location& loc, shared_ptr<TypeRef> ref, const string& value);
    ~StringLiteralNode() {}
    string value() { return value_; }
    string class_name() { return "StringLiteralNode"; }
    shared_ptr<ConstantEntry> entry() { return entry_; }
    
protected:
    void dump_node(Dumper& dumper);

protected:
    string value_;
    shared_ptr<ConstantEntry> entry_;
};

class LHSNode : public ExprNode {
public:
    LHSNode();
    ~LHSNode();
    shared_ptr<Type> type() { return type_; }
    void set_type(shared_ptr<Type> type) { type_ = type; }
    long alloc_size() { return orig_type()->alloc_size(); }
    bool is_lvalue() { return true; }
    bool is_assignable() { return is_loadable(); }
    bool is_loadable();
    string class_name() { return "LHSNode"; }

protected:
    shared_ptr<Type> orig_type() { return orig_type_; }

protected:
    shared_ptr<Type> type_;
    shared_ptr<Type> orig_type_;
};

class VariableNode : public LHSNode {
public:
    VariableNode(const Location& loc, const string& name);
    VariableNode(shared_ptr<DefinedVariable> var);
    ~VariableNode();
    string name() { return name_; }
    Location location() { return loc_; };
    shared_ptr<Entity> entity();
    void set_entity(shared_ptr<Entity> ent);
    bool is_resolved() { return entity_ != nullptr; }
    bool is_lvalue();
    bool is_assignable();
    bool is_parameter();
    shared_ptr<Type> orig_type();
    shared_ptr<TypeNode> type_node();
    string class_name() { return "VariableNode"; }
    
protected:
    void dump_node(Dumper& dumper) {}

protected:
    Location loc_;
    string name_;
    shared_ptr<Entity> entity_;
};

class UnaryOpNode : public ExprNode {
public:
    UnaryOpNode(const string& op, shared_ptr<ExprNode> node);
    ~UnaryOpNode();
    string op() { return op_; }
    shared_ptr<Type> type() { return expr_->type(); }
    shared_ptr<ExprNode> expr() { return expr_; }
    Location location() { return expr_->location(); }
    void set_op_type(shared_ptr<Type> type) { op_type_ = type; }
    void set_expr(shared_ptr<ExprNode> expr) { expr_ = expr; }
    string class_name() { return "UnaryOpNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    string op_;
    shared_ptr<Type> op_type_;
    shared_ptr<ExprNode> expr_;
};

class UnaryArithmeticOpNode : public UnaryOpNode {
public:
    ~UnaryArithmeticOpNode() {}
    UnaryArithmeticOpNode(const string& op, shared_ptr<ExprNode> expr);
    long amount() const { return amount_; }
    void set_expr(shared_ptr<ExprNode> expr) { expr_ = expr; }
    void set_amount(long amount) { amount_ = amount; }
    string class_name() { return "UnaryArithmeticOpNode"; }

protected:
    long amount_;
};

class SuffixOpNode : public UnaryArithmeticOpNode {
public:
    SuffixOpNode(const string& op, shared_ptr<ExprNode> expr);
    string class_name() { return "SuffixOpNode"; }
};


// Array Reference Node
class ArefNode : public LHSNode {
public:
    ArefNode(shared_ptr<ExprNode> expr, shared_ptr<ExprNode> index);
    ~ArefNode();
    shared_ptr<ExprNode> expr() { return expr_; }
    shared_ptr<ExprNode> index() { return index_; }
    shared_ptr<ExprNode> base_expr();
    shared_ptr<Type> orig_type();
    bool is_multi_dimension();
    long element_size() { return orig_type()->alloc_size(); }
    long length();
    Location location() { return expr_->location(); }
    string class_name() { return "ArefNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> expr_;
    shared_ptr<ExprNode> index_;
};

class Slot : public Node {
public:
    Slot();
    Slot(shared_ptr<TypeNode> t, const string& n);
    ~Slot() {}

    shared_ptr<TypeNode> type_node() { return tnode_; }
    shared_ptr<TypeRef> type_ref() { assert(tnode_); return tnode_->type_ref(); }
    shared_ptr<Type> type() { assert(tnode_); return tnode_->type(); }
    string name() { return name_; }
    Location location() { return tnode_->location(); }
    long size() { return type()->size(); }
    long alloc_size() { return type()->alloc_size(); }
    long allignment() { return type()->alignment(); }
    long offset() { return offset_; }
    void set_offset(long offset) { offset_ = offset; }
    string class_name() { return "Slot"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    string name_;
    shared_ptr<TypeNode> tnode_;
    long offset_;
};

class MemberNode : public LHSNode {
public:
    MemberNode(shared_ptr<ExprNode> expr, const string& member);
    CompositeType* base_type();
    Location location() { return expr()->location(); }
    shared_ptr<ExprNode> expr() { return expr_; }
    string member() { return member_; }
    long offset() { return base_type()->member_offset(member_); }
    string class_name() { return "MemberNode"; }

protected:
    shared_ptr<Type> orig_type() { return base_type()->member_type(member_); }
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> expr_;
    string member_;
};

class PtrMemberNode : public LHSNode {
public:
    PtrMemberNode(shared_ptr<ExprNode> expr, const string& member);
    CompositeType* derefered_composite_type();
    shared_ptr<Type> derefered_type();
    shared_ptr<ExprNode> expr() { return expr_; }
    string member() { return member_; }
    long offset() { return derefered_composite_type()->member_offset(member_); }
    Location location() { return expr_->location(); }
    string class_name() { return "PtrMemberNode"; }
    
protected:
    shared_ptr<Type> orig_type() { return derefered_composite_type()->member_type(member_); }
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> expr_;
    string member_;
};

class FuncallNode : public ExprNode {
public:
    FuncallNode(shared_ptr<ExprNode> expr, shared_ptr<vector<shared_ptr<ExprNode>>> args);
    ~FuncallNode();

    shared_ptr<ExprNode> expr() { return expr_; }
    shared_ptr<Type> type();
    // Functionshared_ptr<Type> functionType();
    long num_args() { return args_->size(); }

    shared_ptr<vector<shared_ptr<ExprNode>>> args() { return args_; }

    void replaceArgs(shared_ptr<vector<shared_ptr<ExprNode>>> args) { args_ = args; }

    Location location() { return expr_->location(); }
    string class_name() { return "FuncallNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> expr_;
    shared_ptr<vector<shared_ptr<ExprNode>>> args_;
};

class SizeofExprNode : public ExprNode {
public:
    SizeofExprNode(shared_ptr<ExprNode> expr, shared_ptr<TypeRef> ref);
    shared_ptr<ExprNode> expr() { return expr_; }
    void set_expr(shared_ptr<ExprNode> expr) { expr_ = expr; }
    shared_ptr<Type> type() { return tnode_->type(); }
    shared_ptr<TypeNode> typeNode() { return tnode_; }
    Location location() { return expr_->location(); }
    string class_name() { return "SizeofExprNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> expr_;
    shared_ptr<TypeNode> tnode_;
};

class SizeofTypeNode : public ExprNode {
public:
    SizeofTypeNode(shared_ptr<TypeNode> operand, shared_ptr<TypeRef> type);
    shared_ptr<Type> operand() { return op_->type(); }
    shared_ptr<TypeNode> operand_type_node() { return op_; }
    shared_ptr<Type> type() { return tnode_->type(); }
    shared_ptr<TypeNode> type_node() { return tnode_; }
    Location location() { return op_->location(); }
    string class_name() { return "SizeofTypeNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<TypeNode> op_;
    shared_ptr<TypeNode> tnode_;
};

class AddressNode : public ExprNode {
public:
    AddressNode(shared_ptr<ExprNode> expr);
    shared_ptr<ExprNode> expr() { return expr_; }
    shared_ptr<Type> type();
    void set_type(shared_ptr<Type> type);
    Location location() { return expr_->location(); }
    string class_name() { return "AddressNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> expr_;
    shared_ptr<Type> type_;
};

class DereferenceNode : public LHSNode {
public:
    DereferenceNode(shared_ptr<ExprNode> expr);
    shared_ptr<Type> orig_type();
    shared_ptr<ExprNode> expr() { return expr_; };
    void set_expr(shared_ptr<ExprNode> expr) { expr_ = expr; }
    Location location() { return expr_->location(); }
    string class_name() { return "DereferenceNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> expr_;
};

class PrefixOpNode : public UnaryArithmeticOpNode {
public:
    PrefixOpNode(const string& op, shared_ptr<ExprNode> expr);
    string class_name() { return "PrefixOpNode"; }
};

class CastNode : public ExprNode {
public:
    CastNode(shared_ptr<Type> t, shared_ptr<ExprNode> expr);
    CastNode(shared_ptr<TypeNode> t, shared_ptr<ExprNode> expr);
    ~CastNode();
    shared_ptr<Type> type() { return tnode_->type(); }
    shared_ptr<TypeNode> typde_node() { return tnode_; }
    shared_ptr<ExprNode> expr() { return expr_; }
    bool is_lvalue() { return expr_->is_lvalue(); }
    bool is_assignable() { return expr_->is_assignable(); }
    bool is_effectiveCast() { return type()->size() > expr_->type()->size(); }
    Location location() { return tnode_->location(); }
    string class_name() { return "CastNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<TypeNode> tnode_;
    shared_ptr<ExprNode> expr_;
};

class BinaryOpNode : public ExprNode {
public:
    BinaryOpNode(shared_ptr<ExprNode> left, const string& op, shared_ptr<ExprNode> right);
    BinaryOpNode(shared_ptr<Type> t, shared_ptr<ExprNode> left, const string& op, shared_ptr<ExprNode> right);
    ~BinaryOpNode();
    string op() { return op_; }
    shared_ptr<Type> type() { return type_ ? type_ : left_->type(); }
    void set_type(shared_ptr<Type> type);
    shared_ptr<ExprNode> left() { return left_; }
    shared_ptr<ExprNode> right() { return right_; }
    void set_left(shared_ptr<ExprNode> l) { left_ = l; }
    void set_right(shared_ptr<ExprNode> r) { right_ = r; }

    Location location() { return left_->location(); }
    string class_name() { return "BinaryOpNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> left_;
    shared_ptr<ExprNode> right_;
    shared_ptr<Type> type_;
    string op_;
};

class CondExprNode : public ExprNode {
public:
    CondExprNode(shared_ptr<ExprNode> c, shared_ptr<ExprNode> t, shared_ptr<ExprNode> e);
    ~CondExprNode();
    shared_ptr<Type> type() { return then_expr_->type(); }
    shared_ptr<ExprNode> cond() { return cond_ ;}
    shared_ptr<ExprNode> then_expr() { return then_expr_; }
    shared_ptr<ExprNode> else_expr() { return else_expr_; }

    void set_then_expr(shared_ptr<ExprNode> expr) { then_expr_ = expr; }
    void set_else_expr(shared_ptr<ExprNode> expr) { else_expr_ = expr; }

    Location location() { return cond_->location(); }
    string class_name() { return "CondExprNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> cond_;
    shared_ptr<ExprNode> then_expr_;
    shared_ptr<ExprNode> else_expr_;
};

class LogicalOrNode : public BinaryOpNode {
public:
    LogicalOrNode(shared_ptr<ExprNode> left, shared_ptr<ExprNode> right);
    string class_name() { return "LogicalOrNode"; }
};

class LogicalAndNode : public BinaryOpNode {
public:
    LogicalAndNode(shared_ptr<ExprNode> left, shared_ptr<ExprNode> right);
    string class_name() { return "LogicalAndNode"; }
};

class AbstractAssignNode : public ExprNode {
public:
    AbstractAssignNode(shared_ptr<ExprNode> lhs, shared_ptr<ExprNode> rhs);
    ~AbstractAssignNode();

    shared_ptr<Type> type() { return lhs_->type(); }
    shared_ptr<ExprNode> lhs() { return lhs_; }
    shared_ptr<ExprNode> rhs() { return rhs_; }
    
    void set_rhs(shared_ptr<ExprNode> expr) { rhs_ = expr; }

    Location location() { return lhs_->location(); }
    string class_name() { return "AbstractAssignNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> lhs_;
    shared_ptr<ExprNode> rhs_;
};

class AssignNode : public AbstractAssignNode {
public:
    AssignNode(shared_ptr<ExprNode> lhs, shared_ptr<ExprNode> rhs);
    string class_name() { return "AssignNode"; }
};

class OpAssignNode : public AbstractAssignNode {
public:
    OpAssignNode(shared_ptr<ExprNode> lhs, const string& op, shared_ptr<ExprNode> rhs);
    string op() { return op_; }
    string class_name() { return "OpAssignNode"; }

protected:
    string op_;
};

class StmtNode : public Node {
public:
    StmtNode(const Location& loc);
    Location location() { return loc_; }

protected:
    Location loc_;
};

class BreakNode : public StmtNode {
public: 
    BreakNode(const Location& loc);
    string class_name() { return "BreakNode"; }

protected:
    void dump_node(Dumper& dumper) {}
};

class ContinueNode : public StmtNode {
public: 
    ContinueNode(const Location& loc);
    string class_name() { return "ContinueNode"; }

protected:
    void dump_node(Dumper& dumper) {}
};

class ReturnNode : public StmtNode {
public:
    ReturnNode(const Location& loc, shared_ptr<ExprNode> expr);
    ~ReturnNode();
    shared_ptr<ExprNode> expr() { return expr_; }

    void set_expr(shared_ptr<ExprNode> expr) { expr_ = expr; }
    string class_name() { return "ReturnNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> expr_;
};

class GotoNode : public StmtNode {
public:
    GotoNode(const Location& loc, const string& target);
    string target() { return target_; }
    string class_name() { return "GotoNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    string target_;
};

// TODO：
class BlockNode : public StmtNode {
public:
    BlockNode(const Location& loc, shared_ptr<vector<shared_ptr<DefinedVariable>>> vars, 
        shared_ptr<vector<shared_ptr<StmtNode>>> stmts);

    ~BlockNode();

    shared_ptr<vector<shared_ptr<DefinedVariable>>> variables() { return vars_; }
    shared_ptr<vector<shared_ptr<StmtNode>>> stmts() { return stmts_; }

    Location location() { return Location(); }
    string class_name() { return "BlockNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<vector<shared_ptr<DefinedVariable>>> vars_;
    shared_ptr<vector<shared_ptr<StmtNode>>> stmts_;
};

class ExprStmtNode : public StmtNode {
public:
    ExprStmtNode(const Location& loc, shared_ptr<ExprNode> expr);
    ~ExprStmtNode() {}
    
    shared_ptr<ExprNode> expr() { return expr_; }

    void set_expr(shared_ptr<ExprNode> expr) { expr_ = expr; }
   
    string class_name() { return "ExprStmtNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> expr_;
};

class LabelNode : public StmtNode {
public:
    LabelNode(const Location& loc, const string& name, shared_ptr<StmtNode> stmt);
    ~LabelNode() {}

    string name() { return name_; }
    shared_ptr<StmtNode> stmt() { return stmt_; }
    string class_name() { return "LabelNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    string name_;
    shared_ptr<StmtNode> stmt_;
};

class CaseNode : public StmtNode {
public:
    CaseNode(const Location& loc, 
        shared_ptr<vector<shared_ptr<ExprNode>>> values, shared_ptr<BlockNode> body);

    shared_ptr<vector<shared_ptr<ExprNode>>> values() { return values_; }
    shared_ptr<BlockNode> body() { return body_ ;}
    bool is_default(int n) { return !values_->at(n); }
    string class_name() { return "CaseNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    // TODO: Label
    shared_ptr<vector<shared_ptr<ExprNode>>> values_;
    shared_ptr<BlockNode> body_;
};

class SwitchNode : public StmtNode {
public:
    SwitchNode(const Location& loc, shared_ptr<ExprNode> cond, 
        shared_ptr<vector<shared_ptr<CaseNode>>> cases);

    shared_ptr<ExprNode> cond() { return cond_; }
    shared_ptr<vector<shared_ptr<CaseNode>>> cases() { return cases_; }

    string class_name() { return "SwitchNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> cond_;
    shared_ptr<vector<shared_ptr<CaseNode>>> cases_;
};

class ForNode : public StmtNode {
public:
    ForNode(const Location& loc, shared_ptr<ExprNode> init, shared_ptr<ExprNode> cond, shared_ptr<ExprNode> incr, shared_ptr<StmtNode> body);
    ~ForNode();

    shared_ptr<StmtNode> init() { return init_; }
    shared_ptr<ExprNode> cond() { return cond_; }
    shared_ptr<StmtNode> incr() { return incr_; }
    shared_ptr<StmtNode> body() { return body_; }

    string class_name() { return "ForNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<StmtNode> init_;
    shared_ptr<ExprNode> cond_;
    shared_ptr<StmtNode> incr_;
    shared_ptr<StmtNode> body_;
};

class DoWhileNode : public StmtNode {
public:
    DoWhileNode(const Location& loc, shared_ptr<StmtNode> body, shared_ptr<ExprNode> cond);
    ~DoWhileNode();

    shared_ptr<StmtNode> body() { return body_; }
    shared_ptr<ExprNode> cond() { return cond_; }
    string class_name() { return "DoWhileNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<StmtNode> body_;
    shared_ptr<ExprNode> cond_;
};

class WhileNode : public StmtNode {
public:
    WhileNode(const Location& loc, shared_ptr<ExprNode> cond, shared_ptr<StmtNode> body);
    ~WhileNode();

    shared_ptr<StmtNode> body() { return body_; }
    shared_ptr<ExprNode> cond() { return cond_; }
    string class_name() { return "DoWhileNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<StmtNode> body_;
    shared_ptr<ExprNode> cond_;
};

class IfNode : public StmtNode {
public:
    IfNode(const Location& loc, shared_ptr<ExprNode> c, shared_ptr<StmtNode> t, shared_ptr<StmtNode> e=nullptr);
    ~IfNode();

    shared_ptr<ExprNode> cond() { return cond_; }
    shared_ptr<StmtNode> then_body() { return then_body_; }
    shared_ptr<StmtNode> else_body() { return else_body_; }
    string class_name() { return "IfNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> cond_;
    shared_ptr<StmtNode> then_body_;
    shared_ptr<StmtNode> else_body_;
};

class TypeDefinition : public Node {
public:
    TypeDefinition(const Location& loc, shared_ptr<TypeRef> ref, const string& name);
    ~TypeDefinition();

    Location location() { return loc_; }
    string name() { return name_; }
    shared_ptr<TypeNode> type_node() { return tnode_; }
    shared_ptr<TypeRef> type_ref() { return tnode_->type_ref(); }
    shared_ptr<Type> type() { return tnode_->type(); }
    virtual shared_ptr<Type> defining_type() = 0;

protected:
    string name_;
    Location loc_;
    shared_ptr<TypeNode> tnode_;
};

class CompositeTypeDefinition : public TypeDefinition {
public:
    CompositeTypeDefinition(const Location &loc, shared_ptr<TypeRef> ref,
                            const string& name, shared_ptr<vector<shared_ptr<Slot>>> membs);

    ~CompositeTypeDefinition();

    bool is_compositeType() { return true; }
    virtual string kind() = 0;
    shared_ptr<vector<shared_ptr<Slot>>> members() { return members_; }

    void dump_node(Dumper& dumper);

protected:
    shared_ptr<vector<shared_ptr<Slot>>> members_;
};

class StructNode : public CompositeTypeDefinition {
public:
    StructNode(const Location &loc, shared_ptr<TypeRef> ref,
                const string& name, shared_ptr<vector<shared_ptr<Slot>>> membs);

    string kind() { return "struct"; }
    string class_name() { return "StructNode"; }
    bool is_struct() { return true; }
    shared_ptr<Type> defining_type();
};

class UnionNode : public CompositeTypeDefinition {
public:
    UnionNode(const Location &loc, shared_ptr<TypeRef> ref,
                const string& name, shared_ptr<vector<shared_ptr<Slot>>> membs);

    string kind() { return "union"; }
    string class_name() { return "UnionNode"; }
    bool is_union() { return true; }
    shared_ptr<Type> defining_type();
};

class TypedefNode : public TypeDefinition {
public:
    TypedefNode(const Location& loc, shared_ptr<TypeRef> ref, const string& name);
    bool is_user_type() { return true; }
    shared_ptr<TypeNode> real_type_node() { return real_; }
    shared_ptr<Type> real_type() { return real_->type(); }
    shared_ptr<TypeRef> real_type_ref() { return real_->type_ref(); }
    shared_ptr<Type> defining_type();
    string class_name() { return "TypedefNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    shared_ptr<TypeNode> real_;
};

} // namespace ast

#endif