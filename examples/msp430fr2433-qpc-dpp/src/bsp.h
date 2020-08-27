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
#define BSP_TICKS_PER_SEC    1000U

/**
 * @brief   Initialize the board support package
 * @return  None
 */
void BSP_init(void);

/**
 * @brief   Display paused philosophers
 * @return  None
 */
void BSP_displayPaused(
        uint8_t paused                           /**< [in] paused philosopher */
);

/**
 * @brief   Display philosophers' states
 * @return  None
 */
void BSP_displayPhilStat(
        uint8_t n,                               /**< [in] philosopher number */
        char_t const *stat                    /**< [in] pointer to state data */
);

/**
 * @brief   Terminate the BSP program
 * @return  None
 */
void BSP_terminate(
        int16_t result                             /**< [in] status to return */
);

/**
 * @brief   Seed the pseudo-random number generator
 * @return  None
 */
void BSP_randomSeed(
        uint32_t seed                                   /**< [in] seed to use */
);

/**
 * @brief   Seed the pseudo-random number generator
 * @return  uint32_t pseudo-random number
 */
uint32_t BSP_random(void);

#ifdef __cplusplus
}
#endif

#endif                                                               /* BSP_H */
