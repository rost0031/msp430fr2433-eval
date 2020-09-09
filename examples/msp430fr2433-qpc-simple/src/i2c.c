/**
 * @file    i2c.c
 * @brief   I2C driver for MSP430FR2433
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Includes ------------------------------------------------------------------*/
#include "i2c.h"
#include "msp430fr2433.h"

#include "qpc.h"
#include "cs.h"
#include "bsp.h"

/* Compile-time called macros ------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/**
 * @brief   Buffer structure
 */
typedef struct {
    uint16_t maxLen;                        /**< Max length of the buffer */
    uint16_t len;                           /**< Length of data in the buffer */
    uint8_t* pData;                         /**< Pointer to the actual data */
} Buffer_t;

/* Private define ------------------------------------------------------------*/

#define I2C_SPEED_KBPS (400000)
//#define I2C_SPEED_KBPS (100000)

/* Private macros ------------------------------------------------------------*/
/* Private variables and Local objects ---------------------------------------*/

Buffer_t rxBuffer = {0};
Buffer_t txBuffer = {0};

/* Private function prototypes -----------------------------------------------*/
/* Public and Exported functions ---------------------------------------------*/

/******************************************************************************/
void I2C_init(void)
{
    /* Clear buffers */
    memset(rxBuffer, 0, sizeof(rxBuffer));
    memset(txBuffer, 0, sizeof(txBuffer));

    /* Set up Pins for I2C
     * P1.2 = UCB0SDA
     * P1.3 = UCB0SCL */
    P1SEL0 |= (BIT2 | BIT3);
    P1SEL1 &= ~(BIT2 | BIT3);
    /* Set up I2C peripheral */

    UCB0CTLW0 |= UCSWRST; /* Put into Reset */

    UCB0CTLW0 |= (UCMST    |        /* Master mode */
                  UCMODE_3 |        /* I2C mode */
                  UCSYNC   |        /* Synchronous */
                  UCSSEL__SMCLK );  /* Set clock source */

    UCB0CTLW1 |= UCASTP_2;          /* Generate Auto-Stop */

    /* Compute the clock divider that achieves the fastest speed less than or
     * equal to the desired speed.  The numerator is biased to favor a larger
     * clock divider so that the resulting clock is always less than or equal
     * to the desired clock, never greater. */
    UCB0BRW = 0x2; //(CS_getSMCLK() / I2C_SPEED_KBPS);

}

/******************************************************************************/
void I2C_start(void)
{
    UCB0CTL1 &= ~UCSWRST;                                   /* Clear SW reset */

    UCB0IE |= (
            UCBCNTIE |  /* Start condition interrupt enable */
            UCNACKIE |  /* NACK condition interrupt enable */
            UCALIE   |  /* Arbitration lost condition interrupt enable */
            UCSTPIE     /* Stop condition interrupt enable */
    );
}

/******************************************************************************/
void I2C_stop(void)
{
    UCB0CTL1 |= UCSWRST;                                     /* Put into Reset */
    UCB0IE = 0;
}

/******************************************************************************/
void I2C_setSlaveAddr(uint8_t addr)
{
    UCB0I2CSA = addr;
}

/******************************************************************************/
void I2C_readBlocking(
        uint8_t devAddr,
        uint8_t nBytes,
        uint8_t* const pData
)
{
    rxBuffer.maxLen = nBytes;
    rxBuffer.len = 0;
    rxBuffer.pData = pData;

    /* Initialize slave address and interrupts */
    UCB0I2CSA = devAddr;

    UCB0IFG &= ~(UCTXIFG + UCRXIFG);       // Clear any pending interrupts
    UCB0IE &= ~UCRXIE;                       // Disable RX interrupt
    UCB0IE |= UCTXIE;                        // Enable TX interrupt

    UCB0CTLW0 &= ~UCTR;        /* Transmit/Receive bit clear for receive */

    UCB0CTLW0 |= UCTXSTT;             // I2C TX, start condition
}

