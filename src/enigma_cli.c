
/**
 *
 * MIT License
 * 
 * Copyright (c) 2025 Henrik A. Glass
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 *
 * ABOUT:
 * 
 * Enigma-cli implements a simulation of the Enigma M3 cipher machine used by
 * the German military to send secret messages during WWII. Enimga-cli is usable
 * entirely from the command-line and accepts input on STDIN.
 *
 *
 * USAGE:
 *
 *     Usage: ./enigma-cli [Options]
 *     Options:
 *       -u,--reflector,--umkehrwalze                     Reflector (Ger: Umkehrwalze) (default = "UKW-B")
 *       -w,--rotors,--walzenlage                         Rotor order (Ger: Walzenlage) (default = "I II III")
 *       -r,--ring-setting,--ringstellung                 Ring setting (Ger: Ringstellung) (default = "1 1 1")
 *       -s,--plugboard-setting,--steckerverbindungen     Plugboard transpositions (Ger: Steckerverbindungen) (default = "")
 *       -g,--indicator-setting,--grundstellung           Indicator setting (Ger: Grundstellung) (default = "1 1 1")
 *       --help,--hilfe                                   Displays this message (default = 0)
 *
 * EXAMPLE:
 *
 * To set up the machine according to the "Armee-Stabs-Maschinenschlüssel Nr.28"
 * procedures for October 1st 1944, assuming the machine is fitted with an UKW-C
 * reflector, and assuming an operator-chosen initial indicator setting "HAG",
 * run enimga-cli like this:
 *
 *     $ ./enigma-cli -u "UKW-C" -w "II IV I" -r "6 17 26" -s "AC LS BQ WN MY UV FJ PZ TR OK" -g "HAG"
 *
 * Link to procedures sheet: 
 *
 * https://web.archive.org/web/20250606093439/https://www.ciphermachinesandcryptology.com/img/enigma/hires-wehrmachtkey-stab.jpg
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

/*--- Include files ---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* 
 * Note: Hack/workaround for unit testing. The test program includes this source file 
 *       immediately after including hgl_test.h. hgl_test.h, in turn, includes the 
 *       implementation of hgl_flags.h, meaning we can't include it here.
 */
#ifndef HGL_TEST_H
#  define HGL_FLAGS_PRINT_MARGIN 48
#  define HGL_FLAGS_IMPLEMENTATION
#  include "hgl_flags.h"
#endif

#define HGL_STRING_IMPLEMENTATION
#include "hgl_string.h"

/*--- Private macros --------------------------------------------------------------------*/

#define ENIGMA_ASSERT(cond, ...)                \
    if (!(cond)) {                              \
        fprintf(stderr, "Error: " __VA_ARGS__); \
        fprintf(stderr, "\n");                  \
        exit(1);                                \
    }
#define ENIGMA_ERROR(...)                       \
    do {                                        \
        fprintf(stderr, "Error: " __VA_ARGS__); \
        fprintf(stderr, "\n");                  \
        exit(1);                                \
    } while (0)

#define ENCODE(c) ((c) - 'A') // maps A-Z  --> 0-25
#define DECODE(n) ((n) + 'A') // maps 0-25 --> A->Z

#define SCRATCH_BUF_SIZE (16 * 1024 * 1024)

/*--- Private type definitions ----------------------------------------------------------*/

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t    i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef enum {
    FORWARD,
    REVERSE,
} Direction;

/**
 * Represents a substitution on the set of letters in the enigma alphabet.
 * `image[26]` represents the image of the substitution in alphabetical 
 * order. I.e. using Cauchy's two-line notation:
 *
 *     | ABCDEFGHIJKLMNOPQRSTUVWXYZ |
 *     |         image[26]          |
 *
 * E.g. a ROT13 cipher would look like this:
 *
 *     | ABCDEFGHIJKLMNOPQRSTUVWXYZ |
 *     | NOPQRSTUVWXYZABCDEFGHIJKLM |
 *
 */
typedef struct {
    u8 image[26];
} Substitution;

