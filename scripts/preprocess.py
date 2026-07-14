# This file merges the 10 000 songs from the dataset into a single file

import os

def preprocess():
    dataset = ''

    for file_name in [f for f in os.listdir('data') if os.path.isfile(f'data/{f}')]:
        with open(f'data/{file_name}', 'r') as file:
            for line in file:
                dataset += line.strip() + '\n'

    with open('dataset.txt', 'x') as file:
        file.write(dataset)


if __name__ == '__main__':
    preprocess()
