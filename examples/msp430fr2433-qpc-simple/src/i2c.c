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

/**
 * @brief   I2C dynamic data
 * This data structure has to live in RAM since it will be modified at runtime.
 */
static I2CData_t i2cData = {
        .status   = ERR_NONE,
        .callback = NULL,
        .buffer   = {0},
};

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief   Kick off a receive on I2C bus with pre-configured buffers
 *
 * This private function kicks off the actual I2C RX operation. This function
 * will function in 1 of 2 modes depending on number of bytes being received.
 * If receiving:
 * - 1 byte: This is a special mode that disables auto-stop
 *   generation because of MSP430's rather crude I2C implementation that prevents
 *   stop generation being done on time with only 1 byte reception. In this case,
 *   we have to manually generate a stop before even receiving the byte and on
 *   top of that, we have to wait until we see the start condition on the bus.
 *   This is ugly but it's the only way to get things to work for a single byte
 *   reception.
 * - More than 1 byte: this is the standard way that uses auto stop generation
 *   that is able to set the stop condition at n-1 bytes and things work correctly.
 * @return  None
 */
inline static void I2C_startRx(
        uint8_t bytes          /**< [in] number of bytes expected to transfer */
);

/**
 * @brief   Kick off a transmit on I2C bus with pre-configured buffers
 *
 * This private function kicks off the actual I2C TX operation. Unlike the RX
 * version of this function, it uses only the auto-stop generation mode since
 * we never really have to send only 1 byte in our implementation. If this is
 * needed, a similar method as the RX version should be employed.
 *
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

/**
 * @brief   Set I2C byte counter
 * @note    This function must be called while the I2C bus is in reset
 * @return  None
 */
inline static void I2C_setByteCounter(
        uint8_t bytes                   /**< [in] number of bytes to count to */
);

/**
 * @brief   Set I2C into a reset state
 *
 * This function must be called if some settings such as auto-stop generation,
 * byte counter, and clock configurations need to change.
 *
 * @note:   This function puts and holds the bus in reset state. Caller has to
 * call I2C_clrReset() to take the bus out of reset.
 *
 * @note:   This clears some registers automatically and has the following
 * effects:
 *
 * I2C comms stop
 * SDA and SCL go into high impedance
 * UCBxSTAT [15-9] and [6-4] are cleared
 * UCBxIE and UCBxIFG are cleared
 *
 * See ref manual for details
 *
 * @return  None
 */
inline static void I2C_setReset(void);

/**
 * @brief   Clear I2C out of a reset state
 * @return  None
 */
inline static void I2C_clrReset(void);

/* Public and Exported functions ---------------------------------------------*/

/******************************************************************************/
void I2C_init(void)
{

    i2cData.status = ERR_NONE;
    I2C_clearBuffers();

    /* Set up Pins for I2C
     * P1.2 = UCB0SDA
     * P1.3 = UCB0SCL */
    P1SEL0 |= (BIT2 | BIT3);
    P1SEL1 &= ~(BIT2 | BIT3);
    /* Set up I2C peripheral */

    I2C_setReset();

    UCB0CTLW0 |= (UCMST    |        /* Master mode */
                  UCMODE_3 |        /* I2C mode */
                  UCSYNC   |        /* Synchronous */
                  UCSSEL__SMCLK );  /* Set clock source */

    UCB0CTLW1 |= UCASTP_2;          /* Generate Auto-Stop */
//    UCB0CTLW1 |= UCASTP_0;          /* Don't Generate Auto-Stop */

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
    I2C_clrReset();

    UCB0IE |= (
//            UCBCNTIE  /* Byte counter interrupt enable */
           UCALIE    /* Arbitration lost condition interrupt enable */
//          | UCSTPIE     /* Stop condition interrupt enable */
    );
}

/******************************************************************************/
void I2C_stop(void)
{
    I2C_setReset();
    UCB0IE = 0;
}

/******************************************************************************/
void I2C_clearBuffers(void)
{
    i2cData.buffer.len = 0;
    i2cData.buffer.maxLen = 0;
    i2cData.buffer.pData = NULL;
}

