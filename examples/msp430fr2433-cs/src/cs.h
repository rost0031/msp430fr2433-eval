/**
 * @file    cs.h
 * @brief   Clock System driver for MSP430FR2433
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CS_H
#define __CS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported defines ----------------------------------------------------------*/
#define CS_SPEED_1M_1M_32K    (1U)
#define CS_SPEED_8M_1M_32K    (2U)
#define CS_SPEED_8M_2M_32K    (3U)
#define CS_SPEED_8M_4M_32K    (4U)
#define CS_SPEED_16M_2M_32K   (5U)
#define CS_SPEED_16M_4M_32K   (6U)

/**
 * @brief   Choose a set of clock speeds
 *
 * This set of defines allows compile time choice of different clock speeds.
 * Uncomment one of the choices to set the desired set of speeds for
 * MCLK, SMCLK, and ACLK
 *
 * UART settings were calculated using this online calculator:
 * http://software-dl.ti.com/msp430/msp430_public_sw/mcu/msp430/MSP430BaudRateConverter/index.html
 *
 * @note: ALCK is always 32KHz for this driver
 * @{
 */
//#define CS_CONFIG   (CS_SPEED_1M_1M_32K)
//#define CS_CONFIG   (CS_SPEED_8M_1M_32K)
//#define CS_CONFIG   (CS_SPEED_8M_2M_32K)
//#define CS_CONFIG   (CS_SPEED_8M_4M_32K)
//#define CS_CONFIG   (CS_SPEED_16M_2M_32K)
#define CS_CONFIG   (CS_SPEED_16M_4M_32K)
/** @} */

#if (CS_CONFIG==CS_SPEED_1M_1M_32K)
#define CS_DCO_RANGE_SELECT (DCORSEL_0) /**< 1MHz DCO frequency range */
#define CS_MCLK_FREQ_MHZ    (1U)        /**< 1MHz Main clock */
#define CS_SMCLK_FREQ_MHZ   (1U)        /**< 1MHz Submain clock */
#define CS_DC0_FLLN_MULT    (30)        /**< 1MHz DCO divider */
#define CS_MCLK_DIV         (DIVM_0)    /**< MCLK = DCOCLK, so DIV0 */
#define CS_SMCLK_DIV        (DIVS_0)    /**< SMCLK = MCLK, so DIV0 */

/** 1000000/115200 - INT(1000000/115200)=0.68 */
#define UART_CLK_PRESCALAR  (8)
#define UART_UC0BRW_REG     (UART_CLK_PRESCALAR)

#define UART_OVERSAMPLE     (0)
#define UART_UCBRSx         (0xD6)      /**< Second modulation register */
#define UART_UCBRFx         (0x00)      /**< First modulation register */

#elif (CS_CONFIG==CS_SPEED_8M_1M_32K)

#define CS_DCO_RANGE_SELECT (DCORSEL_3) /**< 8MHz DCO frequency range */
#define CS_MCLK_FREQ_MHZ    (8U)        /**< 8MHz Main clock */
#define CS_SMCLK_FREQ_MHZ   (1U)        /**< 1MHz Submain clock */
#define CS_DC0_FLLN_MULT    (243)       /**< 8MHz DCO divider */
#define CS_MCLK_DIV         (DIVM_0)    /**< MCLK = DCOCLK, so DIV0 */
#define CS_SMCLK_DIV        (DIVS_3)    /**< SMCLK = MCLK/8, so DIVS_3 */

/** 1000000/115200 - INT(1000000/115200)=0.68 */
#define UART_CLK_PRESCALAR  (8)
#define UART_UC0BRW_REG     (UART_CLK_PRESCALAR)

#define UART_OVERSAMPLE     (0)
#define UART_UCBRSx         (0xD6)      /**< Second modulation register */
#define UART_UCBRFx         (0x00)      /**< First modulation register */

#elif (CS_CONFIG==CS_SPEED_8M_2M_32K)
#define CS_DCO_RANGE_SELECT (DCORSEL_3) /**< 8MHz DCO frequency range */
#define CS_MCLK_FREQ_MHZ    (8U)        /**< 8MHz Main clock */
#define CS_SMCLK_FREQ_MHZ   (4U)        /**< 4MHz Submain clock */
#define CS_DC0_FLLN_MULT    (243)       /**< 8MHz DCO divider */
#define CS_MCLK_DIV         (DIVM_0)    /**< MCLK = DCOCLK, so DIV0 */
#define CS_SMCLK_DIV        (DIVS_2)    /**< SMCLK = MCLK/4, so DIVS_2 */

