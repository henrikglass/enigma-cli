
/*--- Include files ---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

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

/*--- Private type definitions ----------------------------------------------------------*/

typedef enum {
    FORWARD,
    REVERSE,
} Direction;

typedef struct {
    uint8_t wiring[26];
} Wiring;

typedef struct {
    Wiring forward;
    Wiring reverse;
    uint8_t turnover1;
    uint8_t turnover2; /* only valid for rotors VI, VII, and VIII */
    uint8_t ring_setting;
    uint8_t position;
} Rotor;

/* 
 * Note: Rotors are indexed from left to right as seen from the perspective of the 
 *       machine operator; i.e. rotor[0] is the leftmost (slow) rotor, and rotor[2] 
 *       is the rightmost (fast) rotor.
 */
typedef struct {
    Rotor rotor[3];
    Wiring reflector;
    Wiring plugboard;
} Enigma;

/*--- Function prototypes ---------------------------------------------------------------*/

int enigma_cli_main(int argc, char *argv[]);
uint8_t encrypt_char(Enigma *enigma, char c);

static void apply_reflector_setting(Enigma *enigma, const char *str);
static void apply_rotor_setting(Enigma *enigma, const char *str);
static void apply_ring_setting(Enigma *enigma, const char *str);
static void apply_plugboard_setting(Enigma *enigma, const char *str);
static void apply_indicator_setting(Enigma *enigma, const char *str);

static uint8_t rotor_pass(const Rotor *r, uint8_t n, Direction dir);
static size_t lex_numeric(HglStringView sv);
static size_t lex_letter(HglStringView sv);


/*--- Enigma functions ------------------------------------------------------------------*/

int enigma_cli_main(int argc, char *argv[])
{
    /* Enigma machine simulation settings */
    const char **opt_reflector_setting = hgl_flags_add_str("-u,--reflector,--umkehrwalze", "Reflector (Ger: Umkehrwalze)", "UKW-B", 0);
    const char **opt_rotor_setting     = hgl_flags_add_str("-r,--rotors,--walzenlage", "Rotor order (Ger: Walzenlage)", "I II III", 0);
    const char **opt_ring_setting      = hgl_flags_add_str("-ri,--ring-setting,--ringstellung", "Ring setting (Ger: Ringstellung)", "1 1 1", 0);
    const char **opt_plugboard_setting = hgl_flags_add_str("-p,--plugboard,--steckerverbindungen", "Plugboard transpositions (Ger: Steckerverbindungen)", "", 0);
    const char **opt_indicator_setting = hgl_flags_add_str("-in,--indicator-setting,--grundstellung", "Indicator setting (Ger: Grundstellung)", "1 1 1", 0);

    /* Enigma-cli general settings */
    //const char **opt_infile = hgl_flags_add_str("-i,--input", "Input file path", NULL, 0);
    bool *opt_help = hgl_flags_add_bool("--help,--hilfe-bitte", "Displays this message", false, 0);

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

    Enigma enigma;
    apply_reflector_setting(&enigma, *opt_reflector_setting);
    apply_rotor_setting(&enigma, *opt_rotor_setting);
    apply_ring_setting(&enigma, *opt_ring_setting);
    apply_plugboard_setting(&enigma, *opt_plugboard_setting);
    apply_indicator_setting(&enigma, *opt_indicator_setting);

    /* DEBUG */
    char c;
    for (int i = 0; i < 300; i++) {
        if (i % 5 == 0) printf(" ");
        if (i % 30 == 0) printf("\n");
        c = encrypt_char(&enigma, 'A');
        printf("%c", c);
    }
    printf("\n");
    /* END DEBUG */

    return 0;
}

