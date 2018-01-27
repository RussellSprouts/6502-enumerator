
#include "stdint.h"
#include <vector>
#include <map>
#include "z3++.h"

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

struct opcode {
  Operations op;
  AddrMode mode;

  bool operator==(const opcode& other) const {
    return op == other.op && mode == other.mode;
  }

  bool operator!=(const opcode& other) const {
    return !(*this == other);
  }
};

typedef std::tuple<opcode, opcode> instruction2;
typedef std::tuple<opcode, opcode, opcode> instruction3;

opcode opcodes[] = {
  { ADC, Immediate0 },
  { AND, Immediate0 }, // same as LDA #0
  { CMP, Immediate0 },
  { CPX, Immediate0 },
  { CPY, Immediate0 },
  { EOR, Immediate0 }, // same as ORA #0
  { LDA, Immediate0 },
  { LDX, Immediate0 },
  { LDY, Immediate0 },
  { ORA, Immediate0 },
  { SBC, Immediate0 },

  { ADC, Immediate1 },
  { AND, Immediate1 },
  { CMP, Immediate1 },
  { CPX, Immediate1 },
  { CPY, Immediate1 },
  { EOR, Immediate1 },
  { LDA, Immediate1 },
  { LDX, Immediate1 },
  { LDY, Immediate1 },
  { ORA, Immediate1 },
  { SBC, Immediate1 },

  { ADC, ImmediateC0 },
  { AND, ImmediateC0 },
  { CMP, ImmediateC0 },
  { CPX, ImmediateC0 },
  { CPY, ImmediateC0 },
  { EOR, ImmediateC0 },
  { LDA, ImmediateC0 },
  { LDX, ImmediateC0 },
  { LDY, ImmediateC0 },
  { ORA, ImmediateC0 },
  { SBC, ImmediateC0 },

  { ADC, ImmediateC1 },
  { AND, ImmediateC1 },
  { CMP, ImmediateC1 },
  { CPX, ImmediateC1 },
  { CPY, ImmediateC1 },
  { EOR, ImmediateC1 },
  { LDA, ImmediateC1 },
  { LDX, ImmediateC1 },
  { LDY, ImmediateC1 },
  { ORA, ImmediateC1 },
  { SBC, ImmediateC1 },

  { ADC, ImmediateC0Plus1 },
  { AND, ImmediateC0Plus1 },
  { CMP, ImmediateC0Plus1 },
  { CPX, ImmediateC0Plus1 },
  { CPY, ImmediateC0Plus1 },
  { EOR, ImmediateC0Plus1 },
  { LDA, ImmediateC0Plus1 },
  { LDX, ImmediateC0Plus1 },
  { LDY, ImmediateC0Plus1 },
  { ORA, ImmediateC0Plus1 },
  { SBC, ImmediateC0Plus1 },

  { ADC, ImmediateC1Plus1 },
  { AND, ImmediateC1Plus1 },
  { CMP, ImmediateC1Plus1 },
  { CPX, ImmediateC1Plus1 },
  { CPY, ImmediateC1Plus1 },
  { EOR, ImmediateC1Plus1 },
  { LDA, ImmediateC1Plus1 },
  { LDX, ImmediateC1Plus1 },
  { LDY, ImmediateC1Plus1 },
  { ORA, ImmediateC1Plus1 },
  { SBC, ImmediateC1Plus1 },

  { ADC, ImmediateC0PlusC1 },
  { AND, ImmediateC0PlusC1 },
  { CMP, ImmediateC0PlusC1 },
  { CPX, ImmediateC0PlusC1 },
  { CPY, ImmediateC0PlusC1 },
  { EOR, ImmediateC0PlusC1 },
  { LDA, ImmediateC0PlusC1 },
  { LDX, ImmediateC0PlusC1 },
  { LDY, ImmediateC0PlusC1 },
  { ORA, ImmediateC0PlusC1 },
  { SBC, ImmediateC0PlusC1 },

  { ADC, ZeroPage0 },
  { AND, ZeroPage0 },
  { ASL, ZeroPage0 },
  { BIT, ZeroPage0 },
  { CMP, ZeroPage0 },
  { CPX, ZeroPage0 },
  { CPY, ZeroPage0 },
  { DEC, ZeroPage0 },
  { EOR, ZeroPage0 },
  { INC, ZeroPage0 },
  { LDA, ZeroPage0 },
  { LDX, ZeroPage0 },
  { LDY, ZeroPage0 },
  { LSR, ZeroPage0 },
  { ORA, ZeroPage0 },
  { ROL, ZeroPage0 },
  { ROR, ZeroPage0 },
  { SBC, ZeroPage0 },
  { STA, ZeroPage0 },
  { STX, ZeroPage0 },
  { STY, ZeroPage0 },

  { ADC, ZeroPage1 },
  { AND, ZeroPage1 },
  { ASL, ZeroPage1 },
  { BIT, ZeroPage1 },
  { CMP, ZeroPage1 },
  { CPX, ZeroPage1 },
  { CPY, ZeroPage1 },
  { DEC, ZeroPage1 },
  { EOR, ZeroPage1 },
  { INC, ZeroPage1 },
  { LDA, ZeroPage1 },
  { LDX, ZeroPage1 },
  { LDY, ZeroPage1 },
  { LSR, ZeroPage1 },
  { ORA, ZeroPage1 },
  { ROL, ZeroPage1 },
  { ROR, ZeroPage1 },
  { SBC, ZeroPage1 },
  { STA, ZeroPage1 },
  { STX, ZeroPage1 },
  { STY, ZeroPage1 },

  { ADC, ZeroPage2 },
  { AND, ZeroPage2 },
  { ASL, ZeroPage2 },
  { BIT, ZeroPage2 },
  { CMP, ZeroPage2 },
  { CPX, ZeroPage2 },
  { CPY, ZeroPage2 },
  { DEC, ZeroPage2 },
  { EOR, ZeroPage2 },
  { INC, ZeroPage2 },
  { LDA, ZeroPage2 },
  { LDX, ZeroPage2 },
  { LDY, ZeroPage2 },
  { LSR, ZeroPage2 },
  { ORA, ZeroPage2 },
  { ROL, ZeroPage2 },
  { ROR, ZeroPage2 },
  { SBC, ZeroPage2 },
  { STA, ZeroPage2 },
  { STX, ZeroPage2 },
  { STY, ZeroPage2 },

  { ADC, ZeroPage3 },
  { AND, ZeroPage3 },
  { ASL, ZeroPage3 },
  { BIT, ZeroPage3 },
  { CMP, ZeroPage3 },
  { CPX, ZeroPage3 },
  { CPY, ZeroPage3 },
  { DEC, ZeroPage3 },
  { EOR, ZeroPage3 },
  { INC, ZeroPage3 },
  { LDA, ZeroPage3 },
  { LDX, ZeroPage3 },
  { LDY, ZeroPage3 },
  { LSR, ZeroPage3 },
  { ORA, ZeroPage3 },
  { ROL, ZeroPage3 },
  { ROR, ZeroPage3 },
  { SBC, ZeroPage3 },
  { STA, ZeroPage3 },
  { STX, ZeroPage3 },
  { STY, ZeroPage3 },

  { ADC, ZeroPageX0 },
  { AND, ZeroPageX0 },
  { ASL, ZeroPageX0 },
  { CMP, ZeroPageX0 },
  { DEC, ZeroPageX0 },
  { EOR, ZeroPageX0 },
  { INC, ZeroPageX0 },
  { LDA, ZeroPageX0 },
  { LDY, ZeroPageX0 },
  { LSR, ZeroPageX0 },
  { ORA, ZeroPageX0 },
  { ROL, ZeroPageX0 },
  { ROR, ZeroPageX0 },
  { SBC, ZeroPageX0 },
  { STA, ZeroPageX0 },
  { STY, ZeroPageX0 },

  { ADC, ZeroPageX1 },
  { AND, ZeroPageX1 },
  { ASL, ZeroPageX1 },
  { CMP, ZeroPageX1 },
  { DEC, ZeroPageX1 },
  { EOR, ZeroPageX1 },
  { INC, ZeroPageX1 },
  { LDA, ZeroPageX1 },
  { LDY, ZeroPageX1 },
  { LSR, ZeroPageX1 },
  { ORA, ZeroPageX1 },
  { ROL, ZeroPageX1 },
  { ROR, ZeroPageX1 },
  { SBC, ZeroPageX1 },
  { STA, ZeroPageX1 },
  { STY, ZeroPageX1 },

  { ADC, ZeroPageX2 },
  { AND, ZeroPageX2 },
  { ASL, ZeroPageX2 },
  { CMP, ZeroPageX2 },
  { DEC, ZeroPageX2 },
  { EOR, ZeroPageX2 },
  { INC, ZeroPageX2 },
  { LDA, ZeroPageX2 },
  { LDY, ZeroPageX2 },
  { LSR, ZeroPageX2 },
  { ORA, ZeroPageX2 },
  { ROL, ZeroPageX2 },
  { ROR, ZeroPageX2 },
  { SBC, ZeroPageX2 },
  { STA, ZeroPageX2 },
  { STY, ZeroPageX2 },

  { ADC, ZeroPageX3 },
  { AND, ZeroPageX3 },
  { ASL, ZeroPageX3 },
  { CMP, ZeroPageX3 },
  { DEC, ZeroPageX3 },
  { EOR, ZeroPageX3 },
  { INC, ZeroPageX3 },
  { LDA, ZeroPageX3 },
  { LDY, ZeroPageX3 },
  { LSR, ZeroPageX3 },
  { ORA, ZeroPageX3 },
  { ROL, ZeroPageX3 },
  { ROR, ZeroPageX3 },
  { SBC, ZeroPageX3 },
  { STA, ZeroPageX3 },
  { STY, ZeroPageX3 },

  { LDX, ZeroPageY0 },
  { STX, ZeroPageY0 },
  { LDX, ZeroPageY1 },
  { STX, ZeroPageY1 },
  { LDX, ZeroPageY2 },
  { STX, ZeroPageY2 },
  { LDX, ZeroPageY3 },
  { STX, ZeroPageY3 },

  { ADC, Absolute0 },
  { AND, Absolute0 },
  { ASL, Absolute0 },
  { BIT, Absolute0 },
  { CMP, Absolute0 },
  { CPX, Absolute0 },
  { CPY, Absolute0 },
  { DEC, Absolute0 },
  { EOR, Absolute0 },
  { INC, Absolute0 },
  { JMP, Absolute0 },
  { LDA, Absolute0 },
  { LDX, Absolute0 },
  { LDY, Absolute0 },
  { LSR, Absolute0 },
  { ORA, Absolute0 },
  { ROL, Absolute0 },
  { ROR, Absolute0 },
  { SBC, Absolute0 },
  { STA, Absolute0 },
  { STX, Absolute0 },
  { STY, Absolute0 },

  { ADC, Absolute1 },
  { AND, Absolute1 },
  { ASL, Absolute1 },
  { BIT, Absolute1 },
  { CMP, Absolute1 },
  { CPX, Absolute1 },
  { CPY, Absolute1 },
  { DEC, Absolute1 },
  { EOR, Absolute1 },
  { INC, Absolute1 },
  { JMP, Absolute1 },
  { LDA, Absolute1 },
  { LDX, Absolute1 },
  { LDY, Absolute1 },
  { LSR, Absolute1 },
  { ORA, Absolute1 },
  { ROL, Absolute1 },
  { ROR, Absolute1 },
  { SBC, Absolute1 },
  { STA, Absolute1 },
  { STX, Absolute1 },
  { STY, Absolute1 },

  { ADC, Absolute2 },
  { AND, Absolute2 },
  { ASL, Absolute2 },
  { BIT, Absolute2 },
  { CMP, Absolute2 },
  { CPX, Absolute2 },
  { CPY, Absolute2 },
  { DEC, Absolute2 },
  { EOR, Absolute2 },
  { INC, Absolute2 },
  { JMP, Absolute2 },
  { LDA, Absolute2 },
  { LDX, Absolute2 },
  { LDY, Absolute2 },
  { LSR, Absolute2 },
  { ORA, Absolute2 },
  { ROL, Absolute2 },
  { ROR, Absolute2 },
  { SBC, Absolute2 },
  { STA, Absolute2 },
  { STX, Absolute2 },
  { STY, Absolute2 },

  { ADC, AbsoluteX0 },
  { AND, AbsoluteX0 },
  { ASL, AbsoluteX0 },
  { CMP, AbsoluteX0 },
  { DEC, AbsoluteX0 },
  { EOR, AbsoluteX0 },
  { INC, AbsoluteX0 },
  { LDA, AbsoluteX0 },
  { LDY, AbsoluteX0 },
  { LSR, AbsoluteX0 },
  { ORA, AbsoluteX0 },
  { ROL, AbsoluteX0 },
  { ROR, AbsoluteX0 },
  { SBC, AbsoluteX0 },
  { STA, AbsoluteX0 },

  { ADC, AbsoluteX1 },
  { AND, AbsoluteX1 },
  { ASL, AbsoluteX1 },
  { CMP, AbsoluteX1 },
  { DEC, AbsoluteX1 },
  { EOR, AbsoluteX1 },
  { INC, AbsoluteX1 },
  { LDA, AbsoluteX1 },
  { LDY, AbsoluteX1 },
  { LSR, AbsoluteX1 },
  { ORA, AbsoluteX1 },
  { ROL, AbsoluteX1 },
  { ROR, AbsoluteX1 },
  { SBC, AbsoluteX1 },
  { STA, AbsoluteX1 },

  { ADC, AbsoluteX2 },
  { AND, AbsoluteX2 },
  { ASL, AbsoluteX2 },
  { CMP, AbsoluteX2 },
  { DEC, AbsoluteX2 },
  { EOR, AbsoluteX2 },
  { INC, AbsoluteX2 },
  { LDA, AbsoluteX2 },
  { LDY, AbsoluteX2 },
  { LSR, AbsoluteX2 },
  { ORA, AbsoluteX2 },
  { ROL, AbsoluteX2 },
  { ROR, AbsoluteX2 },
  { SBC, AbsoluteX2 },
  { STA, AbsoluteX2 },

  { ADC, AbsoluteY0 },
  { AND, AbsoluteY0 },
  { EOR, AbsoluteY0 },
  { LDA, AbsoluteY0 },
  { LDX, AbsoluteY0 },
  { ORA, AbsoluteY0 },
  { SBC, AbsoluteY0 },
  { STA, AbsoluteY0 },

  { ADC, AbsoluteY1 },
  { AND, AbsoluteY1 },
  { EOR, AbsoluteY1 },
  { LDA, AbsoluteY1 },
  { LDX, AbsoluteY1 },
  { ORA, AbsoluteY1 },
  { SBC, AbsoluteY1 },
  { STA, AbsoluteY1 },

  { ADC, AbsoluteY2 },
  { AND, AbsoluteY2 },
  { EOR, AbsoluteY2 },
  { LDA, AbsoluteY2 },
  { LDX, AbsoluteY2 },
  { ORA, AbsoluteY2 },
  { SBC, AbsoluteY2 },
  { STA, AbsoluteY2 },

  { ADC, IndirectX0 },
  { AND, IndirectX0 },
  { CMP, IndirectX0 },
  { EOR, IndirectX0 },
  { LDA, IndirectX0 },
  { ORA, IndirectX0 },
  { SBC, IndirectX0 },
  { STA, IndirectX0 },

  { ADC, IndirectX1 },
  { AND, IndirectX1 },
  { CMP, IndirectX1 },
  { EOR, IndirectX1 },
  { LDA, IndirectX1 },
  { ORA, IndirectX1 },
  { SBC, IndirectX1 },
  { STA, IndirectX1 },

  { ADC, IndirectX2 },
  { AND, IndirectX2 },
  { CMP, IndirectX2 },
  { EOR, IndirectX2 },
  { LDA, IndirectX2 },
  { ORA, IndirectX2 },
  { SBC, IndirectX2 },
  { STA, IndirectX2 },

  { ADC, IndirectX3 },
  { AND, IndirectX3 },
  { CMP, IndirectX3 },
  { EOR, IndirectX3 },
  { LDA, IndirectX3 },
  { ORA, IndirectX3 },
  { SBC, IndirectX3 },
  { STA, IndirectX3 },

  { ADC, IndirectY0 },
  { AND, IndirectY0 },
  { CMP, IndirectY0 },
  { EOR, IndirectY0 },
  { LDA, IndirectY0 },
  { ORA, IndirectY0 },
  { SBC, IndirectY0 },
  { STA, IndirectY0 },

  { ADC, IndirectY1 },
  { AND, IndirectY1 },
  { CMP, IndirectY1 },
  { EOR, IndirectY1 },
  { LDA, IndirectY1 },
  { ORA, IndirectY1 },
  { SBC, IndirectY1 },
  { STA, IndirectY1 },

  { ADC, IndirectY2 },
  { AND, IndirectY2 },
  { CMP, IndirectY2 },
  { EOR, IndirectY2 },
  { LDA, IndirectY2 },
  { ORA, IndirectY2 },
  { SBC, IndirectY2 },
  { STA, IndirectY2 },

  { ADC, IndirectY3 },
  { AND, IndirectY3 },
  { CMP, IndirectY3 },
  { EOR, IndirectY3 },
  { LDA, IndirectY3 },
  { ORA, IndirectY3 },
  { SBC, IndirectY3 },
  { STA, IndirectY3 },

  { JMP, Indirect0 },
  { JMP, Indirect1 },
  { JMP, Indirect2 },

  { BPL, Absolute0 },
  { BMI, Absolute0 },
  { BVC, Absolute0 },
  { BVS, Absolute0 },
  { BCC, Absolute0 },
  { BCS, Absolute0 },
  { BNE, Absolute0 },
  { BEQ, Absolute0 },

  { BPL, Absolute1 },
  { BMI, Absolute1 },
  { BVC, Absolute1 },
  { BVS, Absolute1 },
  { BCC, Absolute1 },
  { BCS, Absolute1 },
  { BNE, Absolute1 },
  { BEQ, Absolute1 },

  { BPL, Absolute2 },
  { BMI, Absolute2 },
  { BVC, Absolute2 },
  { BVS, Absolute2 },
  { BCC, Absolute2 },
  { BCS, Absolute2 },
  { BNE, Absolute2 },
  { BEQ, Absolute2 },

  { ASL_A, None },
  { CLC, None },
  { SEC, None },
  { CLI, None },
  { SEI, None },
  { CLV, None },
  { CLD, None },
  { SED, None },
  { LSR_A, None },
  // { NOP, None },
  { TAX, None },
  { TXA, None },
  { DEX, None },
  { INX, None },
  { TAY, None },
  { TYA, None },
  { DEY, None },
  { INY, None },
  { ROL_A, None },
  { ROR_A, None },
  { RTI, None },
  { RTS, None },
  { TXS, None },
  { TSX, None },
  { PHA, None },
  { PLA, None },
  { PHP, None },
  { PLP, None },
};

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
    return fnv(addr) ^ seed;
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

