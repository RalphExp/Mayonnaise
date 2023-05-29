#include "type.h"
#include "util.h"

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
    base_type_(base)
{
    
}

bool PointerTypeRef::equals(TypeRef* other)     
{
    PointerTypeRef* ref = dynamic_cast<PointerTypeRef*>(other);
    if (!ref)
        return false;
        
    return base_type_->equals(ref->base_type_);
} 

string PointerTypeRef::to_string()
{
    return base_type_->to_string() + "*";
}

NamedType::NamedType(const string& name, const Location& loc)
    : name_(name), loc_(loc)
{

}

PointerType::PointerType(long size, Type* base_type)
    : size_(size), base_type_(base_type)
{
}

}