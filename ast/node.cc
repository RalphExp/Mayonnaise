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

TypeNode::TypeNode(shared_ptr<Type> tp)
    : type_(tp), ref_(nullptr)
{
}

TypeNode::TypeNode(shared_ptr<TypeRef> ref)
    : type_(nullptr), ref_(ref)
{
}

TypeNode::TypeNode(shared_ptr<Type> tp, shared_ptr<TypeRef> ref) : type_(tp), ref_(ref)
{
}

shared_ptr<Type> TypeNode::type()
{
    if (!type_) {
        throw string("Type not resolved");
    }
    return type_;
}

TypeNode::~TypeNode()
{
}

void TypeNode::set_type(shared_ptr<Type> tp)
{
    if (type_) {
        throw string("TypeNode::setType called twice");
    }
    type_ = tp;
}

void TypeNode::dump_node(Dumper& dumper)
{
    dumper.print_member("typeref", ref_);
    dumper.print_member("type", type_);
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

LiteralNode::LiteralNode(const Location& loc, shared_ptr<TypeRef> ref) : 
    loc_(loc), tnode_(new TypeNode(ref))
{
}
    
LiteralNode::~LiteralNode()
{ 
}

IntegerLiteralNode::IntegerLiteralNode(const Location& loc, shared_ptr<TypeRef> ref, long value) : 
    LiteralNode(loc, ref), value_(value)
{
}

void IntegerLiteralNode::dump_node(Dumper& dumper)
{
    dumper.print_member("typeNode", &tnode_);
    dumper.print_member("value", value_);
}

StringLiteralNode::StringLiteralNode(const Location& loc, 
        shared_ptr<TypeRef> ref, const string& value) :
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
    shared_ptr<Type> t = orig_type_;
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

shared_ptr<Entity> VariableNode::entity()
{
    if (!entity_) {
        throw string("VariableNode.entity == null");
    }
    return entity_;
}

void VariableNode::set_entity(shared_ptr<Entity> ent) 
{
    entity_ = ent;
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

shared_ptr<Type> VariableNode::orig_type()
{
    return entity_->type();
}

shared_ptr<TypeNode> VariableNode::type_node()
{
    return entity_->type_node();
}

UnaryOpNode::UnaryOpNode(const string& op, shared_ptr<ExprNode> node) : 
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

UnaryArithmeticOpNode::UnaryArithmeticOpNode(const string& op, shared_ptr<ExprNode> node) : 
    UnaryOpNode(op, node), amount_(0)
{
}

SuffixOpNode::SuffixOpNode(const string& op, shared_ptr<ExprNode> expr) : 
    UnaryArithmeticOpNode(op, expr)
{
}

ArefNode::ArefNode(shared_ptr<ExprNode> expr, shared_ptr<ExprNode> index) : 
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

shared_ptr<ExprNode> ArefNode::base_expr()
{
    return !is_multi_dimension() ? expr_ :
        ((ArefNode*)expr_.get())->base_expr();
}

shared_ptr<Type> ArefNode::orig_type()
{
    return expr_->orig_type()->base_type();
}

long ArefNode::length()
{
    return ((ArrayType*)(expr_->orig_type().get()))->length();
}

void ArefNode::dump_node(Dumper& dumper)
{
    if (type_ != nullptr) {
        dumper.print_member("type", type_);
    }
    dumper.print_member("expr", expr_);
    dumper.print_member("index", index_);
}

Slot::Slot() : tnode_(nullptr), offset_(Type::kSizeUnknown)
{
}


Slot::Slot(shared_ptr<TypeNode> t, const string& n) : 
    tnode_(t), name_(n), offset_(Type::kSizeUnknown)
{
}

void Slot::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("typeNode", tnode_);
}

MemberNode::MemberNode(shared_ptr<ExprNode> expr, const string& member) : 
    expr_(expr), member_(member)
{
}

CompositeType* MemberNode::base_type()
{
    return expr_->type()->get_composite_type();
}

void MemberNode::dump_node(Dumper& dumper)
{
    if (type_) {
        dumper.print_member("type", type_);
    }
    dumper.print_member("expr", expr_);
    dumper.print_member("member", member_);
}

PtrMemberNode::PtrMemberNode(shared_ptr<ExprNode> expr, const string& member) : 
    expr_(expr), member_(member)
{
}

CompositeType* PtrMemberNode::derefered_composite_type()
{
    PointerType* pt = expr_->type()->get_pointer_type();
    return pt->base_type()->get_composite_type();
}

shared_ptr<Type> PtrMemberNode::derefered_type()
{
    PointerType* pt = expr_->type()->get_pointer_type();
    return pt->base_type();
}

void PtrMemberNode::dump_node(Dumper& dumper)
{
    if (type_) {
        dumper.print_member("type", type_);
    }

    dumper.print_member("expr", expr_);
    dumper.print_member("member", member_);
}
    
FuncallNode::FuncallNode(shared_ptr<ExprNode> expr, 
        shared_ptr<vector<shared_ptr<ExprNode>>> args) : 
    expr_(expr), args_(args)
{
}

/* TODO: */
shared_ptr<Type> FuncallNode::type()
{
    return nullptr;
}

void FuncallNode::dump_node(Dumper &dumper)
{
    dumper.print_member("expr", expr_);
    dumper.print_node_list("args", args_);
}

FuncallNode::~FuncallNode()
{
}

SizeofExprNode::SizeofExprNode(shared_ptr<ExprNode> expr, shared_ptr<TypeRef> ref) :
    expr_(expr), tnode_(new TypeNode(ref))
{
}

void SizeofExprNode::dump_node(Dumper& dumper)
{
    dumper.print_member("expr", expr_);
}

SizeofTypeNode::SizeofTypeNode(shared_ptr<TypeNode> operand, shared_ptr<TypeRef> ref) :
    op_(operand), tnode_(new TypeNode(ref))
{
}

void SizeofTypeNode::dump_node(Dumper& dumper)
{
    dumper.print_member("operand", op_);
}

AddressNode::AddressNode(shared_ptr<ExprNode> expr) : expr_(expr)
{
}
    
shared_ptr<Type> AddressNode::type()
{
    if (!type_) 
        throw string("type is null");
    
    return type_;
}
    
void AddressNode::set_type(shared_ptr<Type> type)
{
    if (type_) 
        throw string("type set twice");
    
    type_ = type;
}
    
void AddressNode::dump_node(Dumper& dumper)
{
    if (type_) {
        dumper.print_member("type", type_);
    }
        
    dumper.print_member("expr", expr_);
}

DereferenceNode::DereferenceNode(shared_ptr<ExprNode> expr)
    : expr_(expr)
{
}
    
shared_ptr<Type> DereferenceNode::orig_type()
{
    return expr_->type()->base_type();
}

void DereferenceNode::dump_node(Dumper& dumper)
{
    if (type_) {
        dumper.print_member("type", type_);
    }

    dumper.print_member("expr", expr_);
}

PrefixOpNode::PrefixOpNode(const string& op, shared_ptr<ExprNode> expr) : 
    UnaryArithmeticOpNode(op, expr)
{
}

CastNode::CastNode(shared_ptr<Type> t, shared_ptr<ExprNode> expr) : 
    tnode_(new TypeNode(t)), expr_(expr)
{
}

CastNode::CastNode(shared_ptr<TypeNode> t, shared_ptr<ExprNode> expr) : 
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
}

