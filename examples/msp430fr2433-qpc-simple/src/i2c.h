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

#define I2C_TX_SHIFT        (0x00)
#define I2C_RX_SHIFT        (0x04)

#define I2C_IN_RESET        (0x00)
#define I2C_READY           (0x01)
#define I2C_OP_REQ          (0x03)
#define I2C_BUSY            (0x02)
#define I2C_DONE            (0x07)

/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/**
 * @brief   I2C simple state machine states
 */
typedef enum {
    I2CStateInReset    = 0,

    I2CStateReady      = 1,
    I2CStateTxInProg   = 2,
    I2CStateTxDone     = 3,
    I2CStateRxInProg   = 4,
    I2CStateRxDone     = 5,
} I2CState_t;

/**
 * @brief   I2C events that can occur in the driver
 */
typedef enum {
    I2CEvtStart = 0,                      /**< For indexing through the enums */
    I2CEvtStopCondition = I2CEvtStart,          /**< I2C Stop condition event */
    I2CEvtExchangeDone,            /**< I2C TX and RX exchange finished event */
    I2CEvtTxDone,                                  /**< I2C TX finished event */
    I2CEvtRxDone,                                  /**< I2C RX finished event */
    I2CEvtMax,                            /**< For indexing through the enums */
} I2CEvt_t;

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
    I2CState_t     state;                              /**< Current I2C state */
    Error_t        status;                                /**< Current status */
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
