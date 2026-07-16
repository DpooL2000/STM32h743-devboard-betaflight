# STM32H743 DevBoard Flight Controller (Betaflight Target)

This repository provides an open-source engineering blueprint, custom target files, and a 4-layer hardware carrier board design to run standard Betaflight firmware on a generic **DevEBox STM32H743VIT6 development board**. 

Unlike commercial flight controllers that limit you to pre-configured layouts, utilizing a standard industrial developer board breaks open full control of the silicon. This project serves as a definitive architectural guide to overcoming severe bare-metal implementation bugs, remapping timers for advanced multi-rotor and actuator configurations, and running ultra-low latency PID loops on a highly affordable H7 platform.

---

## 🛠 Hardware Architecture Manifest

The target configuration files provided here are explicitly tailored and tested against the following component topology:

* **Microcontroller Core:** STM32H743VIT6 (ARM Cortex-M7 running at a stable 200MHz bus clock).
* **Primary IMU (SPI1):** BMI160 Gyro/Accelerometer module routed to native SPI1 pins (`PA4`, `PA5`, `PA6`, `PA7`) with dedicated `PC4` EXTI hardware interrupt line synchronization.
* **Secondary IMU (SPI2):** BMI160 Gyro/Accelerometer module mapped onto isolated native SPI2 pins (`PB12`, `PB13`, `PB14`, `PB15`) to isolate high-speed burst sensor reads over dedicated DMA streams.
* **Barometer (I2C1):** BMP280 module routed to `PB8` (SCL) and `PB9` (SDA), configured via hardware jumpers on the breakout module to default $I^2C$ Address `0x76`.
* **Blackbox Telemetry:** High-speed onboard SDIO MicroSD card slot (`PC8`, `PC9`, `PC10`, `PC11`, `PC12`, `PD2`) combined with an onboard QSPI external flash memory layer (`W25Q64JV` on `PB2`, `PB6`, `PD11`, `PD12`, `PD13`, `PE2`).
* **PCB Stackup:** 4-Layer layout featuring two dedicated, uninterrupted internal Ground Planes (`In1.Cu` and `In2.Cu`) positioned exactly 0.1mm below the surface signal layers to establish immediate return path loops and maximize EMI shielding against heavy motor bus noise.

---

## ⚡ Critical System Calibration & Power Architecture

### 1. Power Supply Topology
This target configuration assumes the system is powered by an external step-down switching regulator (such as a generic or FPV-grade power distribution module) delivering a stable, low-ripple rail to the primary 5V pins of the developer board. The onboard linear LDO regulator (AMS1117-3.3) drops this input down to 3.3V for the STM32H7 core and sensor busses, providing robust immunity against heavy LiPo/Li-ion voltage sag down to a structural threshold of 4.4V.

### 2. Telemetry & ADC Scaling (User Calibration Required)
To achieve real-time battery status monitoring inside your Betaflight OSD, the analog Voltage and Current sensing lines from your power module must be routed to the microcontroller's high-precision 16-bit ADC pins:
* **Voltage Telemetry (V_SENS):** Routed to pin `PC1`.
* **Current Telemetry (I_SENS):** Routed to pin `PC0`.

#### ⚠️ Mandatory Calibration Protocol:
Do not rely blindly on stock software scale parameters. Every power module utilizes different internal resistor divider networks (e.g., standard 10.1:1 vs. custom 10.76:1 dividers), and component manufacturing tolerances fluctuate. 

As a structural baseline, this target defaults to a reference scale mapping:
```text
set vbat_scale = 108
set current_meter_scale = 120
```

# Section 2: The Debugging Chronicles (Bare-Metal Troubleshooting)

Bringing Betaflight up on standard industrial development boards reveals discrepancies between consumer flight controllers and raw silicon layouts. Below is the technical documentation of the three critical bare-metal failures encountered during hardware debugging, along with their permanent firmware solutions.

---

### Case Study 2.1: The 25MHz HSE Crystal & PLL1 Lock Loop

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
