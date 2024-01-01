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
    Packet size 8 bits
    Baud rate 115200
