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

/* Exported defines ----------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef enum {
    I2CEvtNone = 0,
}I2CEvt_t;

/**
 * @brief   I2C callback function pointer type
 */
typedef void (*I2CCallback_t) (
        Error_t                                 /**< [in] status of operation */
);

/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif                                                             /* __I2C_H */
