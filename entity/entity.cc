#include "entity.h"
#include "node.h"

namespace cbc {

Entity::Entity(bool priv, TypeNode* type, const string& name)
    : priv_(priv), tnode_(type), name_(name)
{
    tnode_->inc_ref();
}

Entity::~Entity()
{
    tnode_->dec_ref();
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

Constant::Constant(TypeNode* type, const string& name, ExprNode* value)
    : Entity(true, type, name), value_(value)
{
    value_->inc_ref();
}

Constant::~Constant()
{
    value_->dec_ref();
}

ConstantEntry::ConstantEntry(const string& val) :
    val_(val)
{
}

void Constant::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("typeNode", tnode_);
    dumper.print_member("value", value_);
}

Variable::Variable(bool priv, TypeNode* type, const string& name) :
    Entity(priv, type, name)
{
}

DefinedVariable::DefinedVariable(bool priv, TypeNode* type, 
        const string& name, ExprNode* init) :
    Variable(priv, type, name), init_(init)       
{
    init_->inc_ref();
}

DefinedVariable::~DefinedVariable()
{
    init_->dec_ref();
}

void DefinedVariable::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("isPrivate", priv_);
    dumper.print_member("typeNode", tnode_);
    dumper.print_member("initializer", init_);
}

UndefinedVariable::UndefinedVariable(TypeNode* type, const string& name) :
    Variable(false, type, name)
{
}

void UndefinedVariable::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("isPrivate", is_private());
    dumper.print_member("typeNode", tnode_);
}

Parameter::Parameter(TypeNode* type, const string& name) :
    DefinedVariable(false, type, name, nullptr)
{
}

void Parameter::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("typeNode", tnode_);
}

Params::Params(const Location& loc, vector<Parameter*>&& param_desc) :
    ParamSlots<Parameter>(loc, move(param_desc), false)
{
}

void Params::dump(Dumper& dumper)
{
    dumper.print_node_list("parameters", parameters());
}

ParamTypeRefs* Params::parameter_typerefs()
{
    vector<TypeRef*> typeref;
    for (auto* param : parameters()) {
        typeref.push_back(param->type_node()->type_ref());
    }

    return new ParamTypeRefs(loc_, move(typeref), vararg_);
}

void Params::dump_node(Dumper& dumper)
{
    dumper.print_node_list("parameters", parameters());
}

Function::Function(bool priv, TypeNode* type, const string& name) :
    Entity(priv, type, name)
{
}

Type* Function::return_type()
{
    return type()->get_function_type()->return_type(); 
}

DefinedFunction::DefinedFunction(bool priv, TypeNode* type, const string& name,
        Params* params, BlockNode* body) :

    Function(priv, type, name), params_(params), body_(body)
{
}

void DefinedFunction::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("isPrivate", priv_);
    dumper.print_member("params", params_);
    dumper.print_member("body", body_);
}

UndefinedFunction::UndefinedFunction(TypeNode* type, 
        const string& name, Params* params) :

    Function(false, type, name), params_(params)
{
    params_->inc_ref();
}

void UndefinedFunction::dump_node(Dumper& dumper)
{
    dumper.print_member("name", name_);
    dumper.print_member("isPrivate", priv_);
    dumper.print_member("typeNode", tnode_);
    dumper.print_member("params", params_);
}

} // namespace ast