#pragma once

#include <stdint.h>
#include <stddef.h>

uint16_t update_crc(uint16_t crc_accum, uint8_t data[], size_t len);