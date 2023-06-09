#ifndef AST_NODE_H_
#define AST_NODE_H_

#include <cassert>
#include <iostream>

#include "object.h"
#include "entity.h"
#include "util.h"
#include "type.h"

using namespace std;

namespace cbc {

class Node : public Object, public Dumpable {
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
    TypeNode(Type* tp);
    TypeNode(TypeRef* ref);
    TypeNode(Type* tp, TypeRef* ref);
    ~TypeNode();
    Location location() { return ref_->location(); }
    Type* type();
    TypeRef* type_ref() { return ref_; }  
    bool is_resolved() { return !!type_; } 
    void set_type(Type* tp);
    void dump_node(Dumper& dumper);
    string class_name() { return "TypeNode"; }

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
    string class_name() { return "LiteralNode"; }

protected:
    Location loc_;
    TypeNode* tnode_;
};

class IntegerLiteralNode : public LiteralNode {
public:
    IntegerLiteralNode(const Location& loc, TypeRef* ref, long value);
    long value() { return value_; }
    string class_name() { return "IntegerLiteralNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    long value_;
};

class StringLiteralNode : public LiteralNode {
public:
    StringLiteralNode(const Location& loc, TypeRef* ref, const string& value);
    string value() { return value_; }
    string class_name() { return "StringLiteralNode"; }
    ConstantEntry* entry() { return entry_; }
    
protected:
    void dump_node(Dumper& dumper);

protected:
    string value_;
    ConstantEntry* entry_;
};

/* Only 5 kinds of Nodes inherit LHSNode:
 *   VariableNode
 *   DereferenceNode
 *   MemberNode
 *   PtrMemberNode
 *   ArefNode
 */
class LHSNode : public ExprNode {
public:
    LHSNode();
    ~LHSNode();
    Type* type() { return type_ != nullptr ? type_ : orig_type(); }
    void set_type(Type* type);
    long alloc_size() { return orig_type()->alloc_size(); }
    bool is_lvalue() { return true; }
    bool is_assignable() { return is_loadable(); }
    bool is_loadable();
    string class_name() { return "LHSNode"; }

protected:
    Type* orig_type() { return orig_type_; }

protected:
    Type* type_;
    Type* orig_type_;
};

class VariableNode : public LHSNode {
public:
    VariableNode(const Location& loc, const string& name);
    VariableNode(DefinedVariable* var);
    ~VariableNode();
    string name() { return name_; }
    Location location() { return loc_; };

    // Entity represents variable, constant, ... etc
    // something in memory
    Entity* entity();
    void set_entity(Entity* ent);
    bool is_resolved() { return entity_ != nullptr; }
    bool is_lvalue();
    bool is_assignable();
    bool is_parameter();
    Type* orig_type();
    TypeNode* type_node();
    string class_name() { return "VariableNode"; }
    
protected:
    void dump_node(Dumper& dumper);

protected:
    Location loc_;
    string name_;
    Entity* entity_;
};

class UnaryOpNode : public ExprNode {
public:
    UnaryOpNode(const string& op, ExprNode* node);
    ~UnaryOpNode();
    string op() { return op_; }
    Type* type() { return expr_->type(); }
    ExprNode* expr() { return expr_; }
    Location location() { return expr_->location(); }
    void set_op_type(Type* type);
    void set_expr(ExprNode* expr);
    string class_name() { return "UnaryOpNode"; }

protected:
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
    void set_amount(long amount) { amount_ = amount; }
    string class_name() { return "UnaryArithmeticOpNode"; }

protected:
    long amount_;
};

class PrefixOpNode : public UnaryArithmeticOpNode {
public:
    PrefixOpNode(const string& op, ExprNode* expr);
    string class_name() { return "PrefixOpNode"; }
};

class SuffixOpNode : public UnaryArithmeticOpNode {
public:
    SuffixOpNode(const string& op, ExprNode* expr);
    string class_name() { return "SuffixOpNode"; }
};

// Array Reference Node
class ArefNode : public LHSNode {
public:
    ArefNode(ExprNode* expr, ExprNode* index);
    ~ArefNode();

    // e.g. expr of a[x][y] is a[x], i.e. ArefNode(a, x)
    ExprNode* expr() { return expr_; }
    ExprNode* index() { return index_; }

