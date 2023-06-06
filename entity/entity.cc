#include "entity.h"
#include "node.h"

namespace may {

Entity::Entity(bool priv, shared_ptr<TypeNode> type, const string& name)
    : priv_(priv), tnode_(type), name_(name)
{
}

long Entity::alloc_size() 
{ 
    return type()->alloc_size(); 
}

shared_ptr<Type> Entity::type() 
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

Constant::Constant(shared_ptr<TypeNode> type, const string& name, shared_ptr<ExprNode> value)
    : Entity(true, type, name), value_(value)
{
}

void Constant::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("typeNode", tnode_.get());
    dumper.print_member("value", value_.get());
}

Variable::Variable(bool priv, shared_ptr<TypeNode> type, const string& name) :
    Entity(priv, type, name)
{
}

DefinedVariable::DefinedVariable(bool priv, shared_ptr<TypeNode> type, 
        const string& name, ExprNode* init) :
    Variable(priv, type, name), init_(init)       
{
}

void DefinedVariable::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("isPrivate", priv_);
    dumper.print_member("typeNode", tnode_);
    dumper.print_member("initializer", init_);
}

Parameter::Parameter(shared_ptr<TypeNode> type, const string& name) :
    DefinedVariable(false, type, name, nullptr)
{
}

void Parameter::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("typeNode", tnode_);
}

Params::Params(const Location& loc, shared_ptr<vector<shared_ptr<Parameter>>> param_desc) :
    ParamSlots<Parameter>(loc, param_desc, false)
{
}

void Params::dump_node(Dumper& dumper)
{
    dumper.print_node_list("parameters", parameters());
}

Function::Function(bool priv, shared_ptr<TypeNode> t, const string& name) :
    Entity(priv, t, name)
{
}

} // namespace ast