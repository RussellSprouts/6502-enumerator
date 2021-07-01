#include "z3++.h"

template <int N>
struct zu {
    static_assert(N % 8 == 0, "Must be a whole number of bytes");

    zu(z3::expr _val): val(_val) {
        if (_val.is_bool()) {
            val = z3::ite(_val, _val.ctx().bv_val(1, N), _val.ctx().bv_val(0, N));
        } else if (_val.is_bv()) {
            int size = _val.get_sort().bv_size();
            if (size < N) {
                val = z3::zext(_val, N - size);
            } else if (_val.get_sort().bv_size() > N) {
                val = _val.extract(N - 1, 0);
            }
        } else {
            assert(false);
        }
    }

    template <int B>
    operator zu<B>() const {
        return val;
    }

    z3::expr val;
};

int sample() {
    z3::context c;
    c.bv_val(0, 8);

    zu<8> a(c);
    zu<16> b = a;

    return 0;
}