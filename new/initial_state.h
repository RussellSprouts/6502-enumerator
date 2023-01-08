#pragma once

#include "z3++.h"
#include "zstdint.h"
#include "nstdint.h"
#include "special_immediate.h"

struct z_initial_state {
  std::vector<zuint16_t> absolute_vars;
  std::vector<zuint8_t> immediate_vars;
  std::vector<zuint8_t> zp_vars;
  std::vector<zuint8_t> relative_vars;
  std::vector<zuint8_t> stack_offset_vars;
  z3::expr _software_stack;
  z3::expr _temp;

  z_initial_state() :
    _software_stack(z3_ctx.bv_const("stack", 8)),
    _temp(z3_ctx.bv_const("temp", 8))
  {
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

    std::string stack_offset_name("offset");
    for (int i = 0; i < 16; i++) {
      stack_offset_vars.push_back(z3_ctx.bv_const((stack_offset_name + std::to_string(i)).c_str(), 8));
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

  zuint8_t software_stack(uint8_t payload, bool is_constant) const {
    if (is_constant) {
      return _software_stack + payload;
    } else {
      return u8(_software_stack) + stack_offset_vars.at(payload);
    }
  }

  zuint8_t temp() {
    return _temp;
  }

  zuint8_t temp(uint8_t payload, bool is_constant) const {
    if (is_constant) {
      return _temp + payload;
    } else {
      return u8(_temp) + stack_offset_vars.at(payload);
    }
  }

  zuint8_t special(uint16_t payload) const {
    special_immediate val = special_immediate::from_int(payload);
    zuint16_t op1 = 0;
    zuint16_t op2 = 0;
    switch (val.operand1) {
      case special_immediate_operands::ABSOLUTE:
        op1 = absolute(val.payload1, false);
        break;
      case special_immediate_operands::IMMEDIATE:
        op1 = zuint16_t(immediate(val.payload1, false));
        break;
      case special_immediate_operands::ZERO_PAGE:
        op1 = zuint16_t(zp(val.payload1, false));
        break;
      case special_immediate_operands::CONSTANT:
        op1 = special_immediate_constant_values[val.payload1];
        break;
    }

    switch (val.operand2) {
      case special_immediate_operands::ABSOLUTE:
        op2 = absolute(val.payload2, false);
        break;
      case special_immediate_operands::IMMEDIATE:
        op2 = zuint16_t(immediate(val.payload2, false));
        break;
      case special_immediate_operands::ZERO_PAGE:
        op2 = zuint16_t(zp(val.payload2, false));
        break;
      case special_immediate_operands::CONSTANT:
        op2 = special_immediate_constant_values[val.payload2];
        break;
    }

    switch (val.operation) {
      case special_immediate_operations::AND:
        return lobyte(op1 & op2);
      case special_immediate_operations::AND_HI:
        return hibyte(op1 & op2);
      case special_immediate_operations::MINUS:
        return lobyte(op1 - op2);
      case special_immediate_operations::MINUS_HI:
        return hibyte(op1 - op2);
      case special_immediate_operations::NOT:
        return lobyte(~op1);
      case special_immediate_operations::NOT_HI:
        return hibyte(~op1);
      case special_immediate_operations::OR:
        return lobyte(op1 | op2);
      case special_immediate_operations::OR_HI:
        return hibyte(op1 | op2);
      case special_immediate_operations::PLUS:
        return lobyte(op1 + op2);
      case special_immediate_operations::PLUS_HI:
        return hibyte(op1 + op2);
      case special_immediate_operations::XOR:
        return lobyte(op1 ^ op2);
      case special_immediate_operations::XOR_HI:
        return hibyte(op1 ^ op2);
    }
    assert(false);
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

  nuint8_t u8(uint8_t n) const {
    return n;
  }

  nuint8_t u8(const char *n) const {
    return fnv(n);
  }

  nuint16_t u16(uint16_t n) const {
    return n;
  }

  nuint16_t u16(const char *n) const {
    return fnv(n);
  }

  bool boolean(bool b) const {
    return b;
  }

  bool boolean(const char *n) const {
    return fnv(n) & 0x8000;
  }

  hmemory memory() const {
    hmemory result(seed);
    return result;
  }

  nuint16_t absolute(uint16_t payload, bool is_constant) const {
    if (is_constant) {
      return u16(payload);
    } else {
      return fnv(payload + 2334);
    }
  }

  nuint8_t immediate(uint16_t payload, bool is_constant) const {
    if (is_constant) {
      return u8(payload & 0xFF);
    } else {
      return fnv(payload + 12467);
    }
  }

  nuint8_t zp(uint16_t payload, bool is_constant) const {
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

  nuint8_t software_stack(uint8_t offset, bool is_constant) const {
    if (is_constant) {
      return 0x80 + offset;
    } else {
      return 0x80 + (fnv(offset) & 0x7);
    }
  }

  nuint8_t temp() const {
    return 0x10;
  }

  nuint8_t temp(uint8_t offset, bool is_constant) const {
    if (is_constant) {
      return 0x10 + offset;
    } else {
      return 0x10 + (fnv(offset) % TEMP_SIZE);
    }
  }

  nuint8_t special(uint16_t payload) const {
    special_immediate val = special_immediate::from_int(payload);
    nuint16_t op1 = 0;
    nuint16_t op2 = 0;
    switch (val.operand1) {
      case special_immediate_operands::ABSOLUTE:
        op1 = absolute(val.payload1, false);
        break;
      case special_immediate_operands::IMMEDIATE:
        op1 = nuint16_t(immediate(val.payload1, false));
        break;
      case special_immediate_operands::ZERO_PAGE:
        op1 = nuint16_t(zp(val.payload1, false));
        break;
      case special_immediate_operands::CONSTANT:
        op1 = special_immediate_constant_values[val.payload1];
        break;
    }

    switch (val.operand2) {
      case special_immediate_operands::ABSOLUTE:
        op2 = absolute(val.payload2, false);
        break;
      case special_immediate_operands::IMMEDIATE:
        op2 = nuint16_t(immediate(val.payload2, false));
        break;
      case special_immediate_operands::ZERO_PAGE:
        op2 = nuint16_t(zp(val.payload2, false));
        break;
      case special_immediate_operands::CONSTANT:
        op2 = special_immediate_constant_values[val.payload2];
        break;
    }

    switch (val.operation) {
      case special_immediate_operations::AND:
        return lobyte(op1 & op2);
      case special_immediate_operations::AND_HI:
        return hibyte(op1 & op2);
      case special_immediate_operations::MINUS:
        return lobyte(op1 - op2);
      case special_immediate_operations::MINUS_HI:
        return hibyte(op1 - op2);
      case special_immediate_operations::NOT:
        return lobyte(~op1);
      case special_immediate_operations::NOT_HI:
        return hibyte(~op1);
      case special_immediate_operations::OR:
        return lobyte(op1 | op2);
      case special_immediate_operations::OR_HI:
        return hibyte(op1 | op2);
      case special_immediate_operations::PLUS:
        return lobyte(op1 + op2);
      case special_immediate_operations::PLUS_HI:
        return hibyte(op1 + op2);
      case special_immediate_operations::XOR:
        return lobyte(op1 ^ op2);
      case special_immediate_operations::XOR_HI:
        return hibyte(op1 ^ op2);
    }
    assert(false);
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

  nuint8_t u8(uint8_t n) const {
    return n;
  }

  nuint8_t u8(const char *n) const {
    return 0;
  }

  nuint16_t u16(uint16_t n) const {
    return n;
  }

  nuint16_t u16(const char *n) const {
    return 0;
  }

  bool boolean(bool b) const {
    return b;
  }

  bool boolean(const char *n) const {
    return false;
  }

  cmemory memory() {
    return _memory;
  }

  nuint16_t absolute(uint16_t payload, bool is_constant) const {
    if (is_constant) {
      return u16(payload);
    } else {
      return 0;
    }
  }

  nuint8_t immediate(uint16_t payload, bool is_constant) const {
    if (is_constant) {
      return u8(payload & 0xFF);
    } else {
      return 0;
    }
  }

  nuint8_t zp(uint16_t payload, bool is_constant) const {
    if (is_constant) {
      return u8(payload & 0xFF);
    } else {
      return 0;
    }
  }

  nuint8_t relative(uint16_t payload, bool is_constant) const {
    if (is_constant) {
      return u8(payload & 0xFF);
    } else {
      return 0;
    }
  }

  nuint8_t software_stack(uint8_t payload, bool is_constant) const {
      return 0x80 + payload;
  }

  nuint8_t temp() const {
    return 0x10;
  }

  nuint8_t temp(uint8_t offset, bool is_constant) const {
      return 0x10 + offset;
  }

  nuint8_t special(uint16_t payload) const {
    special_immediate val = special_immediate::from_int(payload);
    nuint16_t op1 = 0;
    nuint16_t op2 = 0;
    switch (val.operand1) {
      case special_immediate_operands::ABSOLUTE:
        op1 = absolute(val.payload1, false);
        break;
      case special_immediate_operands::IMMEDIATE:
        op1 = nuint16_t(immediate(val.payload1, false));
        break;
      case special_immediate_operands::ZERO_PAGE:
        op1 = nuint16_t(zp(val.payload1, false));
        break;
      case special_immediate_operands::CONSTANT:
        op1 = special_immediate_constant_values[val.payload1];
        break;
    }

    switch (val.operand2) {
      case special_immediate_operands::ABSOLUTE:
        op2 = absolute(val.payload2, false);
        break;
      case special_immediate_operands::IMMEDIATE:
        op2 = nuint16_t(immediate(val.payload2, false));
        break;
      case special_immediate_operands::ZERO_PAGE:
        op2 = nuint16_t(zp(val.payload2, false));
        break;
      case special_immediate_operands::CONSTANT:
        op2 = special_immediate_constant_values[val.payload2];
        break;
    }

    switch (val.operation) {
      case special_immediate_operations::AND:
        return lobyte(op1 & op2);
      case special_immediate_operations::AND_HI:
        return hibyte(op1 & op2);
      case special_immediate_operations::MINUS:
        return lobyte(op1 - op2);
      case special_immediate_operations::MINUS_HI:
        return hibyte(op1 - op2);
      case special_immediate_operations::NOT:
        return lobyte(~op1);
      case special_immediate_operations::NOT_HI:
        return hibyte(~op1);
      case special_immediate_operations::OR:
        return lobyte(op1 | op2);
      case special_immediate_operations::OR_HI:
        return hibyte(op1 | op2);
      case special_immediate_operations::PLUS:
        return lobyte(op1 + op2);
      case special_immediate_operations::PLUS_HI:
        return hibyte(op1 + op2);
      case special_immediate_operations::XOR:
        return lobyte(op1 ^ op2);
      case special_immediate_operations::XOR_HI:
        return hibyte(op1 ^ op2);
    }
    assert(false);
  }

  nuint16_t known_subroutine(uint16_t payload) {
    return 0;
  }
};