#define UART_CLK_PRESCALAR  (17)        /**< 2000000/115200 = 17.36 */
#define UART_UC0BRW_REG     (UART_CLK_PRESCALAR)

#define UART_OVERSAMPLE     (0)
#define UART_UCBRSx         (0xA4)      /**< Second modulation register 2000000/115200 - INT(2000000/115200)=0.36 */
#define UART_UCBRFx         (0x00)      /**< First modulation register */

#elif (CS_CONFIG==CS_SPEED_8M_4M_32K)
#define CS_DCO_RANGE_SELECT (DCORSEL_3) /**< 8MHz DCO frequency range */
#define CS_MCLK_FREQ_MHZ    (8U)        /**< 8MHz Main clock */
#define CS_SMCLK_FREQ_MHZ   (4U)        /**< 4MHz Submain clock */
#define CS_DC0_FLLN_MULT    (243)       /**< 8MHz DCO divider */
#define CS_MCLK_DIV         (DIVM_0)    /**< MCLK = DCOCLK, so DIV0 */
#define CS_SMCLK_DIV        (DIVS_1)    /**< SMCLK = MCLK/2, so DIVS_1 */

#define UART_CLK_PRESCALAR  (2)         /**< 4000000/115200 = 34.72 */
#define UART_UC0BRW_REG     (UART_CLK_PRESCALAR)

#define UART_OVERSAMPLE     (1)
#define UART_UCBRSx         (0xBB)      /**< Second modulation register 4000000/115200 - INT(4000000/115200)=0.72 */
#define UART_UCBRFx         (0x02)      /**< First modulation register */

#elif (CS_CONFIG==CS_SPEED_16M_2M_32K)
#define CS_DCO_RANGE_SELECT (DCORSEL_5) /**< 16MHz DCO frequency range */
#define CS_MCLK_FREQ_MHZ    (16U)       /**< 16MHz Main clock */
#define CS_SMCLK_FREQ_MHZ   (2U)        /**< 1MHz Submain clock */
#define CS_DC0_FLLN_MULT    (487)       /**< 16MHz DCO divider */
#define CS_MCLK_DIV         (DIVM_0)    /**< MCLK = DCOCLK, so DIV0 */
#define CS_SMCLK_DIV        (DIVS_3)    /**< SMCLK = MCLK/8, so DIVS_3 */


#define UART_CLK_PRESCALAR  (17)        /**< 2000000/115200 = 17.36 */
#define UART_UC0BRW_REG     (UART_CLK_PRESCALAR)

#define UART_OVERSAMPLE     (0)
#define UART_UCBRSx         (0xA4)      /**< Second modulation register 2000000/115200 - INT(2000000/115200)=0.36 */
#define UART_UCBRFx         (0x00)      /**< First modulation register */

#elif (CS_CONFIG==CS_SPEED_16M_4M_32K)
#define CS_DCO_RANGE_SELECT (DCORSEL_5) /**< 16MHz DCO frequency range */
#define CS_MCLK_FREQ_MHZ    (16U)       /**< 16MHz Main clock */
#define CS_SMCLK_FREQ_MHZ   (4U)        /**< 4MHz Submain clock */
#define CS_DC0_FLLN_MULT    (487)       /**< 16MHz DCO divider */
#define CS_MCLK_DIV         (DIVM_0)    /**< MCLK = DCOCLK, so DIV0 */
#define CS_SMCLK_DIV        (DIVS_2)    /**< SMCLK = MCLK/4, so DIVS_2 */

#define UART_CLK_PRESCALAR  (2)         /**< 4000000/115200 = 34.72 */
#define UART_UC0BRW_REG     (UART_CLK_PRESCALAR)

#define UART_OVERSAMPLE     (1)
#define UART_UCBRSx         (0xBB)      /**< Second modulation register 4000000/115200 - INT(4000000/115200)=0.72 */
#define UART_UCBRFx         (0x02)      /**< First modulation register */

#else
#error "CS_CONFIG not defined"
#endif


#define UART_UC0MCTLW_REG   ((UART_UCBRSx << 8) | (UART_UCBRFx << 4) | UART_OVERSAMPLE)

/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 * @brief   Initialize clock system based on defines
 */
void CS_init(void);

#ifdef __cplusplus
}
#endif

#endif                                                              /* __CS_H */
