# Parses a series of tone commands and ignores errors

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
    decoded_commands = []

    for command in commands:
        assert command > 0

        if command < 882:
            decoded_commands.append(f'b={command + 20 - 1}')
        elif command < 888:
            decoded_commands.append(f'{DURATIONS[command - 882]}P')
        else:
            value = command - 888

            duration_index = value // 96
            value %= 96

            tone_index = value // 8 + 1
            value %= 8

            octave = value // 2 + 4

            has_dot = value % 2

            decoded_commands.append(
                f'{DURATIONS[duration_index]}{TONES[tone_index]}{"." if has_dot else ""}{octave}'
            )

    return decoded_commands

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

if __name__ == '__main__':
    encoded_commands = encode_tone_commands(['o=7', 'D', '5E', '32G#.7'])

    print(encoded_commands)
    print(decode_tone_commands(encoded_commands))
