#include "node.h"

void Node::dump(Dumper& dumper)
{
    dumper.print_class(*this, Location());
    dump_node(dumper);
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