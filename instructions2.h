#pragma once

#include "stdint.h"

enum class instruction_name {
  NONE = 0,
  ADC = 1,
  AND = 2,
  ASL = 3,
  BCC = 4,
  BCS = 5,
  BEQ = 6,
  BIT = 7,
  BMI = 8,
  BNE = 9,
  BPL = 10,
  BRK = 11,
  BVC = 12,
  BVS = 13,
  CLC = 14,
  CLD = 15,
  CLI = 16,
  CLV = 17,
  CMP = 18,
  CPX = 19,
  CPY = 20,
  DEC = 21,
  DEX = 22,
  DEY = 23,
  EOR = 24,
  INC = 25,
  INX = 26,
  INY = 27,
  JMP = 28,
  JSR = 29,
  LDA = 30,
  LDX = 31,
  LDY = 32,
  LSR = 33,
  NOP = 34,
  ORA = 35,
  PHA = 36,
  PHP = 37,
  PLA = 38,
  PLP = 39,
  ROL = 40,
  ROR = 41,
  RTI = 42,
  RTS = 43,
  SBC = 44,
  SEC = 45,
  SED = 46,
  SEI = 47,
  STA = 48,
  STX = 49,
  STY = 50,
  TAX = 51,
  TAY = 52,
  TSX = 53,
  TXA = 54,
  TXS = 55,
  TYA = 56,

  JMPI = 57,
  LSRA = 58,
  ASLA = 59,
  ROLA = 60,
  RORA = 61,
};

enum class addr_mode {
  // for instructions with no operands
  NONE = 0,
  // a 16 bit address
  ABSOLUTE = 1,
  // a 16 bit address + x
  ABSOLUTE_X = 2,
  // a 16 bit address + y
  ABSOLUTE_Y = 3,
  // the pointer at (an 8 bit address + x)
  X_INDIRECT = 4,
  // (the pointer at an 8 bit address) + y
  INDIRECT_Y = 5,
  // an 8 bit address
  ZERO_PAGE = 6,
  // an 8 bit address + x
  ZERO_PAGE_X = 7,
  // an 8 bit address + y
  ZERO_PAGE_Y = 8,
  // an arbitrary immediate constant
  IMMEDIATE = 9,
  // specific immediate constants (0, 1, FF, etc.)
  CONSTANT = 10,
};

inline uint8_t addr_mode_variants(addr_mode mode) {
  switch (mode) {
  case addr_mode::NONE:
    return 1;
  case addr_mode::ABSOLUTE:
  case addr_mode::ABSOLUTE_X:
  case addr_mode::ABSOLUTE_Y:
  case addr_mode::INDIRECT_Y:
  case addr_mode::X_INDIRECT:
  case addr_mode::ZERO_PAGE:
  case addr_mode::ZERO_PAGE_X:
  case addr_mode::ZERO_PAGE_Y:
  case addr_mode::IMMEDIATE:
    return 4;
  case addr_mode::CONSTANT:
    return 10;
  }
  return 0;
}

const char *addr_mode_operand_strings[] = {
  "", "absolute", "absolute", "absolute", "zp", "zp", "zp", "zp", "zp", "immediate"
};

const uint8_t addr_mode_constant_values[] = { 0, 1, 2, 4, 8, 16, 32, 64, 128, 255 };

std::string addr_mode_operand_name(addr_mode mode, uint8_t number) {
  if (mode == addr_mode::CONSTANT) {
    return std::to_string(addr_mode_constant_values[number]);
  } else if (mode == addr_mode::NONE) {
    return std::string{};
  } else {
    return addr_mode_operand_strings[(uint8_t)mode] + std::to_string(number);
  }
}

typedef struct instruction {
  uint16_t data;

  instruction(): data(0) {}

  instruction(instruction_name name, addr_mode mode, uint8_t number) {
    data = (uint16_t)name;
    data <<= 4;
    data |= (uint16_t)mode;
    data <<= 4;
    data |= number;
  }

  instruction_name name() const {
    return (instruction_name)((data & 0xFF00) >> 8);
  }

  addr_mode mode() const {
    return (addr_mode)((data & 0xF0) >> 4);
  }

  uint8_t number() const {
    return data & 0xF;
  }

  instruction number(uint8_t number) const {
    instruction result;
    result.data = (data & 0xFFF0) | (number & 0xF);
    return result;
  }

} instruction;

