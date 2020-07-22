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

/* Compile-time called macros ------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables and Local objects ---------------------------------------*/

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
