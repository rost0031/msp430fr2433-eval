/**
 * @file    bsp.c
 * @brief   MSP430 board support package
 *
 * Copyright 2020, Harry Rostovtsev.
 * All other rights reserved.
 */

/* Includes ------------------------------------------------------------------*/
#include <msp430fr2433.h>  /* MSP430 variant used */

#include "qpc.h"
#include "dpp.h"
#include "bsp.h"
#include "cs.h"


/* Compile-time called macros ------------------------------------------------*/
Q_DEFINE_THIS_FILE

/* Private typedef -----------------------------------------------------------*/

#ifdef Q_SPY
enum AppRecords { /* application-specific trace records */
    PHILO_STAT = QS_USER,
    COMMAND_STAT
};
#endif

/* Private define ------------------------------------------------------------*/
#define CS_MCLK_DESIRED_FREQUENCY_IN_KHZ    (8000)
#define CS_MCLK_FLLREF_RATIO                (243)

#ifdef Q_SPY
/* UART1 pins TX:P1.4, RX:P1.5 */
#define TXD     (BIT4)
#define RXD     (BIT5)
#endif

/* Private macros ------------------------------------------------------------*/
/* Private variables and Local objects ---------------------------------------*/

/* random seed */
static uint32_t l_rnd;

#ifdef Q_SPY
QSTimeCtr QS_tickTime_;
static uint8_t const l_timerA_ISR = 0U;
#endif

/* Local-scope objects -----------------------------------------------------*/
/* 1MHz clock setting, see BSP_init() */
#define BSP_MCK     1000000U
#define BSP_SMCLK   1000000U

#define LED1        (BIT0)
#define LED2        (BIT1)

#define BTN_S1      (1U << 1)

/* Private function prototypes -----------------------------------------------*/



/* Public and Exported functions ---------------------------------------------*/

/******************************************************************************/
void BSP_init(void) {
    WDTCTL = WDTPW | WDTHOLD; /* stop watchdog timer */

    // For FRAM devices, at start up, the GPO power-on default
    // high-impedance mode needs to be disabled to activate previously
    // configured port settings. This can be done by clearing the LOCKLPM5
    // bit in PM5CTL0 register
    PM5CTL0 &= ~LOCKLPM5;
    /* leave the MCK and SMCLK at default DCO setting */

    P1DIR |= (LED1 | LED2);  /* set LED1 and LED2 pins to output  */

    //Set DCO FLL reference = REFO
    CS_initClockSignal(
            CS_FLLREF,
            CS_REFOCLK_SELECT,
            CS_CLOCK_DIVIDER_1
    );

    //Set ACLK = REFO
    CS_initClockSignal(
            CS_ACLK,
            CS_REFOCLK_SELECT,
            CS_CLOCK_DIVIDER_1
    );

    CS_initClockSignal(
            CS_MCLK,
            CS_DCOCLKDIV_SELECT,
            CS_CLOCK_DIVIDER_1
    );

    CS_initClockSignal(
            CS_SMCLK,
            CS_DCOCLKDIV_SELECT,
            CS_CLOCK_DIVIDER_8
    );
    //Create struct variable to store proper software trim values
    CS_initFLLParam param = {0};

    //Set Ratio/Desired MCLK Frequency, initialize DCO, save trim values
    CS_initFLLCalculateTrim(
            CS_MCLK_DESIRED_FREQUENCY_IN_KHZ,
            CS_MCLK_FLLREF_RATIO,
            &param
    );

    //Clear all OSC fault flag
    CS_clearAllOscFlagsWithTimeout(1000);

    //For demonstration purpose, change DCO clock freq to 16MHz
    CS_initFLLSettle( 16000, 487 );

    //Clear all OSC fault flag
    CS_clearAllOscFlagsWithTimeout(1000);

    //Reload DCO trim values that were calculated earlier
    CS_initFLLLoadTrim(
            CS_MCLK_DESIRED_FREQUENCY_IN_KHZ,
            CS_MCLK_FLLREF_RATIO,
            &param
    );

    //Clear all OSC fault flag
    CS_clearAllOscFlagsWithTimeout(1000);

    if (QS_INIT((void *)0) == 0) { /* initialize the QS software tracing */
        Q_ERROR();
    }
    QS_OBJ_DICTIONARY(&l_timerA_ISR);
    QS_USR_DICTIONARY(PHILO_STAT);
    QS_USR_DICTIONARY(COMMAND_STAT);
}

