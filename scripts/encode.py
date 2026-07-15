import math
import importlib

import generate_rANS_tables as g
import titles_table as t

def get_state_width(L: int, b: int) -> int:
    max_value = L * b - 1
    bits_needed = max_value.bit_length()

    return math.ceil(bits_needed / 8)

def encode_title(title: str, freq: dict, M: int, L: int, b: int) -> bytes:
    if L < M:
        raise ValueError('L should be larger than or equal to M')

    if len(title) < 1 or title[-1] != ':':
        raise ValueError("The title should end on ':'")

    if title.count(':') != 1:
        raise ValueError("There cannot be another ':' in the title")

    x = L
    stream = []

    for c in reversed(title):
        if ord(c) > 127:
            raise ValueError("Only ASCII characters are allowed")

        f, c = freq[c]

        while M * x >= b * L * f:
            stream.append(x % b)
            x //= b

        x = (x // f) * M + c + (x % f)

    out = bytearray()
    out += x.to_bytes(get_state_width(L, b), byteorder='big')
    out += bytes(stream)

    return bytes(out)

def build_slot_table(freq: dict, M: int) -> list[str]:
    slot_to_symbol = [''] * M

    for char, (f, c) in freq.items():
        for i in range(f):
            slot_to_symbol[c + i] = char

    return slot_to_symbol

def decode_title(data: bytes, freq: dict, M: int, L: int, b: int) -> str:
    state_width = get_state_width(L, b)

    x = int.from_bytes(data[:state_width], byteorder='big')
    stream = list(data[state_width:])

    slot_to_symbol = build_slot_table(freq, M)
    message = []

    stream = list(stream)

    while True:
        slot = x % M
        s = slot_to_symbol[slot]
        f, c = freq[s]
        x = (x // M) * f + slot - c

        while x < L and stream:
            x = x * b + stream.pop()

        message.append(s)

        if s == ':':
            break

    return ''.join(message)

def search_best_M(values: list[int]):
    best_M = values[0]
    best_compression = 0

    for M in values:
        g.build_title_tables(M)

        global t
        t = importlib.reload(t)

        compression = 0
        song_count = 0

        with open('dataset.txt', 'r') as file:
            for song in file:
                title = song.split(':')[0] + ':'

                try:
                    encoded = encode_title(title, t.rans_freq, M, M, 256)
                except ValueError:
                    continue

                song_count += 1
                compression += (len(title) - len(encoded)) / len(title)

        print(f'Compression for M={M}: {compression / song_count * 100:.2f}%')

        if compression / song_count > best_compression:
            best_M = M
            best_compression = compression / song_count

    print(f'Best M: {best_M} with {best_compression * 100:.2f}%')

if __name__ == '__main__':
    search_best_M([128, 256, 512, 1024, 2048, 4096, 8192, 16384])
