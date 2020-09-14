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
//        .state = I2CStateInReset,
        .status = ERR_NONE,
        .callbacks = {0},
        .bufferRx = {0},
        .bufferTx = {0},
        .txReqState = I2CStateNoReq,
        .rxReqState = I2CStateNoReq,
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

/**
 * @brief   Issue an I2C stop condition
 * @return  None
 */
inline static void I2C_issueStopCondition(void);

/**
 * @brief   Set I2C slave device address
 * @return  None
 */
inline static void I2C_setSlaveAddress(
        uint8_t addr                    /**< [in] slave device address to set */
);

/* Public and Exported functions ---------------------------------------------*/

/******************************************************************************/
void I2C_init(void)
{

    i2cData.status = ERR_NONE;
//    i2cData.state = I2CStateInReset;
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


//    UCB0CTLW1 |= UCASTP_0;          /* Don't generate Auto-Stop since MSP430 is dumb*/

    UCB0CTLW1 |= UCASTP_2;          /* Generate Auto-Stop */

    /* Compute the clock divider that achieves the fastest speed less than or
     * equal to the desired speed.  The numerator is biased to favor a larger
     * clock divider so that the resulting clock is always less than or equal
     * to the desired clock, never greater. */
    UCB0BRW = 0x2; //(CS_getSMCLK() / I2C_SPEED_KBPS); // 200KHz
//    UCB0BRW = 0xA; //(CS_getSMCLK() / I2C_SPEED_KBPS); // 100KHz

}

/******************************************************************************/
void I2C_start(void)
{
//    i2cData.state = I2CStateReady;
    UCB0CTL1 &= ~UCSWRST;                                   /* Clear SW reset */

    UCB0IE |= (
            UCBCNTIE |  /* Byte counter interrupt enable */
            UCALIE   |  /* Arbitration lost condition interrupt enable */
            UCSTPIE     /* Stop condition interrupt enable */
    );
}

/******************************************************************************/
void I2C_stop(void)
{
//    i2cData.state = I2CStateInReset;
    UCB0CTL1 |= UCSWRST;                                    /* Put into Reset */
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
    i2cData.status = ERR_NONE;

    /* TX buffer data */
    i2cData.bufferTx.maxLen = nBytesTx;
    i2cData.bufferTx.len = 0;
    i2cData.bufferTx.pData = pDataTx;

    /* If transfer was requested, mark request and not done. Otherwise, mark no
     * request but mark the transfer done. This makes the check in the ISR faster */
    if (pDataTx && nBytesTx) {
        i2cData.txReqState |= I2CStateReq;
        i2cData.txReqState &= ~I2CStateDone;
    } else {
        i2cData.txReqState &= ~I2CStateReq;
        i2cData.txReqState |= I2CStateDone;
    }

    /* RX buffer data */
    i2cData.bufferRx.maxLen = nBytesRx;
    i2cData.bufferRx.len = 0;
    i2cData.bufferRx.pData = pDataRx;

    if (pDataRx && nBytesRx) {
        i2cData.rxReqState |= I2CStateReq;
        i2cData.rxReqState &= ~I2CStateDone;
    } else {
        i2cData.rxReqState &= ~I2CStateReq;
        i2cData.rxReqState |= I2CStateDone;
    }


    UCB0CTL1 |= UCSWRST;                                     /* Put into Reset */
    UCB0TBCNT = nBytesRx;
    I2C_setSlaveAddress(devAddr);
    UCB0CTLW1 |= UCASTP_2;                               /* Generate Auto-Stop */

    UCB0IE &= ~UCBCNTIE;                       /* Byte count interrupt disable */
    UCB0CTL1 &= ~UCSWRST;                                   /* Clear SW reset */

    if (i2cData.txReqState & I2CStateReq) {
        I2C_startTx(i2cData.bufferTx.maxLen);
    } else if (i2cData.rxReqState & I2CStateReq) {
        I2C_startRx(i2cData.bufferRx.maxLen);
    }
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
    UCB0CTLW0 |= UCSWRST; /* Put into Reset */
    if (bytes > 1) {
        UCB0TBCNT = bytes;
        UCB0CTLW1 |= UCASTP_2;                          /* Generate Auto-Stop */
    } else {
        UCB0CTLW1 |= UCASTP_0;                    /* Don't generate Auto-Stop */
    }
    UCB0CTL1 &= ~UCSWRST;                                   /* Clear SW reset */


    UCB0IFG &= ~(UCTXIFG | UCRXIFG);              /* Clear pending interrupts */
    UCB0IE &= ~UCTXIE;                                /* Disable TX interrupt */
    UCB0IE |= UCRXIE;                                  /* Enable RX interrupt */
    UCB0IE |= (
            UCBCNTIE |  /* Byte counter interrupt enable */
            UCALIE   |  /* Arbitration lost condition interrupt enable */
            UCSTPIE     /* Stop condition interrupt enable */
    );
    UCB0CTLW0 &= ~UCTR;             /* Transmit/Receive bit clear for receive */
    UCB0CTLW0 |= UCTXSTT;                                        /* I2C start */


#if 1
    /* if the transfer is a single byte, issue a manual stop before reading the
     * only byte because msp430 has a stupid I2C implementation - See
     * MSP430FR2433 ref manual, section 24.3.5.2.2 - I2C Master Receiver Mode */
    if (bytes == 1) {
        /* Yeah, this is ugly since we are doing this in an ISR but there's
         * really no way around this. Can't listen for I2C START condition
         * interrupt in master mode. Seriously TI, WTF? */
        while((UCB0CTLW0 & UCTXSTT));
        UCB0CTLW0 |= UCTXSTP;
//        I2C_issueStopCondition();
    }
#endif
}

