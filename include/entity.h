#ifndef ENTRY_H_
#define ENTRY_H_

#include <string>

using namespace std;

#include "util.h"

namespace may {

class Type;
class TypeNode;
class ExprNode;

class Entity {
public:
    Entity(bool priv, TypeNode* type, const string& name);
    virtual ~Entity() {}
    
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
    virtual void dump_node(Dumper& dumper) = 0;

protected:
    string name_;
    bool priv_;
    TypeNode* tnode_;
    long nref_;
    // MemoryReference
    // Operand
};

class Variable : public Entity {
public:

};

class DefinedVariable : Variable {
public:
};

class ConstantEntry {

};

}

#endif