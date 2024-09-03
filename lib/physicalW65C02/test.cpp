#ifdef TEST

#include "../json.h"
#include "physicalw65c02.h"

#include <cassert>

void run_test_case(TestCase &test_case, PhysicalW65C02 &cpu, uint8_t ram[65536]) {
	cpu = PhysicalW65C02 {};

	cpu.pc = test_case.initial.pc;
	cpu.s  = test_case.initial.s;
	cpu.a  = test_case.initial.a;
	cpu.x  = test_case.initial.x;
	cpu.y  = test_case.initial.y;
	cpu.p  = test_case.initial.p;

	for (auto [addr, data] : test_case.initial.ram) {
		ram[addr] = data;
	}

	for (int i = 0; i < test_case.cycles.size(); ++i) {
		BusState state;
		cpu.get_bus_state(state);

		assert(state.addr == test_case.cycles[i].addr);
		assert(state.rwb == (bool)test_case.cycles[i].rw);
		if (!state.rwb) {
			assert(state.data == test_case.cycles[i].data);
			ram[state.addr] = state.data;
		}

		cpu.tick_cycle({ ram[state.addr] });
	}

	assert(cpu.seq_cycle == 0);
	assert(cpu.pc == test_case.final.pc);
	assert(cpu.s  == test_case.final.s );
	assert(cpu.a  == test_case.final.a );
	assert(cpu.x  == test_case.final.x );
	assert(cpu.y  == test_case.final.y );
	assert((cpu.p & ~0x20)  == (test_case.final.p & ~0x20));

	for (auto [addr, data] : test_case.final.ram) {
		assert(data == ram[addr]);
	}
}

int main() {
	for (int i = 0x00; i <= 0xFF; ++i) {
		// TODO: WDC/CMOS-specific instructions
		uint8_t lo = i & 0x0F;
		if ((lo == 0x02 && i != 0xA2) || lo == 0x03 || lo == 0x07 || lo == 0x0B || lo == 0x0F) { continue; }

		// TODO: Indirect, Y
		if (i == 0x11 || i == 0x31 || i == 0x51) { continue; }

		// TODO: TSB
		if (i == 0x04 || i == 0x0C) { continue; }
		// TODO: ZEROPAGE_BIT
		if (i == 0x07) { continue; }

		// TODO: BPL ???
		if (i == 0x10 || i == 0x30 || i == 0x50 || i == 0x70 || i == 0x90 || i == 0xB0 || i == 0xD0 || i == 0xF0) { continue; }

		// TODO: Absolute, y and absolute, x ???
		if (i == 0x19 || i == 0x1d || i == 0x39 || i == 0x3C || i == 0x3D || i == 0x3E || i == 0x59) { continue; }

		// RMW: absolute, X
		if (i == 0x1e) { continue; }

		// 5 cycle nop
		if (i == 0x5C) { continue; }


		char path_buf[256];
		snprintf(path_buf, 256, "../wdc65c02/%02x.json", i);
		if (access(path_buf, F_OK) != 0) {
			printf("Skipping nonexistent file %s\n", path_buf);
		}
		printf("Running test %s\n", path_buf);

		std::vector<TestCase> test_cases = parse_test_cases(path_buf);

		PhysicalW65C02 cpu {};
		uint8_t ram[65536];

		memset(ram, 0xA5, 65536);

		for (int i = 0; i < test_cases.size(); ++i) {
			run_test_case(test_cases[i], cpu, ram);
		}
	}
}

#endif