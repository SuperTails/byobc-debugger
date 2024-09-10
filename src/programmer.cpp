#include "programmer.h"
#include "gpio.h"
#include "pins.h"

static void delay_loop(int amount) {
  volatile int i = 0;
  for (i = 0; i < amount; ++i);
}

namespace programmer {

#define WRITE_PAIR(ADDR, DATA)                  \
    do {                                        \
        gpio::write_addr_bus(ADDR | 0x8000);    \
        delay_loop(5);                          \
        WE_PORT.OUTCLR = WE_PIN_MASK;           \
        gpio::write_data_bus(DATA);             \
        delay_loop(5);                          \
        WE_PORT.OUTSET = WE_PIN_MASK;           \
        delay_loop(1);                          \
    } while (0)

void sector_erase(uint16_t sector) {
    gpio::write_progb(false); // disable EEPROM output 

    WRITE_PAIR(0x5555, 0xAA);
    WRITE_PAIR(0x2AAA, 0x55);
    WRITE_PAIR(0x5555, 0x80);
    WRITE_PAIR(0x5555, 0xAA);
    WRITE_PAIR(0x2AAA, 0x55);
    WRITE_PAIR(sector, 0x30);

    delay_loop(50000);
    delay_loop(50000);

    gpio::write_progb(true); // enable EEPROM output 
}

void byte_program(uint16_t addr, uint8_t value) {
    gpio::write_progb(false); // disable EEPROM output 

    WRITE_PAIR(0x5555, 0xAA);
    WRITE_PAIR(0x2AAA, 0x55);
    WRITE_PAIR(0x5555, 0xA0);

    WRITE_PAIR(addr | 0x8000, value);

    delay_loop(20);

    gpio::write_progb(true); // re-enable EEPROM output
}

} // namespace programmer