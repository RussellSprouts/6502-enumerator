
#pragma once

#include "stdint.h"

static const int NUM_ADDRESSES = 16;

struct concrete_machine {
  static const uint8_t literal0 = 0;
  static const uint8_t literal1 = 1;
  static const bool falsy = false;
  static const bool truthy = true;

  concrete_machine(uint32_t _seed) {
    seed = _seed;
    _a = fnv(113) ^ seed;
    _x = fnv(114) ^ seed;
    _y = fnv(115) ^ seed;
    _sp = fnv(116) ^ seed;
    _ccS = (fnv(117) ^ seed);
    _ccV = (fnv(118) ^ seed) & 1;
    _ccI = (fnv(119) ^ seed) & 1;
    _ccD = (fnv(120) ^ seed) & 1;
    _ccC = (fnv(121) ^ seed) & 1;
    _ccZ = (fnv(122) ^ seed) & 1;
    absolute0 = fnv(123) ^ seed;
    absolute1 = fnv(124) ^ seed;
    absolute2 = fnv(125) ^ seed;
    zp0 = fnv(126) ^ seed;
    zp1 = fnv(127) ^ seed;
    zp2 = fnv(128) ^ seed;
    zp3 = fnv(129) ^ seed;
    c0 = fnv(130) ^ seed;
    c1 = fnv(131) ^ seed;
  }

  void instruction(instruction3 ops) {
    opcode zero { (Operations)0, (AddrMode)0 };

    if (std::get<0>(ops) != zero) { instruction(std::get<0>(ops)); }
    if (std::get<1>(ops) != zero) { instruction(std::get<1>(ops)); }
    if (std::get<2>(ops) != zero) { instruction(std::get<2>(ops)); }
  }

  void instruction(opcode op) {
    emulator<concrete_machine> emu;
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
#define E if (earlyExit) { return 0; }

  bool rts() {
    E; return (earlyExit = 0x10000);
  }
  bool rti() {
    E; return (earlyExit = 0x20000);
  }
  bool jmp(uint16_t target) {
    E; return (earlyExit = target | 0x30000);
  }
  bool branch(bool cond, uint16_t target) {
    E; if (cond) { return (earlyExit = target | 0x30000); }
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

  uint16_t inline shl(uint16_t val, int amt) {
    return val << amt;
  }

  uint16_t inline shr(uint16_t val, int amt) {
    return val >> amt;
  }

  uint8_t inline lobyte(uint16_t val) {
    return val;
  }

  uint8_t inline hibyte(uint16_t val) {
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

  // http://isthe.com/chongo/tech/comp/fnv/#FNV-1
  uint32_t fnv(uint16_t value) {
    if (seed == 0) {
      return 0;
    } else if (seed == 0xFFFFFFFF) {
      return 0xFFFFFFFF;
    }
    uint32_t hash = 2166136261;
    hash = (hash ^ seed) * 16777619;
    hash = hash ^ (value & 0xFF);
    hash = hash * 16777619;
    hash = hash ^ ((value & 0xFF00) >> 8);
    hash = hash * 16777619;
    return hash;
  }

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
