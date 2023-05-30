#include "type.h"
#include "util.h"
#include "node.h"

namespace ast {

CompositeType* Type::get_composite_type()
{
    CompositeType* type = dynamic_cast<CompositeType*>(this);
    if (type == nullptr) {
        throw "not a composite type";
    }
    return type;
}

PointerType* Type::get_pointer_type() 
{ 
    PointerType* type = dynamic_cast<PointerType*>(this);
    if (type == nullptr) {
        throw "not a pointer type";
    }
    return type;
}

bool IntegerTypeRef::equals(TypeRef* other)
{
    IntegerTypeRef* ref = dynamic_cast<IntegerTypeRef*>(other);
    if (!ref)
        return false;

    return ref->name() == name();
}

IntegerTypeRef* IntegerTypeRef::char_ref(const Location& loc) 
{
    return new IntegerTypeRef("char", loc);
}   

IntegerTypeRef* IntegerTypeRef::char_ref() 
{
    return new IntegerTypeRef("char");
}   

IntegerTypeRef* IntegerTypeRef::short_ref(const Location& loc) 
{
    return new IntegerTypeRef("short", loc);
}   

IntegerTypeRef* IntegerTypeRef::short_ref()
{
    return new IntegerTypeRef("short");
}   

IntegerTypeRef* IntegerTypeRef::int_ref(const Location& loc)
{
    return new IntegerTypeRef("int", loc);
}   

IntegerTypeRef* IntegerTypeRef::int_ref()
{
    return new IntegerTypeRef("int");
}   

IntegerTypeRef* IntegerTypeRef::long_ref(const Location& loc)
{
    return new IntegerTypeRef("long", loc);
}   

IntegerTypeRef* IntegerTypeRef::long_ref()
{
    return new IntegerTypeRef("long");
}

IntegerTypeRef* IntegerTypeRef::uchar_ref(const Location& loc)
{
    return new IntegerTypeRef("unsigned char", loc);
}

IntegerTypeRef* IntegerTypeRef::uchar_ref()
{
    return new IntegerTypeRef("unsigned char");
}

IntegerTypeRef* IntegerTypeRef::ushort_ref(const Location& loc) 
{
    return new IntegerTypeRef("unsigned short", loc);
}

IntegerTypeRef* IntegerTypeRef::ushortRef() 
{
    return new IntegerTypeRef("unsigned short");
}

IntegerTypeRef* IntegerTypeRef::uint_ref(const Location& loc)
{
    return new IntegerTypeRef("unsigned int", loc);
}

IntegerTypeRef* IntegerTypeRef::uint_ref()
{
    return new IntegerTypeRef("unsigned int");
}

IntegerTypeRef* IntegerTypeRef::ulong_ref(const Location& loc) 
{
    return new IntegerTypeRef("unsigned long", loc);
}

IntegerTypeRef* IntegerTypeRef::ulong_ref()
{
    return new IntegerTypeRef("unsigned long");
}

PointerTypeRef::PointerTypeRef(TypeRef* base) : TypeRef(base->location()),
    base_(base)
{  
}

bool PointerTypeRef::equals(TypeRef* other)     
{
    PointerTypeRef* ref = dynamic_cast<PointerTypeRef*>(other);
    if (!ref)
        return false;
        
    return base_->equals(ref->base_);
} 

string PointerTypeRef::to_string()
{
    return base_->to_string() + "*";
}

NamedType::NamedType(const string& name, const Location& loc)
    : name_(name), loc_(loc)
{

}

PointerType::PointerType(long size, Type* base_type)
    : size_(size), base_(base_type)
{
}

bool PointerType::equals(Type* type)
{
    PointerType* tp = dynamic_cast<PointerType*>(type);
    if (!tp)
        return false;

    // FIXME: XXX
    return base_->equals(type->get_pointer_type()->base_type());
}

bool PointerType::is_same_type(Type* type)
{
    if (!type->is_pointer())
        return false;

    return base_->is_same_type(type->base_type());
}

bool PointerType::is_compatible(Type* other)
{
    if (!other->is_pointer()) 
        return false;
        
    if (base_->is_void())
        return true;
    
    if (other->base_type()->is_void()) {
        return true;
    }
        
    return base_->is_compatible(other->base_type());
}

bool is_castable_to(Type* other)
{
    return other->is_pointer() || other->is_integer();
}

CompositeType::CompositeType(const string& name, 
        const vector<Slot>& membs, const Location& loc)
    : NamedType(name, loc), members_(membs), 
    cached_size_(Type::kSizeUnknown), 
    cached_align_(Type::kSizeUnknown)
{
}
    
CompositeType::CompositeType(const string& name, 
        vector<Slot>&& membs, const Location& loc)
    : NamedType(name, loc), members_(move(membs)),
    cached_size_(Type::kSizeUnknown), 
    cached_align_(Type::kSizeUnknown)
{
}

bool CompositeType::is_same_type(Type* other)
{
    return compare_member_types(other, "is_same_type");
}

bool CompositeType::is_compatible(Type* target)
{
    return compare_member_types(target, "is_compatible");
}
    
bool CompositeType::is_castable_to(Type* target)
{
    return compare_member_types(target, "is_castable_to");
}
    
long CompositeType::size()
{
    if (cached_size_ == Type::kSizeUnknown) {
        compute_offsets();
    }
    return cached_size_;
}
    
long CompositeType::alignmemt()
{
    if (cached_align_ == Type::kSizeUnknown) {
        compute_offsets();
    }
    return cached_align_;
}
    
vector<Slot> CompositeType::members()
{
    return members_;
}
    
vector<Type*> CompositeType::member_types()
{
    vector<Type*> v;
    for (auto &s : members_) {
        v.push_back(s.type());
    }
    return v;
}
    
bool CompositeType::has_member(const string& name)
{
    return get(name).name() != "";
}
    
Type* CompositeType::member_type(const string& name)
{
    return get(name).type();
}
    
long CompositeType::member_offset(const string& name)
{
    Slot s = fetch(name);
    if (s.offset() == Type::kSizeUnknown) {
        compute_offsets();
    }
    return s.offset();
}
    
bool CompositeType::compare_member_types(Type* other, const string& method)
{
    if (is_struct() && !other->is_struct()) 
        return false;
        
    if (is_union() && !other->is_union()) 
        return false;
        
    CompositeType* other_type = other->get_composite_type();
    if (members_.size() != other->size()) 
        return false;
        
    auto other_types = other_type->member_types().begin();
    for (Type* t : member_types()) {
        if (!compare_types_by(method, t, *other_types)) {
            return false;
        }
        ++other_types;
    }
    return true;
}
    
bool CompositeType::compare_types_by(const string& method, Type* t, Type* tt)
{
    if (method == "is_same_type")
        return t->is_same_type(tt);
    if (method == "is_compatible")
        return t->is_compatible(tt);
    if (method == "is_castable_to")
        return t->is_castable_to(tt);
    throw "unknown method: " + method; 
}
        
Slot CompositeType::fetch(const string& name)
{
    Slot s = get(name);
    if (s.name() == "")
        throw string("not such member in " + to_string() + ": " + name);
    return s;
}
    
Slot CompositeType::get(const string& name)
{
    for (Slot& s : members_) {
        if (s.name() == name) {
            return s;
        }
    }
    return Slot();
}

}