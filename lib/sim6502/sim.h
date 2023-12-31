#pragma once

#include <stdint.h>
#include "disassemble.h"

struct status_register {
	uint8_t sr;

	void update_nz(uint8_t value);

	void set_n(bool v);
	void set_v(bool v);
	void set_c(bool v);
	void set_i(bool v);
	void set_d(bool v);
};

struct bus_status_t {
	// From MSB to LSB:
	// MLB, IRQB, RDY, VPB, --,   --,   GPIO1, WE
	// --,  --,   RWB, BE,  PHI2, RESB, SYNC,  NMIB
	uint16_t bits;

	bool mlb()   const { return this->bits & 0x8000; }
	bool irqb()  const { return this->bits & 0x4000; }
	bool rdy()   const { return this->bits & 0x2000; }
	bool vpb()   const { return this->bits & 0x1000; }
	bool gpio1() const { return this->bits & 0x0200; }
	bool we()    const { return this->bits & 0x0100; }
	bool rwb()   const { return this->bits & 0x0020; }
	bool be()    const { return this->bits & 0x0010; }
	bool phi2()  const { return this->bits & 0x0008; }
	bool resb()  const { return this->bits & 0x0004; }
	bool sync()  const { return this->bits & 0x0002; }
	bool nmib()  const { return this->bits & 0x0001; }
};

/*
enum class Interrupt {
	Irq,
	Reset,
	Nmi,
};

class Cpu {
public:
	Cpu() = default;

	void update(uint16_t ab, uint8_t db, bus_status_t status);
private:
	void set_state(CpuState state);

	void make_unknown();

	CpuState state;
	Interrupt int_type;

	BaseOp ir_op;
	AddrMode ir_mode;

	uint16_t pc;
	status_register sr;
	uint8_t a;
	uint8_t x;
	uint8_t y;
	uint8_t sp;

	bool a_known = false;
	bool x_known = false;
	bool y_known = false;
	bool sp_known = false;
	bool sr_known = false;
	bool pc_known = false;
	bool state_known = false;

	bool illegal = false;

public:
	inline void tay() { y = a; sr.update_nz(a); }
	inline void tax() { x = a; sr.update_nz(a); }
	inline void txa() { a = x; sr.update_nz(a); }
	inline void tya() { a = y; sr.update_nz(a); }
	inline void nop() {}
	inline void dex() { --x; sr.update_nz(x); }
	inline void inx() { ++x; sr.update_nz(x); }
	inline void dey() { --y; sr.update_nz(y); }
	inline void iny() { ++y; sr.update_nz(y); }
	inline void clc() { sr.set_c(0); }
	inline void sec() { sr.set_c(1); }
	//inline void cli() { sr.set_i(0); }
	//inline void sei() { sr.set_i(1); }
	inline void clv() { sr.set_v(0); }
	inline void cld() { sr.set_d(0); }
	inline void sed() { sr.set_d(1); }
};
*/