CFLAGS+=-O3
CFLAGS+=-g0

-include config.mk

##############################################################
# cross build
##############################################################
CROSS_COMPILE?=

CXX:=$(CROSS_COMPILE)g++
CC:=$(CROSS_COMPILE)gcc
AS:=$(CROSS_COMPILE)as
AR:=$(CROSS_COMPILE)ar
NM:=$(CROSS_COMPILE)nm
LD:=$(CROSS_COMPILE)ld
OBJDUMP:=$(CROSS_COMPILE)objdump
OBJCOPY:=$(CROSS_COMPILE)objcopy
RANLIB:=$(CROSS_COMPILE)ranlib
STRIP:=$(CROSS_COMPILE)strip
SIZE:=$(CROSS_COMPILE)size

TARGET:=$(shell $(CC) -dumpmachine)

all: true_all

##############################################################
# basic build rules and external variables
##############################################################
include ctrlsw_version.mk
include encoder_defs.mk
include base.mk
-include compiler.mk

##############################################################
# Libraries
##############################################################
-include lib_fpga/project.mk
include lib_app/project.mk
-include lib_common/project.mk
-include lib_rtos/project.mk
-include lib_scheduler/project.mk
-include lib_perfs/project.mk

ifneq ($(ENABLE_TRACES),0)
-include lib_trace/project.mk
endif
ifneq ($(ENABLE_ENCODER),0)
-include lib_common_enc/project.mk
-include lib_buf_mngt/project.mk
-include lib_rate_ctrl/project.mk
-include lib_bitstream/project.mk
-include lib_scheduler_enc/project.mk
-include lib_encode/project.mk
ifneq ($(ENABLE_TILE_SRC),0)
  -include lib_fbc_standalone/project.mk
endif
-include lib_conv_yuv/project.mk
endif

ifneq ($(ENABLE_DECODER),0)
-include lib_common_dec/project.mk
endif

-include ref.mk

##############################################################
# AL_Decoder
##############################################################
ifneq ($(ENABLE_DECODER),0)
  -include lib_parsing/project.mk
  -include lib_scheduler_dec/project.mk
  -include lib_decode/project.mk
  include exe_decoder/project.mk
endif

##############################################################
# AL_Encoder
##############################################################
ifneq ($(ENABLE_ENCODER),0)
  -include exe_encoder/project.mk
endif

##############################################################
# AL_Compress
##############################################################
ifneq ($(ENABLE_COMP),0)
  -include lib_fbc_standalone/project.mk
  -include exe_compress/project.mk
endif

##############################################################
# AL_Decompress
##############################################################
ifneq ($(ENABLE_COMP),0)
  -include exe_decompress/project.mk
endif

##############################################################
# AL_Resize
##############################################################
ifneq ($(ENABLE_RESIZE),0)
  -include exe_resize/project.mk
endif

##############################################################
# AL_PerfMonitor
##############################################################
ifneq ($(ENABLE_PERF),0)
  -include exe_perf_monitor/project.mk
endif

##############################################################
# Unit tests
##############################################################
-include test/test.mk

##############################################################
# Environment tests
##############################################################
-include exe_test_env/project.mk

##############################################################
# tools
##############################################################
-include app_mcu/integration_tests.mk
-include exe_vip/project.mk

INSTALL ?= install -c
PREFIX ?= /usr
HDR_INSTALL_OPT = -m 0644

INCLUDE_DIR := include
HEADER_DIRS_TMP := $(sort $(dir $(wildcard $(INCLUDE_DIR)/*/)))
HEADER_DIRS := $(HEADER_DIRS_TMP:$(INCLUDE_DIR)/%=%)
INSTALL_HDR_PATH := ${PREFIX}/include

install_headers:
	@echo $(HEADER_DIRS)
	for dirname in $(HEADER_DIRS); do \
		$(INSTALL) -d "$(INCLUDE_DIR)/$$dirname" "$(INSTALL_HDR_PATH)/$$dirname"; \
		$(INSTALL) $(HDR_INSTALL_OPT) "$(INCLUDE_DIR)/$$dirname"/*.h "$(INSTALL_HDR_PATH)/$$dirname"; \
	done; \
	$(INSTALL) $(HDR_INSTALL_OPT) "$(INCLUDE_DIR)"/*.h "$(INSTALL_HDR_PATH)/";

true_all: $(TARGETS)

.PHONY: true_all clean all
