import sys
import subprocess
import debugger
import dasm
from debugger import EEPROM_PAGE_SIZE, EEPROM_PAGE_MASK
import copy

def main():
    port = None
    dasm_path = None
    is_bin = False

    argv = copy.copy(sys.argv)
    i = 0
    while i < len(argv):
        if argv[i].startswith('--port='):
            port = argv[i][len('--port='):]
            del argv[i]
        elif argv[i].startswith('--dasm='):
            dasm_path = argv[i][len('--dasm='):]
            del argv[i]
        elif argv[i].startswith('--bin'):
            is_bin = True
            del argv[i]
        else:
            i += 1
    
    if port is None:
        print('Must specify a port for the debugger using --port=<port>')
        sys.exit(1)
    
    if dasm_path is None and not is_bin:
        print('Must specify location of the DASM executable using --dasm=<path>')
        sys.exit(1)

    if len(argv) != 2:
        print('Wrong number of arguments')
        print('Usage: python deploy.py [options] <path to asm>')
        sys.exit(1)
    
    input_path = argv[1]

    output_dir = './'

    if is_bin:
        if input_path.lower().endswith('.s'):
            print('Warning: .S files usually are text, not binary, and should not be used with --bin!')
            response = input("Are you sure you want to flash this file? (y/n): ")
            response = response.strip().lower()
            if response != 'y':
                print('Canceled')
                return

        with open(input_path, 'rb') as f:
            bin_data = f.read()
        chunks = dasm.get_chunks(bin_data)
    else:
        try:
            assert(dasm_path is not None)
            output = dasm.AssembledFile.from_src(dasm_path, input_path, output_dir)
            chunks = output.chunks
        except dasm.AssemblerError:
            print(f'Encountered errors while assembling file')
            sys.exit(1)

    print('Connecting to debugger')
    dbg = debugger.Debugger.open(port)
    print(f'Firmware version: {dbg.print_info()}')
    print('Successfully connected to debugger!')
    print()

    bad_chunks = []
    for chunk in chunks:
        if not (0 <= chunk.base_addr <= 0xFFFF):
            raise Exception(f'Invalid chunk address {chunk.base_addr}')
        if chunk.base_addr < 0x8000:
            bad_chunks.append(chunk)

    if len(bad_chunks) != 0:
        print('Warning: found chunks that were outside of the normal EEPROM range!')
        for chunk in bad_chunks:
            print(f'  {chunk.base_addr:04X}')

        print("This normally means you're trying to flash something that isn't a binary,")
        print("or you've accidentally included your variables in the output.")
        response = input("Do you want to continue? (y/n): ")
        response = response.strip().lower()
        if response != 'y':
            print('Canceled')
            return

    print('Flashing EEPROM')
    total_pages = sum((len(c.data) + EEPROM_PAGE_SIZE - 1) // EEPROM_PAGE_SIZE for c in chunks)
    print(f'Need to flash {total_pages} pages')

    for i, chunk1 in enumerate(chunks):
        for chunk2 in chunks[i + 1:]:
            c1_start = chunk1.base_addr // EEPROM_PAGE_SIZE
            c1_end = (chunk1.base_addr + len(chunk1.data) + EEPROM_PAGE_SIZE - 1) // EEPROM_PAGE_SIZE

            c2_start = chunk2.base_addr // EEPROM_PAGE_SIZE
            c2_end = (chunk2.base_addr + len(chunk2.data) + EEPROM_PAGE_SIZE - 1) // EEPROM_PAGE_SIZE

            if max(c1_start, c2_start) <= min(c1_end, c2_end):
                raise Exception('TODO: Overlapping chunks')
    
    for chunk in chunks:
        base_addr = chunk.base_addr & ~EEPROM_PAGE_MASK
        data = copy.copy(chunk.data)
        if chunk.base_addr & EEPROM_PAGE_MASK != 0:
            data = b'\xFF' * (chunk.base_addr & EEPROM_PAGE_MASK) + data
        while len(data) & EEPROM_PAGE_MASK != 0:
            data = data + b'\xFF'
        
        for page in range(0, len(data), EEPROM_PAGE_SIZE):
            dbg.page_write(base_addr + page, data[page:][:EEPROM_PAGE_SIZE])
            print('.', end='', flush=True)
    print()
    
    print('Resetting CPU')

    dbg.reset_cpu()

    print('Done!')


if __name__ == '__main__':
    main()