#ifndef AST_NODE_H_
#define AST_NODE_H_

#include <iostream>

#include "util.h"

using namespace std;

class Node {
public:
    Node() {}

    void dump(ostream& os=cout) {
        Dumper dumper(os);
        dump(dumper);
    }

    void dump(Dumper& dumper);

    string get_name(void) const { return name; }

    virtual void dump_node(Dumper& dumper) = 0;

protected:
    string name;
    Location loc;    
};

#endif