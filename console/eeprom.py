from serial import Serial
import argparse
import time
import dasm
import copy

from PIL import Image

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
CMD_READ_EEPROM = b'\x03'
CMD_PRINT_INFO = b'\x04'

class InvalidHeader(Exception):
    pass

class Programmer:
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


    def start_command(self, b: bytes):
        assert(len(b) == 1)
        self.write(b)
        echo = self.read_exact(1)
        if echo != b:
            raise InvalidHeader(f'expected {b} but found {echo}')

    def ping(self):
        self.start_command(CMD_PING)
        return self.read(5)

    def page_write(self, page: int, data: bytes):
        self._raw_page_write(page, data)
        time.sleep(0.015)
        written = self.read_eeprom(page, len(data))
        if data != written:
            print('Data mismatch')

            print('(first 32 bytes)')
            print('tried to write: ', end='')
            for d in data[:32]:
                print(f'{d:02X} ', end='')
            print()
            print('but read back:  ', end='')
            for d in written[:32]:
                print(f'{d:02X} ', end='')
            print()
            print('                ', end='')
            for d, w in zip(data[:32], written[:32]):
                if d != w:
                    print('^^ ', end='')
                else:
                    print('   ', end='')
            print()
            print()
            
            print('(last 32 bytes)')
            print('tried to write: ', end='')
            for d in data[32:][:32]:
                print(f'{d:02X} ', end='')
            print()
            print('but read back:  ', end='')
            for d in written[32:][:32]:
                print(f'{d:02X} ', end='')
            print()
            print('                ', end='')
            for d, w in zip(data[32:][:32], written[32:][:32]):
                if d != w:
                    print('^^ ', end='')
                else:
                    print('   ', end='')
            print()
            raise Exception('data mismatch')

    def _raw_page_write(self, page: int, data: bytes):
        assert(page & EEPROM_PAGE_MASK == 0), f'misaligned page {page:04X}'
        assert(len(data) == EEPROM_PAGE_SIZE)

        self.start_command(CMD_WRITE_EEPROM)

        data = page.to_bytes(2, 'little') + data
        checksum = update_crc(0, data)
        data = data + checksum.to_bytes(2, 'little')

        self.write(data)

        confirm = self.read_exact(1)
        if confirm != b'\x00':
            raise Exception(confirm)
    
    def read_eeprom(self, addr: int, len_bytes: int):
        assert(len_bytes < 0x1_0000)

        self.start_command(CMD_READ_EEPROM)

        cmd = addr.to_bytes(2, 'little') + len_bytes.to_bytes(2, 'little')
        self.write(cmd)

        data = self.read_exact(len_bytes)
        assert(len(data) == len_bytes)
        crc = self.read_exact(2)
        return data

EEPROM_PAGE_BITS = 6
EEPROM_PAGE_SIZE = (1 << EEPROM_PAGE_BITS)
EEPROM_PAGE_MASK = (EEPROM_PAGE_SIZE - 1)

def program_chunks(prog, chunks):
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
            prog.page_write(base_addr + page, data[page:][:EEPROM_PAGE_SIZE])
            print('.', end='', flush=True)
    print()
    
def main():
    parser = argparse.ArgumentParser(
        prog='EEPROM Programmer',
        description='Interfaces with a DIY Raspberry Pi Pico EEPROM programmer')

    parser.add_argument('-p', '--port', required=True, help='Serial port path for the programmer')

    args = parser.parse_args()

    prog = Programmer(Serial(args.port, 115200, timeout=0.0), 0.5)

    with open('./asm.bin', 'rb') as f:
        bin_data = f.read()
    chunks = dasm.get_chunks(bin_data)

    print(prog.ping())

    #program_chunks(prog, chunks)

    img = Image.open('/Users/salix/Downloads/sky-thick-scarf.png')

    img = img.resize((64, 64))

    for y in range(64):
        page = []
        for x in range(64):
            r, g, b, a = img.getpixel((x, y))
            rgb = ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6)
            page.append(rgb)

        page = bytes(page)
        prog.page_write(y * 128, page)

    #for i in range(0x0000, 0x2000, 64):
    #    chunk = bytes((i + off) % 256 for off in range(64))
    #    prog.page_write(i, chunk)
    



if __name__ == '__main__':
    main()