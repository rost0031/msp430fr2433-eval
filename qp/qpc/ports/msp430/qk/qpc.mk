#-------------------------------------------------------------------------------
# This mk file is responsible for including all sources, include paths, and
# virtual paths required for inclusion by a Makefile that builds an FW image
# with QPC as an RTOS.
#
#-------------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Locations of sources and include files for QPC and the port we are using
#
QPC_DIR  := $(dir $(lastword $(MAKEFILE_LIST)))

QPC_INC_DIR                 = $(QPC_DIR)/../../../include
QPC_SRC_DIR                 = $(QPC_DIR)/../../../src
QF_SRC_DIR                  = $(QPC_SRC_DIR)/qf
QK_SRC_DIR                  = $(QPC_SRC_DIR)/qk
QS_SRC_DIR                  = $(QPC_SRC_DIR)/qs
QV_SRC_DIR                  = $(QPC_SRC_DIR)/qv
QXK_SRC_DIR                 = $(QPC_SRC_DIR)/qxk
QPC_PRT_DIR                 = $(QPC_DIR)
				   
#-----------------------------------------------------------------------------
# QK sources
#
A_SRCS                     += 

C_SRCS                     += qep_hsm.c \
                              qep_msm.c \
                              qf_act.c \
	                          qf_actq.c \
	                          qf_defer.c \
	                          qf_dyn.c \
	                          qf_mem.c \
	                          qf_ps.c \
	                          qf_qact.c \
	                          qf_qeq.c \
	                          qf_qmact.c \
	                          qf_time.c \
	                          qk.c \
	                          qstamp.c \
	                          
ifeq (spy, $(CONF))  # Spy configuration ................................
#-----------------------------------------------------------------------------
# QS sources (only if the target calls for it)
#
C_SRCS                     += qs.c \
	                          qs_rx.c \
	                          qs_fp.c \
	                          qs_64bit.c \
	                          qutest.c
endif

#-----------------------------------------------------------------------------
# Combine all the sources, include paths, and vpaths into handy variables 
# usable by the calling makefile
#
QPC_VPATH                   = $(QPC_PRT_DIR) \
                              $(QPC_INC_DIR) \
                              $(QF_SRC_DIR) \
                              $(QS_SRC_DIR) \
                              $(QK_SRC_DIR)
                              
                              
QPC_INC_PATHS               = -I$(QPC_INC_DIR) \
                              -I$(QPC_PRT_DIR) \
                              -I$(QPC_SRC_DIR)
                              
INCLUDES                   += $(QPC_INC_PATHS)
VPATH                      += $(QPC_VPATH)
  