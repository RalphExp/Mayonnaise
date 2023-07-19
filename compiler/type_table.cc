#include "type_table.h"

namespace cbc {

TypeTable::TypeTable(int int_size, int long_size, int ptr_size) :
        int_size_(int_size), long_size_(long_size), pointer_size_(ptr_size)
{   
}

TypeTable* TypeTable::new_table(int charsize, int shortsize, int intsize, int longsize, int ptrsize)
{
    TypeTable* table = new TypeTable(intsize, longsize, ptrsize);
    table->put(new VoidTypeRef(), new VoidType());
    table->put(IntegerTypeRef::char_ref(), new IntegerType(charsize, true, "char"));
    table->put(IntegerTypeRef::short_ref(), new IntegerType(shortsize, true, "short"));
    table->put(IntegerTypeRef::int_ref(), new IntegerType(intsize, true, "int"));
    table->put(IntegerTypeRef::long_ref(), new IntegerType(longsize, true, "long"));
    table->put(IntegerTypeRef::uchar_ref(), new IntegerType(charsize, false, "unsigned char"));
    table->put(IntegerTypeRef::ushort_ref(), new IntegerType(shortsize, false, "unsigned short"));
    table->put(IntegerTypeRef::uint_ref(), new IntegerType(intsize, false, "unsigned int"));
    table->put(IntegerTypeRef::ulong_ref(), new IntegerType(longsize, false, "unsigned long"));
    return table;
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
    return table_.count(ref);
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
    if (t->is_array()) {
        t->dec_ref();
        return pointer_to(t->base_type());
    }
    return t;
}

PointerType* TypeTable::pointer_to(Type* base_type)
{
    return new PointerType(pointer_size_, base_type);
}

string TypeTable::ptr_diff_type_name()
{
    auto *sl = signed_long();
    if (sl->size() == pointer_size_) {
        sl->dec_ref();
        return "long";
    }
    sl->dec_ref();

    sl = signed_int();
    if (sl->size() == pointer_size_) {
        sl->dec_ref();
        return "int";
    }
    sl->dec_ref();

    sl = signed_short();
    if (sl->size() == pointer_size_) {
        sl->dec_ref();
        return "short";
    }
    sl->dec_ref();
    throw string("must not happen: integer.size != pointer.size");
}

vector<Type*> TypeTable::types()
{
    vector<Type*> v;
    for (auto& p : table_) {
        p.second->inc_ref();
        v.push_back(p.second);
    }
    return v;
}
    
VoidType* TypeTable::void_type()
{
    auto* ref = new VoidTypeRef();
    auto* tp = get(ref);
    tp->inc_ref();
    ref->dec_ref();
    return (VoidType*)tp;
}

IntegerType* TypeTable::signed_char()
{
    auto* ref = IntegerTypeRef::char_ref();
    auto* tp = get(ref);
    tp->inc_ref();
    ref->dec_ref();
    return (IntegerType*)tp;
}

IntegerType* TypeTable::signed_short()
{
    auto* ref = IntegerTypeRef::short_ref();
    auto* tp = get(ref);
    tp->inc_ref();
    ref->dec_ref();
    return (IntegerType*)tp;
}

IntegerType* TypeTable::signed_int()
{
    auto* ref = IntegerTypeRef::int_ref();
    auto* tp = get(ref);
    tp->inc_ref();
    ref->dec_ref();
    return (IntegerType*)tp;
}

IntegerType* TypeTable::signed_long()
{
    auto* ref = IntegerTypeRef::long_ref();
    auto* tp = get(ref);
    tp->inc_ref();
    ref->dec_ref();
    return (IntegerType*)tp;
}

IntegerType* TypeTable::unsigned_char()
{
    auto* ref = IntegerTypeRef::uchar_ref();
    auto* tp = get(ref);
    tp->inc_ref();
    ref->dec_ref();
    return (IntegerType*)tp;
}

IntegerType* TypeTable::unsigned_short()
{
    auto* ref = IntegerTypeRef::ushort_ref();
    auto* tp = get(ref);
    tp->inc_ref();
    ref->dec_ref();
    return (IntegerType*)tp;
}

IntegerType* TypeTable::unsigned_int()
{
    auto* ref = IntegerTypeRef::uint_ref();
    auto* tp = get(ref);
    tp->inc_ref();
    ref->dec_ref();
    return (IntegerType*)tp;
}

IntegerType* TypeTable::unsigned_long()
{
    auto* ref = IntegerTypeRef::ulong_ref();
    auto* tp = get(ref);
    tp->inc_ref();
    ref->dec_ref();
    return (IntegerType*)tp;
}

} // namespace cbc