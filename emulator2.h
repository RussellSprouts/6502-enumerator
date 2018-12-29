#pragma once
#include "instructions2.h"

template<typename machine>
struct emulator {
  void instruction(machine &m, instruction ins) {
    if (ins.name() == instruction_name::NONE) {
      return;
    }

    auto absoluteVar = m.absolute(0);
    auto immediateVar = m.immediate(0);

    switch(ins.mode()) {
    case addr_mode::NONE:
      break; // nothing to do
    case addr_mode::ABSOLUTE:
      absoluteVar = m.absolute(ins.number());
      break;
    case addr_mode::ABSOLUTE_X:
      absoluteVar = m.absolute(ins.number()) + m.extend(m._x);
      break;
    case addr_mode::ABSOLUTE_Y:
      absoluteVar = m.absolute(ins.number()) + m.extend(m._y);
      break;
    case addr_mode::X_INDIRECT:
      absoluteVar = m.extend(m.read(m.extend(((m.zp(ins.number()) + m._x) & 0xFF))))
        | (m.extend(m.read(m.extend((m.zp(ins.number()) + m._x + 1) & 0xFF))) << 8);
      absoluteVar = (m.extend(m.read(absoluteVar+1)) << 8) | m.extend(m.read(absoluteVar));
      break;
    case addr_mode::INDIRECT_Y:
      absoluteVar = (m.extend(m.read(m.extend(m.zp(ins.number()))))
        | (m.extend(m.read(m.extend((m.zp(ins.number()) + 1) & 0xFF))) << 8)) + m.extend(m._y);
      break;
    case addr_mode::ZERO_PAGE:
      absoluteVar = m.extend(m.zp(ins.number()));
      break;
    case addr_mode::ZERO_PAGE_X:
      absoluteVar = m.extend(m.zp(ins.number()) + m._x);
      break;
    case addr_mode::ZERO_PAGE_Y:
      absoluteVar = m.extend(m.zp(ins.number()) + m._y);
      break;
    case addr_mode::IMMEDIATE:
      immediateVar = m.immediate(ins.number());
      break;
    case addr_mode::CONSTANT:
      immediateVar = m.constant(ins.number());
    }

    // If we aren't using the immediate operand, then read the address we found.
    if (ins.mode() != addr_mode::IMMEDIATE && ins.mode() != addr_mode::CONSTANT) {
      immediateVar = m.read(absoluteVar);
    }

    switch (ins.name()) {
    case instruction_name::NONE:
      break; // nothing to do
    case instruction_name::AND:
      m.a(m.setSZ(m._a & immediateVar));
      break;
    case instruction_name::ORA:
      m.a(m.setSZ(m._a | immediateVar));
      break;
    case instruction_name::EOR: 
      m.a(m.setSZ(m._a ^ immediateVar));
      break;
    case instruction_name::LDA:
      m.a(m.setSZ(immediateVar));
      break;
    case instruction_name::LDX:
      m.x(m.setSZ(immediateVar));
      break;
    case instruction_name::LDY:
      m.y(m.setSZ(immediateVar));
      break;
    case instruction_name::TXA:
      m.a(m.setSZ(m._x));
      break;
    case instruction_name::TAX:
      m.x(m.setSZ(m._a));
      break;
    case instruction_name::TYA:
      m.a(m.setSZ(m._y));
      break;
    case instruction_name::TAY:
      m.y(m.setSZ(m._a));
      break;
    case instruction_name::INX:
      m.x(m.setSZ(m._x + 1));
      break;
    case instruction_name::INY:
      m.y(m.setSZ(m._y + 1));
      break;
    case instruction_name::DEX:
      m.x(m.setSZ(m._x - 1));
      break;
    case instruction_name::DEY:
      m.y(m.setSZ(m._y - 1));
      break;
    case instruction_name::CLC:
      m.ccC(m.falsy);
      break;
    case instruction_name::CLI:
      m.ccI(m.falsy);
      break;
    case instruction_name::CLV:
      m.ccV(m.falsy);
      break;
    case instruction_name::CLD:
      m.ccD(m.falsy);
      break;
    case instruction_name::SEC:
      m.ccC(m.truthy);
      break;
    case instruction_name::SEI:
      m.ccI(m.truthy);
      break;
    case instruction_name::SED:
      m.ccD(m.truthy);
      break;
    case instruction_name::TSX:
      m.x(m._sp);
      break;
    case instruction_name::TXS:
      m.sp(m._x);
      break;
    case instruction_name::NOP:
      // nap.
      break;
    case instruction_name::INC:
      m.write(absoluteVar, m.setSZ(immediateVar + 1));
      break;
    case instruction_name::DEC:
      m.write(absoluteVar, m.setSZ(immediateVar - 1));
      break;
    case instruction_name::BIT:
      m.ccZ((immediateVar & m._a) == 0);
      m.ccS((immediateVar & 0x80) == 0x80);
      m.ccV((immediateVar & 0x40) == 0x40);
      break;
    case instruction_name::ASLA:
      m.ccC((m._a & 0x80) == 0x80);
      m.a(m.setSZ(m.shl(m._a)));
      break;
    case instruction_name::ASL:
      m.ccC((immediateVar & 0x80) == 0x80);
      m.write(absoluteVar, m.setSZ(m.shl(immediateVar)));
      break;
    case instruction_name::ROLA: {
      auto val = m.shl(m._a) | m.ite(m._ccC, m.constant(1), m.constant(0));
      m.ccC((m._a & 0x80) == 0x80);
      m.a(m.setSZ(val));
      break;
      }
    case instruction_name::ROL: {
      auto val = m.shl(immediateVar) | m.ite(m._ccC, m.constant(1), m.constant(0));
      m.ccC((immediateVar & 0x80) == 0x80);
      m.write(absoluteVar, m.setSZ(val));
      break;
      }
    case instruction_name::LSRA:
      m.ccC((m._a & 0x01) == 0x01);
      m.a(m.setSZ(m.shr(m._a)));
      break;
    case instruction_name::LSR:
      m.ccC((immediateVar & 0x01) == 0x01);
      m.write(absoluteVar, m.setSZ(m.shr(immediateVar)));
      break;
    case instruction_name::RORA: {
      auto val = m.shr(m._a) | m.ite(m._ccC, 0x80, 0x00);
      m.ccC((m._a & 0x01) == 0x01);
      m.a(m.setSZ(val));
      break;
      }
    case instruction_name::ROR: {
      auto val = m.shr(immediateVar) | m.ite(m._ccC, 0x80, 0x00);
      m.ccC((immediateVar & 0x01) == 0x01);
      m.write(absoluteVar, m.setSZ(val));
      break;
      }
    case instruction_name::STA:
      m.write(absoluteVar, m._a);
      break;
    case instruction_name::STX:
      m.write(absoluteVar, m._x);
      break;
    case instruction_name::STY:
      m.write(absoluteVar, m._y);
      break;
    case instruction_name::PLA:
      m.sp(m._sp + 1);
      m.a(m.read(m.extend(m._sp) + 0x0100));
      break;
    case instruction_name::PHA:
      m.write(m.extend(m._sp) + 0x0100, m._a);
      m.sp(m._sp - 1);
      break;
    case instruction_name::PHP: {
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
    case instruction_name::PLP: {
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
    case instruction_name::SBC:
      immediateVar = immediateVar ^ 0xFF;
      /* fallthrough */
    case instruction_name::ADC: {
      auto sum = m.extend(immediateVar) + m.extend(m._a) + m.extend(m.ite(m._ccC, 0x01, 0x00));
      m.ccV(0x80 == (0x80 & (m.lobyte(sum) ^ m._a) & (m.lobyte(sum) ^ immediateVar)));
      m.ccC(m.hibyte(sum) == 0x01);
      m.a(m.setSZ(m.lobyte(sum)));
      break;
      }
    case instruction_name::CMP:
      m.ccC(m.uge(m._a, immediateVar));
      m.ccS(m.uge(m._a - immediateVar, 0x80));
      m.ccZ(m._a == immediateVar);
      break;
    case instruction_name::CPX:
      m.ccC(m.uge(m._x, immediateVar));
      m.ccS(m.uge(m._x - immediateVar, 0x80));
      m.ccZ(m._x == immediateVar);
      break;
    case instruction_name::CPY:
      m.ccC(m.uge(m._y, immediateVar));
      m.ccS(m.uge(m._y - immediateVar, 0x80));
      m.ccZ(m._y == immediateVar);
      break;
    case instruction_name::JMP:
      m.jmp(absoluteVar);
      break;
    case instruction_name::JMPI: {
      auto jmpiBug = (absoluteVar & 0xFF00) | ((absoluteVar + 1) & 0xFF);
      m.jmp((m.extend(m.read(jmpiBug)) << 8) | m.extend(m.read(absoluteVar)));
      break;
      }
    case instruction_name::RTI:
      m.rti();
      break;
    case instruction_name::RTS:
      m.rts();
      break;
    case instruction_name::BPL:
      m.branch(!m._ccS, absoluteVar);
      break;
    case instruction_name::BMI:
      m.branch(m._ccS, absoluteVar);
      break;
    case instruction_name::BVS:
      m.branch(m._ccV, absoluteVar);
      break;
    case instruction_name::BVC:
      m.branch(!m._ccV, absoluteVar);
      break;
    case instruction_name::BCC:
      m.branch(!m._ccC, absoluteVar);
      break;
    case instruction_name::BCS:
      m.branch(m._ccC, absoluteVar);
      break;
    case instruction_name::BEQ:
      m.branch(m._ccZ, absoluteVar);
      break;
    case instruction_name::BNE:
      m.branch(!m._ccZ, absoluteVar);
      break;
    case instruction_name::JSR:
    case instruction_name::BRK:
      // not implemented
      break;
    }
  }
};
