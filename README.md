# STM32H743 DevBoard Flight Controller (Betaflight Target)

An open-source, high-performance 4-layer custom carrier board blueprint that transforms a standard STM32H743 developer board into a fully ruggedized, dual-IMU flight controller optimized for heavy-lift utility drones.

## 🛠 Hardware Architecture Specifications

* **MCU:** STM32H743VIT6 running at 480MHz
* **Power Stack:** GM V1.0 Power Module (Step-down buck converter delivering 5.07V rigid rail)
* **Primary IMU (SPI1):** BMI160 (Native pin routing with EXTI hardware interrupt synchronization)
* **Secondary IMU (SPI2):** BMI160 (Isolated high-speed SPI2 peripheral routing)
* **Barometer (I2C1):** BMP280 configured via hardware jumpers to Address `0x76`
* **Blackbox Logging:** High-speed onboard SDIO MicroSD card slot + QSPI external flash option
* **PCB Stackup:** 4-Layer design with continuous internal Ground Planes (`In1.Cu` & `In2.Cu`) for maximum EMI shielding.

---

## 💾 Custom Target Configuration (`target.h`)

Drop this unified header file into your local Betaflight source directory (`src/main/target/DEVBOARD/`) before compiling:

[INSERT CODE HERE]

---

## ⚡ Power & Telemetry Integration Setup

### Voltage Sensing Scaler Calibration
Due to the precise 10.76:1 analog voltage divider circuit integrated into the GM V1.0 power module, the default battery scale multiplier must be overridden in the CLI.

With a live 4S pack providing a calibrated 15.07V reading on the multi-meter, the analog sensing pin outputs exactly 1.40V to the STM32 high-precision 16-bit ADC. Run these commands to synchronize your OSD dashboard telemetry:

```text
resource ADC_BATT 1 C01
set vbat_scale = 108
save
