/**
 * @file    uart.c
 * @brief   UART driver for MSP430FR2433
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Includes ------------------------------------------------------------------*/
#include "uart.h"
#include <stddef.h>

/* Compile-time called macros ------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables and Local objects ---------------------------------------*/

/**
 * @brief   UART dynamic data
 * This data structure has to live in RAM since it will be modified at runtime.
 */
static UartDynamicData_t dynData = {
        .callbacks = {0},
        .bufferRx = {0},
        .bufferTx = {0},
        .isRxBusy = true,
        .isTxBusy = true,
};

/**
 * @brief   Data about our one UART
 * This data structure can live in flash as long as the pointer to the dynamic
 * data is initialized on startup (as it is here).
 */
static const UartData_t uart = {
        .pDynData = &dynData,   /**< initialized to point to the dynamic data */
};

/* Private function prototypes -----------------------------------------------*/
/* Public and Exported functions ---------------------------------------------*/


/******************************************************************************/
void UART_init(const UartInit_t* const pUartInit)
{
    P1SEL0 |= BIT4 | BIT5;                             /* Configure UART pins */

    UCA0CTLW0 |= UCSWRST;                                /* Put UART in reset */
    UCA0CTLW0 |= UCSSEL__SMCLK;                        /* Select clock source */

    /* Baud rate calculation on MSP430 is stupidly complex for absolutely no
     * reason. Instead of allowing user to just pass in a baud rate and do some
     * simple math, there are all sorts of lookup tables required for multiple
     * registers. This could be made more general but frankly, I don't see a
     * reason to invest that much time into it if TI is just going to mail in
     * their HW designs. Just set the baudrate based on user guide */
    UCA0BR0 = 8;                                     /* 1000000/115200 = 8.68 */
    UCA0MCTLW = 0xD600;          /* 1000000/115200 - INT(1000000/115200)=0.68 */

    UCA0BR1 = 0;
    UCA0CTLW0 &= ~UCSWRST;                          /* Take UART out of reset */

    switch (pUartInit->parity) {
        case ParityNone:
            UCA0CTLW0 &= ~UCPEN;
            break;
        case ParityOdd:
            UCA0CTLW0 |= UCPEN;
            UCA0CTLW0 &= ~UCPAR;
            break;
        case ParityEven:
            UCA0CTLW0 |= UCPEN;
            UCA0CTLW0 |= UCPAR;
            break;
        default:
            break;
    }
}

/******************************************************************************/
void UART_start(void)
{
//    UCA0IE |= (UCRXIE | UCTXCPTIE);    /* Enable USCI_A0 TX and RX interrupts */
    uart.pDynData->isTxBusy = false;
    uart.pDynData->isRxBusy = false;
}

/******************************************************************************/
void UART_regCallback(UartEvt_t uartInt, UartCallback_t callback)
{
    uart.pDynData->callbacks[uartInt] = callback;
}

/******************************************************************************/
void UART_clrCallback(UartEvt_t uartInt)
{
    uart.pDynData->callbacks[uartInt] = NULL;
}

/******************************************************************************/
Error_t UART_send(uint16_t dataLen, const uint8_t* const pData)
{
    if (uart.pDynData->isTxBusy) {
        return ERR_HW_BUSY;
    }

    if (0 == dataLen) {
        return ERR_LEN_INVALID;
    }

    if (NULL == pData) {
        return ERR_MEM_NULL;
    }

    uart.pDynData->isTxBusy = true;                          /* Set busy flag */

    /* Save the buffer data. Need to cast pData to avoid warning. The reason the
     * pointer is passed in as const * const is to assure user that the data is
     * not going to change */
    uart.pDynData->bufferTx.len = 0;
    uart.pDynData->bufferTx.maxLen = dataLen;
    uart.pDynData->bufferTx.pData = (uint8_t* const)pData;

    /* Kick off the transfer. We'll get an interrupt immediately indicating that
     * the buffer is empty and we'll transmit in the ISR. */
    UCA0IE |= (UCTXIE | UCTXCPTIE);               /* Enable the transfer complete interrupt */
    UCA0IFG |= (UCTXIE | UCTXCPTIE);              /* Set the interrupt */

    return ERR_NONE;
}

