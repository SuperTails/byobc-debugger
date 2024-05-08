import subprocess
from typing import List, Tuple, Optional, Dict
from tempfile import NamedTemporaryFile
from dataclasses import dataclass

class AssemblerError(Exception):
    pass

def assemble(assembler_path: str, input_file: str, output_bin: str, output_listing: str, output_symbols: str):
	result = subprocess.run([
		assembler_path,
		input_file,
		f'-o{output_bin}',
		f'-l{output_listing}',
		f'-s{output_symbols}',
		'-f2', # Random Access Segment format, to avoid flashing unused parts of the EEPROM
	])

	if result.returncode != 0:
		raise AssemblerError()

def is_number(s: str, base: int):
	try:
		int(s, base=base)
		return True
	except ValueError:
		return False


def next_number(s: str, base: int) -> Tuple[int, str]:
	s = s.lstrip()

	num = None
	end_index = None
	for i in range(1, len(s)):
		try:
			num = int(s[:i], base)
			if s[i].isspace():
				end_index = i
				break
			else:
				end_index = i + 1
		except ValueError:
			break
	
	if num is None:
		raise ValueError
	
	return num, s[end_index:]

@dataclass
class SymbolTable:
	symbols: Dict[str, int]

	def __init__(self, data: str):
		self.symbols = dict()
		for line in data.splitlines()[1:-1]:
			#name = line[:25].strip()
			name, line = line.split(maxsplit=1)
			#addr = int(line[25:29], base=16)
			addr = int(line[:4], base=16)
			self.symbols[name] = addr

@dataclass
class LineInfo:
	line_number: int
	address: int
	data: bytes
	source: str

@dataclass
class Listing:
	file_name: str
	lines: List[LineInfo]

	def __init__(self, data: str):
		lines = data.splitlines()
		header_parts = lines[0].split()
		self.file_name = ' '.join(header_parts[header_parts.index('FILE') + 1:header_parts.index('LEVEL')])
		self.lines = []

		for line in lines[1:]:
			if line == '':
				continue

			# General format:
			# '      9  8002                  8d 00 60    foobar     sta       SOME_MEM_LOC'

			line = tabs_to_spaces(line, 8)

			line_number = int(line[0:7])
			addr = int(line[9:13], base=16)

			asm_bytes = line[31:39]
			asm_bytes = bytes([int(b, base=16) for b in asm_bytes.strip().split()])

			rest = line[43:]

			self.lines.append(LineInfo(line_number, addr, asm_bytes, rest))
		
	def line_to_addr(self, line_number: int) -> Optional[int]:
		for info in self.lines:
			if info.line_number == line_number:
				return info.address
		return None
	
	def neighborhood(self, addr: int) -> List[LineInfo]:
		start_idx = None
		for i, line in enumerate(self.lines):
			if line.address == addr:
				start_idx = i
				break
		end_idx = None
		for i, line in reversed(tuple(enumerate(self.lines))):
			if line.address == addr:
				end_idx = i
				break
		
		if start_idx == None or end_idx == None:
			return []
		
		if start_idx > 0:
			start_idx -= 1
		
		if end_idx + 1 < len(self.lines):
			end_idx += 1
		
		return self.lines[start_idx:end_idx + 1]
			

def tabs_to_spaces(s: str, width: int) -> str:
	result = ''

	for c in s:
		if c == '\t':
			if len(result) % width == 0:
				result += ' ' * width
			else:
				while len(result) % width != 0:
					result += ' '
		else:
			result += c
		
	return result

class ChunkFormatError(Exception):
	'''Raised when reading a bin file that does not have enough data.'''

@dataclass
class Chunk:
	base_addr: int
	data: bytes

def get_chunks(bin_data: bytes):
	chunks = []
	while len(bin_data) > 0:
		if len(bin_data) < 4:
			raise ChunkFormatError()
		chunk_base_addr = int.from_bytes(bin_data[0:2], 'little')
		chunk_length = int.from_bytes(bin_data[2:4], 'little')
		chunk_data = bin_data[4:][:chunk_length]
		if len(bin_data) < 4 + chunk_length:
			raise ChunkFormatError()
		bin_data = bin_data[4 + chunk_length:]
		chunks.append(Chunk(chunk_base_addr, chunk_data))
	return chunks

@dataclass
class AssembledFile:
	chunks: List[Chunk]
	listing: Listing
	symbol_table: SymbolTable

	@classmethod
	def from_src(cls, assembler_path: str, input_file: str, output_dir: Optional[str] = None):
		if output_dir is None:
			bin_file = NamedTemporaryFile('rb')
			lst_file = NamedTemporaryFile('r')
			sym_file = NamedTemporaryFile('r')

			assemble(assembler_path, input_file, bin_file.name, lst_file.name, sym_file.name)

			bin_data = bin_file.read()
			lst_data = lst_file.read()
			sym_data = sym_file.read()
		else:
			bin_path = output_dir + '/asm.bin'
			lst_path = output_dir + '/asm.lst'
			sym_path = output_dir + '/asm.sym'

			assemble(assembler_path, input_file, bin_path, lst_path, sym_path)

			with open(bin_path, 'rb') as f:
				bin_data = f.read()
			with open(lst_path, 'r') as f:
				lst_data = f.read()
			with open(sym_path, 'r') as f:
				sym_data = f.read()

		chunks = get_chunks(bin_data)
		
		return cls(chunks, Listing(lst_data), SymbolTable(sym_data))

def main():
	#assemble('/Users/salix/Code/dasm/bin/dasm', './foobar.asm', './aaaaa.out', './aaaaa.lst')
	f = AssembledFile.from_src('/Users/salix/Code/dasm/bin/dasm', './console/doubledabble.S', './')

if __name__ == '__main__':
	main()