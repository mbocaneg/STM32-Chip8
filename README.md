# STM32-Chip8
A threaded Chip8 core implemented on an ESTM32 microcontroller that runs under FreeRtos. Play retro video games in the palm of you hand!

# Usage

This entire project targets the STM32 microcontroller using CubeMX middleware from ST. As far as
libraries go, I am using my homegrown CHIP8 core, Keypad, and LCD libraries.

The entire bill of materials is minimal and includes only the following:

* STM32F1 MCU (project should work on ANY STM32 microcontroller with minimal code refactoring)
* SSD1306 OLED display(I2C variant in this case)
* Hex button keypad(8 leads)
* 3.3V power source(in my case, a Lipo battery)

# Schematic 


