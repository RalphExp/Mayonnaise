#include "node.h"

#include <memory>

namespace may {

void Node::dump(ostream& os) 
{
    Dumper dumper(os);
    dump(dumper);
}

void Node::dump(Dumper& dumper)
{
    dumper.print_class<Node>(this, Location());
    dump_node(dumper);
}

TypeNode::TypeNode(Type* tp)
    : type_(tp), ref_(nullptr)
{
}

TypeNode::TypeNode(TypeRef* ref)
    : type_(nullptr), ref_(ref)
{
}

TypeNode::TypeNode(Type* tp, TypeRef* ref) : type_(tp), ref_(ref)
{
}

Type* TypeNode::type()
{
    if (type_.get() == nullptr) {
        throw string("Type not resolved");
    }
    return type_.get();
}

TypeNode::~TypeNode()
{
}

void TypeNode::set_type(Type* tp)
{
    if (type_.get() != nullptr) {
        throw string("TypeNode::setType called twice");
    }
    type_.reset(tp);
}

void TypeNode::dump_node(Dumper& dumper)
{
    dumper.print_member("typeref", ref_.get());
    dumper.print_member("type", type());
}

bool ExprNode::is_callable()
{
    try {
        return type()->is_callable();
    } catch (...) {
        /* TODO: Semantic Error */
        return false;
    }
}

bool ExprNode::is_pointer()
{
    try {
        return type()->is_pointer();
    } catch (...) {
        return false;
    }
}

LiteralNode::LiteralNode(const Location& loc, TypeRef* ref) : 
    loc_(loc), tnode_(new TypeNode(ref))
{
}
    
LiteralNode::~LiteralNode()
{ 
}

IntegerLiteralNode::IntegerLiteralNode(const Location& loc, TypeRef* ref, long value) : 
    LiteralNode(loc, ref), value_(value)
{
}

void IntegerLiteralNode::dump_node(Dumper& dumper)
{
    dumper.print_member("typeNode", &tnode_);
    dumper.print_member("value", value_);
}

StringLiteralNode::StringLiteralNode(const Location& loc, 
        TypeRef* ref, const string& value) :
    LiteralNode(loc, ref), value_(value), entry_(nullptr)
{
}

void StringLiteralNode::dump_node(Dumper& dumper) 
{ 
    dumper.print_member("value", value_);
}

LHSNode::LHSNode() : type_(nullptr), orig_type_(nullptr)
{
}

LHSNode::~LHSNode()
{
}

bool LHSNode::is_loadable()
{
    Type* t = orig_type_.get();
    return !t->is_array() && !t->is_function();
}

VariableNode::VariableNode(const Location& loc, const string& name) : 
    loc_(loc), name_(name), entity_(nullptr)
{
}

VariableNode::~VariableNode()
{
    // TODO:
}

Entity* VariableNode::entity()
{
    if (entity_.get() == nullptr) {
        throw string("VariableNode.entity == null");
    }
    return entity_.get();
}

void VariableNode::set_entity(Entity* ent) {
    entity_.reset(ent);
}

bool VariableNode::is_lvalue()
{
    if (entity_->is_constant())
        return false;
    return true;
}

bool VariableNode::is_assignable()
{
    if (entity_->is_constant()) {
        return false;
    }
    return is_loadable();
}

bool VariableNode::is_parameter()
{
    return entity_->is_parameter();
}

Type* VariableNode::orig_type()
{
    return entity_->type();
}

TypeNode* VariableNode::type_node()
{
    return entity_->type_node();
}

UnaryOpNode::UnaryOpNode(const string& op, ExprNode* node) : 
    op_(op), expr_(node), op_type_(nullptr)
{
}

UnaryOpNode::~UnaryOpNode()
{
}

void UnaryOpNode::dump_node(Dumper& dumper)
{ 
    dumper.print_member("operator", op_);
    dumper.print_member("expr", expr_.get());
}

UnaryArithmeticOpNode::UnaryArithmeticOpNode(const string& op, ExprNode* node) : 
    UnaryOpNode(op, node), amount_(0)
{
}

SuffixOpNode::SuffixOpNode(const string& op, ExprNode* expr) : 
    UnaryArithmeticOpNode(op, expr)
{
}

ArefNode::ArefNode(ExprNode* expr, ExprNode* index) : 
    expr_(expr), index_(index)
{
}

bool ArefNode::is_multi_dimension()
{
    ArefNode* expr = dynamic_cast<ArefNode*>(expr_.get());
    return expr && !expr->orig_type()->is_pointer();
}

ArefNode::~ArefNode()
{
}

ExprNode* ArefNode::base_expr()
{
    return !is_multi_dimension() ? expr_.get() :
        ((ArefNode*)expr_.get())->base_expr();
}

Type* ArefNode::orig_type()
{
    return expr_->orig_type()->base_type();
}

long ArefNode::length()
{
    return ((ArrayType*)(expr_.get()->orig_type()))->length();
}

void ArefNode::dump_node(Dumper& dumper)
{
    if (type_ != nullptr) {
        dumper.print_member("type", type_.get());
    }
    dumper.print_member("expr", expr_.get());
    dumper.print_member("index", index_.get());
}

Slot::Slot() : tnode_(nullptr), offset_(Type::kSizeUnknown)
{
}


Slot::Slot(TypeNode* t, const string& n) : 
    tnode_(t), name_(n), offset_(Type::kSizeUnknown)
{
}

void Slot::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("typeNode", tnode_.get());
}

