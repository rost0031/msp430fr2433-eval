##############################################################################
# Product: Makefile for MSP430FR2433 DevKit
#
# Copyright (C) 2020 Harry Rostovtsev. All rights reserved.
#
##############################################################################
# examples of invoking this Makefile:
# building configurations: Debug (default) and Release
#
# make CONF={rel|spy|dbg (default)} all
# make CONF={rel|spy|dbg (default)} clean
# 
# To control output from compiler/linker, use the following flag 
# If TRACE=0 -->TRACE_FLAG=
# If TRACE=1 -->TRACE_FLAG=@
# If TRACE=something -->TRACE_FLAG=something
TRACE                       = 0
TRACEON                     = $(TRACE:0=@)
TRACE_FLAG                  = $(TRACEON:1=)

# Get the current build date and time
NOW                        := $(shell date +%Y%m%d%H%M%S | tr ' :' '__')

# Output file basename (should be the same as the directory name)
TARGET                     := msp430fr2433
PROJECT_BASE               := qpc-simple
PROJECT_NAME               := $(PROJECT_BASE)-$(TARGET)
TARGET_HEX                  = $(BIN_DIR)/$(PROJECT_NAME).hex
TARGET_ELF                  = $(BIN_DIR)/$(PROJECT_NAME).elf
TARGET_BIN                  = $(BIN_DIR)/$(PROJECT_NAME).bin
TARGET_MAP                  = $(BIN_DIR)/$(PROJECT_NAME).map

#-----------------------------------------------------------------------------
# DIRECTORIES
#-----------------------------------------------------------------------------

TOP_DIR                 = ../../
MSP430_DIR              = $(TOP_DIR)/msp430-gcc-support-files/include
#MSP430_DIR              = ../../../msp430-gcc/include
SRC_DIR                 = ./src
QPC_DIR                 = $(TOP_DIR)/qp/qpc
#-----------------------------------------------------------------------------
# INCLUDES FOR MAKEFILE
#-----------------------------------------------------------------------------

# Get info about the compiler from our common mk file
-include $(TOP_DIR)/make/compiler.mk

# version.mk also must contain the version number of components
-include version.mk

# QPC makefile for MSP430 QK port
-include $(QPC_DIR)/ports/msp430/qk/qpc.mk

#-----------------------------------------------------------------------------
# SOURCE VIRTUAL DIRECTORIES
#-----------------------------------------------------------------------------
VPATH                  += \
                          $(SRC_DIR)

#-----------------------------------------------------------------------------
# INCLUDE DIRECTORIES
#-----------------------------------------------------------------------------
INCLUDES               += \
                         -I$(SRC_DIR) \
                         -I$(MSP430_DIR)
                          
#-----------------------------------------------------------------------------
# BUILD OPTIONS FOR VARIOUS CONFIGURATIONS
#-----------------------------------------------------------------------------

MSP430_CPU           := -mmcu=MSP430FR2433
#MSP430_CPU := -mcpu=msp430

# Common options
WARNINGS             += -Wall 

# Flags common to all compile and link flags
# Note: Be careful with -flto (link time optimization). Could cause issues
MFLAGS                = \
						$(MSP430_CPU) \
						-mtiny-printf \
						-std=gnu17 \
						-ffunction-sections \
                        -fdata-sections \
						-ffreestanding \
                        -fno-common \
						-fno-builtin \
						$(WARNINGS)
										
										
										
#						-flto \
#						-fno-cse-follow-jumps \
											
ASFLAGS               = -c $(MFLAGS)

CFLAGS                = -c $(MFLAGS) \
						-W $(INCLUDES) $(DEFINES)  

LINKFLAGS            += $(MFLAGS) \
                         -Wl,-Map,$(TARGET_MAP) \
                         -Wl,--gc-sections \
                         -Wl,--discard-all
                         

# Options dependent on build type
ifeq (rel, $(CONF))       # Release configuration ............................
    BIN_DIR          := rel
    DEFINES          += -DNDEBUG
    ASFLAGS          +=     
    CFLAGS           += -Os
    LINKFLAGS        += -Wl,--strip-all 
else ifeq (spy, $(CONF))  # Spy configuration ................................
    BIN_DIR          := spy
    DEFINES          += -DQ_SPY
    ASFLAGS          += -g3
    CFLAGS           += -Os
    LINKFLAGS        += -g
    DEFINES          += 
else                     # default Debug configuration .......................
    BIN_DIR          := dbg
    DEFINES          += 
    ASFLAGS          += -g3   
    CFLAGS           += -Os -g3 -ggdb 
    LINKFLAGS        += -g 
endif

