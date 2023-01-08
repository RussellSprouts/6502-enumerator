#include "config.h"
#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "z3++.h"
#include "zstdint.h"
#include "initial_state.h"
#include "nstdint.h"
#include "special_immediate.h"

// An enum of instructions, including variants across different
// versions of the processor.
enum struct instruction_name: uint8_t {
  NONE = 0,

  // Basic 6502 instructions
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

  // 65c02 additional instructions
  PHX = 58,
  PHY = 59,
  PLX = 60,
  PLY = 61,
  STP = 64,
  STZ = 65,
  TRB = 66,
  TSB = 67,
  WAI = 68,
  BRA = 69,

  // Reset memory bit
  RMB0 = 70,
  RMB1 = 71,
  RMB2 = 72,
  RMB3 = 73,
  RMB4 = 74,
  RMB5 = 75,
  RMB6 = 76,
  RMB7 = 77,

  // Set memory bit
  SMB0 = 78,
  SMB1 = 79,
  SMB2 = 80,
  SMB3 = 81,
  SMB4 = 82,
  SMB5 = 83,
  SMB6 = 84,
  SMB7 = 85,

  // Branch if bit reset
  BBR0 = 86,
  BBR1 = 87,
  BBR2 = 88,
  BBR3 = 89,
  BBR4 = 90,
  BBR5 = 91,
  BBR6 = 92,
  BBR7 = 93,

  // Branch if bit set
  BBS0 = 94,
  BBS1 = 95,
  BBS2 = 96,
  BBS3 = 97,
  BBS4 = 98,
  BBS5 = 99,
  BBS6 = 100,
  BBS7 = 101,

  // NMOS 6502 stable undocumented instructions
  LAX = 102,
  SAX = 103,
  DCM = 104,
  INS = 105,
  SLO = 106,
  RLA = 107,
  SRE = 108,
  RRA = 109,
  ALR = 110,
  ANC = 111,
  AXS = 112,
  ARR = 113,

  UNKNOWN = 255,
};

std::string instruction_name_to_string(instruction_name ins) {
  switch(ins) {
  case instruction_name::ADC: return "ADC";
  case instruction_name::AND: return "AND"; 
  case instruction_name::ASL: return "ASL"; 
  case instruction_name::BCC: return "BCC"; 
  case instruction_name::BCS: return "BCS"; 
  case instruction_name::BEQ: return "BEQ"; 
  case instruction_name::BIT: return "BIT"; 
  case instruction_name::BMI: return "BMI"; 
  case instruction_name::BNE: return "BNE"; 
  case instruction_name::BPL: return "BPL"; 
  case instruction_name::BRK: return "BRK"; 
  case instruction_name::BVC: return "BVC"; 
  case instruction_name::BVS: return "BVS"; 
  case instruction_name::CLC: return "CLC"; 
  case instruction_name::CLD: return "CLD"; 
  case instruction_name::CLI: return "CLI"; 
  case instruction_name::CLV: return "CLV"; 
  case instruction_name::CMP: return "CMP"; 
  case instruction_name::CPX: return "CPX"; 
  case instruction_name::CPY: return "CPY"; 
  case instruction_name::DEC: return "DEC"; 
  case instruction_name::DEX: return "DEX"; 
  case instruction_name::DEY: return "DEY"; 
  case instruction_name::EOR: return "EOR"; 
  case instruction_name::INC: return "INC"; 
  case instruction_name::INX: return "INX"; 
  case instruction_name::INY: return "INY"; 
  case instruction_name::JMP: return "JMP"; 
  case instruction_name::JSR: return "JSR"; 
  case instruction_name::LDA: return "LDA"; 
  case instruction_name::LDX: return "LDX"; 
  case instruction_name::LDY: return "LDY"; 
  case instruction_name::LSR: return "LSR"; 
  case instruction_name::NOP: return "NOP"; 
  case instruction_name::ORA: return "ORA"; 
  case instruction_name::PHA: return "PHA"; 
  case instruction_name::PHP: return "PHP"; 
  case instruction_name::PLA: return "PLA"; 
  case instruction_name::PLP: return "PLP"; 
  case instruction_name::ROL: return "ROL"; 
  case instruction_name::ROR: return "ROR"; 
  case instruction_name::RTI: return "RTI"; 
  case instruction_name::RTS: return "RTS"; 
  case instruction_name::SBC: return "SBC"; 
  case instruction_name::SEC: return "SEC"; 
  case instruction_name::SED: return "SED"; 
  case instruction_name::SEI: return "SEI"; 
  case instruction_name::STA: return "STA"; 
  case instruction_name::STX: return "STX"; 
  case instruction_name::STY: return "STY"; 
  case instruction_name::TAX: return "TAX"; 
  case instruction_name::TAY: return "TAY"; 
  case instruction_name::TSX: return "TSX"; 
  case instruction_name::TXA: return "TXA"; 
  case instruction_name::TXS: return "TXS"; 
  case instruction_name::TYA: return "TYA"; 
  case instruction_name::PHX: return "PHX"; 
  case instruction_name::PHY: return "PHY"; 
  case instruction_name::PLX: return "PLX"; 
  case instruction_name::PLY: return "PLY"; 
  case instruction_name::STP: return "STP"; 
  case instruction_name::STZ: return "STZ"; 
  case instruction_name::TRB: return "TRB"; 
  case instruction_name::TSB: return "TSB"; 
  case instruction_name::WAI: return "WAI"; 
  case instruction_name::BRA: return "BRA";
  case instruction_name::RMB0: return "RMB0"; 
  case instruction_name::RMB1: return "RMB1"; 
  case instruction_name::RMB2: return "RMB2"; 
  case instruction_name::RMB3: return "RMB3"; 
  case instruction_name::RMB4: return "RMB4"; 
  case instruction_name::RMB5: return "RMB5"; 
  case instruction_name::RMB6: return "RMB6"; 
  case instruction_name::RMB7: return "RMB7"; 
  case instruction_name::SMB0: return "SMB0"; 
  case instruction_name::SMB1: return "SMB1"; 
  case instruction_name::SMB2: return "SMB2"; 
  case instruction_name::SMB3: return "SMB3"; 
  case instruction_name::SMB4: return "SMB4"; 
  case instruction_name::SMB5: return "SMB5"; 
  case instruction_name::SMB6: return "SMB6"; 
  case instruction_name::SMB7: return "SMB7"; 
  case instruction_name::BBR0: return "BBR0"; 
  case instruction_name::BBR1: return "BBR1"; 
  case instruction_name::BBR2: return "BBR2"; 
  case instruction_name::BBR3: return "BBR3"; 
  case instruction_name::BBR4: return "BBR4"; 
  case instruction_name::BBR5: return "BBR5"; 
  case instruction_name::BBR6: return "BBR6"; 
  case instruction_name::BBR7: return "BBR7"; 
  case instruction_name::BBS0: return "BBS0"; 
  case instruction_name::BBS1: return "BBS1"; 
  case instruction_name::BBS2: return "BBS2"; 
  case instruction_name::BBS3: return "BBS3"; 
  case instruction_name::BBS4: return "BBS4"; 
  case instruction_name::BBS5: return "BBS5"; 
  case instruction_name::BBS6: return "BBS6"; 
  case instruction_name::BBS7: return "BBS7"; 
  case instruction_name::LAX: return "LAX"; 
  case instruction_name::SAX: return "SAX"; 
  case instruction_name::DCM: return "DCM"; 
  case instruction_name::INS: return "INS"; 
  case instruction_name::SLO: return "SLO"; 
  case instruction_name::RLA: return "RLA"; 
  case instruction_name::SRE: return "SRE"; 
  case instruction_name::RRA: return "RRA"; 
  case instruction_name::ALR: return "ALR"; 
  case instruction_name::ANC: return "ANC"; 
  case instruction_name::AXS: return "AXS"; 
  case instruction_name::ARR: return "ARR";
  case instruction_name::NONE: return "; END";
  case instruction_name::UNKNOWN: return "XXX";
  }
  return "XXX";
}

// Instructions can have const payloads or arbitrary payloads
// with various addressing modes.
// Arbitrary payloads are either z3 named variables, representing
// an unknown initial state, or a value based of a hash of the name.
// For example, ABSOLUTE with payload 3, will be a z3 value named Absolute3
// or an arbitrary consistent value based on the hash seed.
// Constant payloads are actual values.
enum struct instruction_payload_type: uint8_t {
  // Flag indicating the payload is a constant value.
  CONST_FLAG = 0x80,

  // for instructions with no operands
  NONE = 0x0,
  // an arbitrary 16 bit address
  ABSOLUTE = 0x1,
  // am arbitrary 16 bit address + x
  ABSOLUTE_X = 0x2,
  // an arbitrary 16 bit address + y
  ABSOLUTE_Y = 0x3,
  // the pointer at (an arbitrary 8 bit address + x)
  X_INDIRECT = 0x4,
  // (the pointer at an arbitrary 8 bit address) + y
  INDIRECT_Y = 0x5,
  // an arbitrary 8 bit address
  ZERO_PAGE = 0x6,
  // an arbitrary 8 bit address + x
  ZERO_PAGE_X = 0x7,
  // an arbitrary 8 bit address + y
  ZERO_PAGE_Y = 0x8,
  // an arbitrary immediate constant
  IMMEDIATE = 0x9,
  // the pointer at (an arbitrary 16 bit address)
  INDIRECT_ABSOLUTE = 0xA,
  // the pointer at (an arbitrary 16 bit address + x)
  X_INDIRECT_ABSOLUTE = 0xB,
  // the pointer at (an arbitrary 8 bit address)
  ZERO_PAGE_INDIRECT = 0xC,
  // relative branch address (an arbitrary 16 bit address)
  RELATIVE = 0xD,
  // 8 bit relative branch and 8 bit address
  BRANCH_IF_BIT = 0xE,
  // Software stack addressing on the zero page using the X register
  // as the stack pointer.
  // i.e., lda Stack+offset0, x
  SOFTWARE_STACK = 0xF,
  // i.e., lda (Stack + offset0, x)
  SOFTWARE_STACK_INDIRECT = 0x10,
  // A reference to a known subroutine with the given ID. Allows
  // defining known subroutines and optimizing code which calls them.
  KNOWN_SUBROUTINE_CONST = 0x11 | CONST_FLAG,
  // A temporary memory location
  ZERO_PAGE_TEMP = 0x12,
  // A temporary memory location + x
  ZERO_PAGE_X_TEMP = 0x13,
  // A temporary memory location + y
  ZERO_PAGE_Y_TEMP = 0x14,
  // The pointer at (a temporary memory location) + y
  INDIRECT_Y_TEMP = 0x15,
  // The pointer at (a temporary memory location + x)
  X_INDIRECT_TEMP = 0x16,
  // A special immediate value. Payload is special_immediate
  SPECIAL_IMMEDIATE = 0x17,
  // Read-Modify-Write a zero page location.
  RMW_ZERO_PAGE = 0x18,
  // Read-Modify-Write a zero page location + x
  RMW_ZERO_PAGE_X = 0x19,
  // Read-Modify-Write an absolute location
  RMW_ABSOLUTE = 0x1A,
  // Read-Modify-Write an absolute location + x
  RMW_ABSOLUTE_X = 0x1B,
  // Read-Modify-Write a temporary memory location
  RMW_ZERO_PAGE_TEMP = 0x1C,
  // Read-Modify-Write a temporary memory location + x
  RMW_ZERO_PAGE_X_TEMP = 0x1D,

  // a specific 16 bit address
  ABSOLUTE_CONST = ABSOLUTE | CONST_FLAG,
  // a specific 16 bit address + x
  ABSOLUTE_X_CONST = ABSOLUTE_X | CONST_FLAG,
  // a specific 16 bit address + y
  ABSOLUTE_Y_CONST = ABSOLUTE_Y | CONST_FLAG,
  // the pointer at (a specific 8 bit address + x)
  X_INDIRECT_CONST = X_INDIRECT | CONST_FLAG,
  // (the pointer at a specific 8 bit address) + y
  INDIRECT_Y_CONST = INDIRECT_Y | CONST_FLAG,
  // a specific 8 bit address
  ZERO_PAGE_CONST = ZERO_PAGE | CONST_FLAG,
  // a specific 8 bit address + x
  ZERO_PAGE_X_CONST = ZERO_PAGE_X | CONST_FLAG,
  // a specific 8 bit address + y
  ZERO_PAGE_Y_CONST = ZERO_PAGE_Y | CONST_FLAG,
  // a specific immediate constant
  IMMEDIATE_CONST = IMMEDIATE | CONST_FLAG,
  // the pointer at (a specific 16 bit address)
  INDIRECT_ABSOLUTE_CONST = INDIRECT_ABSOLUTE | CONST_FLAG,
  // the pointer at (a specific 16 bit address + x)
  X_INDIRECT_ABSOLUTE_CONST = X_INDIRECT_ABSOLUTE | CONST_FLAG,
  // the pointer at (a specific 8 bit address)
  ZERO_PAGE_INDIRECT_CONST = ZERO_PAGE_INDIRECT | CONST_FLAG,
  // relative branch address (a specific 16 bit address)
  RELATIVE_CONST = RELATIVE | CONST_FLAG,
  // 8 bit relative branch and 8 bit address
  BRANCH_IF_BIT_CONST = BRANCH_IF_BIT | CONST_FLAG,
  // a specific offset from the software stack, using x as the stack pointer.
  // e.g. lda Stack+2, x
  SOFTWARE_STACK_CONST = SOFTWARE_STACK | CONST_FLAG,
  // the pointer at a specific offset from the software stack, using x as the
  // stack pointer.
  // e.g. lda (Stack+2, x)
  SOFTWARE_STACK_INDIRECT_CONST = SOFTWARE_STACK_INDIRECT | CONST_FLAG,
  ZERO_PAGE_TEMP_CONST = ZERO_PAGE_TEMP | CONST_FLAG,
  // A temporary memory location + x
  ZERO_PAGE_X_TEMP_CONST = ZERO_PAGE_X_TEMP | CONST_FLAG,
  // A temporary memory location + y
  ZERO_PAGE_Y_TEMP_CONST = ZERO_PAGE_Y_TEMP | CONST_FLAG,
  // The pointer at (a temporary memory location) + y
  INDIRECT_Y_TEMP_CONST = INDIRECT_Y_TEMP | CONST_FLAG,
  // The pointer at (a temporary memory location + x)
  X_INDIRECT_TEMP_CONST = X_INDIRECT_TEMP | CONST_FLAG,
  // Read-Modify-Write a specific zero page location.
  RMW_ZERO_PAGE_CONST = RMW_ZERO_PAGE | CONST_FLAG,
  // Read-Modify-Write a specific zero page location + x
  RMW_ZERO_PAGE_X_CONST = RMW_ZERO_PAGE_X | CONST_FLAG,
  // Read-Modify-Write a specific absolute location
  RMW_ABSOLUTE_CONST = RMW_ABSOLUTE | CONST_FLAG,
  // Read-Modify-Write a specific absolute location + x
  RMW_ABSOLUTE_X_CONST = RMW_ABSOLUTE_X | CONST_FLAG,
  // Read-Modify-Write a temporary memory location
  RMW_ZERO_PAGE_TEMP_CONST = RMW_ZERO_PAGE_TEMP | CONST_FLAG,
  // Read-Modify-Write a temporary memory location + x
  RMW_ZERO_PAGE_X_TEMP_CONST = RMW_ZERO_PAGE_X_TEMP | CONST_FLAG,
};

