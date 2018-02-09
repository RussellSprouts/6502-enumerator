
#pragma once

#include "stdint.h"

enum Operations: uint8_t {
  ADC = 1,
  AND = 2,
  ASL = 3,
  BIT = 4,
  CMP = 5,
  CPX = 6,
  CPY = 7,
  DEC = 8,
  EOR = 9,
  INC = 10,
  JMP = 11,
  LDA = 12,
  LDX = 13,
  LDY = 14,
  LSR = 15,
  ORA = 16,
  ROL = 17,
  ROR = 18,
  SBC = 19,
  STA = 20,
  STX = 21,
  STY = 22,
  ASL_A = 23,
  BPL = 24,
  BMI = 25,
  BVC = 26,
  BVS = 27,
  BCC = 28,
  BCS = 29,
  BNE = 30,
  BEQ = 31,
  CLC = 32,
  SEC = 33,
  CLI = 34,
  SEI = 35,
  CLV = 36,
  CLD = 37,
  SED = 38,
  LSR_A = 39,
  NOP = 40,
  TAX = 41,
  TXA = 42,
  DEX = 43,
  INX = 44,
  TAY = 45,
  TYA = 46,
  DEY = 47,
  INY = 48,
  ROL_A = 49,
  ROR_A = 50,
  RTI = 51,
  RTS = 52,
  TXS = 53,
  TSX = 54,
  PHA = 55,
  PLA = 56,
  PHP = 57,
  PLP = 58 
 };

const char *OpNames[] = {
  "", "adc", "and", "asl", "bit", "cmp", "cpx", "cpy", "dec", "eor", "inc", "jmp", "lda", "ldx", "ldy", "lsr", "ora", "rol", "ror", "sbc", "sta", "stx", "sty",
  "asl_a", "bpl", "bmi", "bvc", "bvs", "bcc", "bcs", "bne", "beq", "clc", "sec", "cli", "sei", "clv", "cld", "sed", "lsr_a", "nop", "tax", "txa", "dex", "inx", "tay", "tya", "dey", "iny", "rol_a", "ror_a", "rti", "rts", "txs", "tsx", "pha", "pla", "php", "plp"
};

enum AddrMode: uint8_t {
  ImmediateC0 = 0x00,
  ImmediateC1 = 0x01,
  Immediate0 = 0x02,
  Immediate1 = 0x03,
  ImmediateC0Plus1 = 0x04,
  ImmediateC1Plus1 = 0x05,
  ImmediateC0PlusC1 = 0x06,
  Absolute0 = 0x17,
  Absolute1 = 0x18,
  Absolute2 = 0x19,
  AbsoluteX0 = 0x27,
  AbsoluteX1 = 0x28,
  AbsoluteX2 = 0x29,
  AbsoluteY0 = 0x37,
  AbsoluteY1 = 0x38,
  AbsoluteY2 = 0x39,
  ZeroPage0 = 0x4A,
  ZeroPage1 = 0x4B,
  ZeroPage2 = 0x4C,
  ZeroPage3 = 0x4D,
  ZeroPageX0 = 0x5A,
  ZeroPageX1 = 0x5B,
  ZeroPageX2 = 0x5C,
  ZeroPageX3 = 0x5D,
  ZeroPageY0 = 0x6A,
  ZeroPageY1 = 0x6B,
  ZeroPageY2 = 0x6C,
  ZeroPageY3 = 0x6D,
  Indirect0 = 0x77,
  Indirect1 = 0x78,
  Indirect2 = 0x79,
  IndirectX0 = 0x8A,
  IndirectX1 = 0x8B,
  IndirectX2 = 0x8C,
  IndirectX3 = 0x8D,
  IndirectY0 = 0x9A,
  IndirectY1 = 0x9B,
  IndirectY2 = 0x9C,
  IndirectY3 = 0x9D,
  None = 0xAE
};

std::map<AddrMode, const char*> AddrModeNames {
  { ImmediateC0, "#C0" },
  { ImmediateC1, "#C1"},
  { Immediate0, "#0"},
  { Immediate1, "#1" },
  { ImmediateC0Plus1, "#C0+1" },
  { ImmediateC1Plus1, "#C1+1" },
  { ImmediateC0PlusC1, "#C0+C1" },
  { Absolute0, "Absolute0" },
  { Absolute1, "Absolute1" },
  { Absolute2, "Absolute2" },
  { AbsoluteX0, "Absolute0, x" },
  { AbsoluteX1, "Absolute1, x" },
  { AbsoluteX2, "Absolute2, x" },
  { AbsoluteY0, "Absolute0, y" },
  { AbsoluteY1, "Absolute1, y" },
  { AbsoluteY2, "Absolute2, y" },
  { ZeroPage0, "Zp0" },
  { ZeroPage1, "Zp1" },
  { ZeroPage2, "Zp2" },
  { ZeroPage3, "Zp3" },
  { ZeroPageX0, "Zp0, x" },
  { ZeroPageX1, "Zp1, x" },
  { ZeroPageX2, "Zp2, x" },
  { ZeroPageX3, "Zp3, x" },
  { ZeroPageY0, "Zp0, y" },
  { ZeroPageY1, "Zp1, y" },
  { ZeroPageY2, "Zp2, y" },
  { ZeroPageY3, "Zp3, y" },
  { Indirect0, "(Absolute0)" },
  { Indirect1, "(Absolute1)" },
  { Indirect2, "(Absolute2)" },
  { IndirectX0, "(Zp0, x)" },
  { IndirectX1, "(Zp1, x)" },
  { IndirectX2, "(Zp2, x)" },
  { IndirectX3, "(Zp3, x)" },
  { IndirectY0, "(Zp0), y" },
  { IndirectY1, "(Zp1), y" },
  { IndirectY2, "(Zp2), y" },
  { IndirectY3, "(Zp3), y" },
  { None, "" }
};
