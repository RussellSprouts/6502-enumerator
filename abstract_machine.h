
#pragma once

#include "z3++.h"

z3::expr mkArray(z3::context& c) {
  z3::sort byte = c.bv_sort(8);
  z3::sort word = c.bv_sort(16);
  z3::sort array = c.array_sort(word, byte);
  return c.constant("memory", array);
}

/**
 * This struct represents a 6502 processor which symbolically
 * executes its instructions. It uses the z3 API to provide
 * symbolic execution.
 */
struct abstract_machine {

  /**
   * Creates a new abstract_machine. Pass in the z3 context.
   * The initial state of the machine has everything as an
   * arbitrary unknown value. _earlyExit is set to assume the
   * control flow exits normally.
   */
  abstract_machine(z3::context& c) :
    _a(c.bv_const("a", 8)),
    _x(c.bv_const("x", 8)),
    _y(c.bv_const("y", 8)),
    _sp(c.bv_const("sp", 8)),
    _ccS(c.bool_const("ccS")),
    _ccV(c.bool_const("ccV")),
    _ccI(c.bool_const("ccI")),
    _ccD(c.bool_const("ccD")),
    _ccC(c.bool_const("ccC")),
    _ccZ(c.bool_const("ccZ")),
    absolute0(c.bv_const("absolute0", 16)),
    absolute1(c.bv_const("absolute1", 16)),
    absolute2(c.bv_const("absolute2", 16)),
    zp0(c.bv_const("zp0", 8)),
    zp1(c.bv_const("zp1", 8)),
    zp2(c.bv_const("zp2", 8)),
    zp3(c.bv_const("zp3", 8)),
    c0(c.bv_const("c0", 8)),
    c1(c.bv_const("c1", 8)),
    literal0(c, c.num_val(0, c.bv_sort(8))),
    literal1(c, c.num_val(1, c.bv_sort(8))),
    falsy(c.bool_val(false)),
    truthy(c.bool_val(true)),
    _earlyExit(c.num_val(0, c.bv_sort(17))),
    _memory(mkArray(c)),
    c(c) {}

  void simplify() {
    _a = _a.simplify();
    _x = _x.simplify();
    _y = _y.simplify();
    _sp = _sp.simplify();
    _ccC = _ccC.simplify();
    _ccV = _ccV.simplify();
    _ccI = _ccI.simplify();
    _ccS = _ccS.simplify();
    _ccD = _ccD.simplify();
    _ccZ = _ccZ.simplify();
    _memory = _memory.simplify();
    _earlyExit = _earlyExit.simplify();
  }

  /**
   * Runs a set of three instructions. (Some may do nothing).
   */
  void instruction(instruction3 ops) {
    opcode zero { (Operations)0, (AddrMode)0 };

    if (std::get<0>(ops) != zero) { instruction(std::get<0>(ops)); }
    if (std::get<1>(ops) != zero) { instruction(std::get<1>(ops)); }
    if (std::get<2>(ops) != zero) { instruction(std::get<2>(ops)); }
  }

  /**
   * Runs a single instruction.
   */
  void instruction(opcode op) {
    emulator<abstract_machine> emu;
    emu.instruction(*this, op.op, op.mode);
  } 

  /**
   * Writes the val to the addr given. Returns the current memory array.
   */
  z3::expr write(z3::expr const & addr, z3::expr const & val) {
    return _memory = E(z3::store(_memory, addr, val),_memory);
  }

  /**
   * Reads the value at addr and returns it as an 8-bit bitvector.
   */
  z3::expr read(z3::expr const & addr) {
    return z3::select(_memory, addr);
  }

  /**
   * Takes an 8-bit bitvector and zero-extends it to a 16-bit bv.
   */
  z3::expr extend(z3::expr const & val) {
    Z3_ast r = Z3_mk_zero_ext(c, 8, val);
    return z3::expr(val.ctx(), r);
  }

  /**
   * If-then-else. If c, then t, else e.
   */
  z3::expr ite(z3::expr const & c, z3::expr const & t, z3::expr const & e) const {
    z3::check_context(c, t); z3::check_context(c, e);
    assert(c.is_bool());
    Z3_ast r = Z3_mk_ite(c.ctx(), c, t, e);
    c.check_error();
    return z3::expr(c.ctx(), r);
  }

  /**
   * If-then-else with 8-bit literals instead of `z3::expr`s.
   */
  z3::expr ite(z3::expr const & cond, uint8_t t, uint8_t e) const {
    return ite(cond, c.num_val(t, _a.get_sort()), c.num_val(e, _a.get_sort()));
  }

  /**
   * Shifts the input left by 1.
   */
  z3::expr shl(z3::expr const & val) const {
    return z3::to_expr(val.ctx(), Z3_mk_bvshl(val.ctx(), val, val.ctx().num_val(1, val.get_sort())));
  }

