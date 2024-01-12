import base64
from ecdsa import SigningKey, SECP256k1
from hashlib import sha256

sk = SigningKey.generate(curve=SECP256k1, hashfunc=sha256)
vk = sk.verifying_key
signature = sk.sign(b"message", hashfunc=sha256)
assert vk.verify(signature, b"message", hashfunc=sha256)

with open("./keys/signing.pem", "wb") as priv:
    priv.write(sk.to_pem())

with open("./keys/verifying.pem", "wb") as pub:
    pub.write(vk.to_pem())

# Generate the c include file.

with open("./boot/key/verify.inc", "wb") as f:
    encoded_key = base64.b64encode(vk.to_string())
    size = len(vk.to_string())
    macro_size = "{}".format(size)
    f.write(b'static const char* VERIFYING_KEY="')
    f.write(encoded_key)
    f.write(b'";\n')
    f.write(b'#define VERIFYING_KEY_LENGTH (')
    f.write(bytes(macro_size, "ascii"))
    f.write(b')')


print("Done.")
