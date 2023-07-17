#ifndef TYPE_TABLE_H_
#define TYPE_TABLE_H_

#include <unordered_map>

#include "type.h"

namespace cbc {

using namespace std;

class TypeRefHash {
public:
    size_t operator()(const TypeRef* ref) const {
        return hash<string>()(ref->to_string());
    }
};

class TypeRefEqual {
public:
    bool operator()(const TypeRef* r1, const TypeRef* r2) const {
        return ((TypeRef*)r1)->equals((Object*)r2);
    }
};

class TypeTable {
public:
    ~TypeTable();
    
    // for i386
    static TypeTable* ilp32() { return new_table(1, 2, 4, 4, 4); }
    // for x86-64
    static TypeTable* lp64()  { return new_table(1, 2, 4, 8, 8); }
    // not used in this project
    static TypeTable* ilp64() { return new_table(1, 2, 8, 8, 8); }
    // not used in this project
    static TypeTable* llp64() { return new_table(1, 2, 4, 4, 8); }

    bool is_defined(TypeRef* ref);
    void put(TypeRef* ref, Type* t);
    Type* get(TypeRef* ref);
    Type* get_param_type(TypeRef* ref);
    PointerType* pointer_to(Type* base_type);

    int int_size() { return int_size_; }
    int long_size() { return long_size_; }
    int pointer_size() { return pointer_size_; }
    int max_int_size() { return pointer_size_; }
    Type* ptr_diff_type() { return get(ptr_diff_type_ref()); }

    // returns a IntegerTypeRef whose size is equals to pointer.
    TypeRef* ptr_diff_type_ref() { return new IntegerTypeRef(ptr_diff_type_name()); }
    string ptr_diff_type_name();

protected:
    TypeTable(int int_size, int long_size, int ptr_size);
    static TypeTable* new_table(int charsize, int shortsize, int intsize, int longsize, int ptrsize);

protected:
    int int_size_;
    int long_size_;
    int pointer_size_;
    unordered_map<TypeRef*, Type*, TypeRefHash, TypeRefEqual> table_;
};

}

#endif