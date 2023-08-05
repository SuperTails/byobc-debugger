#pragma once

#include <stdint.h>
#include <stddef.h>

namespace uart {
    void init();

    void put(uint8_t data);

    void put_bytes(const uint8_t data[], size_t len);
    
    uint8_t get();

    void get_bytes(uint8_t data[], size_t len);

    // Returns -1 if no data is available yet.
    int get_nonblocking();
}