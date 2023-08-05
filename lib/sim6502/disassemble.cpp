#include "disassemble.h"

#include <string.h>
#include <stdio.h>

const BaseOp OP_ILLEGAL = static_cast<BaseOp>(0);

const AddrMode ADDR_ILLEGAL = static_cast<AddrMode>(0);

const BaseOp group_1_ops[8] = {
  BaseOp::Ora, BaseOp::And, BaseOp::Eor, BaseOp::Adc,
  BaseOp::Sta, BaseOp::Lda, BaseOp::Cmp, BaseOp::Sbc,
};

const AddrMode group_1_modes[8] = {
  AddrMode::ZpgIndX, AddrMode::Zpg, AddrMode::Imm, AddrMode::Abs,
  AddrMode::ZpgIndY, AddrMode::ZpgX, AddrMode::AbsY, AddrMode::AbsX,
};

const BaseOp group_2_ops[8] = {
  BaseOp::Asl, BaseOp::Rol, BaseOp::Lsr, BaseOp::Ror,
  BaseOp::Stx, BaseOp::Ldx, BaseOp::Dec, BaseOp::Inc,
};

const AddrMode group_2_modes[8] = {
  AddrMode::Imm, AddrMode::Zpg, AddrMode::Acc, AddrMode::Abs,
  ADDR_ILLEGAL, AddrMode::ZpgX, ADDR_ILLEGAL, AddrMode::AbsX,
};

const BaseOp branch_ops[8] = {
  BaseOp::Bpl, BaseOp::Bmi, BaseOp::Bvc, BaseOp::Bvs,
  BaseOp::Bcc, BaseOp::Bcs, BaseOp::Bne, BaseOp::Beq,
};

const BaseOp misc_ops_0[8] = {
  BaseOp::Php, BaseOp::Plp, BaseOp::Pha, BaseOp::Pla,
  BaseOp::Dey, BaseOp::Tay, BaseOp::Iny, BaseOp::Inx,
};

const BaseOp misc_ops_1[8] = {
  BaseOp::Clc, BaseOp::Sec, BaseOp::Cli, BaseOp::Sei,
  BaseOp::Tya, BaseOp::Clv, BaseOp::Cld, BaseOp::Sed,
};

const BaseOp group_3_ops[8] = {
  OP_ILLEGAL, BaseOp::Bit, BaseOp::Jmp, BaseOp::JmpInd,
  BaseOp::Sty, BaseOp::Ldy, BaseOp::Cpy, BaseOp::Cpx,
};

const AddrMode group_3_modes[8] = {
  AddrMode::Imm, AddrMode::Zpg, ADDR_ILLEGAL, AddrMode::Abs, 
  ADDR_ILLEGAL, AddrMode::ZpgX, ADDR_ILLEGAL, AddrMode::AbsX
};

DecodedInstr::DecodedInstr(uint8_t *data, size_t len) {
  Instr i = decode(data[0]);
  --len;
  op = i.base_op;
  mode = i.addr_mode;
  memcpy(args, data + 1, len);
  num_args = len;
}

void DecodedInstr::format(char buf[MAX_INSTR_DISPLAY_SIZE+1]) const {
    memcpy(buf, mnemonics[static_cast<int>(this->op)], 3);
    buf += 3;

    char arg0[3];
    if (this->num_args > 0) {
      sprintf(arg0, "%02hhX", this->args[0]);
    } else {
      memcpy(arg0, "??", 3);
    }

    char arg1[3];
    if (this->num_args > 1) {
      sprintf(arg1, "%02hhX", this->args[1]);
    } else {
      memcpy(arg1, "??", 3);
    }

    switch (this->mode) {
      case AddrMode::Impl:
        break;
      case AddrMode::Imm:
        buf += sprintf(buf, " #$%s", arg0);
        break;
      case AddrMode::Abs:
        buf += sprintf(buf, " $%s%s", arg1, arg0);
        break;
      case AddrMode::Zpg:
        buf += sprintf(buf, " $%s", arg0);
        break;
      case AddrMode::AbsX:
        buf += sprintf(buf, " $%s%s,X", arg1, arg0);
        break;
      case AddrMode::AbsY:
        buf += sprintf(buf, " $%s%s,Y", arg1, arg0);
        break;
      case AddrMode::ZpgX:
        buf += sprintf(buf, " $%s,X", arg0);
        break;
      case AddrMode::ZpgY:
        buf += sprintf(buf, " $%s,Y", arg0);
        break;
      case AddrMode::Ind:
        buf += sprintf(buf, " ($%s%s)", arg1, arg0);
        break;
      case AddrMode::ZpgIndX:
        buf += sprintf(buf, " ($%s,X)", arg0);
        break;
      case AddrMode::ZpgIndY:
        buf += sprintf(buf, " ($%s),Y", arg0);
        break;
      case AddrMode::Offset:
        // TODO:
        buf += sprintf(buf, " [$%s]", arg0);
        break;
      case AddrMode::Acc:
        buf += sprintf(buf, " A");
        break;
    }

    *buf = 0;
    return;

}

