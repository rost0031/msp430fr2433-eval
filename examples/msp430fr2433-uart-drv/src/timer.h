/**
 * @file    timer.h
 * @brief   TIMER driver for MSP430FR2433
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIMER_H
#define __TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "errors.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/**
 * @brief   TIMER callback function pointer type
 */
typedef void (*TimCallback_t) (
        Error_t                                 /**< [in] status of operation */
);

/**
 * @brief   Timer events
 *
 * This set of enums is a list of possible events for this driver.
 * Its primary purpose to allow user to assign callbacks for these events
 */
typedef enum {
    TimEvtStart = 0,                                 /**< For bounds checking */

    TimEvtCompValReached = TimEvtStart,/**< Comparison value reached by timer */
    TimEvtRollover,                                    /**< Timer rolled over */

    TimEvtEnd                                        /**< For bounds checking */
} TimEvt_t;

/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif                                                           /* __TIMER_H */