typedef struct instruction_info {
  instruction ins;
  uint8_t cycles;
  uint8_t bytes;
  const char *desc;

  bool operator<(const instruction_info &other) const {
    if (cycles < other.cycles) { return true; }
    else if (cycles == other.cycles) { return bytes < other.bytes; }
    return false;
  }
} instruction_info;

typedef struct instruction_seq {
  uint8_t cycles;
  uint8_t bytes;
  instruction instructions[7];

  instruction_seq(uint8_t c, uint8_t b) : cycles(c), bytes(b) {
    for (int i = 0; i < 7; i++) {
      instructions[i] = instruction(instruction_name::NONE, addr_mode::NONE, 0);
    }
  }

  instruction_seq() : instruction_seq(0, 0) {}

  instruction_seq(std::array<uint8_t, 16> data) {}

  instruction_seq add(instruction_info info) const {
    instruction_seq newSeq(this->cycles + info.cycles, this->bytes + info.bytes);
    for (int i = 0; i < 7; i++) {
      if (this->instructions[i].name() == instruction_name::NONE) {
        newSeq.instructions[i] = info.ins;
        break;
      } else {
        newSeq.instructions[i] = this->instructions[i];
      }
    }
    return newSeq;
  }

  bool operator>(const instruction_seq &other) const {
    if (cycles > other.cycles) { return true; }
    else if (cycles == other.cycles) { return bytes > other.bytes; }
    return false;
  }
} instruction_seq;

