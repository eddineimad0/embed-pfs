BOOTLOADER_SIZE = 0x4000
BOOTLOADER_FILE = "./out/bin/bootloader.bin"


with open(BOOTLOADER_FILE,"rb") as f:
    file_content = f.read()

bytes_to_pad = BOOTLOADER_SIZE - len(file_content)
padding = bytes([0xFF for _ in range(bytes_to_pad)])

with open(BOOTLOADER_FILE,"wb") as f:
    f.write(file_content + padding)

