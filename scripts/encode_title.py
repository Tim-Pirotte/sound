import importlib
from collections import Counter

import rans as r
import titles_table as t

def encode_char(c):
    assert ord(c) < 128
    assert c != ':'

    if ord(c) < 58:
        return ord(c)
    else:
        return ord(c) - 1

def decode_char(c):
    assert c < 127

    if c < 58:
        return chr(c)
    else:
        return chr(c + 1)

def rans_encode_title(title: str, frequency_table: dict, M: int, L: int, b: int) -> bytes:
    return r.encode([encode_char(c) for c in title], frequency_table, M, L, b)

def rans_decode_title(data: bytes, frequency_table: dict, M: int, L: int, b: int) -> str:
    message = r.decode(data, frequency_table, M, L, b)

    return ''.join([decode_char(c) for c in message])

def grid_search_M_and_L(counter: Counter, M_values: list[int], L_factors: list[int]):
    best_M = M_values[0]
    best_L = L_factors[0]
    best_compression = -float('inf')

    for M in M_values:
        r.generate_frequency_tables(counter, M, 126, 'titles')

        global t
        t = importlib.reload(t)

        for k in L_factors:
            compression = 0
            song_count = 0

            with open('dataset.txt', 'r') as file:
                for song in file:
                    title = song.split(':')[0]

                    skip = False

                    for c in title:
                        if  ord(c) > 127:
                            skip = True
                            break

                    if skip:
                        continue

                    encoded = rans_encode_title(title, t.titles_frequencies, M, M * k, 256)

                    assert title == rans_decode_title(encoded, t.titles_frequencies, M, M * k, 256)

                    song_count += 1
                    compression += (len(title) + 1 - len(encoded)) / (len(title) + 1)

            print(f'Compression for M={M}, L={M * k}: {compression / song_count * 100:.2f}%')

            if compression / song_count > best_compression:
                best_M = M
                best_L = M * k
                best_compression = compression / song_count

    print(f'Best M={best_M}, L={best_L} with {best_compression * 100:.2f}%')

def get_title_frequencies(songs: list[str]) -> Counter:
    counter = Counter()

    for song in songs:
        title = song.split(':')[0] + '\0'

        skip = False

        for char in title:
            if ord(char) > 127:
                skip = True
                break

        if not skip:
            counter.update([encode_char(c) for c in title])

    return counter

if __name__ == '__main__':
    with open('dataset.txt', 'r') as file:
        counter = get_title_frequencies([line for line in file])

    grid_search_M_and_L(
        counter,
        [128, 256, 512, 1024, 2048, 4096, 8192, 16384],
        [1, 2, 4, 8, 16, 32],
    )
