
#include "hgl_test.h"

#include "enigma_cli.c"

GLOBAL_SETUP {
    hgl_flags_reset();
}

TEST(
    test_default_configuration_encipher, 
    .input = 
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA",
    .expect_output =
        "BDZGO WCXLT KSBTM CDLPB MUQOF XYHCX \n"
        "TGYJF LINHN XSHIU NTHEO RXPQP KOVHC \n"
        "BUBTZ SZSOO STGOT FSODB BZZLX LCYZX \n"
        "IFGWF DZEEQ IBMGF JBWZF CKPFM GBXQC \n"
        "IVIBB RNCOC JUVYD KMVJP FMDRM TGLWF \n"
        "OZLXG JEYYQ PVPBW NCKVK LZTCB DLDCT \n"
        "SNRCO OVPTG BVBBI SGJSO YHDEN CTNUU \n"
        "KCUGH REVWB DJCTQ XXOGL EBZMD BRZOS \n"
        "XDTZS ZBGDC FPRBZ YQGSN CCHGY EWOHV \n"
        "JBYZG KDGYN NEUJI WCTYC YTUUM BOYVU \n"
        "NNQUK KSOBS CORSU OSCNV ROQLH EUDSU \n"
        "KYMIG IBSXP IHNTU VGGHI FQTGZ XLGYQ \n"
        "CNVNS RCLVP YOSVR BKCEX RNLGD YWEBF \n"
        "XIVKK TUGKP VMZOT UOGMH HZDRE KJHLE \n"
        "FKKPO XLWBW VBYUK DTQUH DQTRE VRQJM \n \n"
) {
    char *argv[] = {"0"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_default_configuration_decipher, 
    .input = 
        "BDZGO WCXLT KSBTM CDLPB MUQOF XYHCX"
        "TGYJF LINHN XSHIU NTHEO RXPQP KOVHC"
        "BUBTZ SZSOO STGOT FSODB BZZLX LCYZX"
        "IFGWF DZEEQ IBMGF JBWZF CKPFM GBXQC"
        "IVIBB RNCOC JUVYD KMVJP FMDRM TGLWF"
        "OZLXG JEYYQ PVPBW NCKVK LZTCB DLDCT"
        "SNRCO OVPTG BVBBI SGJSO YHDEN CTNUU"
        "KCUGH REVWB DJCTQ XXOGL EBZMD BRZOS"
        "XDTZS ZBGDC FPRBZ YQGSN CCHGY EWOHV"
        "JBYZG KDGYN NEUJI WCTYC YTUUM BOYVU"
        "NNQUK KSOBS CORSU OSCNV ROQLH EUDSU"
        "KYMIG IBSXP IHNTU VGGHI FQTGZ XLGYQ"
        "CNVNS RCLVP YOSVR BKCEX RNLGD YWEBF"
        "XIVKK TUGKP VMZOT UOGMH HZDRE KJHLE"
        "FKKPO XLWBW VBYUK DTQUH DQTRE VRQJM",
    .expect_output =
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n \n"
) {
    char *argv[] = {"0"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_example_settings_encipher, 
    .input = 
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA",
    .expect_output =
        "MYIJE XBYWP MWCVO KJVWX KELDQ NPJVS \n"
        "FWSQO IBHMU HRTWV SRQIY TTJYG FBKDT \n"
        "YXYJT VMKCY WKULC EWTNU XLFQL DEEZV \n"
        "HEPFD OJHMZ UMRTH NBRGO RXRLB NBBPF \n"
        "DYJCD QEESB UWQLG QEWTR YUNLH HTZYO \n"
        "RMZUM XIUKB IXDSE UTVEN VMSJE EBZBL \n"
        "ROVUF JEWYN EYBUT KPFDU JZBQC CMNIZ \n"
        "UTGWY SVIYV LJVOS BSIYZ IJRTR YSCTE \n"
        "WILTU FNPLI NTPMO MWQDQ LXIKN DGZXH \n"
        "FGSMC DIUZR JETCH YEZZT PYORT ERPPF \n"
        "FMXJE CDWHL XQCOQ JEVEF FEJYQ MCMIJ \n"
        "EIBTM LJHEU SJEYY RETPK SPBDF VZSNR \n"
        "IDCDL YGSPF MZMCL ZBPSH VDYNN TFWZM \n"
        "LSODI MMZWQ RKXRS SVVKR KGTDC LODZS \n"
        "ZMSVJ ZNUQO ERHFV OSIET MXIUK EBCHS \n \n"
) {
    // ./enigma-cli -u "UKW-C" -w "II IV I" -r "6 17 26" -s "AC LS BQ WN MY UV FJ PZ TR OK" -g "HAG"
    char *argv[] = {
        "0", 
        "-u", "UKW-C", 
        "-w", "II IV I", 
        "-r", "6 17 26", 
        "-s", "ac ls bq wn my uv fj pz tr ok", 
        "-g", "HAG",
    };
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_example_settings_decipher, 
    .input = 
        "MYIJE XBYWP MWCVO KJVWX KELDQ NPJVS"
        "FWSQO IBHMU HRTWV SRQIY TTJYG FBKDT"
        "YXYJT VMKCY WKULC EWTNU XLFQL DEEZV"
        "HEPFD OJHMZ UMRTH NBRGO RXRLB NBBPF"
        "DYJCD QEESB UWQLG QEWTR YUNLH HTZYO"
        "RMZUM XIUKB IXDSE UTVEN VMSJE EBZBL"
        "ROVUF JEWYN EYBUT KPFDU JZBQC CMNIZ"
        "UTGWY SVIYV LJVOS BSIYZ IJRTR YSCTE"
        "WILTU FNPLI NTPMO MWQDQ LXIKN DGZXH"
        "FGSMC DIUZR JETCH YEZZT PYORT ERPPF"
        "FMXJE CDWHL XQCOQ JEVEF FEJYQ MCMIJ"
        "EIBTM LJHEU SJEYY RETPK SPBDF VZSNR"
        "IDCDL YGSPF MZMCL ZBPSH VDYNN TFWZM"
        "LSODI MMZWQ RKXRS SVVKR KGTDC LODZS"
        "ZMSVJ ZNUQO ERHFV OSIET MXIUK EBCHS",
    .expect_output =
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n"
        "AAAAA AAAAA AAAAA AAAAA AAAAA AAAAA \n \n"
) {
    // ./enigma-cli -u "UKW-C" -w "II IV I" -r "6 17 26" -s "AC LS BQ WN MY UV FJ PZ TR OK" -g "HAG"
    char *argv[] = {
        "0", 
        "-u", "UKW-C", 
        "-w", "II IV I", 
        "-r", "6 17 26", 
        "-s", "ac ls bq wn my uv fj pz tr ok", 
        "-g", "HAG",
    };
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_mixed_case, 
    .input =         "aaAAaAaAaa",
    .expect_output = "BDZGO WCXLT  \n"
) {
    char *argv[] = {"0"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_output_formatting_1, 
    .input =         "AAAAA AAAAA",
    .expect_output = "BD ZG OW CX LT  \n"
) {
    char *argv[] = {"0", "-G", "2", "-N", "10"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_output_formatting_2, 
    .input =         "AAAAA AAAAA",
    .expect_output = "BDZ GOW CXL \nT \n"
) {
    char *argv[] = {"0", "-G", "3", "-N", "3"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_unknown_characters, 
    .input =         "A;AA,öäööAA-AAAAA",
    .expect_output = "BDZGO WCXLT  \n"
) {
    char *argv[] = {"0"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_double_step, 
    .input =         "AAAAA AAAAA",
    .expect_output = "HDZGO VBUYP  \n"
) {
    char *argv[] = {"0", "--rotors", "III II I", "--indicator-setting", "ADO"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_valid_machine_settings, 
    .input =         "AAAAA AAAAA",
    .expect_output = "VFEZT BNMFM  \n"
) {
    char *argv[] = {
        "0", 
        "--reflector",         "UKW-C", 
        "--rotors",            "III II I", 
        "--ring-setting",      "1 23 16",
        "--indicator-setting", "HAG",
        "--plugboard-setting", "AB CD kf Pz XR"
    };
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_invalid_plugboard_setting_1, 
    .expect_exit_code = 1,
    .input = "\n"
) {
    char *argv[] = {"0", "--plugboard-setting", "AA"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_invalid_plugboard_setting_2, 
    .expect_exit_code = 1,
    .input = "\n"
) {
    char *argv[] = {"0", "--plugboard-setting", "AB KA"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_invalid_plugboard_setting_3, 
    .expect_exit_code = 1,
    .input = "\n"
) {
    char *argv[] = {"0", "--plugboard-setting", "AB KTY"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_invalid_reflector_setting_1, 
    .expect_exit_code = 1,
    .input = "\n"
) {
    char *argv[] = {"0", "--reflector", "XXX"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_invalid_rotor_setting_1, 
    .expect_exit_code = 1,
    .input = "\n"
) {
    char *argv[] = {"0", "--rotors", "II I"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_invalid_indicator_setting_1, 
    .expect_exit_code = 1,
    .input = "\n"
) {
    char *argv[] = {"0", "--indicator-setting", "HA"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_invalid_indicator_setting_2, 
    .expect_exit_code = 1,
    .input = "\n"
) {
    char *argv[] = {"0", "--indicator-setting", "HAGS"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}

TEST(
    test_invalid_indicator_setting_3, 
    .expect_exit_code = 1,
    .input = "\n"
) {
    char *argv[] = {"0", "--indicator-setting", "06 1 26"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    int exit_code = enigma_cli_main(argc, argv);
    exit(exit_code);
}
