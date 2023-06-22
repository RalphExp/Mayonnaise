#ifndef TYPE_H_
#define TYPE_H_

#include <string>
#include <vector>

#include "object.h"
#include "util.h"

using namespace std;

namespace cbc {

class Slot;
class TypeNode;
class PointerType;
class CompositeType;
class FunctionType;
class IntegerType;
class StructType;
class UnionType;
class ArrayType;

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
    IntegerType* get_integer_type();
    StructType* get_struct_type();
    UnionType* get_union_type();
    ArrayType* get_array_type();
};

class TypeRef : public Object {
public:
    TypeRef() {}
    TypeRef(const Location& loc) : loc_(loc) {}

    virtual ~TypeRef() {}

    Location location() { return loc_; }
    virtual string to_string() { return ""; }

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

class IntegerType : public Type {
public:
    IntegerType(long size, bool is_signed, const string& name);
    bool is_same_type(Type* other);

    bool is_integer() { return true; }
    bool is_signed() { return is_signed_; }
    bool is_scalar() { return true; }
    bool is_compatible(Type* other);
    bool is_castable_to(Type* target);
    long min_value();
    long max_value();
    bool is_indomain(long i) { return min_value() <= i && i <= max_value(); }
    long size() { return size_; }
    string to_string() { return name_; }
    
protected:
    long size_;
    bool is_signed_;
    string name_;  
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

class PointerTypeRef : public TypeRef {
public:
    PointerTypeRef(TypeRef* base);
    ~PointerTypeRef();

    bool is_pointer() { return true; }
    bool equals(Object* other);

    TypeRef* base_type() { return base_type_; }
    string to_string();

protected:
    TypeRef* base_type_;
};

class PointerType : public Type {
public:
    PointerType(long size, Type* base);
    ~PointerType();
    bool is_pointer() { return true; }
    bool is_scalar() { return true; }
    bool is_signed() { return false; }
    bool is_callable() { return base_type_->is_function(); }
    long size() { return size_; }
    Type* base_type() { return base_type_; }
    bool equals(Object* type);
    bool is_same_type(Type* type);
    bool is_compatible(Type* other);
    bool is_castable_to(Type* other);
    string to_string() { return base_type_->to_string() + "*"; }

protected:
    long size_;
    Type* base_type_;
};

class CompositeType : public NamedType {
public:    
    CompositeType(const string& name, 
        vector<Slot*>&& membs, const Location& loc);

    ~CompositeType();

    bool is_composite_type() { return true; }
    bool is_same_type(Type* other);
    bool is_compatible(Type* target);
    bool is_castable_to(Type* target);
    long size();
    long alignmemt();

    vector<Slot*> members();
    vector<Type*> member_types();
    bool has_member(const string& name);
    Type* member_type(const string& name);
    long member_offset(const string& name);

protected:
    // method should be "is_same_type/is_compatible/is_castable_to"
    bool compare_member_types(Type* other, const string& method);
    bool compare_types_by(const string& method, Type* t, Type* tt);
    virtual void compute_offsets() {};
    Slot* fetch(const string& name);
    Slot* get(const string& name);

protected:
    vector<Slot*> members_;
    long cached_size_;
    long cached_align_;
    bool is_recursive_checked_;
};

class StructType : public CompositeType {
public:
    StructType(const string& name, vector<Slot*>&& membs, const Location& loc);
    bool is_struct() { return true; }
    string to_string() { return "struct " + name_; }
    bool is_same_type(Type* other);
    void compute_offsets();
};

class StructTypeRef : public TypeRef {
public:
    StructTypeRef(const string& name);
    StructTypeRef(const Location& loc, const string& name);
    bool is_struct() { return true; }
    string name() { return name_; }
    string to_string() { return "struct " + name_; }
    bool equals(Object* other);

protected:
    string name_;
};

class UnionType : public CompositeType {
public:
    UnionType(const string& name, vector<Slot*>&& membs, const Location& loc);
    bool is_union() { return true; }
    bool is_same_type(Type* other);
    void compute_offsets();
    string to_string() { return "union " + name_; }
};

class UnionTypeRef : public TypeRef {
public:
    UnionTypeRef(const string& name);
    UnionTypeRef(const Location& loc, const string& name);
    bool is_union() { return true; }
    bool equals(Object* other);
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
    bool equals(Object* other);
    string name() { return name_; }
    string to_string() { return name_; }

protected:
    string name_;
};


class UserType : public NamedType {
public:
    UserType(const string& name, TypeNode* real, const Location& loc);
    ~UserType();