constexpr uint8_t instruction_size(instruction_payload_type pt) {
  switch (pt) {
  case instruction_payload_type::NONE:
    return 1;
  case instruction_payload_type::IMMEDIATE_CONST:
  case instruction_payload_type::IMMEDIATE:
  case instruction_payload_type::INDIRECT_Y_CONST:
  case instruction_payload_type::INDIRECT_Y:
  case instruction_payload_type::RELATIVE_CONST:
  case instruction_payload_type::RELATIVE:
  case instruction_payload_type::SOFTWARE_STACK_CONST:
  case instruction_payload_type::SOFTWARE_STACK:
  case instruction_payload_type::SOFTWARE_STACK_INDIRECT_CONST:
  case instruction_payload_type::SOFTWARE_STACK_INDIRECT:
  case instruction_payload_type::X_INDIRECT_CONST:
  case instruction_payload_type::X_INDIRECT:
  case instruction_payload_type::ZERO_PAGE_CONST:
  case instruction_payload_type::ZERO_PAGE_INDIRECT_CONST:
  case instruction_payload_type::ZERO_PAGE_INDIRECT:
  case instruction_payload_type::ZERO_PAGE_X_CONST:
  case instruction_payload_type::ZERO_PAGE_X:
  case instruction_payload_type::ZERO_PAGE_Y_CONST:
  case instruction_payload_type::ZERO_PAGE_Y:
  case instruction_payload_type::ZERO_PAGE:
  case instruction_payload_type::ZERO_PAGE_TEMP:
  case instruction_payload_type::ZERO_PAGE_TEMP_CONST:
  case instruction_payload_type::ZERO_PAGE_X_TEMP:
  case instruction_payload_type::ZERO_PAGE_X_TEMP_CONST:
  case instruction_payload_type::ZERO_PAGE_Y_TEMP:
  case instruction_payload_type::ZERO_PAGE_Y_TEMP_CONST:
  case instruction_payload_type::INDIRECT_Y_TEMP:
  case instruction_payload_type::INDIRECT_Y_TEMP_CONST:
  case instruction_payload_type::X_INDIRECT_TEMP:
  case instruction_payload_type::X_INDIRECT_TEMP_CONST:
  case instruction_payload_type::SPECIAL_IMMEDIATE:
  case instruction_payload_type::RMW_ZERO_PAGE:
  case instruction_payload_type::RMW_ZERO_PAGE_CONST:
  case instruction_payload_type::RMW_ZERO_PAGE_X:
  case instruction_payload_type::RMW_ZERO_PAGE_X_CONST:
  case instruction_payload_type::RMW_ZERO_PAGE_TEMP:
  case instruction_payload_type::RMW_ZERO_PAGE_TEMP_CONST:
  case instruction_payload_type::RMW_ZERO_PAGE_X_TEMP:
  case instruction_payload_type::RMW_ZERO_PAGE_X_TEMP_CONST:
    return 2;
  case instruction_payload_type::ABSOLUTE_CONST:
  case instruction_payload_type::ABSOLUTE_X_CONST:
  case instruction_payload_type::ABSOLUTE_X:
  case instruction_payload_type::ABSOLUTE_Y_CONST:
  case instruction_payload_type::ABSOLUTE_Y:
  case instruction_payload_type::ABSOLUTE:
  case instruction_payload_type::BRANCH_IF_BIT_CONST:
  case instruction_payload_type::BRANCH_IF_BIT:
  case instruction_payload_type::INDIRECT_ABSOLUTE_CONST:
  case instruction_payload_type::INDIRECT_ABSOLUTE:
  case instruction_payload_type::KNOWN_SUBROUTINE_CONST:
  case instruction_payload_type::X_INDIRECT_ABSOLUTE_CONST:
  case instruction_payload_type::X_INDIRECT_ABSOLUTE:
  case instruction_payload_type::RMW_ABSOLUTE:
  case instruction_payload_type::RMW_ABSOLUTE_CONST:
  case instruction_payload_type::RMW_ABSOLUTE_X:
  case instruction_payload_type::RMW_ABSOLUTE_X_CONST:
    return 3;
  case instruction_payload_type::CONST_FLAG:
    assert(false);
  }
  return 0;
}

constexpr instruction_payload_type operator|(instruction_payload_type lhs, instruction_payload_type rhs)
{
    return static_cast<instruction_payload_type> (
        static_cast<uint8_t>(lhs) |
        static_cast<uint8_t>(rhs)
    );
}

constexpr instruction_payload_type operator&(instruction_payload_type lhs, instruction_payload_type rhs)
{
    return static_cast<instruction_payload_type> (
        static_cast<uint8_t>(lhs) &
        static_cast<uint8_t>(rhs)
    );
}

constexpr instruction_payload_type operator~(instruction_payload_type lhs)
{
    return static_cast<instruction_payload_type> (
        ~static_cast<uint8_t>(lhs)
    );
}

/**
 * Instruction encoding in 4 bytes. Includes instructions from
 * all processor versions and payloads of constant values or
 * symbolic values.
 */
struct instruction {
  explicit instruction(instruction_name name, instruction_payload_type payload_type, uint16_t payload) :
    name(name),
    payload_type(payload_type),
    payload(payload) {}

  instruction_name name;
  instruction_payload_type payload_type;
  uint16_t payload;
};

std::string instruction_to_string(instruction ins) {
  std::string name = instruction_name_to_string(ins.name);

  switch (ins.payload_type) {
    case instruction_payload_type::ABSOLUTE:
    case instruction_payload_type::RMW_ABSOLUTE:
      return name + " absolute" + std::to_string(ins.payload);
    case instruction_payload_type::ABSOLUTE_CONST:
    case instruction_payload_type::RMW_ABSOLUTE_CONST:
      return name + " $" + int_to_hex(ins.payload, 4);
    case instruction_payload_type::ABSOLUTE_X:
    case instruction_payload_type::RMW_ABSOLUTE_X:
      return name + " absolute" + std::to_string(ins.payload) + ", x";
    case instruction_payload_type::ABSOLUTE_X_CONST:
    case instruction_payload_type::RMW_ABSOLUTE_X_CONST:
      return name + " $" + int_to_hex(ins.payload, 4) + ", x";
    case instruction_payload_type::ABSOLUTE_Y:
      return name + " absolute" + std::to_string(ins.payload) + ", y";
    case instruction_payload_type::ABSOLUTE_Y_CONST:
      return name + " $" + int_to_hex(ins.payload, 4) + ", y";
    case instruction_payload_type::BRANCH_IF_BIT:
      return name + " relative" + std::to_string(ins.payload >> 8) + ", zp" + std::to_string(ins.payload & 0xFF);
    case instruction_payload_type::BRANCH_IF_BIT_CONST:
      return name + " $" + int_to_hex(ins.payload >> 8, 2) + ", $" + int_to_hex(ins.payload & 0xFF, 2);
    case instruction_payload_type::IMMEDIATE:
      return name + " #immediate" + std::to_string(ins.payload);
    case instruction_payload_type::IMMEDIATE_CONST:
      return name + " #$" + int_to_hex(ins.payload, 2);
    case instruction_payload_type::INDIRECT_ABSOLUTE:
      return name + " [absolute" + std::to_string(ins.payload) + "]";
    case instruction_payload_type::INDIRECT_ABSOLUTE_CONST:
      return name + " [$" + int_to_hex(ins.payload, 4) + "]";
    case instruction_payload_type::INDIRECT_Y:
      return name + " [zp" + std::to_string(ins.payload) + "], y";
    case instruction_payload_type::INDIRECT_Y_CONST:
      return name + " [$" + int_to_hex(ins.payload, 2) + "], y";
    case instruction_payload_type::INDIRECT_Y_TEMP:
      return name + " [temp + offset" + std::to_string(ins.payload) + "], y";
    case instruction_payload_type::INDIRECT_Y_TEMP_CONST:
      return name + " [temp + $" + int_to_hex(ins.payload, 2) + "], y";
    case instruction_payload_type::KNOWN_SUBROUTINE_CONST:
      return name + " subroutine" + std::to_string(ins.payload);
    case instruction_payload_type::NONE:
      return name;
    case instruction_payload_type::RELATIVE:
      return name + " relative" + std::to_string(ins.payload);
    case instruction_payload_type::RELATIVE_CONST:
      return name + " $" + int_to_hex(ins.payload, 2);
    case instruction_payload_type::SOFTWARE_STACK:
      return name + " stack + offset" + std::to_string(ins.payload) + ", x";
    case instruction_payload_type::SOFTWARE_STACK_CONST:
      if (ins.payload == 0) {
        return name + " stack, x";
      }
      return name + " stack + $" + int_to_hex(ins.payload, 2) + ", x";
    case instruction_payload_type::SOFTWARE_STACK_INDIRECT:
      return name + " [stack + offset" + std::to_string(ins.payload) + ", x]";
    case instruction_payload_type::SOFTWARE_STACK_INDIRECT_CONST:
      if (ins.payload == 0) {
        return name + " [stack, x]";
      }
      return name + " [stack + $" + int_to_hex(ins.payload, 2) + ", x]";
    case instruction_payload_type::SPECIAL_IMMEDIATE:
      return name + " #" + special_immediate_to_string(special_immediate::from_int(ins.payload));
    case instruction_payload_type::X_INDIRECT_ABSOLUTE:
      return name + " [absolute" + std::to_string(ins.payload) + ", x]";
    case instruction_payload_type::X_INDIRECT_ABSOLUTE_CONST:
      return name + " [$" + int_to_hex(ins.payload, 4) + ", x]";
    case instruction_payload_type::X_INDIRECT:
      return name + " [zp" + std::to_string(ins.payload) + ", x]";
    case instruction_payload_type::X_INDIRECT_CONST:
      return name + " [$" + int_to_hex(ins.payload, 2) + ", x]";
    case instruction_payload_type::X_INDIRECT_TEMP:
      return name + " [temp + offset" + std::to_string(ins.payload) + ", x]";
    case instruction_payload_type::X_INDIRECT_TEMP_CONST:
      return name + " [temp + $" + int_to_hex(ins.payload, 2) + ", x]";
    case instruction_payload_type::ZERO_PAGE:
    case instruction_payload_type::RMW_ZERO_PAGE:
      return name + " zp" + std::to_string(ins.payload);
    case instruction_payload_type::ZERO_PAGE_CONST:
    case instruction_payload_type::RMW_ZERO_PAGE_CONST:
      return name + " $" + int_to_hex(ins.payload, 2);
    case instruction_payload_type::ZERO_PAGE_INDIRECT:
      return name + " [zp" + std::to_string(ins.payload) + "]";
    case instruction_payload_type::ZERO_PAGE_INDIRECT_CONST:
      return name + " [$" + int_to_hex(ins.payload, 2) + "]";
    case instruction_payload_type::ZERO_PAGE_TEMP:
    case instruction_payload_type::RMW_ZERO_PAGE_TEMP:
      return name + " temp + offset" + std::to_string(ins.payload);
    case instruction_payload_type::ZERO_PAGE_TEMP_CONST:
    case instruction_payload_type::RMW_ZERO_PAGE_TEMP_CONST:
      return name + " temp + $" + int_to_hex(ins.payload, 2);
    case instruction_payload_type::ZERO_PAGE_X:
    case instruction_payload_type::RMW_ZERO_PAGE_X:
      return name + " zp" + std::to_string(ins.payload) + ", x";
    case instruction_payload_type::ZERO_PAGE_X_CONST:
    case instruction_payload_type::RMW_ZERO_PAGE_X_CONST:
      return name + " $" + int_to_hex(ins.payload, 2) + ", x";
    case instruction_payload_type::ZERO_PAGE_X_TEMP:
    case instruction_payload_type::RMW_ZERO_PAGE_X_TEMP:
      return name + " temp + offset" + std::to_string(ins.payload) + ", x";
    case instruction_payload_type::ZERO_PAGE_X_TEMP_CONST:
    case instruction_payload_type::RMW_ZERO_PAGE_X_TEMP_CONST:
      return name + " temp + $" + int_to_hex(ins.payload, 2) + ", x";
    case instruction_payload_type::ZERO_PAGE_Y:
      return name + " zp" + std::to_string(ins.payload) + ", y";
    case instruction_payload_type::ZERO_PAGE_Y_CONST:
      return name + " $" + int_to_hex(ins.payload, 2) + ", y";
    case instruction_payload_type::ZERO_PAGE_Y_TEMP:
      return name + " temp + offset" + std::to_string(ins.payload) + ", y";
    case instruction_payload_type::ZERO_PAGE_Y_TEMP_CONST:
      return name + " temp + $" + int_to_hex(ins.payload, 2) + ", y";
    case instruction_payload_type::CONST_FLAG:
      assert(false);
  }
  return name;
}

