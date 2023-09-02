#include "pins.h"
#include "led.h"
#include "i2c.h"
#include "uart.h"
#include "message.h"
#include "gpio.h"
#include "version.h"

#include <Arduino.h>

LedDriver LED_DRIVER;

using gpio::status_t;

/*int main() {
  DecodedInstr a = { BaseOp::Lda, AddrMode::AbsX, { 0x42, 0x69 }, 0 };
  char buf[MAX_INSTR_DISPLAY_SIZE+1];
  a.format(buf);
  std::cout << buf << "\n";
}*/

void delay_loop(int amount) {
  volatile int i = 0;
  for (i = 0; i < amount; ++i);
}

void delay_millis(int millis) {
  delay_loop(millis * 12);
}

// Address and data buses must be in output mode.
// Page must be aligned to a 128-byte boundary (mask 0xFF80).
void program_eeprom_page(uint16_t page, const uint8_t data[EEPROM_PAGE_SIZE]) {
  page &= ~EEPROM_PAGE_MASK;

  gpio::write_gpio1(false); // disable EEPROM output 

  gpio::write_addr_bus(0x5555 | 0x8000);
  WE_PORT.OUTCLR = WE_PIN_MASK;
  gpio::write_data_bus(0xAA);
  gpio::write_addr_bus(0x2AAA | 0x8000);
  WE_PORT.OUTSET = WE_PIN_MASK;
  asm volatile ("nop");
  WE_PORT.OUTCLR = WE_PIN_MASK;
  gpio::write_data_bus(0x55);
  gpio::write_addr_bus(0x5555 | 0x8000);
  WE_PORT.OUTSET = WE_PIN_MASK;
  asm volatile ("nop");
  WE_PORT.OUTCLR = WE_PIN_MASK;
  gpio::write_data_bus(0xA0);
  gpio::write_addr_bus(page | 0x8000);
  WE_PORT.OUTSET = WE_PIN_MASK;
  asm volatile ("nop");

  for (int i = 0; i < EEPROM_PAGE_SIZE; ++i) {
    WE_PORT.OUTCLR = WE_PIN_MASK;

    uint16_t addr = page | i;

    delay_loop(10); // address hold time
    gpio::write_addr_bus((addr + 1) | 0x8000); // in preparation for next falling edge
    gpio::write_data_bus(data[i]); // latched on the rising edge

    WE_PORT.OUTSET = WE_PIN_MASK;
  }

  gpio::write_gpio1(true); // re-enable EEPROM output
  
  delay_loop(50000);
}

#define USED_PINS (8 + 8 + 4 + 4 + 6)

int8_t test_bridged_pins() {
  PORT_t* BASE_PORTS[5] = { &PORTA, &PORTD, &PORTE, &PORTF, &PORTB };
  int8_t NUM_BITS[5] = { 8, 8, 4, 4, 6 };

  PORT_t* PORTS[USED_PINS];
  int8_t  BITS[USED_PINS];

  int8_t port_idx = 0;
  int8_t bit = 0;
  for (int8_t i = 0; i < USED_PINS; ++i) {
    PORTS[i] = BASE_PORTS[port_idx];
    BITS[i] = bit;

    ++bit;
    if (bit == NUM_BITS[port_idx]) {
      ++port_idx;
      bit = 0;
    }
  }

  // Ensure all pins have pullups
  for (int8_t i = 0; i < USED_PINS; ++i) {
    *(register8_t*)(((intptr_t)&PORTS[i]->PIN0CTRL) + BITS[i]) |= 0b1000;
  }
  
  int8_t bad_pin = -1;

  for (int8_t i = 0; i < USED_PINS; ++i) {
    // First, ensure all other pins are outputs driving low 
    PORTA.DIRSET = 0xFF;
    PORTD.DIRSET = 0xFF;
    PORTE.DIRSET = 0x0F;
    PORTF.DIRSET = 0x0F;
    PORTB.DIRSET = 0x1F;

    PORTA.OUTCLR = 0xFF;
    PORTD.OUTCLR = 0xFF;
    PORTE.OUTCLR = 0x0F;
    PORTF.OUTCLR = 0x0F;
    PORTB.OUTCLR = 0xFF;
    
    // Now set exactly one pin to an input with a pullup
    PORTS[i]->DIRCLR = (1 << BITS[i]);

    // Wait for the voltage to stabilize...
    delay_loop(10000);

    // Make sure the pin isn't being pulled low by being bridged to another pin
    bool ok = (PORTS[i]->IN >> BITS[i]) & 0x1;

    if (!ok) {
      bad_pin = i;
      break;
    }
  }

  // Set all pins to be inputs again
  PORTA.DIRCLR = 0xFF;
  PORTD.DIRCLR = 0xFF;
  PORTE.DIRCLR = 0xFF;
  PORTB.DIRCLR = 0x1F;

  // Disable pullups for all pins again
  for (int8_t i = 0; i < USED_PINS; ++i) {
    *(register8_t*)(((intptr_t)&PORTS[i]->PIN0CTRL) + BITS[i]) |= 0b1000;
  }

  return bad_pin;

  // PORTA pins (all)
  // PORTD pins (all)
  // PORTE pins (all)
  // PORTB pins 0..=5
}

