#include "ast.h"

namespace cbc {

AST::AST(const Location& source, Declarations* declarations) :
    source_(source), decls_(declarations)
{
}

AST::~AST()
{
    delete decls_;
}

}