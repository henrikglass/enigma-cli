
#include "hgl_test.h"

#include "enigma_cli.c"

TEST(
    test_default_configuration_encrypt, 
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
    enigma_cli_main(argc, argv);
}

TEST(
    test_default_configuration_decrypt, 
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
    enigma_cli_main(argc, argv);
}

TEST(
    test_mixed_case, 
    .input =         "aaAAaAaAaa",
    .expect_output = "BDZGO WCXLT  \n"
) {
    char *argv[] = {"0"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    enigma_cli_main(argc, argv);
}

TEST(
    test_unknown_characters, 
    .input =         "A;AA,öäööAA-AAAAA",
    .expect_output = "BDZGO WCXLT  \n"
) {
    char *argv[] = {"0"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    enigma_cli_main(argc, argv);
}

TEST(
    test_double_step, 
    .input =         "AAAAA AAAAA",
    .expect_output = "HDZGO VBUYP  \n"
) {
    char *argv[] = {"0", "--rotors", "III II I", "--indicator-setting", "ADO"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    enigma_cli_main(argc, argv);
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
        "--plugboard-setting", "AB CD kf Pz XR"};
    int argc = sizeof(argv) / sizeof(argv[0]); 
    enigma_cli_main(argc, argv);
}
