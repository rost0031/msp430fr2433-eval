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
#include "stdint.h"

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

uint8_t data[3] = {0x00};
/* Private function prototypes -----------------------------------------------*/
/* Public and Exported functions ---------------------------------------------*/


/******************************************************************************/
void NTAG_init(void)
{


}

/******************************************************************************/
void NTAG_readReg(NTAGRegNumber_t regNum)
{
    data[0] = (uint8_t)(ntagRegisterMap[regNum][0] >> 8);
    data[1] = (uint8_t)(ntagRegisterMap[regNum][0]);
    data[2] = (uint8_t)(ntagRegisterMap[regNum][1]);

    uint8_t rxData[2] = {0x00};

    I2C_writeBlocking(NTAG_I2C_ADDRESS, sizeof(data), data);

    /* We have to first do a write and then do a read (See READ_REGISTER cmd) */

//    I2C_readBlocking( NTAG_I2C_ADDRESS, data, sizeof(data), rxData, sizeof(rxData));

}

/******************************************************************************/
void NTAG_writeReg(void)
{

}




/* Private functions ---------------------------------------------------------*/

/* Interrupt vectors ---------------------------------------------------------*/