/******************************************************************************/
inline static void I2C_startTx(uint8_t bytes)
{
    UCB0CTLW0 |= UCSWRST;                                   /* Put into Reset */
    UCB0TBCNT = bytes;                                    /* Set byte counter */
    UCB0CTLW1 |= UCASTP_2;                              /* Generate Auto-Stop */
    UCB0CTL1 &= ~UCSWRST;                                   /* Clear SW reset */

    UCB0IFG &= ~(UCTXIFG + UCRXIFG);              /* Clear pending interrupts */
    UCB0IE &= ~UCRXIE;                                /* Disable RX interrupt */
    UCB0IE |= UCTXIE;                                  /* Enable TX interrupt */
    UCB0IE |= (
            UCBCNTIE |  /* Byte counter interrupt enable */
            UCALIE   |  /* Arbitration lost condition interrupt enable */
            UCSTPIE     /* Stop condition interrupt enable */
    );
    UCB0CTLW0 |= (UCTR | UCTXSTT);               /* I2C start with TX bit set */
}

/******************************************************************************/
inline static void I2C_issueStopCondition(void)
{
//    UCB0CTLW0 |= UCTXSTP;
}

/******************************************************************************/
inline static void I2C_setSlaveAddress(uint8_t addr)
{
    UCB0I2CSA = addr;
}

/* Interrupt vectors ---------------------------------------------------------*/


