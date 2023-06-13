#include "node.h"

#include "decl.h"

namespace cbc {

class AST : public Node {
public:
    AST(const Location& source, Declarations* declarations);
    ~AST();

    Location location() { return source_; }

    // vector<TypeDefinition*> types();
    // vector<Entity*> entities();
    // vector<Entity*> declarations();
    // vector<Entity*> definitions();

protected:
    Location source_;
    Declarations* decls_;
};

}