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

} instruction;

typedef struct instruction_info {
  instruction ins;
  uint8_t cycles;
  uint8_t bytes;

  bool operator<(const instruction_info &other) const {
    if (cycles < other.cycles) { return true; }
    else if (cycles == other.cycles) { return bytes < other.bytes; }
    return false;
  }
} instruction_info;

typedef struct instruction_seq {
  const static int max_length;
  uint8_t cycles;
  uint8_t bytes;
  instruction instructions[7];

  instruction_seq(uint8_t c, uint8_t b) : cycles(c), bytes(b) {
    for (int i = 0; i < 7; i++) {
      instructions[i] = instruction(instruction_name::NONE, addr_mode::NONE, 0);
    }
  }

  instruction_seq() : instruction_seq(0, 0) {}

  //instruction_seq(std::array<uint8_t, 16> data) {}

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

const int instruction_seq::max_length = 7;

static instruction_info instructions[] = {
  { instruction(instruction_name::ADC, addr_mode::IMMEDIATE,   0), 20, 2 },
  { instruction(instruction_name::ADC, addr_mode::CONSTANT,    0), 20, 2 },
  { instruction(instruction_name::ADC, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::ADC, addr_mode::ZERO_PAGE_X, 0), 40, 2 },
  { instruction(instruction_name::ADC, addr_mode::ABSOLUTE,    0), 40, 3 },
  { instruction(instruction_name::ADC, addr_mode::ABSOLUTE_X,  0), 41, 3 },
  { instruction(instruction_name::ADC, addr_mode::ABSOLUTE_Y,  0), 41, 3 },
  { instruction(instruction_name::ADC, addr_mode::X_INDIRECT,  0), 60, 2 },
  { instruction(instruction_name::ADC, addr_mode::INDIRECT_Y,  0), 51, 2 },

  { instruction(instruction_name::AND, addr_mode::IMMEDIATE,   0), 20, 2 },
  { instruction(instruction_name::AND, addr_mode::CONSTANT,   0), 20, 2 },
  { instruction(instruction_name::AND, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::AND, addr_mode::ZERO_PAGE_X, 0), 40, 2 },
  { instruction(instruction_name::AND, addr_mode::ABSOLUTE,    0), 40, 3 },
  { instruction(instruction_name::AND, addr_mode::ABSOLUTE_X,  0), 41, 3 },
  { instruction(instruction_name::AND, addr_mode::ABSOLUTE_Y,  0), 41, 3 },
  { instruction(instruction_name::AND, addr_mode::X_INDIRECT,  0), 60, 2 },
  { instruction(instruction_name::AND, addr_mode::INDIRECT_Y,  0), 51, 2 },

  { instruction(instruction_name::ASLA, addr_mode::NONE,       0), 20, 1 },
  { instruction(instruction_name::ASL, addr_mode::ZERO_PAGE,   0), 50, 2 },
  { instruction(instruction_name::ASL, addr_mode::ZERO_PAGE_X, 0), 60, 2 },
  { instruction(instruction_name::ASL, addr_mode::ABSOLUTE,    0), 60, 3 },
  { instruction(instruction_name::ASL, addr_mode::ABSOLUTE_X,  0), 70, 3 },

  { instruction(instruction_name::BIT, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::BIT, addr_mode::ABSOLUTE,    0), 40, 3 },

  { instruction(instruction_name::BPL, addr_mode::ABSOLUTE,    0), 25, 2 },
  { instruction(instruction_name::BMI, addr_mode::ABSOLUTE,    0), 25, 2 },
  { instruction(instruction_name::BVC, addr_mode::ABSOLUTE,    0), 25, 2 },
  { instruction(instruction_name::BVS, addr_mode::ABSOLUTE,    0), 25, 2 },
  { instruction(instruction_name::BCC, addr_mode::ABSOLUTE,    0), 25, 2 },
  { instruction(instruction_name::BCS, addr_mode::ABSOLUTE,    0), 25, 2 },
  { instruction(instruction_name::BNE, addr_mode::ABSOLUTE,    0), 25, 2 },
  { instruction(instruction_name::BEQ, addr_mode::ABSOLUTE,    0), 25, 2 },

  // { instruction(instruction_name::BRK, addr_mode::NONE,        0), 70, 2 },

  { instruction(instruction_name::CMP, addr_mode::IMMEDIATE,   0), 20, 2 },
  { instruction(instruction_name::CMP, addr_mode::CONSTANT,   0), 20, 2 },
  { instruction(instruction_name::CMP, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::CMP, addr_mode::ZERO_PAGE_X, 0), 40, 2 },
  { instruction(instruction_name::CMP, addr_mode::ABSOLUTE,    0), 40, 3 },
  { instruction(instruction_name::CMP, addr_mode::ABSOLUTE_X,  0), 41, 3 },
  { instruction(instruction_name::CMP, addr_mode::ABSOLUTE_Y,  0), 41, 3 },
  { instruction(instruction_name::CMP, addr_mode::X_INDIRECT,  0), 60, 2 },
  { instruction(instruction_name::CMP, addr_mode::INDIRECT_Y,  0), 51, 2 },

  { instruction(instruction_name::CPX, addr_mode::IMMEDIATE,   0), 20, 2 },
  { instruction(instruction_name::CPX, addr_mode::CONSTANT,   0), 20, 2 },
  { instruction(instruction_name::CPX, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::CPX, addr_mode::ABSOLUTE,    0), 40, 3 },

  { instruction(instruction_name::CPY, addr_mode::IMMEDIATE,   0), 20, 2 },
  { instruction(instruction_name::CPY, addr_mode::CONSTANT,   0), 20, 2 },
  { instruction(instruction_name::CPY, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::CPY, addr_mode::ABSOLUTE,    0), 40, 3 },

  { instruction(instruction_name::DEC, addr_mode::ZERO_PAGE,   0), 50, 2 },
  { instruction(instruction_name::DEC, addr_mode::ZERO_PAGE_X, 0), 60, 2 },
  { instruction(instruction_name::DEC, addr_mode::ABSOLUTE,    0), 60, 3 },
  { instruction(instruction_name::DEC, addr_mode::ABSOLUTE_X,  0), 70, 3 },

  { instruction(instruction_name::EOR, addr_mode::IMMEDIATE,   0), 20, 2 },
  { instruction(instruction_name::EOR, addr_mode::CONSTANT,   0), 20, 2 },
  { instruction(instruction_name::EOR, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::EOR, addr_mode::ZERO_PAGE_X, 0), 40, 2 },
  { instruction(instruction_name::EOR, addr_mode::ABSOLUTE,    0), 40, 3 },
  { instruction(instruction_name::EOR, addr_mode::ABSOLUTE_X,  0), 41, 3 },
  { instruction(instruction_name::EOR, addr_mode::ABSOLUTE_Y,  0), 41, 3 },
  { instruction(instruction_name::EOR, addr_mode::X_INDIRECT,  0), 60, 2 },
  { instruction(instruction_name::EOR, addr_mode::INDIRECT_Y,  0), 51, 2 },

  { instruction(instruction_name::CLC, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::SEC, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::CLI, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::SEI, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::CLV, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::CLD, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::SED, addr_mode::NONE,        0), 20, 1 },

  { instruction(instruction_name::INC, addr_mode::ZERO_PAGE,   0), 50, 2 },
  { instruction(instruction_name::INC, addr_mode::ZERO_PAGE_X, 0), 60, 2 },
  { instruction(instruction_name::INC, addr_mode::ABSOLUTE,    0), 60, 3 },
  { instruction(instruction_name::INC, addr_mode::ABSOLUTE_X,  0), 70, 3 },

  { instruction(instruction_name::JMP, addr_mode::ABSOLUTE,  0), 30, 3 },
  { instruction(instruction_name::JSR, addr_mode::ABSOLUTE,  0), 60, 3 },

  { instruction(instruction_name::LDA, addr_mode::IMMEDIATE,   0), 20, 2 },
  { instruction(instruction_name::LDA, addr_mode::CONSTANT,   0), 20, 2 },
  { instruction(instruction_name::LDA, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::LDA, addr_mode::ZERO_PAGE_X, 0), 40, 2 },
  { instruction(instruction_name::LDA, addr_mode::ABSOLUTE,    0), 40, 3 },
  { instruction(instruction_name::LDA, addr_mode::ABSOLUTE_X,  0), 41, 3 },
  { instruction(instruction_name::LDA, addr_mode::ABSOLUTE_Y,  0), 41, 3 },
  { instruction(instruction_name::LDA, addr_mode::X_INDIRECT,  0), 60, 2 },
  { instruction(instruction_name::LDA, addr_mode::INDIRECT_Y,  0), 51, 2 },

  { instruction(instruction_name::LDX, addr_mode::IMMEDIATE,   0), 20, 2 },
  { instruction(instruction_name::LDX, addr_mode::CONSTANT,   0), 20, 2 },
  { instruction(instruction_name::LDX, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::LDX, addr_mode::ZERO_PAGE_Y, 0), 40, 2 },
  { instruction(instruction_name::LDX, addr_mode::ABSOLUTE,    0), 40, 3 },
  { instruction(instruction_name::LDX, addr_mode::ABSOLUTE_Y,  0), 41, 3 },

  { instruction(instruction_name::LDY, addr_mode::IMMEDIATE,   0), 20, 2 },
  { instruction(instruction_name::LDY, addr_mode::CONSTANT,   0), 20, 2 },
  { instruction(instruction_name::LDY, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::LDY, addr_mode::ZERO_PAGE_X, 0), 40, 2 },
  { instruction(instruction_name::LDY, addr_mode::ABSOLUTE,    0), 40, 3 },
  { instruction(instruction_name::LDY, addr_mode::ABSOLUTE_X,  0), 41, 3 },

  { instruction(instruction_name::LSRA, addr_mode::NONE,       0), 20, 1 },
  { instruction(instruction_name::LSR, addr_mode::ZERO_PAGE,   0), 50, 2 },
  { instruction(instruction_name::LSR, addr_mode::ZERO_PAGE_X, 0), 60, 2 },
  { instruction(instruction_name::LSR, addr_mode::ABSOLUTE,    0), 60, 3 },
  { instruction(instruction_name::LSR, addr_mode::ABSOLUTE_X,  0), 70, 3 },

  // { instruction(instruction_name::NOP, addr_mode::NONE,       0), 20, 1 },

  { instruction(instruction_name::ORA, addr_mode::IMMEDIATE,   0), 20, 2 },
  { instruction(instruction_name::ORA, addr_mode::CONSTANT,   0), 20, 2 },
  { instruction(instruction_name::ORA, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::ORA, addr_mode::ZERO_PAGE_X, 0), 40, 2 },
  { instruction(instruction_name::ORA, addr_mode::ABSOLUTE,    0), 40, 3 },
  { instruction(instruction_name::ORA, addr_mode::ABSOLUTE_X,  0), 41, 3 },
  { instruction(instruction_name::ORA, addr_mode::ABSOLUTE_Y,  0), 41, 3 },
  { instruction(instruction_name::ORA, addr_mode::X_INDIRECT,  0), 60, 2 },
  { instruction(instruction_name::ORA, addr_mode::INDIRECT_Y,  0), 51, 2 },

  { instruction(instruction_name::TAX, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::TXA, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::DEX, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::INX, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::TAY, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::TYA, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::DEY, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::INY, addr_mode::NONE,        0), 20, 1 },

  { instruction(instruction_name::ROLA, addr_mode::NONE,       0), 20, 1 },
  { instruction(instruction_name::ROL, addr_mode::ZERO_PAGE,   0), 50, 2 },
  { instruction(instruction_name::ROL, addr_mode::ZERO_PAGE_X, 0), 60, 2 },
  { instruction(instruction_name::ROL, addr_mode::ABSOLUTE,    0), 60, 3 },
  { instruction(instruction_name::ROL, addr_mode::ABSOLUTE_X,  0), 70, 3 },

  { instruction(instruction_name::RORA, addr_mode::NONE,       0), 20, 1 },
  { instruction(instruction_name::ROR, addr_mode::ZERO_PAGE,   0), 50, 2 },
  { instruction(instruction_name::ROR, addr_mode::ZERO_PAGE_X, 0), 60, 2 },
  { instruction(instruction_name::ROR, addr_mode::ABSOLUTE,    0), 60, 3 },
  { instruction(instruction_name::ROR, addr_mode::ABSOLUTE_X,  0), 70, 3 },

  { instruction(instruction_name::RTI, addr_mode::NONE,        0), 60, 1 },
  { instruction(instruction_name::RTS, addr_mode::NONE,        0), 60, 1 },

  { instruction(instruction_name::SBC, addr_mode::IMMEDIATE,   0), 20, 2 },
  { instruction(instruction_name::SBC, addr_mode::CONSTANT,   0), 20, 2 },
  { instruction(instruction_name::SBC, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::SBC, addr_mode::ZERO_PAGE_X, 0), 40, 2 },
  { instruction(instruction_name::SBC, addr_mode::ABSOLUTE,    0), 40, 3 },
  { instruction(instruction_name::SBC, addr_mode::ABSOLUTE_X,  0), 41, 3 },
  { instruction(instruction_name::SBC, addr_mode::ABSOLUTE_Y,  0), 41, 3 },
  { instruction(instruction_name::SBC, addr_mode::X_INDIRECT,  0), 60, 2 },
  { instruction(instruction_name::SBC, addr_mode::INDIRECT_Y,  0), 51, 2 },

  { instruction(instruction_name::STA, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::STA, addr_mode::ZERO_PAGE_X, 0), 40, 2 },
  { instruction(instruction_name::STA, addr_mode::ABSOLUTE,    0), 40, 3 },
  { instruction(instruction_name::STA, addr_mode::ABSOLUTE_X,  0), 50, 3 },
  { instruction(instruction_name::STA, addr_mode::ABSOLUTE_Y,  0), 50, 3 },
  { instruction(instruction_name::STA, addr_mode::X_INDIRECT,  0), 60, 2 },
  { instruction(instruction_name::STA, addr_mode::INDIRECT_Y,  0), 60, 2 },

  { instruction(instruction_name::TXS, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::TSX, addr_mode::NONE,        0), 20, 1 },
  { instruction(instruction_name::PHA, addr_mode::NONE,        0), 30, 1 },
  { instruction(instruction_name::PLA, addr_mode::NONE,        0), 40, 1 },
  { instruction(instruction_name::PHP, addr_mode::NONE,        0), 30, 1 },
  { instruction(instruction_name::PLP, addr_mode::NONE,        0), 40, 1 },

  { instruction(instruction_name::STX, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::STX, addr_mode::ZERO_PAGE_Y, 0), 40, 2 },
  { instruction(instruction_name::STX, addr_mode::ABSOLUTE,    0), 40, 3 },

  { instruction(instruction_name::STY, addr_mode::ZERO_PAGE,   0), 30, 2 },
  { instruction(instruction_name::STY, addr_mode::ZERO_PAGE_X, 0), 40, 2 },
  { instruction(instruction_name::STY, addr_mode::ABSOLUTE,    0), 40, 3 },
};

