#pragma once

// Definitions for standard integers that don't promote
// to wider types

#include <assert.h>
#include "stdint.h"

struct cmemory;
struct hmemory;

template<typename underlying_t>
struct nuint_t {

    nuint_t() : val(0) {}
    nuint_t(nuint_t<underlying_t> const& other) : val(other.val) {}
    nuint_t(nuint_t<underlying_t>&& other) : val(other.val) {}
    nuint_t(underlying_t _val) : val(_val) {}

    nuint_t<underlying_t>& operator=(nuint_t<underlying_t> const& other) {
        val = other.val;
        return *this;
    }

    nuint_t<underlying_t> operator+(const nuint_t<underlying_t> other) const;
    nuint_t<underlying_t> operator-(const nuint_t<underlying_t> other) const;
    nuint_t<underlying_t> operator&(const nuint_t<underlying_t> other) const;
    nuint_t<underlying_t> operator|(const nuint_t<underlying_t> other) const;
    nuint_t<underlying_t> operator^(const nuint_t<underlying_t> other) const;
    nuint_t<underlying_t> operator<<(const uint8_t amount) const;
    nuint_t<underlying_t> operator>>(const uint8_t amount) const;
    nuint_t<underlying_t> operator-() const;
    nuint_t<underlying_t> operator~() const;
    bool operator==(const nuint_t<underlying_t> other) const;
    bool operator!=(const nuint_t<underlying_t> other) const;
    bool operator<=(const nuint_t<underlying_t> other) const;
    bool operator<(const nuint_t<underlying_t> other) const;
    bool operator>=(const nuint_t<underlying_t> other) const;
    bool operator>(const nuint_t<underlying_t> other) const;

    template<typename other_underlying_t>
    explicit operator nuint_t<other_underlying_t>() const;

    underlying_t val;
};

template<typename underlying_t>
nuint_t<underlying_t> nuint_t<underlying_t>::operator+(const nuint_t<underlying_t> other) const {
    return val + other.val;
}

template<typename underlying_t>
nuint_t<underlying_t> nuint_t<underlying_t>::operator-(const nuint_t<underlying_t> other) const {
    return val - other.val;
}

template<typename underlying_t>
nuint_t<underlying_t> nuint_t<underlying_t>::operator&(const nuint_t<underlying_t> other) const {
    return val & other.val;
}

template<typename underlying_t>
nuint_t<underlying_t> nuint_t<underlying_t>::operator|(const nuint_t<underlying_t> other) const {
    return val | other.val;
}

template<typename underlying_t>
nuint_t<underlying_t> nuint_t<underlying_t>::operator^(const nuint_t<underlying_t> other) const {
    return val ^ other.val;
}

template<typename underlying_t>
nuint_t<underlying_t> nuint_t<underlying_t>::operator<<(const uint8_t amount) const {
    return val << amount;
}

template<typename underlying_t>
nuint_t<underlying_t> nuint_t<underlying_t>::operator>>(const uint8_t amount) const {
    return val >> amount;
}

template<typename underlying_t>
nuint_t<underlying_t> nuint_t<underlying_t>::operator-() const {
    return -val;
}

template<typename underlying_t>
nuint_t<underlying_t> nuint_t<underlying_t>::operator~() const {
    return ~val;
}

template<typename underlying_t>
bool nuint_t<underlying_t>::operator==(const nuint_t<underlying_t> other) const {
    return val == other.val;
}

template<typename underlying_t>
bool nuint_t<underlying_t>::operator!=(const nuint_t<underlying_t> other) const {
    return val != other.val;
}

template<typename underlying_t>
bool nuint_t<underlying_t>::operator<=(const nuint_t<underlying_t> other) const {
    return val <= other.val;
}

template<typename underlying_t>
bool nuint_t<underlying_t>::operator<(const nuint_t<underlying_t> other) const {
    return val < other.val;
}

template<typename underlying_t>
bool nuint_t<underlying_t>::operator>=(const nuint_t<underlying_t> other) const {
    return val >= other.val;
}

