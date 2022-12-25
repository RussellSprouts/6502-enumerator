#pragma once

#include "z3++.h"
#include "zstdint.h"
#include "nstdint.h"

struct z_initial_state {
  std::vector<zuint16_t> absolute_vars;
  std::vector<zuint8_t> immediate_vars;
  std::vector<zuint8_t> zp_vars;
  std::vector<zuint8_t> relative_vars;
  z3::expr _software_stack;

  z_initial_state() : _software_stack(z3_ctx.bv_const("stack", 8)) {
    std::string absolute_name("absolute");
    for (int i = 0; i < 16; i++) {
      absolute_vars.push_back(z3_ctx.bv_const((absolute_name + std::to_string(i)).c_str(), 16));
    }
    
    std::string immediate_name("immediate");
    for (int i = 0; i < 16; i++) {
      immediate_vars.push_back(z3_ctx.bv_const((immediate_name + std::to_string(i)).c_str(), 8));
    }
    
    std::string zp_name("zp");
    for (int i = 0; i < 16; i++) {
      zp_vars.push_back(z3_ctx.bv_const((zp_name + std::to_string(i)).c_str(), 8));
    }

    std::string relative_name("relative");
    for (int i = 0; i < 16; i++) {
      relative_vars.push_back(z3_ctx.bv_const((relative_name + std::to_string(i)).c_str(), 8));
    }
  }

  zuint8_t u8(uint8_t n) const {
    return z3_ctx.bv_val(n, 8);
  }

  zuint8_t u8(const char *n) const {
    return z3_ctx.bv_const(n, 8);
  }

  zuint16_t u16(uint16_t n) const {
    return z3_ctx.bv_val(n, 16);
  }

  zuint16_t u16(const char *n) const {
    return z3_ctx.bv_const(n, 16);
  }

  zbool boolean(bool b) const {
    return z3_ctx.bool_val(b);
  }

  zbool boolean(const char *n) const {
    return z3_ctx.bool_const(n);
  }

  zmemory memory() const {
    zmemory result;
    return result;
  }

  zuint16_t absolute(uint16_t payload, bool is_constant) const {
    if (is_constant) {
      return u16(payload);
    } else {
      return absolute_vars.at(payload);
    }
  }

  zuint8_t immediate(uint16_t payload, bool is_constant) const {
    if (is_constant) {
      return u8(payload & 0xFF);
    } else {
      return immediate_vars.at(payload);
    }
  }

  zuint8_t zp(uint16_t payload, bool is_constant) const {
    if (is_constant) {
      return u8(payload & 0xFF);
    } else {
      return zp_vars.at(payload);
    }
  }

  zuint8_t relative(uint16_t payload, bool is_constant) const {
    if (is_constant) {
      return u8(payload & 0xFF);
    } else {
      return relative_vars.at(payload & 0xFF);
    }
  }

  zuint8_t software_stack() const {
    return _software_stack;
  }

  zuint16_t known_subroutine(uint16_t n) const {
    return z3_ctx.bv_const((std::string("subroutine") + std::to_string(n)).c_str(), 16);
  }
};

struct hash_initial_state {
  uint32_t seed;
  uint32_t init;

  hash_initial_state(uint32_t _seed) : seed(_seed) {
    init = 2166136261;
    init = (init ^ (seed & 0xFF)) * 16777619;
    init = (init ^ ((seed >> 8) & 0xFF)) * 16777619;
    init = (init ^ ((seed >> 16) & 0xFF)) * 16777619;
    init = (init ^ ((seed >> 24) & 0xFF)) * 16777619;
  }

  nuint8_t u8(uint8_t n) {
    return n;
  }

  nuint8_t u8(const char *n) {
    return fnv(n);
  }

  nuint16_t u16(uint16_t n) {
    return n;
  }

  nuint16_t u16(const char *n) {
    return fnv(n);
  }

  bool boolean(bool b) {
    return b;
  }

  bool boolean(const char *n) {
    return fnv(n) & 0x8000;
  }

  hmemory memory() {
    hmemory result(seed);
    return result;
  }

  nuint16_t absolute(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return u16(payload);
    } else {
      return fnv(payload + 2334);
    }
  }

  nuint8_t immediate(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return u8(payload & 0xFF);
    } else {
      return fnv(payload + 12467);
    }
  }

  nuint8_t zp(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return u8(payload & 0xFF);
    } else {
      return fnv(payload + 48942);
    }
  }

  nuint8_t relative(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return u8(payload & 0xFF);
    } else {
      return fnv(payload + 12994);
    }
  }

  nuint8_t software_stack() {
      return 0x80;
  }

  nuint16_t known_subroutine(uint16_t payload) {
    return fnv(payload + 21390);
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

  uint32_t fnv(const char *str) const {
    if (seed == 0) {
      return 0;
    } else if (seed == 0xFFFFFFFF) {
      return 0xFFFFFFFF;
    }
    uint32_t hash = init;
    for (int i = 0; str[i] != 0; i++) {
      hash = hash ^ str[i];
      hash = hash * 16777619;
    }
    return hash;
  }
};

struct concrete_initial_state {
  cmemory _memory;
  concrete_initial_state(uint8_t* memory) : _memory(memory) {}

  nuint8_t u8(uint8_t n) {
    return n;
  }

  nuint8_t u8(const char *n) {
    return 0;
  }

  nuint16_t u16(uint16_t n) {
    return n;
  }

  nuint16_t u16(const char *n) {
    return 0;
  }

  bool boolean(bool b) {
    return b;
  }

  bool boolean(const char *n) {
    return false;
  }

  cmemory memory() {
    return _memory;
  }

  nuint16_t absolute(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return u16(payload);
    } else {
      return 0;
    }
  }

  nuint8_t immediate(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return u8(payload & 0xFF);
    } else {
      return 0;
    }
  }

  nuint8_t zp(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return u8(payload & 0xFF);
    } else {
      return 0;
    }
  }

  nuint8_t relative(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return u8(payload & 0xFF);
    } else {
      return 0;
    }
  }

  nuint8_t software_stack() {
      return 0x80;
  }

  nuint16_t known_subroutine(uint16_t payload) {
    return 0;
  }
};