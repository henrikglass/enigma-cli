
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
#  define HGL_FLAGS_IMPLEMENTATION
#  include "hgl_flags.h"
#endif

#define HGL_STRING_IMPLEMENTATION
#include "hgl_string.h"

/*--- Private macros --------------------------------------------------------------------*/

#define C2N(c) ((c) - 'A') // maps A-Z  --> 0-25
#define N2C(n) ((n) + 'A') // maps 0-25 --> A->Z
#define TO_UPPER(c) (((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c))
#define IN_ALPHABET(c) ((c) >= 'A' && (c) <= 'Z') 
#define ENIGMA_ASSERT(cond, ...)               \
    if (!(cond)) {                             \
        fprintf(stderr, "Error:" __VA_ARGS__); \
        fprintf(stderr, "\n");                 \
        exit(1);                               \
    }
#define ENIGMA_ERROR(...)                      \
    do {                                       \
        fprintf(stderr, "Error:" __VA_ARGS__); \
        fprintf(stderr, "\n");                 \
        exit(1);                               \
    } while (0)

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
typedef float    f32;
typedef double   f64;

/* just in case... */
static_assert(sizeof(float) == 4, "");
static_assert(sizeof(double) == 8, "");

typedef enum {
    FORWARD,
    REVERSE,
} Direction;

/**
 * Represents a permutation on the set of letters in the enigma alphabet.
 * `image[26]` represents the image of the permutation in alphabetical 
 * order. I.e. using Cauchy's two-line notation:
 *
 *     | ABCDEFGHIJKLMNOPQRSTUVWXYZ |
 *     |         image[26]          |
 *
 */
typedef struct {
    u8 image[26];
} Permutation;

typedef struct {
    Permutation forward;
    Permutation reverse;
    u8 turnover1;
    u8 turnover2; /* only valid for rotors VI, VII, and VIII */
    u8 ring_setting;
    u8 position;
} Rotor;

/* 
 * Note: Rotors are indexed from left to right as seen from the perspective of the 
 *       machine operator; i.e. rotor[0] is the leftmost (slow) rotor, and rotor[2] 
 *       is the rightmost (fast) rotor.
 */
typedef struct {
    Rotor rotor[3];
    Permutation reflector;
    Permutation plugboard;
} Enigma;

/*--- Private constants -----------------------------------------------------------------*/

static const Rotor ROTOR_I    = {{"EKMFLGDQVZNTOWYHXUSPAIBRCJ"}, {"UWYGADFPVZBECKMTHXSLRINQOJ"}, C2N('Q'), 255 /* no second turnover */, 0, 0};
static const Rotor ROTOR_II   = {{"AJDKSIRUXBLHWTMCQGZNPYFVOE"}, {"AJPCZWRLFBDKOTYUQGENHXMIVS"}, C2N('E'), 255 /* no second turnover */, 0, 0};
static const Rotor ROTOR_III  = {{"BDFHJLCPRTXVZNYEIWGAKMUSQO"}, {"TAGBPCSDQEUFVNZHYIXJWLRKOM"}, C2N('V'), 255 /* no second turnover */, 0, 0};
static const Rotor ROTOR_IV   = {{"ESOVPZJAYQUIRHXLNFTGKDCMWB"}, {"HZWVARTNLGUPXQCEJMBSKDYOIF"}, C2N('J'), 255 /* no second turnover */, 0, 0};
static const Rotor ROTOR_V    = {{"VZBRGITYUPSDNHLXAWMJQOFECK"}, {"QCYLXWENFTZOSMVJUDKGIARPHB"}, C2N('Z'), 255 /* no second turnover */, 0, 0};
static const Rotor ROTOR_VI   = {{"JPGVOUMFYQBENHZRDKASXLICTW"}, {"SKXQLHCNWARVGMEBJPTYFDZUIO"}, C2N('Z'), C2N('M'), 0, 0};
static const Rotor ROTOR_VII  = {{"NZJHGRCXMYSWBOUFAIVLPEKQDT"}, {"QMGYVPEDRCWTIANUXFKZOSLHJB"}, C2N('Z'), C2N('M'), 0, 0};
static const Rotor ROTOR_VIII = {{"FKQHTLXOCBJSPDZRAMEWNIUYGV"}, {"QJINSAYDVKBFRUHMCPLEWZTGXO"}, C2N('Z'), C2N('M'), 0, 0};

static const Permutation UKW_A = {"EJMZALYXVBWFCRQUONTSPIKHGD"};
static const Permutation UKW_B = {"YRUHQSLDPXNGOKMIEBFZCWVJAT"};
static const Permutation UKW_C = {"FVPJIAOYEDRZXWGCTKUQSBNMHL"};

static const Permutation BARE_PLUGBOARD = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};

/*--- Function prototypes ---------------------------------------------------------------*/

int enigma_cli_main(int argc, char *argv[]);
size_t encrypt_str(Enigma *enigma, char *output, const char *input);
u8 encrypt_char(Enigma *enigma, char c);

static void apply_reflector_setting(Enigma *enigma, const char *str);
static void apply_rotor_setting(Enigma *enigma, const char *str);
static void apply_ring_setting(Enigma *enigma, const char *str);
static void apply_plugboard_setting(Enigma *enigma, const char *str);
static void apply_indicator_setting(Enigma *enigma, const char *str);

