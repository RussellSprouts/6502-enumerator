#pragma once

#include <stdint.h>
#include <string>

std::string int_to_hex(uint32_t i, int width) {
  std::stringstream stream;
  stream << std::setfill ('0') << std::setw(width) 
         << std::hex << i;
  return stream.str();
}

// A representation of interesting values you might want
// to load as an immediate. For example, lda #>(Absolute0-1),
// or lda #<(immediate0 & immediate1).
// This will help prove constant folding optimizations.

enum struct special_immediate_operations : uint8_t {
  PLUS = 0,
  MINUS = 1,
  AND = 2,
  OR = 3,
  XOR = 4,
  NOT = 5,

  PLUS_HI = 6,
  MINUS_HI = 7,
  AND_HI = 8,
  OR_HI = 9,
  XOR_HI = 10,
  NOT_HI = 11
};

enum struct special_immediate_operands : uint8_t {
  CONSTANT = 0,
  IMMEDIATE = 1,
  ZERO_PAGE = 2,
  ABSOLUTE = 3,
};

enum special_immediate_constants : uint8_t {
    x0 = 0,
    x1 = 1,
    x2 = 2,
    x3 = 3,
    x4 = 4,
    x8000 = 5,
    xFFFE = 6,
    xFFFF = 7
};

const uint16_t special_immediate_constant_values[8] = {
  0, 1, 2, 3, 4, 0x8000, 0xFFFE, 0xFFFF
};

struct special_immediate {
  special_immediate_operations operation : 4;
  special_immediate_operands operand1 : 3;
  uint8_t payload1 : 3;
  special_immediate_operands operand2 : 3;
  uint8_t payload2 : 3;

  static special_immediate from_int(uint16_t payload) {
    special_immediate result;
    result.operation = static_cast<special_immediate_operations>((payload & 0b11110000'00000000) >> 12);
    result.operand1 = static_cast<special_immediate_operands>((payload & 0b00001110'00000000) >> 9);
    result.payload1 = ((payload & 0b00000001'11000000) >> 6);
    result.operand2 = static_cast<special_immediate_operands>((payload & 0b00000000'00111000) >> 3);
    result.payload2 = (payload & 0b00000000'00000111);
    return result;
  }

  uint16_t to_int() {
    return (static_cast<uint16_t>(operation) << 12) |
        (static_cast<uint16_t>(operand1) << 9) |
        (payload1 << 6) |
        (static_cast<uint16_t>(operand2) << 3) |
        payload2;
  }
};

std::string special_immediate_operand_to_string(special_immediate_operands op, uint8_t payload) {
    switch (op) {
    case special_immediate_operands::ABSOLUTE:
        return "absolute" + std::to_string(payload);
    case special_immediate_operands::IMMEDIATE:
        return "immediate" + std::to_string(payload);
    case special_immediate_operands::ZERO_PAGE:
        return "zp" + std::to_string(payload);
    case special_immediate_operands::CONSTANT:
        return "$" + int_to_hex(special_immediate_constant_values[payload], 4);
    }
    assert(false);
}

std::string special_immediate_to_string(special_immediate s) {
    std::string a = special_immediate_operand_to_string(s.operand1, s.payload1);
    std::string b = special_immediate_operand_to_string(s.operand2, s.payload2);

    switch (s.operation) {
    case special_immediate_operations::AND:
        return "<(" + a + " & " + b + ")";
    case special_immediate_operations::AND_HI:
        return ">(" + a + " & " + b + ")";
    case special_immediate_operations::MINUS:
        return "<(" + a + " - " + b + ")";
    case special_immediate_operations::MINUS_HI:
        return ">(" + a + " - " + b + ")";
    case special_immediate_operations::NOT:
        return "<~" + a;
    case special_immediate_operations::NOT_HI:
        return ">~" + a;
    case special_immediate_operations::OR:
        return "<(" + a + " | " + b + ")";
    case special_immediate_operations::OR_HI:
        return ">(" + a + " | " + b + ")";
    case special_immediate_operations::PLUS:
        if (b == "$00") {
            return "<" + a;
        }
        return "<(" + a + " + " + b + ")";
    case special_immediate_operations::PLUS_HI:
        if (b == "$00") {
            return ">" + a;
        }
        return ">(" + a + " + " + b + ")";
    case special_immediate_operations::XOR:
        return "<(" + a + " ^ " + b + ")";
    case special_immediate_operations::XOR_HI:
        return ">(" + a + " ^ " + b + ")";
    }

    assert(false);
}