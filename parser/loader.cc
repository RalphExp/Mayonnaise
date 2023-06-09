#include "loader.h"
#include "decl.h"
#include "option.h"

#include <fcntl.h>
#include <algorithm>

using namespace std;

#include "parser/lexer.hh"
#include "parser/parser.hh"

namespace cbc {

Loader::Loader() :
    load_path_(Loader::default_load_path())
{
}

Loader::~Loader()
{
    for (auto &p : loaded_) {
        p.second->dec_ref();
    }
}

vector<string> Loader::default_load_path()
{
    // 
    return vector<string>{".", "import", "test"};
}

void Loader::add_load_path(const string& path)
{
    auto it = find(load_path_.begin(), load_path_.end(), path);
    if (it == load_path_.end()) {
        load_path_.push_back(path);
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
    
    int fd;
    option.src_ = search_library(libid, &fd);
    option.start_ = parser::Parser::token::DECLARE;

    FILE* f = fdopen(fd, "r");
    yyset_in(f, lexer);

    parser::Parser parser(lexer);
    int res = parser.parse();
    if (res != 0) {
        fclose(f);
        close(fd);
        yylex_destroy(lexer);
        exit(1);
    }

    // option won't delete anything
    auto* decls = option.decl_;
    loaded_[libid] = decls;
    loading_.pop_back();
    fclose(f);
    close(fd);
    yylex_destroy(lexer);
    return decls;
}

string Loader::search_library(const string& libid, int* fd)
{
    for (auto& path : load_path_) {
        auto s = path + "/" + lib_path(libid) + ".hb";
        // fprintf(stdout, "try path %s\n", s.c_str());

        *fd = open(s.c_str(), O_RDONLY);
        if (*fd == -1) {
            if (errno != ENOENT)
                throw string(strerror(errno));
        } else {
            return s;
        }
    }
    throw string("no such library header file: ") + libid;
}

string Loader::lib_path(const string& libid)
{
    string s(libid);
    replace(s.begin(), s.end(), '.', '/');
    return s;
}

} // namespace cbc
