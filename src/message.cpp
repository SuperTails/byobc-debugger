#include "message.h"
#include "uart.h"
#include "crc.h"

int get_command(Command &cmd) {
	int msg_type = uart::get_nonblocking();
	if (msg_type == -1) {
		return ERR_NO_CMD;
	}

	if (msg_type == 0 || msg_type >= static_cast<int>(CommandType::MAX_CMD)) {
		return ERR_BAD_CMD;
	}

	auto cmd_type = static_cast<CommandType>(msg_type);
	cmd.ty = cmd_type;

	// Echo back the message type to tell the host to send the rest of the command
	uart::put(msg_type);

	switch (cmd_type) {
	case CommandType::Ping:
		break;
	case CommandType::WriteEEPROM: {
		uart::get_bytes(reinterpret_cast<uint8_t*>(&cmd.write_eeprom), sizeof(WriteEEPROMCmd));
		uint16_t rx_crc = cmd.write_eeprom.checksum;
		uint16_t calc_crc = update_crc(0, reinterpret_cast<uint8_t*>(&cmd.write_eeprom), 2 + 64);
		if (rx_crc != calc_crc) {
			uart::put(ERR_BAD_CHECKSUM);
			return ERR_BAD_CHECKSUM;
		}
		if (cmd.write_eeprom.addr & ((1 << 6) - 1)) {
			uart::put(ERR_BAD_ADDR);
			return ERR_BAD_ADDR;
		}

		uart::put(0);
		break;
	}
	case CommandType::ReadMemory: {
		uart::get_bytes(reinterpret_cast<uint8_t*>(&cmd.read_memory), 2 * sizeof(uint16_t));
		break;
	}
	case CommandType::SetBreakpoint: {
		uart::get_bytes(reinterpret_cast<uint8_t*>(&cmd.set_breakpoint), sizeof(SetBreakpointCmd));
		break;
	}
	case CommandType::SectorErase: {
		uart::get_bytes(reinterpret_cast<uint8_t*>(&cmd.sector_erase), sizeof(SectorEraseCmd));
		break;
	}
	case CommandType::ResetCpu:
		break;
	case CommandType::GetBusState:
		break;
	case CommandType::Step:
	case CommandType::StepHalfCycle:
	case CommandType::StepCycle:
	case CommandType::Continue:
	case CommandType::PrintInfo:
	case CommandType::GetCpuState:
	case CommandType::DebuggerReset:
		break;
	case CommandType::MAX_CMD:
		break;
	}

	return 0;
}

void hit_breakpoint(uint8_t which) {
	uart::put((uint8_t)CommandType::HitBreakpoint);
	uart::put(which);
}