z3::expr mkArray(z3::context& c) {
  z3::sort byte = c.bv_sort(8);
  z3::sort word = c.bv_sort(16);
  z3::sort array = c.array_sort(word, byte);
  return c.constant("memory", array);
}

struct abstract_machine {

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
    _earlyExit(c.bv_const("earlyExit", 17)),
    _memory(mkArray(c)),
    c(c) {}

  z3::expr write(z3::expr const & addr, z3::expr const & val) {
    return _memory = E(z3::store(_memory, addr, val),_memory);
  }

  z3::expr read(z3::expr const & addr) {
    return z3::select(_memory, addr);
  }

  z3::expr extend(z3::expr const & val) {
    Z3_ast r = Z3_mk_zero_ext(c, 8, val);
    return z3::expr(val.ctx(), r);
  }

  z3::expr ite(z3::expr const & c, z3::expr const & t, z3::expr const & e) {
    z3::check_context(c, t); z3::check_context(c, e);
    assert(c.is_bool());
    Z3_ast r = Z3_mk_ite(c.ctx(), c, t, e);
    c.check_error();
    return z3::expr(c.ctx(), r);
  }

  z3::expr ite(z3::expr const & cond, uint8_t t, uint8_t e) {
    return ite(cond, c.num_val(t, _a.get_sort()), c.num_val(e, _a.get_sort()));
  }

