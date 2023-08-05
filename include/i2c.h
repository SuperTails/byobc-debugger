#pragma once

#include <stdint.h>
#include <stddef.h>

namespace i2c {
	enum class Mode {
		Write = 0,
		Read = 1,
	};

	class Writer {
	public:
		Writer(uint8_t addr);

		Writer() = delete;
		Writer(Writer&) = delete;
		Writer(Writer&&) = delete;

		~Writer();

		void write(const uint8_t data[], size_t len);
	};

	class Reader {
	public:
		Reader(uint8_t addr);

		Reader() = delete;
		Reader(Reader&) = delete;
		Reader(Reader&&) = delete;

		~Reader();

		void read(uint8_t data[], size_t len);
	
	private:
		bool first;
	};

	void init();

	void start(uint8_t addr, Mode mode);

	void write(const uint8_t data[], size_t len);

	void stop_write();
}