static bool is_at_turnover(const Rotor *r);
static void step_rotor(Rotor *r);
static u8 rotor_pass(const Rotor *r, u8 n, Direction dir);
static size_t lex_numeric(HglStringView sv);
static size_t lex_letter(HglStringView sv);

/*--- Enigma functions ------------------------------------------------------------------*/

int enigma_cli_main(int argc, char *argv[])
{
    /* Enigma machine simulation settings */
    const char **opt_reflector_setting = hgl_flags_add_str("-u,--reflector,--umkehrwalze", "Reflector (Ger: Umkehrwalze)", "UKW-B", 0);
    const char **opt_rotor_setting     = hgl_flags_add_str("-w,--rotors,--walzenlage", "Rotor order (Ger: Walzenlage)", "I II III", 0);
    const char **opt_ring_setting      = hgl_flags_add_str("-r,--ring-setting,--ringstellung", "Ring setting (Ger: Ringstellung)", "1 1 1", 0);
    const char **opt_plugboard_setting = hgl_flags_add_str("-s,--plugboard-setting,--steckerverbindungen", "Plugboard transpositions (Ger: Steckerverbindungen)", "", 0);
    const char **opt_indicator_setting = hgl_flags_add_str("-g,--indicator-setting,--grundstellung", "Indicator setting (Ger: Grundstellung)", "1 1 1", 0);

    /* Enigma-cli general settings */
    bool *opt_help              = hgl_flags_add_bool("--help,--hilfe", "Displays this message", false, 0);

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

    /* encrypt/decrypt from stdin */
    static u8 input[SCRATCH_BUF_SIZE] = {0};
    static u8 output[SCRATCH_BUF_SIZE] = {0};
    size_t n_read_bytes = read(0, input, SCRATCH_BUF_SIZE);
    if (n_read_bytes <= 0) {
        return 1;
    }
    size_t output_size = encrypt_str(&enigma, (char *) output, (char *) input);

    /* print result */
    size_t group_size = 5;
    size_t groups_per_row = 6;
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
 * Encrypts (or decrypts) the string at `input`, places the result into `output`, and
 * returns the length of the encrypted (or decrpyted) output. 
 */
size_t encrypt_str(Enigma *enigma, char *output, const char *input)
{
    char *wr = output;

    char c;
    do {
        c = *input++;
        c = TO_UPPER(c);

        /* Skip unrecognized letters */
        if (!IN_ALPHABET(c)) {
            continue;
        }

        /* encrypt (or decrypt ... transcrypt? crypt?) character */
        *wr++ = encrypt_char(enigma, c);
    } while (c != '\0');

    return wr - output;
}


/**
 * Encrypts (or decrypts) a single character (or letter) `c` given the current machine
 * settings and updates the rotor positions accordingly. 
 *
 * NB:`c` must be in the enigma alphabet and upper-case. 
 */
u8 encrypt_char(Enigma *enigma, char c)
{
    /* 1. advance rotors */
    if (is_at_turnover(&enigma->rotor[1])) { 
        step_rotor(&enigma->rotor[0]);
        step_rotor(&enigma->rotor[1]);
    } else if (is_at_turnover(&enigma->rotor[2])) {
        step_rotor(&enigma->rotor[1]);
    }
    step_rotor(&enigma->rotor[2]);

    /* 2. encrypt character */
    u8 n = C2N(c); 
    n = C2N(enigma->plugboard.image[n]);
    n = rotor_pass(&enigma->rotor[2], n, FORWARD);
    n = rotor_pass(&enigma->rotor[1], n, FORWARD);
    n = rotor_pass(&enigma->rotor[0], n, FORWARD);
    n = C2N(enigma->reflector.image[n]);
    n = rotor_pass(&enigma->rotor[0], n, REVERSE);
    n = rotor_pass(&enigma->rotor[1], n, REVERSE);
    n = rotor_pass(&enigma->rotor[2], n, REVERSE);
    n = C2N(enigma->plugboard.image[n]);
    return N2C(n);
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
            enigma->rotor[i].ring_setting = C2N(TO_UPPER(l.start[0]));
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

        char c0 = TO_UPPER(pair.start[0]);
        char c1 = TO_UPPER(pair.start[1]);
        u8 n0 = C2N(c0);
        u8 n1 = C2N(c1);

        ENIGMA_ASSERT(c0 != c1, "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "A character can not be swapped with itself", HGL_SV_ARG(pair));
        ENIGMA_ASSERT(IN_ALPHABET(c0), "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "Character '%c' is not in the Enigma alphabet", HGL_SV_ARG(pair), c0);
        ENIGMA_ASSERT(IN_ALPHABET(c1), "Invalid plugboard pair \"" HGL_SV_FMT "\". "
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
            enigma->rotor[i].position = C2N(TO_UPPER(l.start[0]));
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
 * Returns the image of 'n' under the permutation given by the rotor `r` (including rotor 
 * position and ring setting) and the current flow direction `dir` through the rotor, where
 * `n` is the numerical encoding of a letter in the Enigma alphabet.
 */
static u8 rotor_pass(const Rotor *r, u8 n, Direction dir)
{
    n = (n - r->ring_setting + r->position + 26) % 26;
    switch (dir) {
        case FORWARD: n = C2N(r->forward.image[n]); break;
        case REVERSE: n = C2N(r->reverse.image[n]); break;
    }
    n = (n + r->ring_setting - r->position + 26) % 26;
    return n;
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
    char c = TO_UPPER(sv.start[0]);
    if (c >= 'A' && c <= 'Z') {
        return 1;
    }
    return 0;
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

