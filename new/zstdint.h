#pragma once

#include "z3++.h"
#include "z3_context.h"

// Forward declare
struct zbool;
template <int N> struct zuint;
struct zmemory;

struct zbool {
    z3::expr val;
    zbool(z3::expr _val);
    zbool(bool _val);
    zbool operator==(const zbool& other);
    zbool operator&&(const zbool& other);
    zbool operator||(const zbool& other);
    zbool operator^(const zbool& other);
    zbool operator!();
    template<int B> explicit operator zuint<B>();
};

template<int N>
struct zuint {
    static_assert(N % 8 == 0, "Must be a whole number of bytes");
    z3::expr val;
    zuint(z3::expr _val);
    zuint(uint32_t _val);

    zuint<N> operator+(const zuint<N>& other) const;
    zuint<N> operator-(const zuint<N>& other) const;
    zuint<N> operator&(const zuint<N>& other) const; 
    zuint<N> operator|(const zuint<N>& other) const;
    zuint<N> operator^(const zuint<N>& other) const;
    zuint<N> operator~() const;
    zbool operator==(const zuint<N>& other) const;
    zbool operator!=(const zuint<N>& other) const;
    zbool operator<(const zuint<N>& other) const;
    zbool operator<=(const zuint<N>& other) const;
    zbool operator>(const zuint<N>& other) const;
    zbool operator>=(const zuint<N>& other) const;
    zuint<N> operator>>(int amount) const;
    zuint<N> operator<<(int amount) const;
    template<int B> explicit operator zuint<B>() const;
};

struct zmemory {
    zmemory();
    zmemory(z3::expr _val);

    zuint<8> read(zuint<16> addr) const;
    void write_if(zbool cond, zuint<16> addr, zuint<8> val);

    z3::expr val;
};

// Implementations
zbool::zbool(const z3::expr _val): val(_val) {
    assert(_val.is_bool());
}
zbool::zbool(bool _val): val(z3_ctx.bool_val(_val)) {}
zbool zbool::operator==(const zbool& other) {
    return val == other.val;
}
zbool zbool::operator&&(const zbool& other) {
    return val && other.val;
}
zbool zbool::operator||(const zbool& other) {
    return val || other.val;
}
zbool zbool::operator^(const zbool& other) {
    return val ^ other.val;
}
zbool zbool::operator!() {
    return !val;
}
template<int B>
zbool::operator zuint<B>() {
    return z3::ite(
        val,
        z3_ctx.bv_val(1, B),
        z3_ctx.bv_val(0, B)
    );
}

template<int N>
zuint<N>::zuint(z3::expr _val): val(_val) {
    assert(_val.is_bv() && _val.get_sort().bv_size() == N);
}
template<int N>
zuint<N>::zuint(uint32_t _val): val(z3_ctx.bv_val(_val, N)) {}

template<int N>
zuint<N> zuint<N>::operator+(const zuint<N>& other) const {
    return val + other.val;
}
template<int N>
zuint<N> zuint<N>::operator-(const zuint<N>& other) const {
    return val - other.val;
}
template<int N>
zuint<N> zuint<N>::operator&(const zuint<N>& other) const {
    return val & other.val;
}
template<int N>
zuint<N> zuint<N>::operator|(const zuint<N>& other) const {
    return val | other.val;
}
template<int N>
zuint<N> zuint<N>::operator^(const zuint<N>& other) const {
    return val ^ other.val;
}
template<int N>
zuint<N> zuint<N>::operator~() const {
    return ~val;
}
template<int N>
zbool zuint<N>::operator==(const zuint<N>& other) const {
    return val == other.val;
}
template<int N>
zbool zuint<N>::operator!=(const zuint<N>& other) const {
    return val != other.val;
}
template<int N>
zbool zuint<N>::operator<(const zuint<N>& other) const {
    return z3::ult(val, other.val);
}
template<int N>
zbool zuint<N>::operator<=(const zuint<N>& other) const {
    return z3::ule(val, other.val);
}
template<int N>
zbool zuint<N>::operator>(const zuint<N>& other) const {
    return z3::ugt(val, other.val);
}
template<int N>
zbool zuint<N>::operator>=(const zuint<N>& other) const {
    return z3::uge(val, other.val);
}
template<int N>
zuint<N> zuint<N>::operator>>(int amount) const {
    return z3::lshr(val, amount);
}
template<int N>
zuint<N> zuint<N>::operator<<(int amount) const {
    return z3::shl(val, amount);
}

template<int N>
template<int B> zuint<N>::operator zuint<B>() const {
    if (B > N) {
        return z3::zext(val, B - N);
    } else {
        return val.extract(B - 1, 0);
    }
}

template<int N>
zuint<8> lobyte(const zuint<N> val) {
    return val.val.extract(7, 0);
}
template<int N>
zuint<8> hibyte(const zuint<N> val) {
    if (N == 8) {
        return 0;
    } else {
        return val.val.extract(15, 8);
    }
}

typedef zuint<8> zuint8_t;
typedef zuint<16> zuint16_t;

zuint8_t ite8(const zbool cond, const zuint8_t cons, const zuint8_t alt) {
    return z3::ite(cond.val, cons.val, alt.val);
}
zuint16_t ite16(const zbool cond, const zuint16_t cons, const zuint16_t alt) {
    return z3::ite(cond.val, cons.val, alt.val);
}

zbool iteB(const zbool cond, const zbool cons, const zbool alt) {
    return z3::ite(cond.val, cons.val, alt.val);
}

zuint16_t from_bytes(const zuint8_t high, const zuint8_t low) {
    return z3::concat(high.val, low.val);
}

zmemory::zmemory() : val(z3_ctx.constant("memory", z3_ctx.array_sort(z3_ctx.bv_sort(16), z3_ctx.bv_sort(8)))) {
}
zmemory::zmemory(z3::expr _val) : val(_val) {}

zuint8_t zmemory::read(zuint16_t addr) const {
    return z3::select(val, addr.val);
}

void zmemory::write_if(zbool cond, zuint16_t addr, zuint8_t new_val) {
    val = z3::ite(cond.val, z3::store(val, addr.val, new_val.val), val);
}

void add_assertion(z3::solver &solver, const zbool assertion) {
    solver.add(assertion.val);
}