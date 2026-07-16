# Source: https://fergusfinn.com/blog/understanding-rans/

import probabilities as p

NUM_CHARS = 127
NUM_COMMANDS = 1463

_C_ESCAPES = {
    '\\': '\\\\',
    "'": "\\'",
    '"': '\\"',
    '\n': '\\n',
    '\t': '\\t',
    '\r': '\\r',
    '\a': '\\a',
    '\b': '\\b',
    '\f': '\\f',
    '\v': '\\v',
    '\0': '\\0',
}

def convert_to_c_char_literal(ch: str) -> str:
    if ch in _C_ESCAPES:
        return f"'{_C_ESCAPES[ch]}'"

    codepoint = ord(ch)

    if 0x20 <= codepoint < 0x7f:
        return f"'{ch}'"

    return f"'\\x{codepoint:02x}'"

def build_title_tables(M: int):
    assert M >= NUM_CHARS

    with open('dataset.txt', 'r') as file:
        counter = p.get_title_probabilities([line for line in file])

    for b in range(NUM_CHARS):
        counter.setdefault(chr(b), 0)

    total_count = sum(counter.values())

    freq_map = {char: max(1, round(count / total_count * M)) for char, count in counter.items()}

    current_sum = sum(freq_map.values())

    while current_sum != M:
        diff = M - current_sum

        sorted_keys = sorted(
            freq_map.keys(),
            key=lambda x: (counter[x] / total_count) * M - freq_map[x],
            reverse=(diff > 0)
        )

        for k in sorted_keys:
            if diff == 0: break
            if diff > 0:
                freq_map[k] += 1
                diff -= 1
            elif diff < 0 and freq_map[k] > 1:
                freq_map[k] -= 1
                diff += 1

        current_sum = sum(freq_map.values())

    sorted_chars = sorted(freq_map.keys(), key=lambda x: ord(x))

    freq_rans = {}
    slot_to_symbol = []

    cum_sum = 0

    for char in sorted_chars:
        f = freq_map[char]
        freq_rans[char] = (f, cum_sum)
        slot_to_symbol.extend([char] * f)
        cum_sum += f

    py = 'title_frequencies = {\n'

    for char, (f, c) in freq_rans.items():
        py += f'    {repr(char)}: ({f}, {c}),\n'

    py += '}\n'

    with open('scripts/titles_table.py', 'w') as file:
        file.write(py)

    c = f'const char TITLE_CHAR_TABLE[{M}] = {{\n'

    for char in slot_to_symbol:
        c += f'    {convert_to_c_char_literal(char)},\n'

    c += "};\n"

    with open('titles_table.c', 'w') as file:
        file.write(c)

def build_tone_commands_table(M: int):
    assert M >= NUM_COMMANDS

    with open('dataset.txt', 'r') as file:
        counter = p.get_command_probabilities([line for line in file])

    for b in range(NUM_COMMANDS):
        counter.setdefault(b, 0)

    total_count = sum(counter.values())

    freq_map = {command: max(1, round(count / total_count * M)) for command, count in counter.items()}

    current_sum = sum(freq_map.values())

    while current_sum != M:
        diff = M - current_sum

        sorted_keys = sorted(
            freq_map.keys(),
            key=lambda x: (counter[x] / total_count) * M - freq_map[x],
            reverse=(diff > 0)
        )

        for k in sorted_keys:
            if diff == 0: break
            if diff > 0:
                freq_map[k] += 1
                diff -= 1
            elif diff < 0 and freq_map[k] > 1:
                freq_map[k] -= 1
                diff += 1

        current_sum = sum(freq_map.values())

    sorted_commands = sorted(freq_map.keys())

    freq_rans = {}
    slot_to_symbol = []

    cum_sum = 0

    for command in sorted_commands:
        f = freq_map[command]
        freq_rans[command] = (f, cum_sum)
        slot_to_symbol.extend([command] * f)
        cum_sum += f

    py = 'command_frequencies = {\n'

    for command, (f, c) in freq_rans.items():
        py += f'    {command}: ({f}, {c}),\n'

    py += '}\n'

    with open('scripts/commands_table.py', 'w') as file:
        file.write(py)

    c = f'const char COMMAND_TABLE[{M}] = {{\n'

    for command in slot_to_symbol:
        c += f'    {command},\n'

    c += "};\n"

    with open('commands_table.c', 'w') as file:
        file.write(c)

if __name__ == '__main__':
    # build_title_tables(256)
    build_tone_commands_table(4096)
