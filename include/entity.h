#ifndef ENTRY_H_
#define ENTRY_H_

#include <string>
#include <memory>

using namespace std;

#include "util.h"
#include "type.h"

namespace may {

class Type;
class TypeNode;
class ExprNode;
class BlockNode;

class Entity {
public:
    Entity(bool priv, shared_ptr<TypeNode> type, const string& name);
    virtual ~Entity() {}
    
    string name() { return name_; }
    string symbol_string() { return name(); }

    shared_ptr<ExprNode> value() { throw string("Entity::value"); }
    shared_ptr<TypeNode> type_node() { return tnode_; }
    shared_ptr<Type> type();

    virtual bool is_defined() = 0;
    virtual bool is_initialized() = 0;
    bool is_constant() { return false; }
    bool is_parameter() { return false; }
    bool is_private() { return priv_; }
    long alloc_size();
    long alignment();
    void refered() { nref_++; }
    bool is_refered() { return nref_ > 0; } 

    // void setMemref(MemoryReference mem);
    // MemoryReference memref();
    // void setAddress(MemoryReference mem);
    // void setAddress(ImmediateValue imm)
    // Operand address()
    // void checkAddress()

    virtual string class_name() = 0;

    Location location();

    void dump(Dumper& dumper);

protected:
    virtual void dump_node(Dumper& dumper) = 0;

protected:
    string name_;
    bool priv_;
    shared_ptr<TypeNode> tnode_;
    long nref_;
    // MemoryReference
    // Operand
};

class Constant : public Entity {
public:
    Constant(shared_ptr<TypeNode> tnode, const string& name, shared_ptr<ExprNode> expr);
    bool is_assignable() { return false; }
    bool is_defined() { return true; }
    bool is_initialized() { return true; }
    bool is_constant() { return true; }

    shared_ptr<ExprNode> value() { return value_; }
    string class_name() { return "Constant"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    string name_;
    shared_ptr<TypeNode> tnode_;
    shared_ptr<ExprNode> value_;
};


class Variable : public Entity {
public:
    Variable(bool priv, shared_ptr<TypeNode> type, const string& name);
    string class_name() { return "Variable"; }
};

// TODO: 
class DefinedVariable : public Variable {
public:
    DefinedVariable(bool priv, shared_ptr<TypeNode> type, 
        const string& name, shared_ptr<ExprNode> init);

    bool is_defined() { return true; }
    bool has_initializer() { return !!init_; }
    bool is_initialized() { return has_initializer(); }

    shared_ptr<ExprNode> initializer() { return init_; }
    string class_name() { return "DefinedVariable"; }

    void dump_node(Dumper& dumper);

protected:
    shared_ptr<ExprNode> init_;
    // Expr ir_;
    long sequence;
    // Symbol symbol_;
};

typedef shared_ptr<vector<shared_ptr<DefinedVariable>>> pv_defined_variable;

class Parameter : public DefinedVariable {
public:
    Parameter(shared_ptr<TypeNode> type, const string& name);

    bool is_parameter() { return true; }
    
    string class_name() { return "Parameter"; }

    void dump_node(Dumper& dumper);
};

typedef shared_ptr<vector<shared_ptr<Parameter>>> pv_parameter;

// TODO:
class Params : public ParamSlots<Parameter> {
public:
    Params(const Location& loc, pv_parameter param_desc);

    pv_parameter parameters() { return param_descs_; }

    shared_ptr<ParamTypeRefs> parameter_typerefs();

    string class_name() { return "Params"; }

    void dump(Dumper& dumper);

    void dump_node(Dumper& dumper);
};

class ConstantEntry {

};

class Function : public Entity {
public:
    Function(bool priv, shared_ptr<TypeNode> t, const string& name);

    virtual pv_parameter parameters() = 0;

    bool is_initialized() { return true; }
    bool is_void() { return return_type()->is_void(); }

    shared_ptr<Type> return_type();
};

class DefinedFunction : public Function {
public:
    DefinedFunction(bool priv, shared_ptr<TypeNode> t, const string& name, 
        shared_ptr<Params> params, 
        shared_ptr<BlockNode> body);

    bool is_defined() { return true;}

    string class_name() { return "DefinedFunction"; }

    pv_parameter parameters() { return params_->parameters(); }

    void dump_node(Dumper& dumper);

protected:
    shared_ptr<Params> params_;
    shared_ptr<BlockNode> body_;
};

}

#endif