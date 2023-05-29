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

Type* TypeNode::type() const
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

bool ExprNode::is_callable() const
{
    try {
        return type()->is_callable();
    } catch (...) {
        /* TODO: Semantic Error */
        return false;
    }
}

bool ExprNode::is_pointer() const
{
    try {
        return type()->is_pointer();
    } catch (...) {
        return false;
    }
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

LHSNode::LHSNode()
{

}

Type* LHSNode::type() const
{
    return nullptr;
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
    : op_(op), expr_(node)
{
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

}