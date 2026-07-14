// This is a binary format that can express the same things as RTTTL
// but it is more efficient to send over UART.

// It uses rANS so we first need the probabilities of every possible character occuring in the data.
// Since only the start can contain chars for the title and the second section only control pairs
// we will use 2 different encoders using different character sets and probabilities.
// For simplicity we will currently use byte-packing instead of bit-packing

// The control section will be merged into the Tone commands since not specifying a duration
// is the same as providing one if an entire note counts as a single value.

// Title
// 126 ASCII values + 1 terminator (':' cannot be used in the title in the text format)
// = 127 values

// Tone commands
// 880 (20..900) bpm values + (6 duration values * 4 octave values * 12 notes * (1 has dot + 1 no dot)) + 6 duration values for pause + 1 terminator
// = 1463 values
//
// 2-byte encoding:
//   Terminator: 0 = 0
//   BPM: 1..880 = 1 + BPM - 20
//   Pauses: 881..886 = 881 + duration index
//   Notes: 887..1462 = 887 + duration index * 96 + note index * 8 + octave index * 2 + has dot

// The dataset used for calculating the probabilities is the 10 000 Mixed Tunes 3 pack on:
// https://picaxe.com/rtttl-ringtones-for-tune-command/
