import time
import serial
from serial import Serial
from serial.threaded import Protocol
from enum import Enum

PACKET_LENGTH_BYTES = 1
PACKET_PAYLOAD_BYTES = 19
PACKET_CRC_BYTES = 4
PACKET_SIZE = PACKET_LENGTH_BYTES + PACKET_PAYLOAD_BYTES + PACKET_CRC_BYTES

PACKET_ACK_PAYLOAD_BYTE0 = 0xAC
PACKET_RET_PAYLOAD_BYTE0 = 0xAB
PACKET_PAYLOAD_PADDING = 0xFF

PACKET_ACK_PAYLOAD = bytearray()
PACKET_ACK_PAYLOAD.append(PACKET_ACK_PAYLOAD_BYTE0)
PACKET_ACK_PAYLOAD.extend([0xFF for i in range(PACKET_PAYLOAD_BYTES - 1)])

PACKET_RET_PAYLOAD = bytearray()
PACKET_RET_PAYLOAD.append(PACKET_RET_PAYLOAD_BYTE0)
PACKET_RET_PAYLOAD.extend([0xFF for i in range(PACKET_PAYLOAD_BYTES - 1)])


SERIAL_PORT = "COM4"
BAUD_RATE = 115200


def crc32bzip2(data: bytes) -> int:
    crc = 0xffffffff
    for x in data:
        crc ^= x << 24
        for k in range(8):
            crc = (crc << 1) ^ 0x04c11db7 if crc & 0x80000000 else crc << 1
    crc = ~crc
    crc &= 0xffffffff
    return crc


class Packet:
    def __init__(self, length: int, data: bytes):
        self.length = length
        self.data = data
        self.crc = self.calculate_crc32()

    def calculate_crc32(self) -> int:
        header = bytearray()
        header.append(self.length)
        header.extend(self.data)
        return crc32bzip2(header)

    def is_cntrl_packet(self, byte: int) -> bool:
        if self.length != 1:
            return False
        elif self.data[0] != byte:
            return False
        for i in range(1, PACKET_PAYLOAD_BYTES):
            if self.data[i] != 0xFF:
                return False
        return True

    def is_ack(self) -> bool:
        return self.is_cntrl_packet(PACKET_ACK_PAYLOAD_BYTE0)

    def is_ret(self) -> bool:
        return self.is_cntrl_packet(PACKET_RET_PAYLOAD_BYTE0)

    def as_bytes(self) -> bytes:
        return bytes([self.length, self.data, self.crc])

    def __str__(self):
        return f"[length={self.length}|data={self.data.hex()}|crc={hex(self.crc)}]"


class ProtocolState(Enum):
    await_length = 1
    await_payload = 2
    await_crc = 3


class SerialReader(Protocol):
    def __init__(self):
        self.buffer = bytearray()
        self.state = ProtocolState.await_length
        self.length = 0
        self.payload = []
        self.crc = 0

    def data_received(self, data: bytes):
        self.buffer.extend(data)
        self.protocol_handler()

    def protocol_handler(self):
        while len(self.buffer) > 1:
            match self.state:
                case ProtocolState.await_length:
                    self.length = int(self.buffer.pop(0))
                    self.state = ProtocolState.await_payload
                    continue
                case ProtocolState.await_payload:
                    if len(self.buffer) < PACKET_PAYLOAD_BYTES:
                        continue
                    else:
                        self.payload = self.buffer[0:PACKET_PAYLOAD_BYTES]
                        self.buffer = self.buffer[PACKET_PAYLOAD_BYTES:]
                        self.state = ProtocolState.await_crc
                        # print(f"[+] Packet payload received result={self.payload}")
                        continue
                case ProtocolState.await_crc:
                    if len(self.buffer) < PACKET_CRC_BYTES:
                        continue
                    else:
                        self.crc = int.from_bytes(self.buffer,
                                                'little', signed=False)
                        pkt = Packet(self.length, self.payload)
                        if pkt.crc != self.crc:
                            print(f"[-] CRC Error, received={hex(self.crc)},calculated={hex(pkt.crc)}")
                            packets_writer.append(RET)
                        elif pkt.is_ret():
                            print("[+] Retrainsmit packet received.")
                            packets_writer.append(
                                packets_reader[len(packets_reader) - 1]
                            )
                        elif pkt.is_ack():
                            print("[+] Acknowledge packet received.")
                            pass
                        else:
                            packets_reader.append(pkt)
                            packets_writer.append(ACK)

                        self.buffer.clear()
                        self.state = ProtocolState.await_length
                        print("[+] Finished receiving packet.")
                        return


ACK = Packet(1, PACKET_ACK_PAYLOAD)
RET = Packet(1, PACKET_RET_PAYLOAD)

packets_reader = []
packets_writer = []


def await_packets():
    time.sleep(1)


def main():
    try:
        serial_comm = Serial(SERIAL_PORT, BAUD_RATE)
    except Exception as e:
        print(e)
        exit(1)

    with serial.threaded.ReaderThread(serial_comm, SerialReader) as protocol:
        while True:

            # while len(packets_writer) > 1:
            #     protocol.write(packets_writer.pop(0))

            while len(packets_reader) > 1:
                pkt = packets_reader.pop(0)
                print(pkt)

            # print("[+] Waiting for packet")
            # await_packets()


if __name__ == "__main__":
    main()