void setup_pin_directions() {
  NMIB_PORT.OUTSET = NMIB_PIN_MASK;
  RESB_PORT.OUTCLR = RESB_PIN_MASK;
  PHI2_PORT.OUTCLR = PHI2_PIN_MASK;
  BE_PORT.OUTSET = BE_PIN_MASK;
  WE_PORT.OUTSET = WE_PIN_MASK;

  NMIB_PORT.DIRSET = NMIB_PIN_MASK;
  RESB_PORT.DIRSET = RESB_PIN_MASK;
  PHI2_PORT.DIRSET = PHI2_PIN_MASK;
  BE_PORT.DIRSET = BE_PIN_MASK;
  WE_PORT.DIRSET = WE_PIN_MASK;

  // Enable pull-up on RDY
  RDY_PORT.PIN5CTRL |= 0b1000;

  IRQB_PORT.PIN6CTRL |= 0b1000;

  PORTA.PIN2CTRL |= 0b1000;
  PORTA.PIN3CTRL |= 0b1000;
}

#define TEST_PINS false

void setup() {
  // Default is CLK_MAIN / 6
  // New is CLK_MAIN / 16

  /*uint8_t value = (CLKCTRL.MCLKCTRLB & ~0x1F) | 0xB;
  CCP = 0xD8;
  CLKCTRL.MCLKCTRLB = value;*/

  setup_pin_directions();

  gpio::set_data_bus_dir(gpio::Direction::Output);
  gpio::write_data_bus(0xEA);

  uart::init();

  if (TEST_PINS) {
    uint8_t result = test_bridged_pins();

    uart::put(result);

    while (1) { asm volatile ("nop"); }
  }

  i2c::init();

  LED_DRIVER.init();

  LED_DRIVER.show_status({ 0xFFFF });

  /*while (1) {
    LED_DRIVER.show_data(FUSE.SYSCFG0);
  }*/

  /*while (1) {
    LED_DRIVER.show_addr(0x1234);

    volatile uint32_t k = 0;
    for (k = 0; k < 500000; ++k);

    LED_DRIVER.show_addr(0x4321);

    for (k = 0; k < 500000; ++k);
  }*/

  /*while (1) {
    uint16_t keys = LED_DRIVER.keyscan();
    LED_DRIVER.show_addr(keys);
  }*/

  gpio::set_gpio1_dir(gpio::Direction::Output);

  gpio::write_gpio1(true); // enable the EEPROM outputs
  gpio::write_we(true);

  //uart::put_bytes("Hello, world!\n", 14);

  gpio::write_be(true); // enable the 6502 buses

  gpio::set_addr_bus_dir(gpio::Direction::Input);
  gpio::set_data_bus_dir(gpio::Direction::Input);

  /*while (1) {
    gpio::set_addr_bus_dir(gpio::Direction::Output);
    gpio::set_data_bus_dir(gpio::Direction::Input);
    gpio::write_gpio1(false);
    for (int i = 0xC0; i < 256; ++i) {
      uint16_t addr = 0xFF00 | i;

      gpio::write_addr_bus(addr);

      LED_DRIVER.show_addr(addr);

      uint8_t data = gpio::read_data_bus();

      LED_DRIVER.show_data(data);

      delay_loop(1000);
    }
  }*/

  #if 0

  uint16_t i = 0;
  while (1) {
    //uart::put_bytes("Hello, world!\n", 14);
    //continue;

    uart::put(0x55);

    LED_DRIVER.show_addr(i++);
  }

  #endif

  RESB_PORT.OUTCLR = RESB_PIN_MASK;
}

#define MAX_BREAKPOINTS 16

struct Breakpoint {
  uint16_t addr;
  bool enabled;
};

static Breakpoint BREAKPOINTS[MAX_BREAKPOINTS];
static int8_t NUM_BREAKPOINTS = 0;

#define STEP

int update_cnt = 0;

enum class Action {
  None,
  StepHalfCycle,
  StepCycle,
  Step,
  Continue,
};

enum class Wait {
  None,
  HalfCycle,
  Cycle,
  Sync
};

static Wait WAIT = Wait::HalfCycle;

