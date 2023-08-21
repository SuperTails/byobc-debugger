import sys
import subprocess
import debugger
import dasm
import copy

def main():
    if len(sys.argv) != 3:
        print('Wrong number of arguments')
        print('Usage: python deploy.py <path to dasm> <path to asm>')
        sys.exit(1)

    dasm_path = sys.argv[1]
    input_path = sys.argv[2]

    output_dir = './'

    try:
        output = dasm.AssembledFile.from_src(dasm_path, input_path, output_dir)
    except dasm.AssemblerError:
        print(f'Encountered errors while assembling file')
        sys.exit(1)

    print('Connecting to debugger')

    dbg = debugger.Debugger.open_default_port()
    dbg.ping()

    print('Flashing EEPROM')
    total_pages = sum((len(c.data) + 0x7F) // 0x80 for c in output.chunks)
    print(f'Need to flash {total_pages} pages')

    for i, chunk1 in enumerate(output.chunks):
        for chunk2 in output.chunks[i + 1:]:
            c1_start = chunk1.base_addr // 0x80
            c1_end = (chunk1.base_addr + len(chunk1.data) + 0x7F) // 0x80

            c2_start = chunk2.base_addr // 0x80
            c2_end = (chunk2.base_addr + len(chunk2.data) + 0x7F) // 0x80

            if max(c1_start, c2_start) <= min(c1_end, c2_end):
                raise Exception('TODO: Overlapping chunks')
    
    for chunk in output.chunks:
        base_addr = chunk.base_addr & ~0x7F
        data = copy.copy(chunk.data)
        if chunk.base_addr & 0x7F != 0:
            data = b'\xFF' * (chunk.base_addr & 0x7F) + data
        while len(data) & 0x7F != 0:
            data = data + b'\xFF'
        
        for page in range(0, len(data), 0x80):
            dbg.page_write(base_addr + page, data[page:][:0x80])
            print('.', end='', flush=True)
    print()
    
    print('Resetting CPU')

    dbg.reset_cpu()

    print('Done!')


if __name__ == '__main__':
    main()