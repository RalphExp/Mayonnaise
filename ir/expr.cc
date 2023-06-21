#include "ir.h"

namespace cbc {

Expr::Expr(Type* type) : type_(type)
{
    type_->inc_ref();
}
    
Expr::~Expr() 
{
    type_->dec_ref();
}

}