// Adds assertions that stack operations never cause
// the stack pointer to overflow or underflow. This
// is useful for ensuring the stack usage is valid.
#define ASSUME_NO_STACK_OVERFLOW true
// Adds assertions that all memory operations in page 1
// are inside of the active stack area, since anything outside
// of it could be overwritten by an interrupt.
#define ASSUME_VALID_STACK_USAGE true
// Adds assertions that the arguments to ADC and SBC are valid
// BCD when the decimal flag is set.
#define ASSUME_VALID_BCD true
// When using the hash machine, the number of reads and writes
// to record
#define NUM_ADDRESSES_TO_REMEMBER 8

// Set this to PROCESSOR_2a03, PROCESSOR_65c02, or PROCESSOR_NMOS_6502
#define PROCESSOR_2a03


#ifdef PROCESSOR_2a03
// The processor on the NES is an NMOS 6502,
// but the decimal mode is cut for patent reasons.

#define DECIMAL_ENABLED        false
#define HAS_JUMP_INDIRECT_BUG  true
#define USE_65c02_DECIMAL      false

#elif defined(PROCESSOR_65c02)
// The 65c02 family of processors includes
// extra instructions and fixes some bugs.

#define DECIMAL_ENABLED        true
#define HAS_JUMP_INDIRECT_BUG  false
#define USE_65c02_DECIMAL      true

#elif defined(PROCESSOR_NMOS_6502)
// The classic 6502

#define DECIMAL_ENABLED        true
#define HAS_JUMP_INDIRECT_BUG  true
#define USE_65c02_DECIMAL      false

#endif

#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>
#include "z3++.h"
#include "zstdint.h"
#include "initial_state.h"
#include "nstdint.h"

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

  // Reset memory bit
  RMB0 = 69,
  RMB1 = 70,
  RMB2 = 71,
  RMB3 = 72,
  RMB4 = 73,
  RMB5 = 74,
  RMB6 = 75,
  RMB7 = 76,

  // Set memory bit
  SMB0 = 77,
  SMB1 = 78,
  SMB2 = 79,
  SMB3 = 80,
  SMB4 = 81,
  SMB5 = 82,
  SMB6 = 83,
  SMB7 = 84,

  // Branch if bit reset
  BBR0 = 85,
  BBR1 = 86,
  BBR2 = 87,
  BBR3 = 88,
  BBR4 = 89,
  BBR5 = 90,
  BBR6 = 91,
  BBR7 = 92,

  // Branch if bit set
  BBS0 = 93,
  BBS1 = 94,
  BBS2 = 95,
  BBS3 = 96,
  BBS4 = 97,
  BBS5 = 98,
  BBS6 = 99,
  BBS7 = 100,

  // NMOS 6502 stable undocumented instructions
  LAX = 101,
  SAX = 102,
  DCM = 103,
  INS = 104,
  SLO = 105,
  RLA = 106,
  SRE = 107,
  RRA = 108,
  ALR = 109,
  ANC = 110,
  AXS = 111,
  ARR = 112,

  UNKNOWN = 255,
};

// Instructions can have const payloads or arbitrary payloads
// with various addressing modes.
// Arbitrary payloads are either z3 named variables, representing
// an unknown initial state, or a value based of a hash of the name.
// For example, ABSOLUTE with payload 3, will be a z3 value named Absolute3
// or an arbitrary consistent value based on the hash seed.
// Constant payloads are actual values.
enum struct instruction_payload_type: uint8_t {
  // Flag indicating the instruction is 2 bytes long
  SIZE_2 = 0x40,
  // Flag indicating the instruction is 3 bytes long
  SIZE_3 = 0x60,
  // Flag indicating the payload is a constant value.
  CONST_FLAG = 0x80,

