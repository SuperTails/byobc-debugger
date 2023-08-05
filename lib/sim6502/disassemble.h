#pragma once

#include <stdint.h>
#include <stddef.h>

enum class BaseOp {
  Ora, And, Eor, Adc,
  Sta, Lda, Cmp, Sbc,
  Asl, Rol, Lsr, Ror,
  Stx, Ldx, Dec, Inc,

  Txa, Tax, Dex, Nop,

  Bpl, Bmi, Bvc, Bvs,
  Bcc, Bcs, Bne, Beq,

  Brk, Jsr, Rti, Rts,

  Php, Plp, Pha, Pla,
  Dey, Tay, Iny, Inx,

  Clc, Sec, Cli, Sei,
  Tya, Clv, Cld, Sed,

  Bit, Jmp, JmpInd,
  Sty, Ldy, Cpy, Cpx
};

const char *const mnemonics[] = {
  "ora", "and", "eor", "adc",
  "sta", "lda", "cmp", "sbc",
  "asl", "rol", "lsr", "ror",
  "stx", "ldx", "dec", "inc",

  "txa", "tax", "dex", "nop",

  "bpl", "bmi", "bvc", "bvs",
  "bcc", "bcs", "bne", "beq",

  "brk", "jsr", "rti", "rts",

  "php", "plp", "pha", "pla",
  "dey", "tay", "iny", "inx",

  "clc", "sec", "cli", "sei",
  "tya", "clv", "cld", "sed",
  
  "bit", "jmp", "jmp",
  "sty", "ldy", "cpy", "cpx"
};

enum class AddrMode {
  Impl,
  ZpgIndX,
  Zpg,
  Imm,
  Abs,
  ZpgIndY,
  ZpgX,
  AbsY,
  AbsX,
  Acc,
  ZpgY,
  Offset, // only for branches
  Ind,    // only for jump indirect
};

// "lda $3120,X"
#define MAX_INSTR_DISPLAY_SIZE (3 + 1 + 1 + 4 + 2)

struct DecodedInstr {
  BaseOp op;
  AddrMode mode;
  uint8_t args[2];
  uint8_t num_args;

  DecodedInstr(uint8_t *data, size_t len);

  void format(char buf[MAX_INSTR_DISPLAY_SIZE+1]) const;
};

struct Instr {
  BaseOp base_op;
  AddrMode addr_mode;
  bool illegal;
};

Instr decode(uint8_t opcode);