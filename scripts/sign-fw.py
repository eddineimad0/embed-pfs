import hashlib
import hmac

FIRMWARE_FILE = "./out/bin/firmware.bin"

with open(FIRMWARE_FILE, "rb") as f:
    file_content = f.read()

key = bytes('addedec0', 'UTF-8')
data = bytes(file_content, 'UTF-8')

digester = hmac.new(key, data, hashlib.sha1)
signature1 = digester.digest()


with open(FIRMWARE_FILE, "wb") as f:
    f.write(signature1 + file_content)
