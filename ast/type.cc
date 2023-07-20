#include "type.h"

#include <cmath>
#include <sstream>

#include "util.h"
#include "node.h"
#include "type_table.h"

namespace cbc {

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

IntegerType* Type::get_integer_type()
{
    IntegerType* type = dynamic_cast<IntegerType*>(this);
    if (type == nullptr) {
        throw "not an integer type";
    }
    return type;
}

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

IntegerTypeRef* IntegerTypeRef::ushort_ref() 
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

IntegerType::IntegerType(long size, bool is_signed, const string& name) :
    size_(size), is_signed_(is_signed), name_(name)
{
}

long IntegerType::min_value() 
{ 
    return is_signed_ ? (long)-pow(2, size_* 8 - 1) : 0; 
}

long IntegerType::max_value() 
{ 
    return is_signed_ ? (long)pow(2, size_* 8 - 1) - 1  : (long)pow(2, size_ * 8) - 1;
}

bool IntegerType::is_same_type(Type* other) 
{
    if (!other->is_integer()) 
        return false;
    return equals(other->get_integer_type());
}

bool IntegerType::is_compatible(Type* other)
{
    return (other->is_integer() && size_ <= other->size());
}

bool IntegerType::is_castable_to(Type* target)
{
    return (target->is_integer() || target->is_pointer());
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
        
    return base_type_->equals(ref->base_type_);
} 

TypeRef* PointerTypeRef::base_type() 
{ 
    base_type_->inc_ref(); 
    return base_type_; 
}

string PointerTypeRef::to_string() const
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

Type* PointerType::base_type() 
{ 
    base_type_->inc_ref(); 
    return base_type_; 
}

bool PointerType::equals(Object* other)
{
    PointerType* tp = dynamic_cast<PointerType*>(other);
    if (!tp)
        return false;

    return base_type_->equals(tp->get_pointer_type()->base_type());
}

bool PointerType::is_same_type(Type* type)
{
    if (!type->is_pointer())
        return false;

    return base_type_->is_same_type(type->base_type());
}

bool PointerType::is_compatible(Type* other)
{
    if (!other->is_pointer()) 
        return false;
        
    if (base_type_->is_void())
        return true;
    
    if (other->base_type()->is_void()) {
        return true;
    }
        
    return base_type_->is_compatible(other->base_type());
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
        // s->inc_ref();
    }
}

