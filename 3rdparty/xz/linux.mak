ROOT=../..
include $(ROOT)/defs.mak

DIRECTORY = lzmalinux

ARCHIVE = liblzmalinux.a

CFLAGS += -Werror 
CFLAGS += -Wall 
CFLAGS += -Os
CFLAGS += -std=c99
CFLAGS += -fno-builtin
CFLAGS += -Wno-maybe-uninitialized
CFLAGS += -Wno-unused-function
CFLAGS += -nostdinc
CFLAGS += $(LINUX_CFLAGS)

ifndef PKGVERSION
$(error "PKGVERSION not defined")
endif

DEFINES += -DHAVE_CONFIG_H
DEFINES += -DVERSION="$(PKGVERSION)"

INCLUDES += -I. 
INCLUDES += -I$(TOP)
INCLUDES += -I$(TOP)/posix/include
INCLUDES += -I./xz/src/common
INCLUDES += -I./xz/src/liblzma/common
INCLUDES += -I./xz/src/liblzma/api
INCLUDES += -I./xz/src/liblzma/lz
INCLUDES += -I./xz/src/liblzma/check
INCLUDES += -I./xz/src/liblzma/simple
INCLUDES += -I./xz/src/liblzma/delta
INCLUDES += -I./xz/src/liblzma/lzma
INCLUDES += -I./xz/src/liblzma/rangecoder

BASE = ./xz/src/liblzma
SOURCES += $(BASE)/rangecoder/price_table.c
SOURCES += $(BASE)/rangecoder/price_tablegen.c
SOURCES += $(BASE)/simple/powerpc.c
SOURCES += $(BASE)/simple/ia64.c
SOURCES += $(BASE)/simple/sparc.c
SOURCES += $(BASE)/simple/simple_decoder.c
SOURCES += $(BASE)/simple/x86.c
SOURCES += $(BASE)/simple/armthumb.c
SOURCES += $(BASE)/simple/simple_coder.c
SOURCES += $(BASE)/simple/arm.c
SOURCES += $(BASE)/simple/simple_encoder.c
SOURCES += $(BASE)/delta/delta_encoder.c
SOURCES += $(BASE)/delta/delta_decoder.c
SOURCES += $(BASE)/delta/delta_common.c
SOURCES += $(BASE)/lzma/fastpos_table.c
SOURCES += $(BASE)/lzma/lzma_encoder.c
SOURCES += $(BASE)/lzma/lzma_encoder_optimum_normal.c
SOURCES += $(BASE)/lzma/lzma2_encoder.c
SOURCES += $(BASE)/lzma/fastpos_tablegen.c
SOURCES += $(BASE)/lzma/lzma_encoder_optimum_fast.c
SOURCES += $(BASE)/lzma/lzma_encoder_presets.c
SOURCES += $(BASE)/lzma/lzma2_decoder.c
SOURCES += $(BASE)/lzma/lzma_decoder.c
SOURCES += $(BASE)/check/crc32_table.c
#SOURCES += $(BASE)/check/crc32_small.c
SOURCES += $(BASE)/check/crc64_fast.c
SOURCES += $(BASE)/check/crc64_table.c
SOURCES += $(BASE)/check/crc32_fast.c
#SOURCES += $(BASE)/check/crc32_tablegen.c
SOURCES += $(BASE)/check/crc64_small.c
SOURCES += $(BASE)/check/check.c
SOURCES += $(BASE)/check/crc64_tablegen.c
SOURCES += $(BASE)/check/sha256.c
SOURCES += $(BASE)/lz/lz_decoder.c
SOURCES += $(BASE)/lz/lz_encoder_mf.c
SOURCES += $(BASE)/lz/lz_encoder.c
SOURCES += $(BASE)/common/vli_decoder.c
SOURCES += $(BASE)/common/block_decoder.c
SOURCES += $(BASE)/common/stream_buffer_decoder.c
SOURCES += $(BASE)/common/alone_encoder.c
SOURCES += $(BASE)/common/stream_flags_common.c
SOURCES += $(BASE)/common/block_buffer_encoder.c
SOURCES += $(BASE)/common/filter_encoder.c
SOURCES += $(BASE)/common/block_util.c
SOURCES += $(BASE)/common/vli_size.c
SOURCES += $(BASE)/common/vli_encoder.c
SOURCES += $(BASE)/common/stream_flags_decoder.c
SOURCES += $(BASE)/common/stream_encoder.c
SOURCES += $(BASE)/common/common.c
SOURCES += $(BASE)/common/easy_preset.c
SOURCES += $(BASE)/common/index_decoder.c
SOURCES += $(BASE)/common/easy_buffer_encoder.c
SOURCES += $(BASE)/common/stream_flags_encoder.c
SOURCES += $(BASE)/common/block_encoder.c
SOURCES += $(BASE)/common/index.c
SOURCES += $(BASE)/common/alone_decoder.c
SOURCES += $(BASE)/common/index_hash.c
SOURCES += $(BASE)/common/filter_flags_encoder.c
SOURCES += $(BASE)/common/filter_buffer_encoder.c
SOURCES += $(BASE)/common/filter_decoder.c
SOURCES += $(BASE)/common/easy_encoder_memusage.c
SOURCES += $(BASE)/common/easy_encoder.c
SOURCES += $(BASE)/common/block_header_decoder.c
SOURCES += $(BASE)/common/index_encoder.c
SOURCES += $(BASE)/common/filter_flags_decoder.c
SOURCES += $(BASE)/common/filter_common.c
SOURCES += $(BASE)/common/block_buffer_decoder.c
SOURCES += $(BASE)/common/easy_decoder_memusage.c
SOURCES += $(BASE)/common/filter_buffer_decoder.c
SOURCES += $(BASE)/common/hardware_physmem.c
SOURCES += $(BASE)/common/block_header_encoder.c
SOURCES += $(BASE)/common/stream_buffer_encoder.c
SOURCES += $(BASE)/common/auto_decoder.c
SOURCES += $(BASE)/common/stream_decoder.c
SOURCES += lzmaextras.c

OBJECTS=$(SOURCES:.c=.o)

include $(ROOT)/rules.mak
-include depend.mak
