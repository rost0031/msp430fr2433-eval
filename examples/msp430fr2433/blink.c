//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  J. Stevenson
//  Texas Instruments, Inc
//  July 2011
//  Built with Code Composer Studio v5
//***************************************************************************************

#include "msp430fr2433.h"

static void __attribute__((naked, section(".crt_0042"), used))
disable_watchdog (void)
{
    WDTCTL = WDTPW | WDTHOLD;
}


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer // See function above

    // For FRAM devices, at start up, the GPO power-on default
    // high-impedance mode needs to be disabled to activate previously
    // configured port settings. This can be done by clearing the LOCKLPM5
    // bit in PM5CTL0 register
    PM5CTL0 &= ~LOCKLPM5;

    P1DIR |= (BIT0 | BIT1);         // Set P1.0 and P1.1 to output direction

    for(;;) {
        volatile unsigned int i;	// volatile to prevent optimization

        P1OUT ^= (BIT0 | BIT1);		// Toggle P1.0 and P1.1 using exclusive-OR


        i = 10000;					// SW Delay
        do i--;
        while(i != 0);
    }

    return 0;
}