  z3::expr shl(z3::expr const & val, int const amt) {
    return z3::to_expr(val.ctx(), Z3_mk_bvshl(val.ctx(), val, val.ctx().num_val(amt, val.get_sort())));
  }

  z3::expr shr(z3::expr const & val, int const amt) {
    return z3::to_expr(val.ctx(), Z3_mk_bvlshr(val.ctx(), val, val.ctx().num_val(amt, val.get_sort())));
  }

  z3::expr lobyte(z3::expr const & val) {
    return val.extract(7, 0);
  }

  z3::expr hibyte(z3::expr const & val) {
    return val.extract(15, 8);
  }

  z3::expr uge(z3::expr const & first, z3::expr const & second) {
    return z3::uge(first, second);
  }

  z3::expr uge(z3::expr const & first, uint8_t second) {
    return z3::uge(first, second);
  }

  void rts() {
    earlyExit(c.num_val(0x00001, _earlyExit.get_sort()));
  }
  void rti() {
    earlyExit(c.num_val(0x00002, _earlyExit.get_sort()));
  }
  void jmp(z3::expr const & target) {
    Z3_ast r = Z3_mk_zero_ext(c, 1, target);
    z3::expr target2(c, r);
    earlyExit(target2 | 0x10000);
  }

  void branch(z3::expr cond, z3::expr const & target) {
    Z3_ast r = Z3_mk_zero_ext(c, 1, target);
    z3::expr target2(c, r);
    earlyExit(ite(cond, target2, _earlyExit));
  }