    // Returns base expression of (multi-dimension) array.
    // e.g.  baseExpr of a[x][y][z] is a.
    ExprNode* base_expr();
    Type* orig_type();

    // isMultiDimension a[x][y][z] = true.
    // isMultiDimension a[x][y] = true.
    // isMultiDimension a[x] = false.
    bool is_multi_dimension();
    long element_size() { return orig_type()->alloc_size(); }
    long length();
    Location location() { return expr_->location(); }
    string class_name() { return "ArefNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
    ExprNode* index_;
};

class Slot : public Node {
public:
    Slot();
    Slot(TypeNode* t, const string& n);
    ~Slot();

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
    string class_name() { return "Slot"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    string name_;
    TypeNode* tnode_;
    long offset_;
};

class MemberNode : public LHSNode {
public:
    MemberNode(ExprNode* expr, const string& member);
    ~MemberNode();
    CompositeType* base_type();
    Location location() { return expr()->location(); }
    ExprNode* expr() { return expr_; }
    string member() { return member_; }
    long offset() { return base_type()->member_offset(member_); }
    string class_name() { return "MemberNode"; }

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
    ~PtrMemberNode();
    CompositeType* derefered_composite_type();
    Type* derefered_type();
    ExprNode* expr() { return expr_; }
    string member() { return member_; }
    long offset() { return derefered_composite_type()->member_offset(member_); }
    Location location() { return expr_->location(); }
    string class_name() { return "PtrMemberNode"; }
    
protected:
    Type* orig_type() { return derefered_composite_type()->member_type(member_); }
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
    string member_;
};

/* TODO: */
class FuncallNode : public ExprNode {
public:
    FuncallNode(ExprNode* expr, vector<ExprNode*>&& args);
    ~FuncallNode();

    ExprNode* expr() { return expr_; }
    vector<ExprNode*> args() { return args_; }
    void replaceArgs(vector<ExprNode*>&& args);

    Location location() { return expr_->location(); }
    long num_args() { return args_.size(); }
    
    Type* type();
    FunctionType* function_type();

    string class_name() { return "FuncallNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
    vector<ExprNode*> args_;
};

class SizeofExprNode : public ExprNode {
public:
    SizeofExprNode(ExprNode* expr, TypeRef* ref);
    ~SizeofExprNode();
    ExprNode* expr() { return expr_; }
    void set_expr(ExprNode* expr);
    Type* type() { return tnode_->type(); }
    TypeNode* typeNode() { return tnode_; }
    Location location() { return expr_->location(); }
    string class_name() { return "SizeofExprNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
    TypeNode* tnode_;
};

class SizeofTypeNode : public ExprNode {
public:
    SizeofTypeNode(TypeNode* operand, TypeRef* type);
    ~SizeofTypeNode();
    Type* operand() { return op_->type(); }
    Type* type() { return tnode_->type(); }
    TypeNode* operand_type_node() { return op_; }
    TypeNode* type_node() { return tnode_; }
    Location location() { return op_->location(); }
    string class_name() { return "SizeofTypeNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    TypeNode* op_;
    TypeNode* tnode_;
};

class AddressNode : public ExprNode {
public:
    AddressNode(ExprNode* expr);
    ~AddressNode();
    ExprNode* expr() { return expr_; }
    Type* type();
    void set_type(Type* type);
    Location location() { return expr_->location(); }
    string class_name() { return "AddressNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
    Type* type_;
};

class DereferenceNode : public LHSNode {
public:
    DereferenceNode(ExprNode* expr);
    ~DereferenceNode();
    Type* orig_type();
    ExprNode* expr() { return expr_; }
    void set_expr(ExprNode* expr);
    Location location() { return expr_->location(); }
    string class_name() { return "DereferenceNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
};

class CastNode : public ExprNode {
public:
    CastNode(Type* t, ExprNode* expr);
    CastNode(TypeNode* t, ExprNode* expr);
    ~CastNode();
    Type* type() { return tnode_->type(); }
    TypeNode* typde_node() { return tnode_; }
    ExprNode* expr() { return expr_; }
    bool is_lvalue() { return expr_->is_lvalue(); }
    bool is_assignable() { return expr_->is_assignable(); }
    bool is_effectiveCast() { return type()->size() > expr_->type()->size(); }
    Location location() { return tnode_->location(); }
    string class_name() { return "CastNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    TypeNode* tnode_;
    ExprNode* expr_;
};

class BinaryOpNode : public ExprNode {
public:
    BinaryOpNode(ExprNode* left, const string& op, ExprNode* right);
    BinaryOpNode(Type* t, ExprNode* left, const string& op, ExprNode* right);
    ~BinaryOpNode();
    string op() { return op_; }
    Type* type() { return type_ ? type_ : left_->type(); }
    void set_type(Type* type);
    ExprNode* left() { return left_; }
    ExprNode* right() { return right_; }
    void set_left(ExprNode* l) { left_ = l; }
    void set_right(ExprNode* r) { right_ = r; }