/******************************************************************************/
void BSP_displayPhilStat(uint8_t n, char const *stat) {
    if (stat[0] == 'h') { /* is Philo hungry? */
        P1OUT |=  LED1;  /* turn LED1 on */
    }
    else {
        P1OUT &= ~LED1;  /* turn LED1 off */
    }

    QS_BEGIN(PHILO_STAT, AO_Philo[n]) /* application-specific record begin */
        QS_U8(1, n);                  /* Philosopher number */
        QS_STR(stat);                 /* Philosopher status */
    QS_END()
}

/******************************************************************************/
void BSP_displayPaused(uint8_t paused)
{
    /* not enough LEDs to implement this feature */
    if (paused != 0U) {
        //P1OUT |=  LED1;
    } else {
        //P1OUT &= ~LED1;
    }
}

/******************************************************************************/
uint32_t BSP_random(void)
{
    /* "Super-Duper" Linear Congruential Generator (LCG)
    * LCG(2^32, 3*7*11*13*23, 0, seed)
    */
    l_rnd = l_rnd * ((uint32_t)3U*7U*11U*13U*23U);

    return l_rnd >> 8;
}

/******************************************************************************/
void BSP_randomSeed(uint32_t seed)
{
    l_rnd = seed;
}

/******************************************************************************/
void BSP_terminate(int16_t result)
{
    (void)result;
}


/* QF callbacks ============================================================*/

/******************************************************************************/
void QF_onStartup(void)
{
    uint16_t clockValueSMCLK = CS_getSMCLK();
    TA0CCTL0 = CCIE;                          // CCR0 interrupt enabled
    TA0CCR0 = clockValueSMCLK;
    TA0CTL = TASSEL__SMCLK | MC_1 | TACLR;         // SMCLK, upmode, clear TAR

//    TA0CCTL0 = CCIE;                          // CCR0 interrupt enabled
//    TA0CCR0 = BSP_MCK / BSP_TICKS_PER_SEC;
//    TA0CTL = TASSEL_2 + MC_1 + TACLR;         // SMCLK, upmode, clear TAR
}

/******************************************************************************/
void QF_onCleanup(void) {
}

/******************************************************************************/
void QK_onIdle(void) {
    /* toggle LED2 on and then off, see NOTE2 */
    QF_INT_DISABLE();
    P1OUT |=  LED2;  /* turn LED2 on */
    P1OUT &= ~LED2;  /* turn LED2 off */
    QF_INT_ENABLE();

#ifdef Q_SPY
//    QS_rxParse();  /* parse all the received bytes */

    if ((UCA0STATW & UCBUSY) == 0U) { /* TX NOT busy? */

        uint16_t b;

        QF_INT_DISABLE();
        b = QS_getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {
            UCA0TXBUF = (uint8_t)b; /* stick the byte to the TX BUF */
        }
    }
#elif defined NDEBUG
    /* Put the CPU and peripherals to the low-power mode.
    * you might need to customize the clock management for your application,
    * see the datasheet for your particular MSP430 MCU.
    */
    __low_power_mode_1(); /* enter LPM1; also ENABLES interrupts, see NOTE1 */
#endif
}

/******************************************************************************/
Q_NORETURN Q_onAssert(char_t const * const module, int_t const loc) {
    /*
    * NOTE: add here your application-specific error handling
    */
    (void)module;
    (void)loc;
    QS_ASSERTION(module, loc, 10000U); /* report assertion to QS */

    /* write invalid password to WDT: cause a password-validation RESET */
    WDTCTL = 0xDEAD;
}

/* QS callbacks ============================================================*/
#ifdef Q_SPY



/******************************************************************************/
uint8_t QS_onStartup(void const *arg) {
    static uint8_t qsBuf[256];  /* buffer for QS; RAM is tight! */
//    static uint8_t qsRxBuf[80]; /* buffer for QS receive channel */
    //uint16_t tmp;

    QS_initBuf(qsBuf, sizeof(qsBuf));
//    QS_rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    /* USCI setup code... */
    P1SEL0 |= (BIT4 | BIT5);                             /* Configure UART pins */
    UCA0CTLW0 |= UCSWRST;                         /* reset USCI state machine */
    UCA0CTLW0 |= UCSSEL__SMCLK;                     /* choose the SMCLK clock */
#if 0 /* 9600 baud rate */
    UCA1BR0 = 6; /* 1MHz 9600 (see User's Guide) */
    UCA1BR1 = 0; /* 1MHz 9600 */
    UCA1MCTL = UCBRS_0 | UCBRF_13 | UCOS16; /* modulationUCBRSx=0, UCBRFx=0, oversampling */
#else /* 115200 baud rate */
    UCA0BR0 = 8;                                     /* 1000000/115200 = 8.68 */
    UCA0BR1 = 0;
    UCA0MCTLW = 0xD600;          /* 1000000/115200 - INT(1000000/115200)=0.68 */
#endif
    UCA0CTLW0 &= ~UCPEN;    /* No parity */

    UCA0CTL1 &= ~UCSWRST;  /* initialize USCI state machine */
    UCA0IE |= UCRXIE;      /* Enable USCI_A1 RX interrupt */

    /* setup the QS filters... */
    QS_FILTER_ON(QS_SM_RECORDS);
    //QS_FILTER_ON(QS_AO_RECORDS);
    QS_FILTER_ON(QS_UA_RECORDS);

    return 1U; /* return success */
}

