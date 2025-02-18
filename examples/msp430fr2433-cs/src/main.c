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
#include "cs.h"
/* Compile-time called macros ------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables and Local objects ---------------------------------------*/

uint8_t RXData = 0;
uint8_t TXData = 1;
static const uint8_t buffer[] = "Hello World\n";

/* Private function prototypes -----------------------------------------------*/
static void __attribute__((naked, section(".crt_0042"), used))
disable_watchdog (void)
{
    WDTCTL = WDTPW | WDTHOLD;
}

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

    /* Set up pins to debug clks */
    // SMCLCK = P1.7 P1DIR 1 P1SELx 10
    // MCLCK  = P1.3 P1DIR 1 P1SELx 10
    P1DIR  |= (BIT7 | BIT3);
    P1SEL0 &= ~BIT3;
    P1SEL0 &= ~BIT7;
    P1SEL1 |= BIT3;
    P1SEL1 |= BIT7;

    CS_init();

    /* LEDs */
    P1DIR |= (BIT0 | BIT1);         // Set P1.0 and P1.1 to output direction

    // Configure UART pins
    P1SEL0 |= BIT4 | BIT5;                    // set 2-UART pin as second function

    // Configure UART
    UCA0CTLW0 |= UCSWRST;                     // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;

    // Baud Rate calculation
    UCA0BRW = UART_UC0BRW_REG;
    UCA0MCTLW = UART_UC0MCTLW_REG;

    UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
    UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

    __enable_interrupt();       // enable all interrupts --> GIE = 1 (HIGH)

    for(;;) {
        P1OUT ^= (BIT0 | BIT1);		// Toggle P1.0 and P1.1 using exclusive-OR


        for (volatile uint16_t i = 0; i < 10000; i++){}

        if (RXData != 0) {
            uint8_t tmpBuffer[32];
            sprintf(tmpBuffer,"Rcvd: %c\n", RXData);
            RXData = 0;
            for (uint16_t i = 0; i < strlen(tmpBuffer); i++) {
                while ((UCA0STATW & UCBUSY));   // Wait until not busy
                UCA0TXBUF = tmpBuffer[i];
            }
        }
        for (uint16_t i = 0; i < sizeof(buffer); i++) {
            while ((UCA0STATW & UCBUSY));   // Wait until not busy
            UCA0TXBUF = buffer[i];
        }
    }

    return 0;
}

/******************************************************************************/
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch (__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG)) {
        case USCI_NONE: break;
        case USCI_UART_UCRXIFG:
              UCA0IFG &=~ UCRXIFG;            // Clear interrupt
              RXData = UCA0RXBUF;             // Clear buffer
              break;
        case USCI_UART_UCTXIFG: break;
        case USCI_UART_UCSTTIFG: break;
        case USCI_UART_UCTXCPTIFG: break;
    }
}
