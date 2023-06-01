#include "node.h"
#include "util.h"
#include "token.h"

Location::Location(const Token& tok) : tok_(tok) {}

Location::Location(const string& src, const Token &tok) : 
    src_(src), tok_(tok) {}

Dumper::Dumper(ostream &os) : os_(os), indent_(0) {}

void Dumper::print_class(ast::Node* node, const Location& loc)
{
    print_indent();
    os_ << "<<" << node->name() << ">>"
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

void Dumper::print_pair(const string& name, const string& val)
{
    print_indent();
    os_ << name << ": " + val << endl;
}

void Dumper::print_member(const string& name, int n)
{
    print_pair(name, to_string(n));
}

void Dumper::print_member(const string& name, const string& str) {
    print_member(name, str, false);
}

void Dumper::print_member(const string& name, const string& str, bool is_resolved)
{
    print_pair(name, str + (is_resolved ? " (resolved)" : ""));
}

void Dumper::print_member(const string& name, long l)
{
    print_pair(name, to_string(l));
}

void Dumper::print_member(const string& name, bool b)
{
    print_pair(name, to_string(b));
}

void Dumper::print_member(const string& name, ast::TypeRef* ref)
{
    print_pair(name, ref->location().to_string());
}

void Dumper::print_member(const string& name, ast::Type* t)
{
    print_pair(name, (t == nullptr ? "null" : t->to_string()));
}

void Dumper::print_member(const string& name, ast::Node* node)
{
    print_indent();
    if (node == nullptr) {
        os_ << name << ": null" << endl;
    } else {
        os_ << name << ":" << endl;
        indent();
        node->dump(*this);
        dedent();
    }
}
