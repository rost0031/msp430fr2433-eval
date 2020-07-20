#------------------------------------------------------------------------------
#  TOOLCHAIN SETUP - Common for all targets
#  MSP430 toolset (NOTE: You need to adjust to your machine)
#------------------------------------------------------------------------------
ifeq ($(GNU_MSP430),)

# For use with the GNU MSP430 compiler released by TI
GNU_MSP430  = ~/workspace/msp430-gcc-9.2.0.50
MSP430_EABI = msp430-elf

endif

# make sure that the GNU-ARM toolset exists...
ifeq ("$(wildcard $(GNU_MSP430))","")
$(error GNU_MSP430 toolset not found. Please adjust the compiler.mk file)
endif

AS     := $(GNU_MSP430)/bin/$(MSP430_EABI)-gcc
CC     := $(GNU_MSP430)/bin/$(MSP430_EABI)-gcc
CPP    := $(GNU_MSP430)/bin/$(MSP430_EABI)-gcc -E
LINK   := $(GNU_MSP430)/bin/$(MSP430_EABI)-gcc
OBJCPY := $(GNU_MSP430)/bin/$(MSP430_EABI)-objcopy
SIZE   := $(GNU_MSP430)/bin/$(MSP430_EABI)-size

MKDIR  := mkdir
ECHO   := echo

ifeq ($(OS),Windows_NT)
	ifeq ($(shell uname -o),Cygwin)
		RM    := rm -rf
	else
		RM    := del /q
	endif
else
	RM    := rm -rf
endif
                  