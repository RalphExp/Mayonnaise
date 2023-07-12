#ifndef TYPE_TABLE_H_
#define TYPE_TABLE_H_

#include <unordered_map>

#include "type.h"

namespace cbc {

using namespace std;

class TypeTable {
public:
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

protected:
    TypeTable(int int_size, int long_size, int ptr_size);
    static TypeTable* new_table(int charsize, int shortsize, int intsize, int longsize, int ptrsize);

protected:
    int int_size_;
    int long_size_;
    int pointer_size_;
    unordered_map<TypeRef*, Type*> table_;
};

}

#endif