def cli_rtttl_encoder():
    while True:
        song = input('RTTTL song:').strip()

        parts = song.split(':')

        if len(parts) != 3:
            print('Format: <title>:<control section>:<tone commands>')
            continue

        title, control_section, tone_commands = parts



if __name__ == '__main__':
    cli_rtttl_encoder()