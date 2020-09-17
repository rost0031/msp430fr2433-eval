/**
 * @file    i2c.h
 * @brief   I2C driver for MSP430FR2433
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_H
#define __I2C_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "errors.h"
#include "buffers.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef enum {
    I2CCmdRx,
    I2CCmdTx,
} I2CCmd_t;

struct I2CData;       /**< Forward declaration to prevent circular dependency */

/**
 * @brief   I2C callback function pointer type
 */
typedef void (*I2CCallback_t) (
        const struct I2CData* const                /**< [in] I2C data pointer */
);

/**
 * @brief   I2C data
 * This structure holds any dynamic data for the I2C bus that should live in
 * RAM as opposed to flash.
 */
typedef struct I2CData {
    Error_t        status;                                /**< Current status */
    I2CCallback_t  callback;                       /**< I2C finished callback */
    Buffer_t       buffer;                        /**< I2C buffer information */

} I2CData_t;

/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize I2C Bus
 *
 * This function initializes the only available I2C bus in the system
 *
 * @return  None
 */
void I2C_init(void);

/**
 * @brief   Start I2C Bus
 *
 * This function release the I2C bus out of reset. This is needed so we can
 * initialize the hardware before the RTOS bring-up and start the bus running
 * only after RTOS is ready to accept interrupts. If not using an RTOS, just
 * call this function right after I2C_init();
 *
 * @return  None
 */
void I2C_start(void);

/**
 * @brief   Stop I2C Bus
 *
 * This function stops the I2C Bus by putting and holding it in a reset state
 *
 * @return  None
 */
void I2C_stop(void);

/**
 * @brief   Clears the RX and TX buffer information
 * @return  None
 */
void I2C_clearBuffers(void);

/**
 * @brief   Register a callback
 * @return  None
 */
void I2C_regCallback(
        I2CCallback_t callback                /**< [in] I2C callback function */
);

/**
 * @brief   Clear the registered callback
 * @return  None
 */
void I2C_clrCallback(void);

#ifdef __cplusplus
}
#endif

#endif                                                             /* __I2C_H */