enum struct usage_flags : uint16_t {
  NONE = 0x0,
  A = 0x1,
  X = 0x2,
  Y = 0x4,
  SP = 0x8,
  CC_S = 0x10,
  CC_V = 0x20,
  CC_I = 0x40,
  CC_D = 0x80,
  CC_C = 0x100,
  CC_Z = 0x200,
  MEMORY = 0x400
};

constexpr usage_flags operator&(const usage_flags lhs, const usage_flags rhs)
{
    return static_cast<usage_flags> (
        static_cast<uint16_t>(lhs) &
        static_cast<uint16_t>(rhs)
    );
}
constexpr usage_flags operator|(const usage_flags lhs, const usage_flags rhs)
{
    return static_cast<usage_flags> (
        static_cast<uint16_t>(lhs) |
        static_cast<uint16_t>(rhs)
    );
}


constexpr inline usage_flags& operator|=(usage_flags& lhs, usage_flags rhs)
{
  return lhs = lhs | rhs;
}

std::string usage_flags_to_string(usage_flags flags) {
  return std::string("")
    + ((flags & usage_flags::A) == usage_flags::NONE ? "" : "a")
    + ((flags & usage_flags::X) == usage_flags::NONE ? "" : "x")
    + ((flags & usage_flags::Y) == usage_flags::NONE ? "" : "y")
    + ((flags & usage_flags::SP) == usage_flags::NONE ? "" : "s")
    + ((flags & usage_flags::MEMORY) == usage_flags::NONE ? "" : "m")
    + "("
    + ((flags & usage_flags::CC_S) == usage_flags::NONE ? "" : "s")
    + ((flags & usage_flags::CC_V) == usage_flags::NONE ? "" : "v")
    + ((flags & usage_flags::CC_I) == usage_flags::NONE ? "" : "i")
    + ((flags & usage_flags::CC_D) == usage_flags::NONE ? "" : "d")
    + ((flags & usage_flags::CC_C) == usage_flags::NONE ? "" : "c")
    + ((flags & usage_flags::CC_Z) == usage_flags::NONE ? "" : "z")
    + ")";
}

/**
 * @brief An emulator which can operate on z3 symbolic values or actual numbers.
 * 
 * @tparam u8 
 * @tparam u16 
 * @tparam boolean 
 * @tparam memory_t 
 * @tparam initial_state 
 */
template<typename u8, typename u16, typename boolean, typename memory_t, typename initial_state>
struct machine {
  u8 _a;
  u8 _x;
  u8 _y;
  u8 _sp;
  boolean _cc_s;
  boolean _cc_v;
  boolean _cc_i;
  boolean _cc_d;
  boolean _cc_c;
  boolean _cc_z;
  u16 _pc;
  boolean _assumptions;
  boolean _has_exited;
  memory_t _memory;
  u16 _cycles;
  initial_state& _initial_state;
  usage_flags _uses;
  usage_flags _affects;

  machine(initial_state &s):
    _a(s.u8("a")),
    _x(s.u8("x")),
    _y(s.u8("y")),
    _sp(s.u8("sp")),
    _cc_s(s.boolean("cc_s")),
    _cc_v(s.boolean("cc_v")),
    _cc_i(s.boolean("cc_i")),
    _cc_d(s.boolean("cc_d")),
    _cc_c(s.boolean("cc_c")),
    _cc_z(s.boolean("cc_z")),
    _pc(s.u16("pc")),
    _assumptions(s.boolean(true)),
    _has_exited(s.boolean(false)),
    _memory(s.memory()),
    _cycles(0),
    _initial_state(s),
    _uses(usage_flags::NONE),
    _affects(usage_flags::NONE) {}

  template <typename other_machine>
  void copy_from(other_machine &other) {
    _has_exited = false;
    a(other.a().val);
    x(other.x().val);
    y(other.y().val);
    sp(other.sp().val);
    cc_s(other.cc_s());
    cc_v(other.cc_v());
    cc_i(other.cc_i());
    cc_d(other.cc_d());
    cc_c(other.cc_c());
    cc_z(other.cc_z());
    pc(other.pc().val);
  }

  // When updating any part of the machine state,
  // we guard on whether the machine has already exited
  // the current instruction sequence. That way, we can
  // execute a series of instructions unconditionally, but
  // model the case where a conditional branch is taken.
  u8 a() { _uses |= usage_flags::A; return _a; }
  void a(u8 val) { _affects |= usage_flags::A; _a = ite8(_has_exited, _a, val); }
  u8 x() { _uses |= usage_flags::X; return _x; }
  void x(u8 val) { _affects |= usage_flags::X; _x = ite8(_has_exited, _x, val); }
  u8 y() { _uses |= usage_flags::Y; return _y; }
  void y(u8 val) { _affects |= usage_flags::Y; _y = ite8(_has_exited, _y, val); }
  u8 sp() { _uses |= usage_flags::SP; return _sp; }
  void sp(u8 val) { _affects |= usage_flags::SP; _sp = ite8(_has_exited, _sp, val); }
  boolean cc_s() { _uses |= usage_flags::CC_S; return _cc_s; }
  void cc_s(boolean val) { _affects |= usage_flags::CC_S; _cc_s = iteB(_has_exited, _cc_s, val); }
  boolean cc_v() { _uses |= usage_flags::CC_V; return _cc_v; }
  void cc_v(boolean val) { _affects |= usage_flags::CC_V; _cc_v = iteB(_has_exited, _cc_v, val); }
  boolean cc_i() { _uses |= usage_flags::CC_I; return _cc_i; }
  void cc_i(boolean val) { _affects |= usage_flags::CC_I; _cc_i = iteB(_has_exited, _cc_i, val); }
  boolean cc_d() { _uses |= usage_flags::CC_D; return _cc_d; }
  void cc_d(boolean val) { _affects |= usage_flags::CC_D; _cc_d = iteB(_has_exited, _cc_d, val); }
  boolean cc_c() { _uses |= usage_flags::CC_C; return _cc_c; }
  void cc_c(boolean val) { _affects |= usage_flags::CC_C; _cc_c = iteB(_has_exited, _cc_c, val); }
  boolean cc_z() { _uses |= usage_flags::CC_Z; return _cc_z; }
  void cc_z(boolean val) { _affects |= usage_flags::CC_Z; _cc_z = iteB(_has_exited, _cc_z, val); }
  u16 pc() const { return _pc; }
  void pc(u16 val) { _pc = ite16(_has_exited, _pc, val); }
  u16 cycles() { return _cycles; }
  void cycles(u16 val) { _cycles = ite16(_has_exited, _cycles, val); }

  // Add an assumption that must be true for the current execution
  // to be valid and defined behavior. If the assumptions end up
  // false, that means that the execution of the instructions resulted
  // in undefined behavior, so it should be ignored.
  void assume(boolean val) {
    _assumptions = _assumptions && (_has_exited || val);
  }
  void exit_if(boolean cond, u16 target) {
    pc(ite16(cond, target, _pc)); 
    _has_exited = _has_exited || cond;
  }
  void exit(u16 target) {
    exit_if(true, target);
  }
  void reset_exit() {
    _has_exited = false;
  }
  void branch_if(boolean cond, u16 target, boolean branch_on_different_page) {
    cycles(cycles() + u16(cond) + u16(cond && branch_on_different_page));
    exit_if(cond, target);
  }

  u8 set_sz(u8 val) {
    cc_s(val >= 0x80);
    cc_z(val == 0);
    return val;
  }

  u8 read(u16 addr) {
    if (ASSUME_VALID_STACK_USAGE) {
      assume((addr & 0xFF00) != 0x100 || ((addr & 0xFF) > u16(_sp)));
    }
    _uses |= usage_flags::MEMORY;
    return _memory.read(addr);
  }

  void write(u16 addr, u8 val) {
    if (ASSUME_VALID_STACK_USAGE) {
      assume((addr & 0xFF00) != 0x100 || ((addr & 0xFF) > u16(_sp)));
    }
    _affects |= usage_flags::MEMORY; 
    _memory.write_if(!_has_exited, addr, val);
  }

  u8 pull() {
    u8 result = read(from_bytes(0x1, sp() + 1));
    sp(sp() + 1);
    cycles(cycles() + 2);
    if (ASSUME_NO_STACK_OVERFLOW) {
      assume(_sp != 0);
    }
    return result;
  }

  u8 push(u8 val) {
    u16 addr = from_bytes(0x1, sp());
    sp(sp() - 1);
    cycles(cycles() + 1);
    write(addr, val);
    if (ASSUME_NO_STACK_OVERFLOW) {
      assume(_sp != 0xFF);
    }
    return val;
  }

  u8 flags_to_byte() {
    return ite8(cc_s(), (u8)0x80, (u8)0x00)
         | ite8(cc_v(), (u8)0x40, (u8)0x00)
         | 0x20
         | ite8(cc_d(), (u8)0x08, (u8)0x00)
         | ite8(cc_i(), (u8)0x04, (u8)0x00)
         | ite8(cc_z(), (u8)0x02, (u8)0x00)
         | ite8(cc_c(), (u8)0x01, (u8)0x00);
  }

  void byte_to_flags(u8 status) {
    cc_s((status & 0x80) == 0x80);
    cc_v((status & 0x40) == 0x40);
    cc_d((status & 0x08) == 0x08);
    cc_i((status & 0x04) == 0x04);
    cc_z((status & 0x02) == 0x02);
    cc_c((status & 0x01) == 0x01);
  }

