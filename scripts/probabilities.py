from collections import Counter

import tone_command_parser as t

def get_command_probabilities(songs: list[str]) -> Counter:
    counter = Counter()

    for song in songs:
        _, defaults, tone_commands = song.split(':')

        commands = defaults.split(',')
        commands.extend(tone_commands.split(','))

        parsed_commands = t.parse_tone_commands(commands)

        counter.update(parsed_commands)

    return counter

if __name__ == '__main__':
    with open('dataset.txt', 'r') as file:
        probabilities = get_command_probabilities([line for line in file])

        for count in sorted(probabilities.items(), key=lambda x: x[1], reverse=True):
            print(count)

        print(len(probabilities))
