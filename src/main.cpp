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

void delay(int amount) {
  volatile int i = 0;
  for (i = 0; i < amount; ++i);
}

void delay_millis(int millis) {
  delay(millis * 12);
}

// Address and data buses must be in output mode.
// Page must be aligned to a 128-byte boundary (mask 0xFF80).
void program_eeprom_page(uint16_t page, const uint8_t data[128]) {
  page &= 0xFF80;

  gpio::write_gpio1(false); // disable EEPROM output 

  gpio::write_addr_bus(0x5555);
  WE_PORT.OUTCLR = WE_PIN_MASK;
  gpio::write_data_bus(0xAA);
  gpio::write_addr_bus(0x2AAA);
  WE_PORT.OUTSET = WE_PIN_MASK;
  asm volatile ("nop");
  WE_PORT.OUTCLR = WE_PIN_MASK;
  gpio::write_data_bus(0x55);
  gpio::write_addr_bus(0x5555);
  WE_PORT.OUTSET = WE_PIN_MASK;
  asm volatile ("nop");
  WE_PORT.OUTCLR = WE_PIN_MASK;
  gpio::write_data_bus(0xA0);
  gpio::write_addr_bus(page);
  WE_PORT.OUTSET = WE_PIN_MASK;
  asm volatile ("nop");

  for (int i = 0; i < 128; ++i) {
    WE_PORT.OUTCLR = WE_PIN_MASK;

    uint16_t addr = page | i;

    delay(10); // address hold time
    gpio::write_addr_bus(addr + 1); // in preparation for next falling edge
    gpio::write_data_bus(data[i]); // latched on the rising edge

    WE_PORT.OUTSET = WE_PIN_MASK;
  }

  gpio::write_gpio1(true); // re-enable EEPROM output
  
  volatile uint16_t k = 0;
  for (k = 0; k < 50000; ++k);
}

void setup() {
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

  // Default is CLK_MAIN / 6
  // New is CLK_MAIN / 16

  /*uint8_t value = (CLKCTRL.MCLKCTRLB & ~0x1F) | 0xB;
  CCP = 0xD8;
  CLKCTRL.MCLKCTRLB = value;*/

  // Enable pull-up on RDY
  RDY_PORT.PIN5CTRL |= 0b1000;

  IRQB_PORT.PIN6CTRL |= 0b1000;

  PORTA.PIN2CTRL |= 0b1000;
  PORTA.PIN3CTRL |= 0b1000;


  gpio::set_data_bus_dir(gpio::Direction::Output);
  gpio::write_data_bus(0xEA);

  uart::init();

  /*while (1) {
    uart::put_bytes("Hello, world!\n", strlen("Hello, world!\n"));
    delay(1000);
  }*/

  /*while (1) {
    uart::put_bytes("Hello, world!\n", 14);

    gpio::write_data_bus(~0xEA);

    volatile long long i;
    for (i = 0; i < 100000; ++i);

    uart::put_bytes("Hello, world!\n", 14);

    gpio::write_data_bus(0xEA);

    for (i = 0; i < 100000; ++i);
  }*/

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

      delay(1000);
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
        delay(1);
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

  delay(50);

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

  delay(50);

  gpio::write_phi2(true);

  /* ---- PHI2 is high ---- */

  delay(50);

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

  delay(50);
}