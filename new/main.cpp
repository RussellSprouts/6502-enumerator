#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>
#include "z3++.h"
#include "operations.h"


// Configs

// Adds assertions that stack operations never cause
// the stack pointer to overflow or underflow
#define ASSUME_NO_STACK_OVERFLOW true
// Adds assertions that all memory operations in page 1
// are inside of the active stack area.
#define ASSUME_VALID_STACK_USAGE true
// Allows use of temporary memory locations.
// The result of writing to these locations is ignored.
#define IGNORE_TMP_MEMORY false

#define PROCESSOR_NMOS_6502

// Results
#ifdef PROCESSOR_NMOS_6502

#define DECIMAL_ENABLED        true
#define HAS_JUMP_INDIRECT_BUG  true

#elif defined(PROCESSOR_65c02)

#define DECIMAL_ENABLED        true
#define HAS_JUMP_INDIRECT_BUG  false

#endif

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

enum struct instruction_payload_type: uint8_t {
  SIZE_2 = 0x40,
  SIZE_3 = 0x60,
  CONST_FLAG = 0x80,

  // for instructions with no operands
  NONE = 0,
  // an arbitrary 16 bit address
  ABSOLUTE = 1 | SIZE_3,
  // am arbitrary 16 bit address + x
  ABSOLUTE_X = 2 | SIZE_3,
  // an arbitrary 16 bit address + y
  ABSOLUTE_Y = 3 | SIZE_3,
  // the pointer at (an arbitrary 8 bit address + x)
  X_INDIRECT = 4 | SIZE_2,
  // (the pointer at an arbitrary 8 bit address) + y
  INDIRECT_Y = 5 | SIZE_2,
  // an arbitrary 8 bit address
  ZERO_PAGE = 6 | SIZE_2,
  // an arbitrary 8 bit address + x
  ZERO_PAGE_X = 7 | SIZE_2,
  // an arbitrary 8 bit address + y
  ZERO_PAGE_Y = 8 | SIZE_2,
  // an arbitrary immediate constant
  IMMEDIATE = 9 | SIZE_2,
  // the pointer at (an arbitrary 16 bit address)
  INDIRECT_ABSOLUTE = 10 | SIZE_3,
  // the pointer at (an arbitrary 16 bit address + x)
  X_INDIRECT_ABSOLUTE = 11 | SIZE_2,
  // the pointer at (an arbitrary 8 bit address)
  ZERO_PAGE_INDIRECT = 12 | SIZE_2,
  // relative branch address (an arbitrary 16 bit address)
  RELATIVE = 13 | SIZE_2,
  // 8 bit relative branch and 8 bit address
  BRANCH_IF_BIT = 14 | SIZE_3,

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
 * Instruction encoding 
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

struct concrete_machine {
  explicit concrete_machine(uint64_t seed, uint8_t *memory):
    _seed(seed),
    _memory(memory),
    _a(0),
    _x(0),
    _y(0),
    _sp(0xFD),
    _p(FLAG_ALWAYS | FLAG_I),
    _addresses_read{0},
    _values_read{0},
    _n_read(0),
    _assumptions(true),
    _record_memory(true)
    {}

  static constexpr uint8_t FLAG_S = 0x80;
  static constexpr uint8_t FLAG_V = 0x40;
  static constexpr uint8_t FLAG_ALWAYS = 0x20;
  static constexpr uint8_t FLAG_D = 0x08;
  static constexpr uint8_t FLAG_I = 0x04;
  static constexpr uint8_t FLAG_Z = 0x02;
  static constexpr uint8_t FLAG_C = 0x01;

  typedef uint8_t expr8;
  typedef uint16_t expr16;

  uint8_t immediate(uint16_t val, bool is_constant) {
    return val;
  }

  uint16_t absolute(uint16_t val, bool is_constant) {
    return val;
  }

  uint8_t zp(uint16_t val, bool is_constant) {
    return val;
  }

  uint8_t relative(uint16_t val, bool is_constant) {
    return val;
  }

  void exit(uint16_t address) {
    exit_if(true, address);
  }

  void exit_if(bool cond, uint16_t address) {
    if (cond) {
      _pc = E(address, _pc); 
      _exited_early = E(cond, _exited_early);
    }
  }

  uint16_t pc(uint16_t val) { return _pc = E(val, _pc); }
  uint8_t a(uint8_t val) { return _a = E(val, _a); }
  uint8_t x(uint8_t val) { return _x = E(val, _x); }
  uint8_t y(uint8_t val) { return _y = E(val, _y); }
  uint8_t sp(uint8_t val) { return _sp = E(val, _sp); }
  bool cc_s(bool val) { return (_p = E((_p & ~FLAG_S) | (val ? FLAG_S : 0), _p)); }
  bool cc_v(bool val) { return (_p = E((_p & ~FLAG_V) | (val ? FLAG_V : 0), _p)); }
  bool cc_i(bool val) { return (_p = E((_p & ~FLAG_I) | (val ? FLAG_I : 0), _p)); }
  bool cc_d(bool val) { return (_p = E((_p & ~FLAG_D) | (val ? FLAG_D : 0), _p)); }
  bool cc_c(bool val) { return (_p = E((_p & ~FLAG_C) | (val ? FLAG_C : 0), _p)); }
  bool cc_z(bool val) { return (_p = E((_p & ~FLAG_Z) | (val ? FLAG_Z : 0), _p)); }

