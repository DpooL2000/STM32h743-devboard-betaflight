#pragma once

#define FC_TARGET_MCU                STM32H743

#define BOARD_NAME                   DEVBOARD
#define MANUFACTURER_ID              USER

// --- CLOCK & USB ---
#ifndef HSE_VALUE
#define HSE_VALUE                    ((uint32_t)25000000)
#endif
#define SYSTEM_HSE_MHZ               25

#define USE_USB_CDC
#define USB_DETECT_PIN               NONE
#define USE_USB_CLOCK_PLL3           // PLL3 provides 48 MHz USB clock from HSE

// --- SENSORS: BMI160 (SPI1) & BMP280 (I2C1) ---
#define USE_ACC
#define USE_GYRO
#define USE_ACCGYRO_BMI160
#define USE_ACC_SPI_BMI160
#define USE_GYRO_SPI_BMI160

#define USE_BARO
#define USE_BARO_BMP280

// BMI160 on SPI1
#define GYRO_1_SPI_INSTANCE          SPI1
#define GYRO_1_CS_PIN                PA4
#define GYRO_1_EXTI_PIN              PC4
#define GYRO_1_ALIGN                 CW0_DEG
#define DEFAULT_GYRO_TO_USE          GYRO_CONFIG_USE_GYRO_1


// BMI160 on SPI2 (Second Gyro)
#define GYRO_2_SPI_INSTANCE          SPI2
#define GYRO_2_CS_PIN                PB12
#define GYRO_2_ALIGN                 CW0_DEG
#define GYRO_2_EXTI_PIN              PD10    

// BMP280 on I2C1 (PB8, PB9)
#define BARO_I2C_INSTANCE            I2CDEV_1
#define I2C1_SCL_PIN                 PB8
#define I2C1_SDA_PIN                 PB9

// --- FLASH MEMORY (QSPI) ---
#define USE_QUADSPI
#define USE_QUADSPI_DEVICE_1         // Activates the default configuration array structure
#define USE_FLASH_QSPI
#define USE_FLASH_W25Q64JV

// Pin definitions matching the quadSpiDefaultConfig macro targets exactly
#define QUADSPI1_SCK_PIN             PB2
#define QUADSPI1_BK1_CS_PIN          PB6
#define QUADSPI1_BK1_IO0_PIN         PD11
#define QUADSPI1_BK1_IO1_PIN         PD12
#define QUADSPI1_BK1_IO2_PIN         PE2
#define QUADSPI1_BK1_IO3_PIN         PD13

// --- SDCARD (SDMMC1) ---
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDIO_DEVICE                  SDIODEV_1
#define SDIO_CK_PIN                  PC12
#define SDIO_CMD_PIN                 PD2
#define SDIO_D0_PIN                  PC8
#define SDIO_D1_PIN                  PC9
#define SDIO_D2_PIN                  PC10
#define SDIO_D3_PIN                  PC11
#define SDCD_PIN                     NONE  // Rely on software polling for card detection

#define DEFAULT_BLACKBOX_DEVICE      BLACKBOX_DEVICE_SDCARD
//#define DEFAULT_BLACKBOX_DEVICE      BLACKBOX_DEVICE_FLASH

// --- ADC ---
#define USE_ADC
#define ADC_VBAT_PIN                 PC1
#define ADC_CURR_PIN                 PC0
#define ADC_RSSI_PIN                 PC5 // not used i dont remember the reason but we discussed somewhere
#define DEFAULT_CURRENT_METER_SOURCE CURRENT_METER_ADC
#define DEFAULT_VOLTAGE_METER_SOURCE VOLTAGE_METER_ADC
#define DEFAULT_CURRENT_METER_SCALE  120
#define DEFAULT_VOLTAGE_METER_SCALE  108

// --- PINS: SPI & I2C ---
#define SPI1_SCK_PIN                 PA5
#define SPI1_SDI_PIN                 PA6
#define SPI1_SDO_PIN                 PA7

#define SPI2_SCK_PIN                 PB13
#define SPI2_SDI_PIN                 PB14
#define SPI2_SDO_PIN                 PB15

// I2C2 is free
#define I2C2_SCL_PIN                 PB10
#define I2C2_SDA_PIN                 PB11

// --- PINS: UARTS ---
// UART1 – used for Serial RX (receiver)
#define UART1_TX_PIN                 PA9
#define UART1_RX_PIN                 PA10

// UART3 – used for ESC telemetry
#define UART3_TX_PIN                 PD8
#define UART3_RX_PIN                 PD9

#define UART6_TX_PIN                 PC6
#define UART6_RX_PIN                 PC7

#define UART7_TX_PIN                 PE8
#define UART7_RX_PIN                 PE7


#define SERIALRX_UART                SERIAL_PORT_UART1
#define ESC_SENSOR_UART              SERIAL_PORT_USART3

// --- PINS: MISC ---
#define BEEPER_PIN                   PC2
#define BEEPER_INVERTED

// --- MOTOR & SERVO ---
// Motors 1-4 stay on original pins
#define MOTOR1_PIN                   PA0
#define MOTOR2_PIN                   PA1
#define MOTOR3_PIN                   PA2
#define MOTOR4_PIN                   PA3

// Motor5 moved to PB0 (free, timer capable) – original PD12 used by QSPI IO1
#define MOTOR5_PIN                   PB0
// Motor6 moved to PB1 (free, timer capable) – original PD13 used by QSPI IO3
#define MOTOR6_PIN                   PB1
#define MOTOR7_PIN                   PD5
#define MOTOR8_PIN                   PD6

// Servo1/2 stay on original pins
#define SERVO1_PIN                   PE5
#define SERVO2_PIN                   PE6
// Servo3 moved to PE1 (free) – original PC8 used by SDIO DAT0
#define SERVO3_PIN                   PE1
// Servo4 moved to PA15 (free) – original PC9 used by SDIO DAT1
#define SERVO4_PIN                   PA15

// --- TIMERS & DMA ---
// Remapped motor/servo timers to new free pins
// Betaflight Format: TIMER_PIN_MAP(index, pin, timer_number, channel_index)
// Channel index is 0-based (0 = CH1, 1 = CH2, 2 = CH3, 3 = CH4)

#define TIMER_PIN_MAPPING                                             \
    TIMER_PIN_MAP( 0,  PA0,  1,  0) /* TIM1_CH1 */                    \
    TIMER_PIN_MAP( 1,  PA1,  1,  1) /* TIM1_CH2 */                    \
    TIMER_PIN_MAP( 2,  PA2,  1,  2) /* TIM1_CH3 */                    \
    TIMER_PIN_MAP( 3,  PA3,  1,  3) /* TIM1_CH4 */                    \
    TIMER_PIN_MAP( 4,  PB0,  2,  1) /* TIM2_CH2 - Replaces Motor 5 */ \
    TIMER_PIN_MAP( 5,  PB1,  2,  2) /* TIM2_CH3 - Replaces Motor 6 */ \
    TIMER_PIN_MAP( 6,  PE1,  3,  0) /* TIM3_CH1 - Replaces Servo 3 */ \
    TIMER_PIN_MAP( 7, PA15,  3,  1) /* TIM3_CH2 - Replaces Servo 4 */

#define ADC1_DMA_OPT                 9
#define ADC3_DMA_OPT                 10
#define TIMUP3_DMA_OPT               11
#define TIMUP4_DMA_OPT               12
#define TIMUP5_DMA_OPT               13
#define DEFAULT_DSHOT_BITBANG        DSHOT_BITBANG_ON