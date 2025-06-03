

rot1 = "EKMFLGDQVZNTOWYHXUSPAIBRCJ"
rot2 = "AJDKSIRUXBLHWTMCQGZNPYFVOE"
rot3 = "BDFHJLCPRTXVZNYEIWGAKMUSQO"
rot4 = "ESOVPZJAYQUIRHXLNFTGKDCMWB"
rot5 = "VZBRGITYUPSDNHLXAWMJQOFECK"
rot6 = "JPGVOUMFYQBENHZRDKASXLICTW"
rot7 = "NZJHGRCXMYSWBOUFAIVLPEKQDT"
rot8 = "FKQHTLXOCBJSPDZRAMEWNIUYGV"

for l in range(ord('A'), ord('Z') + 1):
    for i, _ in enumerate(rot1):
        if rot1[i] == chr(l):
            print(chr(i + ord('A')), end='')
print("")

for l in range(ord('A'), ord('Z') + 1):
    for i, _ in enumerate(rot2):
        if rot2[i] == chr(l):
            print(chr(i + ord('A')), end='')
print("")

for l in range(ord('A'), ord('Z') + 1):
    for i, _ in enumerate(rot3):
        if rot3[i] == chr(l):
            print(chr(i + ord('A')), end='')
print("")

for l in range(ord('A'), ord('Z') + 1):
    for i, _ in enumerate(rot4):
        if rot4[i] == chr(l):
            print(chr(i + ord('A')), end='')
print("")

for l in range(ord('A'), ord('Z') + 1):
    for i, _ in enumerate(rot5):
        if rot5[i] == chr(l):
            print(chr(i + ord('A')), end='')
print("")

for l in range(ord('A'), ord('Z') + 1):
    for i, _ in enumerate(rot6):
        if rot6[i] == chr(l):
            print(chr(i + ord('A')), end='')
print("")

for l in range(ord('A'), ord('Z') + 1):
    for i, _ in enumerate(rot7):
        if rot7[i] == chr(l):
            print(chr(i + ord('A')), end='')
print("")

for l in range(ord('A'), ord('Z') + 1):
    for i, _ in enumerate(rot8):
        if rot8[i] == chr(l):
            print(chr(i + ord('A')), end='')
print("")
