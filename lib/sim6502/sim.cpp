#include "sim.h"
#include "disassemble.h"

/*typedef void (Cpu::*single_cycle_op_t)();

single_cycle_op_t SINGLE_CYCLE_OPS[] = {
  // Ora, And, Eor, Adc,
  nullptr, nullptr, nullptr, nullptr,
  // Sta, Lda, Cmp, Sbc,
  nullptr, nullptr, nullptr, nullptr,
  // Asl, Rol, Lsr, Ror,
  Cpu::asl, Cpu::rol, Cpu::lsr, Cpu::ror,
  // Stx, Ldx, Dec, Inc,
  nullptr, nullptr, nullptr, nullptr,

  // Txa, Tax, Dex, Nop,
  Cpu::txa, Cpu::tax, Cpu::dex, Cpu::nop,

  // Bpl, Bmi, Bvc, Bvs,
  nullptr, nullptr, nullptr, nullptr,
  // Bcc, Bcs, Bne, Beq,
  nullptr, nullptr, nullptr, nullptr,

  // Brk, Jsr, Rti, Rts,
  nullptr, nullptr, nullptr, nullptr,

  // Php, Plp, Pha, Pla,
  nullptr, nullptr, nullptr, nullptr,
  // Dey, Tay, Iny, Inx,
  Cpu::dey, Cpu::tay, Cpu::iny, Cpu::inx,

  // Clc, Sec, Cli, Sei,
  Cpu::clc, Cpu::sec, Cpu::cli, Cpu::sei,
  // Tya, Clv, Cld, Sed,
  Cpu::tya, Cpu::clv, Cpu::cld, Cpu::sed,

  // Bit, Jmp, JmpInd,
  nullptr, nullptr, nullptr, nullptr,
  // Sty, Ldy, Cpy, Cpx
  nullptr, nullptr, nullptr, nullptr,
};*/

#if 0
void Cpu::update(uint16_t ab, uint8_t db, bus_status_t bus) {

    if (bus.phi2()) {
        if (!bus.resb()) {
            set_state(CpuState::Int0);
            int_type = Interrupt::Reset;
            return;
        }

        Instr instr; 

        if (state_known) {
            switch (state) {
            case CpuState::Int0:
            case CpuState::Int1:
            case CpuState::Int2:
            case CpuState::Int3:
            case CpuState::Int4:
            case CpuState::Int5:
            case CpuState::Int6:
            case CpuState::Int7:
                break;

            case CpuState::Fetch:
                if (!bus.sync()) {
                    make_unknown();
                    return;
                }

                pc_known = true;
                pc = ab;
                instr = decode(db);
                ir_op = instr.base_op;
                ir_mode = instr.addr_mode;
                illegal = instr.illegal;
                if (instr.illegal) {
                    make_unknown();
                    return;
                }

                set_state(CpuState::Ex1);
                break;
            case CpuState::Ex1:
                if (ir_mode == AddrMode::Impl) {

                }
            }

        }
    } else {
    }
}

void Cpu::set_state(CpuState state) {
    state_known = true;
    this->state = state;
}
#endif