BinaryOpNode::BinaryOpNode(shared_ptr<ExprNode> left, const string& op, shared_ptr<ExprNode> right) : 
    type_(nullptr), left_(left), op_(op), right_(right)
{
}

BinaryOpNode::BinaryOpNode(shared_ptr<Type> t, shared_ptr<ExprNode> left, const string& op, shared_ptr<ExprNode> right) : 
    type_(t), left_(left), op_(op), right_(right)
{
}

BinaryOpNode::~BinaryOpNode()
{
}

void BinaryOpNode::set_type(shared_ptr<Type> type) 
{
    if (!type_)
        throw string("BinaryOp::set_type called twice");
    type_ = type;
}

void BinaryOpNode::dump_node(Dumper& dumper) 
{
    dumper.print_member("operator", op_);
    dumper.print_member("left", left_);
    dumper.print_member("right", right_);
}

LogicalAndNode::LogicalAndNode(shared_ptr<ExprNode> left, shared_ptr<ExprNode> right) : 
    BinaryOpNode(left, "&&", right)
{
}

LogicalOrNode::LogicalOrNode(shared_ptr<ExprNode> left, shared_ptr<ExprNode> right) : 
    BinaryOpNode(left, "&&", right)
{
}

CondExprNode::CondExprNode(shared_ptr<ExprNode> c, shared_ptr<ExprNode> t, shared_ptr<ExprNode> e) :
    cond_(c), then_expr_(t), else_expr_(e)
{
}

CondExprNode::~CondExprNode()
{
}

void CondExprNode::dump_node(Dumper& dumper) 
{
    dumper.print_member("cond", cond_);
    dumper.print_member("then_expr", then_expr_);
    dumper.print_member("else_expr", else_expr_);
}

AbstractAssignNode::AbstractAssignNode(shared_ptr<ExprNode> lhs, shared_ptr<ExprNode> rhs) : 
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
}

AssignNode::AssignNode(shared_ptr<ExprNode> lhs, shared_ptr<ExprNode> rhs) :
    AbstractAssignNode(lhs, rhs)
{
}

OpAssignNode::OpAssignNode(shared_ptr<ExprNode> lhs, const string& op, shared_ptr<ExprNode> rhs) : 
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

ReturnNode::ReturnNode(const Location& loc, shared_ptr<ExprNode> expr) : 
    StmtNode(loc), expr_(expr)
{
}

ReturnNode::~ReturnNode()
{
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
        shared_ptr<vector<shared_ptr<DefinedVariable>>> vars, 
        shared_ptr<vector<shared_ptr<StmtNode>>> stmts) : 
    StmtNode(loc), vars_(vars), stmts_(stmts)
{
}

BlockNode::~BlockNode()
{
}

void BlockNode::dump_node(Dumper& dumper) 
{
    dumper.print_node_list("variables", vars_);
    dumper.print_node_list("stmts", stmts_);
}

