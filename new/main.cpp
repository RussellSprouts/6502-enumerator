#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>
#include "z3++.h"

enum struct instruction_name: uint8_t {
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

  LSRA = 58,
  ASLA = 59,
  ROLA = 60,
  RORA = 61,

  INCA = 62,
  DECA = 63,
  STZ = 64,
  PHX = 65,
  PHY = 66,
  PLX = 67,
  PLY = 68,
  LAX = 69,
  SAX = 70,
  DCM = 71,
  INS = 72,
  SLO = 73,
  RLA = 74,
  SRE = 75,
  RRA = 76,
  ALR = 77,
  ANC = 78,
  AXS = 79,
  ARR = 80,

  UNKNOWN = 0xFF,
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
    _cc_s(false),
    _cc_v(false),
    _cc_i(true),
    _cc_d(false),
    _cc_c(false),
    _cc_z(false),
    _addresses_read{0},
    _values_read{0},
    _n_read(0)
    {}

  typedef uint8_t expr8;
  typedef uint16_t expr16;

  uint64_t _seed;

  uint16_t _pc;
  uint8_t *_memory;
  uint8_t _a;
  uint8_t _x;
  uint8_t _y;
  uint8_t _sp;
  bool _cc_s;
  bool _cc_v;
  bool _cc_i;
  bool _cc_d;
  bool _cc_c;
  bool _cc_z;
  bool _exited_early;
  uint16_t _addresses_read[8];
  uint8_t _values_read[8];
  uint8_t _n_read;
  uint16_t _addresses_written[8];
  uint8_t _values_written[8];
  uint8_t _n_written;

  uint8_t immediate(uint16_t val, bool is_constant) {
    return val;
  }

  uint16_t absolute(uint16_t val, bool is_constant) {
    return val;
  }

  uint8_t zp(uint16_t val, bool is_constant) {
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
  bool cc_s(bool val) { return _cc_s = E(val, _cc_s); }
  bool cc_v(bool val) { return _cc_v = E(val, _cc_v); }
  bool cc_i(bool val) { return _cc_i = E(val, _cc_i); }
  bool cc_d(bool val) { return _cc_d = E(val, _cc_d); }
  bool cc_c(bool val) { return _cc_c = E(val, _cc_c); }
  bool cc_z(bool val) { return _cc_z = E(val, _cc_z); }

  uint8_t read(uint16_t address) {
    assert(_n_read < 8);
    _addresses_read[_n_read] = address;
    _values_read[_n_read] = _memory[address];
    _n_read++;
    return _memory[address];
  }

  void write(uint16_t address, uint8_t val) {
    assert(_n_written < 8);
    _addresses_written[_n_written] = address;
    _values_written[_n_written] = val;
    _n_written++;
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

  uint16_t from_bytes(uint8_t hi, uint8_t lo) const {
    return (hi << 8) | lo;
  }

  void reset_early_exit() {
    _exited_early = false;
  }

  const bool DECIMAL_ENABLED = false;
  const bool HAS_JUMP_INDIRECT_BUG = true;

  private:
  inline uint16_t E(uint16_t val, uint16_t orig) {
    return _exited_early ? orig : val;
  }
};

struct abstract_machine {
  explicit abstract_machine(z3::context& c) : 
  ctx(c),
  _exited_early(c.bool_val(false)),
  _memory(c.constant("memory", c.array_sort(c.bv_sort(16), c.bv_sort(8)))),
  _a(c.bv_const("a", 8)),
  _x(c.bv_const("x", 8)),
  _y(c.bv_const("y", 8)),
  _sp(c.bv_const("sp", 8)),
  _cc_s(c.bool_const("cc_s")),
  _cc_v(c.bool_const("cc_v")),
  _cc_i(c.bool_const("cc_i")),
  _cc_d(c.bool_const("cc_d")),
  _cc_c(c.bool_const("cc_c")),
  _cc_z(c.bool_const("cc_z")),
  _pc(c.bv_const("pc", 16)) {
    std::string absoluteName("absolute");
    for (int i = 0; i < 16; i++) {
      absoluteVars.push_back(c.bv_const((absoluteName + std::to_string(i)).c_str(), 16));
    }
    
    std::string immediateName("immediate");
    for (int i = 0; i < 16; i++) {
      immediateVars.push_back(c.bv_const((immediateName + std::to_string(i)).c_str(), 8));
    }
    
    std::string zpName("zp");
    for (int i = 0; i < 16; i++) {
      zpVars.push_back(c.bv_const((zpName + std::to_string(i)).c_str(), 8));
    }
  }

  explicit abstract_machine(z3::context& c, concrete_machine &other): abstract_machine(c) {
    _a = ctx.bv_val(other._a, 8);
    _x = ctx.bv_val(other._x, 8);
    _y = ctx.bv_val(other._y, 8);
    _sp = ctx.bv_val(other._sp, 8);
    _cc_s = ctx.bool_val(other._cc_s);
    _cc_v = ctx.bool_val(other._cc_v);
    _cc_i = ctx.bool_val(other._cc_i);
    _cc_d = ctx.bool_val(other._cc_d);
    _cc_c = ctx.bool_val(other._cc_c);
    _cc_z = ctx.bool_val(other._cc_z);

    _exited_early = ctx.bool_val(other._exited_early);
    _pc = ctx.bv_val(other._pc, 16);
  }

  z3::expr absolute(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return ctx.bv_val(payload, 16);
    } else {
      return absoluteVars.at(payload);
    }
  }

  z3::expr immediate(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return ctx.bv_val(payload & 0xFF, 8);
    } else {
      return immediateVars.at(payload);
    }
  }