  void run(const instruction ins) {
    if (ins.name == instruction_name::NONE) {
      return;
    }

    bool is_constant = (uint8_t)(ins.payload_type & instruction_payload_type::CONST_FLAG);

    // The address the instruction is working on
    u16 absolute_var = 0;
    // the value the function is working on
    std::function<u8()> immediate_var = [this, &absolute_var]() { return read(absolute_var); };

    u16 branch_if_bit_address = 0;

    

    // Increment the program counter by the instruction size.
    pc(pc() + instruction_size(ins.payload_type));
    cycles(cycles() + 2);

    boolean branch_on_different_page = false;

    // Apply the addressing mode to the instruction payload
    switch(ins.payload_type) {
    case instruction_payload_type::NONE:
      // nothing to do
      break;
    case instruction_payload_type::RELATIVE:
    case instruction_payload_type::RELATIVE_CONST: {
      // for branches, the target is an 8 bit signed value relative to
      // the current pc.
      u8 relative_payload = _initial_state.relative(ins.payload, is_constant);
      absolute_var = ite16(
        relative_payload >= 0x80,
        pc() - 0x100 + u16(relative_payload),
        pc() + u16(relative_payload)
      );
      branch_on_different_page = hibyte(absolute_var) != hibyte(pc());
      break;
      }
    case instruction_payload_type::ABSOLUTE:
    case instruction_payload_type::ABSOLUTE_CONST:
      absolute_var = _initial_state.absolute(ins.payload, is_constant);
      cycles(cycles() + 2);
      break;
    case instruction_payload_type::RMW_ABSOLUTE:
    case instruction_payload_type::RMW_ABSOLUTE_CONST:
      absolute_var = _initial_state.absolute(ins.payload, is_constant);
      cycles(cycles() + 4);
      break;
    case instruction_payload_type::ABSOLUTE_X:
    case instruction_payload_type::ABSOLUTE_X_CONST: {
      u16 absolute_payload = _initial_state.absolute(ins.payload, is_constant);
      absolute_var = absolute_payload + u16(x());
      // add a cycle if crossing a page boundary.
      cycles(cycles() + 2 + u16(hibyte(absolute_payload) != hibyte(absolute_var)));
      break;
      }
    case instruction_payload_type::RMW_ABSOLUTE_X:
    case instruction_payload_type::RMW_ABSOLUTE_X_CONST: {
      u16 absolute_payload = _initial_state.absolute(ins.payload, is_constant);
      absolute_var = absolute_payload + u16(x());
      if (FASTER_RMW && ins.name != instruction_name::INC && ins.name != instruction_name::DEC) {
        cycles(cycles() + 4 + u16(hibyte(absolute_payload) != hibyte(absolute_var)));
      } else {
        cycles(cycles() + 5);
      }
      break;
      }
    case instruction_payload_type::ABSOLUTE_Y:
    case instruction_payload_type::ABSOLUTE_Y_CONST: {
      u16 absolute_payload = _initial_state.absolute(ins.payload, is_constant);
      absolute_var = absolute_payload + u16(y());
      // add a cycle if crossing a page boundary.
      cycles(cycles() + 2 + u16(hibyte(absolute_payload) != hibyte(absolute_var)));
      break;
      }
    case instruction_payload_type::X_INDIRECT:
    case instruction_payload_type::X_INDIRECT_CONST: {
      u8 zp_payload = _initial_state.zp(ins.payload, is_constant);
      // lda (addr, x). This wraps within the zero page.
      absolute_var = from_bytes(read(u16(zp_payload + x() + 1)),
                                read(u16(zp_payload + x())));
      cycles(cycles() + 4);
      break;
      }
    case instruction_payload_type::INDIRECT_Y:
    case instruction_payload_type::INDIRECT_Y_CONST: {
      u8 zp_payload = _initial_state.zp(ins.payload, is_constant);
      // lda (addr), y. 
      u16 absolute_payload = from_bytes(read(u16(zp_payload + 1)),
                                read(u16(zp_payload)));
      absolute_var = absolute_payload + u16(y());
      // add a cycle if crossing a page boundary.
      cycles(cycles() + 3 + u16(hibyte(absolute_payload) != hibyte(absolute_var)));
      break;
      }
    case instruction_payload_type::ZERO_PAGE:
    case instruction_payload_type::ZERO_PAGE_CONST:
      absolute_var = u16(_initial_state.zp(ins.payload, is_constant));
      cycles(cycles() + 1);
      break;
    case instruction_payload_type::RMW_ZERO_PAGE:
    case instruction_payload_type::RMW_ZERO_PAGE_CONST:
      absolute_var = u16(_initial_state.zp(ins.payload, is_constant));
      cycles(cycles() + 3);
      break;
    case instruction_payload_type::ZERO_PAGE_X:
    case instruction_payload_type::ZERO_PAGE_X_CONST:
      absolute_var = u16(_initial_state.zp(ins.payload, is_constant) + x());
      cycles(cycles() + 2);      
      break;
    case instruction_payload_type::RMW_ZERO_PAGE_X:
    case instruction_payload_type::RMW_ZERO_PAGE_X_CONST:
      absolute_var = u16(_initial_state.zp(ins.payload, is_constant) + x());
      cycles(cycles() + 4);      
      break;
    case instruction_payload_type::ZERO_PAGE_Y:
    case instruction_payload_type::ZERO_PAGE_Y_CONST:
      absolute_var = u16(_initial_state.zp(ins.payload, is_constant) + y());
      cycles(cycles() + 2);
      break;
    case instruction_payload_type::IMMEDIATE:
    case instruction_payload_type::IMMEDIATE_CONST:
      immediate_var = [this, &ins, &is_constant]() {
        return _initial_state.immediate(ins.payload, is_constant);
      };
      break;
    case instruction_payload_type::SPECIAL_IMMEDIATE:
      immediate_var = [this, &ins]() {
        return _initial_state.special(ins.payload);
      };
      break;
    case instruction_payload_type::INDIRECT_ABSOLUTE:
    case instruction_payload_type::INDIRECT_ABSOLUTE_CONST:
      {
      absolute_var = _initial_state.absolute(ins.payload, is_constant);
      // On the original 6502, jmp ($xxFF) will not properly
      // increment the high byte when getting the high byte of
      // the address.
      u16 hi = HAS_JUMP_INDIRECT_BUG
        ? (absolute_var & 0xFF00) | ((absolute_var + 1) & 0xFF)
        : absolute_var + 1;
      absolute_var = from_bytes(read(hi), read(absolute_var));
      // overestimate the length. The implementation of JMP will correct to 5 or 6 cycles total.
      cycles(cycles() + (HAS_JUMP_INDIRECT_BUG ? 4 : 5));
      break;
      }
    case instruction_payload_type::X_INDIRECT_ABSOLUTE:
    case instruction_payload_type::X_INDIRECT_ABSOLUTE_CONST: {
      // e.g. lda ($xxxx, x)
      absolute_var = _initial_state.absolute(ins.payload, is_constant) + u16(x());
      absolute_var = from_bytes(
        read(absolute_var + 1),
        read(absolute_var));
      cycles(cycles() + 4);
      break;
      }
    case instruction_payload_type::ZERO_PAGE_INDIRECT:
    case instruction_payload_type::ZERO_PAGE_INDIRECT_CONST: {
      u8 zp_payload = _initial_state.zp(ins.payload, is_constant);
      // e.g. lda ($xx)
      absolute_var = from_bytes(read(u16((zp_payload + 1) & 0xFF)),
                                read(u16(zp_payload)));
      cycles(cycles() + 3);
      break;
      }
    case instruction_payload_type::BRANCH_IF_BIT:
    case instruction_payload_type::BRANCH_IF_BIT_CONST: {
      u8 relative_payload = _initial_state.relative(ins.payload & 0xFF, is_constant);
      absolute_var = ite16(
        relative_payload >= 0x80,
        pc() - 0x100 + u16(relative_payload),
        pc() + u16(relative_payload)
      );
      branch_if_bit_address = u16(_initial_state.zp(ins.payload >> 8, is_constant));
      branch_on_different_page = hibyte(absolute_var) != hibyte(pc());
      cycles(cycles() + 3);
      break;
      }
    case instruction_payload_type::SOFTWARE_STACK:
    case instruction_payload_type::SOFTWARE_STACK_CONST:
      // e.g. lda stack + payload, x -- using x as a software stack pointer
      // at a zero page address.
      absolute_var = u16(_initial_state.software_stack(ins.payload, is_constant) + x());
      cycles(cycles() + 2);
      break;
    case instruction_payload_type::SOFTWARE_STACK_INDIRECT:
    case instruction_payload_type::SOFTWARE_STACK_INDIRECT_CONST: {
      // e.g. lda (stack + payload, x) -- using x as a software stack
      // pointer at a zero page address
      u8 addr = _initial_state.software_stack(ins.payload, is_constant);
      absolute_var = from_bytes(read(u16(addr + x() + 1)),
                                read(u16(addr + x())));
      cycles(cycles() + 4);
      break;
      }
    case instruction_payload_type::KNOWN_SUBROUTINE_CONST:
      // e.g. jsr subroutine2
      absolute_var = _initial_state.known_subroutine(ins.payload);
      cycles(cycles() + 2); // as if an absolute fetch
      break;
    case instruction_payload_type::ZERO_PAGE_TEMP:
    case instruction_payload_type::ZERO_PAGE_TEMP_CONST:
      // e.g. lda temp+offset0
      absolute_var = u16(_initial_state.temp(ins.payload, is_constant));
      if (ASSUME_VALID_TEMP_USAGE) {
        assume(absolute_var >= u16(_initial_state.temp()));
        assume(absolute_var < u16(_initial_state.temp() + TEMP_SIZE));
      }
      cycles(cycles() + 1);
      break;
    case instruction_payload_type::RMW_ZERO_PAGE_TEMP:
    case instruction_payload_type::RMW_ZERO_PAGE_TEMP_CONST:
      // e.g. inc temp+offset0
      absolute_var = u16(_initial_state.temp(ins.payload, is_constant));
      if (ASSUME_VALID_TEMP_USAGE) {
        assume(absolute_var >= u16(_initial_state.temp()));
        assume(absolute_var < u16(_initial_state.temp() + TEMP_SIZE));
      }
      cycles(cycles() + 3);
      break;
    case instruction_payload_type::ZERO_PAGE_X_TEMP:
    case instruction_payload_type::ZERO_PAGE_X_TEMP_CONST:
      absolute_var = u16(_initial_state.temp(ins.payload, is_constant) + x());
      if (ASSUME_VALID_TEMP_USAGE) {
        assume(absolute_var >= u16(_initial_state.temp()));
        assume(absolute_var < u16(_initial_state.temp() + TEMP_SIZE));
      }
      cycles(cycles() + 2);
      break;
    case instruction_payload_type::RMW_ZERO_PAGE_X_TEMP:
    case instruction_payload_type::RMW_ZERO_PAGE_X_TEMP_CONST:
      absolute_var = u16(_initial_state.temp(ins.payload, is_constant) + x());
      if (ASSUME_VALID_TEMP_USAGE) {
        assume(absolute_var >= u16(_initial_state.temp()));
        assume(absolute_var < u16(_initial_state.temp() + TEMP_SIZE));
      }
      cycles(cycles() + 4);
      break;
    case instruction_payload_type::ZERO_PAGE_Y_TEMP:
    case instruction_payload_type::ZERO_PAGE_Y_TEMP_CONST:
      absolute_var = u16(_initial_state.temp(ins.payload, is_constant) + y());
      if (ASSUME_VALID_TEMP_USAGE) {
        assume(absolute_var >= u16(_initial_state.temp()));
        assume(absolute_var < u16(_initial_state.temp() + TEMP_SIZE));
      }
      cycles(cycles() + 2);
      break;
    case instruction_payload_type::INDIRECT_Y_TEMP:
    case instruction_payload_type::INDIRECT_Y_TEMP_CONST: {
      // lda (temp+offset0), y.
      u16 lo = u16(_initial_state.temp(ins.payload, is_constant));
      u16 hi = u16(_initial_state.temp(ins.payload, is_constant) + 1);
      if (ASSUME_VALID_TEMP_USAGE) {
        assume(hi != 0);
        assume(lo >= u16(_initial_state.temp()));
        assume(hi < u16(_initial_state.temp() + TEMP_SIZE));
      }
      u16 absolute_payload = from_bytes(read(hi),
                                read(lo));
      absolute_var = absolute_payload + u16(y());
      cycles(cycles() + 3 + u16(hibyte(absolute_payload) != hibyte(absolute_var)));
      break;
      }
    case instruction_payload_type::X_INDIRECT_TEMP:
    case instruction_payload_type::X_INDIRECT_TEMP_CONST: {
      u16 lo = u16(_initial_state.temp(ins.payload, is_constant) + x());
      u16 hi = u16(_initial_state.temp(ins.payload, is_constant) + x() + 1);
      if (ASSUME_VALID_TEMP_USAGE) {
        assume(lo >= u16(_initial_state.temp()));
        assume(hi >= u16(_initial_state.temp()));
        assume(lo < u16(_initial_state.temp() + TEMP_SIZE));
        assume(hi < u16(_initial_state.temp() + TEMP_SIZE));
      }
      // lda (temp + offset0, x).
      absolute_var = from_bytes(read(hi),
                                read(lo));
      cycles(cycles() + 4);
      break;
      }
    case instruction_payload_type::CONST_FLAG:
      assert(false);
    }

    switch (ins.name) {
    case instruction_name::UNKNOWN: assert(false); break;

    // These instructions do nothing, or put the processor
    // in a state that it can't recover from on its own.
    case instruction_name::STP:
    case instruction_name::WAI:
    case instruction_name::NOP:
    case instruction_name::NONE: break;

    case instruction_name::AND: a(set_sz(a() & immediate_var())); break;
    case instruction_name::ORA: a(set_sz(a() | immediate_var())); break;
    case instruction_name::EOR: a(set_sz(a() ^ immediate_var())); break;
    case instruction_name::LDA: a(set_sz(immediate_var())); break;
    case instruction_name::LDX: x(set_sz(immediate_var())); break;
    case instruction_name::LDY: y(set_sz(immediate_var())); break;
    case instruction_name::TXA: a(set_sz(x())); break;
    case instruction_name::TAX: x(set_sz(a())); break;
    case instruction_name::TYA: a(set_sz(y())); break;
    case instruction_name::TAY: y(set_sz(a())); break;
    case instruction_name::INX: x(set_sz(x() + 1)); break;
    case instruction_name::INY: y(set_sz(y() + 1)); break;
    case instruction_name::DEX: x(set_sz(x() - 1)); break;
    case instruction_name::DEY: y(set_sz(y() - 1)); break;
    case instruction_name::CLC: cc_c(false); break;
    case instruction_name::CLI: cc_i(false); break;
    case instruction_name::CLV: cc_v(false); break;
    case instruction_name::CLD: cc_d(false); break;
    case instruction_name::SEC: cc_c(true); break;
    case instruction_name::SEI: cc_i(true); break;
    case instruction_name::SED: cc_d(true); break;
    case instruction_name::TSX: x(set_sz(sp())); break;
    case instruction_name::TXS: sp(x()); break;
    case instruction_name::INC:
      if (ins.payload_type == instruction_payload_type::NONE) {
        a(set_sz(a() + 1));
      } else {
        write(absolute_var, set_sz(immediate_var() + 1));
      }
      break;
    case instruction_name::DEC:
      if (ins.payload_type == instruction_payload_type::NONE) {
        a(set_sz(a() - 1));
      } else {
        write(absolute_var, set_sz(immediate_var() - 1));
      }
      break;
    case instruction_name::BIT:
      cc_z((immediate_var() & a()) == 0);
      if (ins.payload_type != instruction_payload_type::IMMEDIATE
        && ins.payload_type != instruction_payload_type::IMMEDIATE_CONST
        && ins.payload_type != instruction_payload_type::SPECIAL_IMMEDIATE) {
          cc_s((immediate_var() & 0x80) == 0x80);
          cc_v((immediate_var() & 0x40) == 0x40);
        }
      break;
    case instruction_name::ASL:
      if (ins.payload_type == instruction_payload_type::NONE) {
        cc_c((a() & 0x80) == 0x80);
        a(set_sz(a() << 1));
      } else {
        cc_c((immediate_var() & 0x80) == 0x80);
        write(absolute_var, set_sz(immediate_var() << 1));
      }
      break;
    case instruction_name::ROL:
      if (ins.payload_type == instruction_payload_type::NONE) {
        u8 val = (a() << 1) | (u8)cc_c();
        cc_c((a() & 0x80) == 0x80);
        a(set_sz(val));
      } else {
        u8 val = (immediate_var() << 1) | (u8)cc_c();
        cc_c((immediate_var() & 0x80) == 0x80);
        write(absolute_var, set_sz(val));
      }
      break;
    case instruction_name::LSR:
      if (ins.payload_type == instruction_payload_type::NONE) {
        cc_c((a() & 0x01) == 0x01);
        a(set_sz(a() >> 1));
      } else {
        cc_c((immediate_var() & 0x01) == 0x01);
        write(absolute_var, set_sz(immediate_var() >> 1));
      }
      break;
    case instruction_name::ROR:
      if (ins.payload_type == instruction_payload_type::NONE) {
        u8 val = (a() >> 1) | ite8(cc_c(), (u8)0x80, (u8)0x00);
        cc_c((a() & 0x01) == 0x01);
        a(set_sz(val));
      } else {
        u8 val = (immediate_var() >> 1) | ite8(cc_c(), (u8)0x80, (u8)0x00);
        cc_c((immediate_var() & 0x01) == 0x01);
        write(absolute_var, set_sz(val));
      }
      break;
    case instruction_name::STZ: write(absolute_var, 0); break;
    case instruction_name::STA: write(absolute_var, a()); break;
    case instruction_name::STX: write(absolute_var, x()); break;
    case instruction_name::STY: write(absolute_var, y()); break;
    case instruction_name::PLA: a(set_sz(pull())); break;
    case instruction_name::PHA: push(a()); break;
    case instruction_name::PLX: x(set_sz(pull())); break;
    case instruction_name::PHX: push(x()); break;
    case instruction_name::PLY: y(set_sz(pull())); break;
    case instruction_name::PHY: push(y()); break;
    case instruction_name::PHP: push(flags_to_byte() | 0x10); break;
    case instruction_name::PLP: byte_to_flags(pull()); break;
    case instruction_name::SBC: {
      if (DECIMAL_ENABLED) {
        if (ASSUME_VALID_BCD) {
          // Each nybble of both operands must be 0-9
          // for a valid BCD result.
          assume(
            !_cc_d || (
              (_a & 0xF) < 0xA
              && (_a & 0xF0) < 0xA0
              && (immediate_var() & 0xF) < 0xA
              && (immediate_var() & 0xF0) < 0xA0));
        }
      
        if (USE_65c02_DECIMAL) {
          // 65c02 decimal mode uses an extra cycle to
          // ensure the flags are more meaningful, and it
          // has different behavior for invalid decimal operands.

          u16 src = (u16)immediate_var();
          u16 bdiff = u16(a()) - src + u16(cc_c()) - 1;
          cc_v(((a() ^ lobyte(bdiff)) & (a() ^ immediate_var()) & 0x80) == 0x80);

          u16 ddiff = bdiff;
          ddiff = ddiff - ite16(ddiff > 0xFF, (u16)0x60, (u16)0);
          u16 tmp2 = u16(a() & 0xF) - (src & 0xF) + u16(cc_c()) - 1;
          ddiff = ddiff - ite16(tmp2 > 0xFF, (u16)6, (u16)0);

          u16 diff = ite16(cc_d(), ddiff, bdiff);
          cc_c(u16(a()) + u16(cc_c()) - 1 >= src);
          a(set_sz(lobyte(diff)));
          // fixing the flags requires an extra cycle if the decimal flag is set.
          cycles(cycles() + u16(cc_d()));
        } else {
            // Difference if decimal
            u16 ddiff = u16(a() & 0xf) - u16(immediate_var() & 0xf) - u16(!cc_c()); 
            ddiff = ite16(
              (ddiff & 0x10) == 0x10,
              ((ddiff - 6) & 0xF) | ((u16(a()) & 0xF0) - (u16(immediate_var()) & 0xF0) - 0x10),
              (ddiff & 0xF) | ((u16(a()) & 0xF0) - (u16(immediate_var()) & 0xF0))
            );

            ddiff = ddiff - ite16((ddiff & 0x100) == 0x100, u16(0x60), u16(0));                

            // Difference if binary
            u16 bdiff = u16(a()) - (u16)immediate_var() - u16(!cc_c());

            // TODO: this matches VICE, but are the flags really just based
            // on the binary difference?
            cc_c(bdiff < 0x100);
            set_sz(lobyte(bdiff));
            cc_v(((a() ^ lobyte(bdiff)) & (a() ^ immediate_var()) & 0x80) == 0x80);

            u16 diff = ite16(cc_d(), ddiff, bdiff);
            a(lobyte(diff));
        }
      } else {
        u16 diff = u16(a()) - (u16)immediate_var() - u16(!cc_c());
        cc_v(((lobyte(diff) ^ a()) & (a() ^ immediate_var()) & 0x80) == 0x80);
        cc_c(diff < 0x100);
        a(set_sz(lobyte(diff)));
      }

      break;
      }
    case instruction_name::ADC: {
      if (DECIMAL_ENABLED) {
        if (ASSUME_VALID_BCD) {
          // Each nybble of both operands must be 0-9
          // for a valid BCD result.
          assume(
            !_cc_d || (
              (_a & 0xF) < 0xA
              && (_a & 0xF0) < 0xA0
              && (immediate_var() & 0xF) < 0xA
              && (immediate_var() & 0xF0) < 0xA0));
        }

        if (USE_65c02_DECIMAL) {
          // 65c02 decimal mode uses an extra cycle to
          // ensure the flags are more meaningful, and it
          // has different behavior for invalid decimal operands.
          
          // Sum if decimal mode
          u16 dsumlo = u16((a() & 0xF) + (immediate_var() & 0xF) + (u8)cc_c());
          u16 dsumhi = u16((a() & 0xF0) + (immediate_var() & 0xF0));
          boolean t = dsumlo > 9;
          dsumlo = dsumlo + ite16(t, (u16)6, (u16)0);
          dsumhi = dsumhi + ite16(t, (u16)0x10, (u16)0);

          // Sum if binary mode
          u16 bsum = u16(immediate_var()) + u16(a()) + u16(cc_c());

          u16 sum = ite16(cc_d(), dsumhi, bsum);
          cc_v(((a() ^ lobyte(sum)) & (a() ^ immediate_var()) & 0x80) == 0x80);
          dsumhi = dsumhi + ite16(dsumhi > 0x90, (u16)0x60, (u16)0);
          
          sum = ite16(cc_d(), dsumhi, bsum);
          cc_c(sum > 0xFF);
          
          u16 dsum = (dsumlo & 0xF) + (dsumhi & 0xF0);
          sum = ite16(cc_d(), dsum, bsum);
          a(set_sz(lobyte(sum)));
          // fixing the flags requires an extra cycle if the decimal flag is set.
          cycles(cycles() + u16(cc_d()));
        } else {
          // Sum if decimal mode
          u16 dsum = u16((a() & 0xF) + (immediate_var() & 0xF) + (u8)cc_c());
          dsum = dsum + ite16(dsum > 0x9, (u16)6, (u16)0);
          dsum = (dsum & 0xF) + u16(a() & 0xF0) + u16(immediate_var() & 0xF0) + ite16(dsum > 0xF, (u16)0x10, (u16)0);

          // Sum if binary mode
          u16 bsum = u16(immediate_var()) + u16(a()) + u16(cc_c());

          u16 sum = ite16(cc_d(), dsum, bsum);

          cc_z(lobyte(bsum) == 0); // Zero flag is always based on binary sum
          cc_s((sum & 0x80) == 0x80);
          cc_v(((lobyte(sum) ^ a()) & (lobyte(sum) ^ immediate_var()) & 0x80) == 0x80);

          // Perform wrap around if greater than 99
          dsum = dsum + ite16((dsum & 0x1F0) > 0x90, (u16)0x60, (u16)0);

          sum = ite16(cc_d(), dsum, bsum);
          cc_c(sum > 0xFF);
          a(lobyte(sum));
        }
      } else {
        u16 carry = u16(cc_c());
        u16 sum = u16(immediate_var()) + u16(a()) + carry;

        cc_v(((lobyte(sum) ^ a()) & (lobyte(sum) ^ immediate_var()) & 0x80) == 0x80);
        cc_c(sum > 0xFF);
        a(set_sz(lobyte(sum)));
      }
      
      break;
      }
    case instruction_name::CMP:
      cc_c(a() >= immediate_var());
      cc_s(u8(a() - immediate_var()) >= 0x80);
      cc_z(a() == immediate_var());
      break;
    case instruction_name::CPX:
      cc_c(x() >= immediate_var());
      cc_s(u8(x() - immediate_var()) >= 0x80);
      cc_z(x() == immediate_var());
      break;
    case instruction_name::CPY:
      cc_c(y() >= immediate_var());
      cc_s(u8(y() - immediate_var()) >= 0x80);
      cc_z(y() == immediate_var());
      break;
    case instruction_name::JMP:
      // jmp is only 3 cycles, faster than expected for an absolute instruction.
      cycles(cycles() - 1);
      exit(absolute_var);
      break;
    case instruction_name::RTI: {
      byte_to_flags(pull());
      u8 lo = pull();
      u8 hi = pull();
      cycles(cycles() - 2); // each pull contributes 2 cycles, but we only want 6 total.
      exit(from_bytes(hi, lo));
      break;
      }
    case instruction_name::RTS: {
      u8 lo = pull();
      u8 hi = pull();
      exit(from_bytes(hi, lo) + 1);
      break;
      }
    case instruction_name::BRA: branch_if(true,    absolute_var, branch_on_different_page); break;
    case instruction_name::BPL: branch_if(!cc_s(), absolute_var, branch_on_different_page); break;
    case instruction_name::BMI: branch_if(cc_s(),  absolute_var, branch_on_different_page); break;
    case instruction_name::BVS: branch_if(cc_v(),  absolute_var, branch_on_different_page); break;
    case instruction_name::BVC: branch_if(!cc_v(), absolute_var, branch_on_different_page); break;
    case instruction_name::BCC: branch_if(!cc_c(), absolute_var, branch_on_different_page); break;
    case instruction_name::BCS: branch_if(cc_c(),  absolute_var, branch_on_different_page); break;
    case instruction_name::BEQ: branch_if(cc_z(),  absolute_var, branch_on_different_page); break;
    case instruction_name::BNE: branch_if(!cc_z(), absolute_var, branch_on_different_page); break;
    case instruction_name::JSR:
      push(hibyte(pc() - 1));
      push(lobyte(pc() - 1));
      // TODO: handle known subroutines
      exit(absolute_var);
      break;
    case instruction_name::BRK:
      push(hibyte(pc() + 1));
      push(lobyte(pc() + 1));
      push(flags_to_byte() | 0x10);
      cc_i(true);
      cycles(cycles() + 2);
      exit(from_bytes(read(0xFFFF), read(0xFFFE)));
      break;
    case instruction_name::LAX:
      a(set_sz(immediate_var()));
      x(a());
      break;
    case instruction_name::SAX: write(absolute_var, a() & x()); break;
    case instruction_name::DCM: {
      u8 immediate = immediate_var() - 1;
      write(absolute_var, set_sz(immediate));
      cc_c(a() >= immediate);
      cc_s(a() - immediate >= 0x80);
      cc_z(a() == immediate);
      break;
      }
    case instruction_name::INS: {
      u8 immediate = immediate_var();
      immediate = immediate + 1;
      write(absolute_var, set_sz(immediate));
      immediate = immediate ^ 0xFF;
      u16 sum = u16(immediate) + u16(a()) + u16(cc_c());
      cc_v(((lobyte(sum) ^ a()) & (lobyte(sum) ^ immediate) & 0x80) == 0x80);
      cc_c(hibyte(sum) == 0x01);
      a(set_sz(lobyte(sum)));
      break;
      }
    case instruction_name::SLO: {
      cc_c((immediate_var() & 0x80) == 0x80);
      u8 immediate = immediate_var() << 1;
      write(absolute_var, set_sz(immediate));
      a(set_sz(a() | immediate));
      break;
      }
    case instruction_name::RLA: {
      u8 val = (immediate_var() << 1) | ite8(cc_c(), (u8)1, (u8)0);
      cc_c((immediate_var() & 0x80) == 0x80);
      write(absolute_var, set_sz(val));
      a(set_sz(val & a()));
      break;
      }
    case instruction_name::SRE: {
      cc_c((immediate_var() & 0x01) == 0x01);
      u8 immediate = immediate_var() >> 1;
      write(absolute_var, immediate);
      a(set_sz(a() ^ immediate));
      break;
      }
    case instruction_name::RRA: {
      u8 val = (immediate_var() >> 1) | ite8(cc_c(), (u8)0x80, (u8)0x00);
      cc_c((immediate_var() & 0x01) == 0x01);
      write(absolute_var, set_sz(val));
      u16 sum = u16(val) + u16(a()) + ite16(cc_c(), (u16)0x01, (u16)0x00);
      cc_v(((lobyte(sum) ^ a()) & (lobyte(sum) ^ val) & 0x80) == 0x80);
      cc_c(hibyte(sum) == 0x01);
      a(set_sz(lobyte(sum)));
      break;
      }
    case instruction_name::ALR: {
      u8 val = (a() & immediate_var()) >> 1;
      cc_c((a() & immediate_var() & 0x01) == 0x01);
      a(set_sz(val));
      break;
      }
    case instruction_name::ANC:
      a(set_sz(a() & immediate_var()));
      cc_c(cc_s());
      break;
    case instruction_name::ARR: {
      a(set_sz(a() & immediate_var()));
      u8 val = (a() >> 1) | ite8(cc_c(), (u8)0x80, (u8)0x00);
      cc_c((val & 0x40) == 0x40);
      boolean bit6 = (val & 0x40) == 0x40;
      boolean bit5 = (val & 0x20) == 0x20;
      cc_v(bit6 ^ bit5);
      a(set_sz(val));
      break;
      }
    case instruction_name::AXS: {
      u8 xa = x() & a();
      x(set_sz(xa - immediate_var()));
      cc_c(xa >= immediate_var());
      break;
      }
    case instruction_name::RMB0:
    case instruction_name::RMB1:
    case instruction_name::RMB2:
    case instruction_name::RMB3:
    case instruction_name::RMB4:
    case instruction_name::RMB5:
    case instruction_name::RMB6:
    case instruction_name::RMB7: {
      uint8_t bit = (uint8_t)ins.name - (uint8_t)instruction_name::RMB0;
      uint8_t mask = (~(1 << bit)) & 0xFF;
      write(absolute_var, immediate_var() & mask);
      break;
      }
    case instruction_name::SMB0:
    case instruction_name::SMB1:
    case instruction_name::SMB2:
    case instruction_name::SMB3:
    case instruction_name::SMB4:
    case instruction_name::SMB5:
    case instruction_name::SMB6:
    case instruction_name::SMB7: {
      uint8_t bit = (uint8_t)ins.name - (uint8_t)instruction_name::RMB0;
      write(absolute_var, immediate_var() | (1 << bit));
      break;
      }
    case instruction_name::BBR0:
    case instruction_name::BBR1:
    case instruction_name::BBR2:
    case instruction_name::BBR3:
    case instruction_name::BBR4:
    case instruction_name::BBR5:
    case instruction_name::BBR6:
    case instruction_name::BBR7: {
      uint8_t bit = (uint8_t)ins.name - (uint8_t)instruction_name::BBR0;
      branch_if((read(branch_if_bit_address) & (1 << bit)) == 0, absolute_var, branch_on_different_page);
      break;
      }
    case instruction_name::BBS0:
    case instruction_name::BBS1:
    case instruction_name::BBS2:
    case instruction_name::BBS3:
    case instruction_name::BBS4:
    case instruction_name::BBS5:
    case instruction_name::BBS6:
    case instruction_name::BBS7: {
      uint8_t bit = (uint8_t)ins.name - (uint8_t)instruction_name::BBS0;
      branch_if((read(branch_if_bit_address) & (1 << bit)) != 0, absolute_var, branch_on_different_page);
      break;
      }
    case instruction_name::TRB: {
      cc_z((immediate_var() & a()) == 0);
      write(absolute_var, immediate_var() & ~a());
      break;
      }
    case instruction_name::TSB: {
      cc_z((immediate_var() & a()) == 0);
      write(absolute_var, immediate_var() | a());
      break;
      }
    }
  }
};