/* 
 * Represents an Enigma machine rotor including its ring setting and 
 * position (even though, strictly speaking, the rotor position is not
 * an attribute of the actual rotor).
 */
typedef struct {
    Substitution forward;
    Substitution reverse;
    u8 turnover1;
    u8 turnover2; /* only valid for rotors VI, VII, and VIII */
    u8 ring_setting;
    u8 position;
} Rotor;

/* 
 * Represents an Enigma M3 machine.
 *
 * Note: Rotors are indexed from left to right as seen from the perspective of the 
 *       machine operator; i.e. rotor[0] is the leftmost (slow) rotor, and rotor[2] 
 *       is the rightmost (fast) rotor.
 */
typedef struct {
    Rotor rotor[3];
    Substitution reflector;
    Substitution plugboard;
} Enigma;

/*--- Private constants -----------------------------------------------------------------*/

/**
 * These are the 8 different rotor supplied with the Enigma M3. The last three rotors were
 * only used by the german navy (Kriegsmarine) and have, whilst the the first five were used
 * by all branches of the German Forces, including the navy. The navy's rotors (VI, VII, and 
 * VII) are special because they have two turnover notches, meaning for every complete 
 * revolution of such a rotor, the rotor to the left of it will have stepped at least twice (
 * I write "at least" here, because of the peculiar double-stepping quirk of the Enigma).
 */
static const Rotor ROTOR_I    = {{"EKMFLGDQVZNTOWYHXUSPAIBRCJ"}, {"UWYGADFPVZBECKMTHXSLRINQOJ"}, ENCODE('Q'), 255 /* no second turnover */, 0, 0};
static const Rotor ROTOR_II   = {{"AJDKSIRUXBLHWTMCQGZNPYFVOE"}, {"AJPCZWRLFBDKOTYUQGENHXMIVS"}, ENCODE('E'), 255 /* no second turnover */, 0, 0};
static const Rotor ROTOR_III  = {{"BDFHJLCPRTXVZNYEIWGAKMUSQO"}, {"TAGBPCSDQEUFVNZHYIXJWLRKOM"}, ENCODE('V'), 255 /* no second turnover */, 0, 0};
static const Rotor ROTOR_IV   = {{"ESOVPZJAYQUIRHXLNFTGKDCMWB"}, {"HZWVARTNLGUPXQCEJMBSKDYOIF"}, ENCODE('J'), 255 /* no second turnover */, 0, 0};
static const Rotor ROTOR_V    = {{"VZBRGITYUPSDNHLXAWMJQOFECK"}, {"QCYLXWENFTZOSMVJUDKGIARPHB"}, ENCODE('Z'), 255 /* no second turnover */, 0, 0};
static const Rotor ROTOR_VI   = {{"JPGVOUMFYQBENHZRDKASXLICTW"}, {"SKXQLHCNWARVGMEBJPTYFDZUIO"}, ENCODE('Z'), ENCODE('M'), 0, 0};
static const Rotor ROTOR_VII  = {{"NZJHGRCXMYSWBOUFAIVLPEKQDT"}, {"QMGYVPEDRCWTIANUXFKZOSLHJB"}, ENCODE('Z'), ENCODE('M'), 0, 0};
static const Rotor ROTOR_VIII = {{"FKQHTLXOCBJSPDZRAMEWNIUYGV"}, {"QJINSAYDVKBFRUHMCPLEWZTGXO"}, ENCODE('Z'), ENCODE('M'), 0, 0};

/**
 * These are three reflectors (Umkehrwalze) used in various versions of the Enigma machine.
 * Typically, for the M3 variant, either UKW-B or UKW-C were used. Later in the war, the
 * germans developed the UKW-D rewireable reflector. I'll probably add this in the future
 * when I'm bored. Info: https://www.cryptomuseum.com/crypto/enigma/ukwd/
 */
