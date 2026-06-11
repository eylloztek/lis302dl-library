#include "lis302dl.h"

/**
 * @file lis302dl.c
 * @brief STM32 HAL based LIS302DL 3-axis accelerometer driver source file.
 *
 * This file implements basic SPI register access, initialization,
 * acceleration reading, g conversion, status checking, and runtime
 * configuration functions for the LIS302DL accelerometer.
 */

/**
 * @brief Pulls the LIS302DL chip-select line low.
 *
 * @param dev Pointer to the LIS302DL handle.
 */
static void LIS302DL_CS_Low(LIS302DL_Handle_t *dev) {
	HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_RESET);
}

/**
 * @brief Pulls the LIS302DL chip-select line high.
 *
 * @param dev Pointer to the LIS302DL handle.
 */
static void LIS302DL_CS_High(LIS302DL_Handle_t *dev) {
	HAL_GPIO_WritePin(dev->csPort, dev->csPin, GPIO_PIN_SET);
}

/**
 * @brief Checks whether the LIS302DL handle contains valid basic resources.
 *
 * This function verifies that the device handle, SPI handle, and chip-select
 * GPIO port are not NULL.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @return LIS302DL_OK if the handle is valid, otherwise LIS302DL_NULL_PARAM.
 */
static LIS302DL_Status_t LIS302DL_CheckHandle(LIS302DL_Handle_t *dev) {
	if (dev == NULL || dev->hspi == NULL || dev->csPort == NULL) {
		return LIS302DL_NULL_PARAM;
	}

	return LIS302DL_OK;
}

/**
 * @brief Validates a LIS302DL configuration structure.
 *
 * @param config Pointer to the configuration structure to validate.
 * @return LIS302DL_OK if valid, otherwise LIS302DL_NULL_PARAM or LIS302DL_INVALID_PARAM.
 */
static LIS302DL_Status_t LIS302DL_ValidateConfig(
		const LIS302DL_Config_t *config) {
	if (config == NULL) {
		return LIS302DL_NULL_PARAM;
	}

	if (config->dataRate != LIS302DL_ODR_100HZ
			&& config->dataRate != LIS302DL_ODR_400HZ) {
		return LIS302DL_INVALID_PARAM;
	}

	if (config->fullScale != LIS302DL_FULL_SCALE_2G
			&& config->fullScale != LIS302DL_FULL_SCALE_8G) {
		return LIS302DL_INVALID_PARAM;
	}

	if (config->powerMode != LIS302DL_POWER_DOWN
			&& config->powerMode != LIS302DL_ACTIVE) {
		return LIS302DL_INVALID_PARAM;
	}

	if (config->selfTest != LIS302DL_SELF_TEST_DISABLE
			&& config->selfTest != LIS302DL_SELF_TEST_M
			&& config->selfTest != LIS302DL_SELF_TEST_P) {
		return LIS302DL_INVALID_PARAM;
	}

	return LIS302DL_OK;
}

/**
 * @brief Builds the CTRL_REG1 value from a LIS302DL configuration structure.
 *
 * @param config Pointer to a valid LIS302DL configuration structure.
 * @return 8-bit value suitable for writing to CTRL_REG1.
 */
static uint8_t LIS302DL_BuildCTRL1Value(const LIS302DL_Config_t *config) {
	uint8_t value = 0;

	if (config->enableX) {
		value |= LIS302DL_CTRL1_XEN;
	}

	if (config->enableY) {
		value |= LIS302DL_CTRL1_YEN;
	}

	if (config->enableZ) {
		value |= LIS302DL_CTRL1_ZEN;
	}

	if (config->selfTest == LIS302DL_SELF_TEST_M) {
		value |= LIS302DL_CTRL1_STM;
	} else if (config->selfTest == LIS302DL_SELF_TEST_P) {
		value |= LIS302DL_CTRL1_STP;
	}

	if (config->fullScale == LIS302DL_FULL_SCALE_8G) {
		value |= LIS302DL_CTRL1_FS;
	}

	if (config->powerMode == LIS302DL_ACTIVE) {
		value |= LIS302DL_CTRL1_PD;
	}

	if (config->dataRate == LIS302DL_ODR_400HZ) {
		value |= LIS302DL_CTRL1_DR;
	}

	return value;
}

