import sys
from serial import Serial
from dataclasses import dataclass
from typing import Optional
from enum import Enum
import time
from disasm import AddrMode
import disasm
import dasm
import copy

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
CMD_HIT_BREAKPOINT = b'\x0B'
CMD_PRINT_INFO = b'\x0C'

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

class PortWrapper:
    def __init__(self, port: Serial, timeout: float):
        self.port = port
        self.timeout = timeout

        assert(self.port.timeout == 0.0)

    def read_nonblocking(self, count: int):
        return self.port.read(count)
    
    def read(self, count: int):
        assert(count >= 0)

        result = b''

        start_time = time.monotonic()
        while time.monotonic() - start_time < self.timeout and len(result) != count:
            result += self.read_nonblocking(count)

        return result
    
    def read_exact(self, count: int):
        result = self.read(count)
        if len(result) != count:
            raise TimeoutError
        return result
    
    def write(self, data: bytes):
        self.port.write(data)

@dataclass
class VersionInfo:
    year: int
    month: int
    day: int

    def __str__(self):
        return f'{self.year:04}-{self.month:02}-{self.day:02}'

class Debugger:
    def __init__(self, port, timeout):
        self.port = PortWrapper(port, timeout)
        self.cpu = Cpu()
        self.cpu.next_state = Cpu()
    
    def start_command(self, b: bytes):
        assert(len(b) == 1)
        self.port.write(b)
        echo = self.port.read_exact(1)
        if echo != b:
            raise InvalidHeader(f'expected {b} but found {echo}')
    
    @classmethod
    def open(cls, path):
        port = Serial(path, 115200, timeout=0.0)
        return cls(port, 5.0)

    def ping(self) -> bytes:
        self.start_command(CMD_PING)
        data = self.port.read_exact(5)
        return data
    
    def print_info(self) -> VersionInfo:
        self.start_command(CMD_PRINT_INFO)

        data = self.port.read_exact(4)
        year  = int.from_bytes(data[0:2], 'little')
        month = int.from_bytes(data[2:3], 'little')
        day   = int.from_bytes(data[3:4], 'little')
        return VersionInfo(year, month, day)

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

        confirm = self.port.read_exact(1)
        if confirm != b'\x00':
            raise Exception(confirm)
    
    def read(self, addr: int, len_bytes: int):
        assert(len_bytes < 0x1_0000)

        self.start_command(CMD_READ_MEMORY)

        cmd = addr.to_bytes(2, 'little') + len_bytes.to_bytes(2, 'little')
        self.port.write(cmd)

        data = self.port.read_exact(len_bytes)
        assert(len(data) == len_bytes)
        return data
    
    def set_breakpoint(self, addr: int):
        assert(addr <= 0xFFFF)

        self.start_command(CMD_SET_BREAKPOINT)

        self.port.write(addr.to_bytes(2, 'little'))
        ok = self.port.read_exact(1)
        if ok == b'\xFF':
            raise Exception('Too many breakpoints')
        print(f'Set breakpoint {ok} at address ${addr:04X}')

    def reset_cpu(self):
        self.start_command(CMD_RESET_CPU)
        self.cpu.reset()
    
    def get_bus_state(self) -> BusState:
        self.start_command(CMD_GET_BUS_STATE)

        state = self.port.read_exact(6)
        return BusState.from_bytes(state)
    
    def poll_breakpoint(self):
        if self.port.read_nonblocking(1) == CMD_HIT_BREAKPOINT:
            which = self.port.read_exact(1)
            return which[0]
        else:
            return None
    
    def step(self):
        while True:
            self.start_command(CMD_STEP_CYCLE)
            state = self.get_bus_state()
            print(state)
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

def are_different_or_none(lhs, rhs):
    if lhs is None:
        return False
    if rhs is None:
        return False
    return lhs != rhs

def byte_to_signed(b: int):
    return int.from_bytes(b.to_bytes(1), 'little', signed=True)