  /**
   * Shifts the input right by 1 (logical shift right).
   */
  z3::expr shr(z3::expr const & val) const {
    return z3::to_expr(val.ctx(), Z3_mk_bvlshr(val.ctx(), val, val.ctx().num_val(1, val.get_sort())));
  }

  /**
   * Extracts the low byte of the 16-bit bv.
   */
  z3::expr lobyte(z3::expr const & val) const {
    return val.extract(7, 0);
  }

  /**
   * Extracts the high byte of the 16-bit bv.
   */
  z3::expr hibyte(z3::expr const & val) const {
    return val.extract(15, 8);
  }

  /**
   * Unsigned greater-than or equal.
   */
  z3::expr uge(z3::expr const & first, z3::expr const & second) {
    return z3::uge(first, second);
  }

  /**
   * Unsigned greater-than or equal, with a constant.
   */
  z3::expr uge(z3::expr const & first, uint8_t second) {
    return z3::uge(first, second);
  }

  /**
   * Causes the machine to exit early with an rts.
   */
  void rts() {
    earlyExit(c.num_val(0x00001, _earlyExit.get_sort()));
  }
  
  /**
   * Causes the machine to exit early with an rti.
   */
  void rti() {
    earlyExit(c.num_val(0x00002, _earlyExit.get_sort()));
  }

  /**
   * Causes the machine to exit early with a jump to the target.
   */
  void jmp(z3::expr const & target) {
    Z3_ast r = Z3_mk_zero_ext(c, 1, target);
    z3::expr target2(c, r);
    earlyExit(target2 | 0x10000);
  }

  /**
   * Causes the machine to exit early with a branch if cond is true.
   */
  void branch(z3::expr cond, z3::expr const & target) {
    Z3_ast r = Z3_mk_zero_ext(c, 1, target);
    z3::expr target2(c, r);
    earlyExit(ite(cond, target2, _earlyExit));
  }

  /**
   * Sets the sign and zero flags based on the sign and value of
   * val, then returns val.
   */
  z3::expr setSZ(z3::expr const & val) {
    ccS(z3::uge(val, 0x80));
    ccZ(val == 0);
    return val;
  }

  z3::context& c;

  // The boolean values, as z3 exprs.
  const z3::expr falsy;
  const z3::expr truthy;

  // The possible operands.
  const z3::expr absolute0;
  const z3::expr absolute1;
  const z3::expr absolute2;
  const z3::expr zp0;
  const z3::expr zp1;
  const z3::expr zp2;
  const z3::expr zp3;
  const z3::expr c0;
  const z3::expr c1;
  const z3::expr literal0;
  const z3::expr literal1;

  /**
   * If the machine exits early because of a branch, rts, jmp, etc.,
   * then we need to freeze the state of the machine. To accomplish this,
   * all assignments to the machine state should be guarded with e.g.
   * 
   * ```
   * _a = E(new_val, _a);
   * ```
   *
   * Returns the new_val if the machine hasn't exited early,
   * otherwise the same value as the original.
   */
  z3::expr E(z3::expr new_val, z3::expr same) const {
    return ite(0x0 == _earlyExit, new_val, same);
  }

  // The internal state of the machine.
  z3::expr _a; z3::expr a(z3::expr const & val) { return _a = E(val, _a); }
  z3::expr _x; z3::expr x(z3::expr const & val) { return _x = E(val, _x); }
  z3::expr _y; z3::expr y(z3::expr const & val) { return _y = E(val, _y); }
  z3::expr _sp; z3::expr sp(z3::expr const & val) { return _sp = E(val, _sp); }
  z3::expr _ccS; z3::expr ccS(z3::expr const & val) { return _ccS = E(val, _ccS); }
  z3::expr _ccV; z3::expr ccV(z3::expr const & val) { return _ccV = E(val, _ccV); }
  z3::expr _ccI; z3::expr ccI(z3::expr const & val) { return _ccI = E(val, _ccI); }
  z3::expr _ccD; z3::expr ccD(z3::expr const & val) { return _ccD = E(val, _ccD); }
  z3::expr _ccC; z3::expr ccC(z3::expr const & val) { return _ccC = E(val, _ccC); }
  z3::expr _ccZ; z3::expr ccZ(z3::expr const & val) { return _ccZ = E(val, _ccZ); }

  z3::expr _memory;

  /**
   * _earlyExit can be 0 for normal flow through the end of the
   * instructions, 1 for exit by rts, 2 for exit by rti, or
   * (0x10000 | addr) for an exit by jump or branch to an address.
   */
  z3::expr _earlyExit; z3::expr earlyExit(z3::expr const & val) { return _earlyExit = E(val, _earlyExit); }
};
