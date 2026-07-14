// This is a binary format that can express the same things as RTTTL
// but it is more efficient to send over UART.

// It uses rANS so we first need the probabilities of every possible character occuring in the data.
// Since only the start can contain chars for the title and the second section only control pairs
// we will use 2 different encoders using different character sets and probabilities.

// The control section will be merged into the Tone commands since not specifying a duration
// is the same as providing one if an entire note counts as a single value.

// Title
// 126 ASCII values + 1 terminator (':' cannot be used in the title in the text format)
// = 127 values

// Tone commands
// 880 (20..900) bpm values + (6 duration values * 4 octave values * 13 notes * (1 has dot + 1 no dot)) + 1 terminator
// 1505

// The dataset used for calculating the probabilities is the 10 000 Mixed Tunes 3 pack on:
// https://picaxe.com/rtttl-ringtones-for-tune-command/