/**
 * @brief Gets the active sensitivity value according to the current full-scale setting.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @return Sensitivity value in g/LSB.
 */
static float LIS302DL_GetSensitivity(LIS302DL_Handle_t *dev) {
	if (dev->config.fullScale == LIS302DL_FULL_SCALE_8G) {
		return LIS302DL_SENSITIVITY_8G;
	}

	return LIS302DL_SENSITIVITY_2G;
}

/**
 * @brief Loads the default LIS302DL configuration.
 *
 * @param config Pointer to the configuration structure to fill.
 * @return LIS302DL_OK if successful, otherwise LIS302DL_NULL_PARAM.
 */
LIS302DL_Status_t LIS302DL_GetDefaultConfig(LIS302DL_Config_t *config) {
	if (config == NULL) {
		return LIS302DL_NULL_PARAM;
	}

	config->dataRate = LIS302DL_ODR_400HZ;
	config->fullScale = LIS302DL_FULL_SCALE_2G;
	config->powerMode = LIS302DL_ACTIVE;
	config->selfTest = LIS302DL_SELF_TEST_DISABLE;

	config->enableX = true;
	config->enableY = true;
	config->enableZ = true;

	return LIS302DL_OK;
}

/**
 * @brief Initializes the LIS302DL device.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param hspi Pointer to the STM32 HAL SPI handle.
 * @param csPort GPIO port used for chip select.
 * @param csPin GPIO pin used for chip select.
 * @param config Pointer to user configuration, or NULL to use default configuration.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_Init(LIS302DL_Handle_t *dev, SPI_HandleTypeDef *hspi,
		GPIO_TypeDef *csPort, uint16_t csPin, const LIS302DL_Config_t *config) {
	LIS302DL_Status_t status;
	LIS302DL_Config_t localConfig;
	uint8_t id;
	uint8_t ctrl1Value;

	if (dev == NULL || hspi == NULL || csPort == NULL) {
		return LIS302DL_NULL_PARAM;
	}

	if (config == NULL) {
		status = LIS302DL_GetDefaultConfig(&localConfig);
		if (status != LIS302DL_OK) {
			return status;
		}
	} else {
		localConfig = *config;
	}

	status = LIS302DL_ValidateConfig(&localConfig);
	if (status != LIS302DL_OK) {
		return status;
	}

	dev->hspi = hspi;
	dev->csPort = csPort;
	dev->csPin = csPin;
	dev->timeout = LIS302DL_DEFAULT_TIMEOUT_MS;
	dev->config = localConfig;

	LIS302DL_CS_High(dev);
	HAL_Delay(5);

	status = LIS302DL_ReadWhoAmI(dev, &id);
	if (status != LIS302DL_OK) {
		return status;
	}

	if (id != LIS302DL_WHO_AM_I_VALUE) {
		return LIS302DL_ID_MISMATCH;
	}

	status = LIS302DL_WriteRegister(dev, LIS302DL_REG_CTRL2, 0x00U);
	if (status != LIS302DL_OK) {
		return status;
	}

	status = LIS302DL_WriteRegister(dev, LIS302DL_REG_CTRL3, 0x00U);
	if (status != LIS302DL_OK) {
		return status;
	}

	ctrl1Value = LIS302DL_BuildCTRL1Value(&dev->config);

	status = LIS302DL_WriteRegister(dev, LIS302DL_REG_CTRL1, ctrl1Value);
	if (status != LIS302DL_OK) {
		return status;
	}

	return LIS302DL_OK;
}

/**
 * @brief Reads a single LIS302DL register over SPI.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param reg Register address to read.
 * @param data Pointer to variable where read data will be stored.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_ReadRegister(LIS302DL_Handle_t *dev, uint8_t reg,
		uint8_t *data) {
	HAL_StatusTypeDef halStatus;
	uint8_t txData[2];
	uint8_t rxData[2];

	if (data == NULL) {
		return LIS302DL_NULL_PARAM;
	}

	if (LIS302DL_CheckHandle(dev) != LIS302DL_OK) {
		return LIS302DL_NULL_PARAM;
	}

	txData[0] = (reg & LIS302DL_SPI_ADDR_MASK) | LIS302DL_SPI_READ;
	txData[1] = 0x00U;

	rxData[0] = 0x00U;
	rxData[1] = 0x00U;

	LIS302DL_CS_Low(dev);
	halStatus = HAL_SPI_TransmitReceive(dev->hspi, txData, rxData, 2,
			dev->timeout);
	LIS302DL_CS_High(dev);

	if (halStatus != HAL_OK) {
		return LIS302DL_SPI_ERROR;
	}

	*data = rxData[1];

	return LIS302DL_OK;
}

/**
 * @brief Writes a single LIS302DL register over SPI.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param reg Register address to write.
 * @param data Data byte to write.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_WriteRegister(LIS302DL_Handle_t *dev, uint8_t reg,
		uint8_t data) {
	HAL_StatusTypeDef halStatus;
	uint8_t txData[2];

	if (LIS302DL_CheckHandle(dev) != LIS302DL_OK) {
		return LIS302DL_NULL_PARAM;
	}

	txData[0] = (reg & LIS302DL_SPI_ADDR_MASK) | LIS302DL_SPI_WRITE;
	txData[1] = data;

	LIS302DL_CS_Low(dev);
	halStatus = HAL_SPI_Transmit(dev->hspi, txData, 2, dev->timeout);
	LIS302DL_CS_High(dev);

	if (halStatus != HAL_OK) {
		return LIS302DL_SPI_ERROR;
	}

	return LIS302DL_OK;
}

/**
 * @brief Reads the WHO_AM_I register.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param id Pointer to variable where device ID will be stored.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_ReadWhoAmI(LIS302DL_Handle_t *dev, uint8_t *id) {
	return LIS302DL_ReadRegister(dev, LIS302DL_REG_WHO_AM_I, id);
}

/**
 * @brief Reads the STATUS_REG register.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param status Pointer to variable where status register value will be stored.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_ReadStatus(LIS302DL_Handle_t *dev, uint8_t *status) {
	return LIS302DL_ReadRegister(dev, LIS302DL_REG_STATUS, status);
}

/**
 * @brief Checks whether a new X/Y/Z acceleration data set is available.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param ready Pointer to boolean variable that receives the data-ready state.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_IsDataReady(LIS302DL_Handle_t *dev, bool *ready) {
	LIS302DL_Status_t status;
	uint8_t statusReg;

	if (ready == NULL) {
		return LIS302DL_NULL_PARAM;
	}

	status = LIS302DL_ReadStatus(dev, &statusReg);
	if (status != LIS302DL_OK) {
		return status;
	}

	*ready = ((statusReg & LIS302DL_STATUS_ZYXDA) != 0U);

	return LIS302DL_OK;
}

/**
 * @brief Checks whether an X/Y/Z data overrun has occurred.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param overrun Pointer to boolean variable that receives the overrun state.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_IsOverrun(LIS302DL_Handle_t *dev, bool *overrun) {
	LIS302DL_Status_t status;
	uint8_t statusReg;

	if (overrun == NULL) {
		return LIS302DL_NULL_PARAM;
	}

	status = LIS302DL_ReadStatus(dev, &statusReg);
	if (status != LIS302DL_OK) {
		return status;
	}

	*overrun = ((statusReg & LIS302DL_STATUS_ZYXOR) != 0U);

	return LIS302DL_OK;
}

/**
 * @brief Reads raw 8-bit acceleration data from X, Y, and Z axes.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param rawData Array where raw X, Y, and Z data will be stored.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_ReadRaw(LIS302DL_Handle_t *dev,
		int8_t rawData[LIS302DL_AXIS_COUNT]) {
	LIS302DL_Status_t status;
	uint8_t temp;

	if (rawData == NULL) {
		return LIS302DL_NULL_PARAM;
	}

	status = LIS302DL_ReadRegister(dev, LIS302DL_REG_OUT_X, &temp);
	if (status != LIS302DL_OK) {
		return status;
	}
	rawData[0] = (int8_t) temp;

	status = LIS302DL_ReadRegister(dev, LIS302DL_REG_OUT_Y, &temp);
	if (status != LIS302DL_OK) {
		return status;
	}
	rawData[1] = (int8_t) temp;

	status = LIS302DL_ReadRegister(dev, LIS302DL_REG_OUT_Z, &temp);
	if (status != LIS302DL_OK) {
		return status;
	}
	rawData[2] = (int8_t) temp;

	return LIS302DL_OK;
}

/**
 * @brief Converts raw acceleration values to g units.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param rawData Input array containing raw X, Y, and Z acceleration data.
 * @param gData Output array where converted g values will be stored.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_ConvertRawToG(LIS302DL_Handle_t *dev,
		const int8_t rawData[LIS302DL_AXIS_COUNT],
		float gData[LIS302DL_AXIS_COUNT]) {
	float sensitivity;

	if (rawData == NULL || gData == NULL) {
		return LIS302DL_NULL_PARAM;
	}

	if (LIS302DL_CheckHandle(dev) != LIS302DL_OK) {
		return LIS302DL_NULL_PARAM;
	}

	sensitivity = LIS302DL_GetSensitivity(dev);

	for (uint8_t i = 0; i < LIS302DL_AXIS_COUNT; i++) {
		gData[i] = (float) rawData[i] * sensitivity;
	}

	return LIS302DL_OK;
}

/**
 * @brief Reads acceleration data and converts it to g units.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param gData Output array where X, Y, and Z acceleration values in g will be stored.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_ReadG(LIS302DL_Handle_t *dev,
		float gData[LIS302DL_AXIS_COUNT]) {
	LIS302DL_Status_t status;
	int8_t rawData[LIS302DL_AXIS_COUNT];

	if (gData == NULL) {
		return LIS302DL_NULL_PARAM;
	}

	status = LIS302DL_ReadRaw(dev, rawData);
	if (status != LIS302DL_OK) {
		return status;
	}

	return LIS302DL_ConvertRawToG(dev, rawData, gData);
}

/**
 * @brief Sets the LIS302DL output data rate.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param dataRate Output data-rate selection.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_SetDataRate(LIS302DL_Handle_t *dev,
		LIS302DL_DataRate_t dataRate) {
	LIS302DL_Status_t status;
	uint8_t ctrl1;

	if (dataRate != LIS302DL_ODR_100HZ && dataRate != LIS302DL_ODR_400HZ) {
		return LIS302DL_INVALID_PARAM;
	}

	status = LIS302DL_ReadRegister(dev, LIS302DL_REG_CTRL1, &ctrl1);
	if (status != LIS302DL_OK) {
		return status;
	}

	if (dataRate == LIS302DL_ODR_400HZ) {
		ctrl1 |= LIS302DL_CTRL1_DR;
	} else {
		ctrl1 &= (uint8_t) ~LIS302DL_CTRL1_DR;
	}

	status = LIS302DL_WriteRegister(dev, LIS302DL_REG_CTRL1, ctrl1);
	if (status != LIS302DL_OK) {
		return status;
	}

	dev->config.dataRate = dataRate;

	return LIS302DL_OK;
}

/**
 * @brief Sets the LIS302DL full-scale range.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param fullScale Full-scale range selection.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_SetFullScale(LIS302DL_Handle_t *dev,
		LIS302DL_FullScale_t fullScale) {
	LIS302DL_Status_t status;
	uint8_t ctrl1;

	if (fullScale != LIS302DL_FULL_SCALE_2G
			&& fullScale != LIS302DL_FULL_SCALE_8G) {
		return LIS302DL_INVALID_PARAM;
	}

	status = LIS302DL_ReadRegister(dev, LIS302DL_REG_CTRL1, &ctrl1);
	if (status != LIS302DL_OK) {
		return status;
	}

	if (fullScale == LIS302DL_FULL_SCALE_8G) {
		ctrl1 |= LIS302DL_CTRL1_FS;
	} else {
		ctrl1 &= (uint8_t) ~LIS302DL_CTRL1_FS;
	}

	status = LIS302DL_WriteRegister(dev, LIS302DL_REG_CTRL1, ctrl1);
	if (status != LIS302DL_OK) {
		return status;
	}

	dev->config.fullScale = fullScale;

	return LIS302DL_OK;
}

/**
 * @brief Sets the LIS302DL power mode.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param powerMode Power mode selection.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_SetPowerMode(LIS302DL_Handle_t *dev,
		LIS302DL_PowerMode_t powerMode) {
	LIS302DL_Status_t status;
	uint8_t ctrl1;

	if (powerMode != LIS302DL_POWER_DOWN && powerMode != LIS302DL_ACTIVE) {
		return LIS302DL_INVALID_PARAM;
	}

	status = LIS302DL_ReadRegister(dev, LIS302DL_REG_CTRL1, &ctrl1);
	if (status != LIS302DL_OK) {
		return status;
	}

	if (powerMode == LIS302DL_ACTIVE) {
		ctrl1 |= LIS302DL_CTRL1_PD;
	} else {
		ctrl1 &= (uint8_t) ~LIS302DL_CTRL1_PD;
	}

	status = LIS302DL_WriteRegister(dev, LIS302DL_REG_CTRL1, ctrl1);
	if (status != LIS302DL_OK) {
		return status;
	}

	dev->config.powerMode = powerMode;

	return LIS302DL_OK;
}

/**
 * @brief Enables or disables the X, Y, and Z axes.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param enableX X-axis enable state.
 * @param enableY Y-axis enable state.
 * @param enableZ Z-axis enable state.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_SetAxes(LIS302DL_Handle_t *dev, bool enableX,
		bool enableY, bool enableZ) {
	LIS302DL_Status_t status;
	uint8_t ctrl1;

	status = LIS302DL_ReadRegister(dev, LIS302DL_REG_CTRL1, &ctrl1);
	if (status != LIS302DL_OK) {
		return status;
	}

	ctrl1 &= (uint8_t) ~(LIS302DL_CTRL1_XEN |
	LIS302DL_CTRL1_YEN |
	LIS302DL_CTRL1_ZEN);

	if (enableX) {
		ctrl1 |= LIS302DL_CTRL1_XEN;
	}

	if (enableY) {
		ctrl1 |= LIS302DL_CTRL1_YEN;
	}

	if (enableZ) {
		ctrl1 |= LIS302DL_CTRL1_ZEN;
	}

	status = LIS302DL_WriteRegister(dev, LIS302DL_REG_CTRL1, ctrl1);
	if (status != LIS302DL_OK) {
		return status;
	}

	dev->config.enableX = enableX;
	dev->config.enableY = enableY;
	dev->config.enableZ = enableZ;

	return LIS302DL_OK;
}

/**
 * @brief Sets the LIS302DL self-test mode.
 *
 * @param dev Pointer to the LIS302DL handle.
 * @param selfTest Self-test mode selection.
 * @return LIS302DL_OK if successful, otherwise an error status.
 */