# Linker script and path						
LD_SCRIPT            := msp430fr2433.ld -static
LD_PATHS             := $(MSP430_DIR)
#-----------------------------------------------------------------------------
# DEFINES
#-----------------------------------------------------------------------------

DEFINES              += 

#-----------------------------------------------------------------------------
# FILES
#-----------------------------------------------------------------------------

# assembler source files
A_SRCS                 += 

# C source files
C_SRCS                 += main.c \
                          bsp.c \
                          cs.c \
                          i2c.c \
                          ntag.c \
                          main_ao.c \
                          ntag_ao.c
                          
                          
                          
A_OBJS                 = $(patsubst %.s,%.o,$(A_SRCS))
A_OBJS_EXT             = $(addprefix $(BIN_DIR)/, $(A_OBJS))
A_DEPS_EXT             = $(patsubst %.o, %.d, $(A_OBJS_EXT))

C_OBJS                 = $(patsubst %.c,%.o,$(C_SRCS))
C_OBJS_EXT             = $(addprefix $(BIN_DIR)/, $(C_OBJS))
C_DEPS_EXT             = $(patsubst %.o, %.d, $(C_OBJS_EXT))

# Make sure not to generate dependencies when doing cleans
NODEPS      := clean show showver ver clean_exe 
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
-include $(A_DEPS_EXT) $(C_DEPS_EXT)
endif


#-----------------------------------------------------------------------------
# BUILD TARGETS
#-----------------------------------------------------------------------------
# This sets a dependency between generating the proto files before compiling the
# rest of the source, allowing multicore compiles

.PHONY: all clean $(BIN_DIR) show clean_exe
.DEFAULT_GOAL := all

all: clean_exe $(TARGET_HEX) 

$(BIN_DIR):
	@echo --- Creating dir $@ 
	mkdir -p $@

$(TARGET_HEX): $(TARGET_ELF) | $(BIN_DIR)
	@echo --- Creating hex $@
	$(TRACE_FLAG)$(OBJCPY) -O binary $< $@
	$(TRACE_FLAG)mv $(TARGET_HEX) $(TARGET_BIN)
	@echo --- Creating bin $(TARGET_BIN)
	$(TRACE_FLAG)$(OBJCPY) -O ihex $< $@

$(TARGET_ELF): $(C_OBJS_EXT) $(A_OBJS_EXT) | $(BIN_DIR)
	@echo --- Building $(PROJECT_NAME)
	$(TRACE_FLAG)$(LINK) -L $(LD_PATHS) $(LINKFLAGS) $(LIB_PATHS)  -o $@ $^ $(LIBS)
	$(SIZE) $(TARGET_ELF)
	
$(BIN_DIR)/%.o : %.c | $(BIN_DIR)
	@echo --- Compiling $(<F)
	$(TRACE_FLAG)$(CC) $(CFLAGS) -D BASE_FILE_NAME=\"$(<F)\" -MD -MP -c $< -o $@
	
$(BIN_DIR)/%.o : %.s | $(BIN_DIR)
	@echo --- Compiling $(<F)
	$(TRACE_FLAG)$(AS) $(ASFLAGS) $< -o $@
	
$(BIN_DIR)/%.o : %.S | $(BIN_DIR)
	@echo --- Compiling $(<F)
	$(TRACE_FLAG)$(AS) $(ASFLAGS) $< -o $@

clean: 
	@echo --- Cleaning all binary files
	$(TRACE_FLAG)-$(RM) dbg rel spy

clean_exe:
	@echo --- Removing $(TARGET_HEX) $(TARGET_BIN) $(TARGET_ELF)
	$(TRACE_FLAG)-$(RM) $(TARGET_HEX) $(TARGET_BIN) $(TARGET_ELF)

show:
	@echo CONF             = $(CONF)
	@echo BIN_DIR          = $(BIN_DIR)
	@echo A_SRCS           = $(A_SRCS)
	@echo A_OBJS_EXT       = $(A_OBJS_EXT)
	@echo A_DEPS_EXT       = $(A_DEPS_EXT)
	@echo C_SRCS           = $(C_SRCS)
	@echo C_OBJS_EXT       = $(C_OBJS_EXT)
	@echo C_DEPS_EXT       = $(C_DEPS_EXT)
	@echo TARGET_HEX       = $(TARGET_HEX)
	@echo TARGET_BIN       = $(TARGET_BIN)
	@echo TARGET_ELF       = $(TARGET_ELF)
	@echo VPATH            = $(VPATH)
	@echo INCLUDES         = $(INCLUDES)
	@echo PROJ_DIR         = $(PROJ_DIR)