  // for instructions with no operands
  NONE = 0x0,
  // an arbitrary 16 bit address
  ABSOLUTE = 0x1 | SIZE_3,
  // am arbitrary 16 bit address + x
  ABSOLUTE_X = 0x2 | SIZE_3,
  // an arbitrary 16 bit address + y
  ABSOLUTE_Y = 0x3 | SIZE_3,
  // the pointer at (an arbitrary 8 bit address + x)
  X_INDIRECT = 0x4 | SIZE_2,
  // (the pointer at an arbitrary 8 bit address) + y
  INDIRECT_Y = 0x5 | SIZE_2,
  // an arbitrary 8 bit address
  ZERO_PAGE = 0x6 | SIZE_2,
  // an arbitrary 8 bit address + x
  ZERO_PAGE_X = 0x7 | SIZE_2,
  // an arbitrary 8 bit address + y
  ZERO_PAGE_Y = 0x8 | SIZE_2,
  // an arbitrary immediate constant
  IMMEDIATE = 0x9 | SIZE_2,
  // the pointer at (an arbitrary 16 bit address)
  INDIRECT_ABSOLUTE = 0xA | SIZE_3,
  // the pointer at (an arbitrary 16 bit address + x)
  X_INDIRECT_ABSOLUTE = 0xB | SIZE_2,
  // the pointer at (an arbitrary 8 bit address)
  ZERO_PAGE_INDIRECT = 0xC | SIZE_2,
  // relative branch address (an arbitrary 16 bit address)
  RELATIVE = 0xD | SIZE_2,
  // 8 bit relative branch and 8 bit address
  BRANCH_IF_BIT = 0xE | SIZE_3,
  // Software stack addressing on the zero page using the X register,
  // i.e., lda Stack+payload, x
  SOFTWARE_STACK_CONST = 0xF | SIZE_2 | CONST_FLAG,
  // i.e., lda (Stack + payload, x)
  SOFTWARE_STACK_INDIRECT_CONST = 0x10 | SIZE_2 | CONST_FLAG,
  // A reference to a known subroutine with the given ID. Allows
  // defining known subroutines and optimizing code which calls them.
  KNOWN_SUBROUTINE_CONST = 0x11 | SIZE_3 | CONST_FLAG,

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
};

