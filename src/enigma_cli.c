
/*--- Include files ---------------------------------------------------------------------*/

#define HGL_STRING_IMPLEMENTATION
#include "hgl_string.h"

#include <stdio.h>
#include <stdint.h>

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

/*--- Private type definitions ----------------------------------------------------------*/

typedef struct {
    uint8_t wiring_fwd[26];
    uint8_t wiring_rev[26];
    uint8_t turnover1;
    uint8_t turnover2; /* only valid for rotors VI, VII, and VIII */
} Rotor;

typedef struct {
    uint8_t wiring[26];
} Reflector;

typedef struct {
    uint8_t wiring[26];
} Plugboard;

typedef struct {
    Rotor rotor[3];
    uint8_t ring_setting[3];
    uint8_t rotor_position[3];
    Reflector reflector;
    Plugboard plugboard;
} Enigma;

/*--- Private function prototypes -------------------------------------------------------*/

Enigma enigma_configure(const char *reflector_str,
                        const char *rotor_order_str, 
                        const char *ring_setting_str, 
                        const char *plugboard_setting_str,
                        const char *indicator_setting_str);
uint8_t enigma_encrypt_character(Enigma *enigma, char c);
static uint8_t enigma_plugboard_pass(Plugboard *plugboard, uint8_t n);
static uint8_t enigma_rotor_fwd_pass(const Rotor *r, uint8_t ring_setting, uint8_t rotor_position, uint8_t n);
static uint8_t enigma_rotor_rev_pass(const Rotor *r, uint8_t ring_setting, uint8_t rotor_position, uint8_t n);
static uint8_t enigma_reflector_pass(const Reflector *r, uint8_t n);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

#ifndef HGL_TEST_H
int main(void)
{
    char c;
    //Enigma enigma = enigma_configure("UKW-B", "I II III", "1 1 1", NULL, "1 1 1");
    Enigma enigma = enigma_configure("UKW-B", "VI I III", "1 1 1", "bq cr di ej kw mt os px uz gh", "1 17 12");
    for (int i = 0; i < 300; i++) {
        if (i % 5 == 0) printf(" ");
        if (i % 15 == 0) printf("\n");
        c = enigma_encrypt_character(&enigma, 'A');
        printf("%c", c);
    }
    printf("\n");

}
#endif

/*--- Private functions -----------------------------------------------------------------*/

