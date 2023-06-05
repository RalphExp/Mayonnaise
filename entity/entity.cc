#include "entity.h"
#include "node.h"

namespace may {

Entity::Entity(bool priv, TypeNode* type, const string& name)
    : priv_(priv), tnode_(type), name_(name)
{
}

long Entity::alloc_size() 
{ 
    return type()->alloc_size(); 
}

Type* Entity::type() 
{ 
    return tnode_->type(); 
}
    
long Entity::alignment() 
{ 
    return type()->alignment(); 
}

Location Entity::location() 
{ 
    return tnode_->location(); 
}

void Entity::dump(Dumper& dumper)
{
    dumper.print_class(this, location());
    dump_node(dumper);
}

Constant::Constant(TypeNode* type, const string& name, ExprNode* value)
    : Entity(true, type, name), value_(value)
{
}

void Constant::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("typeNode", tnode_);
    dumper.print_member("value", value_);
}


}