LIS302DL_Status_t LIS302DL_SetSelfTest(LIS302DL_Handle_t *dev,
		LIS302DL_SelfTest_t selfTest) {
	LIS302DL_Status_t status;
	uint8_t ctrl1;

	if (selfTest != LIS302DL_SELF_TEST_DISABLE
			&& selfTest != LIS302DL_SELF_TEST_M
			&& selfTest != LIS302DL_SELF_TEST_P) {
		return LIS302DL_INVALID_PARAM;
	}

	status = LIS302DL_ReadRegister(dev, LIS302DL_REG_CTRL1, &ctrl1);
	if (status != LIS302DL_OK) {
		return status;
	}

	ctrl1 &= (uint8_t) ~(LIS302DL_CTRL1_STM | LIS302DL_CTRL1_STP);

	if (selfTest == LIS302DL_SELF_TEST_M) {
		ctrl1 |= LIS302DL_CTRL1_STM;
	} else if (selfTest == LIS302DL_SELF_TEST_P) {
		ctrl1 |= LIS302DL_CTRL1_STP;
	}

	status = LIS302DL_WriteRegister(dev, LIS302DL_REG_CTRL1, ctrl1);
	if (status != LIS302DL_OK) {
		return status;
	}

	dev->config.selfTest = selfTest;

	return LIS302DL_OK;
}