/******************************************************************************/
void QS_onCleanup(void)
{
}

/******************************************************************************/
QSTimeCtr QS_onGetTime(void)
{  /* invoked with interrupts DISABLED */
    if ((TA0CTL & TAIFG) == 0U) {  /* interrupt not pending? */
        return QS_tickTime_ + TA0R;
    }
    else { /* the rollover occured, but the timerA_ISR did not run yet */
        return QS_tickTime_
           + (((BSP_SMCLK/8U) + BSP_TICKS_PER_SEC/2U)/BSP_TICKS_PER_SEC) + 1U
           + TA0R;
    }
}

/******************************************************************************/
void QS_onFlush(void)
{
    uint16_t b;
    QF_INT_DISABLE();
    while ((b = QS_getByte()) != QS_EOD) { /* next QS byte available? */
        QF_INT_ENABLE();
        while ((UCA0STATW & UCBUSY) != 0U) { /* TX busy? */
        }
        UCA0TXBUF = (uint8_t)b; /* stick the byte to the TX BUF */
        QF_INT_DISABLE();
    }
    QF_INT_ENABLE();
}

/*! callback function to reset the target (to be implemented in the BSP) */
/******************************************************************************/
void QS_onReset(void) {
    /* write invalid password to WDT: cause a password-validation RESET */
    WDTCTL = 0xDEAD;
}

/******************************************************************************/
/*! callback function to execute a user command (to be implemented in BSP) */
void QS_onCommand(uint8_t cmdId,
                  uint32_t param1, uint32_t param2, uint32_t param3)
{
    (void)cmdId;
    (void)param1;
    (void)param2;
    (void)param3;
    QS_BEGIN(COMMAND_STAT, (void *)1) /* application-specific record begin */
        QS_U8(2, cmdId);
        QS_U32(8, param1);
        QS_U32(8, param2);
        QS_U32(8, param3);
    QS_END()
}

#endif /* Q_SPY */

/* ISRs used in this project =================================================*/

#ifdef Q_SPY
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
__interrupt void USCI_A0_ISR(void); /* prototype */
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
__attribute__ ((interrupt(USCI_A0_VECTOR)))
void USCI_A0_ISR(void)
#else
    #error MSP430 compiler not supported!
#endif
{
    /* NOTE: no need to call QK_ISR_ENTRY/EXIT */
    if (UCA0IV == 2) {
        uint16_t b = UCA0RXBUF;
        QS_RX_PUT(b);
    }
}

#endif /* Q_SPY */
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
__interrupt void TIMER0_A0_ISR(void); /* prototype */
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
#elif defined(__GNUC__)
__attribute__ ((interrupt(TIMER0_A0_VECTOR)))
void TIMER0_A0_ISR(void)
#else
#error MSP430 compiler not supported!
#endif
{
#ifdef NDEBUG
    __low_power_mode_off_on_exit(); /* see NOTE1 */
#endif

#ifdef Q_SPY
    QS_tickTime_ +=
       (((BSP_SMCLK / 8) + BSP_TICKS_PER_SEC/2) / BSP_TICKS_PER_SEC) + 1;
#endif
    QK_ISR_ENTRY();    /* inform QK about entering the ISR */

    QF_TICK_X(0U, (void *)0);  /* process all time events at rate 0 */

    QK_ISR_EXIT();     /* inform QK about exiting the ISR */

#ifdef NDEBUG
    __low_power_mode_off_on_exit(); /* turn the low-power mode OFF, NOTE1 */
#endif
}
/*****************************************************************************
* NOTE1:
* With the preemptive QK kernel for MSP430, the idle callback QK::onIdle()
* will execute only ONCE, if the low-power mode is not explicitly turned OFF
* in the interrupt. This might or might not be what you want.
*
* NOTE2:
* One of the LEDs is used to visualize the idle loop activity. The brightness
* of the LED is proportional to the frequency of invocations of the idle loop.
* Please note that the LED is toggled with interrupts disabled, so no
* interrupt execution time contributes to the brightness of the User LED.
*/

/* Private functions ------- -----------------------------------------------*/
static void __attribute__((naked, section(".crt_0042"), used))
disable_watchdog (void)
{
    WDTCTL = WDTPW | WDTHOLD;
}

