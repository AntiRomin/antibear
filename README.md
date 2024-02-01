# Antibear

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0) 

Antibear is an arm-cortex M7 core MCU learning project. It used STM32H743XI as MCU. The board was made by embedfire.

## Events

| Date  | Event |
| - | - |
| 07-04-2023 | Init the repo |
| 09-26-2023 | Added Debug serial |
| 09-28-2023 | Ported FreeRTOS |
| 01-12-2024 | Added USB MSC |
| 01-17-2024 | Ported FatFs |

## Plans
1. Port FreeRTOS (Finished)
2. Port FatFs (Finished)
3. Use MDMA or DMA for QSPI FLASH Read and Write (Optional)
4. Write QSPI Read and Write Function (Finished)
5. Initialize QSPI DUAL FLASH (Finished)
6. Initialize SDRAM at systemInit (Finished)
7. Add RTC (Finished)
8. Add I2C Bus driver for EEPROM
9. Add CLI

## Goal
1. Learning makefile and arm gcc compiler
2. Learning arm-cortex M7 core resource
3. Learning FMC
4. Learning LCD driver
5. Learning Touch pad driver
6. Learning USB driver(USB Audio and USB MSC)
7. Learning GUI
8. Learning RTOS
9. To be added later

## Caution
I have read the source code of betaflight and it is highly recommended as a source code for learning makefile and mcu development. So there may be shades of betaflight code in my code.I also recommend you to find some valuable open source code to read and learn.