  z3::expr zp(uint16_t payload, bool is_constant) {
    if (is_constant) {
      return ctx.bv_val(payload & 0xFF, 8);
    } else {
      return zpVars.at(payload);
    }
  }

  const bool DECIMAL_ENABLED = false;
  const bool HAS_JUMP_INDIRECT_BUG = true;

  typedef z3::expr expr8;
  typedef z3::expr expr16;

  void exit(z3::expr const address) {
    exit_if(ctx.bool_val(true), address);
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
    return _memory = E(z3::store(_memory, addr, val), _memory);
  }

  /**
   * Writes the val to the addr given. Returns the current memory array.
   */
  z3::expr write(z3::expr addr, uint8_t val) {
    return write(addr, ctx.bv_val(val, 8));
  }

  z3::expr write(uint16_t addr, uint8_t val) {
    return write(ctx.bv_val(addr, 16), ctx.bv_val(val, 8));
  }

  /**
   * Reads the value at addr and returns it as an 8-bit bitvector.
   */
  z3::expr read(z3::expr addr) {
    if (addr.get_sort().bv_size() == 8) {
      addr = extend(addr);
    }
    return z3::select(_memory, addr);
  }

  z3::expr read(uint16_t addr) {
    return read(ctx.bv_val(addr, 16));
  }

  /**
   * Takes an 8-bit bitvector and zero-extends it to a 16-bit bv.
   */
  z3::expr extend(z3::expr const val) const {
    Z3_ast r = Z3_mk_zero_ext(ctx, 8, val);
    return z3::expr(ctx, r);
  }

  /**
   * If-then-else. If c, then t, else e.
   */
  z3::expr ite(z3::expr const c, z3::expr const t, z3::expr const e) const {
    z3::check_context(c, t); z3::check_context(c, e);
    assert(c.is_bool());
    Z3_ast r = Z3_mk_ite(ctx, c, t, e);
    c.check_error();
    return z3::expr(ctx, r);
  }

  /**
   * If-then-else with 8-bit literals instead of `z3::expr`s.
   */
  z3::expr ite(z3::expr const cond, uint8_t t, uint8_t e) const {
    return ite(cond, ctx.num_val(t, _a.get_sort()), ctx.num_val(e, _a.get_sort()));
  }

  /**
   * Shifts the input left by 1.
   */
  z3::expr shl(z3::expr const val) const {
    return z3::to_expr(ctx, Z3_mk_bvshl(ctx, val, ctx.num_val(1, val.get_sort())));
  }

