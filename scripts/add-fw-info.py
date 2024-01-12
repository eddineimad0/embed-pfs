import sys
import struct
from ecdsa import SigningKey
from hashlib import sha256

UPDATE_FILE = "./out/bin/update.bin"


def sign_firmware(sk_pem, firmware):
    sk = SigningKey.from_pem(sk_pem)
    signature = sk.sign(firmware, hashfunc=sha256)
    return signature


if len(sys.argv) < 2:
    print("Usage: add-fw-header [Version number]")


update_version = int(sys.argv[1])
print(f"[Info]: Firmware version {update_version}")

with open("./keys/signing.pem", "rb") as pem:
    key_pem = pem.read()

with open(UPDATE_FILE, "rb") as f:
    update_data = f.read()

update_size = len(update_data)

print(f"[Info]: Firmware size {update_size}")

hash = sha256(update_data)
print(f"[Info]: Firmware hash {hash.hexdigest()}")

signature = sign_firmware(key_pem, update_data)

firmware_info = struct.pack("<LL", update_version, update_size)

with open(UPDATE_FILE + ".signed", "wb") as f:
    f.write(update_data)
    f.write(firmware_info)
    f.write(signature)