  uint16_t pc() { return _pc; }
  uint8_t a() { return _a; }
  uint8_t x() { return _x; }
  uint8_t y() { return _y; }
  uint8_t sp() { return _sp; }
  bool cc_s() { return _p & FLAG_S; }
  bool cc_v() { return _p & FLAG_V; }
  bool cc_i() { return _p & FLAG_I; }
  bool cc_d() { return _p & FLAG_D; }
  bool cc_c() { return _p & FLAG_C; }
  bool cc_z() { return _p & FLAG_Z; }
  bool exited_early() { return _exited_early; }

  bool record_memory() { return _record_memory; }
  bool record_memory(bool val) { return (_record_memory = val); }

  uint8_t read(uint16_t address) {
    if (_record_memory) {
      assert(_n_read < 8);
      _addresses_read[_n_read] = address;
      _values_read[_n_read] = _memory[address];
      _n_read++;
    }
    if (ASSUME_VALID_STACK_USAGE) {
      assume((address & 0xFF00) != 0x0100 || (address & 0xFF) > _sp);
    }
    return _memory[address];
  }

  void write(uint16_t address, uint8_t val) {
    if (_record_memory) {
      assert(_n_written < 8);
      _addresses_written[_n_written] = address;
      _values_written[_n_written] = val;
      _n_written++;
    }
    if (ASSUME_VALID_STACK_USAGE) {
      assume((address & 0xFF00) != 0x0100 || (address & 0xFF) > _sp);
    }
    _memory[address] = E(val, _memory[address]);
  }

  uint8_t lobyte(uint16_t val) {
    return val;
  }

  uint8_t hibyte(uint16_t val) {
    return val >> 8;
  }

  uint16_t ite(bool cond, uint16_t conseq, uint16_t alt) {
    return cond ? conseq : alt;
  }

  uint8_t inline shl(const uint8_t val) const {
    return val << 1;
  }

  uint8_t inline shr(const uint8_t val) const {
    return val >> 1;
  }

  uint16_t extend(uint8_t val) {
    return val;
  }

  bool uge(uint8_t first, uint8_t second) const {
    return first >= second;
  }

  bool ugt(uint8_t first, uint8_t second) const {
    return first > second;
  }

  uint16_t from_bytes(uint8_t hi, uint8_t lo) const {
    return (hi << 8) | lo;
  }

  void reset_early_exit() {
    _exited_early = false;
  }

  uint16_t _addresses_read[8];
  uint8_t _values_read[8];
  uint8_t _n_read;
  uint16_t _addresses_written[8];
  uint8_t _values_written[8];
  uint8_t _n_written;
  bool _assumptions;

  void assume(bool value) {
    _assumptions = E(_assumptions && value,_assumptions);
  }

  private:
  inline uint16_t E(uint16_t val, uint16_t orig) {
    return _exited_early ? orig : val;
  }

  uint64_t _seed;

  uint16_t _pc;
  uint8_t *_memory;
  uint8_t _a;
  uint8_t _x;
  uint8_t _y;
  uint8_t _sp;
  uint8_t _p;
  bool _exited_early;
  bool _record_memory;
};

struct prover_context {
  prover_context() {
    std::string absolute_name("absolute");
    for (int i = 0; i < 16; i++) {
      absolute_vars.push_back(context.bv_const((absolute_name + std::to_string(i)).c_str(), 16));
    }
    
    std::string immediate_name("immediate");
    for (int i = 0; i < 16; i++) {
      immediate_vars.push_back(context.bv_const((immediate_name + std::to_string(i)).c_str(), 8));
    }
    
    std::string zp_name("zp");
    for (int i = 0; i < 16; i++) {
      zp_vars.push_back(context.bv_const((zp_name + std::to_string(i)).c_str(), 8));
    }

    std::string relative_name("relative");
    for (int i = 0; i < 16; i++) {
      zp_vars.push_back(context.bv_const((relative_name + std::to_string(i)).c_str(), 8));
    }
  }

  z3::context context;
  std::vector<z3::expr> absolute_vars;
  std::vector<z3::expr> immediate_vars;
  std::vector<z3::expr> zp_vars;
  std::vector<z3::expr> relative_vars;

  z3::expr u8(uint8_t n) {
    return context.bv_val(n, 8);
  }

  z3::expr u8(const char *n) {
    return context.bv_const(n, 8);
  }

  z3::expr u16(uint16_t n) {
    return context.bv_val(n, 16);
  }

  z3::expr u16(const char *n) {
    return context.bv_const(n, 16);
  }

  z3::expr boolean(bool b) {
    return context.bool_val(b);
  }

  z3::expr boolean(const char *n) {
    return context.bool_const(n);
  }

  z3::expr memory() {
    return context.constant("memory", context.array_sort(context.bv_sort(16), context.bv_sort(8)));
  }
};

struct abstract_machine {
  explicit abstract_machine(prover_context &c) : 
  ctx(c),
  _exited_early(c.boolean(false)),
  _memory(c.memory()),
  _assumptions(c.boolean(true)),
  _a(c.u8("a")),
  _x(c.u8("x")),
  _y(c.u8("y")),
  _sp(c.u8("sp")),
  _cc_s(c.boolean("cc_s")),
  _cc_v(c.boolean("cc_v")),
  _cc_i(c.boolean("cc_i")),
  _cc_d(c.boolean("cc_d")),
  _cc_c(c.boolean("cc_c")),
  _cc_z(c.boolean("cc_z")),
  _pc(c.u16("pc")) {
  }

