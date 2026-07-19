import math
from collections import Counter

class Encoder:
    frequency_table: dict
    M: int
    L: int
    b: int

    def __init__(self, frequency_table: dict, M: int, L: int, b: int) -> None:
        self.frequency_table = frequency_table
        self.M = M
        self.L = L
        self.b = b

def get_state_width_bytes(L: int, b: int) -> int:
    max_value = L * b - 1
    bits_needed = max_value.bit_length()

    return math.ceil(bits_needed / 8)

def encode(data_streams: list[list[int]], encoders: list[Encoder]) -> bytes:
    if len(data_streams) != len(encoders):
        raise ValueError('data_streams must have the same length as encoders')

    streams = []

    for i, encoder in enumerate(encoders):
        data = data_streams[i]

        if encoder.L < encoder.M:
            raise ValueError('L should be at least as large as M')

        if data.count(0) != 0:
            raise ValueError('The data cannot contain 0 since it is the terminator')

        data.append(0)

        x = encoder.L
        stream = []

        for d in reversed(data):
            try:
                f, c = encoder.frequency_table[d]
            except KeyError:
                raise ValueError(f'{d} is not in the frequency table')

            while encoder.M * x >= encoder.b * encoder.L * f:
                stream.append(x % encoder.b)
                x //= encoder.b

            x = (x // f) * encoder.M + c + (x % f)

        out = bytearray()
        out += bytes(stream)
        out += x.to_bytes(get_state_width_bytes(encoder.L, encoder.b), byteorder='big')

        streams.append(out)

    packed_stream = bytearray()

    for stream in reversed(streams):
        packed_stream += stream

    return packed_stream

# TODO we might be able to pack all state values in the same space
def decode(data: bytes, encoders: list[Encoder]) -> list[list[int]]:
    results = []
    remaining = bytearray(data)

    for encoder in encoders:
        state_width = get_state_width_bytes(encoder.L, encoder.b)

        x = int.from_bytes(remaining[-state_width:], byteorder='big')
        stream = list(remaining[:-state_width])

        slot_to_symbol = build_slot_table(encoder.frequency_table, encoder.M)

        message = []

        while True:
            slot = x % encoder.M
            s = slot_to_symbol[slot]
            f, c = encoder.frequency_table[s]
            x = (x // encoder.M) * f + slot - c

            while x < encoder.L and stream:
                x = x * encoder.b + stream.pop()

            if s == 0:
                break

            message.append(s)

        results.append(message)
        remaining = bytearray(stream)

    return results

def build_slot_table(frequency_table: dict, M: int) -> list[int]:
    slot_to_symbol = [0] * M

    for char, (f, c) in frequency_table.items():
        for i in range(f):
            slot_to_symbol[c + i] = char

    return slot_to_symbol

def generate_frequency_tables(counter: Counter, M: int, n_values: int, name: str):
    if M < n_values:
        raise ValueError('M cannot be smaller than n_values')

    if len(counter.keys()) > n_values:
        raise ValueError('counter cannot contain more keys than the value of n_values')

    if n_values >= 2**64:
        raise ValueError('n_values cannot exceed u64 capacity')

    for i in range(n_values):
        counter.setdefault(i, 0)

    total_count = sum(counter.values())

    if total_count == 0:
        raise ValueError('The sum of counts in counter must be larger than 0')

    frequency_map = {value: max(1, round(count / total_count * M)) for value, count in counter.items()}

    current_sum = sum(frequency_map.values())

    while current_sum != M:
        diff = M - current_sum

        sorted_keys = sorted(
            frequency_map.keys(),
            key=lambda x: (counter[x] / total_count) * M - frequency_map[x],
            reverse=(diff > 0)
        )

        for k in sorted_keys:
            if diff == 0: break
            if diff > 0:
                frequency_map[k] += 1
                diff -= 1
            elif diff < 0 and frequency_map[k] > 1:
                frequency_map[k] -= 1
                diff += 1

        current_sum = sum(frequency_map.values())

    sorted_values = sorted(frequency_map.keys())

    frequency_rans = {}
    slot_to_symbol = []

    cum_sum = 0

    for value in sorted_values:
        f = frequency_map[value]
        frequency_rans[value] = (f, cum_sum)
        slot_to_symbol.extend([value] * f)
        cum_sum += f

    py = f'M = {M}\n'
    py += f'n_values = {n_values}\n\n'

    py += f'{name}_frequencies = {{\n'

    for value, (f, c) in frequency_rans.items():
        py += f'    {value}: ({f}, {c}),\n'

    py += '}\n'

    with open(f'scripts/{name}_table.py', 'w') as file:
        file.write(py)

    c = f'#include "{name}_table.h"\n\n'

    data_type = 'uint64_t'

    if n_values < 2**8:
        data_type = 'uint8_t'
    elif n_values < 2**16:
        data_type = 'uint16_t'
    elif n_values < 2**32:
        data_type = 'uint32_t'

    c += f'const {name}_frequency_t {name.upper()}_FREQUENCIES[{name.upper()}_N_VALUES] = {{\n'

    for value in sorted_values:
        f, cum = frequency_rans[value]
        c += f'    {{{f}, {cum}}},\n'

    c += '};\n\n'

    c += f'const {data_type} {name.upper()}_TABLE[{name.upper()}_TABLE_SIZE] = {{\n'

    for command in slot_to_symbol:
        c += f'    {command},\n'

    c += '};\n'

    with open(f'{name}_table.c', 'w') as file:
        file.write(c)

    c_header =  f'#ifndef {name.upper()}_TABLE_H\n'
    c_header += f'#define {name.upper()}_TABLE_H\n\n'

    c_header += '#include <stdint.h>\n\n'

    c_header += f'#define {name.upper()}_TABLE_SIZE {M}\n'
    c_header += f'#define {name.upper()}_N_VALUES {n_values}\n\n'

    c_header += f'typedef struct {{\n'
    c_header += f'    {data_type} f;\n'
    c_header += f'    {data_type} c;\n'
    c_header += f'}} {name}_frequency_t;\n\n'

    c_header += f'extern const {data_type} {name.upper()}_TABLE[{name.upper()}_TABLE_SIZE];\n'
    c_header += f'extern const {name}_frequency_t {name.upper()}_FREQUENCIES[{name.upper()}_N_VALUES];\n\n'

    c_header += '#endif\n'

    with open(f'{name}_table.h', 'w') as file:
        file.write(c_header)