typedef machine<zuint8_t, zuint16_t, zbool, zmemory, z_initial_state> zmachine;
typedef machine<nuint8_t, nuint16_t, bool, hmemory, hash_initial_state> hmachine;
typedef machine<nuint8_t, nuint16_t, bool, cmemory, concrete_initial_state> cmachine;

uint16_t read16(const uint8_t memory[256*256], const uint16_t ip) {
  return memory[(ip + 1) & 0xFFFF] | (memory[(ip + 2) & 0xFFFF] << 8);
}

/**
 * Reads the byte after the IP as a signed relative branch argument,
 * and returns the new instruction pointer.
 */
uint16_t read_rel(const uint8_t memory[256*256], const uint16_t ip) {
  return memory[ip+1];
}

#define BYTE memory[(pc+1)&0xFFFF]
#define ABS  read16(memory, pc)
#define REL  read_rel(memory, pc)
#define DR(name, payload_type, payload) { static_assert(instruction_payload_type::payload_type == instruction_payload_type::NONE || static_cast<uint8_t>(instruction_payload_type::payload_type & instruction_payload_type::CONST_FLAG), #payload_type " must be CONST"); return instruction(instruction_name::name, instruction_payload_type::payload_type, payload); }

instruction decode(const uint8_t memory[256*256], const uint16_t pc) {
  switch (memory[pc]) {
    case 0x00: DR(BRK, NONE, 0);
    case 0x01: DR(ORA, X_INDIRECT_CONST, BYTE);
    case 0x05: DR(ORA, ZERO_PAGE_CONST, BYTE);
    case 0x06: DR(ASL, ZERO_PAGE_CONST, BYTE);
    case 0x08: DR(PHP, NONE, 0);
    case 0x09: DR(ORA, IMMEDIATE_CONST, BYTE);
    case 0x0A: DR(ASL, NONE, 0);
    case 0x0D: DR(ORA, ABSOLUTE_CONST, ABS);
    case 0x0E: DR(ASL, ABSOLUTE_CONST, ABS);
    case 0x10: DR(BPL, RELATIVE_CONST, REL);
    case 0x11: DR(ORA, INDIRECT_Y_CONST, BYTE);
    case 0x15: DR(ORA, ZERO_PAGE_X_CONST, BYTE);
    case 0x16: DR(ASL, ZERO_PAGE_X_CONST, BYTE);
    case 0x18: DR(CLC, NONE, 0);
    case 0x19: DR(ORA, ABSOLUTE_Y_CONST, ABS);
    case 0x1D: DR(ORA, ABSOLUTE_X_CONST, ABS);
    case 0x1E: DR(ASL, ABSOLUTE_X_CONST, ABS);
    case 0x20: DR(JSR, ABSOLUTE_CONST, ABS);
    case 0x21: DR(AND, X_INDIRECT_CONST, BYTE);
    case 0x24: DR(BIT, ZERO_PAGE_CONST, BYTE);
    case 0x25: DR(AND, ZERO_PAGE_CONST, BYTE);
    case 0x26: DR(ROL, ZERO_PAGE_CONST, BYTE);
    case 0x28: DR(PLP, NONE, 0);
    case 0x29: DR(AND, IMMEDIATE_CONST, BYTE);
    case 0x2A: DR(ROL, NONE, 0);
    case 0x2C: DR(BIT, ABSOLUTE_CONST, ABS);
    case 0x2D: DR(AND, ABSOLUTE_CONST, ABS);
    case 0x2E: DR(ROL, ABSOLUTE_CONST, ABS);
    case 0x30: DR(BMI, RELATIVE_CONST, REL);
    case 0x31: DR(AND, INDIRECT_Y_CONST, BYTE);
    case 0x35: DR(AND, ZERO_PAGE_X_CONST, BYTE);
    case 0x36: DR(ROL, ZERO_PAGE_X_CONST, BYTE);
    case 0x38: DR(SEC, NONE, 0);
    case 0x39: DR(AND, ABSOLUTE_Y_CONST, ABS);
    case 0x3D: DR(AND, ABSOLUTE_X_CONST, ABS);
    case 0x3E: DR(ROL, ABSOLUTE_X_CONST, ABS);
    case 0x40: DR(RTI, NONE, 0);
    case 0x41: DR(EOR, X_INDIRECT_CONST, BYTE);
    case 0x45: DR(EOR, ZERO_PAGE_CONST, BYTE);
    case 0x46: DR(LSR, ZERO_PAGE_CONST, BYTE);
    case 0x48: DR(PHA, NONE, 0);
    case 0x49: DR(EOR, IMMEDIATE_CONST, BYTE);
    case 0x4A: DR(LSR, NONE, 0);
    case 0x4C: DR(JMP, ABSOLUTE_CONST, ABS);
    case 0x4D: DR(EOR, ABSOLUTE_CONST, ABS);
    case 0x4E: DR(LSR, ABSOLUTE_CONST, ABS);
    case 0x50: DR(BVC, RELATIVE_CONST, REL);
    case 0x51: DR(EOR, INDIRECT_Y_CONST, BYTE);
    case 0x55: DR(EOR, ZERO_PAGE_X_CONST, BYTE);
    case 0x56: DR(LSR, ZERO_PAGE_X_CONST, BYTE);
    case 0x58: DR(CLI, NONE, 0);
    case 0x59: DR(EOR, ABSOLUTE_Y_CONST, ABS);
    case 0x5D: DR(EOR, ABSOLUTE_X_CONST, ABS);
    case 0x5E: DR(LSR, ABSOLUTE_X_CONST, ABS);
    case 0x60: DR(RTS, NONE, 0);
    case 0x61: DR(ADC, X_INDIRECT_CONST, BYTE);
    case 0x65: DR(ADC, ZERO_PAGE_CONST, BYTE);
    case 0x66: DR(ROR, ZERO_PAGE_CONST, BYTE);
    case 0x68: DR(PLA, NONE, 0);
    case 0x69: DR(ADC, IMMEDIATE_CONST, BYTE);
    case 0x6A: DR(ROR, NONE, 0);
    case 0x6C: DR(JMP, INDIRECT_ABSOLUTE_CONST, ABS);
    case 0x6D: DR(ADC, ABSOLUTE_CONST, ABS);
    case 0x6E: DR(ROR, ABSOLUTE_CONST, ABS);
    case 0x70: DR(BVS, RELATIVE_CONST, REL);
    case 0x71: DR(ADC, INDIRECT_Y_CONST, BYTE);
    case 0x75: DR(ADC, ZERO_PAGE_X_CONST, BYTE);
    case 0x76: DR(ROR, ZERO_PAGE_X_CONST, BYTE);
    case 0x78: DR(SEI, NONE, 0);
    case 0x79: DR(ADC, ABSOLUTE_Y_CONST, ABS);
    case 0x7D: DR(ADC, ABSOLUTE_X_CONST, ABS);
    case 0x7E: DR(ROR, ABSOLUTE_X_CONST, ABS);
    case 0x81: DR(STA, X_INDIRECT_CONST, BYTE);
    case 0x84: DR(STY, ZERO_PAGE_CONST, BYTE);
    case 0x85: DR(STA, ZERO_PAGE_CONST, BYTE);
    case 0x86: DR(STX, ZERO_PAGE_CONST, BYTE);
    case 0x88: DR(DEY, NONE, 0);
    case 0x8A: DR(TXA, NONE, 0);
    case 0x8C: DR(STY, ABSOLUTE_CONST, ABS);
    case 0x8D: DR(STA, ABSOLUTE_CONST, ABS);
    case 0x8E: DR(STX, ABSOLUTE_CONST, ABS);
    case 0x90: DR(BCC, RELATIVE_CONST, REL);
    case 0x91: DR(STA, INDIRECT_Y_CONST, BYTE);
    case 0x94: DR(STY, ZERO_PAGE_X_CONST, BYTE);
    case 0x95: DR(STA, ZERO_PAGE_X_CONST, BYTE);
    case 0x96: DR(STX, ZERO_PAGE_Y_CONST, BYTE);
    case 0x98: DR(TYA, NONE, 0);
    case 0x99: DR(STA, ABSOLUTE_Y_CONST, ABS);
    case 0x9A: DR(TXS, NONE, 0);
    case 0x9D: DR(STA, ABSOLUTE_X_CONST, ABS);
    case 0xA0: DR(LDY, IMMEDIATE_CONST, BYTE);
    case 0xA1: DR(LDA, X_INDIRECT_CONST, BYTE);
    case 0xA2: DR(LDX, IMMEDIATE_CONST, BYTE);
    case 0xA4: DR(LDY, ZERO_PAGE_CONST, BYTE);
    case 0xA5: DR(LDA, ZERO_PAGE_CONST, BYTE);
    case 0xA6: DR(LDX, ZERO_PAGE_CONST, BYTE);
    case 0xA8: DR(TAY, NONE, 0);
    case 0xA9: DR(LDA, IMMEDIATE_CONST, BYTE);
    case 0xAA: DR(TAX, NONE, 0);
    case 0xAC: DR(LDY, ABSOLUTE_CONST, ABS);
    case 0xAD: DR(LDA, ABSOLUTE_CONST, ABS);
    case 0xAE: DR(LDX, ABSOLUTE_CONST, ABS);
    case 0xB0: DR(BCS, RELATIVE_CONST, REL);
    case 0xB1: DR(LDA, INDIRECT_Y_CONST, BYTE);
    case 0xB4: DR(LDY, ZERO_PAGE_X_CONST, BYTE);
    case 0xB5: DR(LDA, ZERO_PAGE_X_CONST, BYTE);
    case 0xB6: DR(LDX, ZERO_PAGE_Y_CONST, BYTE);
    case 0xB8: DR(CLV, NONE, 0);
    case 0xB9: DR(LDA, ABSOLUTE_Y_CONST, ABS);
    case 0xBA: DR(TSX, NONE, 0);
    case 0xBC: DR(LDY, ABSOLUTE_X_CONST, ABS);
    case 0xBD: DR(LDA, ABSOLUTE_X_CONST, ABS);
    case 0xBE: DR(LDX, ABSOLUTE_Y_CONST, ABS);
    case 0xC0: DR(CPY, IMMEDIATE_CONST, BYTE);
    case 0xC1: DR(CMP, X_INDIRECT_CONST, BYTE);
    case 0xC4: DR(CPY, ZERO_PAGE_CONST, BYTE);
    case 0xC5: DR(CMP, ZERO_PAGE_CONST, BYTE);
    case 0xC6: DR(DEC, ZERO_PAGE_CONST, BYTE);
    case 0xC8: DR(INY, NONE, 0);
    case 0xC9: DR(CMP, IMMEDIATE_CONST, BYTE);
    case 0xCA: DR(DEX, NONE, 0);
    case 0xCC: DR(CPY, ABSOLUTE_CONST, ABS);
    case 0xCD: DR(CMP, ABSOLUTE_CONST, ABS);
    case 0xCE: DR(DEC, ABSOLUTE_CONST, ABS);
    case 0xD0: DR(BNE, RELATIVE_CONST, REL);
    case 0xD1: DR(CMP, INDIRECT_Y_CONST, BYTE);
    case 0xD5: DR(CMP, ZERO_PAGE_X_CONST, BYTE);
    case 0xD6: DR(DEC, ZERO_PAGE_X_CONST, BYTE);
    case 0xD8: DR(CLD, NONE, 0);
    case 0xD9: DR(CMP, ABSOLUTE_Y_CONST, ABS);
    case 0xDD: DR(CMP, ABSOLUTE_X_CONST, ABS);
    case 0xDE: DR(DEC, ABSOLUTE_X_CONST, ABS);
    case 0xE0: DR(CPX, IMMEDIATE_CONST, BYTE);
    case 0xE1: DR(SBC, X_INDIRECT_CONST, BYTE);
    case 0xE4: DR(CPX, ZERO_PAGE_CONST, BYTE);
    case 0xE5: DR(SBC, ZERO_PAGE_CONST, BYTE);
    case 0xE6: DR(INC, ZERO_PAGE_CONST, BYTE);
    case 0xE8: DR(INX, NONE, 0);
    case 0xE9: DR(SBC, IMMEDIATE_CONST, BYTE);
    case 0xEA: DR(NOP, NONE, 0);
    case 0xEC: DR(CPX, ABSOLUTE_CONST, ABS);
    case 0xED: DR(SBC, ABSOLUTE_CONST, ABS);
    case 0xEE: DR(INC, ABSOLUTE_CONST, ABS);
    case 0xF0: DR(BEQ, RELATIVE_CONST, REL);
    case 0xF1: DR(SBC, INDIRECT_Y_CONST, BYTE);
    case 0xF5: DR(SBC, ZERO_PAGE_X_CONST, BYTE);
    case 0xF6: DR(INC, ZERO_PAGE_X_CONST, BYTE);
    case 0xF8: DR(SED, NONE, 0);
    case 0xF9: DR(SBC, ABSOLUTE_Y_CONST, ABS);
    case 0xFD: DR(SBC, ABSOLUTE_X_CONST, ABS);
    case 0xFE: DR(INC, ABSOLUTE_X_CONST, ABS);

    // Undocumented NOPs
    case 0x1A:
    case 0x3A:
    case 0x5A:
    case 0x7A:
    case 0xDA:
    case 0xFA: DR(NOP, NONE, 0);

    // Undocumented 2-byte NOPS
    case 0x04:
    case 0x14:
    case 0x34:
    case 0x44:
    case 0x54:
    case 0x64:
    case 0x74:
    case 0x80:
    case 0x82:
    case 0x89:
    case 0xC2:
    case 0xD4:
    case 0xE2:
    case 0xF4: DR(NOP, IMMEDIATE_CONST, BYTE);

    // Undocumented 3 byte NOPs
    case 0x0C: DR(NOP, ABSOLUTE_CONST, ABS);
    case 0x1C: DR(NOP, ABSOLUTE_X_CONST, ABS);
    case 0x3C: DR(NOP, ABSOLUTE_X_CONST, ABS);
    case 0x5C: DR(NOP, ABSOLUTE_X_CONST, ABS);
    case 0x7C: DR(NOP, ABSOLUTE_X_CONST, ABS);
    case 0xDC: DR(NOP, ABSOLUTE_X_CONST, ABS);
    case 0xFC: DR(NOP, ABSOLUTE_X_CONST, ABS);

    // Undocumented Load A and X
    case 0xAB: DR(LAX, IMMEDIATE_CONST, BYTE);
    case 0xAF: DR(LAX, ABSOLUTE_CONST, ABS);
    case 0xBF: DR(LAX, ABSOLUTE_Y_CONST, ABS);
    case 0xA7: DR(LAX, ZERO_PAGE_CONST, BYTE);
    case 0xB7: DR(LAX, ZERO_PAGE_Y_CONST, BYTE);
    case 0xA3: DR(LAX, X_INDIRECT_CONST, BYTE);
    case 0xB3: DR(LAX, INDIRECT_Y_CONST, BYTE);

    // Undocumented Store (A & X)
    case 0x8F: DR(SAX, ABSOLUTE_CONST, ABS);
    case 0x87: DR(SAX, ZERO_PAGE_CONST, BYTE);
    case 0x97: DR(SAX, ZERO_PAGE_Y_CONST, BYTE);
    case 0x83: DR(SAX, X_INDIRECT_CONST, BYTE);

    // Undocumented alternate version of SBC
    case 0xEB: DR(SBC, IMMEDIATE_CONST, BYTE);

    // Undocumented Decrement and Compare
    case 0xCF: DR(DCM, ABSOLUTE_CONST, ABS);
    case 0xDF: DR(DCM, ABSOLUTE_X_CONST, ABS);
    case 0xDB: DR(DCM, ABSOLUTE_Y_CONST, ABS);
    case 0xC7: DR(DCM, ZERO_PAGE_CONST, BYTE);
    case 0xD7: DR(DCM, ZERO_PAGE_X_CONST, BYTE);
    case 0xC3: DR(DCM, X_INDIRECT_CONST, BYTE);
    case 0xD3: DR(DCM, INDIRECT_Y_CONST, BYTE);

    // Undocumented Increment and Subtract
    case 0xEF: DR(INS, ABSOLUTE_CONST, ABS);
    case 0xFF: DR(INS, ABSOLUTE_X_CONST, ABS);
    case 0xFB: DR(INS, ABSOLUTE_Y_CONST, ABS);
    case 0xE7: DR(INS, ZERO_PAGE_CONST, BYTE);
    case 0xF7: DR(INS, ZERO_PAGE_X_CONST, BYTE);
    case 0xE3: DR(INS, X_INDIRECT_CONST, BYTE);
    case 0xF3: DR(INS, INDIRECT_Y_CONST, BYTE);

    // Undocumented ASL and ORA instruction
    case 0x0F: DR(SLO, ABSOLUTE_CONST, ABS);
    case 0x1F: DR(SLO, ABSOLUTE_X_CONST, ABS);
    case 0x1B: DR(SLO, ABSOLUTE_Y_CONST, ABS);
    case 0x07: DR(SLO, ZERO_PAGE_CONST, BYTE);
    case 0x17: DR(SLO, ZERO_PAGE_X_CONST, BYTE);
    case 0x03: DR(SLO, X_INDIRECT_CONST, BYTE);
    case 0x13: DR(SLO, INDIRECT_Y_CONST, BYTE);

    // Undocumented ROL and AND instruction
    case 0x2F: DR(RLA, ABSOLUTE_CONST, ABS);
    case 0x3F: DR(RLA, ABSOLUTE_X_CONST, ABS);
    case 0x3B: DR(RLA, ABSOLUTE_Y_CONST, ABS);
    case 0x27: DR(RLA, ZERO_PAGE_CONST, BYTE);
    case 0x37: DR(RLA, ZERO_PAGE_X_CONST, BYTE);
    case 0x23: DR(RLA, X_INDIRECT_CONST, BYTE);
    case 0x33: DR(RLA, INDIRECT_Y_CONST, BYTE);

    // Undocumented LSR and EOR instruction
    case 0x4F: DR(SRE, ABSOLUTE_CONST, ABS);
    case 0x5F: DR(SRE, ABSOLUTE_X_CONST, ABS);
    case 0x5B: DR(SRE, ABSOLUTE_Y_CONST, ABS);
    case 0x47: DR(SRE, ZERO_PAGE_CONST, BYTE);
    case 0x57: DR(SRE, ZERO_PAGE_X_CONST, BYTE);
    case 0x43: DR(SRE, X_INDIRECT_CONST, BYTE);
    case 0x53: DR(SRE, INDIRECT_Y_CONST, BYTE);

    // Undocumented ROR and ADC
    case 0x6F: DR(RRA, ABSOLUTE_CONST, ABS);
    case 0x7F: DR(RRA, ABSOLUTE_X_CONST, ABS);
    case 0x7B: DR(RRA, ABSOLUTE_Y_CONST, ABS);
    case 0x67: DR(RRA, ZERO_PAGE_CONST, BYTE);
    case 0x77: DR(RRA, ZERO_PAGE_X_CONST, BYTE);
    case 0x63: DR(RRA, X_INDIRECT_CONST, BYTE);
    case 0x73: DR(RRA, INDIRECT_Y_CONST, BYTE);

    case 0x0B: DR(ANC, IMMEDIATE_CONST, BYTE);
    case 0x2B: DR(ANC, IMMEDIATE_CONST, BYTE);
    case 0x4B: DR(ALR, IMMEDIATE_CONST, BYTE);
    case 0x6B: DR(ARR, IMMEDIATE_CONST, BYTE);
    case 0xCB: DR(AXS, IMMEDIATE_CONST, BYTE);

    default:
      printf("Unknown instruction: %02X @ %04X\n", memory[pc], pc);
      DR(UNKNOWN, NONE, 0);
  }
}