#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_B0_VECTOR)))
#endif
void USCIB0_ISR(void)
{
    QK_ISR_ENTRY();    /* inform QK about entering the ISR */

    uint8_t intState = 0;
    switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG)) {
        case USCI_NONE:          break;         // Vector 0: No interrupts
        case USCI_I2C_UCALIFG: {    // Vector 2: ALIFG - Arbitration lost
            i2cData.status = ERR_HW_BUSY;
            intState = 1;
            break;
        }
        case USCI_I2C_UCNACKIFG: {               // Vector 4: NACKIFG - got a NACK
            UCB0CTLW0 |= UCTXSTP;   /* Manually issue a stop */
//            I2C_clearBuffers();
            i2cData.status = ERR_HW_NACK;
            intState = 2;
            break;
        }
        case USCI_I2C_UCSTPIFG: {               // Vector 8: STPIFG - Stop detected

            intState = 4;

//            if (i2cData.txReqState & I2CStateReq) {
//                intState = 41;
//                if (i2cData.txReqState & I2CStateDone) {
//                    if ((i2cData.rxReqState & I2CStateReq) && !(i2cData.rxReqState & I2CStateDone)) {
//                        intState = 42;
//                        I2C_startRx(i2cData.bufferRx.maxLen);
//                    } else {
//                        if (i2cData.callbacks[I2CEvtExchangeDone]) {
//                            i2cData.callbacks[I2CEvtExchangeDone](&i2cData);
//                            intState = 43;
//                        }
//                    }
//                }
//
//            } else if ((i2cData.rxReqState & I2CStateReq) && (i2cData.rxReqState & I2CStateDone)) {
//                if (i2cData.callbacks[I2CEvtExchangeDone]) {
//                    i2cData.callbacks[I2CEvtExchangeDone](&i2cData);
//                    intState = 44;
//                }
//            }


//            if (i2cData.bufferRx.pData && i2cData.bufferRx.len > 0) {
//                if (i2cData.callbacks[I2CEvtRxDone]) {
//                    i2cData.callbacks[I2CEvtRxDone](&i2cData);
//
//                    intState = 41;
//                }
//            }
//
//            if (i2cData.bufferTx.pData && i2cData.bufferTx.len > 0 &&
//                i2cData.bufferRx.pData && i2cData.bufferRx.len > 0) {
//                if (i2cData.callbacks[I2CEvtExchangeDone]) {
//                    i2cData.callbacks[I2CEvtExchangeDone](&i2cData);
//
//                    intState = 42;
//                }
//            }
//
//            if (i2cData.callbacks[I2CEvtStopCondition]) {
//                i2cData.callbacks[I2CEvtStopCondition](&i2cData);
//
//                intState = 43;
//            }
            break;
        }
        case USCI_I2C_UCRXIFG0: {               // Vector 22: RXIFG0
            intState = 5;

            /* if the transfer is not a single byte, issue a manual stop
             * before reading the last byte because msp430 has a stupid
             * I2C implementation - See at MSP430FR2433 ref manual,
             * section 24.3.5.2.2 - I2C Master Receiver Mode section */
//            if ((i2cData.bufferRx.maxLen > 1) &&
//                (i2cData.bufferRx.len+1) == i2cData.bufferRx.maxLen) {
//                I2C_issueStopCondition();
//                intState = 51;
//                UCB0IE &= ~UCRXIE;
//            }

            /* Byte received */
            if (i2cData.bufferRx.len < i2cData.bufferRx.maxLen) {
                i2cData.bufferRx.pData[i2cData.bufferRx.len++] = UCB0RXBUF;

                /* Check if we have any more bytes to receive */
                if (i2cData.bufferRx.len == i2cData.bufferRx.maxLen) {
                    i2cData.rxReqState |= I2CStateDone;
                    if (i2cData.bufferRx.maxLen == 1) {
                        if (i2cData.callbacks[I2CEvtExchangeDone]) {
                            i2cData.callbacks[I2CEvtExchangeDone](&i2cData);

                            intState = 51;
                        }
                    }
                }
            }


#if 0
            if (i2cData.bufferTx.pData && i2cData.bufferTx.len > 0 && i2cData.bufferTx.len == i2cData.bufferTx.maxLen &&
                i2cData.bufferRx.pData && i2cData.bufferRx.len > 0 && i2cData.bufferRx.len == i2cData.bufferRx.maxLen) {
                if (i2cData.callbacks[I2CEvtExchangeDone]) {
                    i2cData.callbacks[I2CEvtExchangeDone](&i2cData);

                    intState = 52;
                }
            }

            if (i2cData.bufferRx.pData && i2cData.bufferRx.len > 0 && i2cData.bufferRx.len == i2cData.bufferRx.maxLen) {
                if (i2cData.callbacks[I2CEvtRxDone]) {
                    i2cData.callbacks[I2CEvtRxDone](&i2cData);

                    intState = 53;
                }
            }
#endif
            break;
        }
        case USCI_I2C_UCTXIFG0: {                // Vector 24: TXIFG0

            intState = 6;
            /* Ready to transmit */
//            if (i2cData.txReqState & I2CStateReq) {
                if (i2cData.bufferTx.len < i2cData.bufferTx.maxLen) {
                    UCB0TXBUF = i2cData.bufferTx.pData[i2cData.bufferTx.len++];

                    if (i2cData.bufferTx.len == i2cData.bufferTx.maxLen) {
                        i2cData.txReqState |= I2CStateDone;
                    }
                    intState = 61;
                } else {




                }
//            }
            break;
        }
        case USCI_I2C_UCBCNTIFG: {
            intState = 7;
            if (i2cData.txReqState & I2CStateReq) {
                intState = 71;
                if (i2cData.txReqState & I2CStateDone) {
                    if ((i2cData.rxReqState & I2CStateReq) && !(i2cData.rxReqState & I2CStateDone)) {
                        intState = 72;
                        I2C_startRx(i2cData.bufferRx.maxLen);
                    } else {
                        if (i2cData.callbacks[I2CEvtExchangeDone]) {
                            i2cData.callbacks[I2CEvtExchangeDone](&i2cData);
                            intState = 73;
                        }
                    }
                }

            } else if ((i2cData.rxReqState & I2CStateReq) && (i2cData.rxReqState & I2CStateDone)) {
                if (i2cData.callbacks[I2CEvtExchangeDone]) {
                    i2cData.callbacks[I2CEvtExchangeDone](&i2cData);
                    intState = 74;
                }
            }

            break;
        }
        default:    /* Some Unhandled vector */
            i2cData.status = ERR_HW_UNKNOWN_IRQ;
            intState = 8;
            break;
    }

    QS_BEGIN(LOG, 0);       /* application-specific record begin */
    QS_U8(1, intState);
    QS_END();

    QK_ISR_EXIT();     /* inform QK about exiting the ISR */
}