  z3::expr setSZ(z3::expr const & val) {
    ccS(z3::uge(val, 0x80));
    ccZ(val == 0);
    return val;
  }

  z3::context& c;

  z3::expr falsy;
  z3::expr truthy;

  z3::expr absolute0;
  z3::expr absolute1;
  z3::expr absolute2;
  z3::expr zp0;
  z3::expr zp1;
  z3::expr zp2;
  z3::expr zp3;
  z3::expr c0;
  z3::expr c1;
  z3::expr literal0;
  z3::expr literal1;

  z3::expr E(z3::expr normal, z3::expr same) {
    return ite(0x0 == _earlyExit, normal, same);
  }

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

  z3::expr _earlyExit; z3::expr earlyExit(z3::expr const & val) { return _earlyExit = E(val, _earlyExit); }
};

void addEquivalence(abstract_machine & m1, concrete_machine & m2, z3::solver & s) {
  //std::cout << "substitute example\n";
  z3::context & c = s.ctx();
  z3::expr x = m1._a;
  //std::cout << x << std::endl;

  z3::expr a(c), initialA(c);
  a   = c.bv_const("a", 8);
  initialA = c.num_val(m2._a, a.get_sort());
  Z3_ast from[] = { a };
  Z3_ast to[]   = { initialA };
  z3::expr new_f(c);
  new_f = to_expr(c, Z3_substitute(c, x, 1, from, to));
  new_f = new_f.simplify();
  //std::cout << new_f << std::endl;
}

