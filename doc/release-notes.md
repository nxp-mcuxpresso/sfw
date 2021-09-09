# MCU-OTA SFW Release Notes

## --- Version 1.0

### What's new

This is the first release of MCU-OTA SFW, a secure firmware for NXP MCU-OTA project.
It is designed based on FreeRTOS, NXP SDK and other functional modules, can work with
MCU-OTA SBL(secure bootloader) to provide a complete secure OTA solution. The SFW
supports OTA via SD card, U-Disk, AWS cloud or Aliyun cloud. The SFW can be built with
GCC toolchain in Linux/Windows, or IAR, MDK IDEs conveniently.

### Platforms

- evkmimxrt500
- evkmimxrt600
- evkmimxrt1010
- evkmimxrt1020
- evkbmimxrt1050
- evkmimxrt1060
- evkmimxrt1064
- evkmimxrt1170
- lpc55s69

### Framework

- SCons based on Python environment for both Windows and Linux
- Self-contained Python environment for Windows
- Conveniently create IAR, MDK project by SCons extended commands
- High and easy scalability via Kconfig mechanism for both Windows and Linux


### Toolchain

- Linux: GCC_ARM
- Windows: GCC_ARM, IAR, MDK

### FOTA

- Local FOTA via SD Card or U-Disk
- Remote FOTA via AWS or Aliyun cloud

### Security

- Version check
- SFW image signature and encryption
- Mbedtls with NXP soc hardware accelerate engine

### Known issues
