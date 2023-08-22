#pragma once

#include "sim.h"
#include <stdint.h>

struct BusState {
	uint16_t addr;
	bus_status_t status;
	uint8_t data;
	uint8_t _padding;
};

enum class CommandType : uint8_t {
	Ping = 0x1,
	WriteEEPROM = 0x2,
	ReadMemory = 0x3,
	SetBreakpoint = 0x4,
	ResetCpu = 0x5,
	GetBusState = 0x6,
	Step = 0x7,
	StepCycle = 0x8,
	StepHalfCycle = 0x9,
	Continue = 0xA,
	HitBreakpoint = 0xB,
	PrintInfo = 0xC,
	MAX_CMD
};

struct WriteEEPROMCmd {
	uint16_t addr;
	uint8_t data[128];
	uint16_t checksum;
};

struct ReadMemoryCmd {
	uint16_t addr;
	uint16_t len;
};

struct SetBreakpointCmd {
	uint16_t addr;
};

struct HitBreakpointCmd {
	uint8_t which;
};

struct Command {
	CommandType ty;
	union {
		WriteEEPROMCmd write_eeprom;
		ReadMemoryCmd read_memory;
		SetBreakpointCmd set_breakpoint;
	};
};

#define ERR_NO_CMD -1
#define ERR_BAD_CMD -2
#define ERR_BAD_ADDR -3
#define ERR_BAD_CHECKSUM -4

// Returns 0 on success and stores the command data in `cmd`.
// Returns ERR_NO_CMD if no command has been sent yet.
int get_command(Command &cmd);

void hit_breakpoint(uint8_t which);