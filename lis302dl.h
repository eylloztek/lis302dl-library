#ifndef LIS302DL_H_
#define LIS302DL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @file lis302dl.h
 * @brief STM32 HAL based LIS302DL 3-axis accelerometer driver header.
 *
 * This driver provides basic SPI communication, device initialization,
 * raw acceleration reading, g-unit conversion, status checking, and
 * runtime configuration functions for the LIS302DL accelerometer.
 *
 * The driver is designed for 4-wire SPI mode.
 */

/* Register map */

/** @brief Device identification register address. */
#define LIS302DL_REG_WHO_AM_I              0x0FU

/** @brief Control register 1 address. */
#define LIS302DL_REG_CTRL1                 0x20U

/** @brief Control register 2 address. */
#define LIS302DL_REG_CTRL2                 0x21U

/** @brief Control register 3 address. */
#define LIS302DL_REG_CTRL3                 0x22U

/** @brief High-pass filter reset register address. */
#define LIS302DL_REG_HP_FILTER_RESET       0x23U

/** @brief Status register address. */
#define LIS302DL_REG_STATUS                0x27U

/** @brief X-axis output register address. */
#define LIS302DL_REG_OUT_X                 0x29U

/** @brief Y-axis output register address. */
#define LIS302DL_REG_OUT_Y                 0x2BU

/** @brief Z-axis output register address. */
#define LIS302DL_REG_OUT_Z                 0x2DU

/** @brief Free-fall/wake-up interrupt 1 configuration register address. */
#define LIS302DL_REG_FF_WU_CFG_1           0x30U

/** @brief Free-fall/wake-up interrupt 1 source register address. */
#define LIS302DL_REG_FF_WU_SRC_1           0x31U

/** @brief Free-fall/wake-up interrupt 1 threshold register address. */
#define LIS302DL_REG_FF_WU_THS_1           0x32U

/** @brief Free-fall/wake-up interrupt 1 duration register address. */
#define LIS302DL_REG_FF_WU_DURATION_1      0x33U

/** @brief Free-fall/wake-up interrupt 2 configuration register address. */
#define LIS302DL_REG_FF_WU_CFG_2           0x34U

/** @brief Free-fall/wake-up interrupt 2 source register address. */
#define LIS302DL_REG_FF_WU_SRC_2           0x35U

/** @brief Free-fall/wake-up interrupt 2 threshold register address. */
#define LIS302DL_REG_FF_WU_THS_2           0x36U

/** @brief Free-fall/wake-up interrupt 2 duration register address. */
#define LIS302DL_REG_FF_WU_DURATION_2      0x37U

/** @brief Expected value of the WHO_AM_I register for LIS302DL. */
#define LIS302DL_WHO_AM_I_VALUE            0x3BU

/* SPI command bits */

/** @brief SPI read command bit. */
#define LIS302DL_SPI_READ                  0x80U

/** @brief SPI write command bit. */
#define LIS302DL_SPI_WRITE                 0x00U

/** @brief SPI multiple-byte auto-increment command bit. */
#define LIS302DL_SPI_MULTI                 0x40U

/** @brief SPI register address mask. */
#define LIS302DL_SPI_ADDR_MASK             0x3FU

/* CTRL_REG1 bits */

/** @brief X-axis enable bit in CTRL_REG1. */
#define LIS302DL_CTRL1_XEN                 (1U << 0)

/** @brief Y-axis enable bit in CTRL_REG1. */
#define LIS302DL_CTRL1_YEN                 (1U << 1)

/** @brief Z-axis enable bit in CTRL_REG1. */
#define LIS302DL_CTRL1_ZEN                 (1U << 2)

/** @brief Self-test M enable bit in CTRL_REG1. */
#define LIS302DL_CTRL1_STM                 (1U << 3)

/** @brief Self-test P enable bit in CTRL_REG1. */
#define LIS302DL_CTRL1_STP                 (1U << 4)

/** @brief Full-scale selection bit in CTRL_REG1. */
#define LIS302DL_CTRL1_FS                  (1U << 5)

/** @brief Power-down control bit in CTRL_REG1. */
#define LIS302DL_CTRL1_PD                  (1U << 6)

/** @brief Data-rate selection bit in CTRL_REG1. */
#define LIS302DL_CTRL1_DR                  (1U << 7)

/* STATUS_REG bits */

/** @brief X-axis new data available bit. */
#define LIS302DL_STATUS_XDA                (1U << 0)

/** @brief Y-axis new data available bit. */
#define LIS302DL_STATUS_YDA                (1U << 1)

