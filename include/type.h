#ifndef TYPE_H_
#define TYPE_H_

#include <string>

#include "util.h"

using namespace std;

namespace ast {

class Type {
public:
    virtual long size() const { return 0;};
    virtual long alloc_size() const { return size(); }
    virtual long alignment() const { return alloc_size(); }
    virtual bool is_void() const { return false; }
    virtual bool is_int() const { return false; }
    virtual bool is_integer() const { return false; }
    virtual bool is_signed() const { throw "issigned for non-integer type "; }
    virtual bool is_pointer() const { return false; }
    virtual bool is_array() const { return false; }
    virtual bool is_composite_type() const { return false; }
    virtual bool is_struct() const { return false; }
    virtual bool is_union() const { return false; }
    virtual bool is_user_type() const { return false; }
    virtual bool is_function() const { return false; }
    virtual bool is_allocated_array() const { return false; }
    virtual bool is_incomplete_array() const { return false; }
    virtual bool is_scalar() const { return false; }
    virtual bool is_callable() const { return false; }
    virtual bool is_compatible(const Type& other) { return false; };
    virtual bool is_castable_to(const Type& other) { return false; };
    virtual string to_string() { return ""; }
    virtual Type* base_type() { throw "base_type() called for undereferable type"; }
};

class TypeRef {
public:
    TypeRef() {}
    TypeRef(const TypeRef& ref) : loc_(ref.loc_) {}
    TypeRef(const Location& loc) : loc_(loc) {}
    virtual ~TypeRef() {}
    bool equals(TypeRef* ref) { return false; }

    Location location() const { return loc_; }
    string to_string() { return ""; }

     /* TODO: */
    int hash_code() { return 0; }

protected:
    Location loc_;
};

class IntegerTypeRef : public TypeRef {
public:
    IntegerTypeRef(const string& name) : name_(name), TypeRef(Location()) {}
    IntegerTypeRef(const string& name, const Location& loc) : name_(name), TypeRef(loc) {}
    ~IntegerTypeRef() {}

    string name() { return name_; }
    string to_string() { return name_; }

    bool equals(TypeRef* other);

    static IntegerTypeRef* char_ref(const Location& loc) {
        return new IntegerTypeRef("char", loc);
    }   

    static IntegerTypeRef* char_ref() {
        return new IntegerTypeRef("char");
    }   

    static IntegerTypeRef* short_ref(const Location& loc) {
        return new IntegerTypeRef("short", loc);
    }   

    static IntegerTypeRef* short_ref() {
        return new IntegerTypeRef("short");
    }   

    static IntegerTypeRef* int_ref(const Location& loc) {
        return new IntegerTypeRef("int", loc);
    }   

    static IntegerTypeRef* int_ref() {
        return new IntegerTypeRef("int");
    }   

    static IntegerTypeRef* long_ref(const Location& loc) {
        return new IntegerTypeRef("long", loc);
    }   

    static IntegerTypeRef* long_ref() {
        return new IntegerTypeRef("long");
    }

    static IntegerTypeRef* uchar_ref(const Location& loc) {
        return new IntegerTypeRef("unsigned char", loc);
    }

    static IntegerTypeRef* uchar_ref() {
        return new IntegerTypeRef("unsigned char");
    }

    static IntegerTypeRef* ushort_ref(const Location& loc) {
        return new IntegerTypeRef("unsigned short", loc);
    }

    static IntegerTypeRef* ushortRef() {
        return new IntegerTypeRef("unsigned short");
    }

    static IntegerTypeRef* uint_ref(const Location& loc) {
        return new IntegerTypeRef("unsigned int", loc);
    }

    static IntegerTypeRef* uint_ref() {
        return new IntegerTypeRef("unsigned int");
    }

    static IntegerTypeRef* ulong_ref(const Location& loc) {
        return new IntegerTypeRef("unsigned long", loc);
    }

    static IntegerTypeRef* ulong_ref() {
        return new IntegerTypeRef("unsigned long");
    }

protected:
    string name_;
};

class PointerTypeRef : public TypeRef {
public:
    PointerTypeRef(TypeRef* base);
    ~PointerTypeRef() { delete base_type_; }
    bool is_pointer() { return true; }
    bool equals(TypeRef* other);
    string to_string();

protected:
    TypeRef* base_type_;
};

class ArrayType : public Type {
public:
    long length() { return length_; }

protected:
    long length_;
};

}
#endif