static instruction_info instructions[] = {
  { instruction(instruction_name::ADC, addr_mode::IMMEDIATE,   0), 20, 2, "adc.#" },
  { instruction(instruction_name::ADC, addr_mode::CONSTANT,    0), 20, 2, "adc.#" },
  { instruction(instruction_name::ADC, addr_mode::ZERO_PAGE,   0), 30, 2, "adc.z" },
  { instruction(instruction_name::ADC, addr_mode::ZERO_PAGE_X, 0), 40, 2, "adc.zx" },
  { instruction(instruction_name::ADC, addr_mode::ABSOLUTE,    0), 40, 3, "adc" },
  { instruction(instruction_name::ADC, addr_mode::ABSOLUTE_X,  0), 41, 3, "adc.x" },
  { instruction(instruction_name::ADC, addr_mode::ABSOLUTE_Y,  0), 41, 3, "adc.y" },
  { instruction(instruction_name::ADC, addr_mode::X_INDIRECT,  0), 60, 2, "adc.xi" },
  { instruction(instruction_name::ADC, addr_mode::INDIRECT_Y,  0), 51, 2, "adc.iy" },

  { instruction(instruction_name::AND, addr_mode::IMMEDIATE,   0), 20, 2, "and.#" },
  { instruction(instruction_name::AND, addr_mode::CONSTANT,    0), 20, 2, "and.#" },
  { instruction(instruction_name::AND, addr_mode::ZERO_PAGE,   0), 30, 2, "and.z" },
  { instruction(instruction_name::AND, addr_mode::ZERO_PAGE_X, 0), 40, 2, "and.zx" },
  { instruction(instruction_name::AND, addr_mode::ABSOLUTE,    0), 40, 3, "and" },
  { instruction(instruction_name::AND, addr_mode::ABSOLUTE_X,  0), 41, 3, "and.x" },
  { instruction(instruction_name::AND, addr_mode::ABSOLUTE_Y,  0), 41, 3, "and.y" },
  { instruction(instruction_name::AND, addr_mode::X_INDIRECT,  0), 60, 2, "and.xi" },
  { instruction(instruction_name::AND, addr_mode::INDIRECT_Y,  0), 51, 2, "and.iy" },

  { instruction(instruction_name::ASLA, addr_mode::NONE,       0), 20, 1, "asl.a" },
  { instruction(instruction_name::ASL, addr_mode::ZERO_PAGE,   0), 50, 2, "asl.z" },
  { instruction(instruction_name::ASL, addr_mode::ZERO_PAGE_X, 0), 60, 2, "asl.zx" },
  { instruction(instruction_name::ASL, addr_mode::ABSOLUTE,    0), 60, 3, "asl" },
  { instruction(instruction_name::ASL, addr_mode::ABSOLUTE_X,  0), 70, 3, "asl.x" },

  { instruction(instruction_name::BIT, addr_mode::ZERO_PAGE,   0), 30, 2, "bit.z" },
  { instruction(instruction_name::BIT, addr_mode::ABSOLUTE,    0), 40, 3, "bit" },

  { instruction(instruction_name::BPL, addr_mode::ABSOLUTE,    0), 25, 2, "bpl" },
  { instruction(instruction_name::BMI, addr_mode::ABSOLUTE,    0), 25, 2, "bmi" },
  { instruction(instruction_name::BVC, addr_mode::ABSOLUTE,    0), 25, 2, "bvc" },
  { instruction(instruction_name::BVS, addr_mode::ABSOLUTE,    0), 25, 2, "bvs" },
  { instruction(instruction_name::BCC, addr_mode::ABSOLUTE,    0), 25, 2, "bcc" },
  { instruction(instruction_name::BCS, addr_mode::ABSOLUTE,    0), 25, 2, "bcs" },
  { instruction(instruction_name::BNE, addr_mode::ABSOLUTE,    0), 25, 2, "bne" },
  { instruction(instruction_name::BEQ, addr_mode::ABSOLUTE,    0), 25, 2, "beq" },

  // { instruction(instruction_name::BRK, addr_mode::NONE,        0), 70, 2 },

  { instruction(instruction_name::CMP, addr_mode::IMMEDIATE,   0), 20, 2, "cmp.#" },
  { instruction(instruction_name::CMP, addr_mode::CONSTANT,    0), 20, 2, "cmp.#" },
  { instruction(instruction_name::CMP, addr_mode::ZERO_PAGE,   0), 30, 2, "cmp.z" },
  { instruction(instruction_name::CMP, addr_mode::ZERO_PAGE_X, 0), 40, 2, "cmp.zx" },
  { instruction(instruction_name::CMP, addr_mode::ABSOLUTE,    0), 40, 3, "cmp" },
  { instruction(instruction_name::CMP, addr_mode::ABSOLUTE_X,  0), 41, 3, "cmp.x" },
  { instruction(instruction_name::CMP, addr_mode::ABSOLUTE_Y,  0), 41, 3, "cmp.y" },
  { instruction(instruction_name::CMP, addr_mode::X_INDIRECT,  0), 60, 2, "cmp.xi" },
  { instruction(instruction_name::CMP, addr_mode::INDIRECT_Y,  0), 51, 2, "cmp.iy" },

  { instruction(instruction_name::CPX, addr_mode::IMMEDIATE,   0), 20, 2, "cpx.#" },
  { instruction(instruction_name::CPX, addr_mode::CONSTANT,    0), 20, 2, "cpx.#" },
  { instruction(instruction_name::CPX, addr_mode::ZERO_PAGE,   0), 30, 2, "cpx.z" },
  { instruction(instruction_name::CPX, addr_mode::ABSOLUTE,    0), 40, 3, "cpx" },

  { instruction(instruction_name::CPY, addr_mode::IMMEDIATE,   0), 20, 2, "cpy.#" },
  { instruction(instruction_name::CPY, addr_mode::CONSTANT,    0), 20, 2, "cpy.#" },
  { instruction(instruction_name::CPY, addr_mode::ZERO_PAGE,   0), 30, 2, "cpy.z" },
  { instruction(instruction_name::CPY, addr_mode::ABSOLUTE,    0), 40, 3, "cpy" },

  { instruction(instruction_name::DEC, addr_mode::ZERO_PAGE,   0), 50, 2, "dec.z" },
  { instruction(instruction_name::DEC, addr_mode::ZERO_PAGE_X, 0), 60, 2, "dec.zx" },
  { instruction(instruction_name::DEC, addr_mode::ABSOLUTE,    0), 60, 3, "dec" },
  { instruction(instruction_name::DEC, addr_mode::ABSOLUTE_X,  0), 70, 3, "dec.x" },

  { instruction(instruction_name::EOR, addr_mode::IMMEDIATE,   0), 20, 2, "eor.#" },
  { instruction(instruction_name::EOR, addr_mode::CONSTANT,    0), 20, 2, "eor.#" },
  { instruction(instruction_name::EOR, addr_mode::ZERO_PAGE,   0), 30, 2, "eor.z" },
  { instruction(instruction_name::EOR, addr_mode::ZERO_PAGE_X, 0), 40, 2, "eor.zx" },
  { instruction(instruction_name::EOR, addr_mode::ABSOLUTE,    0), 40, 3, "eor" },
  { instruction(instruction_name::EOR, addr_mode::ABSOLUTE_X,  0), 41, 3, "eor.x" },
  { instruction(instruction_name::EOR, addr_mode::ABSOLUTE_Y,  0), 41, 3, "eor.y" },
  { instruction(instruction_name::EOR, addr_mode::X_INDIRECT,  0), 60, 2, "eor.xi" },
  { instruction(instruction_name::EOR, addr_mode::INDIRECT_Y,  0), 51, 2, "eor.iy" },

  { instruction(instruction_name::CLC, addr_mode::NONE,        0), 20, 1, "clc" },
  { instruction(instruction_name::SEC, addr_mode::NONE,        0), 20, 1, "sec" },
  { instruction(instruction_name::CLI, addr_mode::NONE,        0), 20, 1, "cli" },
  { instruction(instruction_name::SEI, addr_mode::NONE,        0), 20, 1, "sei" },
  { instruction(instruction_name::CLV, addr_mode::NONE,        0), 20, 1, "clv" },
  { instruction(instruction_name::CLD, addr_mode::NONE,        0), 20, 1, "cld" },
  { instruction(instruction_name::SED, addr_mode::NONE,        0), 20, 1, "sed" },

  { instruction(instruction_name::INC, addr_mode::ZERO_PAGE,   0), 50, 2, "inc.z" },
  { instruction(instruction_name::INC, addr_mode::ZERO_PAGE_X, 0), 60, 2, "inc.zx" },
  { instruction(instruction_name::INC, addr_mode::ABSOLUTE,    0), 60, 3, "inc" },
  { instruction(instruction_name::INC, addr_mode::ABSOLUTE_X,  0), 70, 3, "inc.x" },

  { instruction(instruction_name::JMP, addr_mode::ABSOLUTE,  0), 30, 3, "jmp" },
  { instruction(instruction_name::JSR, addr_mode::ABSOLUTE,  0), 60, 3, "jsr" },

  { instruction(instruction_name::LDA, addr_mode::IMMEDIATE,   0), 20, 2, "lda.#" },
  { instruction(instruction_name::LDA, addr_mode::CONSTANT,    0), 20, 2, "lda.#" },
  { instruction(instruction_name::LDA, addr_mode::ZERO_PAGE,   0), 30, 2, "lda.z" },
  { instruction(instruction_name::LDA, addr_mode::ZERO_PAGE_X, 0), 40, 2, "lda.zx" },
  { instruction(instruction_name::LDA, addr_mode::ABSOLUTE,    0), 40, 3, "lda" },
  { instruction(instruction_name::LDA, addr_mode::ABSOLUTE_X,  0), 41, 3, "lda.x" },
  { instruction(instruction_name::LDA, addr_mode::ABSOLUTE_Y,  0), 41, 3, "lda.y" },
  { instruction(instruction_name::LDA, addr_mode::X_INDIRECT,  0), 60, 2, "lda.xi" },
  { instruction(instruction_name::LDA, addr_mode::INDIRECT_Y,  0), 51, 2, "lda.iy" },

  { instruction(instruction_name::LDX, addr_mode::IMMEDIATE,   0), 20, 2, "ldx.#" },
  { instruction(instruction_name::LDX, addr_mode::CONSTANT,    0), 20, 2, "ldx.#" },
  { instruction(instruction_name::LDX, addr_mode::ZERO_PAGE,   0), 30, 2, "ldx.z" },
  { instruction(instruction_name::LDX, addr_mode::ZERO_PAGE_Y, 0), 40, 2, "ldx.zy" },
  { instruction(instruction_name::LDX, addr_mode::ABSOLUTE,    0), 40, 3, "ldx" },
  { instruction(instruction_name::LDX, addr_mode::ABSOLUTE_Y,  0), 41, 3, "ldx.y" },

  { instruction(instruction_name::LDY, addr_mode::IMMEDIATE,   0), 20, 2, "ldy.#" },
  { instruction(instruction_name::LDY, addr_mode::CONSTANT,    0), 20, 2, "ldy.#" },
  { instruction(instruction_name::LDY, addr_mode::ZERO_PAGE,   0), 30, 2, "ldy.z" },
  { instruction(instruction_name::LDY, addr_mode::ZERO_PAGE_X, 0), 40, 2, "ldy.zx" },
  { instruction(instruction_name::LDY, addr_mode::ABSOLUTE,    0), 40, 3, "ldy" },
  { instruction(instruction_name::LDY, addr_mode::ABSOLUTE_X,  0), 41, 3, "ldy.x" },

  { instruction(instruction_name::LSRA, addr_mode::NONE,       0), 20, 1, "lsr.a" },
  { instruction(instruction_name::LSR, addr_mode::ZERO_PAGE,   0), 50, 2, "lsr.z" },
  { instruction(instruction_name::LSR, addr_mode::ZERO_PAGE_X, 0), 60, 2, "lsr.zx" },
  { instruction(instruction_name::LSR, addr_mode::ABSOLUTE,    0), 60, 3, "lsr" },
  { instruction(instruction_name::LSR, addr_mode::ABSOLUTE_X,  0), 70, 3, "lsr.x" },

  // { instruction(instruction_name::NOP, addr_mode::NONE,       0), 20, 1 },

  { instruction(instruction_name::ORA, addr_mode::IMMEDIATE,   0), 20, 2, "ora.#" },
  { instruction(instruction_name::ORA, addr_mode::CONSTANT,    0), 20, 2, "ora.#" },
  { instruction(instruction_name::ORA, addr_mode::ZERO_PAGE,   0), 30, 2, "ora.z" },
  { instruction(instruction_name::ORA, addr_mode::ZERO_PAGE_X, 0), 40, 2, "ora.zx" },
  { instruction(instruction_name::ORA, addr_mode::ABSOLUTE,    0), 40, 3, "ora" },
  { instruction(instruction_name::ORA, addr_mode::ABSOLUTE_X,  0), 41, 3, "ora.x" },
  { instruction(instruction_name::ORA, addr_mode::ABSOLUTE_Y,  0), 41, 3, "ora.y" },
  { instruction(instruction_name::ORA, addr_mode::X_INDIRECT,  0), 60, 2, "ora.xi" },
  { instruction(instruction_name::ORA, addr_mode::INDIRECT_Y,  0), 51, 2, "ora.iy" },

  { instruction(instruction_name::TAX, addr_mode::NONE,        0), 20, 1, "tax" },
  { instruction(instruction_name::TXA, addr_mode::NONE,        0), 20, 1, "txa" },
  { instruction(instruction_name::DEX, addr_mode::NONE,        0), 20, 1, "dex" },
  { instruction(instruction_name::INX, addr_mode::NONE,        0), 20, 1, "inx" },
  { instruction(instruction_name::TAY, addr_mode::NONE,        0), 20, 1, "tay" },
  { instruction(instruction_name::TYA, addr_mode::NONE,        0), 20, 1, "tya" },
  { instruction(instruction_name::DEY, addr_mode::NONE,        0), 20, 1, "dey" },
  { instruction(instruction_name::INY, addr_mode::NONE,        0), 20, 1, "iny" },

  { instruction(instruction_name::ROLA, addr_mode::NONE,       0), 20, 1, "rol.a" },
  { instruction(instruction_name::ROL, addr_mode::ZERO_PAGE,   0), 50, 2, "rol.z" },
  { instruction(instruction_name::ROL, addr_mode::ZERO_PAGE_X, 0), 60, 2, "rol.zx" },
  { instruction(instruction_name::ROL, addr_mode::ABSOLUTE,    0), 60, 3, "rol" },
  { instruction(instruction_name::ROL, addr_mode::ABSOLUTE_X,  0), 70, 3, "rol.x" },

  { instruction(instruction_name::RORA, addr_mode::NONE,       0), 20, 1, "ror.a" },
  { instruction(instruction_name::ROR, addr_mode::ZERO_PAGE,   0), 50, 2, "ror.z" },
  { instruction(instruction_name::ROR, addr_mode::ZERO_PAGE_X, 0), 60, 2, "ror.zx" },
  { instruction(instruction_name::ROR, addr_mode::ABSOLUTE,    0), 60, 3, "ror" },
  { instruction(instruction_name::ROR, addr_mode::ABSOLUTE_X,  0), 70, 3, "ror.x" },

  { instruction(instruction_name::RTI, addr_mode::NONE,        0), 60, 1, "rti" },
  { instruction(instruction_name::RTS, addr_mode::NONE,        0), 60, 1, "rts" },

  { instruction(instruction_name::SBC, addr_mode::IMMEDIATE,   0), 20, 2, "sbc.#" },
  { instruction(instruction_name::SBC, addr_mode::CONSTANT,    0), 20, 2, "sbc.#" },
  { instruction(instruction_name::SBC, addr_mode::ZERO_PAGE,   0), 30, 2, "sbc.z" },
  { instruction(instruction_name::SBC, addr_mode::ZERO_PAGE_X, 0), 40, 2, "sbc.zx" },
  { instruction(instruction_name::SBC, addr_mode::ABSOLUTE,    0), 40, 3, "sbc" },
  { instruction(instruction_name::SBC, addr_mode::ABSOLUTE_X,  0), 41, 3, "sbc.x" },
  { instruction(instruction_name::SBC, addr_mode::ABSOLUTE_Y,  0), 41, 3, "sbc.y" },
  { instruction(instruction_name::SBC, addr_mode::X_INDIRECT,  0), 60, 2, "sbc.xi" },
  { instruction(instruction_name::SBC, addr_mode::INDIRECT_Y,  0), 51, 2, "sbc.iy" },

  { instruction(instruction_name::STA, addr_mode::ZERO_PAGE,   0), 30, 2, "sta.z" },
  { instruction(instruction_name::STA, addr_mode::ZERO_PAGE_X, 0), 40, 2, "sta.zx" },
  { instruction(instruction_name::STA, addr_mode::ABSOLUTE,    0), 40, 3, "sta" },
  { instruction(instruction_name::STA, addr_mode::ABSOLUTE_X,  0), 50, 3, "sta.x" },
  { instruction(instruction_name::STA, addr_mode::ABSOLUTE_Y,  0), 50, 3, "sta.y" },
  { instruction(instruction_name::STA, addr_mode::X_INDIRECT,  0), 60, 2, "sta.xi" },
  { instruction(instruction_name::STA, addr_mode::INDIRECT_Y,  0), 60, 2, "sta.iy" },

  { instruction(instruction_name::TXS, addr_mode::NONE,        0), 20, 1, "txs" },
  { instruction(instruction_name::TSX, addr_mode::NONE,        0), 20, 1, "tsx" },
  { instruction(instruction_name::PHA, addr_mode::NONE,        0), 30, 1, "pha" },
  { instruction(instruction_name::PLA, addr_mode::NONE,        0), 40, 1, "pla" },
  { instruction(instruction_name::PHP, addr_mode::NONE,        0), 30, 1, "php" },
  { instruction(instruction_name::PLP, addr_mode::NONE,        0), 40, 1, "plp" },

  { instruction(instruction_name::STX, addr_mode::ZERO_PAGE,   0), 30, 2, "stx.z" },
  { instruction(instruction_name::STX, addr_mode::ZERO_PAGE_Y, 0), 40, 2, "stx.zy" },
  { instruction(instruction_name::STX, addr_mode::ABSOLUTE,    0), 40, 3, "stx" },

  { instruction(instruction_name::STY, addr_mode::ZERO_PAGE,   0), 30, 2, "sty.z" },
  { instruction(instruction_name::STY, addr_mode::ZERO_PAGE_X, 0), 40, 2, "sty.zx" },
  { instruction(instruction_name::STY, addr_mode::ABSOLUTE,    0), 40, 3, "sty" },
};
