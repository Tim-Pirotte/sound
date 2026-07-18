import importlib
from collections import Counter

import rans as r
import commands_table as ct

DURATIONS = [1, 2, 4, 8, 16, 32]
TONES = ['p', 'a', 'a#', 'b', 'c', 'c#', 'd', 'd#', 'e', 'f', 'f#', 'g', 'g#']

class Settings:
    def __init__(self) -> None:
        self.default_octave = 6
        self.default_duration = 4
        self.bpm = 63

def encode_tone_commands(commands: list[str]) -> list[int]:
    settings = Settings()
    parsed_commands = []

    for command in commands:
        command = command.lower()

        if '=' in command:
            parse_control_pair(command, settings, parsed_commands)
        else:
            parse_note(command, settings, parsed_commands)

    return parsed_commands

def decode_tone_commands(commands: list[int]) -> list[str]:
    parsed_commands = []

    for command in commands:
        assert command > 0

        if command < 882:
            parsed_commands.append(f'b={command + 20 - 1}')
        elif command < 888:
            parsed_commands.append(f'{DURATIONS[command - 882]}P')
        else:
            value = command - 888

            duration_index = value // 96
            value %= 96

            tone_index = value // 8 + 1
            value %= 8

            octave = value // 2 + 4

            has_dot = value % 2

            parsed_commands.append(
                f'{DURATIONS[duration_index]}{TONES[tone_index]}{"." if has_dot else ""}{octave}'
            )

    return parsed_commands

def rans_encode_tone_commands(commands: list[str], L: int, b: int) -> bytes:
    parsed_commands = encode_tone_commands(commands)

    return r.encode(parsed_commands, ct.commands_frequencies, ct.M, L, b)

def rans_decode_tone_commands(commands: bytes, frequency_table: dict, M: int, L: int, b: int) -> list[str]:
    parsed_commands = r.decode(commands, frequency_table, M, L, b)

    return decode_tone_commands(parsed_commands)

def parse_control_pair(control_pair: str, settings: Settings, parsed_commands: list[int]):
    name, value = control_pair.split('=')

    try:
        value = int(value)
    except ValueError:
        return

    if name == 'o':
        if 4 <= value <= 7:
            settings.default_octave = value
    elif name == 'd':
        if value in DURATIONS:
            settings.default_duration = value
    elif name == 'b':
        if 20 <= value <= 900:
            settings.bpm = value
            parsed_commands.append(1 + value - 20)

def parse_note(note: str, settings: Settings, parsed_commands: list[int]):
    i = 0

    duration = 0

    while i < len(note) and note[i].isdigit():
        duration = duration * 10 + int(note[i])
        i += 1

    if duration not in DURATIONS:
        duration = settings.default_duration

    tone = 'p'
    max_len = 0

    for t in TONES:
        if note[i:i+len(t)] == t and len(t) > max_len:
            tone = t
            max_len = len(t)

    i += max_len

    has_dot = False

    if i < len(note) and note[i] == '.':
        has_dot = True
        i += 1

    octave = 0

    while i < len(note) and note[i].isdigit():
        octave = octave * 10 + int(note[i])
        i += 1

    if  not (4 <= octave <= 7):
        octave = settings.default_octave

    if tone == 'p':
        encoded_command = 882 + DURATIONS.index(duration)
    else:
        encoded_command = 888 + DURATIONS.index(duration) * 96 + (TONES.index(tone) - 1) * 8 + (octave - 4) * 2 + has_dot

    parsed_commands.append(encoded_command)

def grid_search_M_and_L(counter: Counter, M_values: list[int], L_factors: list[int]):
    best_M = M_values[0]
    best_L = L_factors[0]
    best_compression = -float('inf')

    for M in M_values:
        r.generate_frequency_tables(counter, M, 1463, 'commands')

        global ct
        ct = importlib.reload(ct)

        for k in L_factors:
            compression = 0
            song_count = 0

            with open('dataset.txt', 'r') as file:
                for song in file:
                    _, defaults, tone_commands = song.split(':')

                    commands = defaults.split(',')
                    commands.extend(tone_commands.split(','))

                    encoded = rans_encode_tone_commands(commands, M * k, 256)

                    song_count += 1
                    str_len = len(defaults) + len(':') + len(tone_commands) + len(':')
                    compression += (str_len - len(encoded)) / str_len

            print(f'Compression for M={M}, L={M * k}: {compression / song_count * 100:.2f}%')

            if compression / song_count > best_compression:
                best_M = M
                best_L = M * k
                best_compression = compression / song_count

    r.generate_frequency_tables(counter, best_M, 1463, 'commands')

    print(f'Best M={best_M}, L={best_L} with {best_compression * 100:.2f}%')

def get_command_frequencies(songs: list[str]) -> Counter:
    counter = Counter()

    for song in songs:
        _, defaults, tone_commands = song.split(':')

        commands = defaults.split(',')
        commands.extend(tone_commands.split(','))

        parsed_commands = encode_tone_commands(commands)

        counter.update(parsed_commands)

    return counter

if __name__ == '__main__':
    with open('dataset.txt', 'r') as file:
        counter = get_command_frequencies([line for line in file])

    grid_search_M_and_L(
        counter,
        [2048, 4096, 8192, 16384],
        [1, 2, 4, 8, 16, 32],
    )