static const Substitution UKW_A = {"EJMZALYXVBWFCRQUONTSPIKHGD"};
static const Substitution UKW_B = {"YRUHQSLDPXNGOKMIEBFZCWVJAT"};
static const Substitution UKW_C = {"FVPJIAOYEDRZXWGCTKUQSBNMHL"};

/**
 * By default, with no connections made at the plugboard, no substitutions are made.
 */
static const Substitution BARE_PLUGBOARD = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};

/*--- Function prototypes ---------------------------------------------------------------*/

/* Basic interface */
int enigma_cli_main(int argc, char *argv[]);
size_t encipher_str(Enigma *enigma, char *output, const char *input);
u8 encipher_char(Enigma *enigma, char c);

/* Machine setup */
static void apply_reflector_setting(Enigma *enigma, const char *str);
static void apply_rotor_setting(Enigma *enigma, const char *str);
static void apply_ring_setting(Enigma *enigma, const char *str);
static void apply_plugboard_setting(Enigma *enigma, const char *str);
static void apply_indicator_setting(Enigma *enigma, const char *str);

/* Machine logic */
static bool is_at_turnover(const Rotor *r);
static void step_rotor(Rotor *r);
static u8 apply_rotor_subst(const Rotor *r, Direction dir, u8 n);
static u8 apply_subst(const Substitution *s, u8 n);

/* Helpers */
static size_t lex_numeric(HglStringView sv);
static size_t lex_letter(HglStringView sv);
static char to_upper(char c);
static char in_alphabet(char c);

/*--- Enigma functions ------------------------------------------------------------------*/

/**
 * Consider this the entry point of the program. See the actual main function for more info.
 */
int enigma_cli_main(int argc, char *argv[])
{
    /* Enigma machine simulation settings */
    const char **opt_reflector_setting = hgl_flags_add_str("-u,--reflector,--umkehrwalze", "Reflector (Ger: Umkehrwalze)", "UKW-B", 0);
    const char **opt_rotor_setting     = hgl_flags_add_str("-w,--rotors,--walzenlage", "Rotor order (Ger: Walzenlage)", "I II III", 0);
    const char **opt_ring_setting      = hgl_flags_add_str("-r,--ring-setting,--ringstellung", "Ring setting (Ger: Ringstellung)", "1 1 1", 0);
    const char **opt_plugboard_setting = hgl_flags_add_str("-s,--plugboard-setting,--steckerverbindungen", "Plugboard transpositions (Ger: Steckerverbindungen)", "", 0);
    const char **opt_indicator_setting = hgl_flags_add_str("-g,--indicator-setting,--grundstellung", "Indicator setting (Ger: Grundstellung)", "1 1 1", 0);

    /* Enigma-cli general settings */
    bool *opt_help           = hgl_flags_add_bool("--help,--hilfe", "Displays this message", false, 0);
    u64 *opt_group_size      = hgl_flags_add_u64_range("-G,--group-size", "Number of characters per group in the output.", 5, 0, 1, 64);
    u64 *opt_groups_per_line = hgl_flags_add_u64_range("-N,--groups-per-line", "Number of groups per line in the output.", 6, 0, 1, 64);

    /* Parse arguments */
    int err = hgl_flags_parse(argc, argv);
    if (err != 0) {
        printf("Usage: %s [Options]\n", argv[0]);
        hgl_flags_print();
        return 1;
    }
    if (*opt_help) {
        printf("Usage: %s [Options]\n", argv[0]);
        hgl_flags_print();
        return 0;
    }
    
    /* Configure enigma */
    Enigma enigma = {0};
    apply_reflector_setting(&enigma, *opt_reflector_setting);
    apply_rotor_setting(&enigma, *opt_rotor_setting);
    apply_ring_setting(&enigma, *opt_ring_setting);
    apply_plugboard_setting(&enigma, *opt_plugboard_setting);
    apply_indicator_setting(&enigma, *opt_indicator_setting);

    /* encipher/decipher from stdin */
    static u8 input[SCRATCH_BUF_SIZE] = {0};
    static u8 output[SCRATCH_BUF_SIZE] = {0};
    size_t n_read_bytes = read(0, input, SCRATCH_BUF_SIZE);
    if (n_read_bytes <= 0) {
        return 1;
    }
    size_t output_size = encipher_str(&enigma, (char *) output, (char *) input);

    /* Pretty-print result */
    size_t group_size = *opt_group_size;
    size_t groups_per_row = *opt_groups_per_line;
    size_t row_size = group_size * groups_per_row;
    size_t rows = output_size / (group_size * groups_per_row) + 1;
    for (size_t row = 0; row < rows; row++) {
        for (size_t group = 0; group < groups_per_row; group++) {
            size_t idx = row * row_size + group * group_size;
            if (idx > output_size) break;
            printf("%.*s ", (int) group_size, &output[idx]);
        }
        printf("\n");
    }

    return 0;
}

