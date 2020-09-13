/**
 * @file    signals.h
 * @brief   Header containing all the signals that can be posted by the system
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef SIGNALS_H_
#define SIGNALS_H_

/* Includes ------------------------------------------------------------------*/
#include "qpc.h"

/* Exported defines ----------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/**
 * @brief   Signals used by AOs
 *
 * Q_USER_SIG is the first signal that user applications are allowed to use.
 * Should always be at the very top of the very first enum list
 */
typedef enum Signals {
    /* Q_USER_SIG is the first signal that user applications are allowed to use.
     * Should always be at the very top of the very first enum list */
    FIRST_SIG = Q_USER_SIG,
    TIMER_SIG,

    /** @{ Signals for NTAG commands */
    NTAG_REG_READ_SIG,
    NTAG_REG_READ_DONE_SIG,
    NTAG_REG_WRITE_SIG,
    NTAG_REG_WRITE_DONE_SIG,
    NTAG_MEM_READ_SIG,
    NTAG_MEM_READ_DONE_SIG,
    NTAG_MEM_WRITE_SIG,
    NTAG_MEM_WRITE_DONE_SIG,
    /** @} */

    TERMINATE_SIG,
    MAX_SHARED_SIG,       /* Last published shared signal; should always last */
    MAX_PUB_SIG           /* This signal should be used if LWIP is being used */

} Signals_t;

/* Exported variables --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#endif                                                          /* SIGNALS_H_ */
