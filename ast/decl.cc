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

    for_each(decls->declvars_.begin(), decls->declvars_.end(), [this](UndefinedVariable* v) {
        if (declvars_.count(v))
            return;
        v->inc_ref();
        declvars_.insert(v);
    });

    for_each(decls->defuncs_.begin(),  decls->defuncs_.end(), [this](DefinedFunction* v) {
        if (defuncs_.count(v))
            return;
        v->inc_ref();
        defuncs_.insert(v);
    });

    for_each(decls->declfuncs_.begin(), decls->declfuncs_.end(), [this](UndefinedFunction* v) {
        if (declfuncs_.count(v))
            return;
        v->inc_ref();
        declfuncs_.insert(v);
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
    for_each(declvars_.begin(), declvars_.end(), [this](UndefinedVariable* v) { v->dec_ref(); });
    for_each(defuncs_.begin(), defuncs_.end(), [this](DefinedFunction* v) { v->dec_ref(); });
    for_each(declfuncs_.begin(), declfuncs_.end(), [this](UndefinedFunction* v) { v->dec_ref(); });
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

        // XXX: move constructor don't increase ref!!
        defvars_.insert(v);
    });
}

vector<DefinedVariable*> Declarations::defvars()
{
    vector<DefinedVariable*> v;
    copy(defvars_.begin(), defvars_.end(), inserter(v, v.end()));
    return v;
}

void Declarations::add_declvar(UndefinedVariable* var)
{
    if (!declvars_.count(var)) {
        var->inc_ref();
        declvars_.insert(var);
    }
}

vector<UndefinedVariable*> Declarations::declvars()
{
    vector<UndefinedVariable*> v;
    copy(declvars_.begin(), declvars_.end(), inserter(v, v.end()));
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

void Declarations::add_deffunc(DefinedFunction* func)
{
    if (!defuncs_.count(func)) {
        func->inc_ref();
        defuncs_.insert(func);
    }
}

vector<DefinedFunction*> Declarations::deffuncs()
{
    vector<DefinedFunction*> v;
    copy(defuncs_.begin(), defuncs_.end(), inserter(v, v.end()));
    return v;
}

void Declarations::add_declfunc(UndefinedFunction* func)
{
    if (!declfuncs_.count(func)) {
        func->inc_ref();
        declfuncs_.insert(func);
    }
}

vector<UndefinedFunction*> Declarations::declfuncs()
{
    vector<UndefinedFunction*> v;
    copy(declfuncs_.begin(), declfuncs_.end(), inserter(v, v.end()));
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