  explicit abstract_machine(prover_context& c, concrete_machine &other): abstract_machine(c) {
    _a = ctx.u8(other.a());
    _x = ctx.u8(other.x());
    _y = ctx.u8(other.y());
    _sp = ctx.u8(other.sp());
    _cc_s = ctx.boolean(other.cc_s());
    _cc_v = ctx.boolean(other.cc_v());
    _cc_i = ctx.boolean(other.cc_i());
    _cc_d = ctx.boolean(other.cc_d());
    _cc_c = ctx.boolean(other.cc_c());
    _cc_z = ctx.boolean(other.cc_z());

    _exited_early = ctx.boolean(other.exited_early());
    _pc = ctx.u16(other.pc());
  }

  z3::expr absolute(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return ctx.u16(payload);
    } else {
      return ctx.absolute_vars.at(payload);
    }
  }

  z3::expr immediate(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return ctx.u8(payload & 0xFF);
    } else {
      return ctx.immediate_vars.at(payload);
    }
  }

  z3::expr zp(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return ctx.u8(payload & 0xFF);
    } else {
      return ctx.zp_vars.at(payload);
    }
  }

  z3::expr relative(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return ctx.u8((payload >> 8) & 0xFF);
    } else {
      return ctx.relative_vars.at((payload >> 8) & 0xFF);
    }
  }

  typedef z3::expr expr8;
  typedef z3::expr expr16;

  void exit(z3::expr const address) {
    exit_if(ctx.boolean(true), address);
  }

  void exit_if(z3::expr const cond, z3::expr const address) {
    _pc = E(ite(cond, address, _pc), _pc); 
    _exited_early = E(cond, _exited_early);
  }

  z3::expr E(z3::expr new_val, z3::expr same) const {
    return ite(!_exited_early, new_val, same);
  }

  /**
   * Writes the val to the addr given. Returns the current memory array.
   */
  z3::expr write(z3::expr addr, z3::expr val) {
    if (addr.get_sort().bv_size() == 8) {
      addr = extend(addr);
    }
    if (ASSUME_VALID_STACK_USAGE) {
      assume((addr & 0xFF00) != 0x100 || ugt(addr & 0xFF, extend(sp())));
    }
    return _memory = E(z3::store(_memory, addr, val), _memory);
  }

  /**
   * Writes the val to the addr given. Returns the current memory array.
   */
  z3::expr write(z3::expr addr, uint8_t val) {
    return write(addr, ctx.u8(val));
  }

  z3::expr write(uint16_t addr, uint8_t val) {
    return write(ctx.u16(addr), ctx.u8(val));
  }

  /**
   * Reads the value at addr and returns it as an 8-bit bitvector.
   */
  z3::expr read(z3::expr addr) {
    if (addr.get_sort().bv_size() == 8) {
      addr = extend(addr);
    }
    if (ASSUME_VALID_STACK_USAGE) {
      assume((addr & 0xFF00) != 0x100 || ugt(addr & 0xFF, extend(sp())));
    }
    return z3::select(_memory, addr);
  }

  z3::expr read(uint16_t addr) {
    return read(ctx.u16(addr));
  }

  void assume(z3::expr value) {
    _assumptions = E(_assumptions && value, _assumptions);
  }

  /**
   * Takes an 8-bit bitvector and zero-extends it to a 16-bit bv.
   */
  z3::expr extend(z3::expr const val) const {
    return z3::zext(val, 8);
  }

  /**
   * If-then-else. If c, then t, else e.
   */
  z3::expr ite(z3::expr const c, z3::expr const t, z3::expr const e) const {
    return z3::ite(c, t, e);
  }

  /**
   * If-then-else with 8-bit literals instead of `z3::expr`s.
   */
  z3::expr ite(z3::expr const cond, uint8_t t, uint8_t e) const {
    return ite(cond, ctx.u8(t), ctx.u8(e));
  }

  /**
   * Shifts the input left by 1.
   */
  z3::expr shl(z3::expr const val) const {
    return z3::shl(val, 1);
  }

  /**
   * Shifts the input right by 1 (logical shift right).
   */
  z3::expr shr(z3::expr const val) const {
    return z3::lshr(val, 1);
  }

  /**
   * Extracts the low byte of the 16-bit bv.
   */
  z3::expr lobyte(z3::expr const val) const {
    return val.extract(7, 0);
  }

  /**
   * Extracts the high byte of the 16-bit bv.
   */
  z3::expr hibyte(z3::expr const val) const {
    return val.extract(15, 8);
  }

  /**
   * Unsigned greater-than or equal.
   */
  z3::expr uge(z3::expr const first, z3::expr const second) const {
    return z3::uge(first, second);
  }

  /**
   * Unsigned greater-than or equal, with a constant.
   */
  z3::expr uge(z3::expr const first, uint8_t second) const {
    return z3::uge(first, second);
  }

  z3::expr ugt(z3::expr const first, z3::expr const second) const {
    return z3::ugt(first, second);
  }

  z3::expr ugt(z3::expr const first, uint8_t second) const {
    return z3::ugt(first, second);
  }

  z3::expr from_bytes(z3::expr hi, z3::expr lo) const {
    return z3::concat(hi, lo);
  }

  z3::expr from_bytes(uint8_t hi, z3::expr lo) const {
    return from_bytes(ctx.u8(hi), lo);
  }

  z3::expr a(z3::expr const val) { return _a = E(val, _a); }
  z3::expr a(uint8_t val) { return a(ctx.u8(val)); }
  z3::expr x(z3::expr const val) { return _x = E(val, _x); }
  z3::expr x(uint8_t val) { return x(ctx.u8(val)); }
  z3::expr y(z3::expr const val) { return _y = E(val, _y); }
  z3::expr y(uint8_t val) { return y(ctx.u8(val)); }
  z3::expr sp(z3::expr const val) { return _sp = E(val, _sp); }
  z3::expr sp(uint8_t val) { return sp(ctx.u8(val)); }
  z3::expr cc_s(z3::expr const val) { return _cc_s = E(val, _cc_s); }
  z3::expr cc_s(bool val) { return cc_s(ctx.boolean(val)); }
  z3::expr cc_v(z3::expr const val) { return _cc_v = E(val, _cc_v); }
  z3::expr cc_v(bool val) { return cc_v(ctx.boolean(val)); }
  z3::expr cc_i(z3::expr const val) { return _cc_i = E(val, _cc_i); }
  z3::expr cc_i(bool val) { return cc_i(ctx.boolean(val)); }
  z3::expr cc_d(z3::expr const val) { return _cc_d = E(val, _cc_d); }
  z3::expr cc_d(bool val) { return cc_d(ctx.boolean(val)); }
  z3::expr cc_c(z3::expr const val) { return _cc_c = E(val, _cc_c); }
  z3::expr cc_c(bool val) { return cc_c(ctx.boolean(val)); }
  z3::expr cc_z(z3::expr const val) { return _cc_z = E(val, _cc_z); }
  z3::expr cc_z(bool val) { return cc_z(ctx.boolean(val)); }
  z3::expr pc(z3::expr const val) { return _pc = E(val, _pc); }
  z3::expr pc(uint16_t val) { return pc(ctx.u16(val)); }  

  z3::expr a() { return _a; }
  z3::expr x() { return _x; }
  z3::expr y() { return _y; }
  z3::expr sp() { return _sp; }
  z3::expr cc_s() { return _cc_s; }
  z3::expr cc_v() { return _cc_v; }
  z3::expr cc_i() { return _cc_i; }
  z3::expr cc_d() { return _cc_d; }
  z3::expr cc_c() { return _cc_c; }
  z3::expr cc_z() { return _cc_z; }
  z3::expr pc() { return _pc; }
  z3::expr exited_early() { return _exited_early; }
  z3::expr memory() { return _memory; }

  void reset_early_exit() {
    _exited_early = ctx.boolean(false);
  }

  abstract_machine& simplify() {
    _exited_early = _exited_early.simplify();
    _memory = _memory.simplify();
    _a = _a.simplify();
    _x = _x.simplify();
    _y = _y.simplify();
    _sp = _sp.simplify();
    _cc_s = _cc_s.simplify();
    _cc_v = _cc_v.simplify();
    _cc_i = _cc_i.simplify();
    _cc_d = _cc_d.simplify();
    _cc_c = _cc_c.simplify();
    _cc_z = _cc_z.simplify();
    _pc = _pc.simplify();

    return *this;
  }

  private:
  prover_context &ctx;

  z3::expr _exited_early;
  z3::expr _memory;
  z3::expr _assumptions;

  // The internal state of the machine.
  z3::expr _a;
  z3::expr _x;
  z3::expr _y;
  z3::expr _sp;
  z3::expr _cc_s;
  z3::expr _cc_v;
  z3::expr _cc_i;
  z3::expr _cc_d;
  z3::expr _cc_c;
  z3::expr _cc_z;
  z3::expr _pc;
};

