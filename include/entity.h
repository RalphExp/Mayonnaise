#ifndef ENTRY_H_
#define ENTRY_H_

#include <string>

using namespace std;

#include "util.h"
#include "type.h"

namespace cbc {

class Type;
class TypeNode;
class ExprNode;
class BlockNode;

class Entity : public Object {
public:
    Entity(bool priv, TypeNode* type, const string& name);
    virtual ~Entity();
    
    string name() { return name_; }
    string symbol_string() { return name(); }

    ExprNode* value() { throw string("Entity::value"); }
    TypeNode* type_node() { return tnode_; }
    Type* type();

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
    TypeNode* tnode_;
    long nref_;
    // MemoryReference
    // Operand
};

class Constant : public Entity {
public:
    Constant(TypeNode* tnode, const string& name, ExprNode* expr);
    ~Constant();
    bool is_assignable() { return false; }
    bool is_defined() { return true; }
    bool is_initialized() { return true; }
    bool is_constant() { return true; }

    ExprNode* value() { return value_; }
    string class_name() { return "Constant"; }

protected:
    void dump_node(Dumper& dumper);

protected:
    string name_;
    TypeNode* tnode_;
    ExprNode* value_;
};

/* TODO: */
class ConstantEntry : public Object {
public:
    ConstantEntry(const string& val);

protected:
    string val_;
};

class Variable : public Entity {
public:
    Variable(bool priv, TypeNode* type, const string& name);
    string class_name() { return "Variable"; }
};

// TODO: 
class DefinedVariable : public Variable {
public:
    DefinedVariable(bool priv, TypeNode* type, 
        const string& name, ExprNode* init);

    ~DefinedVariable();

    bool is_defined() { return true; }
    bool has_initializer() { return !!init_; }
    bool is_initialized() { return has_initializer(); }

    ExprNode* initializer() { return init_; }
    string class_name() { return "DefinedVariable"; }

    void dump_node(Dumper& dumper);

protected:
    ExprNode* init_;
    // Expr ir_;
    long sequence;
    // Symbol symbol_;
};

class UndefinedVariable : public Variable {
public:
    UndefinedVariable(TypeNode* type, const string& name);

    bool is_defined() { return false; }
    bool is_private() { return false; }
    bool is_initialized() { return false; }
    void dump_node(Dumper& dumper);
};

class Parameter : public DefinedVariable {
public:
    Parameter(TypeNode* type, const string& name);

    bool is_parameter() { return true; }
    
    string class_name() { return "Parameter"; }

    void dump_node(Dumper& dumper);
};

// TODO:
class Params : public ParamSlots<Parameter> {
public:
    Params(const Location& loc, vector<Parameter*>&& param_desc);

    vector<Parameter*> parameters() { return param_descs_; }

    ParamTypeRefs* parameter_typerefs();

    string class_name() { return "Params"; }

    void dump(Dumper& dumper);

    void dump_node(Dumper& dumper);
};

class Function : public Entity {
public:
    Function(bool priv, TypeNode* t, const string& name);

    virtual vector<Parameter*> parameters() = 0;

    bool is_initialized() { return true; }
    bool is_void() { return return_type()->is_void(); }

    Type* return_type();
};

class DefinedFunction : public Function {
public:
    DefinedFunction(bool priv, TypeNode* t, const string& name, 
        Params* params, BlockNode* body);

    bool is_defined() { return true;}
    string class_name() { return "DefinedFunction"; }

    vector<Parameter*> parameters() { return params_->parameters(); }

    void dump_node(Dumper& dumper);

protected:
    Params* params_;
    BlockNode* body_;
};

class UndefinedFunction : public Function {
public:
    UndefinedFunction(TypeNode* t, const string& name, Params* params);

    bool is_defined() { return false; }
    string class_name() { return "UndefinedFunction"; }
    vector<Parameter*> parameters() { return params_->parameters(); }

    void dump_node(Dumper& dumper);

protected:
    Params* params_;
};

} // namespace cbc

#endif