/** @brief Z-axis new data available bit. */
#define LIS302DL_STATUS_ZDA                (1U << 2)

/** @brief X, Y, and Z axes new data available bit. */
#define LIS302DL_STATUS_ZYXDA              (1U << 3)

/** @brief X-axis data overrun bit. */
#define LIS302DL_STATUS_XOR                (1U << 4)

/** @brief Y-axis data overrun bit. */
#define LIS302DL_STATUS_YOR                (1U << 5)

/** @brief Z-axis data overrun bit. */
#define LIS302DL_STATUS_ZOR                (1U << 6)

/** @brief X, Y, and Z axes data overrun bit. */
#define LIS302DL_STATUS_ZYXOR              (1U << 7)

/** @brief Sensitivity value for +-2g full-scale mode, in g/LSB. */
#define LIS302DL_SENSITIVITY_2G            0.018f

/** @brief Sensitivity value for +-8g full-scale mode, in g/LSB. */
#define LIS302DL_SENSITIVITY_8G            0.072f

/** @brief Number of acceleration axes. */
#define LIS302DL_AXIS_COUNT                3U

/** @brief Default SPI timeout value in milliseconds. */
#define LIS302DL_DEFAULT_TIMEOUT_MS        100U

/**
 * @brief LIS302DL driver status codes.
 */
typedef enum {
	LIS302DL_OK = 0, /**< Operation completed successfully. */
	LIS302DL_ERROR, /**< Generic error. */
	LIS302DL_NULL_PARAM, /**< Null pointer parameter error. */
	LIS302DL_SPI_ERROR, /**< SPI communication error. */
	LIS302DL_ID_MISMATCH, /**< WHO_AM_I value does not match LIS302DL. */
	LIS302DL_INVALID_PARAM /**< Invalid configuration or function parameter. */
} LIS302DL_Status_t;

/**
 * @brief LIS302DL output data-rate selection.
 */
typedef enum {
	LIS302DL_ODR_100HZ = 0, /**< 100 Hz output data rate. */
	LIS302DL_ODR_400HZ = 1 /**< 400 Hz output data rate. */
} LIS302DL_DataRate_t;

/**
 * @brief LIS302DL full-scale range selection.
 */
typedef enum {
	LIS302DL_FULL_SCALE_2G = 0, /**< +-2g full-scale range. */
	LIS302DL_FULL_SCALE_8G = 1 /**< +-8g full-scale range. */
} LIS302DL_FullScale_t;

/**
 * @brief LIS302DL power mode selection.
 */
typedef enum {
	LIS302DL_POWER_DOWN = 0, /**< Power-down mode. */
	LIS302DL_ACTIVE = 1 /**< Active measurement mode. */
} LIS302DL_PowerMode_t;

/**
 * @brief LIS302DL self-test mode selection.
 */
typedef enum {
	LIS302DL_SELF_TEST_DISABLE = 0, /**< Self-test disabled. */
	LIS302DL_SELF_TEST_M = 1, /**< Self-test M mode enabled. */
	LIS302DL_SELF_TEST_P = 2 /**< Self-test P mode enabled. */
} LIS302DL_SelfTest_t;

/**
 * @brief LIS302DL configuration structure.
 */
typedef struct {
	LIS302DL_DataRate_t dataRate; /**< Output data-rate selection. */
	LIS302DL_FullScale_t fullScale; /**< Full-scale range selection. */
	LIS302DL_PowerMode_t powerMode; /**< Power mode selection. */
	LIS302DL_SelfTest_t selfTest; /**< Self-test mode selection. */

	bool enableX; /**< X-axis enable state. */
	bool enableY; /**< Y-axis enable state. */
	bool enableZ; /**< Z-axis enable state. */
} LIS302DL_Config_t;

/**
 * @brief LIS302DL device handle structure.
 */
typedef struct {
	SPI_HandleTypeDef *hspi; /**< Pointer to STM32 HAL SPI handle. */
	GPIO_TypeDef *csPort; /**< GPIO port used for chip select. */
	uint16_t csPin; /**< GPIO pin used for chip select. */
	uint32_t timeout; /**< SPI communication timeout in milliseconds. */
	LIS302DL_Config_t config; /**< Current device configuration. */
} LIS302DL_Handle_t;

/**
 * @brief Loads the default LIS302DL configuration.
 *
 * Default configuration:
 * - 400 Hz output data rate
 * - +-2g full-scale range
 * - active mode
 * - self-test disabled
 * - X, Y, and Z axes enabled
 *
 * @param config Pointer to the configuration structure to fill.
 * @return LIS302DL_OK if successful, otherwise LIS302DL_NULL_PARAM.
 */
