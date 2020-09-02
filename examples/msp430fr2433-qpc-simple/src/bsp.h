/**
 * @file    bsp.h
 * @brief   MSP430 board support package
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef BSP_H
#define BSP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported defines ----------------------------------------------------------*/
/**
 * @brief   How many times per second that the RTOS should attempt to handle its
 * events.  Tells QPC how often to run with respect to system clock.
 */
#define BSP_TICKS_PER_SEC    1000U

/**
 * @brief   This macro converts milliseconds from BSP ticks.
 */
#define BSP_TICKS_PER_MS                              (BSP_TICKS_PER_SEC / 1000)

/**
 * @brief   Convert seconds to BSP ticks used by timers in QPC.
 *
 * @param   [in] seconds: number of seconds that is desired
 * @return  ticks: number of ticks to actually arm the QP timer on the board.
 */
#define MSEC_TO_TICKS( ms )                    (uint32_t)(ms * BSP_TICKS_PER_MS)

/**
 * @brief   Initialize the board support package
 * @return  None
 */
void BSP_init(void);

/**
 * @brief   Terminate the BSP program
 * @return  None
 */
void BSP_terminate(
        int16_t result                             /**< [in] status to return */
);

#ifdef __cplusplus
}
#endif

#endif                                                               /* BSP_H */
