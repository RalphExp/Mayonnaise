#include "type.h"
#include "util.h"
#include "node.h"

#include <sstream>

namespace may {

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

FunctionType* Type::get_function_type()
{
    FunctionType* type = dynamic_cast<FunctionType*>(this);
    if (type == nullptr) {
        throw "not a function type";
    }
    return type;
}

// IntegerType* Type::::get_integer_type()
// {
//     IntegerType* type = dynamic_cast<IntegerType*>(this);
//     if (type == nullptr) {
//         throw "not an integer type";
//     }
//     return type;
// }

StructType* Type::get_struct_type()
{
    StructType* type = dynamic_cast<StructType*>(this);
    if (type == nullptr) {
        throw "not a struct type";
    }
    return type;
}

UnionType* Type::get_union_type()
{
    UnionType* type = dynamic_cast<UnionType*>(this);
    if (type == nullptr) {
        throw "not a union type";
    }
    return type;
}

ArrayType* Type::get_array_type()
{
    ArrayType* type = dynamic_cast<ArrayType*>(this);
    if (type == nullptr) {
        throw "not an array type";
    }
    return type;
}

bool IntegerTypeRef::equals(Object* other)
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

bool VoidTypeRef::equals(Object* other)
{
    return !!dynamic_cast<VoidTypeRef*>(other); 
}

bool VoidType::equals(Object* other)
{
    return !!dynamic_cast<VoidType*>(other);
}

NamedType::NamedType(const string& name, const Location& loc)
    : name_(name), loc_(loc)
{
}

PointerTypeRef::PointerTypeRef(TypeRef* base) :
        TypeRef(base->location()), base_type_(base)
{
    base_type_->inc_ref();
}

PointerTypeRef::~PointerTypeRef()
{
    base_type_->dec_ref();
}

bool PointerTypeRef::equals(Object* other)
{
    PointerTypeRef* ref = dynamic_cast<PointerTypeRef*>(other);
    if (!ref)
        return false;
        
    return base_type_->equals(ref->base_);
} 

string PointerTypeRef::to_string()
{
    return base_type_->to_string() + "*";
}

PointerType::PointerType(long size, Type* base_type)
    : size_(size), base_type_(base_type)
{
    base_type_->inc_ref();
}

PointerType::~PointerType()
{
    base_type_->dec_ref();
}

bool PointerType::equals(Object* other)
{
    PointerType* tp = dynamic_cast<PointerType*>(other);
    if (!tp)
        return false;

    return base_type_->equals(type->get_pointer_type()->base_type());
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

bool PointerType::is_castable_to(Type* other)
{
    return other->is_pointer() || other->is_integer();
}
    
CompositeType::CompositeType(const string& name, 
        vector<Slot*>&& membs, const Location& loc) : 
    NamedType(name, loc), members_(move(membs)),
    cached_size_(Type::kSizeUnknown),
    cached_align_(Type::kSizeUnknown)
{
    for (auto* s : members_) {
        s->inc_ref();
    }
}

CompositeType::~CompositeType()
{
    for (auto* s : members_) {
        s->dec_ref();
    }
}

bool CompositeType::is_same_type(shared_ptr<Type> other)
{
    return compare_member_types(other, "is_same_type");
}

bool CompositeType::is_compatible(shared_ptr<Type> target)
{
    return compare_member_types(target, "is_compatible");
}
    
bool CompositeType::is_castable_to(shared_ptr<Type> target)
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
    
shared_ptr<vector<shared_ptr<Slot>>> CompositeType::members()
{
    return members_;
}
    
shared_ptr<vector<shared_ptr<Type>>> CompositeType::member_types()
{
    auto v = make_shared<vector<shared_ptr<Type>>>();
    for (auto s : *members_) {
        v->push_back(s->type());
    }
    return v;
}
    
bool CompositeType::has_member(const string& name)
{
    return get(name)->name() != "";
}
    
Type* CompositeType::member_type(const string& name)
{
    return get(name)->type();
}
    
long CompositeType::member_offset(const string& name)
{
    auto s = fetch(name);
    if (s->offset() == Type::kSizeUnknown) {
        compute_offsets();
    }
    return s->offset();
}
    
bool CompositeType::compare_member_types(shared_ptr<Type> other, const string& method)
{
    if (is_struct() && !other->is_struct()) 
        return false;
        
    if (is_union() && !other->is_union()) 
        return false;
        
    CompositeType* other_type = other->get_composite_type();
    if (members_->size() != other->size()) 
        return false;
        
    auto other_types = other_type->member_types()->begin();
    for (auto t : *member_types()) {
        if (!compare_types_by(method, t, *other_types)) {
            return false;
        }
        ++other_types;
    }
    return true;
}
    
bool CompositeType::compare_types_by(const string& method, shared_ptr<Type> t, shared_ptr<Type> tt)
{
    if (method == "is_same_type")
        return t->is_same_type(tt);
    if (method == "is_compatible")
        return t->is_compatible(tt);
    if (method == "is_castable_to")
        return t->is_castable_to(tt);
    throw string("unknown method: ") + method; 
}
        
shared_ptr<Slot> CompositeType::fetch(const string& name)
{
    auto s = get(name);
    if (s == nullptr)
        throw string("not such member in " + to_string() + ": " + name);
    return s;
}
    
shared_ptr<Slot> CompositeType::get(const string& name)
{
    for (auto s : *members_) {
        if (s->name() == name) {
            return s;
        }
    }
    return nullptr;
}

StructType::StructType(const string& name, 
        vector<Slot*>&& membs, const Location& loc) : 
    CompositeType(name, membs, loc)
{
}
    
bool StructType::is_same_type(shared_ptr<Type> other)
{
     if (!other->is_struct()) 
        return false;
    
    /*FIXME: */
    return equals(other);
}

void StructType::compute_offsets() 
{
    throw string("Not implement!");
}

StructTypeRef::StructTypeRef(const string& name) : 
    name_(name)
{
}
    
StructTypeRef::StructTypeRef(const Location& loc, const string& name) : 
    TypeRef(loc), name_(name)
{
}

bool StructTypeRef::equals(shared_ptr<TypeRef> other)
{
    StructTypeRef* ref = dynamic_cast<StructTypeRef*>(other.get());
    if (!ref)
        return false;

    return name() == ref->name();
}
    
UnionType::UnionType(const string& name, 
        shared_ptr<vector<shared_ptr<Slot>>> membs, const Location& loc) : 
    CompositeType(name, membs, loc)
{
}

bool UnionType::is_same_type(shared_ptr<Type> other)
{
     if (!other->is_union()) 
        return false;
    
    /*FIXME: */
    return equals(other);
}   

void UnionType::compute_offsets()
{
    throw string("Not implement!");
}

UnionTypeRef::UnionTypeRef(const string& name) : 
    name_(name)
{
}

UnionTypeRef::UnionTypeRef(const Location& loc, const string& name) : 
    TypeRef(loc), name_(name)
{
}

bool UnionTypeRef::equals(shared_ptr<TypeRef> other)
{
    UnionTypeRef* ref = dynamic_cast<UnionTypeRef*>(other.get());
    if (!ref)
        return false;

    return name() == ref->name();
}

UserTypeRef::UserTypeRef(const string& name) : 
    name_(name)
{
}
    
UserTypeRef::UserTypeRef(const Location& loc, const string& name) : 
    TypeRef(loc), name_(name)
{
}

bool UserTypeRef::equals(shared_ptr<TypeRef> other)
{
    UserTypeRef* ref = dynamic_cast<UserTypeRef*>(other.get());
    if (!ref)
        return false;

    return name() == ref->name();
}

UserType::UserType(const string& name, TypeNode* real, const Location& loc):
    NamedType(name, loc), real_(real)
{
    real_->inc_ref();
}

UserType::~UserType()
{
    real_->dec_ref();
}

ArrayTypeRef::ArrayTypeRef(shared_ptr<TypeRef> base) : 
    TypeRef(base->location()), base_(base), length_(-1)
{
}
    
ArrayTypeRef::ArrayTypeRef(shared_ptr<TypeRef> base, long length) : 
    TypeRef(base->location()), base_(base), length_(length)
{
    if (length < 0) 
        throw string("negative array length");
}

bool ArrayTypeRef::equals(shared_ptr<TypeRef> other)
{
    ArrayTypeRef* ref = dynamic_cast<ArrayTypeRef*>(other.get());
    return ref && ref->length_ == length_;
}
    
string ArrayTypeRef::to_string()
{
    return base_->to_string() + \
        "[" + (length_ == -1 ? "" : "" + length_) + \
        "]";
}

ParamTypeRefs::ParamTypeRefs(pv_typeref param_descs) :
    ParamSlots<TypeRef>(param_descs)
{
}

ParamTypeRefs::ParamTypeRefs(const Location& loc, 
        pv_typeref param_descs, bool vararg) :
    ParamSlots<TypeRef>(loc, param_descs, vararg)
{
}

bool ParamTypeRefs::equals(shared_ptr<ParamTypeRefs> other)
{
    if (vararg_ != other->vararg_)
        return false;

    if (param_descs_->size() != other->param_descs_->size())
        return false;

    for (size_t i = 0; i < param_descs_->size(); ++i) {
        if (!(*param_descs_)[i]->equals((*other->param_descs_)[i]))
            return false;
    }

    return true;
}

ParamTypes::ParamTypes(const Location& loc, pv_type param_descs, bool vararg) :
    ParamSlots<Type>(loc, param_descs, vararg)
{
}

FunctionTypeRef::FunctionTypeRef(shared_ptr<TypeRef> return_type, 
        shared_ptr<ParamTypeRefs> params) :
    return_type_(return_type), params_(params)
{
}

bool FunctionTypeRef::equals(shared_ptr<TypeRef> other)
{
    FunctionTypeRef* ref = dynamic_cast<FunctionTypeRef*>(other.get());
    return ref && ref->return_type_->equals(return_type_) &&
            ref->params_->equals(params_);
}

string FunctionTypeRef::to_string() 
{
    stringstream ss;
    ss << return_type_->to_string() << " (";
    string sep = "";
    for (auto ref : *params_->typerefs()) {
        ss << sep;
        ss << ref->to_string();
        sep = ", ";
    }
    ss << ")";
    return ss.str();
}

FunctionType::FunctionType(shared_ptr<Type> ret, shared_ptr<ParamTypes> param_types) :
    return_type_(ret), param_types_(param_types)
{
}

} // namespace may