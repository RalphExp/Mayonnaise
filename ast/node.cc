#include "node.h"

namespace cbc {

void Node::dump(ostream& os) 
{
    Dumper dumper(os);
    dump(dumper);
}

void Node::dump(Dumper& dumper)
{
    dumper.print_class<Node>(this, location());
    dump_node(dumper);
}

TypeNode::TypeNode(Type* tp) : 
    type_(tp), ref_(nullptr)
{
    type_->inc_ref();
}

TypeNode::TypeNode(TypeRef* ref) : 
    type_(nullptr), ref_(ref)
{
    ref_->inc_ref();
}

TypeNode::TypeNode(Type* tp, TypeRef* ref) : 
    type_(tp), ref_(ref)
{
    type_->inc_ref();
    ref_->inc_ref();
}

Type* TypeNode::type()
{
    if (!type_) {
        throw string("Type not resolved");
    }
    return type_;
}

TypeNode::~TypeNode()
{
    type_->dec_ref();
    ref_->dec_ref();
}

void TypeNode::set_type(Type* tp)
{
    if (type_) {
        throw string("TypeNode::setType called twice");
    }
    tp->inc_ref();
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

LiteralNode::LiteralNode(const Location& loc, TypeRef* ref) : 
    loc_(loc), tnode_(new TypeNode(ref))
{
    tnode_->inc_ref();
}
    
LiteralNode::~LiteralNode()
{
    tnode_->dec_ref();
}

IntegerLiteralNode::IntegerLiteralNode(const Location& loc, TypeRef* ref, long value) : 
    LiteralNode(loc, ref), value_(value)
{
}

void IntegerLiteralNode::dump_node(Dumper& dumper)
{
    dumper.print_member("typeNode", tnode_);
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
    orig_type_->dec_ref();
    type_->dec_ref();
}

void LHSNode::set_type(Type* type)
{
    type->inc_ref();
    type_ = type;
}

bool LHSNode::is_loadable()
{
    Type* t = orig_type_;
    return !t->is_array() && !t->is_function();
}

VariableNode::VariableNode(const Location& loc, const string& name) : 
    loc_(loc), name_(name), entity_(nullptr)
{
}

VariableNode::~VariableNode()
{
    entity_->dec_ref();
}

Entity* VariableNode::entity()
{
    if (!entity_) {
        throw string("VariableNode.entity == null");
    }
    return entity_;
}

void VariableNode::set_entity(Entity* ent) 
{
    ent->inc_ref();
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

void VariableNode::dump_node(Dumper& dumper)
{
    if (type_ != nullptr) {
        dumper.print_member("type", type_);
    }
    dumper.print_member("name", name_, is_resolved());
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
    expr_->inc_ref();
}

void UnaryOpNode::set_op_type(Type* type) 
{ 
    type->inc_ref();
    op_type_ = type; 
}
    
void UnaryOpNode::set_expr(ExprNode* expr) 
{
    expr->inc_ref();
    expr_ = expr; 
}

UnaryOpNode::~UnaryOpNode()
{
    op_type_->dec_ref();
    expr_->dec_ref();
}

void UnaryOpNode::dump_node(Dumper& dumper)
{ 
    dumper.print_member("operator", op_);
    dumper.print_member("expr", expr_);
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
    expr_->inc_ref();
    index_->inc_ref();
}

ArefNode::~ArefNode()
{
    expr_->dec_ref();
    index_->dec_ref();
}

bool ArefNode::is_multi_dimension()
{
    ArefNode* expr = dynamic_cast<ArefNode*>(expr_);
    return expr && !expr->orig_type()->is_pointer();
}

ExprNode* ArefNode::base_expr()
{
    return !is_multi_dimension() ? expr_ :
        ((ArefNode*)expr_)->base_expr();
}

Type* ArefNode::orig_type()
{
    return expr_->orig_type()->base_type();
}

long ArefNode::length()
{
    return ((ArrayType*)(expr_->orig_type()))->length();
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


Slot::Slot(TypeNode* t, const string& n) : 
    tnode_(t), name_(n), offset_(Type::kSizeUnknown)
{
    tnode_->inc_ref();
}

Slot::~Slot()
{
    tnode_->dec_ref();
}

void Slot::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("typeNode", tnode_);
}

MemberNode::MemberNode(ExprNode* expr, const string& member) : 
    expr_(expr), member_(member)
{
    expr_->inc_ref();
}

MemberNode::~MemberNode()
{
    expr_->dec_ref();
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

PtrMemberNode::PtrMemberNode(ExprNode* expr, const string& member) : 
    expr_(expr), member_(member)
{
    expr_->inc_ref();
}

PtrMemberNode::~PtrMemberNode()
{
    expr_->dec_ref();
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
    if (type_) {
        dumper.print_member("type", type_);
    }

    dumper.print_member("expr", expr_);
    dumper.print_member("member", member_);
}
    
FuncallNode::FuncallNode(ExprNode* expr, vector<ExprNode*>&& args) : 
    expr_(expr), args_(move(args))
{
    expr_->inc_ref();

    for (auto *e : args_) {
        e->inc_ref();
    }
}

FuncallNode::~FuncallNode()
{
    expr_->dec_ref();

    for (auto *e : args_) {
        e->dec_ref();
    }
}

void FuncallNode::replaceArgs(vector<ExprNode*>&& args)
{
    for (auto *e : args_) {
        e->dec_ref();
    }

    args = move(args);

    for (auto *e : args_) {
        e->inc_ref();
    }
}

Type* FuncallNode::type()
{
    return function_type()->return_type();
}
    
FunctionType* FuncallNode::function_type()
{
    return expr_->type()->get_pointer_type()->base_type()->get_function_type();
}

void FuncallNode::dump_node(Dumper &dumper)
{
    dumper.print_member("expr", expr_);
    dumper.print_node_list("args", args_);
}

SizeofExprNode::SizeofExprNode(ExprNode* expr, TypeRef* ref) :
    expr_(expr), tnode_(new TypeNode(ref))
{
    expr_->inc_ref();
    tnode_->inc_ref();
}

void SizeofExprNode::set_expr(ExprNode* expr)
{
    expr->inc_ref();
    expr_->dec_ref();
    expr_  = expr;
}

SizeofExprNode::~SizeofExprNode()
{
    expr_->dec_ref();
    tnode_->dec_ref();
}

void SizeofExprNode::dump_node(Dumper& dumper)
{
    dumper.print_member("expr", expr_);
}

SizeofTypeNode::SizeofTypeNode(TypeNode* operand, TypeRef* ref) :
    op_(operand), tnode_(new TypeNode(ref))
{
    op_->inc_ref();
    tnode_->inc_ref();
}

SizeofTypeNode::~SizeofTypeNode()
{
    op_->dec_ref();
    tnode_->dec_ref();
}

void SizeofTypeNode::dump_node(Dumper& dumper)
{
    dumper.print_member("operand", op_);
}

AddressNode::AddressNode(ExprNode* expr) : expr_(expr)
{
    expr_->inc_ref();
}

AddressNode::~AddressNode()
{
    expr_->dec_ref();
    type_->dec_ref();
}
    
Type* AddressNode::type()
{
    if (!type_) 
        throw string("type is null");
    
    return type_;
}
    
void AddressNode::set_type(Type* type)
{
    if (type_) 
        throw string("type set twice");

    type->inc_ref();
    type_ = type;
}
    
void AddressNode::dump_node(Dumper& dumper)
{
    if (type_) {
        dumper.print_member("type", type_);
    }
        
    dumper.print_member("expr", expr_);
}

DereferenceNode::DereferenceNode(ExprNode* expr)
    : expr_(expr)
{
    expr_->inc_ref();
}

void DereferenceNode::set_expr(ExprNode* expr)
{
    expr->inc_ref();
    expr_->dec_ref();
    expr_ = expr; 
}

Type* DereferenceNode::orig_type()
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

PrefixOpNode::PrefixOpNode(const string& op, ExprNode* expr) :
    UnaryArithmeticOpNode(op, expr)
{
}

CastNode::CastNode(Type* t, ExprNode* expr) : 
    tnode_(new TypeNode(t)), expr_(expr)
{
    tnode_->inc_ref();
    expr_->inc_ref();
}

CastNode::CastNode(TypeNode* t, ExprNode* expr) : 
    tnode_(t), expr_(expr)
{
    tnode_->inc_ref();
    expr_->inc_ref();
}

void CastNode::dump_node(Dumper& dumper)
{
    dumper.print_member("typeNode", tnode_);
    dumper.print_member("expr", expr_);
}

CastNode::~CastNode()
{
    tnode_->dec_ref();
    expr_->dec_ref();
}

BinaryOpNode::BinaryOpNode(ExprNode* left, const string& op, ExprNode* right) : 
    type_(nullptr), left_(left), op_(op), right_(right)
{
    left_->inc_ref();
    right_->inc_ref();
}

BinaryOpNode::BinaryOpNode(Type* t, ExprNode* left, const string& op, ExprNode* right) : 
    type_(t), left_(left), op_(op), right_(right)
{
    type_->inc_ref();
    left_->inc_ref();
    right_->inc_ref();
}

BinaryOpNode::~BinaryOpNode()
{
    type_->dec_ref();
    left_->dec_ref();
    right_->dec_ref();
}

void BinaryOpNode::set_type(Type* type) 
{
    if (!type_)
        throw string("BinaryOp::set_type called twice");
    
    type_->inc_ref();
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
    cond_->inc_ref();
    then_expr_->inc_ref();
    else_expr_->inc_ref();
}

void CondExprNode::set_then_expr(ExprNode* expr) 
{
    expr->inc_ref();
    then_expr_->dec_ref();
    then_expr_ = expr; 
}

void CondExprNode::set_else_expr(ExprNode* expr)
{ 
    expr->inc_ref();
    else_expr_->dec_ref();
    else_expr_ = expr; 
}

CondExprNode::~CondExprNode()
{
    cond_->dec_ref();
    then_expr_->dec_ref();
    else_expr_->dec_ref();  
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
    lhs_->inc_ref();
    rhs_->inc_ref();
}

void AbstractAssignNode::set_rhs(ExprNode* expr) 
{ 
    expr->inc_ref();
    rhs_->dec_ref();
    rhs_ = expr; 
}

void AbstractAssignNode::dump_node(Dumper& dumper) 
{
    dumper.print_member("lhs", lhs_);
    dumper.print_member("rhs", rhs_);
}

AbstractAssignNode::~AbstractAssignNode()
{
    lhs_->dec_ref();
    rhs_->dec_ref();
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
    expr_->inc_ref();
}

void ReturnNode::set_expr(ExprNode* expr)
{
    expr->inc_ref();
    expr_->dec_ref();
    expr_ = expr;
}

ReturnNode::~ReturnNode()
{
    expr_->dec_ref();
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

BlockNode::BlockNode(const Location& loc, vector<DefinedVariable*>&& vars,
        vector<StmtNode*>&& stmts) :
    StmtNode(loc), vars_(move(vars)), stmts_(move(stmts))
{
    for (auto* d : vars_) {
        d->inc_ref();
    }

    for (auto* s : stmts_) {
        s->inc_ref();
    }
}

BlockNode::~BlockNode()
{
    for (auto* d : vars_) {
        d->dec_ref();
    }

    for (auto* s : stmts_) {
        s->dec_ref();
    } 
}

StmtNode* BlockNode::tail_stmt()
{
    if (stmts_.empty())
        return nullptr;

    return stmts_.back();
}

void BlockNode::dump_node(Dumper& dumper) 
{
    dumper.print_node_list("variables", vars_);
    dumper.print_node_list("stmts", stmts_);
}

ExprStmtNode::ExprStmtNode(const Location& loc, ExprNode* expr)
    : StmtNode(loc), expr_(expr)
{
    expr_->inc_ref();
}

ExprStmtNode::~ExprStmtNode()
{
    expr_->dec_ref();
}

void ExprStmtNode::set_expr(ExprNode* expr)
{
    expr->inc_ref();
    expr_->dec_ref();
    expr_ = expr;
}

void ExprStmtNode::dump_node(Dumper& dumper)
{
    dumper.print_member("expr", expr_);
}

LabelNode::LabelNode(const Location& loc, const string& name, StmtNode* stmt) : 
    StmtNode(loc), name_(name), stmt_(stmt)
{
    stmt_->inc_ref();
}

LabelNode::~LabelNode()
{
    stmt_->dec_ref();
}

void LabelNode::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("stmt", stmt_);
}

CaseNode::CaseNode(const Location& loc, vector<ExprNode*>&& values, 
        BlockNode* body) : 
    StmtNode(loc), values_(move(values)), body_(body)
{
    for (auto* e : values_) {
        e->inc_ref();
    }
    body_->inc_ref();
}

CaseNode::~CaseNode()
{
    for (auto* e : values_) {
        e->dec_ref();
    }
    body_->dec_ref();
}

void CaseNode::dump_node(Dumper& dumper)
{
    dumper.print_node_list("values", values_);
    dumper.print_member("body", body_);
}
    
SwitchNode::SwitchNode(const Location& loc, ExprNode* cond, 
        vector<CaseNode*>&& cases) :
    StmtNode(loc), cond_(cond), cases_(move(cases))
{
    cond_->inc_ref();
    for (auto* e : cases_) {
        e->inc_ref();
    }
}

SwitchNode::~SwitchNode()
{
    cond_->dec_ref();
    for (auto* e : cases_) {
        e->dec_ref();
    }
}

void SwitchNode::dump_node(Dumper& dumper)
{
    dumper.print_member("cond", cond_);
    dumper.print_node_list("cases", cases_);
}

ForNode::ForNode(const Location& loc, ExprNode* init, 
        ExprNode* cond, ExprNode* incr, StmtNode* body) : 
    StmtNode(loc), body_(body)
{
    if (init) {
        init_ = new ExprStmtNode(init->location(), init);
        init_->inc_ref();
    } else {
        init_ = nullptr;
    }

    if (cond) {
        cond->inc_ref();
        cond_ = cond;
    } else {
        /* default to be true(1) */
        cond_ = new IntegerLiteralNode(Location(), IntegerTypeRef::int_ref(), 1);
        cond_->inc_ref();
    }

    if (incr) {
        incr_ = new ExprStmtNode(incr->location(), incr);
        incr_->inc_ref();
    } else {
        incr_ = nullptr;
    }
}

ForNode::~ForNode()
{
    init_->dec_ref();
    cond_->dec_ref();
    incr_->dec_ref();
}

void ForNode::dump_node(Dumper& dumper)
{
    dumper.print_member("init", init_);
    dumper.print_member("cond", cond_);
    dumper.print_member("incr", incr_);
    dumper.print_member("body", body_);
}

DoWhileNode::DoWhileNode(const Location& loc, StmtNode* body, ExprNode* cond) : 
    StmtNode(loc), body_(body), cond_(cond)
{
    body_->inc_ref();
    cond_->inc_ref();
}

DoWhileNode::~DoWhileNode()
{
    body_->dec_ref();
    cond_->dec_ref();
}

void DoWhileNode::dump_node(Dumper& dumper)
{
    dumper.print_member("body", body_);
    dumper.print_member("cond", cond_);
}

WhileNode::WhileNode(const Location& loc, ExprNode* cond, StmtNode* body) : 
    StmtNode(loc), cond_(cond), body_(body)
{
    cond_->inc_ref();
    body_->inc_ref();
}

WhileNode::~WhileNode()
{
    cond_->dec_ref();
    body_->dec_ref();
}

void WhileNode::dump_node(Dumper& dumper)
{
    dumper.print_member("cond", cond_);
    dumper.print_member("body", body_);
}

IfNode::IfNode(const Location& loc, ExprNode* c, 
        StmtNode* t, StmtNode* e) : 
    StmtNode(loc), cond_(c), then_body_(t), else_body_(e)
{
    cond_->inc_ref();
    then_body_->inc_ref();
    else_body_->inc_ref();
}

IfNode::~IfNode()
{
    cond_->dec_ref();
    then_body_->dec_ref();
    else_body_->dec_ref();
}

void IfNode::dump_node(Dumper& dumper)
{
    dumper.print_member("cond", cond_);
    dumper.print_member("then_body", then_body_);
    dumper.print_member("else_body", else_body_);
}

TypeDefinition::TypeDefinition(const Location& loc, TypeRef* ref, const string& name) :
    loc_(loc), tnode_(new TypeNode(ref)), name_(name)
{
    tnode_->inc_ref();
}

TypeDefinition::~TypeDefinition()
{
    tnode_->dec_ref();
}

CompositeTypeDefinition::CompositeTypeDefinition(const Location &loc, TypeRef* ref,
        const string& name, vector<Slot*>&& membs) :
    TypeDefinition(loc, ref, name), members_(move(membs))
{
    for (auto* s : members_) {
        s->inc_ref();
    }
}

CompositeTypeDefinition::~CompositeTypeDefinition()
{
    for (auto* s : members_) {
        s->dec_ref();
    }
}

void CompositeTypeDefinition::dump_node(Dumper& dumper) 
{
    dumper.print_member("name", name_);
    dumper.print_node_list("members", members_);
}

StructNode::StructNode(const Location &loc, TypeRef* ref,
        const string& name, vector<Slot*>&& membs):
    CompositeTypeDefinition(loc, ref, name, move(membs))
{
}

Type* StructNode::defining_type()
{
    return new StructType(name(), members(), location());
}

UnionNode::UnionNode(const Location &loc, TypeRef* ref,
        const string& name, vector<Slot*>&& membs):
    CompositeTypeDefinition(loc, ref, name, move(membs))
{
}

Type* UnionNode::defining_type()
{
    return new UnionType(name(), members(), location());
}

TypedefNode::TypedefNode(const Location& loc, TypeRef* real, const string& name) :
    TypeDefinition(loc, new UserTypeRef(name), name), 
    real_(new TypeNode(real))
{
    real_->inc_ref();
}

TypedefNode::~TypedefNode()
{
    real_->dec_ref();
}

Type* TypedefNode::defining_type()
{
    return new UserType(name_, real_, loc_);
}

void TypedefNode::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("typeNode", real_);
}

} // namespace cbc