ExprStmtNode::ExprStmtNode(const Location& loc, shared_ptr<ExprNode> expr)
    : StmtNode(loc), expr_(expr)
{
}

void ExprStmtNode::dump_node(Dumper& dumper)
{
    dumper.print_member("expr", expr_);
}

LabelNode::LabelNode(const Location& loc, const string& name, shared_ptr<StmtNode> stmt) : 
    StmtNode(loc), name_(name), stmt_(stmt)
{
}

void LabelNode::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("stmt", stmt_);
}

CaseNode::CaseNode(const Location& loc, 
        shared_ptr<vector<shared_ptr<ExprNode>>> values, shared_ptr<BlockNode> body) : 
    StmtNode(loc), values_(values), body_(body)
{
}

void CaseNode::dump_node(Dumper& dumper)
{
    dumper.print_node_list("values", values_);
    dumper.print_member("body", body_);
}
    
SwitchNode::SwitchNode(const Location& loc, shared_ptr<ExprNode> cond, 
        shared_ptr<vector<shared_ptr<CaseNode>>> cases) : 
    StmtNode(loc), cond_(cond), cases_(cases)
{
}

void SwitchNode::dump_node(Dumper& dumper)
{
    dumper.print_member("cond", cond_);
    dumper.print_node_list("cases", cases_);
}

ForNode::ForNode(const Location& loc, shared_ptr<ExprNode> init, 
        shared_ptr<ExprNode> cond, shared_ptr<ExprNode> incr, shared_ptr<StmtNode> body) : 
    StmtNode(loc), body_(body)
{
    if (init) {
        init_.reset(new ExprStmtNode(init->location(), init));
    } else {
        init_ = nullptr;
    }

    if (cond) {
        cond_ = cond;
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
    dumper.print_member("init", init_);
    dumper.print_member("cond", cond_);
    dumper.print_member("incr", incr_);
    dumper.print_member("body", body_);
}

DoWhileNode::DoWhileNode(const Location& loc, shared_ptr<StmtNode> body, shared_ptr<ExprNode> cond) : 
    StmtNode(loc), body_(body), cond_(cond)
{
}

DoWhileNode::~DoWhileNode()
{
}

void DoWhileNode::dump_node(Dumper& dumper)
{
    dumper.print_member("body", body_);
    dumper.print_member("cond", cond_);
}

WhileNode::WhileNode(const Location& loc, shared_ptr<ExprNode> cond, shared_ptr<StmtNode> body) : 
    StmtNode(loc), cond_(cond), body_(body)
{
}

WhileNode::~WhileNode()
{
}

void WhileNode::dump_node(Dumper& dumper)
{
    dumper.print_member("cond", cond_);
    dumper.print_member("body", body_);
}

IfNode::IfNode(const Location& loc, shared_ptr<ExprNode> c, shared_ptr<StmtNode> t, shared_ptr<StmtNode> e) : 
    StmtNode(loc), cond_(c), then_body_(t), else_body_(e)
{
}

IfNode::~IfNode()
{
}

void IfNode::dump_node(Dumper& dumper)
{
    dumper.print_member("cond", cond_);
    dumper.print_member("then_body", then_body_);
    dumper.print_member("else_body", else_body_);
}

TypeDefinition::TypeDefinition(const Location& loc, shared_ptr<TypeRef> ref, const string& name) :
    loc_(loc), tnode_(new TypeNode(ref)), name_(name)
{
}

TypeDefinition::~TypeDefinition()
{
}

CompositeTypeDefinition::CompositeTypeDefinition(const Location &loc, shared_ptr<TypeRef> ref,
        const string& name, shared_ptr<vector<shared_ptr<Slot>>> membs) :
    TypeDefinition(loc, ref, name), members_(membs)
{
}

CompositeTypeDefinition::~CompositeTypeDefinition()
{
}

void CompositeTypeDefinition::dump_node(Dumper& dumper) 
{
    dumper.print_member("name", name_);
    dumper.print_node_list("members", members_);
}

StructNode::StructNode(const Location &loc, shared_ptr<TypeRef> ref,
        const string& name, shared_ptr<vector<shared_ptr<Slot>>> membs):
    CompositeTypeDefinition(loc, ref, name, membs)
{
}

shared_ptr<Type> StructNode::defining_type()
{
    return shared_ptr<Type>(
        new StructType(name(), members_, location()));
}

UnionNode::UnionNode(const Location &loc, shared_ptr<TypeRef> ref,
        const string& name, shared_ptr<vector<shared_ptr<Slot>>> membs):
    CompositeTypeDefinition(loc, ref, name, membs)
{
}

shared_ptr<Type> UnionNode::defining_type()
{
    return shared_ptr<Type>(
        new UnionType(name(), members(), location()));
}

TypedefNode::TypedefNode(const Location& loc, shared_ptr<TypeRef> real, const string& name) :
    TypeDefinition(loc, shared_ptr<TypeRef>(new UserTypeRef(name)), name), 
    real_(new TypeNode(real))
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
    dumper.print_member("typeNode", real_);
}

} // namespace may