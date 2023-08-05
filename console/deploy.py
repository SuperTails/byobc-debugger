import sys
import subprocess
import debugger

def assemble(asm_path, binary: bool) -> str:
    args = [
        './vasm6502_oldstyle_Win64/vasm6502_oldstyle.exe',
        asm_path,
        '-dotdir',
    ]

    if binary:
        args += ['-Fbin', '-o', 'img.bin']
    else:
        args += ['-o', 'img.out']

    result = subprocess.run(args)
    if result.returncode != 0:
        raise Exception('assembler failed')

    return 'img.bin' if binary else 'img.out'

def main():
    if len(sys.argv) != 2:
        print('Wrong number of arguments')
        print('Usage: python deploy.py <path to asm>')
        raise Exception()

    asm_path = sys.argv[1]

    sym_path = assemble(asm_path, False)
    bin_path = assemble(asm_path, True)

    with open(bin_path, 'rb') as f:
        bin_data = f.read()

    dbg = debugger.Debugger.open_default_port()
    dbg.ping()

    print('Flashing EEPROM', end='')
    for page in range(0x8000, 0x1_0000, 0x80):
        dbg.page_write(page, bin_data[page - 0x8000:][:0x80])
        print('.', end='', flush=True)
    print()
    
    #dbg.page_write(0x8000, bin_data[:128])
    #dbg.page_write(0x8080, bin_data[128:256])
    #dbg.page_write(0xFF80, bin_data[-128:])

    dbg.set_breakpoint(0x801F)

    dbg.reset_cpu()


if __name__ == '__main__':
    main()