Enigma enigma_configure(const char *reflector_str,
                        const char *rotor_order_str, 
                        const char *ring_setting_str, 
                        const char *plugboard_setting_str,
                        const char *indicator_setting_str)
{
    const Rotor ROTOR_I    = {"EKMFLGDQVZNTOWYHXUSPAIBRCJ", "UWYGADFPVZBECKMTHXSLRINQOJ", C2N('Q'), 255 /* no second turnover*/ };
    const Rotor ROTOR_II   = {"AJDKSIRUXBLHWTMCQGZNPYFVOE", "AJPCZWRLFBDKOTYUQGENHXMIVS", C2N('E'), 255 /* no second turnover*/ };
    const Rotor ROTOR_III  = {"BDFHJLCPRTXVZNYEIWGAKMUSQO", "TAGBPCSDQEUFVNZHYIXJWLRKOM", C2N('V'), 255 /* no second turnover*/ };
    const Rotor ROTOR_IV   = {"ESOVPZJAYQUIRHXLNFTGKDCMWB", "HZWVARTNLGUPXQCEJMBSKDYOIF", C2N('J'), 255 /* no second turnover*/ };
    const Rotor ROTOR_V    = {"VZBRGITYUPSDNHLXAWMJQOFECK", "QCYLXWENFTZOSMVJUDKGIARPHB", C2N('Z'), 255 /* no second turnover*/ };
    const Rotor ROTOR_VI   = {"JPGVOUMFYQBENHZRDKASXLICTW", "SKXQLHCNWARVGMEBJPTYFDZUIO", C2N('Z'), C2N('M')};
    const Rotor ROTOR_VII  = {"NZJHGRCXMYSWBOUFAIVLPEKQDT", "QMGYVPEDRCWTIANUXFKZOSLHJB", C2N('Z'), C2N('M')};
    const Rotor ROTOR_VIII = {"FKQHTLXOCBJSPDZRAMEWNIUYGV", "QJINSAYDVKBFRUHMCPLEWZTGXO", C2N('Z'), C2N('M')};

    const Reflector UKW_A = {"EJMZALYXVBWFCRQUONTSPIKHGD"};
    const Reflector UKW_B = {"YRUHQSLDPXNGOKMIEBFZCWVJAT"};
    const Reflector UKW_C = {"FVPJIAOYEDRZXWGCTKUQSBNMHL"};

    /* Default configuration */
    Enigma enigma = {
        .rotor[0] = ROTOR_I,
        .rotor[1] = ROTOR_II,
        .rotor[2] = ROTOR_III,
        .ring_setting[0] = 0,
        .ring_setting[1] = 0,
        .ring_setting[2] = 0,
        .rotor_position[0] = 0,
        .rotor_position[1] = 0,
        .rotor_position[2] = 0,
        .reflector = UKW_B,
    };
    memcpy(enigma.plugboard.wiring, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);

    /* parse enigma settings: Reflector (Umkehrwalze) */
    if (reflector_str != NULL) {
        HglStringView sv = hgl_sv_from_cstr(reflector_str);
        if (hgl_sv_equals(sv, HGL_SV("UKW-A"))) {
            enigma.reflector = UKW_A;
        } else if (hgl_sv_equals(sv, HGL_SV("UKW-B"))) {
            enigma.reflector = UKW_B;
        } else if (hgl_sv_equals(sv, HGL_SV("UKW-C"))) {
            enigma.reflector = UKW_C;
        } else {
            ENIGMA_ASSERT(false, "Unknown reflector \"" HGL_SV_FMT "\".", HGL_SV_ARG(sv));
        }
    }

    /* parse enigma settings: Rotor Order (Walzenlage) */
    if (rotor_order_str != NULL) {
        HglStringView sv = hgl_sv_from_cstr(rotor_order_str);
        for (int i = 0; i < 3; i++) {
            HglStringView rotor_name = hgl_sv_trim(hgl_sv_lchop_until(&sv, ' '));
            if (hgl_sv_equals(rotor_name, HGL_SV("I"))) {
                enigma.rotor[i] = ROTOR_I;
            } else if (hgl_sv_equals(rotor_name, HGL_SV("II"))) {
                enigma.rotor[i] = ROTOR_II;
            } else if (hgl_sv_equals(rotor_name, HGL_SV("III"))) {
                enigma.rotor[i] = ROTOR_III;
            } else if (hgl_sv_equals(rotor_name, HGL_SV("IV"))) {
                enigma.rotor[i] = ROTOR_IV;
            } else if (hgl_sv_equals(rotor_name, HGL_SV("V"))) {
                enigma.rotor[i] = ROTOR_V;
            } else if (hgl_sv_equals(rotor_name, HGL_SV("VI"))) {
                enigma.rotor[i] = ROTOR_VI;
            } else if (hgl_sv_equals(rotor_name, HGL_SV("VII"))) {
                enigma.rotor[i] = ROTOR_VII;
            } else if (hgl_sv_equals(rotor_name, HGL_SV("VIII"))) {
                enigma.rotor[i] = ROTOR_VIII;
            } else {
                ENIGMA_ASSERT(false, "Unknown rotor \"" HGL_SV_FMT "\".", HGL_SV_ARG(rotor_name));
            }
        }
    }

    /* parse enigma settings: Ring Setting (Ringstellung) */
    if (ring_setting_str != NULL) {
        HglStringView sv = hgl_sv_from_cstr(ring_setting_str);
        for (int i = 0; i < 3; i++) {
            uint64_t ring_setting = hgl_sv_lchop_u64(&sv);
            if (ring_setting < 1 || ring_setting > 26) {
                fprintf(stderr, "invalid ring setting\n"); // TODO better error handling
                abort();
            }
            enigma.ring_setting[i] = (uint8_t) (ring_setting - 1);
        }
    }

    /* parse enigma settings: Plugboard (Steckerverbindungen) */
    if (plugboard_setting_str != NULL) {
        uint8_t wiring[26] = {0};
        HglStringView sv = hgl_sv_from_cstr(plugboard_setting_str);
        while (sv.length > 0) {
            HglStringView pair = hgl_sv_trim(hgl_sv_lchop_until(&sv, ' '));
            ENIGMA_ASSERT(pair.length == 2, "Invalid plugboard pair \"" HGL_SV_FMT "\". String should be formatted as \"ab cd ef ...\"", HGL_SV_ARG(pair));
            char c0 = TO_UPPER(pair.start[0]);
            char c1 = TO_UPPER(pair.start[1]);
            ENIGMA_ASSERT(c0 != c1, "Invalid plugboard pair \"" HGL_SV_FMT "\". A character can not be swapped with itself", HGL_SV_ARG(pair));
            ENIGMA_ASSERT(IN_ALPHABET(c0), "Invalid plugboard pair \"" HGL_SV_FMT "\". Character '%c' is not in the Enigma alphabet", HGL_SV_ARG(pair), c0);
            ENIGMA_ASSERT(IN_ALPHABET(c1), "Invalid plugboard pair \"" HGL_SV_FMT "\". Character '%c' is not in the Enigma alphabet", HGL_SV_ARG(pair), c1);
            uint8_t n0 = C2N(c0);
            uint8_t n1 = C2N(c1);
            ENIGMA_ASSERT(wiring[n0] == 0, "Invalid plugboard pair \"" HGL_SV_FMT "\". The character '%c' has already been used.", HGL_SV_ARG(pair), c0);
            ENIGMA_ASSERT(wiring[n1] == 0, "Invalid plugboard pair \"" HGL_SV_FMT "\". The character '%c' has already been used.", HGL_SV_ARG(pair), c1);
            wiring[n0] = c1;
            wiring[n1] = c0;
        }

        /* apply wiring to plugboard */
        for (int i = 0; i < 26; i++) {
            if (wiring[i] != 0) enigma.plugboard.wiring[i] = wiring[i];
        }
    }

    /* parse enigma settings: Indicator Setting (Grundstellung) */
    if (indicator_setting_str != NULL) {
        HglStringView sv = hgl_sv_from_cstr(indicator_setting_str);
        for (int i = 0; i < 3; i++) {
            uint64_t rotor_position = hgl_sv_lchop_u64(&sv);
            if (rotor_position < 1 || rotor_position > 26) {
                fprintf(stderr, "invalid rotor position\n"); // TODO better error handling
                abort();
            }
            enigma.rotor_position[i] = (uint8_t) (rotor_position - 1);
        }
    }

    return enigma;
}