Action handle_commands() {
  Command cmd;
  while (get_command(cmd) == 0) {
    switch (cmd.ty) {
    case CommandType::Ping:
      uart::put_bytes("Pong!", 5);
      break;
    case CommandType::WriteEEPROM:
      gpio::set_addr_bus_mode(gpio::AddressBusMode::DebuggerDriven);
      gpio::set_data_bus_dir(gpio::Direction::Output);

      program_eeprom_page(cmd.write_eeprom.addr, cmd.write_eeprom.data);

      gpio::set_addr_bus_dir(gpio::Direction::Input);
      gpio::set_addr_bus_mode(gpio::AddressBusMode::CpuDriven);
      break;
    case CommandType::ReadMemory:
      gpio::write_we(true);
      gpio::set_addr_bus_mode(gpio::AddressBusMode::DebuggerDriven);
      gpio::set_data_bus_dir(gpio::Direction::Input);

      for (uint16_t i = 0; i < cmd.read_memory.len; ++i) {
        uint16_t addr = i + cmd.read_memory.addr;

        gpio::write_addr_bus(addr);
        delay_loop(1);
        uint8_t data = gpio::read_data_bus();
        LED_DRIVER.show_data(data);
        uart::put(data);
      }

      gpio::set_data_bus_dir(gpio::Direction::Input);
      gpio::set_addr_bus_mode(gpio::AddressBusMode::CpuDriven);
      break;
    case CommandType::SetBreakpoint:
      if (NUM_BREAKPOINTS == MAX_BREAKPOINTS) {
        uart::put(0xFF);
        break;
      }

      uart::put(NUM_BREAKPOINTS);
      BREAKPOINTS[NUM_BREAKPOINTS].addr = cmd.set_breakpoint.addr;
      ++NUM_BREAKPOINTS;
      break;
    case CommandType::ResetCpu:
      RESB_PORT.OUTCLR = RESB_PIN_MASK;
      WAIT = Wait::HalfCycle;
      break;
    case CommandType::GetBusState: {
      BusState state = {
        gpio::read_addr_bus(),
        gpio::read_status(),
        gpio::read_data_bus(),
        0
      };

      uart::put_bytes(reinterpret_cast<uint8_t*>(&state), sizeof(BusState));
      break;
    }
    case CommandType::StepHalfCycle: {
      return Action::StepHalfCycle;
    }
    case CommandType::StepCycle: {
      return Action::StepCycle;
    }
    case CommandType::Step: {
      return Action::Step;
    }
    case CommandType::Continue: {
      return Action::Continue;
    }
    case CommandType::PrintInfo: {
      uart::put_bytes(reinterpret_cast<const uint8_t*>(&VERSION), sizeof(VERSION));
      break;
    }
    default:
      break;
    }
  }

  return Action::None;
}

void loop() {
  uint16_t addr;
  uint8_t data;
  status_t status;

  // Interrupt signals (IRQB, NMIB, RESB, RDY) are latched on the falling edge.

  gpio::write_phi2(false);

  /* ---- PHI2 is low ---- */

  delay_loop(50);

  RESB_PORT.OUTSET = RESB_PIN_MASK;

  addr = gpio::read_addr_bus();
  for (int8_t i = 0; i < NUM_BREAKPOINTS; ++i) {
    if (BREAKPOINTS[i].addr == addr) {
      WAIT = Wait::HalfCycle;
      hit_breakpoint(i);
      break;
    }
  }

  if (WAIT != Wait::None) {
    data = gpio::read_data_bus();
    status = gpio::read_status();
    LED_DRIVER.show_addr(addr);
    LED_DRIVER.show_data(data);
    LED_DRIVER.show_status(status);
  }

  uint16_t keyscan;
  Action action;
  do {
    action = handle_commands();
    if (action == Action::Continue) {
      WAIT = Wait::None;
      break;
    } else if (action == Action::StepHalfCycle) {
      WAIT = Wait::HalfCycle;
      break;
    } else if (action == Action::StepCycle) {
      WAIT = Wait::Cycle;
      break;
    } else if (action == Action::Step) {
      WAIT = Wait::Sync;
      break;
    }
  } while (WAIT == Wait::HalfCycle);

  delay_loop(50);

  gpio::write_phi2(true);

  /* ---- PHI2 is high ---- */

  delay_loop(50);

  if (WAIT != Wait::None) {
    addr = gpio::read_addr_bus();
    data = gpio::read_data_bus();
    status = gpio::read_status();
    LED_DRIVER.show_addr(addr);
    LED_DRIVER.show_data(data);
    LED_DRIVER.show_status(status);
  }

  do {
    status = gpio::read_status();

    action = handle_commands();
    if (action == Action::Continue) {
      WAIT = Wait::None;
      break;
    } else if (action == Action::StepHalfCycle) {
      WAIT = Wait::HalfCycle;
      break;
    } else if (action == Action::StepCycle) {
      WAIT = Wait::Cycle;
      break;
    } else if (action == Action::Step) {
      WAIT = Wait::Sync;
      break;
    }
  } while (WAIT == Wait::HalfCycle || WAIT == Wait::Cycle || (!status.sync() && WAIT == Wait::Sync));

  delay_loop(50);
}