#ifndef OPTION_H_
#define OPTION_H_

#include <set>
#include <string>

#include "ast.h"

using namespace std;

struct Option {
    Option() : ast_(nullptr), start_(0) {}

    cbc::AST* ast_;
    int start_;      // pseudo start symbpl, see parser.y
    string src_;     // source file name
    set<string> typename_;
};

#endif