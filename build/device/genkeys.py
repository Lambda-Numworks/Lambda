#coding=utf-8
# Generate Ed25519 key pair

import json
import sys

try:
    from nacl.encoding import HexEncoder
    from nacl.signing import SigningKey
except ImportError as e:
    print("You need PyNaCl to be able to sign firmwares!", file=sys.stderr)
    print("Install it with pip: pip install pynacl", file=sys.stderr)
    sys.exit(1)

if len(sys.argv) != 2:
    print("usage:", sys.argv[0], "output.json", file=sys.stderr)
    sys.exit(2)

key = SigningKey.generate()

with open(sys.argv[1], "w") as f:
    json.dump({
        "private": key.encode(encoder=HexEncoder).decode("utf-8"),
        "public": key.verify_key.encode(encoder=HexEncoder).decode("utf-8")
    }, f)
