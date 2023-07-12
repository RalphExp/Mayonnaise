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

bool TypeTable::is_defined(TypeRef* ref)
{
    return !!table_.count(ref);
}
    
Type* TypeTable::get(TypeRef* ref)
{
    auto it = table_.find(ref);
    if (it != table_.end())
        return it->second;

    if (ref->instanceof<UserTypeRef>()) {
        // If unregistered UserType is used in program, it causes
        // parse error instead of semantic error.  So we do not
        // need to handle this error.
        UserTypeRef* uref = (UserTypeRef*)ref;
        throw new string("undefined type: " + uref->name());

    } else if (ref->instanceof<PointerTypeRef>()) {
        PointerTypeRef* pref = (PointerTypeRef*)ref;
        Type* t = new PointerType(pointer_size_, get(pref->base_type()));
        table_[pref] = t;
        return t;
    }
    // } else if (ref->instanceof<ArrayTypeRef>()) {
    //     ArrayTypeRef* aref = (ArrayTypeRef*)ref;
    //     Type* t = new ArrayType(get(aref->base_type()), aref->length(), pointer_size_);
    //     table_[aref] = t;
    //     return t;

    // } else if (ref->instanceof<FunctionTypeRef>) {
    //     FunctionTypeRef* fref = (FunctionTypeRef*)ref;
    //     Type* t = new FunctionType(get(fref->return_type()),
    //                                 fref->params()->intern_types(this));
    //     table_[fref] = t;
    //     return t;
    // }
    throw new string("unregistered type: " + ref->to_string());
}

void TypeTable::put(TypeRef* ref, Type* t)
{
    if (table_.count(ref)) {
        throw new string("duplicated type definition: " + ref->to_string());
    }
    table_[ref] = t;
}

} // namespace cbc