@dataclass
class Cpu:
    pc: Optional[int]

    a: Optional[int]
    x: Optional[int]
    y: Optional[int]
    s: Optional[int]

    p: int
    p_mask: int

    ir: int
    time: int
    phase: Phase

    cycle_known: bool

    next_state: Optional['Cpu']

    def __init__(self, nested=True):
        self.reset(nested)
    
    def do_fetch(self, addr: int, data: int):
        next_state = Cpu(nested=False)

        next_state.pc = (addr + 1) & 0xFFFF

        next_state.a = self.a
        next_state.x = self.x
        next_state.y = self.y
        next_state.s = self.s
        next_state.p = self.p
        next_state.p_mask = self.p_mask

        next_state.ir = data
        next_state.time = 0
        next_state.phase = Phase.Load
        if next_state.addr_mode in ('#', 'impl'):
            next_state.phase = Phase.Execute
        next_state.cycle_known = True

        return next_state
    
    def do_load(self):
        assert(self.cycle_known)

        next_state = Cpu(nested=False)

        next_state.a = self.a
        next_state.x = self.x
        next_state.y = self.y
        next_state.s = self.s
        next_state.p = self.p
        next_state.p_mask = self.p_mask

        next_state.ir = self.ir
        next_state.time = self.time + 1
        next_state.phase = Phase.Execute
        next_state.cycle_known = True

        done = False

        STORE_OR_RMW = (
            'STA', 'STX', 'STY',
            'ASL', 'DEC', 'INC', 'LSR', 'ROL', 'ROR',
        )

        op = self.opcode()
        am = self.addr_mode()
        match am:
            case 'zpg':
                if self.time == 0:
                    done = True
                    next_state.pc = self.pc_plus(1)
            case 'abs':
                if self.time == 1:
                    done = True
                    next_state.pc = self.pc_plus(2)
            case 'ind,X':
                if self.time == 0:
                    # Access base zpg address BAL
                    pass
                elif self.time == 2:
                    # Access ADL at BAL + X
                    pass
                elif self.time == 3:
                    # Access ADH at BAL + X (+ 1)
                    done = True
                    next_state.pc = self.pc_plus(1)
            case 'abs,X' | 'abs,Y':

                if self.time == 0:
                    # Access BAL
                    pass
                elif self.time == 1 and self.opcode not in STORE_OR_RMW:
                    # Access BAH
                    print('TODO: ABS,X OR ABS,Y ADDRESSING MODE COULD END HERE')
                elif self.time == 2:
                    # Compute carry
                    done = True
                    next_state.pc = self.pc_plus(2)
            case 'ind,Y':
                if self.time == 0:
                    # Read IAL
                    pass
                elif self.time == 1:
                    # Access BAL, at IAL
                    pass
                elif self.time == 2 and self.opcode not in STORE_OR_RMW:
                    # Access BAH, at IAL + 1
                    print('TODO: IND,Y ADDRESSING MODE COULD END HERE')
                elif self.time == 3:
                    # Compute carry
                    done = True
                    next_state.pc = self.pc_plus(1)
            case 'zpg,X' | 'zpg,Y':
                if self.time == 0:
                    # Read BAL
                    pass
                elif self.time == 1:
                    # Access BAL (discarded) 
                    done = True
                    next_state.pc = self.pc_plus(1)
            case '#' | 'impl':
                raise Exception('immediate or implied mode does not have a load phase')
            case _:
                print(f'unknown addressing mode {am}')
    
        if done:
            next_state.phase = Phase.Execute
            next_state.time = 0
        
        return next_state
    
    def do_execute(self, addr: int, data: int):
        assert(self.cycle_known)

        next_state = Cpu(nested=False)

        next_state.a = self.a
        next_state.x = self.x
        next_state.y = self.y
        next_state.s = self.s
        next_state.p = self.p
        next_state.p_mask = self.p_mask

        next_state.ir = self.ir
        next_state.time = 0
        next_state.phase = Phase.Execute
        next_state.cycle_known = True

        c = self.opcode()
        match c:
            case 'TXS':
                next_state.s = self.x
                next_state.phase = Phase.Fetch
            case 'TAX':
                next_state.x = self.a
                next_state.phase = Phase.Fetch
            case 'TXA':
                next_state.a = self.x
                next_state.phase = Phase.Fetch
            case 'TAY':
                next_state.y = self.a
                next_state.phase = Phase.Fetch
            case 'TYA':
                next_state.a = self.y
                next_state.phase = Phase.Fetch
            case 'NOP':
                next_state.phase = Phase.Fetch
            case 'DEX':
                next_state.x = ((self.x - 1) & 0xFF) if self.x is not None else None
                next_state.update_flags_nz(next_state.x)
                next_state.phase = Phase.Fetch
            case 'INX':
                next_state.x = ((self.x + 1) & 0xFF) if self.x is not None else None
                next_state.update_flags_nz(next_state.x)
                next_state.phase = Phase.Fetch
            case 'DEY':
                next_state.y = ((self.y - 1) & 0xFF) if self.y is not None else None
                next_state.update_flags_nz(next_state.y)
                next_state.phase = Phase.Fetch
            case 'INY':
                next_state.y = ((self.y + 1) & 0xFF) if self.y is not None else None
                next_state.update_flags_nz(next_state.y)
                next_state.phase = Phase.Fetch
            case 'ADC':
                if self.a is not None and (self.p_mask & 0b0000_0001) != 0:
                    c = self.p & 0x1
                    a = (self.a + data + c)
                    next_state.a = a & 0xFF
                    next_state.update_flag_v((self.a ^ a) & (data ^ a) & 0x80 != 0)
                    next_state.update_flag_c(a & 0x100 != 0)
                    next_state.update_flags_nz(next_state.a)
                else:
                    next_state.a = None
                    next_state.unknown_flags()
            case 'AND':
                next_state.a = (self.a & data) if self.a is not None else None
                next_state.update_flags_nz(next_state.a)
            case 'BIT':
                # TODO:
                self.unknown_flags()
            case 'CMP':
                # TODO:
                self.unknown_flags()
            case 'CPX':
                # TODO:
                self.unknown_flags()
            case 'CPY':
                # TODO:
                self.unknown_flags()
            case 'EOR':
                next_state.a = (self.a ^ data) if self.a is not None else None
                next_state.update_flags_nz(next_state.a)
            case 'LDA':
                next_state.a = data
                next_state.update_flags_nz(next_state.a)
                next_state.phase = Phase.Fetch
            case 'LDX':
                next_state.x = data
                next_state.update_flags_nz(next_state.x)
                next_state.phase = Phase.Fetch
            case 'LDY':
                next_state.y = data
                next_state.update_flags_nz(next_state.y)
                next_state.phase = Phase.Fetch
            case 'ORA':
                next_state.a = (self.a | data) if self.a is not None else None
                next_state.update_flags_nz(next_state.a)
            case 'SBC':
                # TODO:
                self.a = None
                self.unknown_flags()
        
        return next_state
    
    def pc_plus(self, value: int):
        if self.pc is not None:
            return (self.pc + value) & 0xFFFF
        else:
            return None

    '''
    def use_next_state(self):
        assert(self.next_state is not None)
        if are_different_or_none(self.pc, self.next_state.pc):
            print(f'warning: mismatch in PC (expected {self.pc:04X}, actual {self.next_state.pc:04X})')
        self.pc = self.next_state.pc
        if are_different_or_none(self.a, self.next_state.a):
            print(f'warning: mismatch in A (expected {self.a:02X}, actual {})')
        if self.pc is not None and self.next_state.pc is not None and self.next_state.pc
    '''

    def use_next_state(self):
        assert(self.next_state is not None)

        self.pc = self.next_state.pc
        
        self.a = self.next_state.a
        self.x = self.next_state.x
        self.y = self.next_state.y
        self.s = self.next_state.s
        self.p = self.next_state.p

        self.ir = self.next_state.ir
        self.time = self.next_state.time
        self.phase = self.next_state.phase
        self.cycle_known = self.next_state.cycle_known
    
    def reset(self, nested=True):
        self.pc = None
        self.a = None
        self.x = None
        self.y = None
        self.s = None
        self.p = 0
        self.p_mask = 0
        self.ir = 0
        self.time = 0
        self.phase = Phase.Fetch
        self.cycle_known = False
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
        m = disasm.get_modes(self.ir)
        if m is None:
            return None

        c, _ = disasm.get_modes(self.ir)
        return c
    
    def addr_mode(self):
        if self.ir is None:
            return None
        m = disasm.get_modes(self.ir)
        if m is None:
            return None

        _, m = disasm.get_modes(self.ir)
        return m
    
    def update_flag_c(self, c):
        self.p_mask |= 0b0000_0001
        self.p &= ~0b0000_0001
        if c:
            self.p |= 0b0000_0001
    
    def update_flag_v(self, v):
        self.p_mask |= 0b0100_0000
        self.p &= ~0b0100_0000
        if v:
            self.p |= 0b0100_0000
    
    def update_flags_nz(self, value: Optional[int]):
        if value is not None:
            n = byte_to_signed(value) < 0
            z = value == 0
            self.p_mask |= 0b1000_0010
            self.p &= ~0b1000_0010
            self.p |= (+n) << 7
            self.p |= (+z) << 1
        else:
            self.p_mask &= ~0b1000_0010

    def unknown_flags(self):
        self.p_mask = 0b0000_0000

    def update(self, state: BusState):
        assert(self.next_state is not None)
        self.use_next_state()

        if state.sync:
            self.phase = Phase.Fetch
        self.apply_constraints(state)
        
        if self.phase == Phase.Fetch:
            self.next_state = self.do_fetch(state.addr, state.data)
        elif self.phase == Phase.Load:
            self.next_state = self.do_load()
        elif self.phase == Phase.Execute:
            self.next_state = self.do_execute(state.addr, state.data)

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
        
        '''
        result += '  IR: '
        if self.ir is not None:
            result += f'{self.ir:02X}'
            pair = disasm.get_modes(self.ir)
            if pair is not None:
                result += f' ({pair[0]} {pair[1]})'
        else:
            result += '??'
        
        result += f' {self.phase} {self.time} {self.next_state.phase}'
        '''

        return result

