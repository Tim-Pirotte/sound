import rans as r
import encode_title as et
import encode_tone_commands as etc
import titles_table as tt
import commands_table as ct

def cli_rtttl_encoder():
    title_encoder = r.Encoder(tt.titles_frequencies, tt.M, 256, 256)
    commands_encoder = r.Encoder(ct.commands_frequencies, ct.M, 65536, 256)

    while True:
        song = input('RTTTL song: ').strip()

        parts = song.split(':')

        if len(parts) != 3:
            print('Format: <title>:<control section>:<tone commands>')
            continue

        title, control_section, tone_commands = parts

        commands = control_section.split(',')
        commands.extend(tone_commands.split(','))

        title_data = [et.encode_char(c) for c in title]
        commands_data = etc.encode_tone_commands(commands)

        encoded = r.encode([title_data, commands_data], [title_encoder, commands_encoder])

        print('Encoded:')
        print(''.join(f'{x:02x}' for x in encoded))
        print(f'Compression: {(1 - (len(encoded) / len(song))) * 100:.2f}%')
        print('Decoded:')
        decoded = r.decode(encoded, [title_encoder, commands_encoder])
        print(f'{"".join([et.decode_char(c) for c in decoded[0]])}:{etc.decode_tone_commands(decoded[1])}')

if __name__ == '__main__':
    cli_rtttl_encoder()