uint8_t enigma_encrypt_character(Enigma *enigma, char c)
{
    /* 1. advance rotors */
    if ((enigma->rotor_position[1] == enigma->rotor[1].turnover1) ||
        (enigma->rotor_position[1] == enigma->rotor[1].turnover2)) {
        enigma->rotor_position[1] = (enigma->rotor_position[1] + 1) % 26;
        enigma->rotor_position[0] = (enigma->rotor_position[0] + 1) % 26;
    } else if ((enigma->rotor_position[2] == enigma->rotor[2].turnover1) ||
               (enigma->rotor_position[2] == enigma->rotor[2].turnover2)) {
        enigma->rotor_position[1] = (enigma->rotor_position[1] + 1) % 26;
    }
    enigma->rotor_position[2] = (enigma->rotor_position[2] + 1) % 26;

    /* 2. encrypt character */
    uint8_t n = C2N(c); 
    n = enigma_plugboard_pass(&enigma->plugboard, n);
    n = enigma_rotor_fwd_pass(&enigma->rotor[2], enigma->ring_setting[2], enigma->rotor_position[2], n);
    n = enigma_rotor_fwd_pass(&enigma->rotor[1], enigma->ring_setting[1], enigma->rotor_position[1], n);
    n = enigma_rotor_fwd_pass(&enigma->rotor[0], enigma->ring_setting[0], enigma->rotor_position[0], n);
    n = enigma_reflector_pass(&enigma->reflector, n);
    n = enigma_rotor_rev_pass(&enigma->rotor[0], enigma->ring_setting[0], enigma->rotor_position[0], n);
    n = enigma_rotor_rev_pass(&enigma->rotor[1], enigma->ring_setting[1], enigma->rotor_position[1], n);
    n = enigma_rotor_rev_pass(&enigma->rotor[2], enigma->ring_setting[2], enigma->rotor_position[2], n);
    n = enigma_plugboard_pass(&enigma->plugboard, n);
    return N2C(n);
}

static uint8_t enigma_plugboard_pass(Plugboard *plugboard, uint8_t n)
{
    return C2N(plugboard->wiring[n]);
}

static uint8_t enigma_rotor_fwd_pass(const Rotor *r, uint8_t ring_setting, uint8_t rotor_position, uint8_t n)
{
    n = (n - ring_setting + rotor_position + 26) % 26;
    n = C2N(r->wiring_fwd[n]);
    n = (n + ring_setting - rotor_position + 26) % 26;
    return n;
}

static uint8_t enigma_rotor_rev_pass(const Rotor *r, uint8_t ring_setting, uint8_t rotor_position, uint8_t n)
{
    n = (n - ring_setting + rotor_position + 26) % 26;
    n = C2N(r->wiring_rev[n]);
    n = (n + ring_setting - rotor_position + 26) % 26;
    return n;
}

static uint8_t enigma_reflector_pass(const Reflector *r, uint8_t n)
{
    return C2N(r->wiring[n]);
}

