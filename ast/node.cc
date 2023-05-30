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

void TypeNode::setType(Type* tp)
{
    if (type() != nullptr) {
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
    : loc_(loc), type_node_(new TypeNode(ref))
{
}
    
LiteralNode::~LiteralNode()
{ 
    delete type_node_;
}

IntegerLiteralNode::IntegerLiteralNode(const Location& loc, TypeRef* ref, long value) 
    : LiteralNode(loc, ref), value_(value)
{
}

void IntegerLiteralNode::dump_node(Dumper& dumper)
{
    dumper.print_member("typeNode", &type_node_);
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
}

VariableNode::~VariableNode()
{
    // TODO:
}

UnaryOpNode::UnaryOpNode(const string& op, ExprNode* node)
    : op_(op), expr_(node), op_type_(nullptr)
{
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
}

SuffixOpNode::SuffixOpNode(const string& op, ExprNode* expr)
    : UnaryArithmeticOpNode(op, expr)
{
}

ArefNode::ArefNode(ExprNode* expr, ExprNode* index)
    : expr_(expr), index_(index)
{
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

Slot::Slot() : type_node_(nullptr), offset_(Type::kSizeUnknown)
{
}


Slot::Slot(TypeNode* t, const string& n)
    : type_node_(t), name_(n), offset_(Type::kSizeUnknown)
{
}

void Slot::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("typeNode", type_node_);
}

MemberNode::MemberNode(ExprNode* expr, const string& member)
    : expr_(expr), member_(member)
{
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

}