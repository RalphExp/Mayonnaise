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


}