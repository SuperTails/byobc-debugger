#pragma once

#include <stdint.h>

namespace physicalw65c02 {

#define FLAG_N_BIT 7
#define FLAG_V_BIT 6
#define FLAG_B_BIT 4
#define FLAG_D_BIT 3
#define FLAG_I_BIT 2
#define FLAG_Z_BIT 1
#define FLAG_C_BIT 0

#define FLAG_N_MASK (1 << FLAG_N_BIT)
#define FLAG_V_MASK (1 << FLAG_V_BIT)
#define FLAG_B_MASK (1 << FLAG_B_BIT)
#define FLAG_D_MASK (1 << FLAG_D_BIT)
#define FLAG_I_MASK (1 << FLAG_I_BIT)
#define FLAG_Z_MASK (1 << FLAG_Z_BIT)
#define FLAG_C_MASK (1 << FLAG_C_BIT)


enum class Mode {
    FETCH,

    IMMEDIATE,
    IMPLIED,
    IMPLIED_1C,
    IMPLIED_X,
    IMPLIED_Y,
    ZEROPAGE,
    ZEROPAGE_BIT,
    ZEROPAGE_INDIRECT,
    ZEROPAGE_X,
    ZEROPAGE_Y,
    ZEROPAGE_INDIRECT_X,
    ZEROPAGE_INDIRECT_Y,
    ZEROPAGE_INDIRECT_Y_STORE,
    ABSOLUTE,
    ABSOLUTE_X,
    ABSOLUTE_Y,
    ABSOLUTE_X_STORE,
    ABSOLUTE_Y_STORE,
    ABSOLUTE_JUMP,
    ABSOLUTE_INDIRECT,
    ABSOLUTE_INDIRECT_X,
    RELATIVE,
    RELATIVE_BIT,
    RMW_ZEROPAGE,
    RMW_ZEROPAGE_X,
    RMW_ABSOLUTE,
    RMW_ABSOLUTE_X,
    RMW_ABSOLUTE_Y,
    SUBROUTINE,
    RETURN_SUB,
    STACK_BRK,
    STACK_PUSH,
    STACK_PULL,
    STACK_RTI,
    NOP_5C,
    INT_WAIT_STOP,
};

enum Oper {
    W65C02S_OPER_NOP,
    W65C02S_OPER_BRK,

    W65C02S_OPER_ORA,
    W65C02S_OPER_AND,
    W65C02S_OPER_EOR,
    W65C02S_OPER_ADC,
    W65C02S_OPER_SBC,
    W65C02S_OPER_ASL,
    W65C02S_OPER_ASR,
    W65C02S_OPER_LSR,
    W65C02S_OPER_ROL,
    W65C02S_OPER_ROR,
    W65C02S_OPER_BIT,
    W65C02S_OPER_INC,
    W65C02S_OPER_DEC,
    W65C02S_OPER_CMP,
    W65C02S_OPER_CPX,
    W65C02S_OPER_CPY,

    W65C02S_OPER_TSB,
    W65C02S_OPER_TRB,

    W65C02S_OPER_CLC,
    W65C02S_OPER_SEC,
    W65C02S_OPER_CLI,
    W65C02S_OPER_SEI,
    W65C02S_OPER_CLD,
    W65C02S_OPER_SED,
    W65C02S_OPER_CLV,

    W65C02S_OPER_JMP,
    W65C02S_OPER_JSR,
    W65C02S_OPER_RTI,
    W65C02S_OPER_RTS,

    W65C02S_OPER_LDA,
    W65C02S_OPER_LDX,
    W65C02S_OPER_LDY,
    W65C02S_OPER_STA,
    W65C02S_OPER_STX,
    W65C02S_OPER_STY,
    W65C02S_OPER_STZ,

    W65C02S_OPER_TXA,
    W65C02S_OPER_TYA,
    W65C02S_OPER_TAX,
    W65C02S_OPER_TAY,
    W65C02S_OPER_TXS,
    W65C02S_OPER_TSX,

    W65C02S_OPER_PHP,
    W65C02S_OPER_PLP,
    W65C02S_OPER_PHA,
    W65C02S_OPER_PLA,
    W65C02S_OPER_PHY,
    W65C02S_OPER_PLY,
    W65C02S_OPER_PHX,
    W65C02S_OPER_PLX,

    W65C02S_OPER_BRA,
    W65C02S_OPER_BEQ,
    W65C02S_OPER_BNE,
    W65C02S_OPER_BPL,
    W65C02S_OPER_BMI,
    W65C02S_OPER_BVS,
    W65C02S_OPER_BVC,
    W65C02S_OPER_BCS,
    W65C02S_OPER_BCC,

    W65C02S_OPER_WAI,
    W65C02S_OPER_STP
};

const bool rwb_read  = true;
const bool rwb_write = false;

struct BusState {
    uint16_t addr;
    uint8_t  data;
    bool sync;
    bool rwb;
    bool vpb;
    bool error;
};

struct BusInState {
    uint8_t  data;
    bool     resb;
    bool     irqb;
    bool     nmib;
};

class PhysicalW65C02 {
public:
	uint8_t a;   // Accumulator
	uint8_t x;   // X Index Register
	uint8_t y;   // Y Index Register
	uint8_t s;   // Stack Pointer
	uint8_t p = (FLAG_I_MASK | (1 << 5));	 // Processor Status Register
	uint16_t pc; // Program Counter

