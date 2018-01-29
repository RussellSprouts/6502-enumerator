
#pragma once

#include "opcode.h"
#include "operations.h"

template <typename machine>
struct emulator {

  void instruction(machine& m, opcode op) const {
    instruction(m, op.op, op.mode);
  } 

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
        absoluteVar = m.extend(m.read(m.extend(zeroPageVar)))
          | (m.extend(m.read(m.extend((zeroPageVar + 1) & 0xFF))) << 8);
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
        m.x(m.setSZ(m._y + 1));
        break;
      case DEX:
        m.x(m.setSZ(m._x - 1));
        break;
      case DEY:
        m.x(m.setSZ(m._y - 1));
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
        m.a(m.setSZ(m.shl(m._a, 1)));
        break;
      case ASL:
        m.ccC((immediateVar & 0x80) == 0x80);
        m.write(absoluteVar, m.setSZ(m.shl(immediateVar,1)));
        break;
      case ROL_A: {
        auto val = m.shl(m._a, 1) | m.ite(m._ccC, m.literal1, m.literal0);
        m.ccC((m._a & 0x80) == 0x80);
        m.a(m.setSZ(val));
        break;
        }
      case ROL: {
        auto val = m.shl(immediateVar, 1) | m.ite(m._ccC, m.literal1, m.literal0);
        m.ccC((immediateVar & 0x80) == 0x80);
        m.write(absoluteVar, m.setSZ(val));
        break;
        }
      case LSR_A:
        m.ccC((m._a & 0x01) == 0x01);
        m.a(m.setSZ(m.shr(m._a, 1)));
        break;
      case LSR:
        m.ccC((immediateVar & 0x01) == 0x01);
        m.write(absoluteVar, m.setSZ(m.shr(immediateVar, 1)));
        break;
      case ROR_A: {
        auto val = m.shr(m._a, 1) | m.ite(m._ccC, 0x80, 0x00);
        m.ccC((m._a & 0x01) == 0x01);
        m.a(m.setSZ(val));
        break;
        }
      case ROR: {
        auto val = m.shr(immediateVar, 1) | m.ite(m._ccC, 0x80, 0x00);
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