MemberNode::MemberNode(ExprNode* expr, const string& member) : 
    expr_(expr), member_(member)
{
}

CompositeType* MemberNode::base_type()
{
    return expr_->type()->get_composite_type();
}

void MemberNode::dump_node(Dumper& dumper)
{
    if (type_.get() != nullptr) {
        dumper.print_member("type", type_.get());
    }
    dumper.print_member("expr", expr_.get());
    dumper.print_member("member", member_);
}

PtrMemberNode::PtrMemberNode(ExprNode* expr, const string& member) : 
    expr_(expr), member_(member)
{
}

CompositeType* PtrMemberNode::derefered_composite_type()
{
    PointerType* pt = expr_->type()->get_pointer_type();
    return pt->base_type()->get_composite_type();
}

Type* PtrMemberNode::derefered_type()
{
    PointerType* pt = expr_->type()->get_pointer_type();
    return pt->base_type();
}

void PtrMemberNode::dump_node(Dumper& dumper)
{
    if (type_.get() != nullptr) {
        dumper.print_member("type", type_.get());
    }

    dumper.print_member("expr", expr_.get());
    dumper.print_member("member", member_);
}
    
FuncallNode::FuncallNode(ExprNode* expr, vector<ExprNode*>* args) : 
    expr_(expr), args_(args)
{
}

/* TODO: */
Type* FuncallNode::type()
{
    return nullptr;
}

void FuncallNode::dump_node(Dumper &dumper)
{
    dumper.print_member("expr", expr_.get());
    dumper.print_node_list("args", *args_.get());
}

FuncallNode::~FuncallNode()
{
    for (ExprNode* expr : *args_.get()) {
        delete expr;
    }
}

SizeofExprNode::SizeofExprNode(ExprNode* expr, TypeRef* ref) :
    expr_(expr), tnode_(new TypeNode(ref))
{
}

void SizeofExprNode::dump_node(Dumper& dumper)
{
    dumper.print_member("expr", expr_.get());
}

SizeofTypeNode::SizeofTypeNode(TypeNode* operand, TypeRef* ref) :
    op_(operand), tnode_(new TypeNode(ref))
{
}

void SizeofTypeNode::dump_node(Dumper& dumper)
{
    dumper.print_member("operand", op_);
}

AddressNode::AddressNode(ExprNode* expr) : expr_(expr)
{
}
    
Type* AddressNode::type()
{
    if (type_ == nullptr) 
        throw string("type is null");
    
    return type_;
}
    
void AddressNode::set_type(Type* type)
{
    if (type_ != nullptr) 
        throw string("type set twice");
    
    type_ = type;
}
    
void AddressNode::dump_node(Dumper& dumper)
{
    if (type_ != nullptr) {
        dumper.print_member("type", type_);
    }
        
    dumper.print_member("expr", expr_);
}

DereferenceNode::DereferenceNode(ExprNode* expr)
    : expr_(expr)
{
}
    
Type* DereferenceNode::orig_type()
{
    return expr_->type()->base_type();
}

void DereferenceNode::dump_node(Dumper& dumper)
{
    if (type_.get() != nullptr) {
        dumper.print_member("type", type_.get());
    }

    dumper.print_member("expr", expr_.get());
}

PrefixOpNode::PrefixOpNode(const string& op, ExprNode* expr) : 
    UnaryArithmeticOpNode(op, expr)
{
}

CastNode::CastNode(Type* t, ExprNode* expr) : 
    tnode_(new TypeNode(t)), expr_(expr)
{
}

CastNode::CastNode(TypeNode* t, ExprNode* expr) : 
    tnode_(t), expr_(expr)
{
}

void CastNode::dump_node(Dumper& dumper)
{
    dumper.print_member("typeNode", tnode_);
    dumper.print_member("expr", expr_);
}

