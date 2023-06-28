#include "ast.h"

namespace cbc {

AST::AST(const Location& source, Declarations* declarations) :
    source_(source), decls_(declarations)
{
}

AST::~AST()
{
    decls_->dec_ref();
}

void AST::dump_node(Dumper& dumper)
{
    dumper.print_node_list("variables", defined_variables());
    dumper.print_node_list("functions", defined_functions());
}

}