def main():
    port = None

    argv = copy.copy(sys.argv)
    i = 0
    while i < len(argv):
        if argv[i].startswith('--port='):
            port = argv[i].removeprefix('--port=')
            del argv[i]
        else:
            i += 1
    
    if port is None:
        print('Must specify a port for the debugger using --port=<port>')
        sys.exit(1)

    if len(argv) > 3:
        print('Too many arguments')
        print('Usage: debugger.py [options] [asm listing file] [asm symbol table]')
        sys.exit(1)
    if len(argv) == 2:
        print('Must provide both listing file and symbol table (or neither)')
        print('Usage: debugger.py [options] [asm listing file] [asm symbol table]')
        sys.exit(1)
    
    if len(argv) == 3:
        lst_file_path = argv[1]
        with open(lst_file_path, 'r') as f:
            lst_data = f.read()
        listing = dasm.Listing(lst_data)

        sym_file_path = argv[2]
        with open(sym_file_path, 'r') as f:
            sym_data = f.read()
        symbol_table = dasm.SymbolTable(sym_data)

        print(f'Loaded debug info for file {listing.file_name}')
    else:
        listing = None
        symbol_table = None

    dbg = Debugger.open(port)

    info = dbg.print_info()
    print('Connected to debugger')
    print(f'Version: {info}')

    dbg.reset_cpu()

    free_running = False
    while True:
        if free_running:
            try:
                while True:
                    which_breakpoint = dbg.poll_breakpoint()
                    if which_breakpoint is not None:
                        print(f'Hit breakpoint {which_breakpoint}, stopping')
                        break
                    time.sleep(0.1)
            except KeyboardInterrupt:
                print('Keyboard interrupt, stopping')
                dbg.step_half_cycle()
            
            free_running = False

        state = dbg.get_bus_state()
        if dbg.cpu.pc is not None and listing is not None:
            for line in listing.neighborhood(dbg.cpu.pc):
                print(f'${line.address:04X} {line.line_number:6}: ', end='')
                if dbg.cpu.pc == line.address:
                    print(' -> ', end='')
                else:
                    print('    ', end='')
                print(line.source)

        print()
        print(dbg.cpu.display())
        print(f'ADDR: {state.addr:04X}')
        print(f'DATA:   {state.data:02X}')
        print(f'STATUS: ', end='')
        print('SYNC ' if state.sync else '     ', end='')
        print('     ' if state.resb else 'RESB ', end='')
        print('     ' if state.vpb  else 'VPB  ', end='')
        print('   |   ', end='')
        print(f'PHI2: {+state.phi2}')
        print()

        cmd = input('> ')

        cmd, *args = cmd.split()

        try:
            if cmd in ('s', 'step'):
                free_running = False
                dbg.step()
            elif cmd in ('h', 'stephalf'):
                free_running = False
                dbg.step_half_cycle()
            elif cmd in ('y', 'stepcycle'):
                free_running = False
                dbg.step_cycle()
            elif cmd in ('c', 'continue'):
                free_running = True
                dbg.cont()
            elif cmd in ('q'):
                break
            elif cmd in ('r', 'reset'):
                dbg.reset_cpu()
            elif cmd in ('b', 'break'):
                if len(args) > 1:
                    print('Too many arguments')
                elif len(args) == 0:
                    print('Must specify a line number, symbol, or address to set a breakpoint at')
                else:
                    loc = args[0]
                    if loc.startswith('$'):
                        addr = int(loc[1:], base=16)
                        dbg.set_breakpoint(addr)
                    elif listing is not None:
                        assert(symbol_table is not None)
                        if loc in symbol_table.symbols:
                            addr = symbol_table.symbols[loc]
                        else:
                            line_number = int(loc)
                            addr = listing.line_to_addr(line_number)
                        if addr is not None:
                            dbg.set_breakpoint(addr)
                        else:
                            print('No such line or symbol in file')
                    else:
                        print('Cannot set breakpoint on line without source info (provide a listing file)')
            else:
                print(f'Unknown command {repr(cmd)}')
        except ValueError as e:
            print(f'Error while executing command: {e}')
    
    print('Debugger exited')

if __name__ == '__main__':
    main()