CastNode::~CastNode()
{
    delete tnode_;
    delete expr_;
}

BinaryOpNode::BinaryOpNode(ExprNode* left, const string& op, ExprNode* right) : 
    type_(nullptr), left_(left), op_(op), right_(right)
{
}

BinaryOpNode::BinaryOpNode(Type* t, ExprNode* left, const string& op, ExprNode* right) : 
    type_(t), left_(left), op_(op), right_(right)
{
}

BinaryOpNode::~BinaryOpNode()
{
    delete left_;
    delete right_;
}

void BinaryOpNode::set_type(Type* type) 
{
    if (type_ != nullptr)
        throw string("BinaryOp::set_type called twice");
    type_ = type;
}

void BinaryOpNode::dump_node(Dumper& dumper) 
{
    dumper.print_member("operator", op_);
    dumper.print_member("left", left_);
    dumper.print_member("right", right_);
}

LogicalAndNode::LogicalAndNode(ExprNode* left, ExprNode* right) : 
    BinaryOpNode(left, "&&", right)
{
}

LogicalOrNode::LogicalOrNode(ExprNode* left, ExprNode* right) : 
    BinaryOpNode(left, "&&", right)
{
}

CondExprNode::CondExprNode(ExprNode* c, ExprNode* t, ExprNode* e) :
    cond_(c), then_expr_(t), else_expr_(e)
{
}

CondExprNode::~CondExprNode()
{
    delete cond_;
    delete then_expr_;
    delete else_expr_;
}

void CondExprNode::dump_node(Dumper& dumper) 
{
    dumper.print_member("cond", cond_);
    dumper.print_member("then_expr", then_expr_);
    dumper.print_member("else_expr", else_expr_);
}

AbstractAssignNode::AbstractAssignNode(ExprNode* lhs, ExprNode* rhs) : 
    lhs_(lhs), rhs_(rhs)
{
}

void AbstractAssignNode::dump_node(Dumper& dumper) 
{
    dumper.print_member("lhs", lhs_);
    dumper.print_member("rhs", rhs_);
}

AbstractAssignNode::~AbstractAssignNode()
{
    delete lhs_;
    delete rhs_;
}

AssignNode::AssignNode(ExprNode* lhs, ExprNode* rhs) :
    AbstractAssignNode(lhs, rhs)
{
}

OpAssignNode::OpAssignNode(ExprNode* lhs, const string& op, ExprNode* rhs) : 
    AbstractAssignNode(lhs, rhs), op_(op)
{
}

StmtNode::StmtNode(const Location& loc) : loc_(loc)
{   
}

BreakNode::BreakNode(const Location& loc) : StmtNode(loc)
{
}

ContinueNode::ContinueNode(const Location& loc) : StmtNode(loc)
{
}

ReturnNode::ReturnNode(const Location& loc, ExprNode* expr) : 
    StmtNode(loc), expr_(expr)
{
}

ReturnNode::~ReturnNode()
{
    delete expr_;
}

void ReturnNode::dump_node(Dumper& dumper)
{
    dumper.print_member("expr", expr_);
}

GotoNode::GotoNode(const Location& loc, const string& target) : 
    StmtNode(loc), target_(target)
{
}

void GotoNode::dump_node(Dumper& dumper)
{
    dumper.print_member("target", target_);
}
    
BlockNode::BlockNode(const Location& loc, 
        vector<DefinedVariable*>* vars, 
        vector<StmtNode*>* stmts) : 
    StmtNode(loc), vars_(vars), stmts_(stmts)
{
}

BlockNode::~BlockNode()
{
    for (DefinedVariable* var : *vars_) {
        delete var;
    }
    delete vars_;

    for (StmtNode* stmt : *stmts_) {
        delete stmt;
    }
    delete stmts_;
}

void BlockNode::dump_node(Dumper& dumper) 
{
    dumper.print_node_list("variables", *vars_);
    dumper.print_node_list("stmts", *stmts_);
}

ExprStmtNode::ExprStmtNode(const Location& loc, ExprNode* expr)
    : StmtNode(loc), expr_(expr)
{
}

void ExprStmtNode::dump_node(Dumper& dumper)
{
    dumper.print_member("expr", expr_);
}

LabelNode::LabelNode(const Location& loc, const string& name, StmtNode* stmt) : 
    StmtNode(loc), name_(name), stmt_(stmt)
{
}

void LabelNode::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("stmt", stmt_);
}

CaseNode::CaseNode(const Location& loc, 
        vector<ExprNode*>* values, BlockNode* body) : 
    StmtNode(loc), values_(values), body_(body)
{
}