template<typename machine>
struct emulator {

  explicit emulator(machine &m): m(m) {
  }
  machine &m;

  typename machine::expr8 set_sz(typename machine::expr8 val) {
    m.cc_s(m.uge(val, 0x80));
    m.cc_z(val == 0);
    return val;
  }

  typename machine::expr8 pull() {
    m.sp(m.sp() + 1);
    if (ASSUME_NO_STACK_OVERFLOW) {
      m.assume(m.sp() != 0);
    }
    return m.read(m.from_bytes(0x1, m.sp()));
  }

  typename machine::expr8 push(typename machine::expr8 val) {
    auto addr = m.from_bytes(0x1, m.sp());
    m.write(addr, val);
    m.sp(m.sp() - 1);
    if (ASSUME_NO_STACK_OVERFLOW) {
      m.assume(m.sp() != 0xFF);
    }
    return val;
  }

  typename machine::expr8 flags_to_byte() const {
    return m.ite(m.cc_s(), 0x80, 0x00)
         | m.ite(m.cc_v(), 0x40, 0x00)
         | 0x20
         | m.ite(m.cc_d(), 0x08, 0x00)
         | m.ite(m.cc_i(), 0x04, 0x00)
         | m.ite(m.cc_z(), 0x02, 0x00)
         | m.ite(m.cc_c(), 0x01, 0x00);
  }

  void byte_to_flags(typename machine::expr8 status) {
    m.cc_s(0x80 == (status & 0x80));
    m.cc_v(0x40 == (status & 0x40));
    m.cc_d(0x08 == (status & 0x08));
    m.cc_i(0x04 == (status & 0x04));
    m.cc_z(0x02 == (status & 0x02));
    m.cc_c(0x01 == (status & 0x01));
  }

