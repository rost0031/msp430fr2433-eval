/*.$file${.::main_ao.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*
* Model: main_ao.qm
* File:  ${.::main_ao.h}
*
* This code has been generated by QM 5.0.2 <www.state-machine.com/qm/>.
* DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*/
/*.$endhead${.::main_ao.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/**
 * @file    main_ao.h
 * @brief   Main QPC Active Object
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_AO_H
#define __MAIN_AO_H

/* Includes ------------------------------------------------------------------*/
/* Exported defines ----------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/*.$declare${Events::QpcMainEvt} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/
/*.${Events::QpcMainEvt} ...................................................*/
typedef struct {
/* protected: */
    QEvt super;
} QpcMainEvt;
/*.$enddecl${Events::QpcMainEvt} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/* Exported constants --------------------------------------------------------*/
/*.$declare${AOs::AO_QpcMain} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/

/** "opaque" pointer to the Active Object */
extern QActive * const AO_QpcMain;
/*.$enddecl${AOs::AO_QpcMain} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/* Exported functions --------------------------------------------------------*/
/*.$declare${AOs::QpcMain_ctor} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv*/

/**
 * @brief     C "constructor" for this "class".
 *
 * Initializes all the timers and queues used by the AO and sets of the
 * first state.
 *
 * @return None
 */
/*.${AOs::QpcMain_ctor} ....................................................*/
void QpcMain_ctor(void);
/*.$enddecl${AOs::QpcMain_ctor} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


#endif                                                         /* __MAIN_AO_H */
