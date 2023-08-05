#include "i2c.h"
#include <Arduino.h>

namespace i2c {
	void init() {
		// See page 338 for clock calculation equation

		// Assume default CLK_MAIN is 20MHz,
		// prescaler for CLK_PER is 6 -> CLK_PER is 3.3MHz
		// Desired f_SCL is 100kHz.
		// tR should be less than 1000ns, so... 100ns?

		// approximate tOF as 100ns

		// 16MHz / (2 * 100kHz) - (5 + (16MHz * 1000ns) / 2) == 11.498
		// baud is 11

		// at 16MHz: 8
		// at 20MHz: 11

		TWI0.MCTRLA &= ~0xDF; // Disable I2C
		TWI0.MBAUD = /*8*/ 11 /*67*/ /*147*/ /*255*/;
		TWI0.MCTRLA |= 0x01; // Enable I2C host
		TWI0.MSTATUS = (TWI0.MSTATUS & ~0x3) | 0x1; // Set bus state to idle
	}

	static inline void wait_for_wif() {
		while ((TWI0.MSTATUS & 0x40) == 0);
		TWI0.MSTATUS |= 0x40; // Clear WIF
	}

	static inline void wait_for_rif() {
		while ((TWI0.MSTATUS & 0x80) == 0);
		TWI0.MSTATUS |= 0x80; // Clear RIF
	}

	Writer::Writer(uint8_t addr) {
		start(addr, Mode::Write);
	}

	void Writer::write(const uint8_t data[], size_t len) {
		i2c::write(data, len);
	}

	Writer::~Writer() {
		i2c::stop_write();
	}

	Reader::Reader(uint8_t addr) : first(true) {
		start(addr, Mode::Read);
	}

	void Reader::read(uint8_t data[], size_t len) {
		if (len > 0) {
			if (first) {
				first = false;
				wait_for_rif();
				data[0] = TWI0.MDATA;
			} else {
				for (size_t i = 0; i < len; ++i) {
					// Issue an ACK for the previous byte and continue reading
					TWI0.MCTRLB |= 0x6;

					wait_for_rif();

					data[i] = TWI0.MDATA;
				}
			}
		}
	}

	Reader::~Reader() {
		// Issue an ACK for the previous byte and stop
		TWI0.MCTRLB |= 0x7;
	}

	void start(uint8_t addr, Mode mode) {
		uint8_t a = (addr << 1) | static_cast<uint8_t>(mode);

		// TODO: Check status flags

		TWI0.MADDR = a;
	}

	void write(const uint8_t data[], size_t len) {
		for (int i = 0; i < len; ++i) {
			wait_for_wif();
			TWI0.MDATA = data[i];
		}
	}

	void stop_write() {
		wait_for_wif();

		// Send a stop bit
		TWI0.MCTRLB |= 0x3;
	}
}