# Embedded Programming — ESP32

Bare-metal embedded programming projects targeting the **ESP32-WROOM** microcontroller. Built in C and Xtensa Assembly using the ESP-IDF toolchain — no Arduino, no RTOS, just registers and datasheets.

## Projects

### Blinking LED with Timer Interrupts

A GPIO-controlled LED driven by hardware timer interrupts instead of software delays.

- Direct register manipulation for GPIO configuration
- Hardware timer setup using ESP32 Timer Group registers
- Interrupt Service Routine (ISR) registration via the interrupt matrix
- No FreeRTOS, no HAL abstractions — fully bare-metal

## Toolchain & Environment

| Component   | Details                                    |
|-------------|--------------------------------------------|
| **MCU**     | ESP32-WROOM-32                             |
| **Framework** | ESP-IDF (toolchain and flashing only)    |
| **Language** | C, Xtensa Assembly                        |
| **OS**      | Arch Linux                                 |
| **Build**   | CMake + `idf.py`                           |

## Building & Flashing

```bash
cd blinking_led
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

## Roadmap

- [x] GPIO + Timer interrupt LED blink
- [ ] Preemptive task scheduler
- [ ] Context switching in Xtensa Assembly
- [ ] UART communication
- [ ] SPI peripheral driver

## References

- [ESP32 Technical Reference Manual](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)
- [ESP-IDF Programming Guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/)
