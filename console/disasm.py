from enum import Enum

class AddrMode(Enum):
    Zpg  = 'zpg'
    'Zero page'
    ZpgX = 'zp,X'
    'Zero page, X indexed'
    ZpgY = 'zp,Y'
    'Zero page, Y indexed'
    Imm  = '#'
    'Immediate'
    Abs  = 'abs'
    'Absolute'
    AbsX = 'abs,X'
    'Absolute, X indexed'
    AbsY = 'abs,Y'
    'Absolute, Y indexed'
    Ind = '(ind)'
    'Absolute Indirect'
    XInd = 'X,ind'
    'X-indexed Indirect'
    IndY = 'ind,Y'
    'Indirect, Y indexed'
    

def get_modes(inst: int):
    assert(0 <= inst < 256)

    lookup = {
        0x10: 'BPL',
        0x30: 'BMI',
        0x50: 'BVC',
        0x70: 'BVS',
        0x90: 'BCC',
        0xB0: 'BCS',
        0xD0: 'BNE',
        0xF0: 'BEQ',

        0x00: 'BRK',
        0x20: ('JSR', 'abs'),
        0x40: 'RTI',
        0x60: 'RTS',
        
        0x08: 'PHP',
        0x28: 'PLP',
        0x48: 'PHA',
        0x68: 'PLA',
        0x88: 'DEY',
        0xA8: 'TAY',
        0xC8: 'INY',
        0xE8: 'INX',

        0x18: 'CLC',
        0x38: 'SEC',
        0x58: 'CLI',
        0x78: 'SEI',
        0x98: 'TYA',
        0xB8: 'CLV',
        0xD8: 'CLD',
        0xF8: 'SED',

        0x8A: 'TXA',
        0x9A: 'TXS',
        0xAA: 'TAX',
        0xBA: 'TSX',
        0xCA: 'DEX',
        0xEA: 'NOP',
    }

    if inst in lookup:
        l = lookup[inst]
        if isinstance(l, str):
            return (l, 'impl')
        else:
            return l
        
    aaa = (inst >> 5) & 0x7
    bbb = (inst >> 2) & 0x7
    cc = inst & 0x3

    if cc == 0b01:
        # Group 1 instructions

        opcode = ['ORA', 'AND', 'EOR', 'ADC', 'STA', 'LDA', 'CMP', 'SBC'][aaa]
        mode = ['ind,X', 'zpg', '#', 'abs', 'ind,Y', 'zpg,X', 'abs,Y', 'abs,X'][bbb]

        if opcode == 'STA' and mode == '#':
            raise Exception('STA # does not exist')

        return (opcode, mode)
    elif cc == 0b10:
        # Group 2 instructions

        opcode = ['ASL', 'ROL', 'LSR', 'ROR', 'STX', 'LDX', 'DEC', 'INC'][aaa]
        mode = ['#', 'zpg', 'A', 'abs', None, 'zpg,X', None, 'abs,X'][bbb]

        if mode is None:
            raise Exception('invalid addressing mode bits')
        
        if mode == '#' and opcode != 'LDX':
            raise Exception()
        
        if opcode in ('STX', 'LDX') and mode == 'zp,X':
            mode = 'zp,Y'
        
        if opcode == 'LDX' and mode == 'abs,X':
            mode = 'abs,Y'
        
        return (opcode, mode)
    elif cc == 0b00:
        # Group 3 instructions

        opcode = [None, 'BIT', 'JMP', 'JMP', 'STY', 'LDY', 'CPY', 'CPX'][aaa]
        mode = ['#', 'zpg', None, 'abs', None, 'zpg,X', None, 'abs,X'][bbb]

        if opcode is None:
            raise Exception()
        if mode is None:
            raise Exception()

        return (opcode, mode)
    else:
        return None

