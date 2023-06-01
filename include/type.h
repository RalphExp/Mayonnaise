#ifndef TYPE_H_
#define TYPE_H_

#include <string>
#include <vector>

#include "util.h"

using namespace std;

namespace ast {

class Slot;
class PointerType;
class CompositeType;

class Type {
public:
    static const long kSizeUnknown = -1;
    virtual ~Type() {}
    virtual long size() { return 0;};
    virtual long alloc_size() { return size(); }
    virtual long alignment() { return alloc_size(); }
    virtual bool is_same_type(Type* other) = 0;
    virtual bool is_void() { return false; }
    virtual bool is_int() { return false; }
    virtual bool is_integer() { return false; }
    virtual bool is_signed() { throw "issigned for non-integer type "; }
    virtual bool is_pointer() { return false; }
    virtual bool is_array() { return false; }
    virtual bool is_composite_type() { return false; }
    virtual bool is_struct() { return false; }
    virtual bool is_union() { return false; }
    virtual bool is_user_type() { return false; }
    virtual bool is_function() { return false; }
    virtual bool is_allocated_array() { return false; }
    virtual bool is_incomplete_array() { return false; }
    virtual bool is_scalar() { return false; }
    virtual bool is_callable() { return false; }
    virtual bool is_compatible(Type* other) { return false; };
    virtual bool is_castable_to(Type* other) { return false; };
    virtual string to_string() { return ""; }
    virtual bool equals(Type* other) { return this == other; }
    virtual Type* base_type() { throw "base_type() called for undereferable type"; }

    CompositeType* get_composite_type();
    PointerType* get_pointer_type();
};

class TypeRef {
public:
    TypeRef() {}
    TypeRef(const TypeRef& ref) : loc_(ref.loc_) {}
    TypeRef(const Location& loc) : loc_(loc) {}
    virtual ~TypeRef() {}
    bool equals(TypeRef* ref) { return false; }

    Location location() { return loc_; }
    string to_string() { return ""; }

     /* TODO: */
    int hash_code() { return 0; }

protected:
    Location loc_;
};

class VoidType : public Type {
public:
    VoidType() {}
    bool is_void() { return true; }
    long size() { return 1; }
    bool equals(Type* other) { return !!dynamic_cast<VoidType*>(other); }
    bool is_same_type(Type* other) { return other->is_void(); }
    bool is_compatible(Type* other) { return other->is_void(); }
    bool is_castable_to(Type* other) { return other->is_void(); }
    string to_string() { return "void"; }
};

class VoidTypeRef : public TypeRef {
public:
    VoidTypeRef() {}
    VoidTypeRef(const Location &loc) : TypeRef(loc) {}
    bool is_void() { return true; }
    bool equals(TypeRef* other) { return !!dynamic_cast<VoidTypeRef*>(other); }
    string to_string() { return "void"; }
};

class IntegerTypeRef : public TypeRef {
public:
    IntegerTypeRef(const string& name) 
        : name_(name), TypeRef(Location()) {}
    IntegerTypeRef(const string& name, const Location& loc) : 
        name_(name), TypeRef(loc) {}
    
    ~IntegerTypeRef() {}

    string name() { return name_; }
    string to_string() { return name_; }

    bool equals(TypeRef* other);

    static IntegerTypeRef* char_ref(const Location& loc);
    static IntegerTypeRef* char_ref();
    static IntegerTypeRef* short_ref(const Location& loc);
    static IntegerTypeRef* short_ref();
    static IntegerTypeRef* int_ref(const Location& loc);
    static IntegerTypeRef* int_ref();
    static IntegerTypeRef* long_ref(const Location& loc);
    static IntegerTypeRef* long_ref();
    static IntegerTypeRef* uchar_ref(const Location& loc);
    static IntegerTypeRef* uchar_ref();
    static IntegerTypeRef* ushort_ref(const Location& loc);
    static IntegerTypeRef* ushortRef();
    static IntegerTypeRef* uint_ref(const Location& loc);
    static IntegerTypeRef* uint_ref();
    static IntegerTypeRef* ulong_ref(const Location& loc);
    static IntegerTypeRef* ulong_ref();

protected:
    string name_;
};

class PointerTypeRef : public TypeRef {
public:
    PointerTypeRef(TypeRef* base);
    ~PointerTypeRef() { delete base_; }
    bool is_pointer() { return true; }
    bool equals(TypeRef* other);
    string to_string();

protected:
    TypeRef* base_;
};

/* TODO: */
class ArrayType : public Type {
public:
    long length() { return length_; }

protected:
    long length_;
};

class NamedType : public Type {
public:
    NamedType(const string& name, const Location& loc);
    string name() { return name_; }
    Location location() { return loc_; }
protected:
    string name_;
    Location loc_;
};


class PointerType : public Type {
public:
    PointerType(long size, Type* base);
    ~PointerType() { delete base_; }
    bool is_pointer() { return true; }
    bool is_scalar() { return true; }
    bool is_signed() { return false; }
    bool is_callable() { return base_->is_function(); }
    long size() { return size_; }
    Type* base_type() { return base_; }
    bool equals(Type* type);
    bool is_same_type(Type* type);
    bool is_compatible(Type* other);
    string to_string() { return base_->to_string() + "*"; }

protected:
    long size_;
    Type* base_;
};

class CompositeType : public NamedType {
public:
    CompositeType(const string& name, const vector<Slot>& membs, const Location& loc);
    CompositeType(const string& name, vector<Slot>&& membs, const Location& loc);
    bool is_composite_type() { return true; }
    bool is_same_type(Type* other);
    bool is_compatible(Type* target);
    bool is_castable_to(Type* target);
    long size();
    long alignmemt();
    vector<Slot> members();
    vector<Type*> member_types();
    bool has_member(const string& name);
    Type* member_type(const string& name);
    long member_offset(const string& name);

protected:
    // method should be "is_same_type/is_compatible/is_castable_to"
    bool compare_member_types(Type* other, const string& method);
    bool compare_types_by(const string& method, Type* t, Type* tt);
    virtual void compute_offsets() {};
    Slot fetch(const string& name);
    Slot get(const string& name);

protected:
    vector<Slot> members_;
    long cached_size_;
    long cached_align_;
    bool is_recursive_checked_;
};

}
#endif