CompositeType::~CompositeType()
{
    for (auto* s : members_) {
        s->dec_ref();
    }
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
    
vector<Slot*> CompositeType::members()
{
    vector<Slot*> v;
    for (auto m : members_) {
        m->inc_ref();
        v.push_back(m);
    }
    return v;
}
    
vector<Type*> CompositeType::member_types()
{
    vector<Type*> v;
    for (auto m : members_) {
        m->type()->inc_ref();
        v.push_back(m->type());
    }
    return v;
}
    
bool CompositeType::has_member(const string& name)
{
    auto s = get(name);
    bool b = s->name() != "";
    s->dec_ref();
    return b;
}
    
Type* CompositeType::member_type(const string& name)
{
    auto s = get(name);
    auto tp = s->type();
    tp->inc_ref();
    s->dec_ref();
    return tp;
}
    
long CompositeType::member_offset(const string& name)
{
    auto s = fetch(name);
    if (s->offset() == Type::kSizeUnknown) {
        compute_offsets();
    }
    return s->offset();
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
    for (auto* t : member_types()) {
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
    throw string("unknown method: ") + method; 
}
        
Slot* CompositeType::fetch(const string& name)
{
    auto s = get(name);
    if (s == nullptr)
        throw string("not such member in " + to_string() + ": " + name);
    return s;
}
    
Slot* CompositeType::get(const string& name)
{
    for (auto* s : members_) {
        if (s->name() == name) {
            s->inc_ref();
            return s;
        }
    }
    return nullptr;
}

StructType::StructType(const string& name, 
        vector<Slot*>&& membs, const Location& loc) : 
    CompositeType(name, move(membs), loc)
{
}
    
bool StructType::is_same_type(Type* other)
{
     if (!other->is_struct()) 
        return false;
    
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

bool StructTypeRef::equals(Object* other)
{
    StructTypeRef* ref = dynamic_cast<StructTypeRef*>(other);
    if (!ref)
        return false;

    return name() == ref->name();
}
    
UnionType::UnionType(const string& name, 
        vector<Slot*>&& membs, const Location& loc) : 
    CompositeType(name, move(membs), loc)
{
}

bool UnionType::is_same_type(Type* other)
{
     if (!other->is_union()) 
        return false;
    
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

bool UnionTypeRef::equals(Object* other)
{
    UnionTypeRef* ref = dynamic_cast<UnionTypeRef*>(other);
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

bool UserTypeRef::equals(Object* other)
{
    UserTypeRef* ref = dynamic_cast<UserTypeRef*>(other);
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

Type* UserType::real_type()
{ 
    real_->type()->inc_ref();
    return real_->type(); 
}

ArrayTypeRef::ArrayTypeRef(TypeRef* base) : 
    TypeRef(base->location()), base_type_(base), length_(-1)
{
    base_type_->inc_ref();
}
    
ArrayTypeRef::ArrayTypeRef(TypeRef* base, long length) : 
    TypeRef(base->location()), base_type_(base), length_(length)
{
    base_type_->inc_ref();

    if (length < 0) 
        throw string("negative array length");
}

ArrayTypeRef::~ArrayTypeRef()
{
    base_type_->dec_ref();
}

ArrayType::ArrayType(Type* base_type, long pointer_size) :
    base_type_(base_type), length_(-1), pointer_size_(pointer_size)
{
    base_type_->inc_ref();
}

ArrayType::ArrayType(Type* base_type, long length, long pointer_size) :
    base_type_(base_type), length_(length), pointer_size_(pointer_size)
{
    base_type_->inc_ref();
}

ArrayType::~ArrayType()
{
    base_type_->dec_ref();
}

bool ArrayType::is_allocated_array()
{
    return length_ != -1 &&
        (!base_type_->is_array() || base_type_->is_allocated_array());
}

bool ArrayType::is_same_type(Type* other)
{
    if (!other->is_pointer() && !other->is_array())
        return false;
    
    return base_type_->is_same_type(other->base_type());
}

bool ArrayType::is_incomplete_array()
{
    if (!base_type_->is_array()) 
        return false;

    return !base_type_->is_allocated_array();
}

long ArrayType::alloc_size()
{
    if (length_ == -1) {
        return size();
    } else {
        return base_type_->alloc_size() * length_;
    }
}

string ArrayType::to_string() const
{
    if (length_ < 0) {
        return base_type_->to_string() + "[]";
    } else {
        return base_type_->to_string() + "[" + \
            std::to_string(length_) + "]";
    }
}

bool ArrayTypeRef::equals(Object* other)
{
    ArrayTypeRef* ref = dynamic_cast<ArrayTypeRef*>(other);
    return ref && ref->length_ == length_;
}
    
string ArrayTypeRef::to_string() const
{
    return base_type_->to_string() + \
        "[" + (length_ == -1 ? "" : std::to_string(length_)) + \
        "]";
}

ParamTypeRefs::ParamTypeRefs(vector<TypeRef*>&& param_descs) :
    ParamSlots<TypeRef>(move(param_descs))
{
}

ParamTypeRefs::ParamTypeRefs(const Location& loc, 
        vector<TypeRef*>&& param_descs, bool vararg) :
    ParamSlots<TypeRef>(loc, move(param_descs), vararg)
{
}

bool ParamTypeRefs::equals(Object* other)
{
    ParamTypeRefs* ref = dynamic_cast<ParamTypeRefs*>(other);
    return ref && equals(ref);
}

// Internal Types
ParamTypes* ParamTypeRefs::intern_types(TypeTable* table)
{
    vector<Type*> v;
    for (TypeRef* ref : param_descs_) {
        auto type = table->get_param_type(ref);
        type->inc_ref();
        v.push_back(type);
    }
    return new ParamTypes(loc_, move(v), vararg_);
}

bool ParamTypeRefs::equals(ParamTypeRefs* other)
{
    if (vararg_ != other->vararg_)
        return false;

    if (param_descs_.size() != other->param_descs_.size())
        return false;

    for (size_t i = 0; i < param_descs_.size(); ++i) {
        if (param_descs_[i]->equals(other->param_descs_[i]) == false)
            return false;
    }
    return true;
}

ParamTypes::ParamTypes(const Location& loc, vector<Type*>&& param_descs, bool vararg) :
    ParamSlots<Type>(loc, move(param_descs), vararg)
{
}

bool ParamTypes::equals(Object* other)
{
    ParamTypes* ref = dynamic_cast<ParamTypes*>(other);
    return ref && equals(ref);
}
    
bool ParamTypes::equals(ParamTypes* other)
{
    if (vararg_ != other->vararg_)
        return false;

    if (param_descs_.size() != other->param_descs_.size())
        return false;

    for (size_t i = 0; i < param_descs_.size(); ++i) {
        if (param_descs_[i]->equals(other->param_descs_[i]) == false)
            return false;
    }
    return true;
}

FunctionTypeRef::FunctionTypeRef(TypeRef* return_type, 
        ParamTypeRefs* params) :
    TypeRef(return_type->location()), return_type_(return_type), params_(params)
{
    return_type_->inc_ref();
    params_->inc_ref();
}

FunctionTypeRef::~FunctionTypeRef()
{
    return_type_->dec_ref();
    params_->dec_ref();
}

bool FunctionTypeRef::equals(Object* other)
{
    FunctionTypeRef* ref = dynamic_cast<FunctionTypeRef*>(other);
    return ref && equals(ref);
}

bool FunctionTypeRef::equals(FunctionTypeRef* ref)
{
    return ref->return_type_->equals(return_type_) &&
            ref->params_->equals(params_);
}

string FunctionTypeRef::to_string() const
{
    stringstream ss;
    ss << return_type_->to_string() << " (";
    string sep = "";
    for (auto ref : params_->typerefs()) {
        ss << sep;
        ss << ref->to_string();
        sep = ", ";
    }
    ss << ")";
    return ss.str();
}

FunctionType::FunctionType(Type* ret, ParamTypes* param_types) :
    return_type_(ret), param_types_(param_types)
{
    return_type_->inc_ref();
    param_types_->inc_ref();
}

FunctionType::~FunctionType()
{
    return_type_->dec_ref();
    param_types_->dec_ref();
}

bool FunctionType::is_same_type(Type* type)
{
    return false;
}

} // namespace cbc