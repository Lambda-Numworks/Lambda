#coding=utf-8
# Sign a firmware with Ed25519

import json
import struct
import sys

try:
    from nacl.encoding import HexEncoder
    from nacl.signing import SigningKey
except ImportError as e:
    print("You need PyNaCl to be able to sign firmwares!", file=sys.stderr)
    print("Install it with pip: pip install pynacl", file=sys.stderr)
    sys.exit(1)

if len(sys.argv) != 4:
    print("usage:", sys.argv[0], "keys.json", "input.bin", "output.signed.bin", file=sys.stderr)
    sys.exit(2)

with open(sys.argv[1], "r") as f:
    keys = json.load(f)
    key = SigningKey(keys['private'], encoder=HexEncoder)

with open(sys.argv[2], "rb") as f:
    data = f.read()
    output = bytearray(data)
    output[-64:] = key.sign(data[8:-64]).signature
    output[:4] = struct.pack("<I", 0)
    output[4:8] = struct.pack("<I", len(data) - 8 - 64)

    with open(sys.argv[3], "wb") as wf:
        wf.write(output)
