#include "util.h"
#include "node.h"

#include <iostream>

Dumper::Dumper(ostream &os) : os(os), ind(0) {}

void Dumper::print_class(const Node& node, const Location& loc)
{
    print_indent();
    os << "<<" << node.get_name() 
       << ">>" 
       << "(" << loc.to_string() << ")" 
       << endl;
}

static const string indent_string = "    ";

void Dumper::print_indent()
{
    int n = ind;
    while (n-- > 0) {
        os << indent_string;
    }
}