    Location location() { return left_->location(); }
    string class_name() { return "BinaryOpNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    ExprNode* left_;
    ExprNode* right_;
    Type* type_;
    string op_;
};

class CondExprNode : public ExprNode {
public:
    CondExprNode(ExprNode* c, ExprNode* t, ExprNode* e);
    ~CondExprNode();
    Type* type() { return then_expr_->type(); }

    ExprNode* cond() { return cond_ ;}
    ExprNode* then_expr() { return then_expr_; }
    ExprNode* else_expr() { return else_expr_; }

    void set_then_expr(ExprNode* expr);
    void set_else_expr(ExprNode* expr);

    Location location() { return cond_->location(); }
    string class_name() { return "CondExprNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    ExprNode* cond_;
    ExprNode* then_expr_;
    ExprNode* else_expr_;
};

class LogicalOrNode : public BinaryOpNode {
public:
    LogicalOrNode(ExprNode* left, ExprNode* right);
    string class_name() { return "LogicalOrNode"; }
};

class LogicalAndNode : public BinaryOpNode {
public:
    LogicalAndNode(ExprNode* left, ExprNode* right);
    string class_name() { return "LogicalAndNode"; }
};

class AbstractAssignNode : public ExprNode {
public:
    AbstractAssignNode(ExprNode* lhs, ExprNode* rhs);
    ~AbstractAssignNode();

    Type* type() { return lhs_->type(); }
    ExprNode* lhs() { return lhs_; }
    ExprNode* rhs() { return rhs_; }
    
    void set_rhs(ExprNode* expr);

    Location location() { return lhs_->location(); }
    string class_name() { return "AbstractAssignNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    ExprNode* lhs_;
    ExprNode* rhs_;
};

class AssignNode : public AbstractAssignNode {
public:
    AssignNode(ExprNode* lhs, ExprNode* rhs);
    string class_name() { return "AssignNode"; }
};

class OpAssignNode : public AbstractAssignNode {
public:
    OpAssignNode(ExprNode* lhs, const string& op, ExprNode* rhs);
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
    ReturnNode(const Location& loc, ExprNode* expr);
    ~ReturnNode();
    
    ExprNode* expr() { return expr_; }
    void set_expr(ExprNode* expr);

