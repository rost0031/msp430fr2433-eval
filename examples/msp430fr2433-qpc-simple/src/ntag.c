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
#include "ntag_ao.h"
#include "signals.h"

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

/**
 * @brief   NTAG dynamic data
 * This data structure has to live in RAM since it will be modified at runtime.
 */
static NtagData_t ntagData = {
        .status = ERR_NONE,
        .callbacks = {0},
        .bufferRx = {
                .pData = dataRx,
                .len = 0,
                .maxLen = 0,
        },
        .bufferTx = {
                .pData = dataTx,
                .len = 0,
                .maxLen = 0,
        },
        .isBusy = false,
        .currRegNumber = NTAG_MEM_OFFSET_TAG_STATUS_REG,
};

/* Private function prototypes -----------------------------------------------*/

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

static void NTAG_readDummyCallback(const I2CData_t* const pI2CData);

/* Public and Exported functions ---------------------------------------------*/


/******************************************************************************/
void NTAG_init(void)
{
    /* Initialize tag by reading 2 bytes
     * (for some reason the example code does this?) */
//    I2C_regCallback(I2CEvtExchangeDone, NTAG_readDummyCallback);
//    I2C_exchangeNonBlocking(NTAG_I2C_ADDRESS, 0, NULL, 2, &dataRx[0]);

}

/******************************************************************************/
void NTAG_getRegReadHdr(
        NTAGRegNumber_t regNum,
        uint8_t offset,
        uint8_t dataSize,
        uint8_t* const pBytesInData,
        uint8_t* const pData
)
{
    pData[0] = (uint8_t)(ntagRegisterMap[regNum][0] >> 8);
    pData[1] = (uint8_t)(ntagRegisterMap[regNum][0]);
    pData[2] = (uint8_t)(ntagRegisterMap[regNum][1] + offset);
    *pBytesInData = 3;
}

/******************************************************************************/
void NTAG_regCallback(NtagEvt_t ntagEvt, NtagCallback_t callback)
{
    ntagData.callbacks[ntagEvt] = callback;
}

/******************************************************************************/
void NTAG_clrCallback(NtagEvt_t ntagEvt)
{
    ntagData.callbacks[ntagEvt] = NULL;
}

/******************************************************************************/
static void NTAG_readRegSetupAddrCallback(const I2CData_t* const pI2CData)
{
//    I2C_clrCallback(I2CEvtExchangeDone);            /* Clear the callback */
    static const QEvt evt = {NTAG_REG_READ_ADDR_DONE_SIG, 0, 0};
    QACTIVE_POST(AO_Ntag, &evt, 0);
}

/******************************************************************************/
static void NTAG_readRegGetByteCallback(const I2CData_t* const pI2CData)
{
//    I2C_clrCallback(I2CEvtExchangeDone);            /* Clear the callback */
//    static const QEvt evt = {NTAG_REG_READ_BYTE_DONE_SIG, 0, 0};
//    QACTIVE_POST(AO_Ntag, &evt, 0);
//    QS_BEGIN(LOG, 0);       /* application-specific record begin */
//    QS_STR("Byte:");
//    QS_U8(1, pI2CData->bufferRx.pData[0]);
//    QS_END();
}

/******************************************************************************/
void NTAG_readRegSetupAddr(NTAGRegNumber_t regNum, uint8_t offset)
{
//    /* Need to keep track of this over several function and callback interactions*/
//    ntagData.currRegNumber = regNum;
//
//    /* Set up a register read from our internal database of register settings.
//     * If needed, the callback will call this function again with a new offset
//     * after the first byte comes through. This offset will also be used to
//     * put the second value into the next spot in the RX data buffer */
//    ntagData.bufferTx.pData[0] = (uint8_t)(ntagRegisterMap[ntagData.currRegNumber][0] >> 8);
//    ntagData.bufferTx.pData[1] = (uint8_t)(ntagRegisterMap[ntagData.currRegNumber][0]);
//    ntagData.bufferTx.pData[2] = (uint8_t)(ntagRegisterMap[ntagData.currRegNumber][1] + offset);
//
//    ntagData.bufferTx.len = 0;
//    ntagData.bufferRx.len = 0;
//
//    /* Set number of bytes to send */
//    ntagData.bufferTx.maxLen = 3;
//
//    /* Set number of bytes to receive */
//    ntagData.bufferRx.maxLen = 0;
//
//    /* Register a callback for when the TX/RX exchange completes */
////    I2C_regCallback(I2CEvtExchangeDone, NTAG_readRegSetupAddrCallback);
//
//    /* Initiate the TX/RX exchange */
//    I2C_exchangeNonBlocking(NTAG_I2C_ADDRESS,
//            ntagData.bufferTx.maxLen, ntagData.bufferTx.pData,
//            0, NULL);
}


/******************************************************************************/
void NTAG_readRegGetByte(NTAGRegNumber_t regNum)
{
//    /* Need to keep track of this over several function and callback interactions*/
//    ntagData.currRegNumber = regNum;
//
//    ntagData.bufferRx.len = 0;
//
//    /* Set number of bytes to receive */
//    ntagData.bufferRx.maxLen = 1;
//
//    /* Register a callback for when the TX/RX exchange completes */
////    I2C_regCallback(I2CEvtExchangeDone, NTAG_readRegGetByteCallback);
//
//    /* Initiate the TX/RX exchange */
//    I2C_exchangeNonBlocking(NTAG_I2C_ADDRESS,
//            0, NULL,
//            ntagData.bufferRx.maxLen, ntagData.bufferRx.pData);
}

