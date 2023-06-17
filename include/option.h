#ifndef OPTION_H_
#define OPTION_H_

#include <set>
#include <string>

#include "ast.h"

using namespace std;

struct Option {
    Option() : ast_(nullptr) {}

    cbc::AST* ast_;
    string src_;
    set<string> typename_;
};

#endif