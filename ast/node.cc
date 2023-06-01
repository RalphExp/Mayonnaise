#include "node.h"

namespace ast {

void Node::dump(ostream& os) 
{
    Dumper dumper(os);
    dump(dumper);
}

void Node::dump(Dumper& dumper)
{
    dumper.print_class(this, Location());
    dump_node(dumper);
}

TypeNode::TypeNode(Type* tp)
    : type_(tp), ref_(nullptr)
{
    name_ = "TypeNode";
}

TypeNode::TypeNode(TypeRef* ref)
    : type_(nullptr), ref_(ref)
{
    name_ = "TypeNode";
}

TypeNode::TypeNode(Type* tp, TypeRef* ref) : type_(tp), ref_(ref)
{
    name_ = "TypeNode";
}

Type* TypeNode::type()
{
    if (type_ == nullptr) {
        throw string("Type not resolved");
    }
    return type_;
}

TypeNode::~TypeNode()
{
    delete type_;
    delete ref_;
}

void TypeNode::set_type(Type* tp)
{
    if (type_ != nullptr) {
        throw string("TypeNode::setType called twice");
    }
    type_ = tp;
}

void TypeNode::dump_node(Dumper& dumper)
{
    dumper.print_member("typeref", ref_);
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

LiteralNode::LiteralNode(const Location& loc, TypeRef* ref) 
    : loc_(loc), tnode_(new TypeNode(ref))
{
    name_ = "LiteralNode";
}
    
LiteralNode::~LiteralNode()
{ 
    delete tnode_;
}

IntegerLiteralNode::IntegerLiteralNode(const Location& loc, TypeRef* ref, long value) 
    : LiteralNode(loc, ref), value_(value)
{
    name_ = "IntegerLiteralNode";
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
    name_ = "IntegerLiteralNode";
}

void StringLiteralNode::dump_node(Dumper& dumper) 
{ 
    dumper.print_member("value", value_);
}

LHSNode::LHSNode() : type_(nullptr), orig_type_(nullptr)
{
    name_ = "LHSNode";
}

LHSNode::~LHSNode()
{
    delete type_;
    delete orig_type_;
}

bool LHSNode::is_loadable()
{
    Type* t = orig_type_;
    return !t->is_array() && !t->is_function();
}

VariableNode::VariableNode(const Location& loc, const string& name)
    : loc_(loc), name_(name)
{
    name_ = "VariableNode";
}

VariableNode::~VariableNode()
{
    // TODO:
}

UnaryOpNode::UnaryOpNode(const string& op, ExprNode* node)
    : op_(op), expr_(node), op_type_(nullptr)
{
    name_ = "UnaryOpNode";
}

UnaryOpNode::~UnaryOpNode()
{
    delete op_type_;
    delete expr_;
}

void UnaryOpNode::dump_node(Dumper& dumper)
{ 
    dumper.print_member("operator", op_);
    dumper.print_member("expr", expr_);
}

UnaryArithmeticOpNode::UnaryArithmeticOpNode(const string& op, ExprNode* node)
    : UnaryOpNode(op, node), amount_(0)
{
    name_ = "UnaryArithmeticOpNode";
}

SuffixOpNode::SuffixOpNode(const string& op, ExprNode* expr)
    : UnaryArithmeticOpNode(op, expr)
{
    name_ = "SuffixOpNode";
}

ArefNode::ArefNode(ExprNode* expr, ExprNode* index)
    : expr_(expr), index_(index)
{
    name_ = "ArefNode";
}

bool ArefNode::is_multi_dimension()
{
    ArefNode* expr = dynamic_cast<ArefNode*>(expr_);
    return expr && !expr->orig_type()->is_pointer();
}

ArefNode::~ArefNode()
{
    delete expr_;
    delete index_;
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
    return ((ArrayType*)expr_->orig_type())->length();
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
    name_ = "Slot";
}


Slot::Slot(TypeNode* t, const string& n)
    : tnode_(t), name_(n), offset_(Type::kSizeUnknown)
{
    name_ = "Slot";
}

void Slot::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("typeNode", tnode_);
}

MemberNode::MemberNode(ExprNode* expr, const string& member)
    : expr_(expr), member_(member)
{
    name_ = "MemberNode";
}

CompositeType* MemberNode::base_type()
{
    return expr_->type()->get_composite_type();
}

void MemberNode::dump_node(Dumper& dumper)
{
    if (type_ != nullptr) {
        dumper.print_member("type", type_);
    }
    dumper.print_member("expr", expr_);
    dumper.print_member("member", member_);
}

PtrMemberNode::PtrMemberNode(ExprNode* expr, const string& member)
    : expr_(expr), member_(member)
{
    name_ = "PtrMemberNode";
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
    if (type_ != nullptr) {
        dumper.print_member("type", type_);
    }

    dumper.print_member("expr", expr_);
    dumper.print_member("member", member_);
}

FuncallNode::FuncallNode(ExprNode* expr, const vector<ExprNode*>& args)
    : expr_(expr), args_(args)
{
    name_ = "FuncallNode";
}
    
FuncallNode::FuncallNode(ExprNode* expr, vector<ExprNode*>&& args)
    : expr_(expr), args_(move(args))
{
}

/* TODO: */
Type* FuncallNode::type()
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
    delete expr_;
    for (auto expr : args_) {
        delete expr;
    }
}

