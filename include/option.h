#ifndef OPTION_H_
#define OPTION_H_

#include <set>
#include <string>

#include "ast.h"
#include "loader.h"

using namespace std;

struct Option {
    Option() : ast_(nullptr), decl_(nullptr), start_(0) {}
    ~Option() {
        ast_->dec_ref();
        decl_->dec_ref();
    }

    cbc::AST* ast_;
    cbc::Declarations* decl_;
    cbc::Loader loader_;
    int start_;      // pseudo start symbpl, see parser.y
    string src_;     // source file name
    set<string> typename_;
};

#endif