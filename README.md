# STM32H743 DevBoard Flight Controller (Betaflight Target)

This repository provides an open-source engineering blueprint, custom target files, and a 4-layer hardware carrier board design to run standard Betaflight firmware on a generic **DevEBox STM32H743VIT6 development board**. 

Unlike commercial flight controllers that limit you to pre-configured layouts, utilizing a standard industrial developer board breaks open full control of the silicon. This project serves as a definitive architectural guide to overcoming severe bare-metal implementation bugs, remapping timers for advanced multi-rotor and actuator configurations, and running ultra-low latency PID loops on a highly affordable H7 platform.

---

## Hardware Architecture Manifest

The target configuration files provided here are explicitly tailored and tested against the following component topology and hardware pin allocation mapping:

* **Microcontroller Core:** STM32H743VIT6 (ARM Cortex-M7 running at a stable 200MHz bus clock).
* **Primary IMU (SPI1):** BMI160 Gyro/Accelerometer module routed to native SPI1 pins:
  * `PA4` (Chip Select / CS)
  * `PA5` (SCK), `PA6` (MISO / SDI), `PA7` (MOSI / SDO)
  * Dedicated `PC4` EXTI hardware interrupt line synchronization.
* **Secondary IMU (SPI2):** BMI160 Gyro/Accelerometer module mapped onto isolated native SPI2 pins:
  * `PB12` (Chip Select / CS)
  * `PB13` (SCK), `PB14` (MISO / SDI), `PB15` (MOSI / SDO)
  * Dedicated `PD10` EXTI hardware interrupt line synchronization.
* **Barometer (I2C1):** BMP280 module routed to `PB8` (SCL) and `PB9` (SDA), configured via hardware jumpers on the breakout module to default I2C Address `0x76`.
* **Blackbox Telemetry:** High-speed onboard SDIO MicroSD card slot combined with an onboard QSPI external flash memory layer (`W25Q64JV`):
  * **SDIO (SDMMC1):** `PC12` (CK), `PD2` (CMD), `PC8` (D0), `PC9` (D1), `PC10` (D2), `PC11` (D3). Card detection is handled via software polling (`SDCD_PIN` set to `NONE`).
  * **QSPI:** `PB2` (CLK), `PB6` (CS), `PD11` (IO0), `PD12` (IO1), `PE2` (IO2), `PD13` (IO3).
* **Power Supply Module Integration (GM V1.0):** Integrated hardware telemetry lines from the buck-regulated external power module routed directly to the microcontroller's high-precision 16-bit ADC peripheral interface:
  * **Voltage Sensing (V_SENS):** Routed to analog input pin `PC1` (`ADC_VBAT_PIN`).
  * **Current Sensing (I_SENS):** Routed to analog input pin `PC0` (`ADC_CURR_PIN`).
* **Actuator Layout & Communications Matrix:**
  * **Motors (1–8):** `PA0` (Motor 1), `PA1` (Motor 2), `PA2` (Motor 3), `PA3` (Motor 4), `PB0` (Motor 5, timer remapped), `PB1` (Motor 6, timer remapped), `PD5` (Motor 7), `PD6` (Motor 8).
  * **Servos (1–4):** `PE5` (Servo 1), `PE6` (Servo 2), `PE1` (Servo 3, timer remapped), `PA15` (Servo 4, timer remapped).
  * **Serial Peripherals:** `UART1` (`PA9` TX, `PA10` RX) dedicated to Serial RX; `UART3` (`PD8` TX, `PD9` RX) assigned to ESC Telemetry; `UART6` (`PC6` / `PC7`) and `UART7` (`PE8` / `PE7`) exposed for general telemetry/VTX expansion.
  * **Peripherals:** Beeper output assigned to `PC2` (inverted configuration).
* **PCB Stackup:** 4-Layer layout featuring two dedicated, uninterrupted internal Ground Planes (`In1.Cu` and `In2.Cu`) positioned exactly 0.1mm below the surface signal layers to establish immediate return path loops and maximize EMI shielding against heavy motor bus noise.

---

# Bare-Metal Hardware Troubleshooting

Bringing Betaflight up on standard industrial development boards reveals discrepancies between consumer flight controllers and raw silicon layouts. Below is the technical documentation of the three critical bare-metal failures encountered during hardware debugging, along with their permanent firmware solutions.

---

### Bug 3.1: 25MHz HSE Crystal & PLL1 Clock Configuration Failures

**The Bug:**
Standard Betaflight STM32H7 codebases are optimized for 8MHz or 24MHz external crystals. The DevEBox hardware utilizes a 25MHz HSE. Applying default configuration multipliers caused the internal voltage-controlled oscillator (VCO) to hit 3000MHz, which drastically exceeds the 960MHz hardware limit. This caused the phase-locked loop engine to fail, triggering an infinite safety reset loop inside `HandleStuckSysTick()`.

**The Structural Fix:**
The internal clock structures (`pll1ConfigRevY` and `pll1ConfigRevV`) inside `src/platform/STM32/startup/system_stm32h7xx.c` must be rewritten to scale the 25MHz crystal safely:

```c
// Adjusted clock parameters for a stable 200MHz SYSCLK
.clockMhz = 200,
.m = 25,     // 25MHz HSE / 25 = 1MHz reference
.n = 400,    // 1MHz * 400 = 400MHz VCO frequency
.p = 2,      // 400MHz VCO / 2 = 200MHz SYSCLK
.vos = PWR_REGULATOR_VOLTAGE_SCALE1
```

### Bug 3.2: Missing HSI48 Internal Oscillator & PLL3 USB Solution