    Type* real_type();
    long size() { return real_type()->size(); }
    long alloc_size() { return real_type()->alloc_size(); }
    long alignment() { return real_type()->alignment(); }
    bool is_void() { return real_type()->is_void(); }
    bool is_int() { return real_type()->is_int(); }
    bool is_integer() { return real_type()->is_integer(); }
    bool is_signed() { return real_type()->is_signed(); }
    bool is_pointer() { return real_type()->is_pointer(); }
    bool is_array() { return real_type()->is_array(); }
    bool is_allocated_array() { return real_type()->is_allocated_array(); }
    bool is_composite_type() { return real_type()->is_composite_type(); }
    bool is_struct() { return real_type()->is_struct(); }
    bool is_union() { return real_type()->is_union(); }
    bool is_userType() { return true; }
    bool is_function() { return real_type()->is_function(); }
    bool is_callable() { return real_type()->is_callable(); }
    bool is_scalar() { return real_type()->is_scalar(); }
    Type* base_type() { return real_type()->base_type(); }

    bool is_same_type(Type* other) { return real_type()->is_same_type(other); }
    bool is_compatible(Type* other) { return real_type()->is_compatible(other); }
    bool is_castableTo(Type* other) { return real_type()->is_castable_to(other); }
    string to_string() { return name_; }

    CompositeType* get_composite_type() { return real_type()->get_composite_type(); }
    PointerType* get_pointer_type() { return real_type()->get_pointer_type(); }
    FunctionType* get_function_type() { return real_type()->get_function_type(); }
    IntegerType* get_integer_type() { return real_type()->get_integer_type(); } 
    StructType* get_struct_type() { return real_type()->get_struct_type(); }
    UnionType* get_union_type() { return real_type()->get_union_type(); }
    ArrayType* get_array_type() { return real_type()->get_array_type(); }

protected:
    TypeNode* real_;
};

class ArrayTypeRef : public TypeRef {
public:
    ArrayTypeRef(TypeRef* base);
    ArrayTypeRef(TypeRef* base, long length);
    ~ArrayTypeRef();

    bool is_array() { return true; }
    bool equals(Object* other);

    TypeRef* base_type() { return base_type_; }
    long length() { return length_; }
    string to_string();

    bool is_length_undefined() { return length_ == -1; }

protected:
    TypeRef* base_type_;
    long length_;
};

class ArrayType : public Type {
public:
    ArrayType(Type* base_type, long pointerSize);
    ArrayType(Type* base_type, long length, long pointerSize);
    ~ArrayType();

    bool is_array() { return true; }
    bool is_allocated_array();
    Type* base_type() { return base_type_; }
    long length() { return length_; }

    // About incomplete array:
    //   e.g. int a[3] is complete,
    //        int a[] is complete,
    //        int a[][3] is complete,
    //    but int a[3][] is *not* complete

    bool is_incomplete_array();
    long size() { return pointer_size_; }
    long alloc_size();
    long alignment() { return base_type_->alignment(); }

    string to_string();

protected:
    Type* base_type_;
    long pointer_size_;
    long length_;
};


template<typename T>
class ParamSlots : public Object {
public:
    ParamSlots(vector<T*>&& param_descs) :
            param_descs_(move(param_descs)), vararg_(false)
    {
        for (T* t : param_descs_) {
            t->inc_ref();
        }
    }

    ParamSlots(const Location& loc, vector<T*>&& param_descs,
            bool vararg=false) :
        loc_(loc), param_descs_(move(param_descs)), vararg_(vararg)
    {
        for (T* t : param_descs_) {
            t->inc_ref();
        }
    }

    ~ParamSlots() {
        for (T* t : param_descs_) {
            t->dec_ref();
        }
    }

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

public:
    vector<T*> param_descs_;

protected:
    Location loc_;
    bool vararg_;
};

class ParamTypeRefs : public ParamSlots<TypeRef> {
public:
    ParamTypeRefs(vector<TypeRef*>&& param_descs);
    ParamTypeRefs(const Location& loc, vector<TypeRef*>&&  paramDescs, bool vararg);

    // TODO:
    // ParamTypes internTypes(TypeTable table);

    vector<TypeRef*> typerefs() { return param_descs_; }
    bool equals(Object* other);
    bool equals(ParamTypeRefs* other);
};

class ParamTypes : public ParamSlots<Type> {
protected:
    ParamTypes(const Location& loc, vector<Type*>&& param_descs, bool vararg);

public:
    vector<Type*> types() { return param_descs_; }
    bool is_same_type(ParamTypes* other);
    bool equals(Object* other);
    bool equals(ParamTypes* other);
};

class FunctionTypeRef : public TypeRef {
public:
    FunctionTypeRef(TypeRef* return_type, ParamTypeRefs* params);
    ~FunctionTypeRef();

    bool is_function() { return true; }
    bool equals(Object* other);
    bool equals(FunctionTypeRef* other);
    TypeRef* return_type() { return return_type_; }
    ParamTypeRefs* params() { return params_;}
    string to_string();

protected:
    TypeRef* return_type_;
    ParamTypeRefs* params_;
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