#undef BYTE
#undef ABS
#undef REL
#undef DR

int compare_abstract_and_concrete(char *rom) {
  try {
    char header[0x10];
    char *prg;
    char *chr;
    uint8_t memory[0x10000];

    concrete_initial_state c_state(memory);
    cmachine c_machine(c_state);

    std::ifstream input(rom, std::ios::in | std::ios::binary);
    printf("Loading file %s\n", rom);
    input.read(header, sizeof header);
    int prg_size = 0x4000 * header[4];
    prg = new char[prg_size];
    int chr_size = 0x4000 * header[5];

    printf("iNES info: %02X %02X %02X %02X PRG:%02X CHR:%02X\n", header[0], header[1], header[2], header[3], header[4], header[5]);
    chr = new char[chr_size];
    input.read(prg, prg_size);
    input.read(chr, chr_size);

    for (uint16_t i = 0; i < prg_size && i < 0x8000; i++) {
      memory[0x8000 + i] = prg[i];
      if (prg_size == 0x4000) {
        memory[0xC000 + i] = prg[i];
      }
    }

    c_machine.pc(0xC000);
    c_machine.byte_to_flags(0x24);
    c_machine.sp(0xFD);
    printf("pc:%04X\n", c_machine.pc().val);

    std::string status;

    uint64_t instructions = 0;
    while (instructions < 0x10000 || memory[0x6000] == 0x80) {
      instruction ins = decode(memory, c_machine.pc().val);

      instructions++;
      
      printf("%04X  ", c_machine.pc().val);

      int i = 0, size = instruction_size(ins.payload_type);
      for (; i < size; i++) {
        printf("%02X ", memory[(c_machine.pc() + i).val]);
      }
      for (; i < 3; i++) {
        printf("   ");
      }

      printf(" A:%02X X:%02X Y:%02X P:%02X SP:%02X\n",
        c_machine.a().val,
        c_machine.x().val,
        c_machine.y().val,
        c_machine.flags_to_byte().val,
        c_machine.sp().val);

      if (memory[0x6004] != 0) {
        std::string new_status((char *)(memory + 0x6004));
        if (new_status != status) {
          status = new_status;
          std::cout << new_status;
        }
      }

      {
        c_machine.reset_exit();
        // Create an abstract machine copying the current state
        z_initial_state z_state;
        zmachine z_machine(z_state);
        z_machine.copy_from(c_machine);

        // Save the original memory state
        z3::expr original_memory = z_machine._memory.val;
        // Run the instruction on both machines
        c_machine.run(ins);
        z_machine.run(ins);
        z3::solver solver(z3_ctx);
        // Add assertions for the memory locations which were read.
        for (int i = 0; i < c_machine._memory.n_read; i++) {
          z3::expr equal = z3::select(original_memory, c_machine._memory.addresses_read[i].val) == c_machine._memory.values_read[i].val;
          add_assertion(solver, equal);
        }

        // And for the memory locations which were written.
        for (int i = 0; i < c_machine._memory.n_written; i++) {
          add_assertion(solver, z_machine.read(c_machine._memory.addresses_written[i].val) == c_machine._memory.values_written[i].val);
        }

        c_machine._memory.n_read = 0;
        c_machine._memory.n_written = 0;

        add_assertion(solver, z_machine.a() == c_machine.a().val);
        add_assertion(solver, z_machine.x() == c_machine.x().val);
        add_assertion(solver, z_machine.y() == c_machine.y().val);
        add_assertion(solver, z_machine.sp() == c_machine.sp().val);
        add_assertion(solver, z_machine.cc_s() == c_machine.cc_s());
        add_assertion(solver, z_machine.cc_v() == c_machine.cc_v());
        add_assertion(solver, z_machine.cc_i() == c_machine.cc_i());
        add_assertion(solver, z_machine.cc_d() == c_machine.cc_d());
        add_assertion(solver, z_machine.cc_c() == c_machine.cc_c());
        add_assertion(solver, z_machine.cc_z() == c_machine.cc_z());
        add_assertion(solver, z_machine.pc() == c_machine.pc().val);
        add_assertion(solver, z_machine._has_exited == c_machine._has_exited);

        z3::check_result result = solver.check();
        if (result == z3::unsat) {
          std::cout << solver << std::endl;
          return 1;
        }
      }
    }

    exit(memory[0x6000]);

  } catch(z3::exception& e) {
    std::cout << e << std::endl;
  }

  return 0;
}