uint64_t inst2int(opcode op) {
  return (op.op << 8) | op.mode;
}

uint64_t inst2int(opcode op1, opcode op2) {
  return (inst2int(op2) << 16) | inst2int(op1); 
}

uint64_t inst2int(opcode op1, opcode op2, opcode op3) {
  return (inst2int(op3) << 32) | (inst2int(op2) << 16) | inst2int(op1); 
}

uint8_t length(uint64_t op) {
  return !(op & 0xFFFFL) ? 0 :
         !(op & 0xFFFF0000L) ? 1 :
         !(op & 0xFFFF00000000L) ? 2 : 3;
}

opcode int2inst(uint64_t i) {
  return opcode { (Operations)((i >> 8) & 0xFF), (AddrMode)(i & 0xFF) };
}

std::tuple<opcode, opcode, opcode> int2tuple(uint64_t i) {
  return std::make_tuple(int2inst(i), int2inst(i>>16), int2inst(i>>32));
}

bool equivalent(z3::context &c, instruction3 a, instruction3 b) {
  constexpr opcode zero_op { (Operations)0, (AddrMode)0 };
  constexpr emulator<abstract_machine> emu;
  abstract_machine ma(c);
  abstract_machine mb(c);
  auto inst = std::get<0>(a);
  emu.instruction(ma, inst.op, inst.mode);
  inst = std::get<1>(a);
  if (inst != zero_op) {
    emu.instruction(ma, inst.op, inst.mode);
  }
  inst = std::get<2>(a);
  if (inst != zero_op) {
    emu.instruction(ma, inst.op, inst.mode);
  }
  
  inst = std::get<0>(b);
  emu.instruction(mb, inst.op, inst.mode);
  inst = std::get<1>(b);
  if (inst != zero_op) {
    emu.instruction(mb, inst.op, inst.mode);
  }
  inst = std::get<2>(b);
  if (inst != zero_op) {
    emu.instruction(mb, inst.op, inst.mode);
  }

  z3::solver s(c);
  s.add(!(
    ma._memory == mb._memory &&
    ma._a == mb._a &&
    ma._x == mb._x &&
    ma._y == mb._y &&
    ma._sp == mb._sp &&
    ma._ccS == mb._ccS &&
    ma._ccV == mb._ccV &&
    ma._ccD == mb._ccD &&
    ma._ccI == mb._ccI &&
    ma._ccC == mb._ccC &&
    ma._ccZ == mb._ccZ &&
    ma._earlyExit == mb._earlyExit
  ));
  
  if (s.check() == z3::unsat) {
    std::cout << "Same!" << std::endl;
    return true;
  }
  //std::cout << "Different" << std::endl;
  return false;
}

