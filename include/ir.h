#include "object.h"
#include "util.h"
#include "asm.h"

using namespace std;

namespace cbc {

class ImmediateValue;
class Operand;
class MemoryReference;

class Expr : public Object, public Dumpable {
public:
    Expr(Type* type);
    ~Expr();

    Type* type() { return type_; }
    bool is_var() { return false; }
    bool is_addr() { return false; }
    bool is_constant() { return false; }
    
    ImmediateValue* asmValue() { throw string("Expr::asm_value called"); }
    Operand* address() {  throw string("Expr::address called"); }
    MemoryReference* memref() { throw string("Expr::memref called"); }
    Expr* addressNode(Type* type) { throw string("unexpected node for LHS: ") + get_class(); }
    Entity* get_entity_force() { return null; }
    virtual string get_class() = 0;

protected:
    Type* type_; // asm.type
};

} // namespace cbc