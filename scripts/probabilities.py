from collections import Counter

def get_title_probabilities(songs: list[str]) -> Counter:
    counter = Counter()

    for song in songs:
        title = song.split(':')[0] + ':'

        skip = False

        for char in title:
            if ord(char) > 127:
                skip = True
                break

        if not skip:
            counter.update(title)

    return counter

def get_command_probabilities(songs: list[str]) -> Counter:
    counter = Counter()

    for song in songs:
        # For type checker
        defaults, tone_commands = '', ''

        _, defaults, tone_commands = song.split(':')

        default_duration = '4'
        default_octave = '6'

        for default in defaults.split(','):
            value_type = default[0]
            value = default[2:]

            if value_type == 'd':
                default_duration = value
            elif value_type == 'o':
                default_octave = value

        processed_commands = []

        skip_song = False

        for tone_command in tone_commands.split(','):
            tone_command = tone_command.replace('_', '#').replace(' ', '')

            if '=' in tone_command:
                value_type = tone_command[0]
                value = tone_command[2:]

                if value_type == 'd':
                    default_duration = value

                    if default_duration not in ['1', '2', '4', '8', '16', '32']:
                        continue
                elif value_type == 'o':
                    default_octave = value

                    if default_octave not in ['4', '5', '6', '7']:
                        continue
                elif value_type == 'b':
                    processed_commands.append(tone_command)
            else:
                processed_command = ''

                i = 0

                duration = ''

                while i < len(tone_command) and tone_command[i].isdigit():
                    duration += tone_command[i]
                    i += 1

                duration = duration if duration else default_duration

                if duration not in ['1', '2', '4', '8', '16', '32']:
                    skip_song = True
                    break

                processed_command += duration

                # Tone
                if tone_command[i] not in ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'p']:
                    skip_song = True
                    break

                processed_command += tone_command[i]

                if tone_command[i] == 'p':
                    processed_commands.append(processed_command)
                    continue

                i += 1

                if i < len(tone_command) and tone_command[i] == '#':
                    processed_command += '#'
                    i += 1

                if i < len(tone_command) and tone_command[i] == '.':
                    processed_command += '.'
                    i += 1

                octave = ''

                while i < len(tone_command) and tone_command[i].isdigit():
                    octave += tone_command[i]
                    i += 1

                octave = octave if octave else default_octave

                if octave not in ['4', '5', '6', '7']:
                    skip_song = True
                    break

                processed_command += octave

                processed_commands.append(processed_command)

            if skip_song:
                continue

        counter.update(processed_commands)

    return counter

if __name__ == '__main__':
    with open('dataset.txt', 'r') as file:
        probabilities = get_title_probabilities([line for line in file])
        # probabilities = get_command_probabilities([line for line in file])

        for count in sorted(probabilities.items(), key=lambda x: x[1], reverse=True):
            print(count)

        print(len(probabilities))
