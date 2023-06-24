#include "decl.h"

namespace cbc {

class Loader {
public:
    static Declarations* load_library(const string& name);
};

}