/******************************************************************************/
void I2C_exchangeNonBlocking(
        uint8_t devAddr,
        I2CCmd_t i2cCmd,
        uint8_t nBytes,
        uint8_t* const pData
)
{
    i2cData.status = ERR_NONE;

    /* TX buffer data */
    i2cData.buffer.maxLen = nBytes;
    i2cData.buffer.len = 0;
    i2cData.buffer.pData = pData;

    I2C_setSlaveAddress(devAddr);

    I2C_setReset();
    I2C_setByteCounter(i2cData.buffer.maxLen);
    UCB0CTLW1 |= UCASTP_2;                              /* Generate Auto-Stop */
    I2C_clrReset();

    UCB0IFG &= ~(UCTXIFG | UCRXIFG);              /* Clear pending interrupts */

    if (I2CCmdTx == i2cCmd) {
        UCB0IE &= ~UCRXIE;                            /* Disable RX interrupt */
        UCB0IE |= UCTXIE;                              /* Enable TX interrupt */
        UCB0CTLW0 |= UCTR;                      /* TX/RX bit set for transmit */
    } else {
        UCB0IE &= ~UCTXIE;                            /* Disable TX interrupt */
        UCB0IE |= UCRXIE;                              /* Enable RX interrupt */
        UCB0CTLW0 &= ~UCTR;                    /* TX/RX bit clear for receive */
    }

    UCB0IE |= (
//            UCBCNTIE                         /* Byte counter interrupt enable */
             UCALIE           /* Arbitration lost condition interrupt enable */
//            | UCSTPIE                      /* Stop condition interrupt enable */
    );

    UCB0CTLW0 |= UCTXSTT;                                        /* I2C start */
}

/******************************************************************************/
void I2C_regCallback(I2CCallback_t callback)
{
    i2cData.callback = callback;
}

/******************************************************************************/
void I2C_clrCallback(void)
{
    i2cData.callback = NULL;
}

/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
inline static void I2C_issueStopCondition(void)
{
    UCB0CTLW0 |= UCTXSTP;
}

/******************************************************************************/
inline static void I2C_setSlaveAddress(uint8_t addr)
{
    UCB0I2CSA = addr;
}

/******************************************************************************/
inline static void I2C_setByteCounter(uint8_t bytes)
{
    UCB0TBCNT = bytes;
}

/******************************************************************************/
inline static void I2C_setReset(void)
{
    UCB0CTLW0 |= UCSWRST;                                   /* Put into Reset */
}

/******************************************************************************/
inline static void I2C_clrReset(void)
{
    UCB0CTL1 &= ~UCSWRST;                                   /* Clear SW reset */
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
            /* Don't really know what to do here. We should never lose arbitration
             * since we are the only master on the bus in this implementation. */
            intState = 1;
            break;
        }
        case USCI_I2C_UCNACKIFG: {               // Vector 4: NACKIFG - got a NACK
            UCB0CTLW0 |= UCTXSTP;   /* Manually issue a stop */
            i2cData.status = ERR_HW_NACK;
            intState = 2;
            break;
        }
        case USCI_I2C_UCSTPIFG: {               // Vector 8: STPIFG - Stop detected
            intState = 4;
            break;
        }
        case USCI_I2C_UCRXIFG0: {               /* Vector 22: Received a byte */
            intState = 5;

            /* Byte received */
            if (i2cData.buffer.len < i2cData.buffer.maxLen) {
                i2cData.buffer.pData[i2cData.buffer.len++] = UCB0RXBUF;
                intState = 51;
                /* Check if we have any more bytes to receive. If not, call the
                 * callback function if one exists */
                if (i2cData.buffer.len == i2cData.buffer.maxLen) {
                    intState = 52;
                    if (i2cData.callback) {
                        i2cData.callback(&i2cData);
                        intState = 53;
                    }
                }
            }
            break;
        }
        case USCI_I2C_UCTXIFG0: {             /* Vector 24: Ready to transmit */

            intState = 6;
            /* Ready to transmit */

            if (i2cData.buffer.len < i2cData.buffer.maxLen) {
                UCB0TXBUF = i2cData.buffer.pData[i2cData.buffer.len++];
                if (i2cData.buffer.len == i2cData.buffer.maxLen) {
                    intState = 62;
                    if (i2cData.callback) {
                        i2cData.callback(&i2cData);
                        intState = 63;
                    }
                }
            }
            break;
        }
        case USCI_I2C_UCBCNTIFG: {           /* Vector 26: Byte count reached */
            intState = 7;
//            if (i2cData.callback) {
//                i2cData.callback(&i2cData);
//                intState = 71;
//            }
            break;
        }
        default:    /* Some Unhandled vector */
            i2cData.status = ERR_HW_UNKNOWN_IRQ;
            intState = UCB0IV;
            break;
    }
#if 0
    QS_BEGIN(LOG, 0);       /* application-specific record begin */
    QS_U8(1, intState);
    QS_END();
#endif
    QK_ISR_EXIT();     /* inform QK about exiting the ISR */
}