template<typename underlying_t>
bool nuint_t<underlying_t>::operator>(const nuint_t<underlying_t> other) const {
    return val > other.val;
}


template<typename underlying_t>
template<typename other_underlying_t>
nuint_t<underlying_t>::operator nuint_t<other_underlying_t>() const {
    return val;
}

typedef nuint_t<uint8_t> nuint8_t;
typedef nuint_t<uint16_t> nuint16_t;


nuint8_t ite8(bool cond, nuint8_t cons, nuint8_t alt) {
    return cond ? cons : alt;
}

nuint16_t ite16(bool cond, nuint16_t cons, nuint16_t alt) {
    return cond ? cons : alt;
}

bool iteB(bool cond, bool cons, bool alt) {
    return cond ? cons : alt;
}

nuint16_t from_bytes(nuint8_t high, nuint8_t low) {
    return ((nuint16_t)high << 8) | (nuint16_t)low;
}

nuint8_t lobyte(nuint16_t val) {
    return val.val;
}
nuint8_t hibyte(nuint16_t val) {
    return val.val >> 8;
}


struct hmemory {
    uint32_t init;
    uint32_t seed;
    nuint16_t writtenAddresses[NUM_ADDRESSES_TO_REMEMBER];
    nuint8_t  writtenValues[NUM_ADDRESSES_TO_REMEMBER];
    uint8_t  nWritten = 0;

    hmemory(uint32_t _seed) : seed(_seed) {
        init = 2166136261;
        init = (init ^ (seed & 0xFF)) * 16777619;
        init = (init ^ ((seed >> 8) & 0xFF)) * 16777619;
        init = (init ^ ((seed >> 16) & 0xFF)) * 16777619;
        init = (init ^ ((seed >> 24) & 0xFF)) * 16777619;
    }

    nuint8_t read(nuint16_t addr) const {
        for (uint8_t i = 0; i < nWritten; i++) {
            if (writtenAddresses[i] == addr) {
                return writtenValues[i];
            }
        }
        return fnv(addr.val);
    }

    void write_if(bool cond, nuint16_t addr, nuint8_t value) {
        if (cond) {
            for (uint8_t i = 0; i < nWritten; i++) {
                if (writtenAddresses[i] == addr) {
                    writtenValues[i] = value;
                    return;
                }
            }
            writtenAddresses[nWritten] = addr;
            writtenValues[nWritten] = value;
            nWritten++;
        }
    }

    uint32_t fnv(uint16_t value) const {
        if (seed == 0) {
            return 0;
        } else if (seed == 0xFFFFFFFF) {
            return 0xFFFFFFFF;
        }
        uint32_t hash = init;
        // first round use the seed.
        hash = hash ^ (value & 0xFF);
        hash = hash * 16777619;
        hash = hash ^ ((value & 0xFF00) >> 8);
        hash = hash * 16777619;
        return hash;
    }
};

struct cmemory {
    uint8_t* contents;
    nuint16_t addresses_written[NUM_ADDRESSES_TO_REMEMBER];
    nuint16_t addresses_read[NUM_ADDRESSES_TO_REMEMBER];
    nuint8_t values_written[NUM_ADDRESSES_TO_REMEMBER];
    nuint8_t values_read[NUM_ADDRESSES_TO_REMEMBER];
    uint8_t n_written = 0;
    uint8_t n_read = 0;
    bool record_memory;

    cmemory(uint8_t* _contents, bool _record_memory = false) : contents(_contents), record_memory(_record_memory) {}

    nuint8_t read(nuint16_t addr) {
        if (record_memory) {
            assert(n_read != NUM_ADDRESSES_TO_REMEMBER);
            addresses_read[n_read] = addr;
            (values_read[n_read++] = contents[addr.val]);
        }
        return contents[addr.val];
    }

    void write_if(bool cond, nuint16_t addr, nuint8_t value) {
        if (cond) {
            if (record_memory) {
                assert(n_written != NUM_ADDRESSES_TO_REMEMBER);
                addresses_written[n_written] = addr;
                values_written[n_written++] = value;
            }
            contents[addr.val] = value.val;
        }
        
    }
};