/******************************************************************************/
void I2C_writeBlocking(
        uint8_t devAddr,
        uint8_t nBytes,
        uint8_t* const pData
)
{
    /* Initialize state machine */

    txBuffer.maxLen = nBytes;
    txBuffer.len = 0;
    txBuffer.pData = pData;

    UCB0TBCNT = nBytes;

    /* Initialize slave address and interrupts */
    UCB0I2CSA = devAddr;

    UCB0IFG &= ~(UCTXIFG + UCRXIFG);       // Clear any pending interrupts
    UCB0IE &= ~UCRXIE;                       // Disable RX interrupt
    UCB0IE |= UCTXIE;                        // Enable TX interrupt

    UCB0CTLW0 |= (UCTR | UCTXSTT);           // I2C TX, start condition
}

/******************************************************************************/
void I2C_regCallback(I2CEvt_t i2cEvt, I2CCallback_t callback)
{
//    pUarts[port].pUart->callbacks[uartInt] = callback;
}

/******************************************************************************/
void I2C_clrCallback(I2CEvt_t i2cEvt)
{
//    pUarts[port].pUart->callbacks[uartInt] = NULL;
}



/* Private functions ---------------------------------------------------------*/

/* Interrupt vectors ---------------------------------------------------------*/

#define I2C_BLOCKING 1

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_B0_VECTOR)))
#endif
void USCIB0_ISR(void)
{
#if I2C_BLOCKING    // This is a blocking version of the ISR
    uint8_t intState = 0;
    switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG)) {
        case USCI_NONE:          break;         // Vector 0: No interrupts
        case USCI_I2C_UCALIFG:
            intState = 1;
            break;         // Vector 2: ALIFG - Arbitration lost
        case USCI_I2C_UCNACKIFG:                // Vector 4: NACKIFG - got a NACK
            UCB0CTL1 |= UCTXSTT;                  // I2C start condition
            intState = 2;
            break;
        case USCI_I2C_UCSTTIFG:
            intState = 3;
            break;         // Vector 6: STTIFG - Start detected (slave mode only)
        case USCI_I2C_UCSTPIFG:
            intState = 4;
            break;         // Vector 8: STPIFG - Stop detected
        case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
        case USCI_I2C_UCTXIFG3:  break;         // Vector 12: TXIFG3
        case USCI_I2C_UCRXIFG2:  break;         // Vector 14: RXIFG2
        case USCI_I2C_UCTXIFG2:  break;         // Vector 16: TXIFG2
        case USCI_I2C_UCRXIFG1:  break;         // Vector 18: RXIFG1
        case USCI_I2C_UCTXIFG1:  break;         // Vector 20: TXIFG1
        case USCI_I2C_UCRXIFG0: {               // Vector 22: RXIFG0
            intState = 5;

            /* Byte received */
            if (rxBuffer.len < rxBuffer.maxLen) {
                rxBuffer.pData[rxBuffer.len++] = UCB0RXBUF;
            }

            if (rxBuffer.len == (rxBuffer.maxLen - 2)) {
                /* One byte before last, send stop*/
                UCB0CTLW0 |= UCTXSTP;
            } else if (rxBuffer.len == (rxBuffer.maxLen - 1)) {
                /* last byte, go to idle */
                UCB0IE &= ~UCRXIE;
            }
            break;
        }
        case USCI_I2C_UCTXIFG0: {                // Vector 24: TXIFG0

            intState = 6;
            /* Ready to transmit */
            if (txBuffer.len < txBuffer.maxLen) {
                volatile uint8_t byte =  txBuffer.pData[txBuffer.len++];
                UCB0TXBUF = byte; // TransmitBuffer[TransmitIndex++];
            }
//            else {
//                //Done with transmission
//                UCB0CTLW0 |= UCTXSTP;     // Send stop condition
//                UCB0IE &= ~UCTXIE;                       // disable TX interrupt
//            }
            break;
        }

        case USCI_I2C_UCBCNTIFG: {

            intState = 7;
            break;         // Vector 26: UCBCNT - byte count reached
        }
    }

    QS_BEGIN(LOG_OUT, 0);       /* application-specific record begin */
    QS_STR("intState");
    QS_U8(1, intState);
    QS_END();
#else   // This is a non-blocking, event-driven version of the ISR

#endif
}

