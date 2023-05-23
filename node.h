#ifndef AST_NODE_H_
#define AST_NODE_H_

#include <iostream>

#include "util.h"
#include "ast.h"
#include "type.h"

using namespace std;

class Type;

class Node {
public:
    Node() {}

    virtual void dump_node(Dumper& dumper) = 0;

    void dump(ostream& os=cout) {
        Dumper dumper(os);
        dump(dumper);
    }
    void dump(Dumper& dumper);

    string get_name(void) const { return name; }

protected:
    string name;
    Location loc;    
};

class ExprNode : public Node {
public:
    ExprNode() {}
    virtual ~ExprNode() {}
    virtual Type* type() = 0;
    long alloc_size() { return type()->alloc_size(); }
    bool is_const() { return false; }
    bool is_parameter() { return false; }
    bool is_lvalue() { return false; }
    bool is_assignable() { return false; }
    bool is_loadable() { return false; }
    bool is_callable();
    bool is_pointer();

    template<typename S, typename E>
    E accept(const ASTVisitor<S,E> &visitor);
};

#endif