uint8_t encrypt_char(Enigma *enigma, char c)
{
    /* 1. advance rotors */
    if ((enigma->rotor[1].position == enigma->rotor[1].turnover1) ||
        (enigma->rotor[1].position == enigma->rotor[1].turnover2)) {
        enigma->rotor[1].position = (enigma->rotor[1].position + 1) % 26;
        enigma->rotor[0].position = (enigma->rotor[0].position + 1) % 26;
    } else if ((enigma->rotor[2].position == enigma->rotor[2].turnover1) ||
               (enigma->rotor[2].position == enigma->rotor[2].turnover2)) {
        enigma->rotor[1].position = (enigma->rotor[1].position + 1) % 26;
    }
    enigma->rotor[2].position = (enigma->rotor[2].position + 1) % 26;

    /* 2. encrypt character */
    uint8_t n = C2N(c); 
    n = C2N(enigma->plugboard.wiring[n]);
    n = rotor_pass(&enigma->rotor[2], n, FORWARD);
    n = rotor_pass(&enigma->rotor[1], n, FORWARD);
    n = rotor_pass(&enigma->rotor[0], n, FORWARD);
    n = C2N(enigma->reflector.wiring[n]);
    n = rotor_pass(&enigma->rotor[0], n, REVERSE);
    n = rotor_pass(&enigma->rotor[1], n, REVERSE);
    n = rotor_pass(&enigma->rotor[2], n, REVERSE);
    n = C2N(enigma->plugboard.wiring[n]);
    return N2C(n);
}

