# LIS302DL STM32 HAL Driver

A lightweight STM32 HAL driver for the **LIS302DL** 3-axis digital accelerometer using **4-wire SPI**.

The driver supports device initialization, register read/write access, raw acceleration reading, conversion to `g`, data-ready checking, overrun checking, and basic runtime configuration. You can access the datasheet from [here](https://storm.cis.fordham.edu/~gweiss/wisdm-papers/Ipod-accelerometer.pdf).

## Features

* STM32 HAL compatible
* 4-wire SPI communication
* WHO_AM_I device ID validation
* Configurable output data rate:

  * 100 Hz
  * 400 Hz
* Configurable full-scale range:

  * ±2g
  * ±8g
* X/Y/Z axis enable control
* Power-down and active mode support
* Self-test mode support
* Raw 8-bit acceleration reading
* Acceleration conversion to `g`
* Data-ready and overrun status checking
* Chip-select pin is configurable through the driver handle
* No hard-coded GPIO port or pin inside the driver

## Device

| Parameter                   | Value             |
| --------------------------- | ----------------- |
| Sensor                      | LIS302DL          |
| Interface                   | SPI               |
| SPI mode                    | 4-wire SPI        |
| Axes                        | X, Y, Z           |
| Output format               | 8-bit signed data |
| WHO_AM_I value              | `0x3B`            |
| Supported full-scale ranges | ±2g, ±8g          |
| Supported output data rates | 100 Hz, 400 Hz    |

## Requirements

* STM32 microcontroller
* STM32CubeIDE or STM32 HAL-based project
* LIS302DL accelerometer
* SPI peripheral configured in STM32CubeMX
* A GPIO output pin for chip select

## SPI Configuration

Configure the SPI peripheral as follows:

| Setting             | Value                                                    |
| ------------------- | -------------------------------------------------------- |
| Mode                | Full-Duplex Master                                       |
| Data Size           | 8-bit                                                    |
| First Bit           | MSB First                                                |
| Clock Polarity      | High                                                     |
| Clock Phase         | 2 Edge                                                   |
| NSS                 | Software                                                 |
| Baud Rate Prescaler | Start with a conservative value, then increase if stable |

Example STM32 HAL configuration:

```c
hspi1.Init.Mode = SPI_MODE_MASTER;
hspi1.Init.Direction = SPI_DIRECTION_2LINES;
hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
hspi1.Init.NSS = SPI_NSS_SOFT;
hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
```

The chip-select pin must be configured as a regular GPIO output. The driver controls CS manually.

## Wiring Example

| LIS302DL Pin | STM32 Pin          |
| ------------ | ------------------ |
| VDD          | 3.3V               |
| VDD_IO       | 3.3V               |
| GND          | GND                |
| SPC / SCL    | SPI SCK            |
| SDI / SDA    | SPI MOSI           |
| SDO          | SPI MISO           |
| CS           | GPIO Output        |
| INT1         | Optional GPIO/EXTI |
| INT2         | Optional GPIO/EXTI |

The exact STM32 pins depend on your board and CubeMX configuration.

## Installation

Copy these files into your STM32 project:

```text
Core/Inc/lis302dl.h
Core/Src/lis302dl.c
```

Then include the header file:

```c
#include "lis302dl.h"
```

## Basic Usage

### 1. Create a driver handle

```c
LIS302DL_Handle_t lis302dl;
```

### 2. Initialize the sensor with default configuration

```c
LIS302DL_Status_t status;

status = LIS302DL_Init(&lis302dl,
                       &hspi1,
                       GPIOE,
                       CS_Pin,
                       NULL);

if (status != LIS302DL_OK) {
    Error_Handler();
}
```

Passing `NULL` as the configuration argument uses the default configuration:

| Option           | Default  |
| ---------------- | -------- |
| Output data rate | 400 Hz   |
| Full scale       | ±2g      |
| Power mode       | Active   |
| Self-test        | Disabled |
| X axis           | Enabled  |
| Y axis           | Enabled  |
| Z axis           | Enabled  |

### 3. Read acceleration in `g`

```c
float accelG[3];

if (LIS302DL_ReadG(&lis302dl, accelG) == LIS302DL_OK) {
    float x = accelG[0];
    float y = accelG[1];
    float z = accelG[2];
}
```

### 4. Read only when new data is available

```c
bool ready = false;
float accelG[3];

if (LIS302DL_IsDataReady(&lis302dl, &ready) == LIS302DL_OK) {
    if (ready) {
        if (LIS302DL_ReadG(&lis302dl, accelG) == LIS302DL_OK) {
            /*
             * accelG[0] = X axis in g
             * accelG[1] = Y axis in g
             * accelG[2] = Z axis in g
             */
        }
    }
}
```

## Custom Configuration

You can create a custom configuration before initialization.

```c
LIS302DL_Config_t config;

LIS302DL_GetDefaultConfig(&config);

config.dataRate = LIS302DL_ODR_100HZ;
config.fullScale = LIS302DL_FULL_SCALE_8G;
config.powerMode = LIS302DL_ACTIVE;
config.selfTest = LIS302DL_SELF_TEST_DISABLE;

config.enableX = true;
config.enableY = true;
config.enableZ = true;

LIS302DL_Status_t status = LIS302DL_Init(&lis302dl,
                                         &hspi1,
                                         GPIOE,
                                         CS_Pin,
                                         &config);

if (status != LIS302DL_OK) {
    Error_Handler();
}
```

## Reading Raw Data

The LIS302DL outputs signed 8-bit raw acceleration values.

```c
int8_t rawData[3];

if (LIS302DL_ReadRaw(&lis302dl, rawData) == LIS302DL_OK) {
    int8_t rawX = rawData[0];
    int8_t rawY = rawData[1];
    int8_t rawZ = rawData[2];
}
```

## Converting Raw Data to g

```c
int8_t rawData[3];
float gData[3];

if (LIS302DL_ReadRaw(&lis302dl, rawData) == LIS302DL_OK) {
    LIS302DL_ConvertRawToG(&lis302dl, rawData, gData);
}
```

Sensitivity values used by the driver:

| Full Scale | Sensitivity   |
| ---------- | ------------- |
| ±2g        | `0.018 g/LSB` |
| ±8g        | `0.072 g/LSB` |

## Runtime Configuration

### Change output data rate

```c
LIS302DL_SetDataRate(&lis302dl, LIS302DL_ODR_100HZ);
LIS302DL_SetDataRate(&lis302dl, LIS302DL_ODR_400HZ);
```

### Change full-scale range

```c
LIS302DL_SetFullScale(&lis302dl, LIS302DL_FULL_SCALE_2G);
LIS302DL_SetFullScale(&lis302dl, LIS302DL_FULL_SCALE_8G);
```

### Change power mode

```c
LIS302DL_SetPowerMode(&lis302dl, LIS302DL_POWER_DOWN);
LIS302DL_SetPowerMode(&lis302dl, LIS302DL_ACTIVE);
```

### Enable or disable axes

```c
LIS302DL_SetAxes(&lis302dl, true, true, true);    // Enable X, Y, Z
LIS302DL_SetAxes(&lis302dl, true, false, false);  // Enable only X
```

### Set self-test mode

```c
LIS302DL_SetSelfTest(&lis302dl, LIS302DL_SELF_TEST_DISABLE);
LIS302DL_SetSelfTest(&lis302dl, LIS302DL_SELF_TEST_M);
LIS302DL_SetSelfTest(&lis302dl, LIS302DL_SELF_TEST_P);
```

Do not enable both self-test modes at the same time. The driver prevents this by using a single self-test enum.

## API Reference

### Configuration

```c
LIS302DL_Status_t LIS302DL_GetDefaultConfig(LIS302DL_Config_t *config);
```

Loads the default driver configuration.

```c
LIS302DL_Status_t LIS302DL_Init(LIS302DL_Handle_t *dev,
                                SPI_HandleTypeDef *hspi,
                                GPIO_TypeDef *csPort,
                                uint16_t csPin,
                                const LIS302DL_Config_t *config);
```

Initializes the LIS302DL, validates the device ID, and applies the selected configuration.

### Register Access

```c
LIS302DL_Status_t LIS302DL_ReadRegister(LIS302DL_Handle_t *dev,
                                        uint8_t reg,
                                        uint8_t *data);
```

Reads a single register.

```c
LIS302DL_Status_t LIS302DL_WriteRegister(LIS302DL_Handle_t *dev,
                                         uint8_t reg,
                                         uint8_t data);
```

Writes a single register.

```c
LIS302DL_Status_t LIS302DL_ReadWhoAmI(LIS302DL_Handle_t *dev,
                                      uint8_t *id);
```

Reads the device identification register.

### Status

```c
LIS302DL_Status_t LIS302DL_ReadStatus(LIS302DL_Handle_t *dev,
                                      uint8_t *status);
```

Reads the status register.

```c
LIS302DL_Status_t LIS302DL_IsDataReady(LIS302DL_Handle_t *dev,
                                       bool *ready);
```

Checks whether a new X/Y/Z acceleration data set is available.

```c
LIS302DL_Status_t LIS302DL_IsOverrun(LIS302DL_Handle_t *dev,
                                     bool *overrun);
```

Checks whether unread data has been overwritten.

### Acceleration Reading

```c
LIS302DL_Status_t LIS302DL_ReadRaw(LIS302DL_Handle_t *dev,
                                   int8_t rawData[3]);
```

Reads raw X, Y, and Z acceleration values.

```c
LIS302DL_Status_t LIS302DL_ConvertRawToG(LIS302DL_Handle_t *dev,
                                         const int8_t rawData[3],
                                         float gData[3]);
```

Converts raw acceleration data to `g`.

```c
LIS302DL_Status_t LIS302DL_ReadG(LIS302DL_Handle_t *dev,
                                 float gData[3]);
```

Reads raw acceleration data and converts it to `g`.

### Runtime Settings

```c
LIS302DL_Status_t LIS302DL_SetDataRate(LIS302DL_Handle_t *dev,
                                       LIS302DL_DataRate_t dataRate);
```

Changes the output data rate.

```c
LIS302DL_Status_t LIS302DL_SetFullScale(LIS302DL_Handle_t *dev,
                                        LIS302DL_FullScale_t fullScale);
```

Changes the full-scale range.

```c
LIS302DL_Status_t LIS302DL_SetPowerMode(LIS302DL_Handle_t *dev,
                                        LIS302DL_PowerMode_t powerMode);
```

Changes the power mode.

```c
LIS302DL_Status_t LIS302DL_SetAxes(LIS302DL_Handle_t *dev,
                                   bool enableX,
                                   bool enableY,
                                   bool enableZ);
```

Enables or disables the X, Y, and Z axes.

```c
LIS302DL_Status_t LIS302DL_SetSelfTest(LIS302DL_Handle_t *dev,
                                       LIS302DL_SelfTest_t selfTest);
```

Sets the self-test mode.

## Status Codes

| Status                   | Meaning                                     |
| ------------------------ | ------------------------------------------- |
| `LIS302DL_OK`            | Operation completed successfully            |
| `LIS302DL_ERROR`         | Generic error                               |
| `LIS302DL_NULL_PARAM`    | Null pointer parameter                      |
| `LIS302DL_SPI_ERROR`     | SPI communication failed                    |
| `LIS302DL_ID_MISMATCH`   | WHO_AM_I value is not `0x3B`                |
| `LIS302DL_INVALID_PARAM` | Invalid function or configuration parameter |

## Example Main Loop

```c
LIS302DL_Handle_t lis302dl;
float accelG[3];

void App_Init(void)
{
    if (LIS302DL_Init(&lis302dl,
                      &hspi1,
                      GPIOE,
                      CS_Pin,
                      NULL) != LIS302DL_OK) {
        Error_Handler();
    }
}

void App_Loop(void)
{
    bool ready = false;

    if (LIS302DL_IsDataReady(&lis302dl, &ready) == LIS302DL_OK && ready) {
        if (LIS302DL_ReadG(&lis302dl, accelG) == LIS302DL_OK) {
            /*
             * Use accelG[0], accelG[1], accelG[2]
             */
        }
    }
}
```

## Troubleshooting

### `LIS302DL_Init()` returns `LIS302DL_ID_MISMATCH`

The driver expected `WHO_AM_I = 0x3B`, but received a different value.

Check:

* SPI clock polarity and phase
* MISO/MOSI/SCK wiring
* CS pin wiring
* CS GPIO configuration
* Sensor power supply
* Whether the correct SPI instance is passed to the driver
* Whether the sensor is actually LIS302DL

### SPI communication fails

Check:

* SPI peripheral is initialized before calling `LIS302DL_Init()`
* CS pin is configured as GPIO output
* SPI pins are assigned correctly in CubeMX
* Baud rate is not too high
* Sensor is powered at a valid voltage level

### Acceleration values are always zero

Check:

* Device is in active mode
* X/Y/Z axes are enabled
* `LIS302DL_ReadRaw()` return status
* SPI wiring
* `WHO_AM_I` result
* Whether data-ready is being checked correctly

### Values look too small or too large

Check:

* Full-scale configuration
* Sensitivity value
* Whether raw data is interpreted as signed `int8_t`
* Sensor orientation
* Mechanical mounting

## Notes

* The driver currently supports basic acceleration reading and runtime configuration.
* Interrupt configuration registers are defined, but full interrupt configuration helper functions are not implemented yet.
* Free-fall and wake-up detection can be added later using the `FF_WU_CFG`, `FF_WU_THS`, `FF_WU_DURATION`, and `CTRL_REG3` registers.
* The driver uses blocking STM32 HAL SPI functions.
* DMA and interrupt-based SPI transfers are not implemented.

## License

This project is released under the MIT License. See the [LICENSE](https://github.com/eylloztek/lis302dl-library/blob/master/LICENSE.txt) file for details.
