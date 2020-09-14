/**
 * @file    ntag.h
 * @brief   NTAG5 NTA5332 driver for MSP430FR2433
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NTAG_H
#define __NTAG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "errors.h"
#include "buffers.h"
#include <stdbool.h>

/* Exported defines ----------------------------------------------------------*/

#define NTAG_I2C_ADDRESS            (0x54)              /**< Address of NTAG5 */

#define NTAG_MAX_WRITE_DELAY_MS     (10)            /**< Write delay for NTAG */
#define NTAG_I2C_BLOCK_SIZE         (0x04)     /**< Block Size for NTAG5 SRAM */

/**
 * @brief  Layout of SRAM
 * @{
 */
#define NTAG_MEM_BLOCK_START_SRAM       (0x2000)
#define NTAG_MEM_BLOCK_END_SRAM         (0x203F)
#define NTAG_MEM_SRAM_BLOCKS            (0x3F)
#define NTAG_MEM_SESSION_START          (0x10A0)
#define NTAG_MEM_SESSION_END            (0x10AF)
#define NTAG_MEM_EEPROM_END             (0x01FF)
#define NTAG_MEM_CONFIGURATION_START    (0x1000)
/**
 * @}
 */

/**
 * @brief   NTAG Error Codes
 * @{
 */
#define NTAG_ERR_OK                             (0x00)
#define NTAG_ERR_COMMUNICATION                  (-0x01)
#define NTAG_ERR_BUFF_OVERFLOW                  (-0x02)
#define NTAG_ERR_INIT_FAILED                    (-0x03)
#define NTAG_ERR_INVALID_PARAM                  (-0x09)
/**
 * @}
 */

/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/**
 * @brief   Register number offsets in NTAG register array
 * @{
 */
typedef enum {
    NTAG_MEM_OFFSET_TAG_STATUS_REG          = 0x00,
    NTAG_MEM_OFFSET_TAG_CONFIG0_REG         = 0x01,
    NTAG_MEM_OFFSET_TAG_CONFIG1_REG         = 0x02,
    NTAG_MEM_OFFSET_NDEF_DATA_BLOCK_REG     = 0x03,
    NTAG_MEM_OFFSET_PWM_GPIO_CONFIG_REG     = 0x04,
    NTAG_MEM_OFFSET_PWM0_ON_REG             = 0x05,
    NTAG_MEM_OFFSET_PWM0_OFF_REG            = 0x06,
    NTAG_MEM_OFFSET_PWM1_ON_REG             = 0x07,
    NTAG_MEM_OFFSET_PWM1_OFF_REG            = 0x08,
    NTAG_MEM_OFFSET_WDT_CONF_REG            = 0x09,
    NTAG_MEM_OFFSET_WDT_EN_REG              = 0x0A,
    NTAG_MEM_OFFSET_EH_CONF_REG             = 0x0B,
    NTAG_MEM_OFFSET_ED_CONF_REG             = 0x0C,
    NTAG_MEM_OFFSET_I2C_SLAVE_CONF_REG      = 0x0D,
    NTAG_MEM_OFFSET_RESET_GEN_REG           = 0x0E,
    NTAG_MEM_OFFSET_ED_INTR_CLEAR_REG       = 0x0F,
    NTAG_MEM_OFFSET_I2CM_SLAVE_ADDR_REG     = 0x10,
    NTAG_MEM_OFFSET_I2CM_DATA_LEN_REG       = 0x11,
    NTAG_MEM_OFFSET_ALM_STATUS_REG          = 0x12,
    NTAG_MEM_OFFSET_I2CM_STATUS_REG         = 0x13,
} NTAGRegNumber_t;

/**
 * @}
 */

/**
 * @brief   NTAG5 Status0 Register bit masks
 * @{
 */
typedef enum {
    NTAG_REG_STATUS0_NFC_FIELD_OK_MASK          = 0x01,
    NTAG_REG_STATUS0_VCC_SUPPLY_OK_MASK         = 0x02,
    NTAG_REG_STATUS0_PT_TRANSFER_DIR_MASK       = 0x04,
    NTAG_REG_STATUS0_PT_SYNC_BLOCK_READ_MASK    = 0x08,
    NTAG_REG_STATUS0_PT_SYNC_BLOCK_WRITE_MASK   = 0x10,
    NTAG_REG_STATUS0_PT_SRAM_DATA_READY_MASK    = 0x20,
    NTAG_REG_STATUS0_PT_EEPROM_WRITE_ERR_MASK   = 0x40,
    NTAG_REG_STATUS0_PT_EEPROM_WRITE_BUSY_MASK  = 0x80,
} NTAGRegStatus0Mask_t;
/**
 * @}
 */

/**
 * @brief   NTAG events that can occur in the driver
 */
typedef enum {
    NtagEvtStart = 0,                     /**< For indexing through the enums */
    NtagEvtDone = NtagEvtStart,                 /**< NTAG initialization done */

    NtagEvtRegReadDone,                      /**< NTAG register read finished */
    NtagEvtRegWriteDone,                     /**< NTAG register read finished */
    NtagEvtMax,                           /**< For indexing through the enums */
} NtagEvt_t;

struct NtagData;      /**< Forward declaration to prevent circular dependency */

/**
 * @brief   NTAG callback function pointer type
 */
typedef void (*NtagCallback_t) (
        const struct NtagData* const              /**< [in] NTAG data pointer */
);

/**
 * @brief   I2C data
 * This structure holds any dynamic data for the I2C bus that should live in
 * RAM as opposed to flash.
 */
typedef struct NtagData {
    Error_t         status;                               /**< Current status */
    NtagCallback_t  callbacks[NtagEvtMax];            /**< Array of callbacks */
    Buffer_t        bufferTx;                      /**< TX Buffer information */
    Buffer_t        bufferRx;                      /**< RX Buffer information */
    bool            isBusy;         /**< Disallow new calls while we are busy */
    NTAGRegNumber_t currRegNumber;   /**< which register we are currently r/w */
} NtagData_t;

/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif                                                            /* __NTAG_H */