Instr decode(uint8_t opcode) {
  bool illegal = false;
  BaseOp base_op;
  AddrMode addr_mode;

  uint8_t group = opcode & 0x3;
  uint8_t addr = (opcode >> 2) & 0x7;
  uint8_t opc = (opcode >> 5) & 0x7;

  switch (group) {
  case 0b00:
    if (addr == 0b100) {
      base_op = branch_ops[opc];
      addr_mode = AddrMode::Offset;
    } else if (addr == 0b010) {
      base_op = misc_ops_0[opc];
      addr_mode = AddrMode::Impl;
    } else if (addr == 0b110) {
      base_op = misc_ops_1[opc];
      addr_mode = AddrMode::Impl;
    } else if (opcode == 0x00) {
      base_op = BaseOp::Brk;
      addr_mode = AddrMode::Impl;
    } else if (opcode == 0x20) {
      base_op = BaseOp::Jsr;
      addr_mode = AddrMode::Abs;
    } else if (opcode == 0x40) {
      base_op = BaseOp::Rti;
      addr_mode = AddrMode::Impl;
    } else if (opcode == 0x60) {
      base_op = BaseOp::Rts;
      addr_mode = AddrMode::Impl;
    } else {
      base_op = group_3_ops[opc];
      addr_mode = group_3_modes[addr];
      if (addr_mode == AddrMode::Imm && base_op == BaseOp::Sty) {
        illegal = true;
      } else if (addr_mode == AddrMode::Zpg && (base_op == BaseOp::Jmp || base_op == BaseOp::JmpInd)) {
        illegal = true;
      } else if (addr_mode == AddrMode::ZpgX && (base_op != BaseOp::Sty && base_op != BaseOp::Ldy)) {
        illegal = true;
      } else if (addr_mode == AddrMode::AbsX && base_op != BaseOp::Ldy) {
        illegal = true;
      }

      if (base_op == BaseOp::JmpInd) {
        addr_mode = AddrMode::Ind;
      }
    }
    break;
  case 0b01:
    base_op = group_1_ops[opc];
    addr_mode = group_1_modes[addr];
    if (base_op == BaseOp::Sta && addr_mode == AddrMode::Imm) {
      illegal = true;
    }
    break;
  case 0b10:
    if (addr == 0b100 || addr == 0b110) {
      illegal = true;
    } else {
      base_op = group_2_ops[opc];
      addr_mode = group_2_modes[addr];
      if (base_op == BaseOp::Ldx || base_op == BaseOp::Stx) {
        if (addr_mode == AddrMode::ZpgX) {
          addr_mode = AddrMode::ZpgY;
        } else if (addr_mode == AddrMode::AbsX) {
          addr_mode = AddrMode::AbsY;
        }
      }

      if (base_op != BaseOp::Ldx && addr_mode == AddrMode::Imm) {
        illegal = true;
      } else if (base_op == BaseOp::Stx && addr_mode == AddrMode::Acc) {
        base_op = BaseOp::Txa;
        addr_mode = AddrMode::Impl;
      } else if (base_op == BaseOp::Ldx && addr_mode == AddrMode::Acc) {
        base_op = BaseOp::Tax;
        addr_mode = AddrMode::Impl;
      } else if (base_op == BaseOp::Dec && addr_mode == AddrMode::Acc) {
        base_op = BaseOp::Dex;
        addr_mode = AddrMode::Impl;
      } else if (base_op == BaseOp::Inc && addr_mode == AddrMode::Acc) {
        base_op = BaseOp::Nop;
        addr_mode = AddrMode::Impl;
      } else if (base_op == BaseOp::Stx && addr_mode == AddrMode::AbsY) {
        illegal = true;
      }
    }
    break;
  case 0b11:
    illegal = true;
    break;
  }

  return { base_op, addr_mode, illegal };
}