static void apply_reflector_setting(Enigma *enigma, const char *str)
{
    const Wiring UKW_A = {"EJMZALYXVBWFCRQUONTSPIKHGD"};
    const Wiring UKW_B = {"YRUHQSLDPXNGOKMIEBFZCWVJAT"};
    const Wiring UKW_C = {"FVPJIAOYEDRZXWGCTKUQSBNMHL"};

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

static void apply_rotor_setting(Enigma *enigma, const char *str)
{
    const Rotor ROTOR_I    = {{"EKMFLGDQVZNTOWYHXUSPAIBRCJ"}, {"UWYGADFPVZBECKMTHXSLRINQOJ"}, C2N('Q'), 255 /* no second turnover */, 0, 0};
    const Rotor ROTOR_II   = {{"AJDKSIRUXBLHWTMCQGZNPYFVOE"}, {"AJPCZWRLFBDKOTYUQGENHXMIVS"}, C2N('E'), 255 /* no second turnover */, 0, 0};
    const Rotor ROTOR_III  = {{"BDFHJLCPRTXVZNYEIWGAKMUSQO"}, {"TAGBPCSDQEUFVNZHYIXJWLRKOM"}, C2N('V'), 255 /* no second turnover */, 0, 0};
    const Rotor ROTOR_IV   = {{"ESOVPZJAYQUIRHXLNFTGKDCMWB"}, {"HZWVARTNLGUPXQCEJMBSKDYOIF"}, C2N('J'), 255 /* no second turnover */, 0, 0};
    const Rotor ROTOR_V    = {{"VZBRGITYUPSDNHLXAWMJQOFECK"}, {"QCYLXWENFTZOSMVJUDKGIARPHB"}, C2N('Z'), 255 /* no second turnover */, 0, 0};
    const Rotor ROTOR_VI   = {{"JPGVOUMFYQBENHZRDKASXLICTW"}, {"SKXQLHCNWARVGMEBJPTYFDZUIO"}, C2N('Z'), C2N('M'), 0, 0};
    const Rotor ROTOR_VII  = {{"NZJHGRCXMYSWBOUFAIVLPEKQDT"}, {"QMGYVPEDRCWTIANUXFKZOSLHJB"}, C2N('Z'), C2N('M'), 0, 0};
    const Rotor ROTOR_VIII = {{"FKQHTLXOCBJSPDZRAMEWNIUYGV"}, {"QJINSAYDVKBFRUHMCPLEWZTGXO"}, C2N('Z'), C2N('M'), 0, 0};

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

static void apply_ring_setting(Enigma *enigma, const char *str)
{
    HglStringView sv = hgl_sv_from_cstr(str);
    for (int i = 0; i < 3; i++) {
        HglStringView l;
        sv = hgl_sv_trim(sv);
        l = hgl_sv_lchop_lexeme(&sv, lex_numeric);
        if (l.length != 0) {
            enigma->rotor[i].ring_setting = (uint8_t) hgl_sv_to_u64(l) - 1;
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

static void apply_plugboard_setting(Enigma *enigma, const char *str)
{
    /* reset plugboard */
    const Wiring BARE_PLUGBOARD = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    enigma->plugboard = BARE_PLUGBOARD;

    uint8_t wiring[26] = {0};
    HglStringView sv = hgl_sv_from_cstr(str);
    while (sv.length > 0) {
        HglStringView pair = hgl_sv_trim(hgl_sv_lchop_until(&sv, ' '));
        ENIGMA_ASSERT(pair.length == 2, "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "String should be formatted as \"ab cd ef ...\"", HGL_SV_ARG(pair));
        char c0 = TO_UPPER(pair.start[0]);
        char c1 = TO_UPPER(pair.start[1]);
        ENIGMA_ASSERT(c0 != c1, "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "A character can not be swapped with itself", HGL_SV_ARG(pair));
        ENIGMA_ASSERT(IN_ALPHABET(c0), "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "Character '%c' is not in the Enigma alphabet", HGL_SV_ARG(pair), c0);
        ENIGMA_ASSERT(IN_ALPHABET(c1), "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "Character '%c' is not in the Enigma alphabet", HGL_SV_ARG(pair), c1);
        uint8_t n0 = C2N(c0);
        uint8_t n1 = C2N(c1);
        ENIGMA_ASSERT(wiring[n0] == 0, "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "The character '%c' has already been used.", HGL_SV_ARG(pair), c0);
        ENIGMA_ASSERT(wiring[n1] == 0, "Invalid plugboard pair \"" HGL_SV_FMT "\". "
                      "The character '%c' has already been used.", HGL_SV_ARG(pair), c1);
        wiring[n0] = c1;
        wiring[n1] = c0;
        enigma->plugboard.wiring[n0] = c1;
        enigma->plugboard.wiring[n1] = c0;
    }
}

static void apply_indicator_setting(Enigma *enigma, const char *str)
{
    HglStringView sv = hgl_sv_from_cstr(str);
    for (int i = 0; i < 3; i++) {
        HglStringView l;
        sv = hgl_sv_trim(sv);
        l = hgl_sv_lchop_lexeme(&sv, lex_numeric);
        if (l.length != 0) {
            enigma->rotor[i].position = (uint8_t) hgl_sv_to_u64(l) - 1;
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
 * Returns the image of 'n' under the permutation given by the rotor `r` (including rotor 
 * position and ring setting) and the current flow direction `dir` through the rotor, where
 * `n` is the numerical encoding of a letter in the Enigma alphabet.
 */
static uint8_t rotor_pass(const Rotor *r, uint8_t n, Direction dir)
{
    n = (n - r->ring_setting + r->position + 26) % 26;
    switch (dir) {
        case FORWARD: n = C2N(r->forward.wiring[n]); break;
        case REVERSE: n = C2N(r->reverse.wiring[n]); break;
    }
    n = (n + r->ring_setting - r->position + 26) % 26;
    return n;
}

/**
 * Lexer rule which matches the numerical encodings of the letters from the Enigma alphabet.
 */
static size_t lex_numeric(HglStringView sv)
{
    size_t original_length = sv.length;
    if (sv.start[0] < '1' || sv.start[0] > '9') return 0;
    uint64_t value = hgl_sv_lchop_u64(&sv);
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
    char c = sv.start[0];
    if (c >= 'a' && c <= 'z') return 1;
    if (c >= 'A' && c <= 'Z') return 1;
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

