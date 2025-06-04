
#include "hgl_test.h"

#include "enigma_cli.c"

TEST(test_default_configuration) {
    Enigma enigma;
    apply_reflector_setting(&enigma, "UKW-B");
    apply_rotor_setting(&enigma, "I II III");
    apply_ring_setting(&enigma, "AAA");
    apply_plugboard_setting(&enigma, "");
    apply_indicator_setting(&enigma, "AAA");
    ASSERT(encrypt_char(&enigma, 'A') == 'B');
    ASSERT(encrypt_char(&enigma, 'A') == 'D');
    ASSERT(encrypt_char(&enigma, 'A') == 'Z');
    ASSERT(encrypt_char(&enigma, 'A') == 'G');
    ASSERT(encrypt_char(&enigma, 'A') == 'O');
}

