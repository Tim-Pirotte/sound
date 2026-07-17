import math
import importlib
from collections import Counter

import rans as r
import titles_table as t
import probabilities as p

def get_state_width_bytes(L: int, b: int) -> int:
    max_value = L * b - 1
    bits_needed = max_value.bit_length()

    return math.ceil(bits_needed / 8)

def encode_title(title: str, freq: dict, M: int, L: int, b: int) -> bytes:
    assert len(title) >= 1 and title[-1] == ':'
    assert title.count(':') == 1

    return r.encode([ord(c) for c in title], freq, M, L, b)

def build_slot_table(freq: dict, M: int) -> list[int]:
    slot_to_symbol = [0] * M

    for char, (f, c) in freq.items():
        for i in range(f):
            slot_to_symbol[c + i] = char

    return slot_to_symbol

def decode_title(data: bytes, freq: dict, M: int, L: int, b: int) -> str:
    state_width = get_state_width_bytes(L, b)

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

        message.append(chr(s))

        if s == ord(':'):
            break

    return ''.join(message)

def grid_search_M_and_L(counter: Counter, M_values: list[int], L_factors: list[int]):
    best_M = M_values[0]
    best_L = L_factors[0]
    best_compression = -float('inf')

    for M in M_values:
        r.generate_frequency_tables(counter, M, 127, 'titles')

        global t
        t = importlib.reload(t)

        for k in L_factors:
            compression = 0
            song_count = 0

            with open('dataset.txt', 'r') as file:
                for song in file:
                    title = song.split(':')[0] + ':'

                    skip = False

                    for c in title:
                        if  ord(c) > 127:
                            skip = True
                            break

                    if skip:
                        continue

                    encoded = encode_title(title, t.titles_frequencies, M, M * k, 256)

                    assert title == decode_title(encoded, t.titles_frequencies, M, M * k, 256)

                    song_count += 1
                    compression += (len(title) - len(encoded)) / len(title)

            print(f'Compression for M={M}, L={M * k}: {compression / song_count * 100:.2f}%')

            if compression / song_count > best_compression:
                best_M = M
                best_L = M * k
                best_compression = compression / song_count

    print(f'Best M={best_M}, L={best_L} with {best_compression * 100:.2f}%')

if __name__ == '__main__':
    with open('dataset.txt', 'r') as file:
        lines = [line for line in file]

        counter = p.get_title_probabilities(lines)
        counter = Counter({ord(k): v for k, v in counter.items()})

    grid_search_M_and_L(
        counter,
        [128, 256, 512, 1024, 2048, 4096, 8192, 16384],
        [1, 2, 4, 8, 16, 32],
    )
