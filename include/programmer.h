#pragma once

#include <stdint.h>

#define FLASH_SECTOR_BITS 12
#define FLASH_SECTOR_SIZE (1 << FLASH_SECTOR_BITS)
#define FLASH_SECTOR_MASK (EEPROM_PAGE_SIZE - 1)

namespace programmer {
    // Must be aligned to FLASH_SECTOR_SIZE.
    void sector_erase(uint16_t sector);

    void byte_program(uint16_t addr, uint8_t value);
}