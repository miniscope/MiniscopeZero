# Miniscope-SAMD-Framework

Firmware framework for wireless miniature microscopes (miniscopes) based on Microchip SAM D51 microcontrollers. The framework reads out a CMOS image sensor (ON Semiconductor PYTHON 480 or AMS NanEye) through the parallel capture controller (PCC), buffers image data in MCU RAM, and streams it via DMA to a serial data link (SPI/USART) or an SD card. It also controls miniscope peripherals such as the electrowetting lens (EWL), excitation LED, battery and wireless-power voltage monitoring, an IR receiver for remote control, and status LEDs.

The framework is designed to be used as a git submodule inside a Microchip Studio (Atmel START) project. Pre-build scripts link the framework sources into the generated project so that Atmel START files are never edited directly.

## Repository structure

- `src/`: application sources
- `include/`: headers, including the build configuration (`MS_config.h`)
- `ASF_custom/`: customized Atmel Software Framework (ASF) drivers, stored with `.csrc`/`.hsrc` extensions and installed by the pre-build script
- `script/`: PowerShell scripts that install/uninstall the framework sources into the parent Atmel START project
- `atstart/`: Atmel START configuration archives (`.atzip`) for supported hardware
- `main.csrc`: application entry point, installed as the parent project's `main.c`
- `Doxyfile`, `html/`: Doxygen documentation; open `html/index.html` in a browser after cloning

## Requirements

- Microchip Studio with an Atmel START project targeting a SAM D51 device
- PowerShell (invoked by the pre-build event)

## Setup

1. Create an Atmel START project configured with the drivers and pins listed under [Peripheral requirements](#peripheral-requirements-atmel-start-config). The archives in `atstart/` can be used as a starting point.
2. From the directory containing the project's `main.c`, add this repository as a submodule:
   ```bash
   git submodule add https://github.com/Aharoni-Lab/Miniscope-SAMD-Framework ./MS_module
   ```
3. In the Microchip Studio Solution Explorer, click "Show All Files", right-click `MS_module`, and select "Include in Project".
4. Add the following to the include paths (Project -> Properties -> Toolchain -> ARM/GNU C Compiler -> Directories, configuration: All configurations):
   ```
   ../MS_module/include
   ```
5. Add the following pre-build event (configuration: All configurations):
   ```
   powershell.exe -ExecutionPolicy Bypass -NoProfile -NonInteractive -File "..\MS_module\script\MS_prebuild.ps1"
   ```

The pre-build script replaces the generated `main.c` and selected ASF drivers with the versions in this repository (`main.csrc`, `ASF_custom/`) using hard links. Before re-running Atmel START code generation, run `script/MS_pre_reconfig.ps1` to restore the original files.

Do not modify Atmel START generated driver files directly. To customize an ASF driver, add the modified file to `ASF_custom/` (with a `.csrc`/`.hsrc` extension) and register its path in `$cfilepatharray`/`$headerpatharray` in `MS_prebuild.ps1` and `MS_pre_reconfig.ps1`.

## Configuration

All build configuration is done with compile-time flags in `include/MS_config.h`.

### Mode flags

Select exactly one hardware mode (`*_MODE` or `*_TESTMODE`), which determines the data path and the set of peripherals compiled in:

```c
// ------ HARDWARE MODE ------------------------
//#define V4WF_MODE
#define WLMS_SPI_MODE
//#define BERT_MODE
//#define GS_MODE
//#define WLMS_USART_MODE
//#define WLMS_SD_MODE
//#define DMA_TO_SPI_TESTMODE
//#define DMA_TO_SPI_METRO_TESTMODE
```

For example, `WLMS_SPI_MODE` streams PYTHON 480 image data over SPI, `WLMS_SD_MODE` records to an SD card, and `BERT_MODE` streams a PRBS test pattern for bit-error-rate testing of the data link.

### Peripheral flags

Each mode defines the peripherals to enable (`*_ENABLE` / `*_DISABLE`) together with mode-specific parameters (buffer sizes, frame rate, sensor ROI, device ID, etc.):

```c
#ifdef WLMS_SPI_MODE
#define PYTHON480_ENABLE
#define EWL_ENABLE
#define DMA_TO_SPI_ENABLE
#define EXLED_PWM_ENABLE
#define BATTERY_ENABLE
// ...
#endif
```

### Conditional compilation

Peripheral code should be guarded by the peripheral's `_ENABLE` flag (not by mode flags):

```c
#ifdef PYTHON480_ENABLE
gpio_set_pin_level(EN_3V3, true); // Enable the 3.3V regulator
I2C_BB_init();
#endif
```

### SERCOM for data output

The SERCOM used for serial data output is configured in Atmel START. On the firmware side, select the corresponding `SPI_SERCOMx_ENABLE` or `USART_SERCOMx_ENABLE` flag in the mode definition; the DMA destination register is resolved from this flag (see `src/MS_global_variable.c`).

## Peripheral requirements (Atmel START config)

### PYTHON480_ENABLE

Drivers:

```
TIMER_0
CAMERA_0
EXTERNAL_IRQ_0
```

Pins:

```
SPI_BB_SCK
SPI_BB_MOSI
SPI_BB_MISO
SPI_BB_NSS
PCC_D0,...,PCC_D7
PCC_CLK
PCC_HV
PCC_FV
FrameValid
EN_3V3
RESET_CMOS
MONITOR0
GCLK1_OUT
```

### BATTERY_ENABLE

```
BATT_VOLT
```

### WPT_ADC_ENABLE

```
WPT_VOLT
```

### EWL_ENABLE

```
I2C_BB_SCL
I2C_BB_SDA
```

### IR_TRIGGER_ENABLE / IR_UART_ENABLE

```
IR_RX
```

### EXLED_PWM_ENABLE

```
LED_PWM
ENT_LED
```

## Documentation

API documentation is generated with Doxygen and committed under `html/`; open `html/index.html` in a browser. To regenerate, run `doxygen Doxyfile` in the repository root.

## License

This project is licensed under the GNU Affero General Public License v3.0; see [LICENSE](LICENSE). The modified ASF driver files in `ASF_custom/` are derived from the Atmel Software Framework and retain their original Microchip/Atmel license headers.
