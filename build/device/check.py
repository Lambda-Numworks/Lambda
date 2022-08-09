# coding=utf-8
# Sign a firmware with Ed25519

import csv
import struct
import sys
import traceback

try:
    from nacl.encoding import HexEncoder
    from nacl.signing import VerifyKey
except ImportError as e:
    print("You need PyNaCl to be able to sign firmwares!", file=sys.stderr)
    print("Install it with pip: pip install pynacl", file=sys.stderr)
    sys.exit(1)

if len(sys.argv) != 3:
    print("usage:", sys.argv[0], "keys.csv",
          "input.signed.bin", file=sys.stderr)
    sys.exit(2)

with open(sys.argv[2], "rb") as f:
    data = f.read()
    unknown, = struct.unpack("<I", data[0:4])
    signature_offset, = struct.unpack("<I", data[4:8])
    signature = data[8+signature_offset:8+signature_offset+64]
    message = data[8:signature_offset+8]

    if len(signature) != 64:
        print("Invalid signature length", file=sys.stderr)

    with open(sys.argv[1], "r") as csvf:
        reader = list(csv.DictReader(csvf))

        for row in reader:
            key = VerifyKey(row["key"], encoder=HexEncoder)
            try:
                key.verify(message, signature)
                print("Signed with:", row["name"], "(" + ("trusted" if row["trusted"] else "untrusted") + ") -", row["key"])
                sys.exit(0)
            except Exception as e:
                pass

print("Not signed or corrupt signature")
