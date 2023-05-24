#include "node.h"

void Node::dump(ostream& os) 
{
    Dumper dumper(os);
    dump(dumper);
}

void Node::dump(Dumper& dumper)
{
    dumper.print_class(*this, Location());
    dump_node(dumper);
}

Type* TypeNode::type()
{
    Type* tp = type_.get();
    if (tp == nullptr) {
        throw "Type not resolved";
    }
    return tp;
}

void TypeNode::setType(Type* tp)
{
    if (type() != nullptr) {
        throw "TypeNode::setType called twice";
    }
    type_.reset(tp);
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