LIS302DL_Status_t LIS302DL_GetDefaultConfig(LIS302DL_Config_t *config);

/**
 * @brief Initializes the LIS302DL device.
 *
 * This function assigns the SPI and chip-select resources, validates the
 * configuration, checks the WHO_AM_I register, and writes the initial
 * configuration to CTRL_REG1, CTRL_REG2, and CTRL_REG3.
 *
 * If @p config is NULL, the default configuration is used.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param hspi Pointer to the STM32 HAL SPI handle.
 * @param csPort GPIO port used for chip select.
 * @param csPin GPIO pin used for chip select.
 * @param config Pointer to user configuration, or NULL for default configuration.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_Init(LIS302DL_Handle_t *dev, SPI_HandleTypeDef *hspi,
		GPIO_TypeDef *csPort, uint16_t csPin, const LIS302DL_Config_t *config);

/**
 * @brief Reads a single LIS302DL register over SPI.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param reg Register address to read.
 * @param data Pointer to variable where read data will be stored.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_ReadRegister(LIS302DL_Handle_t *dev, uint8_t reg,
		uint8_t *data);

/**
 * @brief Writes a single LIS302DL register over SPI.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param reg Register address to write.
 * @param data Data byte to write.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_WriteRegister(LIS302DL_Handle_t *dev, uint8_t reg,
		uint8_t data);

/**
 * @brief Reads the WHO_AM_I register.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param id Pointer to variable where device ID will be stored.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_ReadWhoAmI(LIS302DL_Handle_t *dev, uint8_t *id);

/**
 * @brief Reads the STATUS_REG register.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param status Pointer to variable where status register value will be stored.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_ReadStatus(LIS302DL_Handle_t *dev, uint8_t *status);

/**
 * @brief Checks whether a new X/Y/Z acceleration data set is available.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param ready Pointer to boolean variable that receives the data-ready state.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_IsDataReady(LIS302DL_Handle_t *dev,
bool *ready);

/**
 * @brief Checks whether an X/Y/Z data overrun has occurred.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param overrun Pointer to boolean variable that receives the overrun state.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_IsOverrun(LIS302DL_Handle_t *dev,
bool *overrun);

/**
 * @brief Reads raw 8-bit acceleration data from X, Y, and Z axes.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param rawData Array where raw X, Y, and Z data will be stored.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_ReadRaw(LIS302DL_Handle_t *dev,
		int8_t rawData[LIS302DL_AXIS_COUNT]);

/**
 * @brief Converts raw acceleration values to g units.
 *
 * The conversion uses the sensitivity value selected by the current
 * full-scale configuration stored in the device handle.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param rawData Input array containing raw X, Y, and Z acceleration data.
 * @param gData Output array where converted g values will be stored.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_ConvertRawToG(LIS302DL_Handle_t *dev,
		const int8_t rawData[LIS302DL_AXIS_COUNT],
		float gData[LIS302DL_AXIS_COUNT]);

/**
 * @brief Reads acceleration data and converts it to g units.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param gData Output array where X, Y, and Z acceleration values in g will be stored.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_ReadG(LIS302DL_Handle_t *dev,
		float gData[LIS302DL_AXIS_COUNT]);

/**
 * @brief Sets the LIS302DL output data rate.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param dataRate Output data-rate selection.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_SetDataRate(LIS302DL_Handle_t *dev,
		LIS302DL_DataRate_t dataRate);

/**
 * @brief Sets the LIS302DL full-scale range.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param fullScale Full-scale range selection.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_SetFullScale(LIS302DL_Handle_t *dev,
		LIS302DL_FullScale_t fullScale);

/**
 * @brief Sets the LIS302DL power mode.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param powerMode Power mode selection.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_SetPowerMode(LIS302DL_Handle_t *dev,
		LIS302DL_PowerMode_t powerMode);

/**
 * @brief Enables or disables the X, Y, and Z axes.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param enableX X-axis enable state.
 * @param enableY Y-axis enable state.
 * @param enableZ Z-axis enable state.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_SetAxes(LIS302DL_Handle_t *dev,
bool enableX,
bool enableY,
bool enableZ);

/**
 * @brief Sets the LIS302DL self-test mode.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param selfTest Self-test mode selection.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_SetSelfTest(LIS302DL_Handle_t *dev,
		LIS302DL_SelfTest_t selfTest);

#ifdef __cplusplus
}
#endif

#endif /* LIS302DL_H_ */
