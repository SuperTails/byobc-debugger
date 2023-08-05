from serial import Serial
from dataclasses import dataclass
from typing import Optional
from enum import Enum
import time
from disasm import AddrMode
import disasm

def update_crc(crc_accum: int, data: bytes) -> int:
    crc_table = [
        0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
        0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
        0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
        0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
        0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
        0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
        0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
        0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
        0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
        0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
        0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
        0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
        0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
        0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
        0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
        0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
        0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
        0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
        0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
        0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
        0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
        0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
        0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
        0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
        0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
        0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
        0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
        0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
        0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
        0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
        0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
        0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
    ]

    for b in data:
        i = ((crc_accum >> 8) ^ b) & 0xFF;
        i %= 256
        crc_accum = ((crc_accum << 8) & 0xFF00) ^ crc_table[i];

    return crc_accum

CMD_PING = b'\x01'
CMD_WRITE_EEPROM = b'\x02'
CMD_READ_MEMORY = b'\x03'
CMD_SET_BREAKPOINT = b'\x04'
CMD_RESET_CPU = b'\x05'
CMD_GET_BUS_STATE = b'\x06'
CMD_STEP = b'\x07'
CMD_STEP_CYCLE = b'\x08'
CMD_STEP_HALF_CYCLE = b'\x09'
CMD_CONTINUE = b'\x0A'

@dataclass
class BusState:
    addr: int
    data: int

    mlb:   bool
    irqb:  bool
    rdy:   bool
    vpb:   bool
    gpio1: bool
    we:    bool
    rwb:   bool
    be:    bool
    phi2:  bool
    resb:  bool
    sync:  bool
    nmib:  bool

    @classmethod
    def from_bytes(cls, b: bytes):
        assert(len(b) == 6)

        addr = int.from_bytes(b[0:2], 'little')
        status = int.from_bytes(b[2:4], 'little')
        data = int.from_bytes(b[4:5], 'little')

        return cls(
            addr=addr,
            data=data,

            mlb  =(status & 0x8000) != 0,
            irqb =(status & 0x4000) != 0,
            rdy  =(status & 0x2000) != 0,
            vpb  =(status & 0x1000) != 0,
            gpio1=(status & 0x0200) != 0,
            we   =(status & 0x0100) != 0,
            rwb  =(status & 0x0020) != 0,
            be   =(status & 0x0010) != 0,
            phi2 =(status & 0x0008) != 0,
            resb =(status & 0x0004) != 0,
            sync =(status & 0x0002) != 0,
            nmib =(status & 0x0001) != 0,
        )

class InvalidHeader(Exception):
    pass

