#include "z3++.h"
#include "z3_context.h"

#define SIMPLE_OP(op) zu<N> operator op(const zu<N> other) { return zu<N>(val op other.val); }

struct zbool {
    zbool(z3::expr _val): val(_val) {
        if (!_val.is_bool()) {
            assert(false);
        }
    }
    zbool(bool _val): val(z3_ctx.bool_val(_val)) {}

    zbool operator==(const zbool& other) {
        return val == other.val;
    }
    zbool operator&&(const zbool& other) {
        return val && other.val;
    }
    zbool operator||(const zbool& other) {
        return val || other.val;
    }

    template<int B>
    explicit operator zu<B>() const {
        return z3::ite(val, val.ctx().bv_val(1, B), val.ctx().bv_val(0, B));
    }

    z3::expr val;
};

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
    zu(uint16_t _val): val(z3_ctx.bv_val(_val, N)) {}

    SIMPLE_OP(+)
    SIMPLE_OP(-)
    SIMPLE_OP(&)
    SIMPLE_OP(|)
    SIMPLE_OP(^)

    zbool operator==(const zu<N> other) const {
        return val == other.val;
    }

    zbool operator!=(const zu<N> other) const {
        return val != other.val;
    }

    zbool operator>(const zu<N> other) const {
        return z3::ugt(val, other.val);
    }

    zbool operator>=(const zu<N> other) const {
        return z3::uge(val, other.val);
    }

    zbool operator<(const zu<N> other) const {
        return z3::ult(val, other.val);
    }

    zbool operator<=(const zu<N> other) const {
        return z3::ule(val, other.val);
    }

    zu<N> operator>>(uint8_t amount) const {
        return z3::lshr(val, amount);
    }

    zu<N> operator<<(uint8_t amount) const {
        return z3::shl(val, amount);
    }


    zu<8> low() {
        if (N > 8) {
            return val.extract(7,0);
        } else {
            return val;
        }
    }

    zu<8> high() {
        if (N > 8) {
            return val.extract(15, 8);
        } else {
            return val.ctx().bv_val(0, 8);
        }
    }

    static zu<N> ite(const zbool cond, const zu<N> cons, const zu<N> alt) {
        return z3::ite(cond, cons, alt);
    }

    static zu<N> from_bytes(zu<8> low, zu<8> high) {
        static_assert(N == 16, "from_bytes is only for 16 bit numbers");
        return (zu<16>)low + ((zu<16>)high << 8);
    }

    z3::expr val; 
};

typedef zbool zbool;
typedef zu<8> zuint8_t;
typedef zu<16> zuint16_t;

struct zmemory {
    zmemory(z3::expr expr): memory(expr) {
    }

    zu<8> read(zu<16> address) {
        return z3::select(memory, address.val);
    }

    void write(zu<16> address, zu<8> val) {
        memory = z3::store(memory, address.val, val.val);
    }

    z3::expr memory;
};

template <typename T>
struct cu {
    cu(T _val): val(_val) {}

    cu<T> operator+(const cu<T> other) const {
        return val + other.val;
    }
    cu<T> operator-(const cu<T> other) const {
        return val - other.val;
    }
    cu<T> operator&(const cu<T> other) const {
        return val | other.val;
    }
    cu<T> operator|(const cu<T> other) const {
        return val - other.val;
    }
    cu<T> operator^(const cu<T> other) const {
        return val - other.val;
    }
    bool operator==(const cu<T> other) const {
        return val == other.val;
    }
    bool operator!=(const cu<T> other) const {
        return val != other.val;
    }
    bool operator>(const cu<T> other) const {
        return val > other.val;
    }
    bool operator>=(const cu<T> other) const {
        return val >= other.val;
    }
    bool operator<(const cu<T> other) const {
        return val < other.val;
    }
    bool operator<=(const cu<T> other) const {
        return val <= other.val;
    }
    cu<T> operator<<(uint8_t amount) const {
        return val << amount;
    }
    cu<T> operator>>(uint8_t amount) const {
        return val >> amount;
    }

    cu<uint8_t> low() {
        return (uint8_t)val;
    }

    cu<uint8_t> high() {
        return (uint8_t)(val >> 8);
    }

    static cu<T> ite(bool cond, const cu<T> cons, const cu<T> alt) {
        return cond ? cons : alt;
    }

    const T val; 
};

int sample() {
    zbool b1(false);
    zbool b2 = b1 == false;

    zuint8_t a1(5);
    std::cout << (a1 + 4).val << std::endl;

    auto v = z3_ctx.bv_val(14, 8);

    zu<8> a(v);
    zu<16> b = (zu<16>)a;

    zu<16> d = (zu<16>)a & b;

    auto t = zu<16>::from_bytes(a, a);
 
    return 0;
}

struct cmemory {
    cmemory():
        _addresses_read{0},
        _values_read{0},
        _n_read(0),
        _addresses_written{0},
        _values_written{0},
        _n_written(0),
        _record_memory(false),
        _memory(nullptr) {}

    cu<uint8_t> read(cu<uint16_t> address) {
        if (_record_memory) {
            assert(_n_read < 8);
            _addresses_read[_n_read] = address.val;
            _values_read[_n_read] = _memory[address.val];
            _n_read++;
        }
    }

    void write(cu<uint16_t> address, cu<uint8_t> value) {
        if (_record_memory) {

        }
    }

    uint16_t _addresses_read[8];
    uint8_t _values_read[8];
    uint8_t _n_read;
    uint16_t _addresses_written[8];
    uint8_t _values_written[8];
    uint8_t _n_written;

    uint8_t* _memory;
    bool _record_memory;
};