/**
 * Enciphers (or deciphers) the string at `input`, places the result into `output`, and
 * returns the length of the enciphered (or deciphered) output. 
 */
size_t encipher_str(Enigma *enigma, char *output, const char *input)
{
    char *wr = output;

    char c;
    do {
        c = *input++;
        c = to_upper(c);

        /* Skip unrecognized letters */
        if (!in_alphabet(c)) {
            continue;
        }

        /* encipher (or decipher ... transcipher? cipher?) character */
        *wr++ = encipher_char(enigma, c);
    } while (c != '\0');

    return wr - output;
}


/**
 * Enciphers (or deciphers) a single character (or letter) `c` given the current machine
 * settings and updates the rotor positions accordingly. 
 *
 * NB:`c` must be in the enigma alphabet and upper-case. 
 */
u8 encipher_char(Enigma *enigma, char c)
{
    /* 1. advance rotors */
    if (is_at_turnover(&enigma->rotor[1])) { 
        step_rotor(&enigma->rotor[0]);
        step_rotor(&enigma->rotor[1]);
    } else if (is_at_turnover(&enigma->rotor[2])) {
        step_rotor(&enigma->rotor[1]);
    }
    step_rotor(&enigma->rotor[2]);

    /* 2. encipher character */
    u8 n = ENCODE(c); 
    n = apply_subst(&enigma->plugboard, n);
    n = apply_rotor_subst(&enigma->rotor[2], FORWARD, n);
    n = apply_rotor_subst(&enigma->rotor[1], FORWARD, n);
    n = apply_rotor_subst(&enigma->rotor[0], FORWARD, n);
    n = apply_subst(&enigma->reflector, n);
    n = apply_rotor_subst(&enigma->rotor[0], REVERSE, n);
    n = apply_rotor_subst(&enigma->rotor[1], REVERSE, n);
    n = apply_rotor_subst(&enigma->rotor[2], REVERSE, n);
    n = apply_subst(&enigma->plugboard, n);
    return DECODE(n);
}

/**
 * Mounts the given reflector ("Umkehrwalze") to the machine.
 */
static void apply_reflector_setting(Enigma *enigma, const char *str)
{
    HglStringView sv = hgl_sv_from_cstr(str);
    if (hgl_sv_equals(sv, HGL_SV("UKW-A"))) {
        enigma->reflector = UKW_A;
    } else if (hgl_sv_equals(sv, HGL_SV("UKW-B"))) {
        enigma->reflector = UKW_B;
    } else if (hgl_sv_equals(sv, HGL_SV("UKW-C"))) {
        enigma->reflector = UKW_C;
    } else {
        ENIGMA_ASSERT(false, "Unknown reflector \"" HGL_SV_FMT "\".", HGL_SV_ARG(sv));
    }
}

/**
 * Mounts the given rotors (e.g. "I VI II", from left to right, as seen from the 
 * machine operator) in the machine ("Walzenlage"). 
 *
 * NB: This will reset any earlier applied ring- and indicator settings.
 *
 */
