/**
 * @file    main.c
 * @brief   UART demo
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Includes ------------------------------------------------------------------*/
#include <msp430fr2433.h>  /* MSP430 variant used */
#include "qpc.h"
#include "bsp.h"
#include "signals.h"
#include "main_ao.h"
#include "ntag_ao.h"

/* Compile-time called macros ------------------------------------------------*/
Q_DEFINE_THIS_FILE

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables and Local objects ---------------------------------------*/

static QEvt const *qpcMainQueueSto[3];
static QEvt const *qpcNtagQueueSto[3];
static QF_MPOOL_EL(QpcMainEvt) smlPoolSto[3];                     /* sml pool */
static QF_MPOOL_EL(NtagReadRegEvt) medPoolSto[3];                 /* med pool */

/* Private function prototypes -----------------------------------------------*/

/* Public and Exported functions ---------------------------------------------*/

/******************************************************************************/
int main(void)
{

    QpcMain_ctor();                         /* instantiate Main active object */
    QpcNtag_ctor();                         /* instantiate NTAG active object */

    QF_init();       /* initialize the framework and the underlying RT kernel */
    BSP_init();                       /* initialize the Board Support Package */

    /* initialize event pools... */
    QF_poolInit(smlPoolSto, sizeof(smlPoolSto), sizeof(smlPoolSto[0]));
    QF_poolInit(medPoolSto, sizeof(medPoolSto), sizeof(medPoolSto[0]));

    /* start the active objects... */
    QACTIVE_START(AO_QpcMain,                                  /* AO to start */
            (uint_fast8_t)(2),                       /* QP priority of the AO */
            qpcMainQueueSto,                           /* event queue storage */
            Q_DIM(qpcMainQueueSto),                  /* queue length [events] */
            (void *)0,                            /* stack storage (not used) */
            0U,                                  /* size of the stack [bytes] */
            (QEvt *)0);                               /* initialization event */

    QACTIVE_START(AO_QpcNtag,                                  /* AO to start */
            (uint_fast8_t)(1),                       /* QP priority of the AO */
            qpcNtagQueueSto,                           /* event queue storage */
            Q_DIM(qpcNtagQueueSto),                  /* queue length [events] */
            (void *)0,                            /* stack storage (not used) */
            0U,                                  /* size of the stack [bytes] */
            (QEvt *)0);                               /* initialization event */

    return QF_run(); /* run the QF application */
}

/* Private functions ---------------------------------------------------------*/
