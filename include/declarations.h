#include <unordered_set>

#include "entity.h"
#include "node.h"

namespace may {
class Declarations {
public:

protected:
    unordered_set<DefinedVariable> defvars_;
    unordered_set<UndefinedVariable> vardecls_;
    unordered_set<DefinedFunction> defuns_;
    unordered_set<UndefinedFunction> funcdecls_;
    unordered_set<Constant> constants_;
    unordered_set<StructNode> defstructs_;
    unordered_set<UnionNode> defunions_;
    unordered_set<TypedefNode> typedefs_;
};

}