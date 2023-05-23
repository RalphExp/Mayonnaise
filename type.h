#ifndef TYPE_H_
#define TYPE_H_

using namespace std;

class Type {
public:
    virtual long size() const = 0;
    long alloc_size() const { return size(); }
    long alignment() const { return alloc_size(); }
    bool is_void() const { return false; }
    bool is_int() const { return false; }
    bool is_integer() const { return false; }
    bool is_signed() const { throw "#issignedfor non-integer type "; }
    bool is_pointer() const { return false; }
    bool is_array() const { return false; }
    bool is_composite_type() const { return false; }
    bool is_struct() const { return false; }
    bool is_union() const { return false; }
    bool is_user_type() const { return false; }
    bool is_function() const { return false; }
    bool is_allocated_array() const { return false; }
    bool is_incomplete_array() const { return false; }
    bool is_scalar() const { return false; }
    bool is_callable() const { return false; }

    virtual bool is_compatible(const Type& other) const = 0;
    virtual bool is_castable_to(const Type& other) const = 0;
};

#endif