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
    ~Loader();
    Declarations* load_library(const string& name);
    string search_library(const string& libid);

protected:
    vector<string> load_path_;
    vector<string> loading_;
    map<string, Declarations*> loaded_;
};

}

#endif