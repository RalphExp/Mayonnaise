#include "type.h"

bool IntegerTypeRef::equals(TypeRef* other)
{
    IntegerTypeRef* ref = dynamic_cast<IntegerTypeRef*>(other);
    if (!ref)
        return false;

    return ref->name() == name();
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