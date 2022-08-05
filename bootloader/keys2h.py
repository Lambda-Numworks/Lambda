#coding=utf-8
# Convert Ed25519 keys list to header file

import csv
import struct
import sys

if len(sys.argv) != 4:
    print("usage:", sys.argv[0], "keys.csv", "output.cpp", "output.h", file=sys.stderr)
    sys.exit(2)

with open(sys.argv[1], "r") as csvf:
    reader = list(csv.DictReader(csvf))

    with open(sys.argv[2], "w") as cpp, open(sys.argv[3], "w") as h:
        print("#ifndef BOOTLOADER_KEYS_H", file=h)
        print("#define BOOTLOADER_KEYS_H", file=h)
        print(file=h)
        print("#include <bootloader/key.h>", file=h)
        print(file=h)

        print("extern const Key AvailableKeys[" + str(len(reader)) + "];", file=h)

        print("#endif", file=h)


        print("#include <bootloader/keys.h>", file=cpp)
        print(file=cpp)

        print("const Key AvailableKeys[" + str(len(reader)) + "] __attribute__((used)) = {", file=cpp)

        for row in reader:
            key = [row['key'][i:i+2] for i in range(0, len(row['key']), 2)]
            key = map(lambda x: "0x" + x, key)
            key = ', '.join(key)
            print('{ {' + key + '}, "' + row["name"] + '", ' + row["trusted"] + '},', file=cpp)

        print("};", file=cpp)
        print(file=cpp)