class Debugger:
    def __init__(self, port):
        self.port = port
        self.cpu = Cpu()
        self.cpu.next_state = Cpu()
    
    def start_command(self, b: bytes):
        assert(len(b) == 1)
        self.port.write(b)
        echo = self.port.read(1)
        if echo != b:
            raise InvalidHeader(f'expected {b} but found {echo}')
    
    @classmethod
    def open_default_port(cls):
        port = Serial('COM5', 115200, timeout=5.0)
        return cls(port)

    def ping(self) -> bytes:
        self.start_command(CMD_PING)
        data = self.port.read(5)
        return data

    def page_write(self, page: int, data: bytes):
        self._raw_page_write(page, data)
        time.sleep(0.015)
        written = self.read(page, len(data))
        if data != written:
            print(f'wrote data:    {data}')
            print(f'but read back: {written}')
            raise Exception("data mismatch")

    def _raw_page_write(self, page: int, data: bytes):
        assert(page & ~0xFF80 == 0)
        assert(len(data) == 128)

        self.start_command(CMD_WRITE_EEPROM)

        data = page.to_bytes(2, 'little') + data
        checksum = update_crc(0, data)
        data = data + checksum.to_bytes(2, 'little')

        self.port.write(data)

        confirm = self.port.read(1)
        if confirm != b'\x00':
            raise Exception(confirm)
    
    def read(self, addr: int, len_bytes: int):
        assert(len_bytes < 0x1_0000)

        self.start_command(CMD_READ_MEMORY)

        cmd = addr.to_bytes(2, 'little') + len_bytes.to_bytes(2, 'little')
        self.port.write(cmd)

        data = self.port.read(len_bytes)
        assert(len(data) == len_bytes)
        return data
    
    def set_breakpoint(self, addr: int):
        assert(addr <= 0xFFFF)

        self.start_command(CMD_SET_BREAKPOINT)

        self.port.write(addr.to_bytes(2, 'little'))
        ok = self.port.read(1)
        if ok == b'\xFF':
            raise Exception('Too many breakpoints')
        print(f'Set breakpoint {ok} at address ${addr:04X}')

    def reset_cpu(self):
        self.start_command(CMD_RESET_CPU)
        self.cpu.reset()
    
    def get_bus_state(self) -> BusState:
        self.start_command(CMD_GET_BUS_STATE)

        state = self.port.read(6)
        return BusState.from_bytes(state)
    
    def step(self):
        while True:
            self.start_command(CMD_STEP_CYCLE)
            state = self.get_bus_state()
            self.cpu.update(state)
            if state.sync:
                break
    
    def step_half_cycle(self):
        self.start_command(CMD_STEP_HALF_CYCLE)
        state = self.get_bus_state()
        self.cpu.update(state)
    
    def step_cycle(self):
        self.start_command(CMD_STEP_CYCLE)
        state = self.get_bus_state()
        self.cpu.update(state)
    
    def cont(self):
        self.start_command(CMD_CONTINUE)
        self.cpu.reset()

class AddrConstraint(Enum):
    Pc = 0

class DataConstraint(Enum):
    A = 0
    X = 1
    Y = 2
    
class Phase(Enum):
    Fetch = 0
    Load = 1
    Execute = 2