  /**
   * Shifts the input right by 1 (logical shift right).
   */
  z3::expr shr(z3::expr const val) const {
    return z3::to_expr(ctx, Z3_mk_bvlshr(ctx, val, ctx.num_val(1, val.get_sort())));
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

  z3::expr from_bytes(z3::expr hi, z3::expr lo) const {
    return z3::concat(hi, lo);
  }

  z3::expr from_bytes(uint8_t hi, z3::expr lo) const {
    return from_bytes(ctx.bv_val(hi, 8), lo);
  }

  z3::expr a(z3::expr const val) { return _a = E(val, _a); }
  z3::expr a(uint8_t val) { return a(ctx.bv_val(val, 8)); }
  z3::expr x(z3::expr const val) { return _x = E(val, _x); }
  z3::expr x(uint8_t val) { return x(ctx.bv_val(val, 8)); }
  z3::expr y(z3::expr const val) { return _y = E(val, _y); }
  z3::expr y(uint8_t val) { return y(ctx.bv_val(val, 8)); }
  z3::expr sp(z3::expr const val) { return _sp = E(val, _sp); }
  z3::expr sp(uint8_t val) { return sp(ctx.bv_val(val, 8)); }
  z3::expr cc_s(z3::expr const val) { return _cc_s = E(val, _cc_s); }
  z3::expr cc_s(bool val) { return cc_s(ctx.bool_val(val)); }
  z3::expr cc_v(z3::expr const val) { return _cc_v = E(val, _cc_v); }
  z3::expr cc_v(bool val) { return cc_v(ctx.bool_val(val)); }
  z3::expr cc_i(z3::expr const val) { return _cc_i = E(val, _cc_i); }
  z3::expr cc_i(bool val) { return cc_i(ctx.bool_val(val)); }
  z3::expr cc_d(z3::expr const val) { return _cc_d = E(val, _cc_d); }
  z3::expr cc_d(bool val) { return cc_d(ctx.bool_val(val)); }
  z3::expr cc_c(z3::expr const val) { return _cc_c = E(val, _cc_c); }
  z3::expr cc_c(bool val) { return cc_c(ctx.bool_val(val)); }
  z3::expr cc_z(z3::expr const val) { return _cc_z = E(val, _cc_z); }
  z3::expr cc_z(bool val) { return cc_z(ctx.bool_val(val)); }
  z3::expr pc(z3::expr const val) { return _pc = E(val, _pc); }
  
  void reset_early_exit() {
    _exited_early = ctx.bool_val(false);
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

  std::vector<z3::expr> absoluteVars;
  std::vector<z3::expr> immediateVars;
  std::vector<z3::expr> zpVars;

  z3::context &ctx;

  z3::expr _exited_early;
  z3::expr _memory;

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
    m.sp(m._sp + 1);
    return m.read(m.from_bytes(0x1, m._sp));
  }

  typename machine::expr8 push(typename machine::expr8 val) {
    auto addr = m.from_bytes(0x1, m._sp);
    m.write(addr, val);
    m.sp(m._sp - 1);
    return val;
  }

  typename machine::expr8 flags_to_byte() const {
    return m.ite(m._cc_s, 0x80, 0x00)
         | m.ite(m._cc_v, 0x40, 0x00)
         | 0x20
         | m.ite(m._cc_d, 0x08, 0x00)
         | m.ite(m._cc_i, 0x04, 0x00)
         | m.ite(m._cc_z, 0x02, 0x00)
         | m.ite(m._cc_c, 0x01, 0x00);
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

    m.pc(m._pc + instruction_size(ins.payload_type));

    bool is_constant = static_cast<uint8_t>(ins.payload_type & instruction_payload_type::CONST_FLAG);

    // Load possible values from the machine
    auto absolute_payload = m.absolute(ins.payload, is_constant);
    auto zp_payload = m.zp(ins.payload, is_constant);
    auto immediate_payload = m.immediate(ins.payload, is_constant);

    // The address the instruction is working on
    auto absolute_var = absolute_payload;
    // The value the instruction is working on
    auto immediate_var = immediate_payload;

    switch(ins.payload_type & ~instruction_payload_type::CONST_FLAG) {
    case instruction_payload_type::NONE:
      // nothing to do
      break;
    case instruction_payload_type::RELATIVE:
    case instruction_payload_type::ABSOLUTE:
      absolute_var = absolute_payload;
      break;
    case instruction_payload_type::ABSOLUTE_X:
      absolute_var = absolute_payload + m.extend(m._x);
      break;
    case instruction_payload_type::ABSOLUTE_Y:
      absolute_var = absolute_payload + m.extend(m._y);
      break;
    case instruction_payload_type::X_INDIRECT:
      // lda (addr, x)
      absolute_var = m.from_bytes(m.read((zp_payload + m._x + 1) & 0xFF),
                                m.read((zp_payload + m._x) & 0xFF));
      break;
    case instruction_payload_type::INDIRECT_Y:
      // lda (addr), y
      absolute_var = m.from_bytes(m.read((zp_payload + 1) & 0xFF),
                                m.read(zp_payload)) + m.extend(m._y);
      break;
    case instruction_payload_type::ZERO_PAGE:
      absolute_var = m.extend(zp_payload);
      break;
    case instruction_payload_type::ZERO_PAGE_X:
      absolute_var = m.extend(zp_payload + m._x);
      break;
    case instruction_payload_type::ZERO_PAGE_Y:
      absolute_var = m.extend(zp_payload + m._y);
      break;
    case instruction_payload_type::IMMEDIATE:
      immediate_var = immediate_payload;
      break;
    case instruction_payload_type::INDIRECT_ABSOLUTE: {
      auto hi = m.HAS_JUMP_INDIRECT_BUG
        ? (absolute_var & 0xFF00) | ((absolute_var + 1) & 0xFF)
        : absolute_var + 1;
      absolute_var = m.from_bytes(m.read(hi), m.read(absolute_var));
      break;
      }
    case instruction_payload_type::X_INDIRECT_ABSOLUTE:
      absolute_var = m.from_bytes(
        m.read(absolute_payload + m.extend(m._x) + 1),
        m.read(absolute_payload + m.extend(m._x)));
      break;
    case instruction_payload_type::ZERO_PAGE_INDIRECT:
      absolute_var = m.from_bytes(m.read((zp_payload + 1) & 0xFF),
                                m.read(zp_payload));
      break;
    default:
      assert(false);
    }

    // If we aren't using the immediate operand, then read the address we found.
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
      m.a(set_sz(m._a & immediate_var));
      break;
    case instruction_name::ORA:
      m.a(set_sz(m._a | immediate_var));
      break;
    case instruction_name::EOR: 
      m.a(set_sz(m._a ^ immediate_var));
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
      m.a(set_sz(m._x));
      break;
    case instruction_name::TAX:
      m.x(set_sz(m._a));
      break;
    case instruction_name::TYA:
      m.a(set_sz(m._y));
      break;
    case instruction_name::TAY:
      m.y(set_sz(m._a));
      break;
    case instruction_name::INX:
      m.x(set_sz(m._x + 1));
      break;
    case instruction_name::INY:
      m.y(set_sz(m._y + 1));
      break;
    case instruction_name::DEX:
      m.x(set_sz(m._x - 1));
      break;
    case instruction_name::DEY:
      m.y(set_sz(m._y - 1));
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
      m.x(set_sz(m._sp));
      break;
    case instruction_name::TXS:
      m.sp(m._x);
      break;
    case instruction_name::NOP:
      // nap.
      break;
    case instruction_name::INC:
      m.write(absolute_var, set_sz(immediate_var + 1));
      break;
    case instruction_name::INCA:
      m.a(set_sz(m._a + 1));
      break;
    case instruction_name::DEC:
      m.write(absolute_var, set_sz(immediate_var - 1));
      break;
    case instruction_name::DECA:
      m.a(set_sz(m._a - 1));
    case instruction_name::BIT:
      m.cc_z((immediate_var & m._a) == 0);
      m.cc_s((immediate_var & 0x80) == 0x80);
      m.cc_v((immediate_var & 0x40) == 0x40);
      break;
    case instruction_name::ASLA:
      m.cc_c((m._a & 0x80) == 0x80);
      m.a(set_sz(m.shl(m._a)));
      break;
    case instruction_name::ASL:
      m.cc_c((immediate_var & 0x80) == 0x80);
      m.write(absolute_var, set_sz(m.shl(immediate_var)));
      break;
    case instruction_name::ROLA: {
      auto val = m.shl(m._a) | m.ite(m._cc_c, 1, 0);
      m.cc_c((m._a & 0x80) == 0x80);
      m.a(set_sz(val));
      break;
      }
    case instruction_name::ROL: {
      auto val = m.shl(immediate_var) | m.ite(m._cc_c, 1, 0);
      m.cc_c((immediate_var & 0x80) == 0x80);
      m.write(absolute_var, set_sz(val));
      break;
      }
    case instruction_name::LSRA:
      m.cc_c((m._a & 0x01) == 0x01);
      m.a(set_sz(m.shr(m._a)));
      break;
    case instruction_name::LSR:
      m.cc_c((immediate_var & 0x01) == 0x01);
      m.write(absolute_var, set_sz(m.shr(immediate_var)));
      break;
    case instruction_name::RORA: {
      auto val = m.shr(m._a) | m.ite(m._cc_c, 0x80, 0x00);
      m.cc_c((m._a & 0x01) == 0x01);
      m.a(set_sz(val));
      break;
      }
    case instruction_name::ROR: {
      auto val = m.shr(immediate_var) | m.ite(m._cc_c, 0x80, 0x00);
      m.cc_c((immediate_var & 0x01) == 0x01);
      m.write(absolute_var, set_sz(val));
      break;
      }
    case instruction_name::STZ:
      m.write(absolute_var, 0);
    case instruction_name::STA:
      m.write(absolute_var, m._a);
      break;
    case instruction_name::STX:
      m.write(absolute_var, m._x);
      break;
    case instruction_name::STY:
      m.write(absolute_var, m._y);
      break;
    case instruction_name::PLA:
      m.a(set_sz(pull()));
      break;
    case instruction_name::PHA:
      push(m._a);
      break;
    case instruction_name::PLX:
      m.x(set_sz(pull()));
      break;
    case instruction_name::PHX:
      push(m._x);
      break;
    case instruction_name::PLY:
      m.y(set_sz(pull()));
      break;
    case instruction_name::PHY:
      push(m._y);
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
      auto sum = m.extend(immediate_var) + m.extend(m._a) + m.extend(m.ite(m._cc_c, 0x01, 0x00));
      m.cc_v(0x80 == (0x80 & (m.lobyte(sum) ^ m._a) & (m.lobyte(sum) ^ immediate_var)));
      m.cc_c(m.hibyte(sum) == 0x01);
      m.a(set_sz(m.lobyte(sum)));
      break;
      }
    case instruction_name::CMP:
      m.cc_c(m.uge(m._a, immediate_var));
      m.cc_s(m.uge(m._a - immediate_var, 0x80));
      m.cc_z(m._a == immediate_var);
      break;
    case instruction_name::CPX:
      m.cc_c(m.uge(m._x, immediate_var));
      m.cc_s(m.uge(m._x - immediate_var, 0x80));
      m.cc_z(m._x == immediate_var);
      break;
    case instruction_name::CPY:
      m.cc_c(m.uge(m._y, immediate_var));
      m.cc_s(m.uge(m._y - immediate_var, 0x80));
      m.cc_z(m._y == immediate_var);
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
      m.exit_if(!m._cc_s, absolute_var);
      break;
    case instruction_name::BMI:
      m.exit_if(m._cc_s, absolute_var);
      break;
    case instruction_name::BVS:
      m.exit_if(m._cc_v, absolute_var);
      break;
    case instruction_name::BVC:
      m.exit_if(!m._cc_v, absolute_var);
      break;
    case instruction_name::BCC:
      m.exit_if(!m._cc_c, absolute_var);
      break;
    case instruction_name::BCS:
      m.exit_if(m._cc_c, absolute_var);
      break;
    case instruction_name::BEQ:
      m.exit_if(m._cc_z, absolute_var);
      break;
    case instruction_name::BNE:
      m.exit_if(!m._cc_z, absolute_var);
      break;
    case instruction_name::JSR:
      push(m.hibyte(m._pc - 1));
      push(m.lobyte(m._pc - 1));
      m.exit(absolute_var);
      break;
    case instruction_name::BRK:
      push(m.hibyte(m._pc + 1));
      push(m.lobyte(m._pc + 1));
      push(flags_to_byte() | 0x10);
      m.cc_i(true);
      m.exit(m.from_bytes(m.read(0xFFFF), m.read(0xFFFE)));
      break;
    case instruction_name::LAX:
      m.a(set_sz(immediate_var));
      m.x(m._a);
      break;
    case instruction_name::SAX:
      m.write(absolute_var, m._a & m._x);
      break;
    case instruction_name::DCM:
      immediate_var = immediate_var - 1;
      m.write(absolute_var, set_sz(immediate_var));
      m.cc_c(m.uge(m._a, immediate_var));
      m.cc_s(m.uge(m._a - immediate_var, 0x80));
      m.cc_z(m._a == immediate_var);
      break;
    case instruction_name::INS: {
      immediate_var = immediate_var + 1;
      m.write(absolute_var, set_sz(immediate_var));
      immediate_var = immediate_var ^ 0xFF;
      auto sum = m.extend(immediate_var) + m.extend(m._a) + m.extend(m.ite(m._cc_c, 0x01, 0x00));
      m.cc_v(0x80 == (0x80 & (m.lobyte(sum) ^ m._a) & (m.lobyte(sum) ^ immediate_var)));
      m.cc_c(m.hibyte(sum) == 0x01);
      m.a(set_sz(m.lobyte(sum)));
      break;
      }
    case instruction_name::SLO:
      m.cc_c((immediate_var & 0x80) == 0x80);
      immediate_var = m.shl(immediate_var);
      m.write(absolute_var, set_sz(immediate_var));
      m.a(set_sz(m._a | immediate_var));
      break;
    case instruction_name::RLA: {
      auto val = m.shl(immediate_var) | m.ite(m._cc_c, 1, 0);
      // printf("vals: %02X %02X %04X\n", immediate_var, val, absolute_var);
      m.cc_c((immediate_var & 0x80) == 0x80);
      m.write(absolute_var, set_sz(val));
      m.a(set_sz(val & m._a));
      break;
      }
    case instruction_name::SRE:
      m.cc_c((immediate_var & 0x01) == 0x01);
      immediate_var = m.shr(immediate_var);
      m.write(absolute_var, set_sz(immediate_var));
      m.a(set_sz(m._a ^ immediate_var));
      break;
    case instruction_name::RRA: {
      auto val = m.shr(immediate_var) | m.ite(m._cc_c, 0x80, 0x00);
      m.cc_c((immediate_var & 0x01) == 0x01);
      m.write(absolute_var, set_sz(val));
      auto sum = m.extend(val) + m.extend(m._a) + m.extend(m.ite(m._cc_c, 0x01, 0x00));
      m.cc_v(0x80 == (0x80 & (m.lobyte(sum) ^ m._a) & (m.lobyte(sum) ^ val)));
      m.cc_c(m.hibyte(sum) == 0x01);
      m.a(set_sz(m.lobyte(sum)));
      break;
      }
    case instruction_name::ALR: {
      auto val = m.shr(m._a & immediate_var);
      m.cc_c((m._a & immediate_var & 0x01) == 0x01);
      m.a(set_sz(val));
      break;
      }
    case instruction_name::ANC:
      m.a(set_sz(m._a & immediate_var));
      m.cc_c(m._cc_s);
      break;
    case instruction_name::ARR: {
      m.a(set_sz(m._a & immediate_var));
      auto val = m.shr(m._a) | m.ite(m._cc_c, 0x80, 0x00);
      m.cc_c((val & 0x40) == 0x40);
      auto bit6 = (val & 0x40) == 0x40;
      auto bit5 = (val & 0x20) == 0x20;
      m.cc_v(bit6 ^ bit5);
      m.a(set_sz(val));
      break;
      }
    case instruction_name::AXS: {
      auto xa = m._x & m._a;
      m.x(set_sz(xa - immediate_var));
      m.cc_c(m.uge(xa, immediate_var));
      break;
      }
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
  uint8_t delta = memory[ip+1];
  if (delta >= 0x80) {
    return ip + 2 - 0x100 + delta;
  } else {
    return ip + 2 + delta;
  }
  return static_cast<int8_t>(memory[ip+1]);
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
    case 0x0A: DR(ASLA, NONE, 0);
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
    case 0x2A: DR(ROLA, NONE, 0);
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
    case 0x4A: DR(LSRA, NONE, 0);
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
    case 0x6A: DR(RORA, NONE, 0);
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

int main(int argc, char **argv) {
    try {
      char header[0x10];
      char *prg;
      char *chr;
      uint8_t memory[0x10000];

      z3::context ctx;
      concrete_machine c_machine(0, memory);
      emulator<concrete_machine> c_emulator(c_machine);

      std::ifstream input(argv[1], std::ios::in | std::ios::binary);
      printf("Loading file %s\n", argv[1]);
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
      printf("pc:%04X\n", c_machine._pc);

      std::string status;

      uint64_t instructions = 0;
      while (instructions < 0x10000 || memory[0x6000] == 0x80) {
        instruction ins = decode(memory, c_machine._pc);

        instructions++;
        
        
        printf("%04X  ", c_machine._pc);

        int i = 0, size = instruction_size(ins.payload_type);
        for (; i < size; i++) {
          printf("%02X ", memory[c_machine._pc + i]);
        }
        for (; i < 3; i++) {
          printf("   ");
        }

        printf(" A:%02X X:%02X Y:%02X P:%02X SP:%02X\n",
          c_machine._a,
          c_machine._x,
          c_machine._y,
          c_emulator.flags_to_byte(),
          c_machine._sp);

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
          abstract_machine a_machine(ctx, c_machine);
          // Save the original memory state
          auto original_memory = a_machine._memory;
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

          for (int i = 0; i < c_machine._n_written; i++) {
            auto equal = z3::select(a_machine._memory, c_machine._addresses_written[i]) == c_machine._values_written[i];
            solver.add(equal);
          }

          solver.add(a_machine._a == c_machine._a);
          solver.add(a_machine._x == c_machine._x);
          solver.add(a_machine._y == c_machine._y);
          solver.add(a_machine._sp == c_machine._sp);
          if (c_machine._cc_s) { solver.add(a_machine._cc_s); } else { solver.add(!a_machine._cc_s); }
          if (c_machine._cc_v) { solver.add(a_machine._cc_v); } else { solver.add(!a_machine._cc_v); }
          if (c_machine._cc_i) { solver.add(a_machine._cc_i); } else { solver.add(!a_machine._cc_i); }
          if (c_machine._cc_d) { solver.add(a_machine._cc_d); } else { solver.add(!a_machine._cc_d); }
          if (c_machine._cc_c) { solver.add(a_machine._cc_c); } else { solver.add(!a_machine._cc_c); }
          if (c_machine._cc_z) { solver.add(a_machine._cc_z); } else { solver.add(!a_machine._cc_z); }
          solver.add(a_machine._pc == c_machine._pc);
          if (c_machine._exited_early) { solver.add(a_machine._exited_early); } else { solver.add(!a_machine._exited_early); }

          auto result = solver.check();
          if (result == z3::unsat) {
            std::cout << solver << std::endl;
            return 1;
          }
        }
      }

      exit(memory[0x6000]);

    } catch(z3::exception e) {
      std::cout << e << std::endl;
    }

    return 0;
}

int runTests() {
  return 0;
}