void print(opcode op) {
  std::cout << OpNames[op.op] << AddrModeNames[op.mode];
}

void print(instruction3 ops) {
  opcode zero { (Operations)0, (AddrMode)0 };
  if (std::get<0>(ops) != zero) { print(std::get<0>(ops)); }
  if (std::get<1>(ops) != zero) {
    std::cout << "; ";
    print(std::get<1>(ops));
  }
  if (std::get<2>(ops) != zero) {
    std::cout << "; ";
    print(std::get<2>(ops));
  }
}

bool isCanonical(instruction3 ops) {
  opcode zero { (Operations)0, (AddrMode)0 };
  opcode one = std::get<0>(ops);
  opcode two = std::get<1>(ops);
  opcode three = std::get<2>(ops);
  opcode opsArr[3] { one, two, three };
  int abs = 7;
  int zp = 0xA;
  bool c0 = false;
  for (int i = 0; i < 3; i++) {
    if (opsArr[i] == zero) { break; }
    uint8_t val = opsArr[i].op & 0x0F;
    switch (opsArr[i].op & 0xF0) {
    case 0: // Immediate
      if (val == 0 || 
          val == 4 || 
          val == 6) {
        c0 = true;
      } else if (!c0 && (val == 1 || val == 5)) {
        return false;
      }
      break;
    case 1: // Absolute
    case 2: // AbsoluteX
    case 3: // AbsoluteY
    case 7: // Indirect
      if (val > abs) { return false; }
      else if (val == abs) { abs++; }
      break;
    case 4: // ZeroPage
    case 5: // ZeroPageX
    case 6: // ZeroPageY
    case 8: // IndirectX
    case 9: // IndirectY
      if (val > zp) { return false; }
      else if (val == zp) { zp++; }
      break;
    }
  }
  return true;
}