    // The half-cycle count within the current instruction.
    // If this is 0 or 1 the processor is always in fetch or interrupt mode.
    uint8_t seq_cycle = 0;

    // The current execution mode of the processor.
    // When the halfcycle is 0 or 1, this either fetching or handling an interrupt.
    // Fetch mode will change to the appropriate addressing mode once the instruction has been fetched.
    Mode mode;

    Oper oper;

    bool prev_nmi_state = false;

    bool in_rst = false;
    bool in_nmi = false;
    bool in_irq = false;

    bool error = false;

    // Advances the processor by one halfcycle.
    void tick_cycle(BusInState state);

    void get_bus_state(BusState& out) const;

private:

    uint8_t bal;
    uint16_t adr;

    uint8_t tmp;

    void tick_cycle_fetch(BusInState state);
    void get_bus_state_fetch(BusState& out) const;

    void tick_cycle_immediate(BusInState state);
    void get_bus_state_immediate(BusState& out) const;

    void tick_cycle_implied(BusInState state);
    void get_bus_state_implied(BusState& out) const;

    void tick_cycle_stack_brk(BusInState state);
    void get_bus_state_stack_brk(BusState& out) const;
    
    void tick_cycle_zeropage(BusInState state);
    void get_bus_state_zeropage(BusState& out) const;

    void tick_cycle_zeropage_bit(BusInState state);
    void get_bus_state_zeropage_bit(BusState& out) const;

    void tick_cycle_zeropage_indirect(BusInState state);
    void get_bus_state_zeropage_indirect(BusState& out) const;

    void tick_cycle_zeropage_x(BusInState state);
    void get_bus_state_zeropage_x(BusState& out) const;

    void tick_cycle_zeropage_y(BusInState state);
    void get_bus_state_zeropage_y(BusState& out) const;

    void tick_cycle_zeropage_indirect_x(BusInState state);
    void get_bus_state_zeropage_indirect_x(BusState& out) const;

    void tick_cycle_zeropage_indirect_y(BusInState state);
    void get_bus_state_zeropage_indirect_y(BusState& out) const;

    void tick_cycle_zeropage_indirect_y_store(BusInState state);
    void get_bus_state_zeropage_indirect_y_store(BusState& out) const;

    void tick_cycle_absolute(BusInState state);
    void get_bus_state_absolute(BusState& out) const;

    void tick_cycle_absolute_x(BusInState state);
    void get_bus_state_absolute_x(BusState& out) const;

    void tick_cycle_absolute_y(BusInState state);
    void get_bus_state_absolute_y(BusState& out) const;

    void tick_cycle_absolute_x_store(BusInState state);
    void get_bus_state_absolute_x_store(BusState& out) const;

    void tick_cycle_absolute_y_store(BusInState state);
    void get_bus_state_absolute_y_store(BusState& out) const;

    void tick_cycle_absolute_jump(BusInState state);
    void get_bus_state_absolute_jump(BusState& out) const;

    void tick_cycle_absolute_indirect(BusInState state);
    void get_bus_state_absolute_indirect(BusState& out) const;

    void tick_cycle_absolute_indirect_x(BusInState state);
    void get_bus_state_absolute_indirect_x(BusState& out) const;

    void tick_cycle_rmw_zeropage(BusInState state);
    void get_bus_state_rmw_zeropage(BusState& out) const;

    void tick_cycle_rmw_zeropage_x(BusInState state);
    void get_bus_state_rmw_zeropage_x(BusState& out) const;

    void tick_cycle_rmw_absolute(BusInState state);
    void get_bus_state_rmw_absolute(BusState& out) const;

    void tick_cycle_rmw_absolute_x(BusInState state);
    void get_bus_state_rmw_absolute_x(BusState& out) const;

    void tick_cycle_rmw_absolute_y(BusInState state);
    void get_bus_state_rmw_absolute_y(BusState& out) const;

    void tick_cycle_subroutine(BusInState state);
    void get_bus_state_subroutine(BusState& out) const;

    void tick_cycle_return_sub(BusInState state);
    void get_bus_state_return_sub(BusState& out) const;

    void tick_cycle_relative(BusInState state);
    void get_bus_state_relative(BusState& out) const;
    
    void tick_cycle_stack_push(BusInState state);
    void get_bus_state_stack_push(BusState& out) const;

    void tick_cycle_stack_pull(BusInState state);
    void get_bus_state_stack_pull(BusState& out) const;

    void tick_cycle_stack_rti(BusInState state);
    void get_bus_state_stack_rti(BusState& out) const;

    void execute_accum_oper(uint8_t arg);
    uint8_t execute_unary_oper(uint8_t arg);

    void update_nz(uint8_t value);
    void update_z(uint8_t value);
    void set_c(bool c);
    void set_v(bool v);

    void adc(uint8_t arg);
};

} // namespace physicalw65c02

using physicalw65c02::PhysicalW65C02;