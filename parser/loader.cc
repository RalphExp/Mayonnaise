#include "loader.h"
#include "decl.h"
#include "option.h"

#include <algorithm>

using namespace std;

#include "parser/lexer.hh"
#include "parser/parser.hh"

namespace cbc {

Loader::~Loader()
{
    for (auto& pair  : loaded_) {
        delete pair.second;
    }
}

Declarations* Loader::load_library(const string& libid) {
    if (loading_.end() != find_if(loading_.begin(), loading_.end(), 
            [&libid](const string& name) -> bool {return libid == name; })) {
        throw string("recursive import from ") + loading_.back() + ": " + libid;
    }

    loading_.push_back(libid);   // stop recursive import

    auto it = loaded_.find(libid);
    if (it != loaded_.end()) {
        // Already loaded import file.  Returns cached declarations.
        return it->second;
    }

    Option option;
    yyscan_t lexer;
    yylex_init(&lexer);
    yyset_extra(&option, lexer);
    option.src_ = search_library(libid);
    option.start_ = parser::Parser::token::DECLARE;

    parser::Parser parser(lexer);
    int res = parser.parse();
    if (res != 0) {
        yylex_destroy(lexer);
        exit(1);
    }

    auto* decls = option.decl_;
    option.decl_ = nullptr;

    loaded_[libid] = decls;
    loading_.pop_back();
    yylex_destroy(lexer);
    return decls;
}

string Loader::search_library(const string& libid)
{
    return "";
}

} // namespace cbc
