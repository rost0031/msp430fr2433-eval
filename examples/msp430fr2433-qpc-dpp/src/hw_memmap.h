#ifndef __HW_MEMMAP__
#define __HW_MEMMAP__

#define __DRIVERLIB_MSP430FR2XX_4XX_FAMILY__

//*****************************************************************************
//
// Include device specific header file
//
//*****************************************************************************
#include <msp430fr2433.h>


#ifndef ADCRES_1
#define ADCRES_1                         (0x0010)        /* 10 bit */
#endif
#ifndef ADCCONSEQ
#define ADCCONSEQ                        (0x0006)        /* conversion sequence mode select */
#endif

#include "stdint.h"
#include "stdbool.h"

//*****************************************************************************
//
// SUCCESS and FAILURE for API return value
//
//*****************************************************************************
#define STATUS_SUCCESS  0x01
#define STATUS_FAIL     0x00

//*****************************************************************************
//
// Macro for enabling assert statements for debugging - set by makefile
//
//*****************************************************************************
//#define NDEBUG

//*****************************************************************************
//
// Macros for hardware access
//
//*****************************************************************************
#define HWREG32(x)                                                              \
        (*((volatile uint32_t *)((uint16_t)x)))
#define HWREG16(x)                                                             \
        (*((volatile uint16_t *)((uint16_t)x)))
#define HWREG8(x)                                                             \
        (*((volatile uint8_t *)((uint16_t)x)))


#endif // #ifndef __HW_MEMMAP__
