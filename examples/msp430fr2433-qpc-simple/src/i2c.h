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
#include <stdbool.h>

/* Exported defines ----------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef enum {
    I2CEvtStart = 0,
    I2CEvtStopCondition = I2CEvtStart,
    I2CEvtByteCountReached,
    I2CEvtReadyToTx,
    I2CEvtReadyToRx,
    I2CEvtMax,
}I2CEvt_t;

/**
 * @brief   I2C callback function pointer type
 */
typedef void (*I2CCallback_t) (
        Buffer_t*                     /**< [in] pointer to buffer information */
);

/**
 * @brief   I2C data
 * This structure holds any dynamic data for the I2C bus including callbacks array,
 * RX/TX buffer information, and other things that should live in RAM as opposed
 * to flash.
 */
typedef struct {
    I2CCallback_t  callbacks[I2CEvtMax];              /**< Array of callbacks */
    Buffer_t       bufferRx;                       /**< RX Buffer information */
    Buffer_t       bufferTx;                       /**< TX Buffer information */
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
 * @brief   Initiate an I2C receive operation
 *
 * This function sets up a receive operation and activates all the necessary interrupts
 *
 * @return  None
 */
void I2C_receiveNonBlocking(
        uint8_t devAddr,        /**< [in] I2C device address */
        uint8_t nBytes,         /**< [in] number of bytes to receive */
        uint8_t* const pData    /**< [in] where to store received bytes */
);

/**
 * @brief   Initiate an I2C transmit operation
 *
 * This function sets up a transmit operation and activates all the necessary
 * interrupts to send nBytes from pData buffer.
 *
 * @note:   The caller is responsible for storage of stability of pData buffer
 * and should not release it until a callback for I2CEvtStopCondition or
 * I2CEvtByteCountReached event has been called.
 *
 * @return  None
 */
void I2C_transmitNonBlocking(
        uint8_t devAddr,        /**< [in] I2C device address */
        uint8_t nBytes,         /**< [in] number of bytes to send */
        uint8_t* const pData    /**< [in] data buffer of bytes to send */
);

/**
 * @brief   Register a callback for a given I2C event
 * @return  None
 */
void I2C_regCallback(
        I2CEvt_t i2cEvt,        /**< [in] I2C event to register callback for */
        I2CCallback_t callback  /**< [in] I2C callback function */
);

/**
 * @brief   Clear the registered callback for a given I2C event
 * @return  None
 */
void I2C_clrCallback(
        I2CEvt_t i2cEvt         /**< [in] I2C event to clear callback for */
);

#ifdef __cplusplus
}
#endif

#endif                                                             /* __I2C_H */