@dataclass
class Cpu:
    pc: Optional[int]

    a: Optional[int]
    x: Optional[int]
    y: Optional[int]

    ir: Optional[int]

    time: Optional[int]

    phase: Optional[Phase]

    next_state: Optional['Cpu']

    sp: Optional[int]

    def __init__(self, nested=True):
        self.reset(nested)

    def reset(self, nested=True):
        self.pc = None
        self.a = None
        self.x = None
        self.y = None
        self.ir = None
        self.time = None
        self.phase = None
        self.sp = None
        if nested:
            self.next_state = Cpu(nested=False)
    
    def bus_state_constraints(self):
        if self.phase == Phase.Fetch:
            return AddrConstraint.Pc, None

        if self.ir is None:
            return None, None

        if self.phase == Phase.Load:
            am = self.addr_mode()
            match am:
                case 'zpg':
                    return AddrConstraint.Pc, None
                case 'abs':
                    return AddrConstraint.Pc, None
        
        if self.phase == Phase.Execute:
            c = self.opcode()
            match c:
                case 'STA' | 'LDA':
                    return None, DataConstraint.A
                case 'STX' | 'LDX':
                    return None, DataConstraint.X
                case 'STY' | 'LDY':
                    return None, DataConstraint.Y

        
        return None, None
    
    def apply_constraints(self, bus_state: BusState):
        addr_cons, data_cons = self.bus_state_constraints()
        match addr_cons:
            case AddrConstraint.Pc:
                self.pc = bus_state.addr
        
        match data_cons:
            case DataConstraint.A:
                self.a = bus_state.data
            case DataConstraint.X:
                self.x = bus_state.data
            case DataConstraint.Y:
                self.y = bus_state.data
    
    def opcode(self):
        if self.ir is None:
            return None
        c, _ = disasm.get_modes(self.ir)
        return c
    
    def addr_mode(self):
        if self.ir is None:
            return None
        _, m = disasm.get_modes(self.ir)
        return m
    
    def update_flags_nz(self, value: Optional[int]):
        # TODO:
        if value is not None:
            # TODO:
            pass
        else:
            self.unknown_flags()

    def unknown_flags(self):
        # TODO:
        pass

    def update(self, state: BusState):
        assert(self.next_state is not None)

        self.pc = self.next_state.pc
        self.ir = self.next_state.ir
        self.time = self.next_state.time
        self.phase = self.next_state.phase
        self.time = self.next_state.time

        if state.sync:
            self.phase = Phase.Fetch

        self.apply_constraints(state)
        
        if self.phase == Phase.Fetch:
            assert(self.pc is not None)

            self.next_state.pc = self.pc + 1
            self.next_state.ir = state.data
            self.next_state.phase = Phase.Load
            self.next_state.time = 0

            pair = disasm.get_modes(state.data)
            if pair is not None and pair[1] in ('#', 'impl'):
                self.next_state.phase = Phase.Execute

        elif self.phase == Phase.Load:
            self.next_state.time = self.time + 1

            am = self.addr_mode()
            match am:
                case 'zpg':
                    self.next_state.phase = Phase.Execute
                    self.next_state.time = 0
                case '#':
                    raise Exception('immediate does not have a load phase')
                case 'abs':
                    if self.time == 1:
                        self.next_state.phase = Phase.Execute
                        self.next_state.time = 0
        
        elif self.phase == Phase.Execute:
            self.next_state.phase = Phase.Execute

            c = self.opcode()
            match c:
                case 'TXS':
                    self.sp = self.x
                case 'TAX':
                    self.x = self.a
                case 'TXA':
                    self.a = self.x
                case 'TAY':
                    self.y = self.a
                case 'TYA':
                    self.a = self.y
                case 'NOP':
                    pass
                case 'DEX':
                    if self.x is not None:
                        self.x = (self.x - 1) & 0xFF
                    self.update_flags_nz(self.x)
                case 'INX':
                    if self.x is not None:
                        self.x = (self.x + 1) & 0xFF
                    self.update_flags_nz(self.x)
                case 'DEY':
                    if self.y is not None:
                        self.y = (self.y - 1) & 0xFF
                    self.update_flags_nz(self.y)
                case 'INY':
                    if self.y is not None:
                        self.y = (self.y + 1) & 0xFF
                    self.update_flags_nz(self.y)


    def display(self):
        result = 'PC: '
        if self.pc is not None:
            result += f'{self.pc:04X}'
        else:
            result += '????'
        
        result += '  A: '
        if self.a is not None:
            result += f'{self.a:02X}'
        else:
            result += '??'
        
        result += '  X: '
        if self.x is not None:
            result += f'{self.x:02X}'
        else:
            result += '??'
        
        result += '  Y: '
        if self.y is not None:
            result += f'{self.y:02X}'
        else:
            result += '??'
        
        result += '  IR: '
        if self.ir is not None:
            result += f'{self.ir:02X}'
            pair = disasm.get_modes(self.ir)
            if pair is not None:
                result += f' ({pair[0]} {pair[1]})'
        else:
            result += '??'
        
        result += f' {self.phase} {self.time} {self.next_state.phase}'

        return result

def main():
    dbg = Debugger.open_default_port()
    print(dbg.ping())

    while True:
        state = dbg.get_bus_state()
        print(dbg.cpu.display())
        print(f'ADDR: {state.addr:04X}')
        print(f'DATA:   {state.data:02X}')
        print(f'STATUS: {state.sync} (lots of pins)')

        cmd = input('> ')

        if cmd in ('s', 'step'):
            dbg.step()
        elif cmd in ('h', 'stephalf'):
            dbg.step_half_cycle()
        elif cmd in ('y', 'stepcycle'):
            dbg.step_cycle()
        elif cmd in ('c', 'continue'):
            dbg.cont()
        elif cmd in ('q'):
            break
        elif cmd in ('r'):
            dbg.reset_cpu()
        else:
            print(f'Unknown command {repr(cmd)}')
    
    print('Debugger exited')

if __name__ == '__main__':
    main()