static void apply_rotor_setting(Enigma *enigma, const char *str)
{
    HglStringView sv = hgl_sv_from_cstr(str);
    for (int i = 0; i < 3; i++) {
        HglStringView r = hgl_sv_trim(hgl_sv_lchop_until(&sv, ' '));
        if (hgl_sv_equals(r, HGL_SV("I"))) {
            enigma->rotor[i] = ROTOR_I;
        } else if (hgl_sv_equals(r, HGL_SV("II"))) {
            enigma->rotor[i] = ROTOR_II;
        } else if (hgl_sv_equals(r, HGL_SV("III"))) {
            enigma->rotor[i] = ROTOR_III;
        } else if (hgl_sv_equals(r, HGL_SV("IV"))) {
            enigma->rotor[i] = ROTOR_IV;
        } else if (hgl_sv_equals(r, HGL_SV("V"))) {
            enigma->rotor[i] = ROTOR_V;
        } else if (hgl_sv_equals(r, HGL_SV("VI"))) {
            enigma->rotor[i] = ROTOR_VI;
        } else if (hgl_sv_equals(r, HGL_SV("VII"))) {
            enigma->rotor[i] = ROTOR_VII;
        } else if (hgl_sv_equals(r, HGL_SV("VIII"))) {
            enigma->rotor[i] = ROTOR_VIII;
        } else {
            ENIGMA_ASSERT(false, "Unknown rotor \"" HGL_SV_FMT "\".", HGL_SV_ARG(r));
        }
    }
}

/**
 * Applies an indicator (or "Ringstellung") setting (e.g. "ABC" or "1 2 3") to 
 * the currently mounted rotors.
 */
static void apply_ring_setting(Enigma *enigma, const char *str)
{
    HglStringView sv = hgl_sv_from_cstr(str);
    for (int i = 0; i < 3; i++) {
        HglStringView l;
        sv = hgl_sv_trim(sv);
        l = hgl_sv_lchop_lexeme(&sv, lex_numeric);
        if (l.length != 0) {
            enigma->rotor[i].ring_setting = (u8) hgl_sv_to_u64(l) - 1;
            continue;
        }
        l = hgl_sv_lchop_lexeme(&sv, lex_letter);
        if (l.length != 0) {
            enigma->rotor[i].ring_setting = ENCODE(to_upper(l.start[0]));
            continue;
        }
        ENIGMA_ERROR("Invalid ring setting \"%s\".", str);
    }
    ENIGMA_ASSERT(hgl_sv_trim(sv).length == 0, "Invalid ring setting \"%s\".", str);
}

/**
 * Applies a plugboard (or "Steckerverbindungen") setting (e.g. "ab cd ef gh") 
 * to the machine.
 */
