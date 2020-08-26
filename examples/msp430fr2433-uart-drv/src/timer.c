/**
 * @file    timer.c
 * @brief   Timer driver for MSP430FR2433
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Includes ------------------------------------------------------------------*/
#include "timer.h"
#include "msp430fr2xx_4xxgeneric.h"

#include "cs.h"                         /* Need access to the clock subsystem */

/* Compile-time called macros ------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables and Local objects ---------------------------------------*/

const TimData_t timers[TimerMax] = {
        [Timer0] = {
                .baseAddr = TIMER_A0_BASE,
        },
        [Timer1] = {
                .baseAddr = TIMER_A1_BASE,
        },
        [Timer2] = {
                .baseAddr = TIMER_A2_BASE,
        },
        [Timer3] = {
                .baseAddr = TIMER_A3_BASE,
        },
};

/* Private function prototypes -----------------------------------------------*/
/* Public and Exported functions ---------------------------------------------*/


/******************************************************************************/
void TIM_init(Timer_t timer, TimerMode_t mode )
{
//    switch(timer) {
//        case Timer0:
//            break;
//        case Timer1:
//            break;
//        case Timer2:
//            break;
//        case Timer3:
//            break;
//        default:
//            break;
//    }
    uint16_t clockValueSMCLK = CS_getSMCLK();
    uint16_t clockValueMCLK = CS_getMCLK();
    uint16_t clockValueACLK = CS_getACLK();

    uint16_t value = clockValueSMCLK * 0.001;

}

/******************************************************************************/
void TIM_start(Timer_t timer)
{
//    timers[timer].baseAddr
}

/******************************************************************************/
void TIM_stop(Timer_t timer)
{

}

/******************************************************************************/
void TIM_regCallback(TimEvt_t timEvt, TimCallback_t callback)
{
//    pUarts[port].pUart->callbacks[uartInt] = callback;
}

/******************************************************************************/
void TIM_clrCallback(TimEvt_t timEvt)
{
//    pUarts[port].pUart->callbacks[uartInt] = NULL;
}



/* Private functions ---------------------------------------------------------*/


/* Interrupt service routines ------------------------------------------------*/
#if 0
/******************************************************************************/
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
__interrupt void TIMER0_A0_ISR(void); /* prototype */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
#elif defined(__GNUC__)
__attribute__ ((interrupt(TIMER0_A0_VECTOR)))
void TIMER0_A0_ISR(void)
#else
    #error MSP430 compiler not supported!
#endif
{



}

/******************************************************************************/
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
__interrupt void TIMER0_A1_ISR(void); /* prototype */
#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void)
#elif defined(__GNUC__)
__attribute__ ((interrupt(TIMER0_A1_VECTOR)))
void TIMER0_A1_ISR(void)
#else
    #error MSP430 compiler not supported!
#endif
{
    switch(__even_in_range(TA0IV,TA0IV_TAIFG))
    {
        case TA0IV_NONE:
            break;                               // No interrupt
        case TA0IV_TACCR1:
//            TA0CCTL1 &= ~CCIE;                          // CCR0 interrupt enabled
//            TA0CCR1 = 0;
//            P1OUT ^= BIT1;                       // overflow
            break;                               // CCR1 not used
        case TA0IV_TACCR2:
            break;                               // CCR2 not used
        case TA0IV_TAIFG:
//            TA0CCTL1 &= ~CCIE;                          // CCR0 interrupt enabled
//            TA0CCR1 = 0;
//            P1OUT ^= BIT0;                       // overflow
            break;
        default:
            break;
    }
}
#endif
