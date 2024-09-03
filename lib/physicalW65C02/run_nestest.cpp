#include "physicalw65c02.h"

#ifdef TEST

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <cstdint>
#include <cassert>

using std::uint8_t;
using std::uint16_t;

std::vector<uint8_t> read_to_end(const char* path) {
	FILE* f = fopen(path, "r");
	assert(f);
	assert(fseek(f, 0, SEEK_END) == 0);
	off_t file_sz = ftell(f);
	assert(file_sz > 0);
	assert(fseek(f, 0, SEEK_SET)  == 0);

	std::vector<uint8_t> file_data;
	file_data.resize(file_sz);

	assert(fread(file_data.data(), 1, file_sz, f) == file_sz);

	return file_data;
}

struct LogLine {
	uint16_t pc;
	uint8_t a, x, y, p, s;
	int cyc;
};

std::vector<LogLine> read_log(const char* path) {
	FILE* f = fopen(path, "r"); assert(f);

	std::vector<LogLine> result;

	while (true) {
		char buf[256];

		if (fgets(buf, 256, f) == NULL) {
			assert(feof(f));
			return result;
		}

		printf("%s\n", buf);

		uint16_t pc = strtoul(buf +  0, NULL, 16);
		uint8_t  a  = strtoul(buf + 50, NULL, 16);
		uint8_t  x  = strtoul(buf + 55, NULL, 16);
		uint8_t  y  = strtoul(buf + 60, NULL, 16);
		uint8_t  p  = strtoul(buf + 65, NULL, 16);
		uint8_t  s  = strtoul(buf + 71, NULL, 16);

		int    cyc  = strtoul(buf + 90, NULL, 10);

		result.push_back({ pc, a, x, y, p, s, cyc });
	}
}

struct Rom {
	std::vector<uint8_t> prg_rom;
	std::vector<uint8_t> chr_rom;
};

Rom read_rom(const char *path) {
	FILE* f = fopen(path, "r"); assert(f);

	struct {
		char magic[4];			// 0 - 3
		uint8_t prg_rom_size;   // 4
		uint8_t chr_rom_size;   // 5
		uint8_t flags_6;		// 6
		uint8_t flags_7;		// 7
		uint8_t flags_8;		// 8
		uint8_t flags_9;		// 9
		uint8_t flags_10;		// 10
		uint8_t padding[5];		// 11 - 15
	} header;

	assert(fread((char*)&header, 16, 1, f) == 1);
	assert(strncmp(header.magic, "NES\x1A", 4) == 0);

	printf("Flags 6: %02x\n", (int)header.flags_6);
	
	size_t prg_rom_size = header.prg_rom_size * 16 * 1024;
	size_t chr_rom_size = header.chr_rom_size *  8 * 1024;

	std::vector<uint8_t> prg_rom;
	prg_rom.resize(prg_rom_size);
	assert(fread(prg_rom.data(), prg_rom_size, 1, f) == 1);

	std::vector<uint8_t> chr_rom;
	chr_rom.resize(chr_rom_size);
	assert(fread(chr_rom.data(), chr_rom_size, 1, f) == 1);

	assert(fgetc(f) == EOF);

	return { prg_rom, chr_rom };
}

// Subtract small - big
//
// 17 - 42
//
//   0001 0001
// - 0010 1010
// -----------
//
//   0001 0001
// + 1101 0110
// -----------
// 0 1110 0111
// C = 0 -> "borrow"

int main() {
	std::vector<LogLine> log = read_log("../nestest.log");

	Rom rom = read_rom("../nestest.nes");

	PhysicalW65C02 cpu = {};

	int cycle = 7;
	cpu.pc = 0xC000;
	cpu.p  = 0x24;
	cpu.s  = 0xFD;

	uint8_t internal_ram[0x800];

	for (LogLine& line : log) {
		while (cycle != line.cyc) {
			BusState bus_state;
			cpu.get_bus_state(bus_state);

			uint8_t *ptr;

			if (bus_state.addr < 0x2000) {
				ptr = &internal_ram[bus_state.addr & 0x07FF];
			} else if (0x2000 <= bus_state.addr && bus_state.addr < 0x8000) {
				assert(false && "TODO: memory access in the middle");
			} else if (0x8000 < bus_state.addr) {
				assert(bus_state.rwb); // No writes to ROM
				ptr = &rom.prg_rom[(bus_state.addr - 0x8000) & 0x3FFF];
			}

			if (!bus_state.rwb) {
				*ptr = bus_state.data;
			}

			cpu.tick_cycle({ *ptr });

			++cycle;
		}

		if (line.pc == 0xE19D) {
			printf("hewo\n");
		}
		
		printf("%04X\n", line.pc);

		assert(cpu.seq_cycle == 0);
		assert(cpu.pc == line.pc);
		assert(cpu.a  == line.a );
		assert(cpu.x  == line.x );
		assert(cpu.y  == line.y );
		assert(cpu.p  == line.p );
		assert(cpu.s  == line.s );
	}

	printf("%lu\n", rom.prg_rom.size());
}

#endif