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

#include <string.h>

/* Compile-time called macros ------------------------------------------------*/
Q_DEFINE_THIS_FILE

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define I2C_SPEED_KBPS (400000)
//#define I2C_SPEED_KBPS (100000)

/* Private macros ------------------------------------------------------------*/
/* Private variables and Local objects ---------------------------------------*/

//Buffer_t i2cData.bufferRx = {0};
//Buffer_t i2cData.bufferTx = {0};

/**
 * @brief   UART dynamic data
 * This data structure has to live in RAM since it will be modified at runtime.
 */
static I2CData_t i2cData = {
        .callbacks = {0},
        .bufferRx = {0},
        .bufferTx = {0},
};

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief   Kick off a receive on I2C bus with pre-configured buffers
 * @return  None
 */
inline static void I2C_startRx(
        uint8_t bytes          /**< [in] number of bytes expected to transfer */
);

/**
 * @brief   Kick off a transmit on I2C bus with pre-configured buffers
 * @return  None
 */
inline static void I2C_startTx(
        uint8_t bytes          /**< [in] number of bytes expected to transfer */
);

/* Public and Exported functions ---------------------------------------------*/

/******************************************************************************/
void I2C_init(void)
{

    I2C_clearBuffers();

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


    UCB0CTLW1 |= UCASTP_0;          /* Don't generate Auto-Stop since MSP430 is dumb*/

//    UCB0CTLW1 |= UCASTP_2;          /* Generate Auto-Stop */

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
            UCBCNTIE |  /* Byte counter interrupt enable */
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
void I2C_clearBuffers(void)
{
    i2cData.bufferRx.len = 0;
    i2cData.bufferRx.maxLen = 0;
    i2cData.bufferRx.pData = NULL;

    i2cData.bufferTx.len = 0;
    i2cData.bufferTx.maxLen = 0;
    i2cData.bufferTx.pData = NULL;
}

/******************************************************************************/
void I2C_exchangeNonBlocking(
        uint8_t devAddr,
        uint8_t nBytesTx,
        uint8_t* const pDataTx,
        uint8_t nBytesRx,
        uint8_t* const pDataRx
)
{
    /* TX buffer data */
    i2cData.bufferTx.maxLen = nBytesTx;
    i2cData.bufferTx.len = 0;
    i2cData.bufferTx.pData = pDataTx;

    /* RX buffer data */
    i2cData.bufferRx.maxLen = nBytesRx;
    i2cData.bufferRx.len = 0;
    i2cData.bufferRx.pData = pDataRx;

    /* Initialize slave address and interrupts */
    UCB0I2CSA = devAddr;

    I2C_startTx(nBytesTx);
}

/******************************************************************************/
void I2C_receiveNonBlocking(
        uint8_t devAddr,
        uint8_t nBytes,
        uint8_t* const pData
)
{
    I2C_clearBuffers();

    i2cData.bufferRx.maxLen = nBytes;
    i2cData.bufferRx.len = 0;
    i2cData.bufferRx.pData = pData;

    /* Initialize slave address */
    UCB0I2CSA = devAddr;

    I2C_startRx(nBytes);
}

/******************************************************************************/
void I2C_transmitNonBlocking(
        uint8_t devAddr,
        uint8_t nBytes,
        uint8_t* const pData
)
{

    I2C_clearBuffers();

    i2cData.bufferTx.maxLen = nBytes;
    i2cData.bufferTx.len = 0;
    i2cData.bufferTx.pData = pData;

    /* Initialize slave address and interrupts */
    UCB0I2CSA = devAddr;

    I2C_startTx(nBytes);
}

/******************************************************************************/
void I2C_regCallback(I2CEvt_t i2cEvt, I2CCallback_t callback)
{
    i2cData.callbacks[i2cEvt] = callback;
}

/******************************************************************************/
void I2C_clrCallback(I2CEvt_t i2cEvt)
{
    i2cData.callbacks[i2cEvt] = NULL;
}


/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
inline static void I2C_startRx(uint8_t bytes)
{
    UCB0TBCNT = bytes;

    UCB0IFG &= ~(UCTXIFG + UCRXIFG);  /* Clear pending interrupts */
    UCB0IE &= ~UCTXIE;                    /* Disable TX interrupt */
    UCB0IE |= UCRXIE;                      /* Enable RX interrupt */
    UCB0CTLW0 &= ~UCTR; /* Transmit/Receive bit clear for receive */
    UCB0CTLW0 |= UCTXSTT;        /* I2C start with TX bit cleared */
}

/******************************************************************************/
inline static void I2C_startTx(uint8_t bytes)
{
    UCB0TBCNT = bytes;

    UCB0IFG &= ~(UCTXIFG + UCRXIFG);  /* Clear pending interrupts */
    UCB0IE &= ~UCRXIE;                    /* Disable RX interrupt */
    UCB0IE |= UCTXIE;                      /* Enable TX interrupt */

    UCB0CTLW0 |= (UCTR | UCTXSTT);   /* I2C start with TX bit set */
}

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
    QK_ISR_ENTRY();    /* inform QK about entering the ISR */

#if I2C_BLOCKING    // This is a blocking version of the ISR
    uint8_t intState = 0;
    switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG)) {
        case USCI_NONE:          break;         // Vector 0: No interrupts
        case USCI_I2C_UCALIFG: {    // Vector 2: ALIFG - Arbitration lost
            intState = 1;
            break;
        }
        case USCI_I2C_UCNACKIFG: {               // Vector 4: NACKIFG - got a NACK
//            UCB0CTL1 |= UCTXSTT;                  // I2C stop condition
//            I2C_clearBuffers();
            intState = 2;
            break;
        }
        case USCI_I2C_UCSTTIFG: {   // Vector 6: STTIFG - Start detected (slave mode only)
            intState = 3;
            break;
        }
        case USCI_I2C_UCSTPIFG: {   // Vector 8: STPIFG - Stop detected
            /* If we expect to receive after a stop, switch to receiver mode and
             * start that process. */
            if ((i2cData.bufferRx.len < i2cData.bufferRx.maxLen) &&
                    (i2cData.bufferRx.pData)) {
                I2C_startRx(i2cData.bufferRx.maxLen);
            }

            if (i2cData.callbacks[I2CEvtStopCondition]) {
//                i2cData.callbacks[I2CEvtStopCondition]();
            }
            intState = 4;
            break;         // Vector 8: STPIFG - Stop detected
        }
        case USCI_I2C_UCRXIFG3:  break;         // Vector 10: RXIFG3
        case USCI_I2C_UCTXIFG3:  break;         // Vector 12: TXIFG3
        case USCI_I2C_UCRXIFG2:  break;         // Vector 14: RXIFG2
        case USCI_I2C_UCTXIFG2:  break;         // Vector 16: TXIFG2
        case USCI_I2C_UCRXIFG1:  break;         // Vector 18: RXIFG1
        case USCI_I2C_UCTXIFG1:  break;         // Vector 20: TXIFG1
        case USCI_I2C_UCRXIFG0: {               // Vector 22: RXIFG0
            intState = 5;
//            volatile uint8_t rxByte = UCB0RXBUF;
            /* Byte received */
            if (i2cData.bufferRx.len < i2cData.bufferRx.maxLen) {
//                i2cData.bufferRx.pData[i2cData.bufferRx.len++] = rxByte;
                if ((i2cData.bufferRx.len+1) == i2cData.bufferRx.maxLen) {
                    UCB0CTLW0 |= UCTXSTP;   /* Manually issue a stop */
                }
                i2cData.bufferRx.pData[i2cData.bufferRx.len++] = UCB0RXBUF;
            }

//            if (i2cData.bufferRx.len == i2cData.bufferRx.maxLen) {
//                UCB0CTLW0 |= UCTXSTP;   /* Manually issue a stop */
//            }

            if (i2cData.callbacks[I2CEvtReadyToRx]) {
//                i2cData.callbacks[I2CEvtReadyToRx]();
            }
            break;
        }
        case USCI_I2C_UCTXIFG0: {                // Vector 24: TXIFG0

            intState = 6;
            /* Ready to transmit */
            if (i2cData.bufferTx.pData) {
                if (i2cData.bufferTx.len < i2cData.bufferTx.maxLen) {
                    UCB0TXBUF = i2cData.bufferTx.pData[i2cData.bufferTx.len++];
                } else {
                    UCB0CTLW0 |= UCTXSTP;   /* Manually issue a stop */
                }

//                if (i2cData.bufferTx.len == i2cData.bufferTx.maxLen) {
//                    UCB0CTLW0 |= UCTXSTP;   /* Manually issue a stop */
//                }

                if (i2cData.callbacks[I2CEvtReadyToTx]) {
                    //                i2cData.callbacks[I2CEvtReadyToTx]();
                }
            }
            break;
        }

        case USCI_I2C_UCBCNTIFG: {      /* Vector 26: UCBCNT - byte count reached */

            if (i2cData.bufferRx.pData) {
                if (i2cData.bufferRx.len == i2cData.bufferRx.maxLen) {

                }
            }
            if (i2cData.callbacks[I2CEvtByteCountReached]) {
//                i2cData.callbacks[I2CEvtByteCountReached](ERR_NONE);
            }
            intState = 7;
            break;
        }
    }

    QS_BEGIN(LOG_OUT, 0);       /* application-specific record begin */
    QS_STR("intState");
    QS_U8(1, intState);
    QS_END();
#else   // This is a non-blocking, event-driven version of the ISR

#endif

    QK_ISR_EXIT();     /* inform QK about exiting the ISR */
}

