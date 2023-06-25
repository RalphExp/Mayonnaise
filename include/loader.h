#ifndef LOADER_H_
#define LOADER_H_

#include <string>
#include <map>
#include <set>
#include <vector>

using namespace std;

namespace cbc {
class Declarations;

class Loader {
public:
    Loader();
    Loader(const vector<string>& load_path);
    Loader(vector<string>&& load_path);
    ~Loader();

    static vector<string> default_load_path();
    void add_load_path(const string& path);
    Declarations* load_library(const string& name);
    string search_library(const string& libid, int* fd);
    string lib_path(const string& libid);

protected:
    vector<string> load_path_;
    vector<string> loading_;
    map<string, Declarations*> loaded_;
};

}

#endif