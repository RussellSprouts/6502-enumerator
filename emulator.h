
#pragma once

#include <vector>
#include <unordered_set>
#include "opcode.h"
#include "operations.h"

typedef std::tuple<opcode, opcode> instruction2;
//typedef std::tuple<opcode, opcode, opcode> instruction_seq;
typedef std::tuple<opcode, opcode, opcode, opcode> instruction4;


struct instruction_seq {
  const static int max_length;
  opcode ops[3];

  instruction_seq()
    : ops{ opcode::zero, opcode::zero, opcode::zero } {}

  instruction_seq append(opcode op) const {
    instruction_seq copy = *this;
    for (int i = 0; i < max_length; i++) {
      if (copy.ops[i] == opcode::zero) {
        copy.ops[i] = op;
        break;
      }
    }
    return copy;
  }

  std::vector<instruction_seq> alternates() {
    std::vector<instruction_seq> result;
    return result;
  }

  bool operator==(const instruction_seq &other) const {
    for (int i = 0; i < max_length; i++) {
      if (ops[i] != other.ops[i]) return false;
    }
    return true;
  }

  bool in(const std::unordered_set<instruction_seq> &set) const;

  instruction_seq canonicalize() const {
    const int abs_start = 7;
    const int zp_start = 0xA;

    uint8_t absolute_vars[3] { 0xFF, 0xFF, 0xFF };
    int abs = abs_start;
    uint8_t zp_vars[4] = { 0xFF, 0xFF, 0xFF, 0xFF };
    int zp = zp_start;
    bool c0_first = false;
    bool c1_first = false;
   
    for (int i = 0; i < max_length; i++) {
      if (ops[i] == opcode::zero) { break; }
      uint8_t val = ops[i].mode & 0x0F;
      switch (ops[i].mode & 0xF0) {
      case 0x00: // Immediate
         if (!(c0_first || c1_first)) {
           if (val == 0 || 
               val == 4 || 
               val == 6) {
             c0_first = true;
           } else if (val == 1 || val == 5) {
             c1_first = true;
           }
         }
        break;
      case 0x10: // Absolute
      case 0x20: // AbsoluteX
      case 0x30: // AbsoluteY
      case 0x70: // Indirect
        // if we see a new absolute var, give it the next
        // value.
        if (absolute_vars[val - abs_start] == 0xFF) {
          absolute_vars[val - abs_start] = abs++;
        }
        break;
      case 0x40: // ZeroPage
      case 0x50: // ZeroPageX
      case 0x60: // ZeroPageY
      case 0x80: // IndirectX
      case 0x90: // IndirectY
        // if we see a new zp var, give it the next
        // value.
        if (zp_vars[val - zp_start] == 0xFF) {
          zp_vars[val - zp_start] = zp++;
        }
        break;
      }
    }
    
    instruction_seq result;

    // now return a new instruction_seq with the replacements made.
    for (int i = 0; i < max_length; i++) {
      if (ops[i] == opcode::zero) { break; }
      uint8_t mode = ops[i].mode & 0xF0;
      uint8_t operand = ops[i].mode & 0x0F;
      Operations op = ops[i].op;
      switch (operand) {
      case 0:
      case 4: // C0
        if (c1_first) {
          result = result.append(opcode { op, (AddrMode)(mode | (operand + 1)) });
        }
        else {
          result = result.append(ops[i]);
        }
        break;
      case 1:
      case 5: // C1
        if (c1_first) {
          result = result.append(opcode { op, (AddrMode)(mode | (operand - 1)) });
        }
        else {
          result = result.append(ops[i]);
        }
        break;
      case 0x7:
      case 0x8:
      case 0x9: // Absolute
        result = result.append(opcode { op, (AddrMode)(mode | absolute_vars[operand - abs_start]) });
        break; 
      case 0xA:
      case 0xB:
      case 0xC:
      case 0xD: // ZP
        result = result.append(opcode { op, (AddrMode)(mode | zp_vars[operand - zp_start]) });
        break; 
      default:
        result = result.append(ops[i]);  
      }
    }
    return result;
  }
};