void enumerate() {
  std::multimap<uint32_t, uint64_t> buckets;
  
  emulator<concrete_machine> emu;
  constexpr uint8_t nMachines = 32;

  for (uint32_t i = 0; i < sizeof(opcodes)/sizeof(opcodes[0]); i++) {
    uint32_t hash = 0;
    for (uint8_t m = 0; m < nMachines; m++) {
      concrete_machine machine(0xFFA4BCAD + m);
      emu.instruction(machine, opcodes[i].op, opcodes[i].mode);
      hash ^= machine.hash();
    }
    buckets.insert(std::make_pair(hash, inst2int(opcodes[i])));
  }
  for (uint32_t i = 0; i < sizeof(opcodes)/sizeof(opcodes[0]); i++) {
    for (uint32_t j = 0; j < sizeof(opcodes)/sizeof(opcodes[0]); j++) {
      uint32_t hash = 0;
      for (uint8_t m = 0; m < nMachines; m++) {
        concrete_machine machine(0xFFA4BCAD + m);
        emu.instruction(machine, opcodes[i].op, opcodes[i].mode);
        emu.instruction(machine, opcodes[j].op, opcodes[j].mode);
        hash ^= machine.hash();
      }
      buckets.insert(std::make_pair(hash, inst2int(opcodes[i], opcodes[j])));
    }
  }

  /*for (uint32_t i = 0; i < sizeof(opcodes)/sizeof(opcodes[0]); i++) {
    for (uint32_t j = 0; j < sizeof(opcodes)/sizeof(opcodes[0]); j++) {
      for (uint32_t k = 0; k < sizeof(opcodes)/sizeof(opcodes[0]); k++) {
        uint32_t hash = 0;
        for (uint8_t m = 0; m < nMachines; m++) {
          concrete_machine machine(0xFFA4BCAD + m);
          emu.instruction(machine, opcodes[i].op, opcodes[i].mode);
          emu.instruction(machine, opcodes[j].op, opcodes[j].mode);
          emu.instruction(machine, opcodes[k].op, opcodes[k].mode);
          hash ^= machine.hash();
        }
        buckets.insert(std::make_pair(hash, inst2int(opcodes[i], opcodes[j], opcodes[k])));
      }
    }
  }*/
 
  z3::context c;

  uint32_t last = 0;
  std::vector<uint64_t> sequences;
  for (const auto &p : buckets) {
    if (p.first != last) {
      if (sequences.size() > 1) {
        int l = length(sequences[0]);
        bool differentLengths = false;
        for (auto sequence : sequences) {
          if (length(sequence) != l) {
            differentLengths = true;
            break;
          }
        }
        bool hasCanonical = false;
        for (auto sequence : sequences) {
          auto tup = int2tuple(sequence);
          if (isCanonical(tup)) {
            hasCanonical = true;
            break;
          }
        }
        if (differentLengths && hasCanonical) {
          for (auto sequence : sequences) {
            auto tup = int2tuple(sequence);
            for (auto sequence2 : sequences) {
              if (sequence != sequence2) {
                if (equivalent(c, tup, int2tuple(sequence2))) {
                  print(tup); std::cout << std::endl;
                  print(int2tuple(sequence2)); std::cout << std::endl;
                }
              }
            }
          }
          std::cout << "---------------" << std::endl;
        }
      }
      sequences.clear();
    }
    sequences.push_back(p.second);
    last = p.first;
  }
}

