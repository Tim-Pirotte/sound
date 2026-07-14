from collections import Counter

def print_title_probabilities(songs: list[str]):
    counter = Counter()

    for song in songs:
        title = song.split(':')[0] + ':'

        for char in title:
            counter.update(char)

    print(counter)

if __name__ == '__main__':
    print_title_probabilities([])