**The Bug:**
To establish a stable USB Virtual COM Port (VCP), the STM32 USB peripheral requires a 48MHz clock stream. Betaflight natively attempts to activate the internal HSI48 oscillator. However, hardware debugging proved the DevEBox completely ignores `RCC_CR_HSI48ON` register writes, leaving the USB peripheral without a clock signal and failing to enumerate.

**The Structural Fix:**
Bypass the dead internal HSI48 circuit and synthesize a 48MHz USB clock using the primary 25MHz HSE crystal routed through the PLL3 engine.

Define the macro override in your local `config.h`:

```c
#define USE_USB_CLOCK_PLL3
```

Apply these specific division parameters inside the USB initialization block of `system_stm32h7xx.c` to step the 25MHz HSE down to an exact 48MHz envelope:

```c
PLL3M = 5,  // 25MHz HSE / 5 = 5MHz reference
PLL3N = 48, // 5MHz * 48 = 240MHz VCO3
PLL3Q = 5   // 240MHz VCO3 / 5 = 48MHz USB output clock
```

---

### Bug 3.3: Default BMI160 I2C Interface Boot Lockout

**The Bug:**
The BMI160 IMU boots into I2C communication mode by default. Because stock Betaflight sensor drivers expect the device to natively listen on the SPI bus, the initial `WHO_AM_I` registry read over SPI is ignored by the sensor. The firmware assumes the gyro is dead and disables the PID loop.

**The Structural Fix:**
Force the sensor onto the high-speed SPI interface by executing a manual Chip Select (CS) edge transition sequence before attempting any communication.

Patch the initialization routine inside `src/main/drivers/accgyro/accgyro_spi_bmi160.c` within the `bmi160Detect()` function:

```c
// Force BMI160 interface reconfiguration from I2C mode to native SPI
IOWrite(csPin, true);
delay(10); 
IOWrite(csPin, false); // Falling edge tells the sensor to look for SPI frames
delay(10); 
IOWrite(csPin, true);  // Interface locked into SPI mode
```

# Firmware Source Tree Modification & Compilation Guide

To replicate this build and compile the custom DevEBox target, follow these exact modifications within the Betaflight source tree.

### 1. Target Directory Setup
Create a new directory named `DEVBOARD` inside `src/main/target/`. 
Place your custom `target.h`, `target.c`, and `config.h` files into this new folder.

### 2. Core Source Modifications
You must manually patch the following core Betaflight files to implement the bare-metal hardware fixes detailed in Section 2:

**File: `src/platform/STM32/startup/system_stm32h7xx.c`**
* Locate the `pll1ConfigRevY` and `pll1ConfigRevV` structures. Update the multipliers for the 25MHz HSE: set `.m = 25`, `.n = 400`, and `.p = 2`.
* Locate the USB clock configuration block. Comment out the `HSI48` initialization and insert the PLL3 dividers: `PLL3M = 5`, `PLL3N = 48`, and `PLL3Q = 5`.
* Locate the `SystemInit()` function and comment out the `memProtConfigure()` call to prevent early boot memory protection faults on this specific developer silicon.

**File: `src/main/drivers/accgyro/accgyro_spi_bmi160.c`**
* Locate the `bmi160Detect()` function.
* Inject the manual Chip Select (CS) toggle sequence (`IOWrite(csPin, true)`, delay, `false`, delay, `true`) immediately before the first `WHO_AM_I` registry read to force the sensor out of I2C mode and into SPI mode.

**File: `src/main/sensors/gyro_init.c`**
* Locate the primary gyro initialization sequence. 
* Add a fallback condition to directly call `bmi160Detect` mapped to SPI1 if the hardware device list returns empty during the initial boot scan.

### 3. Compilation & Flashing
Once the source files are patched and your target directory is established, open your terminal in the Betaflight root directory and execute the standard make command:

`make TARGET=DEVBOARD`

Once compilation is complete, flash the resulting `.hex` file to your STM32H743 via STM32CubeProgrammer or the Betaflight Configurator while the board is in DFU mode.

# Power Distribution & Critical Sensor Calibration

### 1. Power Supply Topology
This target configuration assumes the system is powered by an external step-down switching regulator delivering a stable 5V rail to the developer board. The onboard AMS1117-3.3 LDO regulator drops this input to 3.3V for the STM32H7 core and sensor buses, providing robust immunity against heavy Li-ion voltage sag down to a structural threshold of 4.4V.

### 2. Telemetry & ADC Scaling (User Calibration Required)
To achieve real-time battery status monitoring, the analog Voltage (V_SENS) and Current (I_SENS) lines from your power module must be routed to the microcontroller's ADC pins (`PC1` for Voltage, `PC0` for Current).

**Mandatory Calibration Protocol:**
Do not rely blindly on stock software scale parameters. Every power module utilizes different internal resistor divider networks, and component manufacturing tolerances fluctuate. 

As a structural baseline, this target defaults to our specific hardware mapping (a GM V1.0 power module with a 10.76:1 voltage divider):

```text
set vbat_scale = 108
set current_meter_scale = 120
save
```

⚠️ Before your first test flight, you must manually calibrate your hardware:

1. Connect your fully assembled battery pack to the drone and measure the raw voltage at the main XT60 bus using a calibrated bench digital multimeter.
2. Boot into the Betaflight Configurator GUI and check the reported voltage reading on the main Setup dashboard.
3. Micro-adjust the vbat_scale numerical values up or down in the CLI tab (or the Power & Battery tab) until the software telemetry read-out matches your multimeter's hardware voltage precisely.
4. Perform a similar verification for the current sensor by measuring the draw of a known load and adjusting the current_meter_scale accordingly.