/*void loadEmulator() {
  z3::context c;

  {
    z3::solver s(c);
    // ensure abstract and concrete machines are the same.
    for (int seed = 0; seed < 200; seed += 13) {
      std::cout << "seed: " << seed << std::endl;
      
      concrete_machine initialMachine(seed);
      emulator<concrete_machine> c_emu;
      emulator<abstract_machine> a_emu;

      z3::expr a = c.num_val(initialMachine._a, c.bv_sort(8));
      z3::expr x = c.num_val(initialMachine._x, c.bv_sort(8));
      z3::expr y = c.num_val(initialMachine._y, c.bv_sort(8));
      z3::expr sp = c.num_val(initialMachine._sp, c.bv_sort(8));
      z3::expr ccS = c.bool_val(initialMachine._ccS);
      z3::expr ccV = c.bool_val(initialMachine._ccV);
      z3::expr ccC = c.bool_val(initialMachine._ccC);
      z3::expr ccD = c.bool_val(initialMachine._ccD);
      z3::expr ccI = c.bool_val(initialMachine._ccI);
      z3::expr ccZ = c.bool_val(initialMachine._ccZ);
      z3::expr absolute0 = c.num_val(initialMachine.absolute0, c.bv_sort(16));
      z3::expr absolute1 = c.num_val(initialMachine.absolute1, c.bv_sort(16));
      z3::expr absolute2 = c.num_val(initialMachine.absolute2, c.bv_sort(16));
      z3::expr zp0 = c.num_val(initialMachine.zp0, c.bv_sort(8));
      z3::expr zp1 = c.num_val(initialMachine.zp1, c.bv_sort(8));
      z3::expr zp2 = c.num_val(initialMachine.zp2, c.bv_sort(8));
      z3::expr zp3 = c.num_val(initialMachine.zp3, c.bv_sort(8));
      z3::expr c0 = c.num_val(initialMachine.c0, c.bv_sort(8));
      z3::expr c1 = c.num_val(initialMachine.c1, c.bv_sort(8));
      z3::expr memory = mkArray(c);
      for (uint32_t i = 0; i < 0x10000; i++) {
        memory = z3::store(memory, i, initialMachine.read(i));
      }
      for (uint32_t i = 0; i < sizeof(opcodes)/sizeof(opcodes[0]); i++) {
        abstract_machine m1(c);
        concrete_machine m2(seed);
        m1._a = a;
        m1._x = x;
        m1._y = y;
        m1._sp = sp;
        m1._ccS = ccS;
        m1._ccV = ccV;
        m1._ccC = ccC;
        m1._ccD = ccD;
        m1._ccI = ccI;
        m1._ccZ = ccZ;
        m1.absolute0 = absolute0;
        m1.absolute1 = absolute1;
        m1.absolute2 = absolute2;
        m1.zp0 = zp0;
        m1.zp1 = zp1;
        m1.zp2 = zp2;
        m1.zp3 = zp3;
        m1.c0 = c0;
        m1.c1 = c1;
        m1._memory = memory;
        
        a_emu.instruction(m1, opcodes[i].op, opcodes[i].mode);
        c_emu.instruction(m2, opcodes[i].op, opcodes[i].mode);
        addEquivalence(m1, initialMachine, s);
        if (s.check() != z3::sat) {
          std::cout << "different: " << OpNames[opcodes[i].op] << " " << opcodes[i].mode << std::endl;
        }
      }
    }
  }

  for (uint32_t i = 0; i < sizeof(opcodes)/sizeof(opcodes[0]); i++) {
    abstract_machine m1(c);

    e1.instruction(opcodes[i].op, opcodes[i].mode);
    z3::solver s(c);


    concrete_machine cm1(0);
    a_emu.instruction(m1, opcodes[i].op, opcodes[i].mode);
    for (int j = 0; j < i; j++) {
 
      concrete_machine cm2(0);
      emulator<concrete_machine> ce2(cm2);
      ce2.instruction(opcodes[j].op, opcodes[j].mode);

      if (cm1._a != cm2._a || cm1._x != cm2._x || cm1._y != cm2._y) {
        continue;
      }
   
      abstract_machine m2(c);
      emulator<abstract_machine> e2(m2);

      e2.instruction(opcodes[j].op, opcodes[j].mode);

      s.push();
      s.add(!(
        m1._memory == m2._memory &&
        m1._a == m2._a &&
        m1._x == m2._x &&
        m1._y == m2._y &&
        m1._sp == m2._sp &&
        m1._ccS == m2._ccS &&
        m1._ccV == m2._ccV &&
        m1._ccD == m2._ccD &&
        m1._ccI == m2._ccI &&
        m1._ccC == m2._ccC &&
        m1._ccZ == m2._ccZ &&
        m1._earlyExit == m2._earlyExit
      ));

      if (s.check() == z3::unsat) {
        std::cout << "Same: " << OpNames[(int)opcodes[i].op] << "," << AddrModeNames[opcodes[i].mode] << std::endl;;
        std::cout << "and:  " << OpNames[(int)opcodes[j].op] << "," << (int)opcodes[j].mode << std::endl;;
      }
      s.pop();
    }
  }
}*/

int main() {
  try {
    enumerate();
  } catch (z3::exception & ex) {
    std::cout << "unexpected error: " << ex << "\n";
  }
}
