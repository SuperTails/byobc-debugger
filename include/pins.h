#pragma once

#include <Arduino.h>

#define NMIB_PORT       PORTB
#define NMIB_PIN        0
#define NMIB_PIN_MASK   (1 << NMIB_PIN)

#define SYNC_PORT       PORTB
#define SYNC_PIN        1
#define SYNC_PIN_MASK   (1 << SYNC_PIN)

#define RESB_PORT       PORTB
#define RESB_PIN        2
#define RESB_PIN_MASK   (1 << RESB_PIN)

#define PHI2_PORT       PORTB
#define PHI2_PIN        3
#define PHI2_PIN_MASK   (1 << PHI2_PIN)

#define BE_PORT         PORTB
#define BE_PIN          4
#define BE_PIN_MASK     (1 << BE_PIN)

#define RWB_PORT        PORTB
#define RWB_PIN         5
#define RWB_PIN_MASK    (1 << RWB_PIN)

#define WE_PORT         PORTA
#define WE_PIN          0
#define WE_PIN_MASK     (1 << WE_PIN)

#define GPIO1_PORT      PORTA
#define GPIO1_PIN       1
#define GPIO1_PIN_MASK  (1 << GPIO1_PIN)

#define VPB_PORT        PORTA
#define VPB_PIN         4
#define VPB_PIN_MASK    (1 << VPB_PIN)

#define RDY_PORT        PORTA
#define RDY_PIN         5
#define RDY_PIN_MASK    (1 << RDY_PIN)

#define IRQB_PORT       PORTA
#define IRQB_PIN        6
#define IRQB_PIN_MASK   (1 << IRQB_PIN)

#define MLB_PORT        PORTA
#define MLB_PIN         7
#define MLB_PIN_MASK    (1 << MLB_PIN_MASK)