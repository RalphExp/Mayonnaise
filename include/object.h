#include <cassert>

class Object {
public:
    Object() : ref_(1) {};

    virtual ~Object() {};

    void inc_ref() { 
        assert(ref_ > 0);
        ++ref_; 
    }

    void dec_ref() {
        --ref_;
        if (ref_ == 0) {
            delete this;
            return;
        }

        assert(ref_ > 0);
    }

protected:
    int ref_;
};