constexpr uint8_t instruction_size(instruction_payload_type pt) {
  auto flags = (static_cast<uint8_t>(pt) & 0x60) >> 5;
  if (flags) { return flags; }
  return 1;
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
  initial_state& _initial_state;

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
    _initial_state(s) {}

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
  u8 a() const { return _a; }
  void a(u8 val) { _a = ite8(_has_exited, _a, val); }
  u8 x() const { return _x; }
  void x(u8 val) { _x = ite8(_has_exited, _x, val); }
  u8 y() const { return _y; }
  void y(u8 val) { _y = ite8(_has_exited, _y, val); }
  u8 sp() const { return _sp; }
  void sp(u8 val) { _sp = ite8(_has_exited, _sp, val); }
  boolean cc_s() const { return _cc_s; }
  void cc_s(boolean val) { _cc_s = iteB(_has_exited, _cc_s, val); }
  boolean cc_v() const { return _cc_v; }
  void cc_v(boolean val) { _cc_v = iteB(_has_exited, _cc_v, val); }
  boolean cc_i() const { return _cc_i; }
  void cc_i(boolean val) { _cc_i = iteB(_has_exited, _cc_i, val); }
  boolean cc_d() const { return _cc_d; }
  void cc_d(boolean val) { _cc_d = iteB(_has_exited, _cc_d, val); }
  boolean cc_c() const { return _cc_c; }
  void cc_c(boolean val) { _cc_c = iteB(_has_exited, _cc_c, val); }
  boolean cc_z() const { return _cc_z; }
  void cc_z(boolean val) { _cc_z = iteB(_has_exited, _cc_z, val); }
  u16 pc() const { return _pc; }
  void pc(u16 val) { _pc = ite16(_has_exited, _pc, val); }

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

  u8 set_sz(u8 val) {
    cc_s(val >= 0x80);
    cc_z(val == 0);
    return val;
  }

  u8 read(u16 addr) {
    if (ASSUME_VALID_STACK_USAGE) {
      assume((addr & 0xFF00) != 0x100 || ((addr & 0xFF) > u16(sp())));
    }
    return _memory.read(addr);
  }

  void write(u16 addr, u8 val) {
    if (ASSUME_VALID_STACK_USAGE) {
      assume((addr & 0xFF00) != 0x100 || ((addr & 0xFF) > u16(sp())));
    }
    _memory.write_if(!_has_exited, addr, val);
  }

  u8 pull() {
    sp(sp() + 1);
    if (ASSUME_NO_STACK_OVERFLOW) {
      assume(sp() != 0);
    }
    return read(from_bytes(0x1, sp()));
  }

  u8 push(u8 val) {
    auto addr = from_bytes(0x1, sp());
    write(addr, val);
    sp(sp() - 1);
    if (ASSUME_NO_STACK_OVERFLOW) {
      assume(sp() != 0xFF);
    }
    return val;
  }

  u8 flags_to_byte() const {
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

    // Get some useful values for the instruction implementations.
    const u16 absolute_payload = _initial_state.absolute(ins.payload, is_constant);
    const u8 zp_payload = _initial_state.zp(ins.payload & 0xFF, is_constant);
    const u8 immediate_payload = _initial_state.immediate(ins.payload & 0xFF, is_constant);
    const u8 upper_payload = _initial_state.zp(ins.payload >> 8, is_constant);
    const u8 relative_payload = _initial_state.relative(ins.payload & 0xFF, is_constant);

    // The address the instruction is working on
    u16 absolute_var = absolute_payload;
    // The value the instruction is working on
    u8 immediate_var = immediate_payload;

    // Increment the program counter by the instruction size.
    pc(pc() + instruction_size(ins.payload_type));

    // Apply the addressing mode to the instruction payload
    switch(ins.payload_type) {
    case instruction_payload_type::NONE:
      // nothing to do
      break;
    case instruction_payload_type::RELATIVE:
    case instruction_payload_type::RELATIVE_CONST:
      absolute_var = ite16(
        relative_payload >= 0x80,
        pc() - 0x100 + u16(relative_payload),
        pc() + u16(relative_payload)
      );
      break;
    case instruction_payload_type::ABSOLUTE:
    case instruction_payload_type::ABSOLUTE_CONST:
      absolute_var = absolute_payload;
      break;
    case instruction_payload_type::ABSOLUTE_X:
    case instruction_payload_type::ABSOLUTE_X_CONST:
      absolute_var = absolute_payload + u16(x());
      break;
    case instruction_payload_type::ABSOLUTE_Y:
    case instruction_payload_type::ABSOLUTE_Y_CONST:
      absolute_var = absolute_payload + u16(y());
      break;
    case instruction_payload_type::X_INDIRECT:
    case instruction_payload_type::X_INDIRECT_CONST:
      // lda (addr, x)
      absolute_var = from_bytes(read(u16(u8(zp_payload + x() + 1))),
                                read(u16(u8(zp_payload + x()))));
      break;
    case instruction_payload_type::INDIRECT_Y:
    case instruction_payload_type::INDIRECT_Y_CONST:
      // lda (addr), y
      absolute_var = from_bytes(read(u16(u8(zp_payload + 1))),
                                read(u16(zp_payload))) + u16(y());
      break;
    case instruction_payload_type::ZERO_PAGE:
    case instruction_payload_type::ZERO_PAGE_CONST:
      absolute_var = u16(zp_payload);
      break;
    case instruction_payload_type::ZERO_PAGE_X:
    case instruction_payload_type::ZERO_PAGE_X_CONST:
      absolute_var = u16(u8(zp_payload + x()));
      break;
    case instruction_payload_type::ZERO_PAGE_Y:
    case instruction_payload_type::ZERO_PAGE_Y_CONST:
      absolute_var = u16(u8(zp_payload + y()));
      break;
    case instruction_payload_type::IMMEDIATE:
    case instruction_payload_type::IMMEDIATE_CONST:
      immediate_var = immediate_payload;
      break;
    case instruction_payload_type::INDIRECT_ABSOLUTE:
    case instruction_payload_type::INDIRECT_ABSOLUTE_CONST:
      {
      auto hi = HAS_JUMP_INDIRECT_BUG
        ? (absolute_var & 0xFF00) | ((absolute_var + 1) & 0xFF)
        : absolute_var + 1;
      absolute_var = from_bytes(read(hi), read(absolute_var));
      break;
      }
    case instruction_payload_type::X_INDIRECT_ABSOLUTE:
    case instruction_payload_type::X_INDIRECT_ABSOLUTE_CONST:
      absolute_var = from_bytes(
        read(absolute_payload + u16(x()) + 1),
        read(absolute_payload + u16(x())));
      break;
    case instruction_payload_type::ZERO_PAGE_INDIRECT:
    case instruction_payload_type::ZERO_PAGE_INDIRECT_CONST:
      absolute_var = from_bytes(read(u16((zp_payload + 1) & 0xFF)),
                                read(u16(zp_payload)));
      break;
    case instruction_payload_type::BRANCH_IF_BIT:
    case instruction_payload_type::BRANCH_IF_BIT_CONST:
      absolute_var = ite16(
        relative_payload >= 0x80,
        pc() - 0x100 + u16(relative_payload),
        pc() + u16(relative_payload)
      );
      immediate_var = upper_payload;
      break;
    case instruction_payload_type::SOFTWARE_STACK_CONST:
      absolute_var = u16(_initial_state.software_stack() + x() + (ins.payload & 0xFF));
      break;
    case instruction_payload_type::SOFTWARE_STACK_INDIRECT_CONST: {
      // e.g. lda (stack + payload, x)
      u8 addr = _initial_state.software_stack() + x() + (ins.payload & 0xFF);
      absolute_var = from_bytes(read(u16(addr + 1)),
                                read(u16(addr + x())));
      break;
      }
    case instruction_payload_type::KNOWN_SUBROUTINE_CONST:
      // e.g. jsr subroutine2
      absolute_var = _initial_state.known_subroutine(ins.payload);
      break;
    case instruction_payload_type::SIZE_2:
    case instruction_payload_type::SIZE_3:
    case instruction_payload_type::CONST_FLAG:
      assert(false);
    }

    // If we aren't using the immediate operand, then read the address
    // and store it there.
    if (ins.payload_type != instruction_payload_type::IMMEDIATE
    && ins.payload_type != instruction_payload_type::IMMEDIATE_CONST
    && ins.payload_type != instruction_payload_type::BRANCH_IF_BIT
    && ins.payload_type != instruction_payload_type::BRANCH_IF_BIT_CONST) {
      immediate_var = read(absolute_var);
    }

    switch (ins.name) {
    case instruction_name::UNKNOWN: exit(0xFF); break;

    // These instructions do nothing, or put the processor
    // in a state that it can't recover from on its own.
    case instruction_name::STP:
    case instruction_name::WAI:
    case instruction_name::NOP:
    case instruction_name::NONE: break;

    case instruction_name::AND: a(set_sz(a() & immediate_var)); break;
    case instruction_name::ORA: a(set_sz(a() | immediate_var)); break;
    case instruction_name::EOR: a(set_sz(a() ^ immediate_var)); break;
    case instruction_name::LDA: a(set_sz(immediate_var)); break;
    case instruction_name::LDX: x(set_sz(immediate_var)); break;
    case instruction_name::LDY: y(set_sz(immediate_var)); break;
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
        write(absolute_var, set_sz(immediate_var + 1));
      }
      break;
    case instruction_name::DEC:
      if (ins.payload_type == instruction_payload_type::NONE) {
        a(set_sz(a() - 1));
      } else {
        write(absolute_var, set_sz(immediate_var - 1));
      }
      break;
    case instruction_name::BIT:
      cc_z((immediate_var & a()) == 0);
      cc_s((immediate_var & 0x80) == 0x80);
      cc_v((immediate_var & 0x40) == 0x40);
      break;
    case instruction_name::ASL:
      if (ins.payload_type == instruction_payload_type::NONE) {
        cc_c((a() & 0x80) == 0x80);
        a(set_sz(a() << 1));
      } else {
        cc_c((immediate_var & 0x80) == 0x80);
        write(absolute_var, set_sz(immediate_var << 1));
      }
      break;
    case instruction_name::ROL:
      if (ins.payload_type == instruction_payload_type::NONE) {
        auto val = (a() << 1) | (u8)cc_c();
        cc_c((a() & 0x80) == 0x80);
        a(set_sz(val));
      } else {
        auto val = (immediate_var << 1) | (u8)cc_c();
        cc_c((immediate_var & 0x80) == 0x80);
        write(absolute_var, set_sz(val));
      }
      break;
    case instruction_name::LSR:
      if (ins.payload_type == instruction_payload_type::NONE) {
        cc_c((a() & 0x01) == 0x01);
        a(set_sz(a() >> 1));
      } else {
        cc_c((immediate_var & 0x01) == 0x01);
        write(absolute_var, set_sz(immediate_var >> 1));
      }
      break;
    case instruction_name::ROR:
      if (ins.payload_type == instruction_payload_type::NONE) {
        auto val = (a() >> 1) | ite8(cc_c(), (u8)0x80, (u8)0x00);
        cc_c((a() & 0x01) == 0x01);
        a(set_sz(val));
      } else {
        auto val = (immediate_var >> 1) | ite8(cc_c(), (u8)0x80, (u8)0x00);
        cc_c((immediate_var & 0x01) == 0x01);
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
            !cc_d() || (
              (a() & 0xF) < 0xA
              && (a() & 0xF0) < 0xA0
              && (immediate_var & 0xF) < 0xA
              && (immediate_var & 0xF0) < 0xA0));
        }
      
        if (USE_65c02_DECIMAL) {
          // 65c02 decimal mode uses an extra cycle to
          // ensure the flags are more meaningful, and it
          // has different behavior for invalid decimal operands.

          u16 src = (u16)immediate_var;
          u16 bdiff = u16(a()) - src + u16(cc_c()) - 1;
          cc_v(((a() ^ lobyte(bdiff)) & (a() ^ immediate_var) & 0x80) == 0x80);

          u16 ddiff = bdiff;
          ddiff = ddiff - ite16(ddiff > 0xFF, (u16)0x60, (u16)0);
          u16 tmp2 = u16(a() & 0xF) - (src & 0xF) + u16(cc_c()) - 1;
          ddiff = ddiff - ite16(tmp2 > 0xFF, (u16)6, (u16)0);

          u16 diff = ite16(cc_d(), ddiff, bdiff);
          cc_c(u16(a()) + u16(cc_c()) - 1 >= src);
          a(set_sz(lobyte(diff)));
        } else {
            // Difference if decimal
            u16 ddiff = u16(a() & 0xf) - u16(immediate_var & 0xf) - u16(!cc_c()); 
            ddiff = ite16(
              (ddiff & 0x10) == 0x10,
              ((ddiff - 6) & 0xF) | ((u16(a()) & 0xF0) - (u16(immediate_var) & 0xF0) - 0x10),
              (ddiff & 0xF) | ((u16(a()) & 0xF0) - (u16(immediate_var) & 0xF0))
            );

            ddiff = ddiff - ite16((ddiff & 0x100) == 0x100, u16(0x60), u16(0));                

            // Difference if binary
            u16 bdiff = u16(a()) - (u16)immediate_var - u16(!cc_c());

            // TODO: this matches VICE, but are the flags really just based
            // on the binary difference?
            cc_c(bdiff < 0x100);
            set_sz(lobyte(bdiff));
            cc_v(((a() ^ lobyte(bdiff)) & (a() ^ immediate_var) & 0x80) == 0x80);

            u16 diff = ite16(cc_d(), ddiff, bdiff);
            a(lobyte(diff));
        }
      } else {
        u16 diff = u16(a()) - (u16)immediate_var - u16(!cc_c());
        cc_v(((lobyte(diff) ^ a()) & (a() ^ immediate_var) & 0x80) == 0x80);
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
            !cc_d() || (
              (a() & 0xF) < 0xA
              && (a() & 0xF0) < 0xA0
              && (immediate_var & 0xF) < 0xA
              && (immediate_var & 0xF0) < 0xA0));
        }

        if (USE_65c02_DECIMAL) {
          // 65c02 decimal mode uses an extra cycle to
          // ensure the flags are more meaningful, and it
          // has different behavior for invalid decimal operands.
          
          // Sum if decimal mode
          u16 dsumlo = u16((a() & 0xF) + (immediate_var & 0xF) + (u8)cc_c());
          u16 dsumhi = u16((a() & 0xF0) + (immediate_var & 0xF0));
          boolean t = dsumlo > 9;
          dsumlo = dsumlo + ite16(t, (u16)6, (u16)0);
          dsumhi = dsumhi + ite16(t, (u16)0x10, (u16)0);

          // Sum if binary mode
          u16 bsum = u16(immediate_var) + u16(a()) + u16(cc_c());

          u16 sum = ite16(cc_d(), dsumhi, bsum);
          cc_v(((a() ^ lobyte(sum)) & (a() ^ immediate_var) & 0x80) == 0x80);
          dsumhi = dsumhi + ite16(dsumhi > 0x90, (u16)0x60, (u16)0);
          
          sum = ite16(cc_d(), dsumhi, bsum);
          cc_c(sum > 0xFF);
          
          u16 dsum = (dsumlo & 0xF) + (dsumhi & 0xF0);
          sum = ite16(cc_d(), dsum, bsum);
          a(set_sz(lobyte(sum)));
        } else {
          // Sum if decimal mode
          u16 dsum = u16((a() & 0xF) + (immediate_var & 0xF) + (u8)cc_c());
          dsum = dsum + ite16(dsum > 0x9, (u16)6, (u16)0);
          dsum = (dsum & 0xF) + u16(a() & 0xF0) + u16(immediate_var & 0xF0) + ite16(dsum > 0xF, (u16)0x10, (u16)0);

          // Sum if binary mode
          u16 bsum = u16(immediate_var) + u16(a()) + u16(cc_c());

          u16 sum = ite16(cc_d(), dsum, bsum);

          cc_z(lobyte(bsum) == 0); // Zero flag is always based on binary sum
          cc_s((sum & 0x80) == 0x80);
          cc_v(((lobyte(sum) ^ a()) & (lobyte(sum) ^ immediate_var) & 0x80) == 0x80);

          // Perform wrap around if greater than 99
          dsum = dsum + ite16((dsum & 0x1F0) > 0x90, (u16)0x60, (u16)0);

          sum = ite16(cc_d(), dsum, bsum);
          cc_c(sum > 0xFF);
          a(lobyte(sum));
        }
      } else {
        auto carry = u16(cc_c());
        auto sum = u16(immediate_var) + u16(a()) + u16(carry);

        cc_v(((lobyte(sum) ^ a()) & (lobyte(sum) ^ immediate_var) & 0x80) == 0x80);
        cc_c(sum > 0xFF);
        a(set_sz(lobyte(sum)));
      }
      
      break;
      }
    case instruction_name::CMP:
      cc_c(a() >= immediate_var);
      cc_s(u8(a() - immediate_var) >= 0x80);
      cc_z(a() == immediate_var);
      break;
    case instruction_name::CPX:
      cc_c(x() >= immediate_var);
      cc_s(u8(x() - immediate_var) >= 0x80);
      cc_z(x() == immediate_var);
      break;
    case instruction_name::CPY:
      cc_c(y() >= immediate_var);
      cc_s(u8(y() - immediate_var) >= 0x80);
      cc_z(y() == immediate_var);
      break;
    case instruction_name::JMP: exit(absolute_var); break;
    case instruction_name::RTI: {
      byte_to_flags(pull());
      u8 lo = pull();
      u8 hi = pull();
      exit(from_bytes(hi, lo));
      break;
      }
    case instruction_name::RTS: {
      u8 lo = pull();
      u8 hi = pull();
      exit(from_bytes(hi, lo) + 1);
      break;
      }
    case instruction_name::BPL: exit_if(!cc_s(), absolute_var); break;
    case instruction_name::BMI: exit_if(cc_s(),  absolute_var); break;
    case instruction_name::BVS: exit_if(cc_v(),  absolute_var); break;
    case instruction_name::BVC: exit_if(!cc_v(), absolute_var); break;
    case instruction_name::BCC: exit_if(!cc_c(), absolute_var); break;
    case instruction_name::BCS: exit_if(cc_c(),  absolute_var); break;
    case instruction_name::BEQ: exit_if(cc_z(),  absolute_var); break;
    case instruction_name::BNE: exit_if(!cc_z(), absolute_var); break;
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
      exit(from_bytes(read(0xFFFF), read(0xFFFE)));
      break;
    case instruction_name::LAX:
      a(set_sz(immediate_var));
      x(a());
      break;
    case instruction_name::SAX: write(absolute_var, a() & x()); break;
    case instruction_name::DCM:
      immediate_var = immediate_var - 1;
      write(absolute_var, set_sz(immediate_var));
      cc_c(a() >= immediate_var);
      cc_s(a() - immediate_var >= 0x80);
      cc_z(a() == immediate_var);
      break;
    case instruction_name::INS: {
      immediate_var = immediate_var + 1;
      write(absolute_var, set_sz(immediate_var));
      immediate_var = immediate_var ^ 0xFF;
      auto sum = u16(immediate_var) + u16(a()) + u16(cc_c());
      cc_v(((lobyte(sum) ^ a()) & (lobyte(sum) ^ immediate_var) & 0x80) == 0x80);
      cc_c(hibyte(sum) == 0x01);
      a(set_sz(lobyte(sum)));
      break;
      }
    case instruction_name::SLO:
      cc_c((immediate_var & 0x80) == 0x80);
      immediate_var = immediate_var << 1;
      write(absolute_var, set_sz(immediate_var));
      a(set_sz(a() | immediate_var));
      break;
    case instruction_name::RLA: {
      auto val = (immediate_var << 1) | ite8(cc_c(), (u8)1, (u8)0);
      cc_c((immediate_var & 0x80) == 0x80);
      write(absolute_var, set_sz(val));
      a(set_sz(val & a()));
      break;
      }
    case instruction_name::SRE:
      cc_c((immediate_var & 0x01) == 0x01);
      immediate_var = immediate_var >> 1;
      write(absolute_var, set_sz(immediate_var));
      a(set_sz(a() ^ immediate_var));
      break;
    case instruction_name::RRA: {
      auto val = (immediate_var >> 1) | ite8(cc_c(), (u8)0x80, (u8)0x00);
      cc_c((immediate_var & 0x01) == 0x01);
      write(absolute_var, set_sz(val));
      auto sum = u16(val) + u16(a()) + ite16(cc_c(), (u16)0x01, (u16)0x00);
      cc_v(((lobyte(sum) ^ a()) & (lobyte(sum) ^ val) & 0x80) == 0x80);
      cc_c(hibyte(sum) == 0x01);
      a(set_sz(lobyte(sum)));
      break;
      }
    case instruction_name::ALR: {
      auto val = (a() & immediate_var) >> 1;
      cc_c((a() & immediate_var & 0x01) == 0x01);
      a(set_sz(val));
      break;
      }
    case instruction_name::ANC:
      a(set_sz(a() & immediate_var));
      cc_c(cc_s());
      break;
    case instruction_name::ARR: {
      a(set_sz(a() & immediate_var));
      auto val = (a() >> 1) | ite8(cc_c(), (u8)0x80, (u8)0x00);
      cc_c((val & 0x40) == 0x40);
      auto bit6 = (val & 0x40) == 0x40;
      auto bit5 = (val & 0x20) == 0x20;
      cc_v(bit6 ^ bit5);
      a(set_sz(val));
      break;
      }
    case instruction_name::AXS: {
      auto xa = x() & a();
      x(set_sz(xa - immediate_var));
      cc_c(xa >= immediate_var);
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
      write(absolute_var, immediate_var & mask);
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
      write(absolute_var, immediate_var | (1 << bit));
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
      exit_if((immediate_var & (1 << bit)) == 0, absolute_var);
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
      exit_if((immediate_var & (1 << bit)) != 0, absolute_var);
      break;
      }
    case instruction_name::TRB: {
      cc_z((immediate_var & a()) == 0);
      write(absolute_var, immediate_var & ~a());
      break;
      }
    case instruction_name::TSB: {
      cc_z((immediate_var & a()) == 0);
      write(absolute_var, immediate_var | a());
      break;
      }
    }
  }
};

