
#pragma once

#include "stdint.h"

static const int NUM_ADDRESSES = 16;

/**
 * random_machine represents a 6502 processor with a random
 * initial state, determined by the seed.
 */
struct random_machine {
  static const uint8_t literal0 = 0;
  static const uint8_t literal1 = 1;
  static const bool falsy = false;
  static const bool truthy = true;

  random_machine(uint32_t _seed) {
    seed = _seed;
    _a = fnv(113);
    _x = fnv(114);
    _y = fnv(115);
    _sp = fnv(116);
    _ccS = (fnv(117)) > 0x80000000;
    _ccV = (fnv(118)) > 0x80000000;
    _ccI = (fnv(119)) > 0x80000000;
    _ccD = (fnv(120)) > 0x80000000;
    _ccC = (fnv(121)) > 0x80000000;
    _ccZ = (fnv(122)) > 0x80000000;
    absolute0 = fnv(123);
    absolute1 = fnv(124);
    absolute2 = fnv(125);
    zp0 = fnv(126);
    zp1 = fnv(127);
    zp2 = fnv(128);
    zp3 = fnv(129);
    c0 = fnv(130);
    c1 = fnv(131);
  }

  void instruction(instruction_seq ops) {
    for (int i = 0; i < instruction_seq::max_length; i++) {
      if (ops.ops[i] != opcode::zero) { instruction(ops.ops[i]); }
      else { break; }
    }
  }

  void instruction(opcode op) {
    emulator<random_machine> emu;
    emu.instruction(*this, op.op, op.mode);
  } 

  uint32_t seed;
  uint32_t earlyExit = 0;
  
  uint16_t writtenAddresses[NUM_ADDRESSES];

  uint16_t absolute0;
  uint16_t absolute1;
  uint16_t absolute2;

  uint8_t writtenValues[NUM_ADDRESSES];
  uint8_t numAddressesWritten = 0;

  uint8_t zp0;
  uint8_t zp1;
  uint8_t zp2;
  uint8_t zp3;
  uint8_t c0;
  uint8_t c1;
  uint8_t _a;
  uint8_t _x;
  uint8_t _y;
  uint8_t _sp;
  bool _ccS;
  bool _ccV;
  bool _ccI;
  bool _ccD;
  bool _ccC;
  bool _ccZ;

// once the machine has exited, don't make any more changes
// All methods that change the initial state of the machine
// should be prefaced with E, causing them to be no-ops if
// the machine has exited.
#define E if (earlyExit) { return 0; }

  bool rts() {
    E; return (earlyExit = 0x0001);
  }
  bool rti() {
    E; return (earlyExit = 0x0002);
  }
  bool jmp(uint16_t target) {
    E; return (earlyExit = target | 0x10000);
  }
  bool branch(bool cond, uint16_t target) {
    E; if (cond) { return (earlyExit = target | 0x10000); }
    return 0;
  }

  uint8_t a(uint8_t val) { E return _a = val; }
  uint8_t x(uint8_t val) { E return _x = val; }
  uint8_t y(uint8_t val) { E return _y = val; }
  uint8_t sp(uint8_t val) { E return _sp = val; }
  bool ccS(bool val) { E return _ccS = val; }
  bool ccV(bool val) { E return _ccV = val; }
  bool ccI(bool val) { E return _ccI = val; }
  bool ccD(bool val) { E return _ccD = val; }
  bool ccC(bool val) { E return _ccC = val; }
  bool ccZ(bool val) { E return _ccZ = val; }

  // Reading on the concrete machine uses a randomly
  // filled memory space using the fnv hash.
  // It also remembers previous stores and returns
  // consistent results.
  uint8_t read(uint16_t addr) {
    E
    for (int i = 0; i < numAddressesWritten; i++) {
      if (writtenAddresses[i] == addr) return writtenValues[i];
    }
    return fnv(addr);
  }

  uint8_t setSZ(uint8_t val) {
    ccS(val >= 0x80);
    ccZ(val == 0);
    return val;
  }

  bool uge(uint8_t first, uint8_t second) {
    return first >= second;
  }

  uint16_t inline ite(bool cond, uint16_t conseq, uint16_t alter) {
    return cond ? conseq : alter;
  }

  uint16_t inline shl(const uint16_t val) const {
    return val << 1;
  }

  uint16_t inline shr(const uint16_t val) const {
    return val >> 1;
  }

  uint8_t inline lobyte(uint16_t val) const {
    return val;
  }

  uint8_t inline hibyte(uint16_t val) const {
    return val >> 8;
  }

  // Remember written values so that it returns
  // consistent results.
  uint8_t write(uint16_t addr, uint8_t val) {
    E
    for (int i = 0; i < numAddressesWritten; i++) {
      if (addr == writtenAddresses[i]) {
        return writtenValues[i] = val;
      }
    }
    writtenAddresses[numAddressesWritten] = addr;
    return writtenValues[numAddressesWritten++] = val;
  }

#undef E

  uint16_t extend(uint8_t val) {
    return val;
  }

  // A hash function based on the fnv hash.
  // See:
  // http://isthe.com/chongo/tech/comp/fnv/#FNV-1
  uint32_t fnv(uint16_t value) {
    if (seed == 0) {
      return 0;
    } else if (seed == 0xFFFFFFFF) {
      return 0xFFFFFFFF;
    }
    uint32_t hash = 2166136261;
    // first round use the seed.
    hash = (hash ^ (seed & 0xFF)) * 16777619;
    hash = (hash ^ ((seed >> 8) & 0xFF)) * 16777619;
    hash = (hash ^ ((seed >> 16) & 0xFF)) * 16777619;
    hash = (hash ^ ((seed >> 24) & 0xFF)) * 16777619;
    hash = hash ^ (value & 0xFF);
    hash = hash * 16777619;
    hash = hash ^ ((value & 0xFF00) >> 8);
    hash = hash * 16777619;
    return hash;
  }

  /**
   * Returns a hash representing the internal state
   * of the machine. It follows the rule that if two
   * instruction sequences are equivalent and run on two
   * random_machines with the same seed, then the machine's
   * hashes will be equivalent.
   *
   * The hash is basically the fnv-32 hash over the bytes
   * of the internal state of the machine.
   */
  uint32_t hash() {
    uint32_t hash = 2166136261;
#define h(var) hash = (hash ^ (var)) * 16777619;
    h(_a)
    h(_x)
    h(_y)
    h(_sp)
    h(_ccS)
    h(_ccV)
    h(_ccI)
    h(_ccD)
    h(_ccC)
    h(_ccZ)
    h(numAddressesWritten)
    for (uint8_t i = 0; i < numAddressesWritten; i++) {
      h(writtenAddresses[i] >> 8)
      h(writtenAddresses[i] & 0xFF)
      h(writtenValues[i])
    }
    h(earlyExit)
#undef h
    return hash;
  }
};
