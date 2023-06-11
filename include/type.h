#ifndef TYPE_H_
#define TYPE_H_

#include <string>
#include <vector>
#include <memory>

#include "util.h"

using namespace std;

namespace cbc {

class Slot;
class TypeNode;
class PointerType;
class CompositeType;
class FunctionType;

class Type : public Object {
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
    virtual Type* base_type() { throw "base_type() called for undereferable type"; }

    CompositeType* get_composite_type();
    PointerType* get_pointer_type();
    FunctionType* get_function_type();
    // IntegerType* get_integer_type();
    StructType* get_struct_type();
    UnionType* get_union_type();
    ArrayType* get_array_type();
};

class TypeRef : public Object {
public:
    TypeRef() {}
    TypeRef(const Location& loc) : loc_(loc) {}

    virtual ~TypeRef() {}

    /* for hash table, see declaration.h */
    bool equal_to(TypeRef* ref) { return false; }
    Location location() { return loc_; }
    string to_string() { return ""; }

protected:
    Location loc_;
};

class VoidType : public Type {
public:
    VoidType() {}
    bool is_void() { return true; }
    long size() { return 1; }
    bool equals(Object* other);
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
    bool equals(Object* other);
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

    bool equals(Object* other);

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
    PointerTypeRef(shared_ptr<TypeRef> base);
    ~PointerTypeRef() {}
    bool is_pointer() { return true; }
    bool equals(shared_ptr<TypeRef> other);
    string to_string();

protected:
    shared_ptr<TypeRef> base_;
};

/* TODO: */
class ArrayType : public Type {
public:
    long length() { return length_; }

protected:
    long length_;
};

class ArrayTypeRef : public TypeRef {
public:
    ArrayTypeRef(shared_ptr<TypeRef> base);
    ArrayTypeRef(shared_ptr<TypeRef> base, long length);

    bool is_array() { return true; }
    bool equals(shared_ptr<TypeRef> other);
    shared_ptr<TypeRef> base_type() { return base_; }
    long length() { return length_; }
    string to_string();

    bool is_length_undefined() { return length_ == -1; }

protected:
    shared_ptr<TypeRef> base_;
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
    ~PointerType() {}
    bool is_pointer() { return true; }
    bool is_scalar() { return true; }
    bool is_signed() { return false; }
    bool is_callable() { return base_->is_function(); }
    long size() { return size_; }
    Type* base_type() { return base_; }
    bool equals(Type* type);
    bool is_same_type(Type* type);
    bool is_compatible(Type* other);
    bool is_castable_to(Type* other);
    string to_string() { return base_->to_string() + "*"; }

protected:
    long size_;
    Type* base_;
};

class CompositeType : public NamedType {
public:    
    CompositeType(const string& name, 
        shared_ptr<vector<shared_ptr<Slot>>> membs, const Location& loc);

    bool is_composite_type() { return true; }
    bool is_same_type(shared_ptr<Type> other);
    bool is_compatible(shared_ptr<Type> target);
    bool is_castable_to(shared_ptr<Type> target);
    long size();
    long alignmemt();

