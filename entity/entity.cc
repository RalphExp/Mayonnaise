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

void Entity::dump(Dumper& dumper)
{
    dumper.print_class(this, location());
    dump_node(dumper);
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
        const string& name, shared_ptr<ExprNode> init) :
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

void Params::dump(Dumper& dumper)
{
    dumper.print_node_list("parameters", parameters());
}

shared_ptr<ParamTypeRefs> Params::parameter_typerefs()
{
    pv_typeref typeref = pv_typeref(new vector<shared_ptr<TypeRef>>);
    for (auto param : *parameters()) {
        typeref->push_back(param->type_node()->type_ref());
    }

    return shared_ptr<ParamTypeRefs>(
        new ParamTypeRefs(loc_, typeref, vararg_));
}

void Params::dump_node(Dumper& dumper)
{
    dumper.print_node_list("parameters", parameters());
}

Function::Function(bool priv, shared_ptr<TypeNode> t, const string& name) :
    Entity(priv, t, name)
{
}

shared_ptr<Type> Function::return_type()
{
    return type()->get_function_type()->return_type(); 
}

DefinedFunction::DefinedFunction(bool priv, shared_ptr<TypeNode> t, const string& name,
        shared_ptr<Params> params, 
        shared_ptr<BlockNode> body) :

    Function(priv, t, name), params_(params), body_(body)
{
}

void DefinedFunction::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("isPrivate", priv_);
    dumper.print_member("params", params_);
    dumper.print_member("body", body_);
}

} // namespace ast