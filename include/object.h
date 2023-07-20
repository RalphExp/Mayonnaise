#ifndef OBJECT_H_
#define OBJECT_H_

#include <atomic>
#include <cassert>

/* XXX: Object should be always used as pointers.
 * It can also use c++ shared_ptr to manage the memory,
 * but it make the code hard to read. The principle is,
 * 1) when the object is passed as a parameter, inc_ref() should be called. 
 * 2) When the dtor is called, dec_ref() should be called for each of its pointer members.
 * 3) move operation doesn't change reference count.
 * 4) when an internal pointer is passed out, inc_ref() should be called, 
 *    and when this pointer is not used, dec_ref() should be called.
 */
namespace cbc {

class Object {
public:
    Object() : oref_(1) {
        // printf("create object\n");
    };

    virtual ~Object() {
        // printf("destroy object\n");
    };

    /* for hash table, see declaration.h */
    virtual bool equals(Object* other) { return this == other; }

    int get_ref() {
        return oref_;
    }

    void inc_ref() {
        if (this) {
            ++oref_;
        }
    }

    void dec_ref(int n=1) {
        if (this) {
            oref_ -= n;
            if (oref_ == 0) {
                delete this;
                return;
            }
            assert(oref_ >= 1);
        }
    }

protected:
    std::atomic<int> oref_;
};

} // namespace cbc;

#endif