SizeofExprNode::SizeofExprNode(ExprNode* expr, TypeRef* ref) :
    expr_(expr), tnode_(new TypeNode(ref))
{
    name_ = "SizeofExprNode";
}

void SizeofExprNode::dump_node(Dumper& dumper)
{
    dumper.print_member("expr", expr_);
}

SizeofTypeNode::SizeofTypeNode(TypeNode* operand, TypeRef* ref)
    : op_(operand), tnode_(new TypeNode(ref))
{
    name_ = "SizeofTypeNode";
}

void SizeofTypeNode::dump_node(Dumper& dumper)
{
    dumper.print_member("operand", op_);
}

AddressNode::AddressNode(ExprNode* expr)
    : expr_(expr)
{
    name_ = "AddressNode";
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
    name_ = "DereferenceNode";
}
    
Type* DereferenceNode::orig_type()
{
    return expr_->type()->base_type();
}

void DereferenceNode::dump_node(Dumper& dumper)
{
    if (type_ != nullptr) {
        dumper.print_member("type", type_);
    }

    dumper.print_member("expr", expr_);
}

PrefixOpNode::PrefixOpNode(const string& op, ExprNode* expr)
    : UnaryArithmeticOpNode(op, expr)
{
    name_ = "PrefixOpNode";
}

CastNode::CastNode(Type* t, ExprNode* expr)
    : tnode_(new TypeNode(t)), expr_(expr)
{
    name_ = "CastNode";
}

CastNode::CastNode(TypeNode* t, ExprNode* expr)
    : tnode_(t), expr_(expr)
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

BinaryOpNode::BinaryOpNode(ExprNode* left, const string& op, ExprNode* right)
    : type_(nullptr), left_(left), op_(op), right_(right)
{
    name_ = "BinaryOpNode";
}

BinaryOpNode::BinaryOpNode(Type* t, ExprNode* left, const string& op, ExprNode* right)
    : type_(t), left_(left), op_(op), right_(right)
{
    name_ = "BinaryOpNode";
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

LogicalAndNode::LogicalAndNode(ExprNode* left, ExprNode* right)
    : BinaryOpNode(left, "&&", right)
{
    name_ = "LogicalAndNode";
}

LogicalOrNode::LogicalOrNode(ExprNode* left, ExprNode* right)
    : BinaryOpNode(left, "&&", right)
{
    name_ = "LogicalOrNode";
}

CondExprNode::CondExprNode(ExprNode* c, ExprNode* t, ExprNode* e) :
    cond_(c), then_expr_(t), else_expr_(e)
{
    name_ = "CondExprNode";
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

} // namespace ast