static void apply_plugboard_setting(Enigma *enigma, const char *str)
{
    enigma->plugboard = BARE_PLUGBOARD;
    u8 wiring[26] = {0};
    HglStringView sv = hgl_sv_from_cstr(str);
    while (sv.length > 0) {
        /* grab next pair */
        HglStringView pair = hgl_sv_trim(hgl_sv_lchop_until(&sv, ' '));
        ENIGMA_ASSERT(pair.length == 2, "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "String should be formatted as \"ab cd ef ...\"", HGL_SV_ARG(pair));

        char c0 = to_upper(pair.start[0]);
        char c1 = to_upper(pair.start[1]);
        u8 n0 = ENCODE(c0);
        u8 n1 = ENCODE(c1);

        ENIGMA_ASSERT(c0 != c1, "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "A character can not be swapped with itself", HGL_SV_ARG(pair));
        ENIGMA_ASSERT(in_alphabet(c0), "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "Character '%c' is not in the Enigma alphabet", HGL_SV_ARG(pair), c0);
        ENIGMA_ASSERT(in_alphabet(c1), "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "Character '%c' is not in the Enigma alphabet", HGL_SV_ARG(pair), c1);
        ENIGMA_ASSERT(wiring[n0] == 0, "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "The character '%c' has already been used.", HGL_SV_ARG(pair), c0);
        ENIGMA_ASSERT(wiring[n1] == 0, "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "The character '%c' has already been used.", HGL_SV_ARG(pair), c1);

        /* apply wiring */
        wiring[n0] = c1;
        wiring[n1] = c0;
        enigma->plugboard.image[n0] = c1;
        enigma->plugboard.image[n1] = c0;
    }
}

/**
 * Applies an indicator (or "Grundstellung") setting (e.g. "ABC" or "1 2 3") to 
 * the currently mounted rotors.
 */
static void apply_indicator_setting(Enigma *enigma, const char *str)
{
    HglStringView sv = hgl_sv_from_cstr(str);
    for (int i = 0; i < 3; i++) {
        HglStringView l;
        sv = hgl_sv_trim(sv);
        l = hgl_sv_lchop_lexeme(&sv, lex_numeric);
        if (l.length != 0) {
            enigma->rotor[i].position = (u8) hgl_sv_to_u64(l) - 1;
            continue;
        }
        l = hgl_sv_lchop_lexeme(&sv, lex_letter);
        if (l.length != 0) {
            enigma->rotor[i].position = ENCODE(to_upper(l.start[0]));
            continue;
        }
        ENIGMA_ERROR("Invalid indicator setting \"%s\".", str);
    }
    ENIGMA_ASSERT(hgl_sv_trim(sv).length == 0, "Invalid indicator setting \"%s\".", str);
}

/**
 * Returns true if rotor `r` is positioned at a turnover notch.
 */
static bool is_at_turnover(const Rotor *r)
{
    return (r->position == r->turnover1) ||
           (r->position == r->turnover2);
}

/**
 * Steps rotor `r` once.
 */
static void step_rotor(Rotor *r)
{
    r->position = (r->position + 1) % 26;
}

/**
 * Returns the image of 'n' under the substitution given by the rotor `r` (including rotor 
 * position and ring setting) and the current flow direction `dir` through the rotor, where
 * `n` is the numerical encoding of a letter in the Enigma alphabet.
 */
static u8 apply_rotor_subst(const Rotor *r, Direction dir, u8 n)
{
    n = (n - r->ring_setting + r->position + 26) % 26;
    switch (dir) {
        case FORWARD: n = apply_subst(&r->forward, n); break;
        case REVERSE: n = apply_subst(&r->reverse, n); break;
    }
    n = (n + r->ring_setting - r->position + 26) % 26;
    return n;
}

/**
 * Applies substitution `s` to `n`.
 */
static u8 apply_subst(const Substitution *s, u8 n)
{
    return ENCODE(s->image[n]);
}

/**
 * Lexer rule which matches the numerical encodings of the letters from the Enigma alphabet.
 */
static size_t lex_numeric(HglStringView sv)
{
    if (sv.length == 0) {
        return 0;
    }
    size_t original_length = sv.length;
    if (sv.start[0] < '1' || sv.start[0] > '9') return 0;
    u64 value = hgl_sv_lchop_u64(&sv);
    if (value < 1 || value > 26) {
        return 0;
    }
    return original_length - sv.length;
}

/**
 * Lexer rule which matches the letters from the Enigma alphabet.
 */
static size_t lex_letter(HglStringView sv)
{
    if (sv.length == 0) {
        return 0;
    }
    char c = to_upper(sv.start[0]);
    if (c >= 'A' && c <= 'Z') {
        return 1;
    }
    return 0;
}

/**
 * Returns the uppercase of `c`.
 */
static char to_upper(char c)
{
    return (c >= 'a' && c <= 'z') ? (c - 0x20) : c;
}

/**
 * Returns true if `c` is in the Enigma alphabet.
 */
static char in_alphabet(char c)
{
    return c >= 'A' && c <= 'Z';
}


/*--- Main function ---------------------------------------------------------------------*/

/* 
 * Note: This main function omitted when we're building the test program, as the hgl_test.h
 *       unit testing library includes its own main function.
 */
#ifndef HGL_TEST_H
int main(int argc, char *argv[])
{ 
    return enigma_cli_main(argc, argv);
} 
#endif