int initialize() {

  return 0;
}

int enumerate() {

  return 0;
}

enum struct compare_result_type : uint8_t {
  cannot_satify = 1,
  matching = 2,
  different = 3,
  unknown = 4
};

compare_result_type compare_from_check_result(z3::check_result result) {
  if (result == z3::unsat) {
    return compare_result_type::matching;
  } else if (result == z3::sat) {
    return compare_result_type::different;
  } else {
    return compare_result_type::unknown;
  }
}

struct compare_result {
  compare_result(compare_result_type _type) : type(_type) {}
  compare_result(compare_result_type cc_s, compare_result_type cc_v, compare_result_type cc_i, compare_result_type cc_d, compare_result_type cc_c, compare_result_type cc_z)
    : type(compare_result_type::matching), cc_s_matches(cc_s), cc_v_matches(cc_v),
      cc_i_matches(cc_i), cc_d_matches(cc_d), cc_c_matches(cc_c), cc_z_matches(cc_z) {}

  compare_result_type type;
  compare_result_type cc_s_matches;
  compare_result_type cc_v_matches;
  compare_result_type cc_i_matches;
  compare_result_type cc_d_matches;
  compare_result_type cc_c_matches;
  compare_result_type cc_z_matches;
};

inline std::ostream& operator<<(std::ostream& os, const compare_result& result)
{
  switch (result.type) {
    case compare_result_type::cannot_satify:
      os << "cannot_satisy"; return os;
    case compare_result_type::different:
      os << "different"; return os;
    case compare_result_type::matching:
      os << "matching("
      << (result.cc_s_matches == compare_result_type::matching ? "s" : "S")
      << (result.cc_v_matches == compare_result_type::matching ? "v" : "V")
      << (result.cc_i_matches == compare_result_type::matching ? "i" : "I")
      << (result.cc_d_matches == compare_result_type::matching ? "d" : "D")
      << (result.cc_c_matches == compare_result_type::matching ? "c" : "C")
      << (result.cc_z_matches == compare_result_type::matching ? "z" : "Z")
      << ")";
      return os;
    case compare_result_type::unknown:
      os << "unknown"; return os;
  }
  return os;
}

