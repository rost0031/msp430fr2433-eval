/**
 * @file    ntag.c
 * @brief   NTAG5 NTA5332 driver for MSP430FR2433
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Includes ------------------------------------------------------------------*/
#include "ntag.h"
#include "i2c.h"
#include "stddef.h"
#include "qpc.h"
#include "bsp.h"

/* Compile-time called macros ------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

#define I2C_SPEED_KBPS (400000)
//#define I2C_SPEED_KBPS (100000)

/* Private macros ------------------------------------------------------------*/
/* Private variables and Local objects ---------------------------------------*/

/**
 * @brief NTAG Registers with their respective addresses, offsets, and sizes.
 */
static const uint16_t ntagRegisterMap[][3] =
{
     /* Block Address,    Byte Offset,    Byte Width */
        {0x10A0,          0,              2},     /**< Tag Status Reg */
        {0x10A1,          0,              2},     /**< Tag Config Reg 0 */
        {0x10A1,          2,              2},     /**< Tag Config Reg 1 */
        {0x10A2,          0,              2},     /**< Ndef Data Block Reg */
        {0x10A3,          0,              2},     /**< PWM GPIO Config Reg */
        {0x10A4,          0,              2},     /**< PWM0 ON Reg */
        {0x10A4,          2,              2},     /**< PWM0 OFF Reg */
        {0x10A5,          0,              2},     /**< PWM1 ON Reg */
        {0x10A5,          2,              2},     /**< PWM1 OFF Reg */
        {0x10A6,          0,              2},     /**< WDT CONF Reg */
        {0x10A6,          2,              1},     /**< WDT EN Reg */
        {0x10A7,          0,              1},     /**< EH CONF Reg */
        {0x10A8,          0,              1},     /**< ED CONF Reg */
        {0x10A9,          0,              2},     /**< I2C SLAVE CONF Reg */
        {0x10AA,          0,              1},     /**< RESET GEN Reg */
        {0x10AB,          0,              1},     /**< ED INTR CLEAR Reg */
        {0x10AC,          0,              1},     /**< I2CM SLAVE ADDR Reg */
        {0x10AC,          1,              1},     /**< I2CM DATA LEN Reg */
        {0x10AC,          2,              1},     /**< ALM Status Reg */
        {0x10AD,          0,              1}      /**< I2CM STATUS Reg */
};

static uint8_t dataTx[6] = {0x00};
static uint8_t dataRx[6] = {0x00};

static NTAGRegNumber_t currentRegNumber = NTAG_MEM_OFFSET_TAG_STATUS_REG;
static uint8_t         currentRegByteOffset = 0;

/* Private function prototypes -----------------------------------------------*/

/**
 * @brief   Get size of the register
 * @return  uint8_t size of register in bytes
 */
static inline uint8_t NTAG_getRegSize(
        NTAGRegNumber_t regNum                            /**< Which register */
);

/**
 * @brief   Internal read register function
 * This function allows specification of byte offset that is necessary for
 * reading registers that are bigger than a single byte
 * @return  None
 */
static void NTAG_readRegPrivate(
        uint8_t offset                  /**< [in] byte offset in the register */
);

/**
 * @brief   I2C exchange done callback
 * @return  None
 */
static void NTAG_readRegByteDoneCallback(
        const I2CData_t* const pI2CData         /**< [in] pointer to I2C Data */
);

/* Public and Exported functions ---------------------------------------------*/


/******************************************************************************/
void NTAG_init(void)
{
    /* Initialize tag by reading 2 bytes
     * (for some reason the example code does this?) */

    I2C_exchangeNonBlocking(NTAG_I2C_ADDRESS, 0, NULL, 3, &dataRx[0]);

//    I2C_receiveNonBlocking(NTAG_I2C_ADDRESS, 2, dataRx);
}


/******************************************************************************/
void NTAG_readReg(NTAGRegNumber_t regNum)
{
    /* Need to keep track of this over several function and callback interactions*/
    currentRegNumber = regNum;
    currentRegByteOffset = 0;

    NTAG_readRegPrivate(currentRegByteOffset);
}

/******************************************************************************/
void NTAG_writeReg(void)
{

}




/* Private functions ---------------------------------------------------------*/
/******************************************************************************/
static inline uint8_t NTAG_getRegSize(NTAGRegNumber_t regNum)
{
    return (ntagRegisterMap[regNum][2]);
}

/******************************************************************************/
static void NTAG_readRegPrivate(uint8_t offset)
{
    dataTx[0] = (uint8_t)(ntagRegisterMap[currentRegNumber][0] >> 8);
    dataTx[1] = (uint8_t)(ntagRegisterMap[currentRegNumber][0]);
    dataTx[2] = (uint8_t)(ntagRegisterMap[currentRegNumber][1] + offset);

//    I2C_transmitNonBlocking(NTAG_I2C_ADDRESS, 3, dataTx);

    /* Register a callback for when the TX/RX exchange completes */
    I2C_regCallback(I2CEvtExchangeDone, NTAG_readRegByteDoneCallback);

    /* Initiate the TX/RX exchange */
    I2C_exchangeNonBlocking(NTAG_I2C_ADDRESS, 3, dataTx, 2, &dataRx[0+offset]);

    /* We have to first do a write and then do a read (See READ_REGISTER cmd) */

//    I2C_readBlocking( NTAG_I2C_ADDRESS, data, sizeof(data), rxData, sizeof(rxData));

}

/* Callback functions --------------------------------------------------------*/

/******************************************************************************/
static void NTAG_readRegByteDoneCallback(const I2CData_t* const pI2CData)
{
    currentRegByteOffset++;                                /* Bump the offset */

    /* Get size of the register size */
    uint8_t regSize = NTAG_getRegSize(currentRegNumber);

    /* If size of register is bigger than the current offset, call another*/
    if (currentRegByteOffset < regSize) {
        NTAG_readRegPrivate(currentRegByteOffset);
    } else {
        /* Finished */
        QS_BEGIN(LOG, 0);       /* application-specific record begin */
        QS_STR("Reg Data:");
        QS_U8(1, dataRx[0]);
        QS_U8(1, dataRx[1]);
        QS_END();

        I2C_clrCallback(I2CEvtExchangeDone);            /* Clear the callback */
    }
}

