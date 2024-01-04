# Intro

The Goal of this project is to implement secure boot, and secure firmware flash for programmable microschip.

# Hardware

STM32F103C8T6 microcontroller.

## Caracteristiques

    CPU: 32-bit ARM CORTEX-M3, frequence 72 MHz
    Flash Mem: 64KB
    SRAM: 20KB

## MCU Overview

![image](./chip_components.png)

![image](./pins_datasheet.png)

## Memory Map

![image](./arm_cortex_m_fixed_memory_map.png)

## Bootloader
    Size 16KiB
    Base adress 0x8000000

## UART
    Baud rate 115200
    Packet size 8 bits
    1 start bit, 1 stop bit and 0 parity bit
    10 bits to send an 8 bits data
    debit : 115200/10 = 11,520 KB

## Packet Protocol
    24 bytes datagramme.
        1 byte: datagramme size.(0~255);
        19 bytes: payload.
        4 byte: CRC checksum.

    ### Control datagramme
    ACK: length=1 , 1st Byte = 0xAC
    Retransmit: length=1 , 1st Byte = 0xAB
