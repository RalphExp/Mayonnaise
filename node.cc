#include "node.h"

void Node::dump(Dumper& dumper)
{
    dumper.print_class(*this, Location());
    dump_node(dumper);
}
