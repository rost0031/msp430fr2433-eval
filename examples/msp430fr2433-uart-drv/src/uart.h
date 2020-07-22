/**
 * @file    uart.h
 * @brief   UART driver for MSP430FR2433
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <msp430fr2433.h>
#include <stdbool.h>

#include "buffers.h"
#include "errors.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/**
 * @brief   UART events
 *
 * This set of enums is a list of possible events for this driver.
 * Its primary purpose to allow user to assign callbacks for these events
 *
 * This list started by listing all interrupts on UART which is a long list.
 * Really, the user doesn't need to be aware of all of these error and action
 * interrupts. Instead, the user only cares if:
 *
 * 1. Data sent successfully:
 * tells caller driver is available to send more data
 *
 * 2. Data received successfully:
 * tells caller driver has data available and that the caller needs to
 * handle it and initiate a new receive with a new buffer for storage.
 *
 * 3. Error during receive:
 * tells caller to retry or somehow handle this information
 *
 * 4. Error during send:
 * tells caller to resend or somehow handle this information.
 */
typedef enum {
    UartEvtStart = 0,                                /**< For bounds checking */

    UartEvtDataRcvd = UartEvtStart,                        /**< Data received */
    UartEvtDataSent,                              /**< Data finished sending  */
    UartEvtDataRecvErr,                      /**< Error during receiving data */
    UartEvtDataSendErr,                        /**< Error during sending data */

    UartEvtEnd                                       /**< For bounds checking */
} UartEvt_t;


/**
 * @brief   UART callback function pointer type
 */
typedef void (*UartCallback_t) (
        Error_t,                                /**< [in] status of operation */
        Buffer_t*                     /**< [in] pointer to buffer information */
);

typedef enum {
    UCLK = 0,

} UartClkSrc_t;

typedef enum {
    ParityNone = 0x00,
    ParityOdd  = 0x01,
    ParityEven = 0x02,
} UartParity_t;

typedef enum {
    StopBits1  = 0x00,
    StopBits2  = UCSPB,
} UartStopBits_t;

/**
 * @brief   UART dynamic data
 * This structure holds any dynamic data for the UART including callbacks array,
 * RX/TX buffer information, and other things that should live in RAM as opposed
 * to flash.
 */
typedef struct {
    UartCallback_t callbacks[UartEvtEnd];             /**< Array of callbacks */
    Buffer_t       bufferRx;                       /**< RX Buffer information */
    Buffer_t       bufferTx;                       /**< TX Buffer information */
    bool           isRxBusy;                      /**< Keep track of RX state */
    bool           isTxBusy;                      /**< Keep track of TX state */
} UartDynamicData_t;

/**
 * @brief   UART initialization structure
 */
typedef struct {
    const UartParity_t     parity;                       /**< Parity settings */
    const UartStopBits_t   stopBits;                           /**< Stop bits */
} UartInit_t;

/**
 * @brief   UART internal data structure
 */
typedef struct {
    UartDynamicData_t* const pDynData;           /**< pointer to dynamic data */
} UartData_t;

/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize UART
 *
 * This function initializes the only available UART in the system
 *
 * @return  None
 */
void UART_init(
        const UartInit_t* const pUartInit  /**< [in] UART initialization data */
);

#ifdef __cplusplus
}
#endif

#endif                                                            /* __UART_H */
