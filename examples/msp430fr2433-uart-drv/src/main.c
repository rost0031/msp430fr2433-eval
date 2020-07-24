/**
 * @file    main.c
 * @brief   UART demo
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Includes ------------------------------------------------------------------*/
#include "msp430fr2433.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "uart.h"
#include "cs.h"

/* Compile-time called macros ------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CS_MCLK_DESIRED_FREQUENCY_IN_KHZ    (8000)
#define CS_MCLK_FLLREF_RATIO                (243)
/* Private macros ------------------------------------------------------------*/
/* Private variables and Local objects ---------------------------------------*/
static volatile uint32_t clockValueSMCLK = 0;
static volatile uint32_t clockValueMCLK = 0;
static volatile uint32_t clockValueACLK = 0;

static const uint8_t buffer[] = "Hello World\n";

static volatile bool isTxBusy = false;

/* Private function prototypes -----------------------------------------------*/
static void __attribute__((naked, section(".crt_0042"), used))
disable_watchdog (void)
{
    WDTCTL = WDTPW | WDTHOLD;
}

static void UART_callbackTxDone(Error_t status, Buffer_t* pBuffer);

/* Public and Exported functions ---------------------------------------------*/

/******************************************************************************/
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer // See function above

    // For FRAM devices, at start up, the GPO power-on default
    // high-impedance mode needs to be disabled to activate previously
    // configured port settings. This can be done by clearing the LOCKLPM5
    // bit in PM5CTL0 register
    PM5CTL0 &= ~LOCKLPM5;

    P1DIR |= (BIT0 | BIT1);         // Set P1.0 and P1.1 to output direction

#if 1
    //Set DCO FLL reference = REFO
    CS_initClockSignal(
            CS_FLLREF,
            CS_REFOCLK_SELECT,
            CS_CLOCK_DIVIDER_1
    );

    //Set ACLK = REFO
    CS_initClockSignal(
            CS_ACLK,
            CS_REFOCLK_SELECT,
            CS_CLOCK_DIVIDER_1
    );

    CS_initClockSignal(
            CS_MCLK,
            CS_DCOCLKDIV_SELECT,
            CS_CLOCK_DIVIDER_1
    );

    CS_initClockSignal(
            CS_SMCLK,
            CS_DCOCLKDIV_SELECT,
            CS_CLOCK_DIVIDER_8
    );

    //Create struct variable to store proper software trim values
    CS_initFLLParam param = {0};

    //Set Ratio/Desired MCLK Frequency, initialize DCO, save trim values
    CS_initFLLCalculateTrim(
            CS_MCLK_DESIRED_FREQUENCY_IN_KHZ,
            CS_MCLK_FLLREF_RATIO,
            &param
    );

    //Clear all OSC fault flag
    CS_clearAllOscFlagsWithTimeout(1000);

    //For demonstration purpose, change DCO clock freq to 16MHz
    CS_initFLLSettle(
            16000,
            487
    );

    //Clear all OSC fault flag
    CS_clearAllOscFlagsWithTimeout(1000);

    //Reload DCO trim values that were calculated earlier
    CS_initFLLLoadTrim(
            CS_MCLK_DESIRED_FREQUENCY_IN_KHZ,
            CS_MCLK_FLLREF_RATIO,
            &param
    );

    //Clear all OSC fault flag
    CS_clearAllOscFlagsWithTimeout(1000);


//    CS_initClockSignal(
//            CS_SMCLK,
//            CS_REFOCLK_SELECT,
//            CS_CLOCK_DIVIDER_1
//    );

    //Clear all OSC fault flag
    CS_clearAllOscFlagsWithTimeout(1000);

    //Verify if the Clock settings are as expected
    clockValueSMCLK = CS_getSMCLK();
    clockValueMCLK = CS_getMCLK();
    clockValueACLK = CS_getACLK();
#endif



    /* Initialize the UART */
    const UartInit_t uartInit = {.parity = ParityNone, .stopBits = StopBits1};
    UART_init(&uartInit);

    /* Register callbacks */
    UART_regCallback(UartEvtDataSent, UART_callbackTxDone);

    /* Start the driver */
    UART_start();

    __enable_interrupt();       // enable all interrupts --> GIE = 1 (HIGH)

    for(;;) {
        P1OUT ^= (BIT0 | BIT1);		// Toggle P1.0 and P1.1 using exclusive-OR


        for (volatile uint16_t i = 0; i < 10000; i++){}
//        __delay_cycles(50000);
        if (!isTxBusy) {
            isTxBusy = true;
            if (ERR_NONE != UART_send(sizeof(buffer), buffer)) {
                isTxBusy = false;
            }
        }


    }

    return 0;
}

/* Private functions ---------------------------------------------------------*/
static void UART_callbackTxDone(Error_t status, Buffer_t* pBuffer)
{
    isTxBusy = false;
}
