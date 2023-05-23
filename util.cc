#include "util.h"
#include "node.h"

#include <iostream>

Dumper::Dumper(ostream &os) : os_(os), indent_(0) {}

void Dumper::print_class(const Node& node, const Location& loc)
{
    print_indent();
    os_ << "<<" << node.name() 
       << ">>" 
       << "(" << loc.to_string() << ")" 
       << endl;
}

static const string indent_string = "    ";

void Dumper::print_indent()
{
    int n = indent_;
    while (n-- > 0) {
        os_ << indent_string;
    }
}