    shared_ptr<vector<shared_ptr<Slot>>> members();
    shared_ptr<vector<shared_ptr<Type>>> member_types(); // no pointer
    bool has_member(const string& name);
    Type* member_type(const string& name);
    long member_offset(const string& name);

protected:
    // method should be "is_same_type/is_compatible/is_castable_to"
    bool compare_member_types(shared_ptr<Type> other, const string& method);
    bool compare_types_by(const string& method, shared_ptr<Type> t, shared_ptr<Type> tt);
    virtual void compute_offsets() {};
    shared_ptr<Slot> fetch(const string& name);
    shared_ptr<Slot> get(const string& name);

protected:
    shared_ptr<vector<shared_ptr<Slot>>> members_;
    long cached_size_;
    long cached_align_;
    bool is_recursive_checked_;
};

class StructType : public CompositeType {
public:
    StructType(const string& name, vector<Slot*>&& membs, const Location& loc);
    bool is_struct() { return true; }
    string to_string() { return "struct " + name_; }
    bool is_same_type(shared_ptr<Type> other);
    void compute_offsets();
};

class StructTypeRef : public TypeRef {
public:
    StructTypeRef(const string& name);
    StructTypeRef(const Location& loc, const string& name);
    bool is_struct() { return true; }
    string name() { return name_; }
    bool equals(shared_ptr<TypeRef> other);

protected:
    string name_;
};

class UnionType : public CompositeType {
public:
    UnionType(const string& name, vector<Slot*>&& membs, const Location& loc);
    bool is_union() { return true; }
    bool is_same_type(shared_ptr<Type> other);
    void compute_offsets();
    string to_string() { return "union " + name_; }
};

class UnionTypeRef : public TypeRef {
public:
    UnionTypeRef(const string& name);
    UnionTypeRef(const Location& loc, const string& name);
    bool is_union() { return true; }
    bool equals(shared_ptr<TypeRef> other);
    string name() { return name_; }
    string to_string() { return "union " + name_; }

protected:
    string name_;
};

class UserTypeRef : public TypeRef {
public:
    UserTypeRef(const string& name);
    UserTypeRef(const Location& loc, const string& name);
    bool is_user_type() { return true; }
    bool equals(shared_ptr<TypeRef> other);
    string name() { return name_; }
    string to_string() { return name_; }

protected:
    string name_;
};

// TODO:
class UserType : public NamedType {
public:
    UserType(const string& name, TypeNode* real, const Location& loc);
    ~UserType();

    bool is_same_type(Type* other) { throw "not implement"; }

protected:
    TypeNode* real_;
};

template<typename T>
class ParamSlots {
public:
    ParamSlots(shared_ptr<vector<shared_ptr<T>>> param_descs) :
            param_descs_(param_descs), vararg_(false)
    {
    }

    ParamSlots(const Location& loc, shared_ptr<vector<shared_ptr<T>>> param_descs,
            bool vararg=false) :
        loc_(loc), param_descs_(param_descs), vararg_(vararg)
    {
    }

    ~ParamSlots() {}

    int argc() {
        if (vararg_) {
            throw string("must not happen: Param::argc for vararg");
        }
        return param_descs_->size();
    }

    int min_argc() {
        return param_descs_->size();
    }

    void accept_varargs() {
        vararg_ = true;
    }

    bool is_vararg() {
        return vararg_;
    }

    Location location() {
        return loc_;
    }

protected:
    Location loc_;
    shared_ptr<vector<shared_ptr<T>>> param_descs_;
    bool vararg_;
};

class ParamTypeRefs : public ParamSlots<TypeRef> {
public:
    ParamTypeRefs(pv_typeref param_descs);
    ParamTypeRefs(const Location& loc, pv_typeref paramDescs, bool vararg);

    // TODO:
    // ParamTypes internTypes(TypeTable table);

    pv_typeref typerefs() { return param_descs_; }
    bool equals(shared_ptr<ParamTypeRefs> other);
};

class ParamTypes : public ParamSlots<Type> {
protected:
    ParamTypes(const Location& loc, pv_type param_descs, bool vararg);

public:
    pv_type types() { return param_descs_; }
    bool is_same_type(shared_ptr<ParamTypes> other);
    bool equals(shared_ptr<Type> other);
};

class FunctionTypeRef : public TypeRef {
public:
    FunctionTypeRef(shared_ptr<TypeRef> return_type, shared_ptr<ParamTypeRefs> params);
    bool is_function() { return true; }
    bool equals(shared_ptr<TypeRef> other);
    shared_ptr<TypeRef> return_type() { return return_type_; }
    shared_ptr<ParamTypeRefs> params() { return params_;}
    string to_string();

protected:
    shared_ptr<TypeRef> return_type_;
    shared_ptr<ParamTypeRefs> params_;
};

class FunctionType : public Type {
public:
    FunctionType(Type* ret, ParamTypes* partypes);
    ~FunctionType();

    bool is_function() { return true; }
    bool is_callable() { return true; }
    Type* return_type() { return return_type_; }

protected: 
    Type* return_type_;
    ParamTypes* param_types_;
};

} // namespace cbc
#endif
