#include "util.h"
#include "node.h"

#include <iostream>

Dumper::Dumper(ostream &os) : os(os), indent(0) {}

void Dumper::print_class(const Node& node, const Location& loc)
{
    print_indent();
    os << "<<" << node.get_name() 
       << ">>" 
       << "(" << loc.to_string() << ")" 
       << endl;
}

void Dumper::print_indent()
{

}