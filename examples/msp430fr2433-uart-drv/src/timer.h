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
#include <msp430fr2433.h>
#include <stdbool.h>

#include "buffers.h"
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
 * @brief   Timers available on this system
 */
typedef enum {
    TimerMin = 0,                      /**< Minimum Timer for bounds checking */
    Timer0 = TimerMin,                                            /**< Timer 0*/
    Timer1,                                                       /**< Timer 1*/
    Timer2,                                                       /**< Timer 2*/
    Timer3,                                                       /**< Timer 3*/
    TimerMax,                          /**< Maximum Timer for bounds checking */
} Timer_t;

/**
 * @brief   Timer operating modes
 */
typedef enum {
    TimerModeStop = 0,                                            /**< halted */
    TimerModeUp,                     /**< repeatedly counts from 0 to TAxCCR0 */
    TimerModeContinuous,              /**< repeatedly counts from 0 to 0xFFFF */
    TimerModeUpDown,   /**< repeatedly counts from 0 to TAxCCR0 and back to 0 */
} TimerMode_t;

/**
 * @brief   Timer capture/compare modes
 */
typedef enum {
    TimerCompareMode = 0,   /**< Used to generate PWM output or interrupts at
                                 specific time intervals */
    TimerCaptureMode,       /**< Used to record time events for measurements
                                 of durations of events */
} TimerCapCmpMode_t;

/**
 * @brief   Timer output modes
 */
typedef enum {
    TimerOutput = 0,    /**< The output signal OUTn is defined by the OUT bit.
                             The OUTn signal updates immediately when OUT is
                             updated. */
    TimerSet,           /**< The output is set when the timer counts to the
                             TAxCCRn value. It remains set until a reset of the
                             timer, or until another output mode is selected and
                             affects the output */
    TimerToggleReset,   /**< The output is toggled when the timer counts to the
                             TAxCCRn value. It is reset when the timer counts to
                             the TAxCCR0 value. */
    TimerSetReset,      /**< The output is set when the timer counts to the
                             TAxCCRn value. It is reset when the timer counts to
                             the TAxCCR0 value */
    TimerToggle,        /**< The output is toggled when the timer counts to the
                             TAxCCRn value. The output period is double the
                             timer period */
    TimerReset,         /**< The output is reset when the timer counts to the
                             TAxCCRn value. It remains reset until another
                             output mode is selected and affects the output */
    TimerToggleSet,     /**< The output is toggled when the timer counts to the
                             TAxCCRn value. It is set when the timer counts to
                             the TAxCCR0 value */
    TimerResetSet       /**< The output is reset when the timer counts to the
                             TAxCCRn value. It is set when the timer counts to
                             the TAxCCR0 value */
} TimerOutputMode_t;

/**
 * @brief   Timer channel per timer for 3 channel timers
 */
typedef enum {
    Timer3ChnlMin = 0,          /**< Minimum Timer channel for bounds checking */
    Timer3Chnl0 = Timer3ChnlMin,                          /**< Timer channel 0 */
    Timer3Chnl1,                                          /**< Timer channel 1 */
    Timer3Chnl2,                                          /**< Timer channel 2 */
    Timer3ChnlMax,              /**< Maximum Timer channel for bounds checking */
} TimerChannel3_t;

/**
 * @brief   Timer channel per timer
 */
typedef enum {
    Timer2ChnlMin = 0,          /**< Minimum Timer channel for bounds checking */
    Timer2Chnl0 = Timer2ChnlMin,                          /**< Timer channel 0 */
    Timer2Chnl1,                                          /**< Timer channel 1 */
    Timer2ChnlMax,              /**< Maximum Timer channel for bounds checking */
} TimerChannel2_t;

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

/**
 * @brief   Timer dynamic data
 * This structure holds any dynamic data for the Timer including callbacks array,
 * and other things that should live in RAM as opposed to flash.
 */
typedef struct {
    TimCallback_t  callbacks[TimEvtEnd];              /**< Array of callbacks */
    bool           isBusy;                     /**< Keep track of timer state */
    bool           isCallbackRemoved;     /**< remove callback after calling? */
    uint16_t       currentValue;
    uint16_t       triggerValue;
} TimDynamicData_t;

typedef enum {
    TimIntervalHour = 0,
    TimIntervalMinute,
    TimIntervalSecond,
    TimIntervalMillisecond,
    TimIntervalMicrosecond,
    TimIntervalNanosecond,
} TimerInterval_t;

typedef struct {
    const TimerInterval_t interval;
    const uint16_t        value;
} Time_t;

typedef struct {
    const uint16_t baseAddr;

} TimerInitData_t;

/**
 * @brief   TIM internal data structure
 */
typedef struct {
    TimDynamicData_t* const pDynData;           /**< pointer to dynamic data */
    const uint16_t clockSelect;
    const uint16_t baseAddr;
} TimData_t;


/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif                                                           /* __TIMER_H */
