#include "type_table.h"

namespace cbc {

TypeTable* TypeTable::new_table(int charsize, int shortsize, int intsize, int longsize, int ptrsize)
{
    TypeTable* table = new TypeTable(intsize, longsize, ptrsize);
    return table;
}

TypeTable::TypeTable(int int_size, int long_size, int ptr_size) :
        int_size_(int_size), long_size_(long_size), pointer_size_(ptr_size)
{
}

TypeTable::~TypeTable()
{
    for (auto p : table_) {
        p.first->dec_ref();
        p.second->dec_ref();
    }
}

bool TypeTable::is_defined(TypeRef* ref)
{
    return !!table_.count(ref);
}
    
Type* TypeTable::get(TypeRef* ref)
{
    auto it = table_.find(ref);
    if (it != table_.end()) {
        it->second->inc_ref();
        return it->second;
    }

    if (ref->instanceof<UserTypeRef>()) {
        // If unregistered UserType is used in program, it causes
        // parse error instead of semantic error.  So we do not
        // need to handle this error.
        UserTypeRef* uref = (UserTypeRef*)ref;
        throw new string("undefined type: " + uref->name());

    } else if (ref->instanceof<PointerTypeRef>()) {
        PointerTypeRef* pref = (PointerTypeRef*)ref;
        Type* t = new PointerType(pointer_size_, get(pref->base_type()));
        ref->inc_ref();
        table_[pref] = t;
        return t;

    } else if (ref->instanceof<ArrayTypeRef>()) {
        ArrayTypeRef* aref = (ArrayTypeRef*)ref;
        Type* t = new ArrayType(get(aref->base_type()), aref->length(), pointer_size_);
        ref->inc_ref();
        table_[aref] = t;
        return t;

    } else if (ref->instanceof<FunctionTypeRef>()) {
        FunctionTypeRef* fref = (FunctionTypeRef*)ref;
        Type* t = new FunctionType(get(fref->return_type()), fref->params()->intern_types(this));
        ref->inc_ref();
        table_[fref] = t;
        return t;
    }
    throw new string("unregistered type: " + ref->to_string());
}

void TypeTable::put(TypeRef* ref, Type* t)
{
    if (table_.count(ref)) {
        throw new string("duplicated type definition: " + ref->to_string());
    }
    ref->inc_ref();
    t->inc_ref();
    table_[ref] = t;
}

// array is really a pointer on parameters.
Type* TypeTable::get_param_type(TypeRef* ref)
{
    Type* t = get(ref);
    return t->is_array() ? pointer_to(t->base_type()) : t;
}

PointerType* TypeTable::pointer_to(Type* base_type)
{
    return new PointerType(pointer_size_, base_type);
}

} // namespace cbc