namespace std {
  template <> struct hash<opcode>
  {
    size_t operator()(const opcode &x) const
    {
      return std::hash<size_t>{}(std::hash<uint8_t>{}((uint8_t&)x.op) ^ std::hash<uint8_t>{}((uint8_t&)x.mode));
      /* your code here, e.g. "return hash<int>()(x.value);" */
    }
  };

  template <> struct hash<instruction_seq>
  {
    size_t operator()(const instruction_seq &x) const
    {
      size_t result = 0;
      for (int i = 0; i < instruction_seq::max_length; i++) {
        result ^= std::hash<opcode>{}(x.ops[i]);
        result *= 1337;
      }
      return std::hash<size_t>{}(result);
    }
  };
}

bool instruction_seq::in(const std::unordered_set<instruction_seq> &set) const {
  instruction_seq needle = this->canonicalize();
  if (set.find(needle) != set.end()) { return true; }
  for (int i = 0; i < max_length - 1; i++) {
    instruction_seq s;
    s = s.append(needle.ops[i]);   
    s = s.append(needle.ops[i+1]);
    if (set.find(s) != set.end()) { return true; }  
  }
  return false;
}

const int instruction_seq::max_length = 3;

template <typename machine>
struct emulator {

  void instruction(machine& m, Operations op, AddrMode mode) const {
    auto absoluteVar = m.absolute0;
    auto zeroPageVar = m.zp0;
    auto immediateVar = m.c0;
    switch (mode & 0xF) {
      case 0x00: // c0 - good.
        break;
      case 0x01: // c1
        immediateVar = m.c1;
        break;
      case 0x02: // literal0
        immediateVar = m.literal0;
        break;
      case 0x03: // literal1
        immediateVar = m.literal1;
        break;
      case 0x04: // c0+1
        immediateVar = m.c0 + m.literal1;
        break;
      case 0x05: // c1+1
        immediateVar = m.c1 + m.literal1;
        break;
      case 0x06: // c0+c1
        immediateVar = m.c0 + m.c1;
        break;
      case 0x07: // absolute0 - good
        break;
      case 0x08: // absolute 1
        absoluteVar = m.absolute1;
        break;
      case 0x09:
        absoluteVar = m.absolute2;
        break;
      case 0x0A: // zp0 - good
        break;
      case 0x0B: // zp1
        zeroPageVar = m.zp1;
        break;
      case 0x0C: // zp2
        zeroPageVar = m.zp2;
        break;
      case 0x0D: // zp3
        zeroPageVar = m.zp3;
        break;
      case 0x0E: case 0x0F: // none - good
        break;
    }
    switch (mode & 0xF0) {
      case 0x00: // immediate
        break;
      case 0x10: // absolute
        break;
      case 0x20: // absolute, x
        absoluteVar = absoluteVar + m.extend(m._x);
        break;
      case 0x30: // absolute, y
        absoluteVar = absoluteVar + m.extend(m._y);
        break;
      case 0x40: // zero page
        absoluteVar = m.extend(zeroPageVar);
        break;
      case 0x50: // zeroPage, x
        absoluteVar = m.extend((zeroPageVar + m._x) & 0xFF);
        break;
      case 0x60: // zeroPage, y
        absoluteVar = m.extend((zeroPageVar + m._y) & 0xFF);
        break;
      case 0x70: { // indirect
        auto jmpiBug = (absoluteVar & 0xFF00) | ((absoluteVar + 1) & 0xFF);
        absoluteVar = (m.extend(m.read(jmpiBug)) << 8) | m.extend(m.read(absoluteVar));
        break;
        }
      case 0x80: // (indirect, x)
        absoluteVar = m.extend(m.read(m.extend(((zeroPageVar + m._x) & 0xFF))))
          | (m.extend(m.read(m.extend((zeroPageVar + m._x + 1) & 0xFF))) << 8);
        absoluteVar = (m.extend(m.read(absoluteVar+1)) << 8) | m.extend(m.read(absoluteVar));
        break;
      case 0x90: // (indirect), y
        absoluteVar = (m.extend(m.read(m.extend(zeroPageVar)))
          | (m.extend(m.read(m.extend((zeroPageVar + 1) & 0xFF))) << 8)) + m.extend(m._y);
        break;
      default: // none
        break;
    }
    if ((mode & 0xF0) != 0)  {
      immediateVar = m.read(absoluteVar);
    }

    // Now either immediateVar, or absoluteVar will hold the correct value.
    switch (op) {
      case AND:
        m.a(m.setSZ(m._a & immediateVar));
        break;
      case ORA:
        m.a(m.setSZ(m._a | immediateVar));
        break;
      case EOR: 
        m.a(m.setSZ(m._a ^ immediateVar));
        break;
      case LDA:
        m.a(m.setSZ(immediateVar));
        break;
      case LDX:
        m.x(m.setSZ(immediateVar));
        break;
      case LDY:
        m.y(m.setSZ(immediateVar));
        break;
      case TXA:
        m.a(m.setSZ(m._x));
        break;
      case TAX:
        m.x(m.setSZ(m._a));
        break;
      case TYA:
        m.a(m.setSZ(m._y));
        break;
      case TAY:
        m.y(m.setSZ(m._a));
        break;
      case INX:
        m.x(m.setSZ(m._x + 1));
        break;
      case INY:
        m.y(m.setSZ(m._y + 1));
        break;
      case DEX:
        m.x(m.setSZ(m._x - 1));
        break;
      case DEY:
        m.y(m.setSZ(m._y - 1));
        break;
      case CLC:
        m.ccC(m.falsy);
        break;
      case CLI:
        m.ccI(m.falsy);
        break;
      case CLV:
        m.ccV(m.falsy);
        break;
      case CLD:
        m.ccD(m.falsy);
        break;
      case SEC:
        m.ccC(m.truthy);
        break;
      case SEI:
        m.ccI(m.truthy);
        break;
      case SED:
        m.ccD(m.truthy);
        break;
      case TSX:
        m.x(m._sp);
        break;
      case TXS:
        m.sp(m._x);
        break;
      case NOP:
        // nap.
        break;
      case INC:
        m.write(absoluteVar, m.setSZ(immediateVar + 1));
        break;
      case DEC:
        m.write(absoluteVar, m.setSZ(immediateVar - 1));
        break;
      case BIT:
        m.ccZ((immediateVar & m._a) == 0);
        m.ccS((immediateVar & 0x80) == 0x80);
        m.ccV((immediateVar & 0x40) == 0x40);
        break;
      case ASL_A:
        m.ccC((m._a & 0x80) == 0x80);
        m.a(m.setSZ(m.shl(m._a)));
        break;
      case ASL:
        m.ccC((immediateVar & 0x80) == 0x80);
        m.write(absoluteVar, m.setSZ(m.shl(immediateVar)));
        break;
      case ROL_A: {
        auto val = m.shl(m._a) | m.ite(m._ccC, m.literal1, m.literal0);
        m.ccC((m._a & 0x80) == 0x80);
        m.a(m.setSZ(val));
        break;
        }
      case ROL: {
        auto val = m.shl(immediateVar) | m.ite(m._ccC, m.literal1, m.literal0);
        m.ccC((immediateVar & 0x80) == 0x80);
        m.write(absoluteVar, m.setSZ(val));
        break;
        }
      case LSR_A:
        m.ccC((m._a & 0x01) == 0x01);
        m.a(m.setSZ(m.shr(m._a)));
        break;
      case LSR:
        m.ccC((immediateVar & 0x01) == 0x01);
        m.write(absoluteVar, m.setSZ(m.shr(immediateVar)));
        break;
      case ROR_A: {
        auto val = m.shr(m._a) | m.ite(m._ccC, 0x80, 0x00);
        m.ccC((m._a & 0x01) == 0x01);
        m.a(m.setSZ(val));
        break;
        }
      case ROR: {
        auto val = m.shr(immediateVar) | m.ite(m._ccC, 0x80, 0x00);
        m.ccC((immediateVar & 0x01) == 0x01);
        m.write(absoluteVar, m.setSZ(val));
        break;
        }
      case STA:
        m.write(absoluteVar, m._a);
        break;
      case STX:
        m.write(absoluteVar, m._x);
        break;
      case STY:
        m.write(absoluteVar, m._y);
        break;
      case PLA:
        m.sp(m._sp + 1);
        m.a(m.read(m.extend(m._sp) + 0x0100));
        break;
      case PHA:
        m.write(m.extend(m._sp) + 0x0100, m._a);
        m.sp(m._sp - 1);
        break;
      case PHP: {
        auto p = m.ite(m._ccS, 0x80, 0x00)
               | m.ite(m._ccV, 0x40, 0x00)
               | m.ite(m._ccD, 0x08, 0x00)
               | m.ite(m._ccI, 0x04, 0x00)
               | m.ite(m._ccZ, 0x02, 0x00)
               | m.ite(m._ccC, 0x01, 0x00);
        m.write(m.extend(m._sp) + 0x0100, p);
        m.sp(m._sp - 1);
        break;
        }
      case PLP: {
        m.sp(m._sp + 1);
        auto pull = m.read(m.extend(m._sp) + 0x0100);
        m.ccS(0x80 == (pull & 0x80));     
        m.ccV(0x40 == (pull & 0x40));     
        m.ccD(0x08 == (pull & 0x08));     
        m.ccI(0x04 == (pull & 0x04));     
        m.ccZ(0x02 == (pull & 0x02));     
        m.ccC(0x01 == (pull & 0x01));     
        break;
        }
      case SBC:
        immediateVar = immediateVar ^ 0xFF;
        /* fallthrough */
      case ADC: {
        auto sum = m.extend(immediateVar) + m.extend(m._a) + m.extend(m.ite(m._ccC, 0x01, 0x00));
        m.ccV(0x80 == (0x80 & (m.lobyte(sum) ^ m._a) & (m.lobyte(sum) ^ immediateVar)));
        m.ccC(m.hibyte(sum) == 0x01);
        m.a(m.setSZ(m.lobyte(sum)));
        break;
        }
      case CMP:
        m.ccC(m.uge(m._a, immediateVar));
        m.ccS(m.uge(m._a - immediateVar, 0x80));
        m.ccZ(m._a == immediateVar);
        break;
      case CPX:
        m.ccC(m.uge(m._x, immediateVar));
        m.ccS(m.uge(m._x - immediateVar, 0x80));
        m.ccZ(m._x == immediateVar);
        break;
      case CPY:
        m.ccC(m.uge(m._y, immediateVar));
        m.ccS(m.uge(m._y - immediateVar, 0x80));
        m.ccZ(m._y == immediateVar);
        break;
      case JMP:
        m.jmp(absoluteVar);
        break;
      case RTI:
        m.rti();
        break;
      case RTS:
        m.rts();
        break;
      case BPL:
        m.branch(!m._ccS, absoluteVar);
        break;
      case BMI:
        m.branch(m._ccS, absoluteVar);
        break;
      case BVS:
        m.branch(m._ccV, absoluteVar);
        break;
      case BVC:
        m.branch(!m._ccV, absoluteVar);
        break;
      case BCC:
        m.branch(!m._ccC, absoluteVar);
        break;
      case BCS:
        m.branch(m._ccC, absoluteVar);
        break;
      case BEQ:
        m.branch(m._ccZ, absoluteVar);
        break;
      case BNE:
        m.branch(!m._ccZ, absoluteVar);
        break;
    }
  }
};

