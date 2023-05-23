#ifndef AST_NODE_H_
#define AST_NODE_H_

#include <iostream>

#include "util.h"

using namespace std;

class Node {
public:
    Node() {}
    void dump() {}
    void dump(const string& file) {}
    void dump(const Dumper& duper) {}

protected:
    Location loc;    
};

#endif