void CaseNode::dump_node(Dumper& dumper)
{
    dumper.print_node_list("values", *values_);
    dumper.print_member("body", body_);
}
    
SwitchNode::SwitchNode(const Location& loc, ExprNode* cond, 
        vector<CaseNode*>* cases) : 
    StmtNode(loc), cond_(cond), cases_(cases)
{
}

void SwitchNode::dump_node(Dumper& dumper)
{
    dumper.print_member("cond", cond_);
    dumper.print_node_list("cases", *cases_);
}

ForNode::ForNode(const Location& loc, ExprNode* init, 
        ExprNode* cond, ExprNode* incr, StmtNode* body) : 
    StmtNode(loc), body_(body)
{
    if (init) {
        init_.reset(new ExprStmtNode(init->location(), init));
    } else {
        init_ = nullptr;
    }

    if (cond) {
        cond_.reset(cond);
    } else {
        /* default to be true(1) */
        cond_.reset(new IntegerLiteralNode(Location(), IntegerTypeRef::int_ref(), 1));
    }

    if (incr) {
        incr_.reset(new ExprStmtNode(incr->location(), incr));
    } else {
        incr_ = nullptr;
    }
}

ForNode::~ForNode()
{
}

void ForNode::dump_node(Dumper& dumper)
{
    dumper.print_member("init", init_.get());
    dumper.print_member("cond", cond_.get());
    dumper.print_member("incr", incr_.get());
    dumper.print_member("body", body_.get());
}

DoWhileNode::DoWhileNode(const Location& loc, StmtNode* body, ExprNode* cond) : 
    StmtNode(loc), body_(body), cond_(cond)
{
}

DoWhileNode::~DoWhileNode()
{
}

void DoWhileNode::dump_node(Dumper& dumper)
{
    dumper.print_member("body", body_.get());
    dumper.print_member("cond", cond_.get());
}

WhileNode::WhileNode(const Location& loc, ExprNode* cond, StmtNode* body) : 
    StmtNode(loc), cond_(cond), body_(body)
{
}

WhileNode::~WhileNode()
{
}

void WhileNode::dump_node(Dumper& dumper)
{
    dumper.print_member("cond", cond_.get());
    dumper.print_member("body", body_.get());
}

IfNode::IfNode(const Location& loc, ExprNode* c, StmtNode* t, StmtNode* e) : 
    StmtNode(loc), cond_(c), then_body_(t), else_body_(e)
{
}

IfNode::~IfNode()
{
}

void IfNode::dump_node(Dumper& dumper)
{
    dumper.print_member("cond", cond_.get());
    dumper.print_member("then_body", then_body_.get());
    dumper.print_member("else_body", else_body_.get());
}

TypeDefinition::TypeDefinition(const Location& loc, TypeRef* ref, const string& name) :
    loc_(loc), tnode_(new TypeNode(ref)), name_(name)
{
}

TypeDefinition::~TypeDefinition()
{
}

CompositeTypeDefinition::CompositeTypeDefinition(const Location &loc, TypeRef* ref,
        const string& name, vector<Slot*>* membs) :
    TypeDefinition(loc, ref, name), members_(membs)
{
}

CompositeTypeDefinition::~CompositeTypeDefinition()
{
    for (Slot* slot : *members_.get())
        delete slot;
}

void CompositeTypeDefinition::dump_node(Dumper& dumper) 
{
    dumper.print_member("name", name_);
    dumper.print_node_list("members", *members_);
}

StructNode::StructNode(const Location &loc, TypeRef* ref,
        const string& name, vector<Slot*>* membs):
    CompositeTypeDefinition(loc, ref, name, membs)
{
}

shared_ptr<Type> StructNode::defining_type()
{
    return shared_ptr<Type>(
        new StructType(name(), members_, location()));
}

UnionNode::UnionNode(const Location &loc, TypeRef* ref,
        const string& name, vector<Slot*>* membs):
    CompositeTypeDefinition(loc, ref, name, membs)
{
}

shared_ptr<Type> UnionNode::defining_type()
{
    return shared_ptr<Type>(
        new UnionType(name(), members(), location()));
}

TypedefNode::TypedefNode(const Location& loc, TypeRef* real, const string& name) :
    TypeDefinition(loc, new UserTypeRef(name), name), real_(new TypeNode(real))
{
}

shared_ptr<Type> TypedefNode::defining_type()
{
    return shared_ptr<Type>(
        new UserType(name_, real_, loc_));
}

void TypedefNode::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("typeNode", real_.get());
}

} // namespace may