    string class_name() { return "ReturnNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
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
    BlockNode(const Location& loc, vector<DefinedVariable*>&& vars,
        vector<StmtNode*>&& stmts);

    ~BlockNode();

    vector<DefinedVariable*> variables() { return vars_; }
    vector<StmtNode*> stmts() { return stmts_; }
    StmtNode* tail_stmt();

    string class_name() { return "BlockNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    vector<DefinedVariable*> vars_;
    vector<StmtNode*> stmts_;
};

class ExprStmtNode : public StmtNode {
public:
    ExprStmtNode(const Location& loc, ExprNode* expr);
    ~ExprStmtNode();
    
    ExprNode* expr() { return expr_; }
    void set_expr(ExprNode* expr);
    string class_name() { return "ExprStmtNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    ExprNode* expr_;
};

class LabelNode : public StmtNode {
public:
    LabelNode(const Location& loc, const string& name, StmtNode* stmt);
    ~LabelNode();

    string name() { return name_; }
    StmtNode* stmt() { return stmt_; }
    string class_name() { return "LabelNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    string name_;
    StmtNode* stmt_;
};

class CaseNode : public StmtNode {
public:
    CaseNode(const Location& loc, vector<ExprNode*>&& values, BlockNode* body);
    ~CaseNode();

    vector<ExprNode*> values() { return values_; }
    BlockNode* body() { return body_; }

    bool is_default(int n) { return values_.at(n) == nullptr; }
    string class_name() { return "CaseNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    // TODO: Label
    vector<ExprNode*> values_;
    BlockNode* body_;
};

class SwitchNode : public StmtNode {
public:
    SwitchNode(const Location& loc, ExprNode* cond, 
        vector<CaseNode*>&& cases);
    ~SwitchNode();

    ExprNode* cond() { return cond_; }
    vector<CaseNode*> cases() { return cases_; }

    string class_name() { return "SwitchNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    ExprNode* cond_;
    vector<CaseNode*> cases_;
};

class ForNode : public StmtNode {
public:
    ForNode(const Location& loc, ExprNode* init, 
        ExprNode* cond, ExprNode* incr, StmtNode* body);
    
    ~ForNode();

    StmtNode* init() { return init_; }
    ExprNode* cond() { return cond_; }
    StmtNode* incr() { return incr_; }
    StmtNode* body() { return body_; }

    string class_name() { return "ForNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    StmtNode* init_;
    ExprNode* cond_;
    StmtNode* incr_;
    StmtNode* body_;
};

class DoWhileNode : public StmtNode {
public:
    DoWhileNode(const Location& loc, StmtNode* body, ExprNode* cond);
    ~DoWhileNode();

    StmtNode* body() { return body_; }
    ExprNode* cond() { return cond_; }
    string class_name() { return "DoWhileNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    StmtNode* body_;
    ExprNode* cond_;
};

class WhileNode : public StmtNode {
public:
    WhileNode(const Location& loc, ExprNode* cond, StmtNode* body);
    ~WhileNode();

    StmtNode* body() { return body_; }
    ExprNode* cond() { return cond_; }
    string class_name() { return "DoWhileNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    StmtNode* body_;
    ExprNode* cond_;
};

class IfNode : public StmtNode {
public:
    IfNode(const Location& loc, ExprNode* c, 
        StmtNode* t, StmtNode* e=nullptr);
    ~IfNode();

    ExprNode* cond() { return cond_; }
    StmtNode* then_body() { return then_body_; }
    StmtNode* else_body() { return else_body_; }
    string class_name() { return "IfNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    ExprNode* cond_;
    StmtNode* then_body_;
    StmtNode* else_body_;
};

class TypeDefinition : public Node {
public:
    TypeDefinition(const Location& loc, TypeRef* ref, const string& name);
    ~TypeDefinition();

    Location location() { return loc_; }
    string name() { return name_; }
    TypeNode* type_node() { return tnode_; }
    TypeRef* type_ref() { return tnode_->type_ref(); }
    Type* type() { return tnode_->type(); }
    virtual Type* defining_type() = 0;

protected:
    string name_;
    Location loc_;
    TypeNode* tnode_;
};

class CompositeTypeDefinition : public TypeDefinition {
public:
    CompositeTypeDefinition(const Location &loc, TypeRef* ref,
                            const string& name, vector<Slot*>&& membs);
    ~CompositeTypeDefinition();

    virtual string kind() = 0;

    bool is_compositeType() { return true; }
    vector<Slot*> members() { return members_; }
    void dump_node(Dumper& dumper);

public:
    vector<Slot*> members_;
};

class StructNode : public CompositeTypeDefinition {
public:
    StructNode(const Location &loc, TypeRef* ref,
                const string& name, vector<Slot*>&& membs);

    string kind() { return "struct"; }
    string class_name() { return "StructNode"; }
    bool is_struct() { return true; }
    Type* defining_type();
};

class UnionNode : public CompositeTypeDefinition {
public:
    UnionNode(const Location &loc, TypeRef* ref,
                const string& name, vector<Slot*>&& membs);

    string kind() { return "union"; }
    string class_name() { return "UnionNode"; }
    bool is_union() { return true; }
    Type* defining_type();
};

class TypedefNode : public TypeDefinition {
public:
    TypedefNode(const Location& loc, TypeRef* ref, const string& name);
    ~TypedefNode();
    bool is_user_type() { return true; }
    TypeNode* real_type_node() { return real_; }
    Type* real_type() { return real_->type(); }
    TypeRef* real_type_ref() { return real_->type_ref(); }
    Type* defining_type();
    string class_name() { return "TypedefNode"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    TypeNode* real_;
};

} // namespace cbc

#endif
