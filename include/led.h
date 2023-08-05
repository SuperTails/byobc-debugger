#pragma once

#include "gpio.h"

#include <stdint.h>
#include <stddef.h>

#define LED_ADDR 0x00

enum class LedDriverReg {
    Digit0 = 0b00001,
    Digit1 = 0b00010,
    Digit2 = 0b00011,
    Digit3 = 0b00100,
    Digit4 = 0b00101,
    Digit5 = 0b00110,
    Digit6 = 0b00111,
    Digit7 = 0b01000,

    DecodeMode      = 0b01001,
    GlobalIntensity = 0b01010,
    ScanLimit       = 0b01011,
    Shutdown        = 0b01100,
    Feature         = 0b01110,

    Digit01Intensity = 0b10000,
    Digit23Intensity = 0b10001,
    Digit45Intensity = 0b10010,
    Digit67Intensity = 0b10011,

    KeyA = 0b11100,
    KeyB = 0b11101
};

LedDriverReg digit_to_reg(int digit);

class LedDriver {
public:
    void init();

    void show_addr(uint16_t addr);

    void show_data(uint8_t data);

    void show_status(gpio::status_t status);

    uint16_t keyscan();

private:
    void write(LedDriverReg addr, uint8_t data);

    uint8_t read(LedDriverReg addr);

    // Sets the address to be used for a read operation
	// void set_addr();
};

extern LedDriver LED_DRIVER;