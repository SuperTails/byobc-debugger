#include "physicalw65c02.h"

#define assert(X) do { if (!(X)) { while (1) { asm volatile ("nop"); } } } while (0)

#define UNREACHABLE() do { assert(false && "unreachable"); } while (0)

#define W65C02S_OPCODE_TABLE()                                                 \
W65C02S_OPCODE(0x00, STACK_BRK,                 W65C02S_OPER_BRK) /* BRK brk */\
W65C02S_OPCODE(0x01, ZEROPAGE_INDIRECT_X,       W65C02S_OPER_ORA) /* ORA zix */\
W65C02S_OPCODE(0x02, IMMEDIATE,                 W65C02S_OPER_NOP) /* NOP imm */\
W65C02S_OPCODE(0x03, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x04, RMW_ZEROPAGE,              W65C02S_OPER_TSB) /* TSB wzp */\
W65C02S_OPCODE(0x05, ZEROPAGE,                  W65C02S_OPER_ORA) /* ORA zpg */\
W65C02S_OPCODE(0x06, RMW_ZEROPAGE,              W65C02S_OPER_ASL) /* ASL wzp */\
W65C02S_OPCODE(0x07, ZEROPAGE_BIT,              000)              /* 000 zpb */\
W65C02S_OPCODE(0x08, STACK_PUSH,                W65C02S_OPER_PHP) /* PHP phv */\
W65C02S_OPCODE(0x09, IMMEDIATE,                 W65C02S_OPER_ORA) /* ORA imm */\
W65C02S_OPCODE(0x0A, IMPLIED,                   W65C02S_OPER_ASL) /* ASL imp */\
W65C02S_OPCODE(0x0B, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x0C, RMW_ABSOLUTE,              W65C02S_OPER_TSB) /* TSB wab */\
W65C02S_OPCODE(0x0D, ABSOLUTE,                  W65C02S_OPER_ORA) /* ORA abs */\
W65C02S_OPCODE(0x0E, RMW_ABSOLUTE,              W65C02S_OPER_ASL) /* ASL wab */\
W65C02S_OPCODE(0x0F, RELATIVE_BIT,              000)              /* 000 rlb */\
W65C02S_OPCODE(0x10, RELATIVE,                  W65C02S_OPER_BPL) /* BPL rel */\
W65C02S_OPCODE(0x11, ZEROPAGE_INDIRECT_Y,       W65C02S_OPER_ORA) /* ORA ziy */\
W65C02S_OPCODE(0x12, ZEROPAGE_INDIRECT,         W65C02S_OPER_ORA) /* ORA zpi */\
W65C02S_OPCODE(0x13, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x14, RMW_ZEROPAGE,              W65C02S_OPER_TRB) /* TRB wzp */\
W65C02S_OPCODE(0x15, ZEROPAGE_X,                W65C02S_OPER_ORA) /* ORA zpx */\
W65C02S_OPCODE(0x16, RMW_ZEROPAGE_X,            W65C02S_OPER_ASL) /* ASL wzx */\
W65C02S_OPCODE(0x17, ZEROPAGE_BIT,              001)              /* 001 zpb */\
W65C02S_OPCODE(0x18, IMPLIED,                   W65C02S_OPER_CLC) /* CLC imp */\
W65C02S_OPCODE(0x19, ABSOLUTE_Y,                W65C02S_OPER_ORA) /* ORA aby */\
W65C02S_OPCODE(0x1A, IMPLIED,                   W65C02S_OPER_INC) /* INC imp */\
W65C02S_OPCODE(0x1B, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x1C, RMW_ABSOLUTE,              W65C02S_OPER_TRB) /* TRB wab */\
W65C02S_OPCODE(0x1D, ABSOLUTE_X,                W65C02S_OPER_ORA) /* ORA abx */\
W65C02S_OPCODE(0x1E, RMW_ABSOLUTE_X,            W65C02S_OPER_ASL) /* ASL wax */\
W65C02S_OPCODE(0x1F, RELATIVE_BIT,              001)              /* 001 rlb */\
W65C02S_OPCODE(0x20, SUBROUTINE,                W65C02S_OPER_JSR) /* JSR sbj */\
W65C02S_OPCODE(0x21, ZEROPAGE_INDIRECT_X,       W65C02S_OPER_AND) /* AND zix */\
W65C02S_OPCODE(0x22, IMMEDIATE,                 W65C02S_OPER_NOP) /* NOP imm */\
W65C02S_OPCODE(0x23, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x24, ZEROPAGE,                  W65C02S_OPER_BIT) /* BIT zpg */\
W65C02S_OPCODE(0x25, ZEROPAGE,                  W65C02S_OPER_AND) /* AND zpg */\
W65C02S_OPCODE(0x26, RMW_ZEROPAGE,              W65C02S_OPER_ROL) /* ROL wzp */\
W65C02S_OPCODE(0x27, ZEROPAGE_BIT,              002)              /* 002 zpb */\
W65C02S_OPCODE(0x28, STACK_PULL,                W65C02S_OPER_PLP) /* PLP plv */\
W65C02S_OPCODE(0x29, IMMEDIATE,                 W65C02S_OPER_AND) /* AND imm */\
W65C02S_OPCODE(0x2A, IMPLIED,                   W65C02S_OPER_ROL) /* ROL imp */\
W65C02S_OPCODE(0x2B, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x2C, ABSOLUTE,                  W65C02S_OPER_BIT) /* BIT abs */\
W65C02S_OPCODE(0x2D, ABSOLUTE,                  W65C02S_OPER_AND) /* AND abs */\
W65C02S_OPCODE(0x2E, RMW_ABSOLUTE,              W65C02S_OPER_ROL) /* ROL wab */\
W65C02S_OPCODE(0x2F, RELATIVE_BIT,              002)              /* 002 rlb */\
W65C02S_OPCODE(0x30, RELATIVE,                  W65C02S_OPER_BMI) /* BMI rel */\
W65C02S_OPCODE(0x31, ZEROPAGE_INDIRECT_Y,       W65C02S_OPER_AND) /* AND ziy */\
W65C02S_OPCODE(0x32, ZEROPAGE_INDIRECT,         W65C02S_OPER_AND) /* AND zpi */\
W65C02S_OPCODE(0x33, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x34, ZEROPAGE_X,                W65C02S_OPER_BIT) /* BIT zpx */\
W65C02S_OPCODE(0x35, ZEROPAGE_X,                W65C02S_OPER_AND) /* AND zpx */\
W65C02S_OPCODE(0x36, RMW_ZEROPAGE_X,            W65C02S_OPER_ROL) /* ROL wzx */\
W65C02S_OPCODE(0x37, ZEROPAGE_BIT,              003)              /* 003 zpb */\
W65C02S_OPCODE(0x38, IMPLIED,                   W65C02S_OPER_SEC) /* SEC imp */\
W65C02S_OPCODE(0x39, ABSOLUTE_Y,                W65C02S_OPER_AND) /* AND aby */\
W65C02S_OPCODE(0x3A, IMPLIED,                   W65C02S_OPER_DEC) /* DEC imp */\
W65C02S_OPCODE(0x3B, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x3C, ABSOLUTE_X,                W65C02S_OPER_BIT) /* BIT abx */\
W65C02S_OPCODE(0x3D, ABSOLUTE_X,                W65C02S_OPER_AND) /* AND abx */\
W65C02S_OPCODE(0x3E, RMW_ABSOLUTE_X,            W65C02S_OPER_ROL) /* ROL wax */\
W65C02S_OPCODE(0x3F, RELATIVE_BIT,              003)              /* 003 rlb */\
W65C02S_OPCODE(0x40, STACK_RTI,                 W65C02S_OPER_RTI) /* RTI rti */\
W65C02S_OPCODE(0x41, ZEROPAGE_INDIRECT_X,       W65C02S_OPER_EOR) /* EOR zix */\
W65C02S_OPCODE(0x42, IMMEDIATE,                 W65C02S_OPER_NOP) /* NOP imm */\
W65C02S_OPCODE(0x43, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x44, ZEROPAGE,                  W65C02S_OPER_NOP) /* NOP zpg */\
W65C02S_OPCODE(0x45, ZEROPAGE,                  W65C02S_OPER_EOR) /* EOR zpg */\
W65C02S_OPCODE(0x46, RMW_ZEROPAGE,              W65C02S_OPER_LSR) /* LSR wzp */\
W65C02S_OPCODE(0x47, ZEROPAGE_BIT,              004)              /* 004 zpb */\
W65C02S_OPCODE(0x48, STACK_PUSH,                W65C02S_OPER_PHA) /* PHA phv */\
W65C02S_OPCODE(0x49, IMMEDIATE,                 W65C02S_OPER_EOR) /* EOR imm */\
W65C02S_OPCODE(0x4A, IMPLIED,                   W65C02S_OPER_LSR) /* LSR imp */\
W65C02S_OPCODE(0x4B, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x4C, ABSOLUTE_JUMP,             W65C02S_OPER_JMP) /* JMP abj */\
W65C02S_OPCODE(0x4D, ABSOLUTE,                  W65C02S_OPER_EOR) /* EOR abs */\
W65C02S_OPCODE(0x4E, RMW_ABSOLUTE,              W65C02S_OPER_LSR) /* LSR wab */\
W65C02S_OPCODE(0x4F, RELATIVE_BIT,              004)              /* 004 rlb */\
W65C02S_OPCODE(0x50, RELATIVE,                  W65C02S_OPER_BVC) /* BVC rel */\
W65C02S_OPCODE(0x51, ZEROPAGE_INDIRECT_Y,       W65C02S_OPER_EOR) /* EOR ziy */\
W65C02S_OPCODE(0x52, ZEROPAGE_INDIRECT,         W65C02S_OPER_EOR) /* EOR zpi */\
W65C02S_OPCODE(0x53, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x54, ZEROPAGE_X,                W65C02S_OPER_NOP) /* NOP zpx */\
W65C02S_OPCODE(0x55, ZEROPAGE_X,                W65C02S_OPER_EOR) /* EOR zpx */\
W65C02S_OPCODE(0x56, RMW_ZEROPAGE_X,            W65C02S_OPER_LSR) /* LSR wzx */\
W65C02S_OPCODE(0x57, ZEROPAGE_BIT,              005)              /* 005 zpb */\
W65C02S_OPCODE(0x58, IMPLIED,                   W65C02S_OPER_CLI) /* CLI imp */\
W65C02S_OPCODE(0x59, ABSOLUTE_Y,                W65C02S_OPER_EOR) /* EOR aby */\
W65C02S_OPCODE(0x5A, STACK_PUSH,                W65C02S_OPER_PHY) /* PHY phv */\
W65C02S_OPCODE(0x5B, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x5C, NOP_5C,                    W65C02S_OPER_NOP) /* NOP x5c */\
W65C02S_OPCODE(0x5D, ABSOLUTE_X,                W65C02S_OPER_EOR) /* EOR abx */\
W65C02S_OPCODE(0x5E, RMW_ABSOLUTE_X,            W65C02S_OPER_LSR) /* LSR wax */\
W65C02S_OPCODE(0x5F, RELATIVE_BIT,              005)              /* 005 rlb */\
W65C02S_OPCODE(0x60, RETURN_SUB,                W65C02S_OPER_RTS) /* RTS sbr */\
W65C02S_OPCODE(0x61, ZEROPAGE_INDIRECT_X,       W65C02S_OPER_ADC) /* ADC zix */\
W65C02S_OPCODE(0x62, IMMEDIATE,                 W65C02S_OPER_NOP) /* NOP imm */\
W65C02S_OPCODE(0x63, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x64, ZEROPAGE,                  W65C02S_OPER_STZ) /* STZ zpg */\
W65C02S_OPCODE(0x65, ZEROPAGE,                  W65C02S_OPER_ADC) /* ADC zpg */\
W65C02S_OPCODE(0x66, RMW_ZEROPAGE,              W65C02S_OPER_ROR) /* ROR wzp */\
W65C02S_OPCODE(0x67, ZEROPAGE_BIT,              006)              /* 006 zpb */\
W65C02S_OPCODE(0x68, STACK_PULL,                W65C02S_OPER_PLA) /* PLA plv */\
W65C02S_OPCODE(0x69, IMMEDIATE,                 W65C02S_OPER_ADC) /* ADC imm */\
W65C02S_OPCODE(0x6A, IMPLIED,                   W65C02S_OPER_ROR) /* ROR imp */\
W65C02S_OPCODE(0x6B, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x6C, ABSOLUTE_INDIRECT,         W65C02S_OPER_JMP) /* JMP ind */\
W65C02S_OPCODE(0x6D, ABSOLUTE,                  W65C02S_OPER_ADC) /* ADC abs */\
W65C02S_OPCODE(0x6E, RMW_ABSOLUTE,              W65C02S_OPER_ROR) /* ROR wab */\
W65C02S_OPCODE(0x6F, RELATIVE_BIT,              006)              /* 006 rlb */\
W65C02S_OPCODE(0x70, RELATIVE,                  W65C02S_OPER_BVS) /* BVS rel */\
W65C02S_OPCODE(0x71, ZEROPAGE_INDIRECT_Y,       W65C02S_OPER_ADC) /* ADC ziy */\
W65C02S_OPCODE(0x72, ZEROPAGE_INDIRECT,         W65C02S_OPER_ADC) /* ADC zpi */\
W65C02S_OPCODE(0x73, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x74, ZEROPAGE_X,                W65C02S_OPER_STZ) /* STZ zpx */\
W65C02S_OPCODE(0x75, ZEROPAGE_X,                W65C02S_OPER_ADC) /* ADC zpx */\
W65C02S_OPCODE(0x76, RMW_ZEROPAGE_X,            W65C02S_OPER_ROR) /* ROR wzx */\
W65C02S_OPCODE(0x77, ZEROPAGE_BIT,              007)              /* 007 zpb */\
W65C02S_OPCODE(0x78, IMPLIED,                   W65C02S_OPER_SEI) /* SEI imp */\
W65C02S_OPCODE(0x79, ABSOLUTE_Y,                W65C02S_OPER_ADC) /* ADC aby */\
W65C02S_OPCODE(0x7A, STACK_PULL,                W65C02S_OPER_PLY) /* PLY plv */\
W65C02S_OPCODE(0x7B, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x7C, ABSOLUTE_INDIRECT_X,       W65C02S_OPER_JMP) /* JMP idx */\
W65C02S_OPCODE(0x7D, ABSOLUTE_X,                W65C02S_OPER_ADC) /* ADC abx */\
W65C02S_OPCODE(0x7E, RMW_ABSOLUTE_X,            W65C02S_OPER_ROR) /* ROR wax */\
W65C02S_OPCODE(0x7F, RELATIVE_BIT,              007)              /* 007 rlb */\
W65C02S_OPCODE(0x80, RELATIVE,                  W65C02S_OPER_BRA) /* BRA rel */\
W65C02S_OPCODE(0x81, ZEROPAGE_INDIRECT_X,       W65C02S_OPER_STA) /* STA zix */\
W65C02S_OPCODE(0x82, IMMEDIATE,                 W65C02S_OPER_NOP) /* NOP imm */\
W65C02S_OPCODE(0x83, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x84, ZEROPAGE,                  W65C02S_OPER_STY) /* STY zpg */\
W65C02S_OPCODE(0x85, ZEROPAGE,                  W65C02S_OPER_STA) /* STA zpg */\
W65C02S_OPCODE(0x86, ZEROPAGE,                  W65C02S_OPER_STX) /* STX zpg */\
W65C02S_OPCODE(0x87, ZEROPAGE_BIT,              010)              /* 010 zpb */\
W65C02S_OPCODE(0x88, IMPLIED_Y,                 W65C02S_OPER_DEC) /* DEC imy */\
W65C02S_OPCODE(0x89, IMMEDIATE,                 W65C02S_OPER_BIT) /* BIT imm */\
W65C02S_OPCODE(0x8A, IMPLIED,                   W65C02S_OPER_TXA) /* TXA imp */\
W65C02S_OPCODE(0x8B, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x8C, ABSOLUTE,                  W65C02S_OPER_STY) /* STY abs */\
W65C02S_OPCODE(0x8D, ABSOLUTE,                  W65C02S_OPER_STA) /* STA abs */\
W65C02S_OPCODE(0x8E, ABSOLUTE,                  W65C02S_OPER_STX) /* STX abs */\
W65C02S_OPCODE(0x8F, RELATIVE_BIT,              010)              /* 010 rlb */\
W65C02S_OPCODE(0x90, RELATIVE,                  W65C02S_OPER_BCC) /* BCC rel */\
W65C02S_OPCODE(0x91, ZEROPAGE_INDIRECT_Y_STORE, W65C02S_OPER_STA) /* STA ziy */\
W65C02S_OPCODE(0x92, ZEROPAGE_INDIRECT,         W65C02S_OPER_STA) /* STA zpi */\
W65C02S_OPCODE(0x93, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x94, ZEROPAGE_X,                W65C02S_OPER_STY) /* STY zpx */\
W65C02S_OPCODE(0x95, ZEROPAGE_X,                W65C02S_OPER_STA) /* STA zpx */\
W65C02S_OPCODE(0x96, ZEROPAGE_Y,                W65C02S_OPER_STX) /* STX zpy */\
W65C02S_OPCODE(0x97, ZEROPAGE_BIT,              011)              /* 011 zpb */\
W65C02S_OPCODE(0x98, IMPLIED,                   W65C02S_OPER_TYA) /* TYA imp */\
W65C02S_OPCODE(0x99, ABSOLUTE_Y_STORE,          W65C02S_OPER_STA) /* STA aby */\
W65C02S_OPCODE(0x9A, IMPLIED,                   W65C02S_OPER_TXS) /* TXS imp */\
W65C02S_OPCODE(0x9B, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0x9C, ABSOLUTE,                  W65C02S_OPER_STZ) /* STZ abs */\
W65C02S_OPCODE(0x9D, ABSOLUTE_X_STORE,          W65C02S_OPER_STA) /* STA abx */\
W65C02S_OPCODE(0x9E, ABSOLUTE_X_STORE,          W65C02S_OPER_STZ) /* STZ abx */\
W65C02S_OPCODE(0x9F, RELATIVE_BIT,              011)              /* 011 rlb */\
W65C02S_OPCODE(0xA0, IMMEDIATE,                 W65C02S_OPER_LDY) /* LDY imm */\
W65C02S_OPCODE(0xA1, ZEROPAGE_INDIRECT_X,       W65C02S_OPER_LDA) /* LDA zix */\
W65C02S_OPCODE(0xA2, IMMEDIATE,                 W65C02S_OPER_LDX) /* LDX imm */\
W65C02S_OPCODE(0xA3, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0xA4, ZEROPAGE,                  W65C02S_OPER_LDY) /* LDY zpg */\
W65C02S_OPCODE(0xA5, ZEROPAGE,                  W65C02S_OPER_LDA) /* LDA zpg */\
W65C02S_OPCODE(0xA6, ZEROPAGE,                  W65C02S_OPER_LDX) /* LDX zpg */\
W65C02S_OPCODE(0xA7, ZEROPAGE_BIT,              012)              /* 012 zpb */\
W65C02S_OPCODE(0xA8, IMPLIED,                   W65C02S_OPER_TAY) /* TAY imp */\
W65C02S_OPCODE(0xA9, IMMEDIATE,                 W65C02S_OPER_LDA) /* LDA imm */\
W65C02S_OPCODE(0xAA, IMPLIED,                   W65C02S_OPER_TAX) /* TAX imp */\
W65C02S_OPCODE(0xAB, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0xAC, ABSOLUTE,                  W65C02S_OPER_LDY) /* LDY abs */\
W65C02S_OPCODE(0xAD, ABSOLUTE,                  W65C02S_OPER_LDA) /* LDA abs */\
W65C02S_OPCODE(0xAE, ABSOLUTE,                  W65C02S_OPER_LDX) /* LDX abs */\
W65C02S_OPCODE(0xAF, RELATIVE_BIT,              012)              /* 012 rlb */\
W65C02S_OPCODE(0xB0, RELATIVE,                  W65C02S_OPER_BCS) /* BCS rel */\
W65C02S_OPCODE(0xB1, ZEROPAGE_INDIRECT_Y,       W65C02S_OPER_LDA) /* LDA ziy */\
W65C02S_OPCODE(0xB2, ZEROPAGE_INDIRECT,         W65C02S_OPER_LDA) /* LDA zpi */\
W65C02S_OPCODE(0xB3, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0xB4, ZEROPAGE_X,                W65C02S_OPER_LDY) /* LDY zpx */\
W65C02S_OPCODE(0xB5, ZEROPAGE_X,                W65C02S_OPER_LDA) /* LDA zpx */\
W65C02S_OPCODE(0xB6, ZEROPAGE_Y,                W65C02S_OPER_LDX) /* LDX zpy */\
W65C02S_OPCODE(0xB7, ZEROPAGE_BIT,              013)              /* 013 zpb */\
W65C02S_OPCODE(0xB8, IMPLIED,                   W65C02S_OPER_CLV) /* CLV imp */\
W65C02S_OPCODE(0xB9, ABSOLUTE_Y,                W65C02S_OPER_LDA) /* LDA aby */\
W65C02S_OPCODE(0xBA, IMPLIED,                   W65C02S_OPER_TSX) /* TSX imp */\
W65C02S_OPCODE(0xBB, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0xBC, ABSOLUTE_X,                W65C02S_OPER_LDY) /* LDY abx */\
W65C02S_OPCODE(0xBD, ABSOLUTE_X,                W65C02S_OPER_LDA) /* LDA abx */\
W65C02S_OPCODE(0xBE, ABSOLUTE_Y,                W65C02S_OPER_LDX) /* LDX aby */\
W65C02S_OPCODE(0xBF, RELATIVE_BIT,              013)              /* 013 rlb */\
W65C02S_OPCODE(0xC0, IMMEDIATE,                 W65C02S_OPER_CPY) /* CPY imm */\
W65C02S_OPCODE(0xC1, ZEROPAGE_INDIRECT_X,       W65C02S_OPER_CMP) /* CMP zix */\
W65C02S_OPCODE(0xC2, IMMEDIATE,                 W65C02S_OPER_NOP) /* NOP imm */\
W65C02S_OPCODE(0xC3, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0xC4, ZEROPAGE,                  W65C02S_OPER_CPY) /* CPY zpg */\
W65C02S_OPCODE(0xC5, ZEROPAGE,                  W65C02S_OPER_CMP) /* CMP zpg */\
W65C02S_OPCODE(0xC6, RMW_ZEROPAGE,              W65C02S_OPER_DEC) /* DEC wzp */\
W65C02S_OPCODE(0xC7, ZEROPAGE_BIT,              014)              /* 014 zpb */\
W65C02S_OPCODE(0xC8, IMPLIED_Y,                 W65C02S_OPER_INC) /* INC imy */\
W65C02S_OPCODE(0xC9, IMMEDIATE,                 W65C02S_OPER_CMP) /* CMP imm */\
W65C02S_OPCODE(0xCA, IMPLIED_X,                 W65C02S_OPER_DEC) /* DEC imx */\
W65C02S_OPCODE(0xCB, INT_WAIT_STOP,             W65C02S_OPER_WAI) /* WAI wai */\
W65C02S_OPCODE(0xCC, ABSOLUTE,                  W65C02S_OPER_CPY) /* CPY abs */\
W65C02S_OPCODE(0xCD, ABSOLUTE,                  W65C02S_OPER_CMP) /* CMP abs */\
W65C02S_OPCODE(0xCE, RMW_ABSOLUTE,              W65C02S_OPER_DEC) /* DEC wab */\
W65C02S_OPCODE(0xCF, RELATIVE_BIT,              014)              /* 014 rlb */\
W65C02S_OPCODE(0xD0, RELATIVE,                  W65C02S_OPER_BNE) /* BNE rel */\
W65C02S_OPCODE(0xD1, ZEROPAGE_INDIRECT_Y,       W65C02S_OPER_CMP) /* CMP ziy */\
W65C02S_OPCODE(0xD2, ZEROPAGE_INDIRECT,         W65C02S_OPER_CMP) /* CMP zpi */\
W65C02S_OPCODE(0xD3, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0xD4, ZEROPAGE_X,                W65C02S_OPER_NOP) /* NOP zpx */\
W65C02S_OPCODE(0xD5, ZEROPAGE_X,                W65C02S_OPER_CMP) /* CMP zpx */\
W65C02S_OPCODE(0xD6, RMW_ZEROPAGE_X,            W65C02S_OPER_DEC) /* DEC wzx */\
W65C02S_OPCODE(0xD7, ZEROPAGE_BIT,              015)              /* 015 zpb */\
W65C02S_OPCODE(0xD8, IMPLIED,                   W65C02S_OPER_CLD) /* CLD imp */\
W65C02S_OPCODE(0xD9, ABSOLUTE_Y,                W65C02S_OPER_CMP) /* CMP aby */\
W65C02S_OPCODE(0xDA, STACK_PUSH,                W65C02S_OPER_PHX) /* PHX phv */\
W65C02S_OPCODE(0xDB, INT_WAIT_STOP,             W65C02S_OPER_STP) /* STP wai */\
W65C02S_OPCODE(0xDC, ABSOLUTE,                  W65C02S_OPER_NOP) /* NOP abs */\
W65C02S_OPCODE(0xDD, ABSOLUTE_X,                W65C02S_OPER_CMP) /* CMP abx */\
W65C02S_OPCODE(0xDE, RMW_ABSOLUTE_X,            W65C02S_OPER_DEC) /* DEC wax */\
W65C02S_OPCODE(0xDF, RELATIVE_BIT,              015)              /* 015 rlb */\
W65C02S_OPCODE(0xE0, IMMEDIATE,                 W65C02S_OPER_CPX) /* CPX imm */\
W65C02S_OPCODE(0xE1, ZEROPAGE_INDIRECT_X,       W65C02S_OPER_SBC) /* SBC zix */\
W65C02S_OPCODE(0xE2, IMMEDIATE,                 W65C02S_OPER_NOP) /* NOP imm */\
W65C02S_OPCODE(0xE3, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0xE4, ZEROPAGE,                  W65C02S_OPER_CPX) /* CPX zpg */\
W65C02S_OPCODE(0xE5, ZEROPAGE,                  W65C02S_OPER_SBC) /* SBC zpg */\
W65C02S_OPCODE(0xE6, RMW_ZEROPAGE,              W65C02S_OPER_INC) /* INC wzp */\
W65C02S_OPCODE(0xE7, ZEROPAGE_BIT,              016)              /* 016 zpb */\
W65C02S_OPCODE(0xE8, IMPLIED_X,                 W65C02S_OPER_INC) /* INC imx */\
W65C02S_OPCODE(0xE9, IMMEDIATE,                 W65C02S_OPER_SBC) /* SBC imm */\
W65C02S_OPCODE(0xEA, IMPLIED,                   W65C02S_OPER_NOP) /* NOP imp */\
W65C02S_OPCODE(0xEB, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0xEC, ABSOLUTE,                  W65C02S_OPER_CPX) /* CPX abs */\
W65C02S_OPCODE(0xED, ABSOLUTE,                  W65C02S_OPER_SBC) /* SBC abs */\
W65C02S_OPCODE(0xEE, RMW_ABSOLUTE,              W65C02S_OPER_INC) /* INC wab */\
W65C02S_OPCODE(0xEF, RELATIVE_BIT,              016)              /* 016 rlb */\
W65C02S_OPCODE(0xF0, RELATIVE,                  W65C02S_OPER_BEQ) /* BEQ rel */\
W65C02S_OPCODE(0xF1, ZEROPAGE_INDIRECT_Y,       W65C02S_OPER_SBC) /* SBC ziy */\
W65C02S_OPCODE(0xF2, ZEROPAGE_INDIRECT,         W65C02S_OPER_SBC) /* SBC zpi */\
W65C02S_OPCODE(0xF3, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0xF4, ZEROPAGE_X,                W65C02S_OPER_NOP) /* NOP zpx */\
W65C02S_OPCODE(0xF5, ZEROPAGE_X,                W65C02S_OPER_SBC) /* SBC zpx */\
W65C02S_OPCODE(0xF6, RMW_ZEROPAGE_X,            W65C02S_OPER_INC) /* INC wzx */\
W65C02S_OPCODE(0xF7, ZEROPAGE_BIT,              017)              /* 017 zpb */\
W65C02S_OPCODE(0xF8, IMPLIED,                   W65C02S_OPER_SED) /* SED imp */\
W65C02S_OPCODE(0xF9, ABSOLUTE_Y,                W65C02S_OPER_SBC) /* SBC aby */\
W65C02S_OPCODE(0xFA, STACK_PULL,                W65C02S_OPER_PLX) /* PLX plv */\
W65C02S_OPCODE(0xFB, IMPLIED_1C,                W65C02S_OPER_NOP) /* NOP im1 */\
W65C02S_OPCODE(0xFC, ABSOLUTE,                  W65C02S_OPER_NOP) /* NOP abs */\
W65C02S_OPCODE(0xFD, ABSOLUTE_X,                W65C02S_OPER_SBC) /* SBC abx */\
W65C02S_OPCODE(0xFE, RMW_ABSOLUTE_X,            W65C02S_OPER_INC) /* INC wax */\
W65C02S_OPCODE(0xFF, RELATIVE_BIT,              017)              /* 017 rlb */

