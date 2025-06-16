[![Build and Test](https://github.com/henrikglass/enigma-cli/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/henrikglass/enigma-cli/actions/workflows/build-and-test.yml)

# enigma-cli
Enigma-cli is a simulation of the Enigma M3 cipher machine used by the German military 
to send secret messages during WWII. Enimga-cli is usable entirely from the 
command-line and accepts input on STDIN.

## Usage

```
Usage: ./enigma-cli [Options]
Options:
  -u,--reflector,--umkehrwalze                     Reflector (Ger: Umkehrwalze) (default = "UKW-B")
  -w,--rotors,--walzenlage                         Rotor order (Ger: Walzenlage) (default = "I II III")
  -r,--ring-setting,--ringstellung                 Ring setting (Ger: Ringstellung) (default = "1 1 1")
  -s,--plugboard-setting,--steckerverbindungen     Plugboard transpositions (Ger: Steckerverbindungen) (default = "")
  -g,--indicator-setting,--grundstellung           Indicator setting (Ger: Grundstellung) (default = "1 1 1")
  -G,--group-size                                  Number of characters per group in the output. (default = 5, valid range = [1, 64])
  -N,--groups-per-line                             Number of groups per line in the output. (default = 6, valid range = [1, 64])
  --help,--hilfe                                   Displays this message (default = 0)
```

## Example

To set up the machine according to the "Armee-Stabs-Maschinenschlüssel Nr.28"
procedures for October 1st 1944, assuming the machine is fitted with an UKW-C
reflector, and assuming an operator-chosen initial indicator setting "HAG",
run enimga-cli like this:

```bash
$ ./enigma-cli -u "UKW-C" -w "II IV I" -r "6 17 26" -s "AC LS BQ WN MY UV FJ PZ TR OK" -g "HAG"
```

Link to procedures sheet: 

https://web.archive.org/web/20250606093439/https://www.ciphermachinesandcryptology.com/img/enigma/hires-wehrmachtkey-stab.jpg

## Building

To build enigma-cli, run:

```bash
$ make
```

To build and run the tests, run:

```bash
$ make test
```