/**
 * @brief Checks if the two zmachines have the same machine state.
 * 
 * @param a 
 * @param b 
 * @return z3::unsat if the results are the same
 *   z3::sat if they are different, or z3::unknown
 */
compare_result compare(zmachine& a, zmachine& b) {
  z3::solver solver(z3_ctx);
  add_assertion(solver, a._assumptions);
  add_assertion(solver, b._assumptions);
  z3::check_result assumptions_check = solver.check();
  if (assumptions_check == z3::unsat) {
    return compare_result_type::cannot_satify;
  } else if (assumptions_check == z3::unknown) {
    return compare_result_type::unknown;
  }
  solver.push();
  zbool assertions = false;
  assertions = assertions || (a.a().val.simplify()    != b.a().val.simplify());
  assertions = assertions || (a.x().val.simplify()    != b.x().val.simplify());
  assertions = assertions || (a.y().val.simplify()    != b.y().val.simplify());
  assertions = assertions || (a.sp().val.simplify()   != b.sp().val.simplify());

  zuint16_t addr = z3_ctx.bv_const("addr", 16);
  zbool special_memory = false;
  if (ASSUME_VALID_STACK_USAGE) {
    // Any memory in the stack page that isn't on the stack is irrelevant.
    special_memory = special_memory || ((hibyte(addr) == 0x01) && (lobyte(addr) <= a.sp()));
  }
  if (ASSUME_VALID_TEMP_USAGE) {
    // Any memory in the temp region is irrelevant.
    special_memory = special_memory || (hibyte(addr) == 0 && lobyte(addr) >= a._initial_state.temp() && lobyte(addr) < a._initial_state.temp() + TEMP_SIZE);
  }

  // Compare the two memories, but treat the memory in special areas
  // as always 0.
  z3::expr a_memory = z3::ite(special_memory.val, z3_ctx.bv_val(0, 8), a._memory.read(addr).val).simplify();
  z3::expr b_memory = z3::ite(special_memory.val, z3_ctx.bv_val(0, 8), b._memory.read(addr).val).simplify();
  assertions = assertions || (a_memory != b_memory);

  assertions = assertions || (a._has_exited.val.simplify() != b._has_exited.val.simplify());
  assertions = assertions || (z3::ite(a._has_exited.val, a.pc().val, z3_ctx.bv_val(0, 16)).simplify() != z3::ite(b._has_exited.val, b.pc().val, z3_ctx.bv_val(0, 16)).simplify());

  add_assertion(solver, assertions);

  z3::check_result result = solver.check();
  if (result == z3::sat) {
    std::cout << assertions.val << std::endl;
    std::cout << solver.get_model() << std::endl;
  }
  if (result == z3::sat) {
    return compare_result_type::different;
  } else if (result == z3::unsat) {
    solver.pop();
    solver.push();
    solver.add(a.cc_s().val.simplify() != b.cc_s().val.simplify());
    compare_result_type cc_s = compare_from_check_result(solver.check());
    solver.pop();
    solver.push();
    solver.add(a.cc_v().val.simplify() != b.cc_v().val.simplify());
    compare_result_type cc_v = compare_from_check_result(solver.check());
    solver.pop();
    solver.push();
    solver.add(a.cc_i().val.simplify() != b.cc_i().val.simplify());
    compare_result_type cc_i = compare_from_check_result(solver.check());
    solver.pop();
    solver.push();
    solver.add(a.cc_d().val.simplify() != b.cc_d().val.simplify());
    compare_result_type cc_d = compare_from_check_result(solver.check());
    solver.pop();
    solver.push();
    solver.add(a.cc_c().val.simplify() != b.cc_c().val.simplify());
    compare_result_type cc_c = compare_from_check_result(solver.check());
    solver.pop();
    solver.push();
    solver.add(a.cc_z().val.simplify() != b.cc_z().val.simplify());
    compare_result_type cc_z = compare_from_check_result(solver.check());
    return compare_result(cc_s, cc_v, cc_i, cc_d, cc_c, cc_z);
  } else {
    return compare_result_type::unknown;
  }
}

int nes_test(char *rom) {
  try {
    char header[0x10];
    char *prg;
    char *chr;
    uint8_t memory[0x10000];

    concrete_initial_state c_state(memory);
    c_state._memory.record_memory = false;
    cmachine c_machine(c_state);

    std::ifstream input(rom, std::ios::in | std::ios::binary);
    printf("Loading file %s\n", rom);
    input.read(header, sizeof header);
    int prg_size = 0x4000 * header[4];
    prg = new char[prg_size];
    int chr_size = 0x4000 * header[5];

    printf("iNES info: %02X %02X %02X %02X PRG:%02X CHR:%02X\n", header[0], header[1], header[2], header[3], header[4], header[5]);
    chr = new char[chr_size];
    input.read(prg, prg_size);
    input.read(chr, chr_size);

    for (uint16_t i = 0; i < prg_size && i < 0x8000; i++) {
      memory[0x8000 + i] = prg[i];
      if (prg_size == 0x4000) {
        memory[0xC000 + i] = prg[i];
      }
    }

    c_machine.pc(0xC000);
    c_machine.byte_to_flags(0x24);
    c_machine.sp(0xFD);
    printf("pc:%04X\n", c_machine.pc().val);

    std::string status;

    uint64_t instructions = 0;
    while (c_machine.pc() != 1) {
      instruction ins = decode(memory, c_machine.pc().val);

      instructions++;

      printf("%04X  ", c_machine.pc().val);
      int i = 0, size = instruction_size(ins.payload_type);
      for (; i < size; i++) {
        printf("%02X ", memory[(c_machine.pc() + i).val]);
      }
      for (; i < 3; i++) {
        printf("   ");
      }
      printf(" A:%02X X:%02X Y:%02X P:%02X SP:%02X\n",
        c_machine.a().val,
        c_machine.x().val,
        c_machine.y().val,
        c_machine.flags_to_byte().val,
        c_machine.sp().val);

      c_machine.reset_exit();
      c_machine.run(ins);
    }

  } catch(z3::exception& e) {
    std::cout << e << std::endl;
  }
  return 0;
}

instruction comparisons[] = {
  // COMPARE:
  // LDA #immediate1
  // AND #immediate2
  //
  // LDA #(immediate1 & immediate2)
  instruction(instruction_name::LDA, instruction_payload_type::IMMEDIATE, 0),
  instruction(instruction_name::AND, instruction_payload_type::IMMEDIATE, 1),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  instruction(instruction_name::LDA, instruction_payload_type::SPECIAL_IMMEDIATE, special_immediate{
    .operation = special_immediate_operations::AND,
    .operand1 = special_immediate_operands::IMMEDIATE,
    .payload1 = 0,
    .operand2 = special_immediate_operands::IMMEDIATE,
    .payload2 = 1
  }.to_int()),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  // COMPARE
  // lda #0
  // jmp absolute0
  //
  // lda #>(absolute0-1)
  // pha
  // lda #<(absolute0-1)
  // pha
  // lda #0
  // rts
  instruction(instruction_name::LDA, instruction_payload_type::IMMEDIATE_CONST, 0),
  instruction(instruction_name::JMP, instruction_payload_type::ABSOLUTE, 0),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  instruction(instruction_name::LDA, instruction_payload_type::SPECIAL_IMMEDIATE, special_immediate {
    .operation = special_immediate_operations::MINUS_HI,
    .operand1 = special_immediate_operands::ABSOLUTE,
    .payload1 = 0,
    .operand2 = special_immediate_operands::CONSTANT,
    .payload2 = special_immediate_constants::x1
  }.to_int()),
  instruction(instruction_name::PHA, instruction_payload_type::NONE, 0),
  instruction(instruction_name::LDA, instruction_payload_type::SPECIAL_IMMEDIATE, special_immediate {
    .operation = special_immediate_operations::MINUS,
    .operand1 = special_immediate_operands::ABSOLUTE,
    .payload1 = 0,
    .operand2 = special_immediate_operands::CONSTANT,
    .payload2 = special_immediate_constants::x1
  }.to_int()),
  instruction(instruction_name::PHA, instruction_payload_type::NONE, 0),
  instruction(instruction_name::LDA, instruction_payload_type::IMMEDIATE_CONST, 0),
  instruction(instruction_name::RTS, instruction_payload_type::NONE, 0),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  // COMPARE
  // clc
  //
  // nop
  //
  // This tests the detection of equivalence except flags
  instruction(instruction_name::CLC, instruction_payload_type::NONE, 0),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  instruction(instruction_name::NOP, instruction_payload_type::NONE, 0),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  // COMPARE
  // sta TEMP + offset0
  // inc TEMP + offset0
  // lda TEMP + offset0
  //
  // inc a
  // This tests equivalence ignoring TEMP stores.
  instruction(instruction_name::STA, instruction_payload_type::ZERO_PAGE_TEMP, 0),
  instruction(instruction_name::INC, instruction_payload_type::RMW_ZERO_PAGE_TEMP, 0),
  instruction(instruction_name::LDA, instruction_payload_type::ZERO_PAGE_TEMP, 0),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  instruction(instruction_name::INC, instruction_payload_type::NONE, 0),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  // COMPARE
  // bit zp0
  // php
  // pha
  // eor #$FF
  // and zp0
  // sta zp0
  // pla
  // plp
  //
  // trb zp0
  // This should be equivalent except for the S and V flags.
  instruction(instruction_name::BIT, instruction_payload_type::ZERO_PAGE, 0),
  instruction(instruction_name::PHP, instruction_payload_type::NONE, 0),
  instruction(instruction_name::PHA, instruction_payload_type::NONE, 0),
  instruction(instruction_name::EOR, instruction_payload_type::IMMEDIATE_CONST, 0xFF),
  instruction(instruction_name::AND, instruction_payload_type::ZERO_PAGE, 0),
  instruction(instruction_name::STA, instruction_payload_type::ZERO_PAGE, 0),
  instruction(instruction_name::PLA, instruction_payload_type::NONE, 0),
  instruction(instruction_name::PLP, instruction_payload_type::NONE, 0),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  instruction(instruction_name::TRB, instruction_payload_type::ZERO_PAGE, 0),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  // COMPARE
  // clv
  // bvc relative0
  //
  // bra relative0
  // This is equivalent, other than the overflow flag.
  // However, this isn't detected now, since it assumes the relative0
  // is a fixed offset from the current instruction, and the instructions
  // are at different locations. TODO, fix the handling of relative branches.
  // Adding a NOP to make this work for now.
  instruction(instruction_name::CLV, instruction_payload_type::NONE, 0),
  instruction(instruction_name::BVC, instruction_payload_type::RELATIVE, 0),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  instruction(instruction_name::NOP, instruction_payload_type::NONE, 0),
  instruction(instruction_name::BRA, instruction_payload_type::RELATIVE, 0),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  // COMPARE
  // lda zp0
  // ldx zp0
  //
  // lax zp0
  instruction(instruction_name::LDA, instruction_payload_type::ZERO_PAGE, 0),
  instruction(instruction_name::LDX, instruction_payload_type::ZERO_PAGE, 0),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  instruction(instruction_name::LAX, instruction_payload_type::ZERO_PAGE, 0),
  instruction(instruction_name::NONE, instruction_payload_type::NONE, 0),

  // signal the end of the sequences to test.
  instruction(instruction_name::UNKNOWN, instruction_payload_type::NONE, 0),
};

int test_compare() {
  int i = 0;
  while (comparisons[i].name != instruction_name::UNKNOWN) {
    std::cout << "COMPARING:" << std::endl;
    z_initial_state z_state;
    zmachine a(z_state);
    zmachine b(z_state);

    while (comparisons[i++].name != instruction_name::NONE) {
      zuint16_t cycles = a.cycles();
      a.run(comparisons[i-1]);
      std::cout << "  "
        << instruction_to_string(comparisons[i-1])
        << " {" << usage_flags_to_string(a._uses)
        << " -> " << usage_flags_to_string(a._affects)
        << "}"
        << "{" << (a.cycles() - cycles).val.simplify() << " cycles}"
        << std::endl;
      a._uses = usage_flags::NONE;
      a._affects = usage_flags::NONE;
    }
    std::cout << "WITH:" << std::endl;

    while (comparisons[i++].name != instruction_name::NONE) {
      b.run(comparisons[i-1]);
      std::cout << "  "
        << instruction_to_string(comparisons[i-1])
        << " {" << usage_flags_to_string(b._uses)
        << " -> " << usage_flags_to_string(b._affects)
        << "}"
        << "{" << b.cycles().val.simplify() << " cycles}"
        << std::endl;
      b._uses = usage_flags::NONE;
      b._affects = usage_flags::NONE;
    }

    std::cout << compare(a, b) << std::endl << std::endl;
  }
  return 0;
}

int main(int argc, char **argv) {
  if (argv[1] == std::string("test")) {
    return compare_abstract_and_concrete(argv[2]);
  } else if (argv[1] == std::string("nestest")) {
    return nes_test(argv[2]);
  } else if (argv[1] == std::string("init")) {
    return initialize();
  } else if (argv[1] == std::string("enumerate")) {
    return enumerate();
  } else if (argv[1] == std::string("compare")) {
    return test_compare();
  }
}