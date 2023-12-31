#pragma once

#include <stdint.h>
#include "sim.h"

namespace gpio {
	enum class AddressBusMode {
		// Enables the 6502 and causes the debugger to read from the address bus
		CpuDriven,
		// Disables the 6502 and allows the debugger to read from/write to the address bus
		DebuggerDriven,
	};

	void set_addr_bus_mode(AddressBusMode);

	enum class Direction {
		Input = 0b0,
		Output = 0b1,
	};

	void set_data_bus_dir(Direction dir);
	void write_data_bus(uint8_t data);
	uint8_t read_data_bus();

	void set_addr_bus_dir(Direction dir);
	void write_addr_bus(uint16_t data);
	uint16_t read_addr_bus();

	void set_irqb_dir(Direction dir);
	void set_gpio1_dir(Direction dir);
	void set_phi2_dir(Direction dir);
	void set_rwb_dir(Direction dir);
	void set_sync_dir(Direction dir);
	void set_vpb_dir(Direction dir);
	void set_resb_dir(Direction dir);
	void set_nmib_dir(Direction dir);

	void write_irqb(bool level);
	void write_gpio1(bool level);
	void write_we(bool level);
	void write_be(bool level);
	void write_phi2(bool level);
	void write_rwb(bool level);
	void write_sync(bool level);
	void write_vpb(bool level);
	void write_resb(bool level);
	void write_nmib(bool level);

	using status_t = ::bus_status_t;

	status_t read_status();

	//void eeprom_page_write(uint16_t addr, const uint8_t data[128]);

	//void eeprom_page_read(uint16_t addr, uint8_t data[128]);
}