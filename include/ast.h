#ifndef AST_H_
#define AST_H_

#include "node.h"

#include "decl.h"

namespace cbc {

class AST : public Node {
public:
    AST(const Location& source, Declarations* declarations);
    ~AST();

    Location location() { return source_; }

    string class_name() { return "AST"; }

    vector<Constant*> constants() { return decls_->constants(); }
    vector<DefinedVariable*> defined_variables() { return decls_->defvars(); }
    vector<DefinedFunction*> defined_functions() { return decls_->deffuncs(); }

    void dump_node(Dumper& dumper);

protected:
    Location source_;
    Declarations* decls_;
};

} // namespace cbc

#endif