  void run(const instruction ins) {
    if (ins.name == instruction_name::NONE) {
      return;
    }

    bool is_constant = (uint8_t)(ins.payload_type & instruction_payload_type::CONST_FLAG);

    // Load possible values from the machine
    const auto absolute_payload = m.absolute(ins.payload, is_constant);
    const auto zp_payload = m.zp(ins.payload & 0xFF, is_constant);
    const auto immediate_payload = m.immediate(ins.payload & 0xFF, is_constant);

    // The address the instruction is working on
    auto absolute_var = absolute_payload;
    // The value the instruction is working on
    auto immediate_var = immediate_payload;

    m.pc(m.pc() + instruction_size(ins.payload_type));

    switch(ins.payload_type & ~instruction_payload_type::CONST_FLAG) {
    case instruction_payload_type::NONE:
      // nothing to do
      break;
    case instruction_payload_type::RELATIVE:
      if (m.uge(zp_payload, 0x80)) {
        absolute_var = m.pc() - 0x100 + m.extend(zp_payload);
      } else {
        absolute_var = m.pc() + m.extend(zp_payload);
      }
      break;
    case instruction_payload_type::ABSOLUTE:
      absolute_var = absolute_payload;
      break;
    case instruction_payload_type::ABSOLUTE_X:
      absolute_var = absolute_payload + m.extend(m.x());
      break;
    case instruction_payload_type::ABSOLUTE_Y:
      absolute_var = absolute_payload + m.extend(m.y());
      break;
    case instruction_payload_type::X_INDIRECT:
      // lda (addr, x)
      absolute_var = m.from_bytes(m.read((zp_payload + m.x() + 1) & 0xFF),
                                m.read((zp_payload + m.x()) & 0xFF));
      break;
    case instruction_payload_type::INDIRECT_Y:
      // lda (addr), y
      absolute_var = m.from_bytes(m.read((zp_payload + 1) & 0xFF),
                                m.read(zp_payload)) + m.extend(m.y());
      break;
    case instruction_payload_type::ZERO_PAGE:
      absolute_var = m.extend(zp_payload);
      break;
    case instruction_payload_type::ZERO_PAGE_X:
      absolute_var = m.extend(zp_payload + m.x());
      break;
    case instruction_payload_type::ZERO_PAGE_Y:
      absolute_var = m.extend(zp_payload + m.y());
      break;
    case instruction_payload_type::IMMEDIATE:
      immediate_var = immediate_payload;
      break;
    case instruction_payload_type::INDIRECT_ABSOLUTE: {
      auto hi = HAS_JUMP_INDIRECT_BUG
        ? (absolute_var & 0xFF00) | ((absolute_var + 1) & 0xFF)
        : absolute_var + 1;
      absolute_var = m.from_bytes(m.read(hi), m.read(absolute_var));
      break;
      }
    case instruction_payload_type::X_INDIRECT_ABSOLUTE:
      absolute_var = m.from_bytes(
        m.read(absolute_payload + m.extend(m.x()) + 1),
        m.read(absolute_payload + m.extend(m.x())));
      break;
    case instruction_payload_type::ZERO_PAGE_INDIRECT:
      absolute_var = m.from_bytes(m.read((zp_payload + 1) & 0xFF),
                                m.read(zp_payload));
      break;
    case instruction_payload_type::BRANCH_IF_BIT:
      if (m.uge(zp_payload, 0x80)) {
        absolute_var = m.pc() - 0x100 + m.extend(zp_payload);
      } else {
        absolute_var = m.pc() + m.extend(zp_payload);
      }
      immediate_var = m.zp(m.hibyte(absolute_payload), is_constant);
      break;
    default:
      assert(false);
    }

    // If we aren't using the immediate operand, then read the address
    // and store it there.
    if (ins.payload_type != instruction_payload_type::IMMEDIATE && ins.payload_type != instruction_payload_type::IMMEDIATE_CONST) {
      immediate_var = m.read(absolute_var);
    }

    switch (ins.name) {
    case instruction_name::UNKNOWN:
      exit(0);
    case instruction_name::NONE:
      // nothing to do
      break;
    case instruction_name::AND:
      m.a(set_sz(m.a() & immediate_var));
      break;
    case instruction_name::ORA:
      m.a(set_sz(m.a() | immediate_var));
      break;
    case instruction_name::EOR: 
      m.a(set_sz(m.a() ^ immediate_var));
      break;
    case instruction_name::LDA:
      m.a(set_sz(immediate_var));
      break;
    case instruction_name::LDX:
      m.x(set_sz(immediate_var));
      break;
    case instruction_name::LDY:
      m.y(set_sz(immediate_var));
      break;
    case instruction_name::TXA:
      m.a(set_sz(m.x()));
      break;
    case instruction_name::TAX:
      m.x(set_sz(m.a()));
      break;
    case instruction_name::TYA:
      m.a(set_sz(m.y()));
      break;
    case instruction_name::TAY:
      m.y(set_sz(m.a()));
      break;
    case instruction_name::INX:
      m.x(set_sz(m.x() + 1));
      break;
    case instruction_name::INY:
      m.y(set_sz(m.y() + 1));
      break;
    case instruction_name::DEX:
      m.x(set_sz(m.x() - 1));
      break;
    case instruction_name::DEY:
      m.y(set_sz(m.y() - 1));
      break;
    case instruction_name::CLC:
      m.cc_c(false);
      break;
    case instruction_name::CLI:
      m.cc_i(false);
      break;
    case instruction_name::CLV:
      m.cc_v(false);
      break;
    case instruction_name::CLD:
      m.cc_d(false);
      break;
    case instruction_name::SEC:
      m.cc_c(true);
      break;
    case instruction_name::SEI:
      m.cc_i(true);
      break;
    case instruction_name::SED:
      m.cc_d(true);
      break;
    case instruction_name::TSX:
      m.x(set_sz(m.sp()));
      break;
    case instruction_name::TXS:
      m.sp(m.x());
      break;
    case instruction_name::NOP:
      // nap.
      break;
    case instruction_name::INC:
      if (ins.payload_type == instruction_payload_type::NONE) {
        m.a(set_sz(m.a() + 1));
      } else {
        m.write(absolute_var, set_sz(immediate_var + 1));
      }
      break;
    case instruction_name::DEC:
      if (ins.payload_type == instruction_payload_type::NONE) {
        m.a(set_sz(m.a() - 1));
      } else {
        m.write(absolute_var, set_sz(immediate_var - 1));
      }
      break;
    case instruction_name::BIT:
      m.cc_z((immediate_var & m.a()) == 0);
      m.cc_s((immediate_var & 0x80) == 0x80);
      m.cc_v((immediate_var & 0x40) == 0x40);
      break;
    case instruction_name::ASL:
      if (ins.payload_type == instruction_payload_type::NONE) {
        m.cc_c((m.a() & 0x80) == 0x80);
        m.a(set_sz(m.shl(m.a())));
      } else {
        m.cc_c((immediate_var & 0x80) == 0x80);
        m.write(absolute_var, set_sz(m.shl(immediate_var)));
      }
      break;
    case instruction_name::ROL:
      if (ins.payload_type == instruction_payload_type::NONE) {
        auto val = m.shl(m.a()) | m.ite(m.cc_c(), 1, 0);
        m.cc_c((m.a() & 0x80) == 0x80);
        m.a(set_sz(val));
      } else {
        auto val = m.shl(immediate_var) | m.ite(m.cc_c(), 1, 0);
        m.cc_c((immediate_var & 0x80) == 0x80);
        m.write(absolute_var, set_sz(val));
      }
      break;
    case instruction_name::LSR:
      if (ins.payload_type == instruction_payload_type::NONE) {
        m.cc_c((m.a() & 0x01) == 0x01);
        m.a(set_sz(m.shr(m.a())));
      } else {
        m.cc_c((immediate_var & 0x01) == 0x01);
        m.write(absolute_var, set_sz(m.shr(immediate_var)));
      }
      break;
    case instruction_name::ROR:
      if (ins.payload_type == instruction_payload_type::NONE) {
        auto val = m.shr(m.a()) | m.ite(m.cc_c(), 0x80, 0x00);
        m.cc_c((m.a() & 0x01) == 0x01);
        m.a(set_sz(val));
      } else {
        auto val = m.shr(immediate_var) | m.ite(m.cc_c(), 0x80, 0x00);
        m.cc_c((immediate_var & 0x01) == 0x01);
        m.write(absolute_var, set_sz(val));
      }
      break;
    case instruction_name::STZ:
      m.write(absolute_var, 0);
    case instruction_name::STA:
      m.write(absolute_var, m.a());
      break;
    case instruction_name::STX:
      m.write(absolute_var, m.x());
      break;
    case instruction_name::STY:
      m.write(absolute_var, m.y());
      break;
    case instruction_name::PLA:
      m.a(set_sz(pull()));
      break;
    case instruction_name::PHA:
      push(m.a());
      break;
    case instruction_name::PLX:
      m.x(set_sz(pull()));
      break;
    case instruction_name::PHX:
      push(m.x());
      break;
    case instruction_name::PLY:
      m.y(set_sz(pull()));
      break;
    case instruction_name::PHY:
      push(m.y());
      break;
    case instruction_name::PHP: {
      push(flags_to_byte() | 0x10);
      break;
      }
    case instruction_name::PLP: {
      byte_to_flags(pull());
      break;
      }
    case instruction_name::SBC:
      immediate_var = immediate_var ^ 0xFF;
      /* fallthrough */
    case instruction_name::ADC: {
      auto carry = m.ite(m.cc_c(), 0x01, 0x00);

      if (DECIMAL_ENABLED) {
        auto sumIfDecimal =  (m.a() & 0xF) + (immediate_var & 0xF) + carry;
        sumIfDecimal = m.ite(m.ugt(sumIfDecimal, 0x09), sumIfDecimal + 0x6, sumIfDecimal);
        sumIfDecimal = (m.a() & 0xF0) + (immediate_var & 0xF0) + m.ite(m.ugt(sumIfDecimal, 0x0F), 0x10, 0) + (sumIfDecimal & 0x0F);

        auto sumIfBinary = m.extend(immediate_var) + m.extend(m.a()) + m.extend(carry);

        auto sum = m.ite(m.cc_d(), sumIfDecimal, sumIfBinary);
        m.cc_v(0x80 == (0x80 & (m.lobyte(sum) ^ m.a()) & (m.lobyte(sum) ^ immediate_var)));
        sum = sum + m.ite(m.cc_d() && m.uge(sum, 0xA0), 0x60, 0);
        m.cc_c(m.hibyte(sum) != 0x00);
        m.a(set_sz(m.lobyte(sum)));
      } else {
        auto sum = m.extend(immediate_var) + m.extend(m.a()) + m.extend(carry);

        m.cc_v(0x80 == (0x80 & (m.lobyte(sum) ^ m.a()) & (m.lobyte(sum) ^ immediate_var)));
        m.cc_c(m.hibyte(sum) == 0x01);
        m.a(set_sz(m.lobyte(sum)));        m.a(set_sz(m.lobyte(sum)));

      }
      
      break;
      }
    case instruction_name::CMP:
      m.cc_c(m.uge(m.a(), immediate_var));
      m.cc_s(m.uge(m.a() - immediate_var, 0x80));
      m.cc_z(m.a() == immediate_var);
      break;
    case instruction_name::CPX:
      m.cc_c(m.uge(m.x(), immediate_var));
      m.cc_s(m.uge(m.x() - immediate_var, 0x80));
      m.cc_z(m.x() == immediate_var);
      break;
    case instruction_name::CPY:
      m.cc_c(m.uge(m.y(), immediate_var));
      m.cc_s(m.uge(m.y() - immediate_var, 0x80));
      m.cc_z(m.y() == immediate_var);
      break;
    case instruction_name::JMP:
      m.exit(absolute_var);
      break;
    case instruction_name::RTI: {
      byte_to_flags(pull());
      auto lo = pull();
      auto hi = pull();
      m.exit(m.from_bytes(hi, lo));
      break;
      }
    case instruction_name::RTS: {
      auto lo = pull();
      auto hi = pull();
      m.exit(m.from_bytes(hi, lo) + 1);
      break;
      }
    case instruction_name::BPL:
      m.exit_if(!m.cc_s(), absolute_var);
      break;
    case instruction_name::BMI:
      m.exit_if(m.cc_s(), absolute_var);
      break;
    case instruction_name::BVS:
      m.exit_if(m.cc_v(), absolute_var);
      break;
    case instruction_name::BVC:
      m.exit_if(!m.cc_v(), absolute_var);
      break;
    case instruction_name::BCC:
      m.exit_if(!m.cc_c(), absolute_var);
      break;
    case instruction_name::BCS:
      m.exit_if(m.cc_c(), absolute_var);
      break;
    case instruction_name::BEQ:
      m.exit_if(m.cc_z(), absolute_var);
      break;
    case instruction_name::BNE:
      m.exit_if(!m.cc_z(), absolute_var);
      break;
    case instruction_name::JSR:
      push(m.hibyte(m.pc() - 1));
      push(m.lobyte(m.pc() - 1));
      m.exit(absolute_var);
      break;
    case instruction_name::BRK:
      push(m.hibyte(m.pc() + 1));
      push(m.lobyte(m.pc() + 1));
      push(flags_to_byte() | 0x10);
      m.cc_i(true);
      m.exit(m.from_bytes(m.read(0xFFFF), m.read(0xFFFE)));
      break;
    case instruction_name::LAX:
      m.a(set_sz(immediate_var));
      m.x(m.a());
      break;
    case instruction_name::SAX:
      m.write(absolute_var, m.a() & m.x());
      break;
    case instruction_name::DCM:
      immediate_var = immediate_var - 1;
      m.write(absolute_var, set_sz(immediate_var));
      m.cc_c(m.uge(m.a(), immediate_var));
      m.cc_s(m.uge(m.a() - immediate_var, 0x80));
      m.cc_z(m.a() == immediate_var);
      break;
    case instruction_name::INS: {
      immediate_var = immediate_var + 1;
      m.write(absolute_var, set_sz(immediate_var));
      immediate_var = immediate_var ^ 0xFF;
      auto sum = m.extend(immediate_var) + m.extend(m.a()) + m.extend(m.ite(m.cc_c(), 0x01, 0x00));
      m.cc_v(0x80 == (0x80 & (m.lobyte(sum) ^ m.a()) & (m.lobyte(sum) ^ immediate_var)));
      m.cc_c(m.hibyte(sum) == 0x01);
      m.a(set_sz(m.lobyte(sum)));
      break;
      }
    case instruction_name::SLO:
      m.cc_c((immediate_var & 0x80) == 0x80);
      immediate_var = m.shl(immediate_var);
      m.write(absolute_var, set_sz(immediate_var));
      m.a(set_sz(m.a() | immediate_var));
      break;
    case instruction_name::RLA: {
      auto val = m.shl(immediate_var) | m.ite(m.cc_c(), 1, 0);
      m.cc_c((immediate_var & 0x80) == 0x80);
      m.write(absolute_var, set_sz(val));
      m.a(set_sz(val & m.a()));
      break;
      }
    case instruction_name::SRE:
      m.cc_c((immediate_var & 0x01) == 0x01);
      immediate_var = m.shr(immediate_var);
      m.write(absolute_var, set_sz(immediate_var));
      m.a(set_sz(m.a() ^ immediate_var));
      break;
    case instruction_name::RRA: {
      auto val = m.shr(immediate_var) | m.ite(m.cc_c(), 0x80, 0x00);
      m.cc_c((immediate_var & 0x01) == 0x01);
      m.write(absolute_var, set_sz(val));
      auto sum = m.extend(val) + m.extend(m.a()) + m.extend(m.ite(m.cc_c(), 0x01, 0x00));
      m.cc_v(0x80 == (0x80 & (m.lobyte(sum) ^ m.a()) & (m.lobyte(sum) ^ val)));
      m.cc_c(m.hibyte(sum) == 0x01);
      m.a(set_sz(m.lobyte(sum)));
      break;
      }
    case instruction_name::ALR: {
      auto val = m.shr(m.a() & immediate_var);
      m.cc_c((m.a() & immediate_var & 0x01) == 0x01);
      m.a(set_sz(val));
      break;
      }
    case instruction_name::ANC:
      m.a(set_sz(m.a() & immediate_var));
      m.cc_c(m.cc_s());
      break;
    case instruction_name::ARR: {
      m.a(set_sz(m.a() & immediate_var));
      auto val = m.shr(m.a()) | m.ite(m.cc_c(), 0x80, 0x00);
      m.cc_c((val & 0x40) == 0x40);
      auto bit6 = (val & 0x40) == 0x40;
      auto bit5 = (val & 0x20) == 0x20;
      m.cc_v(bit6 ^ bit5);
      m.a(set_sz(val));
      break;
      }
    case instruction_name::AXS: {
      auto xa = m.x() & m.a();
      m.x(set_sz(xa - immediate_var));
      m.cc_c(m.uge(xa, immediate_var));
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
      m.write(absolute_var, immediate_var & mask);
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
      m.write(absolute_var, immediate_var | (1 << bit));
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
      m.exit_if((immediate_var & (1 << bit)) == 0, absolute_var);
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
      m.exit_if((immediate_var & (1 << bit)) != 0, absolute_var);
      break;
      }
    case instruction_name::TRB: {
      m.cc_z((immediate_var & m.a()) == 0);
      m.write(absolute_var, immediate_var & ~m.a());
      break;
      }
    case instruction_name::TSB: {
      m.cc_z((immediate_var & m.a()) == 0);
      m.write(absolute_var, immediate_var | m.a());
      break;
      }
    case instruction_name::STP:
    case instruction_name::WAI:
      // TODO: What to do here? These shouldn't be included
      // in optimization anyway.
      break;
    }
  }
};

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
  std::cout << "Sizes: a:" << (sizeof(abstract_machine)) << std::endl << "c: " << (sizeof(concrete_machine)) << std::endl;

  try {
    char header[0x10];
    char *prg;
    char *chr;
    uint8_t memory[0x10000];

    prover_context prover_ctx;

    z3::context &ctx = prover_ctx.context;
    concrete_machine c_machine(0, memory);
    emulator<concrete_machine> c_emulator(c_machine);

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

    c_machine.pc(c_machine.from_bytes(memory[0xFFFD], memory[0xFFFC]));
    printf("pc:%04X\n", c_machine.pc());

    std::string status;

    uint64_t instructions = 0;
    while (instructions < 0x10000 || memory[0x6000] == 0x80) {
      instruction ins = decode(memory, c_machine.pc());

      instructions++;
      
      
      printf("%04X  ", c_machine.pc());

      int i = 0, size = instruction_size(ins.payload_type);
      for (; i < size; i++) {
        printf("%02X ", memory[c_machine.pc() + i]);
      }
      for (; i < 3; i++) {
        printf("   ");
      }

      printf(" A:%02X X:%02X Y:%02X P:%02X SP:%02X\n",
        c_machine.a(),
        c_machine.x(),
        c_machine.y(),
        c_emulator.flags_to_byte(),
        c_machine.sp());

      if (memory[0x6004] != 0) {
        std::string new_status((char *)(memory + 0x6004));
        if (new_status != status) {
          status = new_status;
          std::cout << new_status;
        }
      }

      {
        c_machine.reset_early_exit();
        c_machine._n_read = 0;
        c_machine._n_written = 0;
        // Create an abstract machine copying the current state
        abstract_machine a_machine(prover_ctx, c_machine);
        // Save the original memory state
        auto original_memory = a_machine.memory();
        emulator<abstract_machine> a_emulator(a_machine);
        // Run the instruction on both machines
        c_emulator.run(ins);
        a_emulator.run(ins);
        //a_machine.simplify();
        z3::solver solver(ctx);
        // Add assertions for the memory locations which were read.
        for (int i = 0; i < c_machine._n_read; i++) {
          auto equal = z3::select(original_memory, c_machine._addresses_read[i]) == c_machine._values_read[i];
          solver.add(equal);
        }

        // And for the memory locations which were written.
        for (int i = 0; i < c_machine._n_written; i++) {
          solver.add(a_machine.read(c_machine._addresses_written[i]) == c_machine._values_written[i]);
        }

        solver.add(a_machine.a() == c_machine.a());
        solver.add(a_machine.x() == c_machine.x());
        solver.add(a_machine.y() == c_machine.y());
        solver.add(a_machine.sp() == c_machine.sp());
        if (c_machine.cc_s()) { solver.add(a_machine.cc_s()); } else { solver.add(!a_machine.cc_s()); }
        if (c_machine.cc_v()) { solver.add(a_machine.cc_v()); } else { solver.add(!a_machine.cc_v()); }
        if (c_machine.cc_i()) { solver.add(a_machine.cc_i()); } else { solver.add(!a_machine.cc_i()); }
        if (c_machine.cc_d()) { solver.add(a_machine.cc_d()); } else { solver.add(!a_machine.cc_d()); }
        if (c_machine.cc_c()) { solver.add(a_machine.cc_c()); } else { solver.add(!a_machine.cc_c()); }
        if (c_machine.cc_z()) { solver.add(a_machine.cc_z()); } else { solver.add(!a_machine.cc_z()); }
        solver.add(a_machine.pc() == c_machine.pc());
        if (c_machine.exited_early()) { solver.add(a_machine.exited_early()); } else { solver.add(!a_machine.exited_early()); }

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

    concrete_machine c_machine(0, memory);
    c_machine.record_memory(false);
    emulator<concrete_machine> c_emulator(c_machine);

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
    printf("pc:%04X\n", c_machine.pc());

    std::string status;

    uint64_t instructions = 0;
    while (c_machine.pc() != 1) {
      instruction ins = decode(memory, c_machine.pc());

      instructions++;

      printf("%04X  ", c_machine.pc());
      int i = 0, size = instruction_size(ins.payload_type);
      for (; i < size; i++) {
        printf("%02X ", memory[c_machine.pc() + i]);
      }
      for (; i < 3; i++) {
        printf("   ");
      }
      printf(" A:%02X X:%02X Y:%02X P:%02X SP:%02X\n",
        c_machine.a(),
        c_machine.x(),
        c_machine.y(),
        c_emulator.flags_to_byte(),
        c_machine.sp());

      c_machine.reset_early_exit();
      c_emulator.run(ins);
    }

  } catch(z3::exception& e) {
    std::cout << e << std::endl;
  }
  return 0;
}

int main(int argc, char **argv) {
  sample();
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