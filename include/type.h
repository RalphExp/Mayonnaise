#ifndef TYPE_H_
#define TYPE_H_

using namespace std;

class Location;

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
};

class TypeRef {
public:
    TypeRef() {}
    TypeRef(const TypeRef& ref) : loc_(ref.loc_) {}
    TypeRef(const Location& loc) : loc_(loc) {}
    
    Location location() const { return loc_; }

     /* TODO: */
    int hash_code() { return 0; }

protected:
    Location loc_;
};

#endif