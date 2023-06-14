#include "decl.h"

#include <algorithm>

namespace cbc {

void Declarations::add(Declarations* decls)
{
    for_each(decls->defvars_.begin(), decls->defvars_.end(), [this](DefinedVariable* v) {
        if (defvars_.count(v))
            return;
        v->inc_ref();
        defvars_.insert(v);
    });

    for_each(decls->vardecls_.begin(), decls->vardecls_.end(), [this](UndefinedVariable* v) {
        if (vardecls_.count(v))
            return;
        v->inc_ref();
        vardecls_.insert(v);
    });

    for_each(decls->defuns_.begin(),  decls->defuns_.end(), [this](DefinedFunction* v) {
        if (defuns_.count(v))
            return;
        v->inc_ref();
        defuns_.insert(v);
    });

    for_each(decls->funcdecls_.begin(), decls->funcdecls_.end(), [this](UndefinedFunction* v) {
        if (funcdecls_.count(v))
            return;
        v->inc_ref();
        funcdecls_.insert(v);
    });
   
    for_each(decls->constants_.begin(), decls->constants_.end(), [this](Constant* v) {
        if (constants_.count(v))
            return;
        v->inc_ref();
        constants_.insert(v);
    });

    for_each(decls->defstructs_.begin(), decls->defstructs_.end(), [this](StructNode* v) {
        if (defstructs_.count(v))
            return;
        v->inc_ref();
        defstructs_.insert(v);
    });
 
    for_each(decls->defunions_.begin(), decls->defunions_.end(), [this](UnionNode* v) {
        if (defunions_.count(v))
            return;
        v->inc_ref();
        defunions_.insert(v);
    });

    for_each(decls->typedefs_.begin(), decls->typedefs_.end(), [this](TypedefNode* v) {
        if (typedefs_.count(v))
            return;
        v->inc_ref();
        typedefs_.insert(v);
    });
}

Declarations::~Declarations()
{
    for_each(defvars_.begin(), defvars_.end(), [this](DefinedVariable* v) { v->dec_ref(); });
    for_each(vardecls_.begin(), vardecls_.end(), [this](UndefinedVariable* v) { v->dec_ref(); });
    for_each(defuns_.begin(), defuns_.end(), [this](DefinedFunction* v) { v->dec_ref(); });
    for_each(funcdecls_.begin(), funcdecls_.end(), [this](UndefinedFunction* v) { v->dec_ref(); });
    for_each(constants_.begin(), constants_.end(), [this](Constant* v) { v->dec_ref(); });
    for_each(defstructs_.begin(), defstructs_.end(), [this](StructNode* v) { v->dec_ref(); });
    for_each(defunions_.begin(), defunions_.end(), [this](UnionNode* v) { v->dec_ref(); });
    for_each(typedefs_.begin(), typedefs_.end(), [this](TypedefNode* v) { v->dec_ref(); });
}

void Declarations::add_defvar(DefinedVariable* var)
{
    if (!defvars_.count(var)) {
        var->inc_ref();
        defvars_.insert(var);
    }
}
    
void Declarations::add_defvars(vector<DefinedVariable*>&& vars)
{
    for_each(vars.begin(), vars.end(), [this](DefinedVariable* v) {
        if (defvars_.count(v))
            return;
        v->inc_ref();
        defvars_.insert(v);
    });
}

vector<DefinedVariable*> Declarations::defvars()
{
    vector<DefinedVariable*> v;
    copy(defvars_.begin(), defvars_.end(), inserter(v, v.end()));
    return v;
}

void Declarations::add_vardecl(UndefinedVariable* var)
{
    if (!vardecls_.count(var)) {
        var->inc_ref();
        vardecls_.insert(var);
    }
}

vector<UndefinedVariable*> Declarations::vardecls()
{
    vector<UndefinedVariable*> v;
    copy(vardecls_.begin(), vardecls_.end(), inserter(v, v.end()));
    return v;
}

void Declarations::add_constant(Constant* c)
{
    if (!constants_.count(c)) {
        c->inc_ref();
        constants_.insert(c);
    }
}

vector<Constant*> Declarations::constants()
{
    vector<Constant*> v;
    copy(constants_.begin(), constants_.end(), inserter(v, v.end()));
    return v;
}

void Declarations::add_defun(DefinedFunction* func)
{
    if (!defuns_.count(func)) {
        func->inc_ref();
        defuns_.insert(func);
    }
}

vector<DefinedFunction*> Declarations::defuns()
{
    vector<DefinedFunction*> v;
    copy(defuns_.begin(), defuns_.end(), inserter(v, v.end()));
    return v;
}

void Declarations::add_funcdecls(UndefinedFunction* func)
{
    if (!funcdecls_.count(func)) {
        func->inc_ref();
        funcdecls_.insert(func);
    }
}

vector<UndefinedFunction*> Declarations::funcdecls()
{
    vector<UndefinedFunction*> v;
    copy(funcdecls_.begin(), funcdecls_.end(), inserter(v, v.end()));
    return v;
}

void Declarations::add_defstruct(StructNode* n)
{
    if (!defstructs_.count(n)) {
        n->inc_ref();
        defstructs_.insert(n);
    }
}

vector<StructNode*> Declarations::defstructs()
{
    vector<StructNode*> v;
    copy(defstructs_.begin(), defstructs_.end(), inserter(v, v.end()));
    return v;
}

void Declarations::add_defunion(UnionNode* n)
{
    if (!defunions_.count(n)) {
        n->inc_ref();
        defunions_.insert(n);
    }
}

vector<UnionNode*> Declarations::defunions()
{
    vector<UnionNode*> v;
    copy(defunions_.begin(), defunions_.end(), inserter(v, v.end()));
    return v;
}

void Declarations::add_typedef(TypedefNode* n)
{
    if (!typedefs_.count(n)) {
        n->inc_ref();
        typedefs_.insert(n);
    }
}

vector<TypedefNode*> Declarations::typedefs()
{
    vector<TypedefNode*> v;
    copy(typedefs_.begin(), typedefs_.end(), inserter(v, v.end()));
    return v;
}

} // namespace cbc