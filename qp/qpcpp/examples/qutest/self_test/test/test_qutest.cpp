/// @file
/// @brief Fixture for QUTEST self-test
/// @ingroup qs
/// @cond
///***************************************************************************
/// Last updated for version 6.8.0
/// Last updated on  2020-03-30
///
///                    Q u a n t u m  L e a P s
///                    ------------------------
///                    Modern Embedded Software
///
/// Copyright (C) 2005-2018 Quantum Leaps, LLC. All rights reserved.
///
/// This program is open source software: you can redistribute it and/or
/// modify it under the terms of the GNU General Public License as published
/// by the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Alternatively, this program may be distributed and modified under the
/// terms of Quantum Leaps commercial licenses, which expressly supersede
/// the GNU General Public License and are specifically designed for
/// licensees interested in retaining the proprietary status of their code.
///
/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with this program. If not, see <www.gnu.org/licenses>.
///
/// Contact information:
/// <www.state-machine.com/licensing>
/// <info@state-machine.com>
///***************************************************************************
/// @endcond

#include "qpcpp.hpp" // for QUTEST

using namespace QP;

Q_DEFINE_THIS_FILE

//----------------------------------------------------------------------------
static uint8_t buffer[100];
static uint32_t myFun(void);

enum {
    FIXTURE_SETUP = QS_USER,
    FIXTURE_TEARDOWN,
    COMMAND_X,
    COMMAND_Y,
    MY_RECORD,
};

//----------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    QF::init();  // initialize the framework

    // initialize the QS software tracing
    Q_ALLEGE(QS_INIT(argc > 1 ? argv[1] : nullptr));

    // global filter
    QS_FILTER_ON(QS_ALL_RECORDS); // enable all QS records

    // dictionaries...
    QS_OBJ_DICTIONARY(buffer);
    QS_FUN_DICTIONARY(&myFun);

    QS_USR_DICTIONARY(FIXTURE_SETUP);
    QS_USR_DICTIONARY(FIXTURE_TEARDOWN);
    QS_USR_DICTIONARY(COMMAND_X);
    QS_USR_DICTIONARY(COMMAND_Y);
    QS_USR_DICTIONARY(MY_RECORD);

    return QF::run(); // run the tests
}

//............................................................................
void QS::onTestSetup(void) {
    QS_BEGIN(FIXTURE_SETUP, nullptr)
    QS_END()
}
//............................................................................
void QS::onTestTeardown(void) {
    QS_BEGIN(FIXTURE_TEARDOWN, nullptr)
    QS_END()
}

//............................................................................
//! callback function to execute user commands
void QS::onCommand(uint8_t cmdId,
                   uint32_t param1, uint32_t param2, uint32_t param3)
{
    (void)param1;
    (void)param2;
    (void)param3;

    switch (cmdId) {
        case COMMAND_X: {
            uint32_t x = myFun();
            QS_BEGIN(COMMAND_X, nullptr) // application-specific record
                QS_U32(0, x);
                // ...
            QS_END()
            break;
        }
        case COMMAND_Y: {
            QS_BEGIN(COMMAND_Y, nullptr) // application-specific record
                QS_FUN(&myFun);
                QS_MEM(buffer, param1);
                QS_STR((char const *)&buffer[33]);
            QS_END()
            break;
        }
        default:
            break;
    }
}

//............................................................................
// callback function to "massage" the event, if necessary
void QS::onTestEvt(QEvt *e) {
    (void)e;
#ifdef Q_HOST  // is this test compiled for a desktop Host computer?
#else // this test is compiled for an embedded Target system
#endif
}
//............................................................................
// callback function to output the posted QP events (not used here)
void QS::onTestPost(void const *sender, QActive *recipient,
                    QEvt const *e, bool status)
{
    (void)sender;
    (void)recipient;
    (void)e;
    (void)status;
}

//----------------------------------------------------------------------------
static uint32_t myFun(void) {
    QS_TEST_PROBE_DEF(&myFun)
    QS_TEST_PROBE(
        return qs_tp_;
    )
    return 0;
}

