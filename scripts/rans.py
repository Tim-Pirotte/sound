import math
from collections import Counter

import probabilities as p
import encode_title as et

def get_state_width_bytes(L: int, b: int) -> int:
    max_value = L * b - 1
    bits_needed = max_value.bit_length()

    return math.ceil(bits_needed / 8)

def encode(data: list[int], frequency_table: dict, M: int, L: int, b: int):
    if L < M:
        raise ValueError('L should be at least as large as M')

    if data.count(0) != 0:
        raise ValueError('The data cannot contain 0 since it is the terminator')

    data.append(0)

    x = L
    stream = []

    for d in reversed(data):
        try:
            f, c = frequency_table[d]
        except KeyError:
            raise ValueError(f'{d} is not in the frequency table')

        while M * x >= b * L * f:
            stream.append(x % b)
            x //= b

        x = (x // f) * M + c + (x % f)

    out = bytearray()
    out += x.to_bytes(get_state_width_bytes(L, b), byteorder='big')
    out += bytes(stream)

    return bytes(out)

def decode(data: bytes, frequency_table: dict, M: int, L: int, b: int) -> list[int]:
    state_width = get_state_width_bytes(L, b)

    x = int.from_bytes(data[:state_width], byteorder='big')
    stream = list(data[state_width:])

    slot_to_symbol = build_slot_table(frequency_table, M)
    message = []

    stream = list(stream)

    while True:
        slot = x % M
        s = slot_to_symbol[slot]
        f, c = frequency_table[s]
        x = (x // M) * f + slot - c

        while x < L and stream:
            x = x * b + stream.pop()

        if s == 0:
            break

        message.append(s)

    return message

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

    py = f'{name}_frequencies = {{\n'

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

    c += f'const {data_type} {name.upper()}_TABLE[{name.upper()}_TABLE_SIZE] = {{\n'

    for command in slot_to_symbol:
        c += f'    {command},\n'

    c += '};\n'

    with open(f'{name}_table.c', 'w') as file:
        file.write(c)

    c_header = f'#ifndef {name.upper()}_TABLE_H\n'
    c_header += f'#define {name.upper()}_TABLE_H\n\n'
    c_header += '#include <stdint.h>\n\n'
    c_header += f'#define {name.upper()}_TABLE_SIZE {M}\n\n'
    c_header += f'extern const {data_type} {name.upper()}_TABLE[{name.upper()}_TABLE_SIZE];\n\n'
    c_header += '#endif\n'

    with open(f'{name}_table.h', 'w') as file:
        file.write(c_header)

if __name__ == '__main__':
    with open('dataset.txt', 'r') as file:
        lines = [line for line in file]

        title_counter = et.get_title_probabilities(lines)
        tone_command_counter = p.get_command_probabilities(lines)

    generate_frequency_tables(title_counter, 256, 127, 'titles')
    generate_frequency_tables(tone_command_counter, 4096, 1463, 'commands')