namespace physicalw65c02 {

uint16_t add_page_wrapping_s(uint16_t lhs, int8_t offset) {
    uint16_t page = lhs & 0xFF00;
    uint16_t byte = (lhs + offset) & 0x00FF;
    return page | byte;
}

uint16_t add_page_wrapping_u(uint16_t lhs, uint8_t offset) {
    uint16_t page = lhs & 0xFF00;
    uint16_t byte = (lhs + offset) & 0x00FF;
    return page | byte;
}

bool crosses_boundary_s(uint16_t lhs, int8_t offset) {
    uint16_t full = lhs + offset;
    uint16_t wrapped = add_page_wrapping_s(lhs, offset);
    return full != wrapped;
}

bool crosses_boundary_u(uint16_t lhs, uint8_t offset) {
    uint16_t full = lhs + offset;
    uint16_t wrapped = add_page_wrapping_u(lhs, offset);
    return full != wrapped;
}


void PhysicalW65C02::tick_cycle(BusInState state) {
    if (!state.resb) {
        in_rst = true;
        in_nmi = false;
        in_irq = false;
        seq_cycle = 1;
        mode = Mode::STACK_BRK;
        oper = W65C02S_OPER_BRK;
        return;
    }

	switch (mode) {
		case Mode::FETCH: tick_cycle_fetch(state); break;

		case Mode::IMMEDIATE: tick_cycle_immediate(state); break;

		case Mode::IMPLIED: tick_cycle_implied(state); break;
		case Mode::IMPLIED_1C: error = true; break;
		case Mode::IMPLIED_X: if (oper == W65C02S_OPER_INC) update_nz(++x); else update_nz(--x); seq_cycle = -1; break;
		case Mode::IMPLIED_Y: if (oper == W65C02S_OPER_INC) update_nz(++y); else update_nz(--y); seq_cycle = -1; break;

		case Mode::ZEROPAGE: tick_cycle_zeropage(state); break;
		case Mode::ZEROPAGE_BIT: tick_cycle_zeropage_bit(state); break; 
		case Mode::ZEROPAGE_INDIRECT: tick_cycle_zeropage_indirect(state); break; 
		case Mode::ZEROPAGE_X: tick_cycle_zeropage_x(state); break; 
		case Mode::ZEROPAGE_Y: tick_cycle_zeropage_y(state); break; 
		case Mode::ZEROPAGE_INDIRECT_X: tick_cycle_zeropage_indirect_x(state); break; 
		case Mode::ZEROPAGE_INDIRECT_Y: tick_cycle_zeropage_indirect_y(state); break; 
		case Mode::ZEROPAGE_INDIRECT_Y_STORE: tick_cycle_zeropage_indirect_y_store(state); break; 
		case Mode::ABSOLUTE: tick_cycle_absolute(state); break; 
		case Mode::ABSOLUTE_X: tick_cycle_absolute_x(state); break; 
		case Mode::ABSOLUTE_Y: tick_cycle_absolute_y(state); break; 
		case Mode::ABSOLUTE_X_STORE: tick_cycle_absolute_x_store(state); break; 
		case Mode::ABSOLUTE_Y_STORE: tick_cycle_absolute_y_store(state); break; 
		case Mode::ABSOLUTE_JUMP: tick_cycle_absolute_jump(state); break; 
		case Mode::ABSOLUTE_INDIRECT: tick_cycle_absolute_indirect(state); break; 
		case Mode::ABSOLUTE_INDIRECT_X: tick_cycle_absolute_indirect_x(state); break; 
		case Mode::RELATIVE: tick_cycle_relative(state); break; 
		case Mode::RELATIVE_BIT: error = true; break; 
		case Mode::RMW_ZEROPAGE: tick_cycle_rmw_zeropage(state); break; 
		case Mode::RMW_ZEROPAGE_X: tick_cycle_rmw_zeropage_x(state); break; 
		case Mode::RMW_ABSOLUTE: tick_cycle_rmw_absolute(state); break; 
		case Mode::RMW_ABSOLUTE_X: tick_cycle_rmw_absolute_x(state); break; 
		case Mode::RMW_ABSOLUTE_Y: tick_cycle_rmw_absolute_y(state); break; 
		case Mode::SUBROUTINE: tick_cycle_subroutine(state); break; 
		case Mode::RETURN_SUB: tick_cycle_return_sub(state); break; 
		case Mode::STACK_BRK: tick_cycle_stack_brk(state); break; 
		case Mode::STACK_PUSH: tick_cycle_stack_push(state); break; 
		case Mode::STACK_PULL: tick_cycle_stack_pull(state); break; 
		case Mode::STACK_RTI: tick_cycle_stack_rti(state); break; 
		case Mode::NOP_5C: error = true; break; 
		case Mode::INT_WAIT_STOP: error = true; break; 

		default: error = true; break;
	}

    if (++seq_cycle == 0) {
        mode = Mode::FETCH;
    }
}

void PhysicalW65C02::tick_cycle_fetch(BusInState state) {
	assert(seq_cycle == 0);

    if (prev_nmi_state && !state.nmib) {
        in_nmi = true;
        in_irq = false;

        mode = Mode::STACK_BRK;
        oper = W65C02S_OPER_BRK;
        return;
    } else if ((p & FLAG_I_MASK) == 0 && !state.irqb) {
        in_irq = true;

        mode = Mode::STACK_BRK;
        oper = W65C02S_OPER_BRK;
        return;
    }

	++pc;

#define W65C02S_OPCODE(OPC, MODE, OPER) case OPC: mode = Mode::MODE; oper = (Oper)OPER; break;
	switch (state.data) {
		W65C02S_OPCODE_TABLE()
	}
#undef W65C02S_OPCODE
}

void PhysicalW65C02::get_bus_state(BusState& out) const {
	out.sync = false;
	out.vpb  = true;
	out.rwb  = rwb_read;
    out.addr = 0x0000;
    out.data = 0x00;
    out.error = false;

    if (error) {
        out.error = true;
        return;
    }

	switch (mode) {
		case Mode::FETCH: get_bus_state_fetch(out); break;

		case Mode::IMPLIED_1C: assert(false && "unreachable"); break;

		case Mode::IMPLIED_X:
		case Mode::IMPLIED_Y:
		case Mode::IMPLIED:   out.addr = pc; break; // TODO: Check for dummy read correctness?

		case Mode::IMMEDIATE: get_bus_state_immediate(out); break;

		case Mode::ZEROPAGE: get_bus_state_zeropage(out); break;
		case Mode::ZEROPAGE_BIT: get_bus_state_zeropage_bit(out); break; 
		case Mode::ZEROPAGE_INDIRECT: get_bus_state_zeropage_indirect(out); break; 
		case Mode::ZEROPAGE_X: get_bus_state_zeropage_x(out); break; 
		case Mode::ZEROPAGE_Y: get_bus_state_zeropage_y(out); break; 
		case Mode::ZEROPAGE_INDIRECT_X: get_bus_state_zeropage_indirect_x(out); break; 
		case Mode::ZEROPAGE_INDIRECT_Y: get_bus_state_zeropage_indirect_y(out); break; 
		case Mode::ZEROPAGE_INDIRECT_Y_STORE: get_bus_state_zeropage_indirect_y_store(out); break; 
		case Mode::ABSOLUTE: get_bus_state_absolute(out); break; 
		case Mode::ABSOLUTE_X: get_bus_state_absolute_x(out); break; 
		case Mode::ABSOLUTE_Y: get_bus_state_absolute_y(out); break; 
		case Mode::ABSOLUTE_X_STORE: get_bus_state_absolute_x_store(out); break; 
		case Mode::ABSOLUTE_Y_STORE: get_bus_state_absolute_y_store(out); break; 
		case Mode::ABSOLUTE_JUMP: get_bus_state_absolute_jump(out); break; 
		case Mode::ABSOLUTE_INDIRECT: get_bus_state_absolute_indirect(out); break; 
		case Mode::ABSOLUTE_INDIRECT_X: get_bus_state_absolute_indirect_x(out); break; 
		case Mode::RELATIVE: get_bus_state_relative(out); break; 
		case Mode::RELATIVE_BIT: out.error = true; break; 
		case Mode::RMW_ZEROPAGE: get_bus_state_rmw_zeropage(out); break; 
		case Mode::RMW_ZEROPAGE_X: get_bus_state_rmw_zeropage_x(out); break; 
		case Mode::RMW_ABSOLUTE: get_bus_state_rmw_absolute(out); break; 
		case Mode::RMW_ABSOLUTE_X: get_bus_state_rmw_absolute_x(out); break; 
		case Mode::RMW_ABSOLUTE_Y: get_bus_state_rmw_absolute_y(out); break; 
		case Mode::SUBROUTINE: get_bus_state_subroutine(out); break; 
		case Mode::RETURN_SUB: get_bus_state_return_sub(out); break; 
		case Mode::STACK_BRK: get_bus_state_stack_brk(out); break; 
		case Mode::STACK_PUSH: get_bus_state_stack_push(out); break; 
		case Mode::STACK_PULL: get_bus_state_stack_pull(out); break; 
		case Mode::STACK_RTI: get_bus_state_stack_rti(out); break; 
		case Mode::NOP_5C: out.error = true; break; 
		case Mode::INT_WAIT_STOP: out.error = true; break; 

		default: out.error = true;
	}
}

void PhysicalW65C02::get_bus_state_fetch(BusState& out) const {
	out.addr = pc;
	out.sync = true;
}

#define SET_LO_PART(value, lo_part) ((value & 0xFF00) | ((lo_part & 0xFF) << 0))
#define SET_HI_PART(value, hi_part) ((value & 0x00FF) | ((hi_part & 0xFF) << 8))

void PhysicalW65C02::tick_cycle_immediate(BusInState state) {
    switch (seq_cycle) {
        case 1: execute_accum_oper(state.data); ++pc; seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_immediate(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_implied(BusInState state) {
    switch (seq_cycle) {
        case 1:
            switch (oper) {
                case W65C02S_OPER_TAX: x = a; update_nz(x); break;
                case W65C02S_OPER_TAY: y = a; update_nz(y); break;
                case W65C02S_OPER_TXA: a = x; update_nz(a); break;
                case W65C02S_OPER_TYA: a = y; update_nz(a); break;
                case W65C02S_OPER_TSX: x = s; update_nz(x); break;
                case W65C02S_OPER_TXS: s = x;               break;
                default: a = execute_unary_oper(a); break;
            }
            seq_cycle = -1;
            break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_implied(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_zeropage(BusInState state) {
    switch (seq_cycle) {
        case 1: bal = state.data; ++pc;                         break;
        case 2:
            switch (oper) {
                case W65C02S_OPER_STA: break;
                case W65C02S_OPER_STX: break;
                case W65C02S_OPER_STY: break;
                default: execute_accum_oper(state.data); break;
            }
            seq_cycle = -1;
            break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_zeropage(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;  break;
        case 2: out.addr = bal;
            switch (oper) {
                case W65C02S_OPER_STA: out.rwb = rwb_write; out.data = a; break;
                case W65C02S_OPER_STX: out.rwb = rwb_write; out.data = x; break;
                case W65C02S_OPER_STY: out.rwb = rwb_write; out.data = y; break;
                default: break;
            }
            break;
        default: UNREACHABLE(); break;
    }
}

void PhysicalW65C02::tick_cycle_zeropage_bit(BusInState state) {
    UNREACHABLE();
}
void PhysicalW65C02::get_bus_state_zeropage_bit(BusState& out) const {
    UNREACHABLE();
}

void PhysicalW65C02::tick_cycle_zeropage_indirect(BusInState state) {
    UNREACHABLE();
}
void PhysicalW65C02::get_bus_state_zeropage_indirect(BusState& out) const {
    UNREACHABLE();
}

void PhysicalW65C02::tick_cycle_zeropage_x(BusInState state) {
    switch (seq_cycle) {
        case 1: bal = state.data; ++pc;                         break;
        case 2:                                                 break;
        case 3: 
            switch (oper) {
                case W65C02S_OPER_STA: break;
                case W65C02S_OPER_STY: break;
                default: execute_accum_oper(state.data); break;
            }
            seq_cycle = -1;
            break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_zeropage_x(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;                 break;
        case 2: out.addr = bal;                break;
        case 3:
            out.addr = (uint8_t)(bal + x); 
            switch (oper) {
                case W65C02S_OPER_STA: out.rwb = rwb_write; out.data = a; break;
                case W65C02S_OPER_STY: out.rwb = rwb_write; out.data = y; break;
                default: break;
            }
            break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_zeropage_y(BusInState state) {
    switch (seq_cycle) {
        case 1: bal = state.data; ++pc;                         break;
        case 2:                                                 break;
        case 3:
            switch (oper) {
                case W65C02S_OPER_STA: break;
                case W65C02S_OPER_STX: break;
                default: execute_accum_oper(state.data); break;
            }
            seq_cycle = -1;
            break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_zeropage_y(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;                 break;
        case 2: out.addr = bal;                break;
        case 3:
            out.addr = (uint8_t)(bal + y);
            switch (oper) {
                case W65C02S_OPER_STA: out.rwb = rwb_write; out.data = a; break;
                case W65C02S_OPER_STX: out.rwb = rwb_write; out.data = x; break;
                default: break;
            }
            break;
        default: UNREACHABLE();
    }
}


void PhysicalW65C02::tick_cycle_rmw_zeropage(BusInState state) {
    switch (seq_cycle) {
        case 1: bal = state.data; ++pc; break;
        case 2: tmp = state.data; break;
        case 3: tmp = execute_unary_oper(tmp); break;
        case 4: seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_rmw_zeropage(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;  break;
        case 2: out.addr = bal; break;
        case 3: out.addr = bal; out.rwb = rwb_read; out.data = tmp; break; // TODO: Check
        case 4: out.addr = bal; out.rwb = rwb_write; out.data = tmp; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_rmw_zeropage_x(BusInState state) {
    switch (seq_cycle) {
        case 1: bal = state.data; ++pc; break;
        case 2:                         break;
        case 3: tmp = state.data;       break;
        case 4: tmp = execute_unary_oper(tmp); break;
        case 5: seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_rmw_zeropage_x(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;                 break;
        case 2: out.addr = bal;                break;
        case 3: out.addr = (uint8_t)(bal + x); break;
        case 4: out.addr = (uint8_t)(bal + x); out.rwb = rwb_read; out.data = tmp; break; // TODO: Check
        case 5: out.addr = (uint8_t)(bal + x); out.rwb = rwb_write; out.data = tmp; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_rmw_absolute(BusInState state) {
    switch (seq_cycle) {
        case 1: adr = SET_LO_PART(adr, state.data); ++pc;       break;
        case 2: adr = SET_HI_PART(adr, state.data); ++pc;       break;
        case 3: tmp = state.data;                               break;
        case 4: tmp = execute_unary_oper(tmp);                  break;
        case 5: seq_cycle = -1;                                 break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_rmw_absolute(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc; break;
        case 2: out.addr = pc; break;
        case 3: out.addr = adr; break;
        case 4: out.addr = adr;/*out.rwb = rwb_write;*/ out.data = tmp; break; // TODO: Check write or read
        case 5: out.addr = adr; out.rwb = rwb_write; out.data = tmp; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_rmw_absolute_x(BusInState state) {
    switch (seq_cycle) {
        case 1: adr = SET_LO_PART(adr, state.data); ++pc;       break;
        case 2: adr = SET_HI_PART(adr, state.data); ++pc;       break;
        case 3:                                                 break;
        case 4: tmp = state.data;                               break;
        case 5: tmp = execute_unary_oper(tmp);                  break;
        case 6: seq_cycle = -1;                                 break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_rmw_absolute_x(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc; break;
        case 2: out.addr = pc; break;
        case 3: out.addr = /*add_page_wrapping(adr, x)*/ pc - 1; break; // TODO: Check
        case 4: out.addr = adr + x; break;
        case 5: out.addr = adr + x;/*out.rwb = rwb_write;*/ out.data = tmp; break; // TODO: Check write or read
        case 6: out.addr = adr + x; out.rwb = rwb_write; out.data = tmp; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_rmw_absolute_y(BusInState state) {
    switch (seq_cycle) {
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_rmw_absolute_y(BusState& out) const {
    switch (seq_cycle) {
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_subroutine(BusInState state) {
    switch (seq_cycle) {
        case 1: adr = SET_LO_PART(adr, state.data); ++pc; break;
        case 2:                                           break;
        case 3: --s;                                      break;
        case 4: --s;                                      break;
        case 5: pc  = SET_HI_PART(adr, state.data); seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_subroutine(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;  break;
        case 2: out.addr = 0x0100 | s; break;
        case 3: out.addr = 0x0100 | s; out.rwb = rwb_write; out.data = (pc >> 8) & 0xFF; break;
        case 4: out.addr = 0x0100 | s; out.rwb = rwb_write; out.data = (pc     ) & 0xFF; break;
        case 5: out.addr = pc; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_return_sub(BusInState state) {
    switch (seq_cycle) {
        case 1: break;
        case 2: ++s; break;
        case 3: ++s; pc = SET_LO_PART(pc, state.data); break;
        case 4:      pc = SET_HI_PART(pc, state.data); break;
        case 5: ++pc; seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::get_bus_state_return_sub(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;         break;
        case 2: out.addr = 0x0100 | s; break;
        case 3: out.addr = 0x0100 | s; break;
        case 4: out.addr = 0x0100 | s; break;
        case 5: out.addr = pc;         break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_zeropage_indirect_x(BusInState state) {
    switch (seq_cycle) {
        case 1: bal = state.data; ++pc;                         break;
        case 2:                                                 break;
        case 3: adr = SET_LO_PART(adr, state.data);             break;
        case 4: adr = SET_HI_PART(adr, state.data);             break;
        case 5:
            switch (oper) {
                case W65C02S_OPER_STA: break;
                case W65C02S_OPER_STX: break;
                case W65C02S_OPER_STY: break;
                default: execute_accum_oper(state.data); break;
            }
            seq_cycle = -1;
            break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_zeropage_indirect_x(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;                      break;
        case 2: out.addr = (uint8_t)(bal);          break;
        case 3: out.addr = (uint8_t)(bal + x + 0);  break;
        case 4: out.addr = (uint8_t)(bal + x + 1);  break;
        case 5:
            out.addr = adr; 
            switch (oper) {
                case W65C02S_OPER_STA: out.rwb = rwb_write; out.data = a; break;
                case W65C02S_OPER_STX: out.rwb = rwb_write; out.data = x; break;
                case W65C02S_OPER_STY: out.rwb = rwb_write; out.data = y; break;
                default: break;
            }
            break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_zeropage_indirect_y(BusInState state) {
    bool is_store = (oper == W65C02S_OPER_STA || oper == W65C02S_OPER_STX || oper == W65C02S_OPER_STY);
    switch (seq_cycle) {
        case 1: bal = state.data; ++pc;                         break;
        case 2: adr = SET_LO_PART(adr, state.data);             break;
        case 3: adr = SET_HI_PART(adr, state.data);             break;
        case 4:
            if (!crosses_boundary_u(adr, y) && !is_store) {
                execute_accum_oper(state.data); seq_cycle = -1;
            }
            break;
        case 5: execute_accum_oper(state.data); seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_zeropage_indirect_y(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;                  break;
        case 2: out.addr = (uint8_t)(bal + 0);  break;
        case 3: out.addr = (uint8_t)(bal + 1);  break;
        case 4: out.addr = add_page_wrapping_u(adr, y); break;
        case 5: out.addr = adr + y;                     break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_zeropage_indirect_y_store(BusInState state) {
    assert(oper == W65C02S_OPER_STA);
    switch (seq_cycle) {
        case 1: bal = state.data; ++pc;                         break;
        case 2: adr = SET_LO_PART(adr, state.data);             break;
        case 3: adr = SET_HI_PART(adr, state.data);             break;
        case 4:                                                 break;
        case 5: seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_zeropage_indirect_y_store(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;                  break;
        case 2: out.addr = (uint8_t)(bal + 0);  break;
        case 3: out.addr = (uint8_t)(bal + 1);  break;
        case 4: out.addr = add_page_wrapping_u(adr, y); break;
        case 5: out.addr = adr + y; out.rwb = rwb_write; out.data = a; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_absolute(BusInState state) {
    switch (seq_cycle) {
        case 1: adr = SET_LO_PART(adr, state.data); ++pc;       break;
        case 2: adr = SET_HI_PART(adr, state.data); ++pc;       break;
        case 3:
            switch (oper) {
                case W65C02S_OPER_STA: break;
                case W65C02S_OPER_STX: break;
                case W65C02S_OPER_STY: break;
                default: execute_accum_oper(state.data); break;
            }
            seq_cycle = -1;
            break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_absolute(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;  break;
        case 2: out.addr = pc;  break;
        case 3:
            out.addr = adr; 
            switch (oper) {
                case W65C02S_OPER_STA: out.rwb = rwb_write; out.data = a; break;
                case W65C02S_OPER_STX: out.rwb = rwb_write; out.data = x; break;
                case W65C02S_OPER_STY: out.rwb = rwb_write; out.data = y; break;
                default: break;
            }
            break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_absolute_x(BusInState state) {
    switch (seq_cycle) {
        case 1: adr = SET_LO_PART(adr, state.data); ++pc;       break;
        case 2: adr = SET_HI_PART(adr, state.data); ++pc;       break;
        case 3:
            if (!crosses_boundary_u(adr, x)) {
                execute_accum_oper(state.data);
                seq_cycle = -1;
            }
            break;
        case 4: execute_accum_oper(state.data); seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_absolute_x(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;  break;
        case 2: out.addr = pc;  break;
        case 3: out.addr = add_page_wrapping_u(adr, x); break;
        case 4: out.addr = adr + x; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_absolute_y(BusInState state) {
    switch (seq_cycle) {
        case 1: adr = SET_LO_PART(adr, state.data); ++pc;       break;
        case 2: adr = SET_HI_PART(adr, state.data); ++pc;       break;
        case 3:
            if (!crosses_boundary_u(adr, y)) {
                execute_accum_oper(state.data);
                seq_cycle = -1;
            }
            break;
        case 4: execute_accum_oper(state.data); seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_absolute_y(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;  break;
        case 2: out.addr = pc;  break;
        case 3: out.addr = add_page_wrapping_u(adr, y); break;
        case 4: out.addr = adr + y; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_absolute_x_store(BusInState state) {
    switch (seq_cycle) {
        case 1: adr = SET_LO_PART(adr, state.data); ++pc;       break;
        case 2: adr = SET_HI_PART(adr, state.data); ++pc;       break;
        case 3:                                                 break;
        case 4: seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_absolute_x_store(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;  break;
        case 2: out.addr = pc;  break;
        case 3: out.addr = add_page_wrapping_u(adr, x); break;
        case 4: out.addr = adr + x; out.data = a; out.rwb = rwb_write; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_absolute_y_store(BusInState state) {
    switch (seq_cycle) {
        case 1: adr = SET_LO_PART(adr, state.data); ++pc;       break;
        case 2: adr = SET_HI_PART(adr, state.data); ++pc;       break;
        case 3:                                                 break;
        case 4: seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_absolute_y_store(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;  break;
        case 2: out.addr = pc;  break;
        case 3: out.addr = add_page_wrapping_u(adr, y); break;
        case 4: out.addr = adr + y; out.data = a; out.rwb = rwb_write; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_absolute_jump(BusInState state) {
    switch (seq_cycle) {
        case 1: adr = SET_LO_PART(adr, state.data); ++pc;                 break;
        case 2:  pc = SET_HI_PART(adr, state.data);       seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_absolute_jump(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc; break;
        case 2: out.addr = pc; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_absolute_indirect(BusInState state) {
    switch (seq_cycle) {
        case 1: adr = SET_LO_PART(adr, state.data); ++pc; break;
        case 2: adr = SET_HI_PART(adr, state.data); ++pc; break;
        case 3: pc = SET_LO_PART(pc, state.data); break;
        case 4: pc = SET_HI_PART(pc, state.data); seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_absolute_indirect(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc; break;
        case 2: out.addr = pc; break;
        case 3: out.addr = add_page_wrapping_u(adr, 0); break;
        case 4: out.addr = add_page_wrapping_u(adr, 1); break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_absolute_indirect_x(BusInState state) {
    UNREACHABLE();
}
void PhysicalW65C02::get_bus_state_absolute_indirect_x(BusState& out) const {
    UNREACHABLE();
}

void PhysicalW65C02::tick_cycle_relative(BusInState state) {
    bool taken;

    switch (seq_cycle) {
        case 1:
            bal = state.data;
            ++pc;
            switch (oper) {
                case W65C02S_OPER_BRA: taken = true;               break;
                case W65C02S_OPER_BEQ: taken =  (p & FLAG_Z_MASK); break;
                case W65C02S_OPER_BNE: taken = !(p & FLAG_Z_MASK); break;
                case W65C02S_OPER_BMI: taken =  (p & FLAG_N_MASK); break;
                case W65C02S_OPER_BPL: taken = !(p & FLAG_N_MASK); break;
                case W65C02S_OPER_BVS: taken =  (p & FLAG_V_MASK); break;
                case W65C02S_OPER_BVC: taken = !(p & FLAG_V_MASK); break;
                case W65C02S_OPER_BCS: taken =  (p & FLAG_C_MASK); break;
                case W65C02S_OPER_BCC: taken = !(p & FLAG_C_MASK); break;
                default: UNREACHABLE();
            }
            if (!taken) {
                seq_cycle = -1;
            }
            break;
        case 2: if (!crosses_boundary_s(pc, bal)) { seq_cycle = -1; } pc += (int8_t)bal; break;
        case 3: seq_cycle = -1;                                                break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_relative(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;                           break;
        case 2: out.addr = add_page_wrapping_s(pc, bal); break;
        case 3: out.addr = pc + bal;                     break;
    }
}

void PhysicalW65C02::tick_cycle_stack_brk(BusInState state) {
    bool is_brk = (!in_rst && !in_nmi && !in_irq);

    switch (seq_cycle) {
        case 1: if (is_brk) { ++pc; }                   break;
        case 2: --s;                                    break;
        case 3: --s;                                    break;
        case 4: --s;                                    break;
        case 5: pc = (pc & 0xFF00) | (state.data << 0); break;
        case 6: pc = (pc & 0x00FF) | (state.data << 8); p &= ~FLAG_D_MASK; p |= FLAG_I_MASK; seq_cycle = -1;
            in_rst = false;
            in_nmi = false;
            in_irq = false;
            break; // TODO: CHECK I?
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_stack_brk(BusState& out) const {
    bool is_brk = (!in_rst && !in_nmi && !in_irq);

    uint16_t vector;
    if (in_rst) {
        vector = 0xFFFC;
    } else if (in_nmi) {
        vector = 0xFFFA;
    } else {
        vector = 0xFFFE;
    }

    bool rwb = (in_rst ? rwb_read : rwb_write);

	switch (seq_cycle) {
		case 1: out.addr = pc;                                                    break;
		case 2: out.addr = 0x0100 | s; out.rwb = rwb; out.data = ((pc >> 8) & 0xFF);           break;
		case 3: out.addr = 0x0100 | s; out.rwb = rwb; out.data = ((pc >> 0) & 0xFF);           break;
		case 4: out.addr = 0x0100 | s; out.rwb = rwb; out.data = is_brk ? p | FLAG_B_MASK : p; break;
		case 5: out.addr = vector + 0; out.vpb = false;                                break;
		case 6: out.addr = vector + 1; out.vpb = false;                                break;
        default: UNREACHABLE();
	}
}

void PhysicalW65C02::tick_cycle_stack_push(BusInState state) {
    switch (seq_cycle) {
        case 1:                      break;
        case 2: --s; seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_stack_push(BusState& out) const {
    auto get_data = [&]() {
        switch (oper) {
            case W65C02S_OPER_PHP: return (uint8_t)(p | FLAG_B_MASK | (1 << 5));
            case W65C02S_OPER_PHA: return a;
            case W65C02S_OPER_PHX: return x;
            case W65C02S_OPER_PHY: return y;
            default: UNREACHABLE(); break;
        }
    };

    switch (seq_cycle) {
        case 1: out.addr = pc; break;
        case 2: out.addr = 0x0100 | s; out.rwb = rwb_write; out.data = get_data(); break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_stack_pull(BusInState state) {
    auto set_data = [&](uint8_t arg) {
        switch (oper) {
            case W65C02S_OPER_PLP: p = (arg & ~(FLAG_B_MASK)) | (1 << 5); break;
            case W65C02S_OPER_PLA: a = arg; update_nz(a);               break;
            case W65C02S_OPER_PLX: x = arg; update_nz(x);               break;
            case W65C02S_OPER_PLY: y = arg; update_nz(y);               break;
            default: UNREACHABLE();
        }
    };

    switch (seq_cycle) {
        case 1:                                       break;
        case 2: ++s;                                  break;
        case 3: set_data(state.data); seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_stack_pull(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc; break;
        case 2: out.addr = 0x0100 | s; break;
        case 3: out.addr = 0x0100 | s; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::tick_cycle_stack_rti(BusInState state) {
    switch (seq_cycle) {
        case 1:                                                        break;
        case 2: ++s;                                                   break;
        case 3: ++s; p = (state.data & ~FLAG_B_MASK) | (1 << 5);       break;
        case 4: ++s; pc = SET_LO_PART(pc, state.data);                 break;
        case 5:      pc = SET_HI_PART(pc, state.data); seq_cycle = -1; break;
        default: UNREACHABLE();
    }
}
void PhysicalW65C02::get_bus_state_stack_rti(BusState& out) const {
    switch (seq_cycle) {
        case 1: out.addr = pc;         break;
        case 2: out.addr = 0x0100 | s; break;
        case 3: out.addr = 0x0100 | s; break;
        case 4: out.addr = 0x0100 | s; break;
        case 5: out.addr = 0x0100 | s; break;
        default: UNREACHABLE();
    }
}

void PhysicalW65C02::adc(uint8_t arg) {
    uint16_t result_16 = a + arg + ((p & FLAG_C_MASK) != 0);
    uint8_t result = result_16;

    bool overflow = !((a^arg) & 0x80) && ((a^result) & 0x80);

    a = result;

    update_nz(result);
    set_c(result_16 & 0x100);
    set_v(overflow);
}

void PhysicalW65C02::execute_accum_oper(uint8_t arg) {
    bool c_in = (p & FLAG_C_MASK) != 0;
    bool c_out;

    uint16_t result;

    switch (oper) {
        case W65C02S_OPER_NOP: break;
        case W65C02S_OPER_ORA: a |= arg; update_nz(a); break;
        case W65C02S_OPER_AND: a &= arg; update_nz(a); break;
        case W65C02S_OPER_EOR: a ^= arg; update_nz(a); break;
        case W65C02S_OPER_ADC: adc( arg); break;
        case W65C02S_OPER_SBC: adc(~arg); break;
        case W65C02S_OPER_BIT: p &= ~0xC0; p |= arg & 0xC0; update_z(a & arg); break;

        case W65C02S_OPER_INC: assert(false && "TODO: W65C02S_OPER_INC"); break;
        case W65C02S_OPER_DEC: assert(false && "TODO: W65C02S_OPER_INC"); break;
        case W65C02S_OPER_CMP: result = a + (uint16_t)(uint8_t)(~arg) + 1; update_nz(result); set_c(result & 0x100); break;
        case W65C02S_OPER_CPX: result = x + (uint16_t)(uint8_t)(~arg) + 1; update_nz(result); set_c(result & 0x100); break;
        case W65C02S_OPER_CPY: result = y + (uint16_t)(uint8_t)(~arg) + 1; update_nz(result); set_c(result & 0x100); break;

        case W65C02S_OPER_TSB: assert(false && "TODO: W65C02S_OPER_TSB"); break;
        case W65C02S_OPER_TRB: assert(false && "TODO: W65C02S_OPER_TRB"); break;


        case W65C02S_OPER_JMP: assert(false && "TODO: W65C02S_OPER_JMP"); break;
        case W65C02S_OPER_JSR: assert(false && "TODO: W65C02S_OPER_JSR"); break;
        case W65C02S_OPER_RTI: assert(false && "TODO: W65C02S_OPER_RTI"); break;
        case W65C02S_OPER_RTS: assert(false && "TODO: W65C02S_OPER_RTS"); break;

        case W65C02S_OPER_LDA: a = arg; update_nz(a); break;
        case W65C02S_OPER_LDX: x = arg; update_nz(x); break;
        case W65C02S_OPER_LDY: y = arg; update_nz(y); break;
        case W65C02S_OPER_STA: assert(false && "TODO: W65C02S_OPER_STA"); break;
        case W65C02S_OPER_STX: assert(false && "TODO: W65C02S_OPER_STX"); break;
        case W65C02S_OPER_STY: assert(false && "TODO: W65C02S_OPER_STY"); break;
        case W65C02S_OPER_STZ: assert(false && "TODO: W65C02S_OPER_STZ"); break;

        case W65C02S_OPER_TXA: assert(false && "TODO: W65C02S_OPER_TXA"); break;
        case W65C02S_OPER_TYA: assert(false && "TODO: W65C02S_OPER_TYA"); break;
        case W65C02S_OPER_TAX: assert(false && "TODO: W65C02S_OPER_TAX"); break;
        case W65C02S_OPER_TAY: assert(false && "TODO: W65C02S_OPER_TAY"); break;
        case W65C02S_OPER_TXS: assert(false && "TODO: W65C02S_OPER_TXS"); break;
        case W65C02S_OPER_TSX: assert(false && "TODO: W65C02S_OPER_TSX"); break;

        case W65C02S_OPER_PHP: assert(false && "TODO: W65C02S_OPER_PHP"); break;
        case W65C02S_OPER_PLP: assert(false && "TODO: W65C02S_OPER_PLP"); break;
        case W65C02S_OPER_PHA: assert(false && "TODO: W65C02S_OPER_PHA"); break;
        case W65C02S_OPER_PLA: assert(false && "TODO: W65C02S_OPER_PLA"); break;
        case W65C02S_OPER_PHY: assert(false && "TODO: W65C02S_OPER_PHY"); break;
        case W65C02S_OPER_PLY: assert(false && "TODO: W65C02S_OPER_PLY"); break;
        case W65C02S_OPER_PHX: assert(false && "TODO: W65C02S_OPER_PHX"); break;
        case W65C02S_OPER_PLX: assert(false && "TODO: W65C02S_OPER_PLX"); break;

        case W65C02S_OPER_BRA: assert(false && "TODO: W65C02S_OPER_BRA"); break;
        case W65C02S_OPER_BEQ: assert(false && "TODO: W65C02S_OPER_BEQ"); break;
        case W65C02S_OPER_BNE: assert(false && "TODO: W65C02S_OPER_BNE"); break;
        case W65C02S_OPER_BPL: assert(false && "TODO: W65C02S_OPER_BPL"); break;
        case W65C02S_OPER_BMI: assert(false && "TODO: W65C02S_OPER_BMI"); break;
        case W65C02S_OPER_BVS: assert(false && "TODO: W65C02S_OPER_BVS"); break;
        case W65C02S_OPER_BVC: assert(false && "TODO: W65C02S_OPER_BVC"); break;
        case W65C02S_OPER_BCS: assert(false && "TODO: W65C02S_OPER_BCS"); break;
        case W65C02S_OPER_BCC: assert(false && "TODO: W65C02S_OPER_BCC"); break;

        case W65C02S_OPER_WAI: assert(false && "TODO: W65C02S_OPER_WAI"); break;
        case W65C02S_OPER_STP: assert(false && "TODO: W65C02S_OPER_STP"); break;

        default: UNREACHABLE(); break;
    }
}

uint8_t PhysicalW65C02::execute_unary_oper(uint8_t arg) {
    bool c_in = (p & FLAG_C_MASK) != 0;
    bool c_out;

    switch (oper) {
        case W65C02S_OPER_NOP: return arg;
        case W65C02S_OPER_ASL: set_c(arg & 0x80); arg <<= 1; update_nz(arg); return arg;
        case W65C02S_OPER_ASR: assert(false && "TODO: ASR"); break;
        case W65C02S_OPER_LSR: set_c(arg & 0x01); arg >>= 1; update_nz(arg); return arg;
        case W65C02S_OPER_ROL: c_out = arg & 0x80; arg <<= 1; arg |= c_in;      update_nz(arg); set_c(c_out); return arg;
        case W65C02S_OPER_ROR: c_out = arg & 0x01; arg >>= 1; arg |= c_in << 7; update_nz(arg); set_c(c_out); return arg;
        case W65C02S_OPER_BIT: assert(false && "TODO: BIT"); break;
        
        case W65C02S_OPER_CLC: p &= ~FLAG_C_MASK; return arg;
        case W65C02S_OPER_SEC: p |=  FLAG_C_MASK; return arg;
        case W65C02S_OPER_CLI: p &= ~FLAG_I_MASK; return arg;
        case W65C02S_OPER_SEI: p |=  FLAG_I_MASK; return arg;
        case W65C02S_OPER_CLD: p &= ~FLAG_D_MASK; return arg;
        case W65C02S_OPER_SED: p |=  FLAG_D_MASK; return arg;
        case W65C02S_OPER_CLV: p &= ~FLAG_V_MASK; return arg;

        case W65C02S_OPER_INC: arg += 1; update_nz(arg); return arg;
        case W65C02S_OPER_DEC: arg -= 1; update_nz(arg); return arg;
        case W65C02S_OPER_CMP: assert(false && "TODO: W65C02S_OPER_CMP"); break;
        case W65C02S_OPER_CPX: assert(false && "TODO: W65C02S_OPER_CPX"); break;
        case W65C02S_OPER_CPY: assert(false && "TODO: W65C02S_OPER_CPY"); break;

        case W65C02S_OPER_TSB: update_z(arg & a); return arg |  a;
        case W65C02S_OPER_TRB: update_z(arg & a); return arg & ~a;


        case W65C02S_OPER_JMP: assert(false && "TODO: W65C02S_OPER_JMP"); break;
        case W65C02S_OPER_JSR: assert(false && "TODO: W65C02S_OPER_JSR"); break;
        case W65C02S_OPER_RTI: assert(false && "TODO: W65C02S_OPER_RTI"); break;
        case W65C02S_OPER_RTS: assert(false && "TODO: W65C02S_OPER_RTS"); break;

        case W65C02S_OPER_LDA: assert(false && "TODO: W65C02S_OPER_LDA"); break;
        case W65C02S_OPER_LDX: assert(false && "TODO: W65C02S_OPER_LDX"); break;
        case W65C02S_OPER_LDY: assert(false && "TODO: W65C02S_OPER_LDY"); break;
        case W65C02S_OPER_STA: assert(false && "TODO: W65C02S_OPER_STA"); break;
        case W65C02S_OPER_STX: assert(false && "TODO: W65C02S_OPER_STX"); break;
        case W65C02S_OPER_STY: assert(false && "TODO: W65C02S_OPER_STY"); break;
        case W65C02S_OPER_STZ: assert(false && "TODO: W65C02S_OPER_STZ"); break;

        case W65C02S_OPER_PHP: assert(false && "TODO: W65C02S_OPER_PHP"); break;
        case W65C02S_OPER_PLP: assert(false && "TODO: W65C02S_OPER_PLP"); break;
        case W65C02S_OPER_PHA: assert(false && "TODO: W65C02S_OPER_PHA"); break;
        case W65C02S_OPER_PLA: assert(false && "TODO: W65C02S_OPER_PLA"); break;
        case W65C02S_OPER_PHY: assert(false && "TODO: W65C02S_OPER_PHY"); break;
        case W65C02S_OPER_PLY: assert(false && "TODO: W65C02S_OPER_PLY"); break;
        case W65C02S_OPER_PHX: assert(false && "TODO: W65C02S_OPER_PHX"); break;
        case W65C02S_OPER_PLX: assert(false && "TODO: W65C02S_OPER_PLX"); break;

        case W65C02S_OPER_BRA: assert(false && "TODO: W65C02S_OPER_BRA"); break;
        case W65C02S_OPER_BEQ: assert(false && "TODO: W65C02S_OPER_BEQ"); break;
        case W65C02S_OPER_BNE: assert(false && "TODO: W65C02S_OPER_BNE"); break;
        case W65C02S_OPER_BPL: assert(false && "TODO: W65C02S_OPER_BPL"); break;
        case W65C02S_OPER_BMI: assert(false && "TODO: W65C02S_OPER_BMI"); break;
        case W65C02S_OPER_BVS: assert(false && "TODO: W65C02S_OPER_BVS"); break;
        case W65C02S_OPER_BVC: assert(false && "TODO: W65C02S_OPER_BVC"); break;
        case W65C02S_OPER_BCS: assert(false && "TODO: W65C02S_OPER_BCS"); break;
        case W65C02S_OPER_BCC: assert(false && "TODO: W65C02S_OPER_BCC"); break;

        case W65C02S_OPER_WAI: assert(false && "TODO: W65C02S_OPER_WAI"); break;
        case W65C02S_OPER_STP: assert(false && "TODO: W65C02S_OPER_STP"); break;

        default: UNREACHABLE(); break;
    }
}

void PhysicalW65C02::update_nz(uint8_t value) {
	p &= ~(FLAG_N_MASK | FLAG_Z_MASK);
	if (value == 0)        { p |= FLAG_Z_MASK; }
	if ((int8_t)value < 0) { p |= FLAG_N_MASK; }
}

void PhysicalW65C02::update_z(uint8_t value) {
    p &= ~FLAG_Z_MASK;
    if (value == 0) { p |= FLAG_Z_MASK; }
}

void PhysicalW65C02::set_c(bool c) {
    if (c) { p |= FLAG_C_MASK; } else { p &= ~FLAG_C_MASK; }
}

void PhysicalW65C02::set_v(bool v) {
    if (v) { p |= FLAG_V_MASK; } else { p &= ~FLAG_V_MASK; }
}

} // namespace physicalw65c02