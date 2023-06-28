#ifndef OBJECT_H_
#define OBJECT_H_

#include <cassert>

/* XXX: Object should be always used as pointers.
 * It can also use c++ shared_ptr to manage the memory,
 * but it make the code hard to read. The principle is,
 * when the object is passed as a parameter, inc_ref()
 * should be called. When the dtor is called, dec_ref()
 * should be called for each of its pointer members.
 */
namespace cbc {

class Object {
public:
    Object() : oref_(1) {
        printf("create object\n");
    };

    virtual ~Object() {
        printf("destroy object\n");
    };

    /* for hash table, see declaration.h */
    virtual bool equals(Object* other) { return this == other; }

    int get_oref() {
        return oref_;
    }

    void inc_ref() {
        if (this) {
            assert(oref_ >= 0);
            ++oref_;
        }
    }

    void dec_ref() {
        if (this) {
            --oref_;
            if (oref_ == 0) {
                delete this;
                return;
            }
            assert(oref_ >= 0);
        }
    }

protected:
    int oref_;
};

} // namespace cbc;

#endif
