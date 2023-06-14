#ifndef DECL_H_
#define DECL_H_

#include <unordered_set>

#include "entity.h"
#include "node.h"

namespace cbc {
class Declarations {
public:
    ~Declarations();
    void add(Declarations* decls);

    void add_defvar(DefinedVariable* var);
    void add_defvars(vector<DefinedVariable*>&& vars);
    vector<DefinedVariable*> defvars();

    void add_vardecl(UndefinedVariable* var);
    vector<UndefinedVariable*> vardecls();

    void add_constant(Constant* c);
    vector<Constant*> constants();

    void add_defun(DefinedFunction* func);
    vector<DefinedFunction*> defuns();

    void add_funcdecls(UndefinedFunction* func);
    vector<UndefinedFunction*> funcdecls();

    void add_defstruct(StructNode* n);
    vector<StructNode*> defstructs();

    void add_defunion(UnionNode* n);
    vector<UnionNode*> defunions();

    void add_typedef(TypedefNode* n);
    vector<TypedefNode*> typedefs();
    
protected:
    unordered_set<DefinedVariable*> defvars_;
    unordered_set<UndefinedVariable*> vardecls_;
    unordered_set<DefinedFunction*> defuns_;
    unordered_set<UndefinedFunction*> funcdecls_;
    unordered_set<Constant*> constants_;
    unordered_set<StructNode*> defstructs_;
    unordered_set<UnionNode*> defunions_;
    unordered_set<TypedefNode*> typedefs_;
};

}

#endif