/******************************************************************************/
void NTAG_readRegWithCallback(
        NTAGRegNumber_t regNum,
        NtagEvt_t ntagEvt,
        NtagCallback_t callback
)
{
    NTAG_regCallback(ntagEvt, callback);
    NTAG_readReg(regNum);
}

/******************************************************************************/
void NTAG_readReg(NTAGRegNumber_t regNum)
{
    /* Need to keep track of this over several function and callback interactions*/
//    ntagData.currRegNumber = regNum;
//
//    /* Set up a register read from our internal database of register settings.
//     * If needed, the callback will call this function again with a new offset
//     * after the first byte comes through. This offset will also be used to
//     * put the second value into the next spot in the RX data buffer */
//    ntagData.bufferTx.pData[0] = (uint8_t)(ntagRegisterMap[ntagData.currRegNumber][0] >> 8);
//    ntagData.bufferTx.pData[1] = (uint8_t)(ntagRegisterMap[ntagData.currRegNumber][0]);
//
//    ntagData.bufferTx.len = 0;
//    ntagData.bufferRx.len = 0;
//
//    /* Set number of bytes to send */
//    ntagData.bufferTx.maxLen = 3;
//
//    /* Set number of bytes to receive */
//    ntagData.bufferRx.maxLen = NTAG_getRegSize(ntagData.currRegNumber);
//
//    NTAG_readRegPrivate(ntagData.bufferRx.len);
}

/******************************************************************************/
void NTAG_writeReg(void)
{

}




/* Private functions ---------------------------------------------------------*/
/******************************************************************************/
uint8_t NTAG_getRegSize(NTAGRegNumber_t regNum)
{
    return (ntagRegisterMap[regNum][2]);
}

/******************************************************************************/
static void NTAG_readRegPrivate(uint8_t offset)
{
    /* The first 2 bytes have already been set so we shouldn't have to modify
     * them but the last byte is the offset into the register and has to get set
     * separately since it changes between calls. If needed, the callback will
     * call this function again with a new offset after the first byte comes
     * through. This offset will also be used to put the second value into the
     * next spot in the RX data buffer */
//    ntagData.bufferTx.pData[0] = (uint8_t)(ntagRegisterMap[ntagData.currRegNumber][0] >> 8);
//    ntagData.bufferTx.pData[1] = (uint8_t)(ntagRegisterMap[ntagData.currRegNumber][0]);
    ntagData.bufferTx.pData[2] = (uint8_t)(ntagRegisterMap[ntagData.currRegNumber][1] + offset);

//    /* Register a callback for when the TX/RX exchange completes */
//    I2C_regCallback(I2CEvtExchangeDone, NTAG_readRegByteDoneCallback);
//
//    /* Initiate the TX/RX exchange */
//    I2C_exchangeNonBlocking(NTAG_I2C_ADDRESS,
//            ntagData.bufferTx.maxLen, ntagData.bufferTx.pData,
//            1, &ntagData.bufferRx.pData[0+offset]);
}

/* Callback functions --------------------------------------------------------*/

/******************************************************************************/
static void NTAG_readDummyCallback(const I2CData_t* const pI2CData)
{
//    /* Finished */
//    QS_BEGIN(LOG, 0);       /* application-specific record begin */
//    QS_STR("Reg Data:");
//    for (uint8_t i = 0; i < pI2CData->bufferRx.maxLen; i++) {
//        QS_U8(1, pI2CData->bufferRx.pData[i]);
//    }
//    QS_END();
//    I2C_clrCallback(I2CEvtExchangeDone);            /* Clear the callback */
//    static const QEvt evt = {NTAG_REG_READ_DONE_SIG, 0, 0};
//    QACTIVE_POST(AO_Ntag, &evt, 0);
}

/******************************************************************************/
static void NTAG_readRegByteDoneCallback(const I2CData_t* const pI2CData)
{
//    ntagData.bufferRx.len += pI2CData->bufferRx.len;       /* Bump the offset */
//
//    /* If size of register is bigger than the current offset, call another*/
//    if (ntagData.bufferRx.len < ntagData.bufferRx.maxLen) {
//        NTAG_readRegPrivate(ntagData.bufferRx.len);
//    } else {
//        /* Finished */
////        QS_BEGIN(LOG, 0);       /* application-specific record begin */
////        QS_STR("Reg Data:");
////        QS_U8(1, pI2CData->bufferRx.pData[0]);
////        QS_U8(1, pI2CData->bufferRx.pData[1]);
////        QS_END();
//
//        if (ntagData.callbacks[NtagEvtDone]) {
//            ntagData.callbacks[NtagEvtDone](&ntagData);
//            NTAG_clrCallback(NtagEvtDone);
//        }
//
//        I2C_clrCallback(I2CEvtExchangeDone);            /* Clear the callback */
//    }
}

