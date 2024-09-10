#include "gpio.h"
#include "pins.h"
#include <Arduino.h>

namespace gpio {
	void set_addr_bus_mode(AddressBusMode mode) {
		switch (mode) {
			case AddressBusMode::CpuDriven:
				set_addr_bus_dir(Direction::Input);
				write_be(true);
				break;
			case AddressBusMode::DebuggerDriven:
				write_be(false);
				set_addr_bus_dir(Direction::Output);
				break;
		}
	}

	void set_data_bus_dir(Direction dir) {
		uint8_t mask = -static_cast<int8_t>(dir);
		PORTD.DIR = mask;
	}

	void write_data_bus(uint8_t data) {
		PORTD.OUT = data;
	}

	uint8_t read_data_bus() {
		return PORTD.IN;
	}

	void set_addr_bus_dir(Direction dir) {
		uint8_t mask = -static_cast<int16_t>(dir);

		PORTE.DIR = (PORTE.DIR & ~0x0F) | (mask & 0x0F);
		PORTF.DIR = (PORTF.DIR & ~0x0F) | (mask & 0x0F);
		PORTC.DIR = mask;
	}

	void write_addr_bus(uint16_t data) {
		PORTE.OUT = (PORTE.OUT & ~0x0F) | ((data >> 0) & 0x0F);
		PORTF.OUT = (PORTF.OUT & ~0x0F) | ((data >> 4) & 0x0F);
		PORTC.OUT = data >> 8;
	}

	uint16_t read_addr_bus() {
		uint16_t lo_nybble = PORTE.IN & 0x0F;
		uint16_t hi_nybble = PORTF.IN & 0x0F;
		uint16_t hi_byte   = PORTC.IN & 0xFF;

		return (hi_byte << 8) | (hi_nybble << 4) | (lo_nybble);

	}

	status_t read_status() {
		uint16_t lo_byte = PORTB.IN & 0x3F;
		uint16_t hi_byte = PORTA.IN & 0xF2;

		return { (hi_byte << 8) | lo_byte };
	}

	void set_irqb_dir(Direction dir) {
		IRQB_PORT.DIR = (IRQB_PORT.DIR & ~IRQB_PIN_MASK) | (static_cast<uint8_t>(dir) << IRQB_PIN);
	}
	void set_progb_dir(Direction dir) {
		GPIO1_PORT.DIR = (GPIO1_PORT.DIR & ~GPIO1_PIN_MASK) | (static_cast<uint8_t>(dir) << GPIO1_PIN);
	}
	void set_phi2_dir(Direction dir) {
		PHI2_PORT.DIR = (PHI2_PORT.DIR & ~PHI2_PIN_MASK) | (static_cast<uint8_t>(dir) << PHI2_PIN);
	}
	void set_rwb_dir(Direction dir) {
		RWB_PORT.DIR = (RWB_PORT.DIR & ~RWB_PIN_MASK) | (static_cast<uint8_t>(dir) << RWB_PIN);
	}
	void set_sync_dir(Direction dir) {
		SYNC_PORT.DIR = (SYNC_PORT.DIR & ~SYNC_PIN_MASK) | (static_cast<uint8_t>(dir) << SYNC_PIN);
	}
	void set_vpb_dir(Direction dir) {
		VPB_PORT.DIR = (VPB_PORT.DIR & ~VPB_PIN_MASK) | (static_cast<uint8_t>(dir) << VPB_PIN);
	}
	void set_resb_dir(Direction dir) {
		RESB_PORT.DIR = (RESB_PORT.DIR & ~RESB_PIN_MASK) | (static_cast<uint8_t>(dir) << RESB_PIN);
	}
	void set_nmib_dir(Direction dir) {
		NMIB_PORT.DIR = (NMIB_PORT.DIR & ~NMIB_PIN_MASK) | (static_cast<uint8_t>(dir) << NMIB_PIN);
	}

	void write_irqb(bool level) {
		IRQB_PORT.OUT = (IRQB_PORT.OUT & ~IRQB_PIN_MASK) | (level << IRQB_PIN);
	}
	void write_progb(bool level) {
		GPIO1_PORT.OUT = (GPIO1_PORT.OUT & ~GPIO1_PIN_MASK) | (level << GPIO1_PIN);
	}
	void write_we(bool level) {
		WE_PORT.OUT = (WE_PORT.OUT & ~WE_PIN_MASK) | (level << WE_PIN);
	}
	void write_be(bool level) {
		BE_PORT.OUT = (BE_PORT.OUT & ~BE_PIN_MASK) | (level << BE_PIN);
	}
	void write_phi2(bool level) {
		PHI2_PORT.OUT = (PHI2_PORT.OUT & ~PHI2_PIN_MASK) | (level << PHI2_PIN);
	}
	void write_rwb(bool level) {
		RWB_PORT.OUT = (RWB_PORT.OUT & ~RWB_PIN_MASK) | (level << RWB_PIN);
	}
	void write_sync(bool level) {
		SYNC_PORT.OUT = (SYNC_PORT.OUT & ~SYNC_PIN_MASK) | (level << SYNC_PIN);
	}
	void write_vpb(bool level) {
		VPB_PORT.OUT = (VPB_PORT.OUT & ~VPB_PIN_MASK) | (level << VPB_PIN);
	}
	void write_resb(bool level) {
		RESB_PORT.OUT = (RESB_PORT.OUT & ~RESB_PIN_MASK) | (level << RESB_PIN);
	}
	void write_nmib(bool level) {
		NMIB_PORT.OUT = (NMIB_PORT.OUT & ~NMIB_PIN_MASK) | (level << NMIB_PIN);
	}
}