/******************************************************************************/
Error_t UART_recv(uint16_t maxDataLen, uint8_t* const pData)
{
    if (uart.pDynData->isRxBusy) {
        return ERR_HW_BUSY;
    }

    if (0 == maxDataLen) {
        return ERR_LEN_INVALID;
    }

    if (NULL == pData) {
        return ERR_MEM_NULL;
    }

    uart.pDynData->isRxBusy = true;                          /* Set busy flag */

    /* Save the buffer data */
    uart.pDynData->bufferRx.len = 0;
    uart.pDynData->bufferRx.maxLen = maxDataLen;
    uart.pDynData->bufferRx.pData = pData;

    UCA0IE |= UCRXIE;                         /* Enable the receive interrupt */

    return ERR_NONE;
}


/* Private functions ---------------------------------------------------------*/

/* Interrupt vectors ---------------------------------------------------------*/
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
        case USCI_UART_UCRXIFG:                       /* Received a character */
            /* Save the received byte into the buffer and check if we reached
             * max size. If we have, call the interrupt if it exists */
            uart.pDynData->bufferRx.pData[(uart.pDynData->bufferRx.len)++] = UCA0RXBUF;
            if (uart.pDynData->bufferRx.len >= uart.pDynData->bufferRx.maxLen) {
                if (uart.pDynData->callbacks[UartEvtDataRcvd]) {
                    uart.pDynData->callbacks[UartEvtDataRcvd](ERR_NONE, &(uart.pDynData->bufferRx));
                }

                uart.pDynData->isRxBusy = false;   /* We are finished with RX */
            }

            /* Clear the interrupt only after we have returned from the
             * callback to avoid accidentally getting nested interrupts since
             * MSP430 really sucks at those */
            UCA0IFG &=~ UCRXIFG;                           /* Clear interrupt */

            break;
        case USCI_UART_UCSTTIFG:                {
            break;
        }
        case USCI_UART_UCTXIFG: {                    /* Ready to transmit more */
//            volatile uint16_t g = 0;
//            while(UCA0STATW & UCBUSY) {
//                g++;
//            }

//            UCA0TXBUF = uart.pDynData->bufferTx.pData[(uart.pDynData->bufferTx.len)++];

            if (uart.pDynData->bufferTx.len >= uart.pDynData->bufferTx.maxLen) {
                if (uart.pDynData->callbacks[UartEvtDataSent]) {
                    uart.pDynData->callbacks[UartEvtDataSent](ERR_NONE, &(uart.pDynData->bufferTx));
                }
//                UCA0IE &= ~UCTXIE;                   /* Disable the interrupt */
                UCA0IE &= ~(UCTXIE | UCTXCPTIE);       /* Disable the interrupt */
                uart.pDynData->isTxBusy = false;   /* We are finished with TX */
            }
//            UCA0IFG &=~ (UCTXIFG | UCTXCPTIFG);           /* Clear interrupts */
            break;
        }
        case USCI_UART_UCTXCPTIFG:             {
//            volatile uint16_t g = 0;
//
//            while(UCA0STATW & UCBUSY) {
//                g++;
//            }
            UCA0TXBUF = uart.pDynData->bufferTx.pData[(uart.pDynData->bufferTx.len)++];

//            if (uart.pDynData->bufferTx.len == uart.pDynData->bufferTx.maxLen) {
//                if (uart.pDynData->callbacks[UartEvtDataSent]) {
//                    uart.pDynData->callbacks[UartEvtDataSent](ERR_NONE, &(uart.pDynData->bufferTx));
//                }
//                UCA0IE &= ~UCTXIE;                   /* Disable the interrupt */
//                uart.pDynData->isTxBusy = false;   /* We are finished with TX */
//            }
//            UCA0IFG &=~ (UCTXIFG | UCTXCPTIFG);           /* Clear interrupts */
//            volatile uint8_t h = 0;
//            h++;
            break;
        }
        case USCI_NONE:                         /* Intentionally fall through */
        default:
            break;
    }
}