typedef machine<zuint8_t, zuint16_t, zbool, zmemory, z_initial_state> zmachine;
typedef machine<nuint8_t, nuint16_t, bool, hmemory, hash_initial_state> hmachine;
typedef machine<nuint8_t, nuint16_t, bool, cmemory, concrete_initial_state> cmachine;

uint16_t read16(const uint8_t memory[256*256], const uint16_t ip) {
  return memory[ip + 1] | (memory[ip + 2] << 8);
}

/**
 * Reads the byte after the IP as a signed relative branch argument,
 * and returns the new instruction pointer.
 */
uint16_t read_rel(const uint8_t memory[256*256], const uint16_t ip) {
  return memory[ip+1];
}

#define BYTE memory[pc+1]
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
        auto original_memory = z_machine._memory.val;
        // Run the instruction on both machines
        c_machine.run(ins);
        z_machine.run(ins);
        z3::solver solver(z3_ctx);
        // Add assertions for the memory locations which were read.
        for (int i = 0; i < c_machine._memory.n_read; i++) {
          auto equal = z3::select(original_memory, c_machine._memory.addresses_read[i].val) == c_machine._memory.values_read[i].val;
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

        auto result = solver.check();
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

int main(int argc, char **argv) {
  instruction ins(instruction_name::INX, instruction_payload_type::NONE, 0);
  z_initial_state z_state;
  zmachine z(z_state);
  z.run(ins);
  hash_initial_state h_state(1234);
  hmachine h(h_state);
  h.run(ins);

  uint8_t memory[65536];
  concrete_initial_state c_state(memory);
  cmachine c(c_state);
  c.run(ins);

  if (argv[1] == std::string("test")) {
    return compare_abstract_and_concrete(argv[2]);
  } else if (argv[1] == std::string("nestest")) {
    return nes_test(argv[2]);
  } else if (argv[1] == std::string("init")) {
    return initialize();
  } else if (argv[1] == std::string("enumerate")) {
    return enumerate();
  }
}