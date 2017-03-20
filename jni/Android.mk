HAVE_GRIFFIN    := 0
GIT_VERSION := " $(shell git rev-parse --short HEAD || echo unknown)"
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)


MAIN_FBA_DIR := ../src
FBA_BURN_DIR := $(MAIN_FBA_DIR)/burn
FBA_BURN_DRIVERS_DIR := $(MAIN_FBA_DIR)/burn/drv
FBA_BURNER_DIR := $(MAIN_FBA_DIR)/burner
LIBRETRO_DIR := $(FBA_BURNER_DIR)/libretro
FBA_CPU_DIR := $(MAIN_FBA_DIR)/cpu
FBA_LIB_DIR := $(MAIN_FBA_DIR)/dep/libs
FBA_INTERFACE_DIR := $(MAIN_FBA_DIR)/intf
FBA_GENERATED_DIR = $(MAIN_FBA_DIR)/dep/generated
FBA_SCRIPTS_DIR = $(MAIN_FBA_DIR)/dep/scripts
GRIFFIN_DIR := ../griffin-libretro
 
ifeq ($(TARGET_ARCH),arm)
	LOCAL_CXXFLAGS += -DANDROID_ARM -DARM -marm
	LOCAL_CFLAGS += -DANDROID_ARM -DARM -marm
endif

ifeq ($(TARGET_ARCH_ABI), armeabi)
	LOCAL_ARM_MODE := arm
endif

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
	LOCAL_CFLAGS += -munaligned-access
#	LOCAL_ARM_NEON := true
endif

ifeq ($(TARGET_ARCH),x86)
LOCAL_CXXFLAGS +=  -DANDROID_X86
endif

ifeq ($(TARGET_ARCH),mips)
LOCAL_CXXFLAGS += -DANDROID_MIPS -D__mips__ -D__MIPSEL__
endif

BURN_BLACKLIST := $(FBA_BURNER_DIR)/un7z.cpp \
	$(FBA_CPU_DIR)/arm7/arm7exec.c \
	$(FBA_CPU_DIR)/arm7/arm7core.c \
	$(FBA_CPU_DIR)/adsp2100/2100ops.c \
	$(FBA_CPU_DIR)/hd6309/6309tbl.c \
	$(FBA_CPU_DIR)/hd6309/6309ops.c \
	$(FBA_CPU_DIR)/konami/konamtbl.c \
	$(FBA_CPU_DIR)/konami/konamops.c \
	$(FBA_CPU_DIR)/m68k/m68k_in.c \
	$(FBA_CPU_DIR)/m6800/6800ops.c \
	$(FBA_CPU_DIR)/m6800/6800tbl.c \
	$(FBA_CPU_DIR)/m6805/6805ops.c \
	$(FBA_CPU_DIR)/m6809/6809ops.c \
	$(FBA_CPU_DIR)/m6809/6809tbl.c \
	$(FBA_CPU_DIR)/sh2/mksh2.cpp \
	$(FBA_CPU_DIR)/sh2/mksh2-x86.cpp \
	$(FBA_CPU_DIR)/m68k/m68kmake.c \
	$(FBA_CPU_DIR)/m68k/m68kfpu.c \
	$(FBA_BURNER_DIR)/wave_writer.cpp \
	$(FBA_CPU_DIR)/m68k/m68kdasm.c \
	$(FBA_LIBRETRO_DIR)/menu.cpp \
	$(FBA_CPU_DIR)/sh2/mksh2.cpp \
	$(FBA_BURNER_DIR)/sshot.cpp \
	$(FBA_BURNER_DIR)/conc.cpp \
	$(FBA_BURNER_DIR)/dat.cpp \
	$(FBA_BURNER_DIR)/cong.cpp \
	$(FBA_BURNER_DIR)/image.cpp \
	$(FBA_BURNER_DIR)/misc.cpp \
	$(FBA_BURNER_DIR)/state.cpp \
	$(FBA_CPU_DIR)/h6280/tblh6280.c \
	$(FBA_CPU_DIR)/m6502/t65sc02.c \
	$(FBA_CPU_DIR)/m6502/t65c02.c \
	$(FBA_CPU_DIR)/m6502/tdeco16.c \
	$(FBA_CPU_DIR)/m6502/tn2a03.c \
	$(FBA_CPU_DIR)/m6502/t6502.c \
	$(FBA_CPU_DIR)/nec/v25sfr.c \
	$(FBA_CPU_DIR)/nec/v25instr.c \
	$(FBA_CPU_DIR)/nec/necinstr.c \
	$(FBA_CPU_DIR)/mips3/mips3_dasm.cpp \
	$(FBA_CPU_DIR)/tms34010/tms34010_dasm.cpp \
	$(FBA_CPU_DIR)/tms34010/tms34010_newdasm.cpp \
	$(FBA_BURN_DIR)/drv/capcom/ctv_make.cpp \
	$(FBA_BURN_DIR)/drv/pgm/pgm_sprite_create.cpp \
	$(FBA_INTERFACE_DIR)/audio/aud_interface.cpp \
	$(FBA_CPU_DIR)/i8051/i8051ops.c

ifeq ($(HAVE_GRIFFIN), 1)
	GRIFFIN_CXX_SRC_FILES := $(GRIFFIN_DIR)/cps12.cpp $(GRIFFIN_DIR)/cps3.cpp $(GRIFFIN_DIR)/neogeo.cpp $(GRIFFIN_DIR)/pgm.cpp $(GRIFFIN_DIR)/snes.cpp $(GRIFFIN_DIR)/galaxian.cpp
	GRIFFIN_CXX_SRC_FILES += $(GRIFFIN_DIR)/cpu-m68k.cpp
	BURN_BLACKLIST += $(FBA_CPU_DIR)/m68000_intf.cpp
else
	CPS2_DIR := $(FBA_BURN_DRIVERS_DIR)/capcom
	CPS3_DIR := $(FBA_BURN_DRIVERS_DIR)/cps3
	GALAXIAN_DIR := $(FBA_BURN_DRIVERS_DIR)/galaxian
	NEOGEO_DIR := $(FBA_BURN_DRIVERS_DIR)/neogeo
	PGM_DIR := $(FBA_BURN_DRIVERS_DIR)/pgm
	SNES_DIR := $(FBA_BURN_DRIVERS_DIR)/snes
	SMS_DIR := $(FBA_BURN_DRIVERS_DIR)/sms
	M68K_DIR := $(FBA_CPU_DIR)/m68k
	MIPS3_DIR := $(FBA_CPU_DIR)/mips3
	MD_DIR := $(FBA_BURN_DRIVERS_DIR)/megadrive
	MIDWAY_DIR := $(FBA_BURN_DRIVERS_DIR)/midway
	PCE_DIR := $(FBA_BURN_DRIVERS_DIR)/pce
endif

ifeq ($(NO_MD), 1)
	MD_DIR := 
endif

ifeq ($(NO_PCE), 1)
	PCE_DIR :=
endif

ifeq ($(NO_SMS), 1)
	SMS_DIR :=
endif

ifeq ($(NO_CPS), 1)
	BURN_BLACKLIST += $(FBA_BURN_DRIVERS_DIR)/capcom/cps.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/cps2_crpt.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/cps_config.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/cps_draw.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/cps_mem.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/cps_obj.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/cps_pal.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/cps_run.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/cps_rw.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/cps_scr.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/cpsr.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/cpsrd.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/cpst.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/ctv.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/d_cps1.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/d_cps2.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/fcrash_snd.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/ps.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/ps_m.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/ps_z.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/qs.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/qs_c.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/qs_z.cpp \
		$(FBA_BURN_DRIVERS_DIR)/capcom/sf2mdt_snd.cpp
endif

ifeq ($(NO_NEO), 1)
	NEOGEO_DIR :=
endif

FBA_BURN_DIRS := $(FBA_BURN_DIR) \
	$(FBA_BURN_DIR)/devices \
	$(FBA_BURN_DIR)/snd \
	$(CPS2_DIR) \
	$(FBA_BURN_DRIVERS_DIR)/cave \
	$(FBA_BURN_DRIVERS_DIR)/coleco \
	$(CPS3_DIR) \
	$(FBA_BURN_DRIVERS_DIR)/dataeast \
	$(GALAXIAN_DIR) \
	$(FBA_BURN_DRIVERS_DIR)/irem \
	$(FBA_BURN_DRIVERS_DIR)/konami \
	$(MD_DIR) \
	$(MIDWAY_DIR) \
	$(FBA_BURN_DRIVERS_DIR)/msx \
	$(NEOGEO_DIR) \
	$(PCE_DIR) \
	$(PGM_DIR) \
	$(FBA_BURN_DRIVERS_DIR)/pre90s \
	$(FBA_BURN_DRIVERS_DIR)/psikyo \
	$(FBA_BURN_DRIVERS_DIR)/pst90s \
	$(FBA_BURN_DRIVERS_DIR)/sega \
	$(FBA_BURN_DRIVERS_DIR)/sg1000 \
	$(SMS_DIR) \
	$(SNES_DIR) \
	$(FBA_BURN_DRIVERS_DIR)/taito \
	$(FBA_BURN_DRIVERS_DIR)/toaplan \
	$(FBA_BURN_DRIVERS_DIR)

FBA_CPU_DIRS := $(FBA_CPU_DIR) \
	$(FBA_CPU_DIR)/adsp2100 \
	$(FBA_CPU_DIR)/arm \
	$(FBA_CPU_DIR)/arm7 \
	$(FBA_CPU_DIR)/h6280 \
	$(FBA_CPU_DIR)/hd6309 \
	$(FBA_CPU_DIR)/i8039 \
	$(FBA_CPU_DIR)/i8051 \
	$(FBA_CPU_DIR)/konami \
	$(FBA_CPU_DIR)/m6502 \
	$(FBA_CPU_DIR)/m6800 \
	$(FBA_CPU_DIR)/m6805 \
	$(FBA_CPU_DIR)/m6809 \
	$(M68K_DIR) \
	$(MIPS3_DIR) \
	$(FBA_CPU_DIR)/nec \
	$(FBA_CPU_DIR)/pic16c5x \
	$(FBA_CPU_DIR)/s2650 \
	$(FBA_CPU_DIR)/sh2 \
	$(FBA_CPU_DIR)/tlcs90 \
	$(FBA_CPU_DIR)/tms34010 \
	$(FBA_CPU_DIR)/z80

FBA_LIB_DIRS := $(FBA_LIB_DIR)/zlib

FBA_INTERFACE_DIRS := $(FBA_INTERFACE_DIR)/audio

FBA_SRC_DIRS := $(FBA_BURNER_DIR) $(FBA_BURN_DIRS) $(FBA_CPU_DIRS) $(FBA_BURNER_DIRS) $(FBA_INTERFACE_DIRS)

ifeq ($(EXTERNAL_ZLIB), 1)
	FBA_DEFINES += -DEXTERNAL_ZLIB
else
	FBA_SRC_DIRS += $(FBA_LIB_DIRS)
endif

# GIT HASH
ifneq ($(GIT_VERSION)," unknown")
   FBA_DEFINES += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif

LOCAL_SRC_FILES := $(GRIFFIN_CXX_SRC_FILES) $(filter-out $(BURN_BLACKLIST),$(foreach dir,$(FBA_SRC_DIRS),$(wildcard $(dir)/*.cpp)))

LOCAL_SRC_FILES += $(LIBRETRO_DIR)/bind_map.cpp $(LIBRETRO_DIR)/libretro.cpp $(LIBRETRO_DIR)/neocdlist.cpp 

LOCAL_SRC_FILES += $(filter-out $(BURN_BLACKLIST),$(foreach dir,$(FBA_SRC_DIRS),$(wildcard $(dir)/*.c)))

LOCAL_C_INCLUDES = $(FBA_BURNER_DIR)/win32 \
	$(LIBRETRO_DIR) \
	$(LIBRETRO_DIR)/tchar \
	$(FBA_BURN_DIR) \
	$(MAIN_FBA_DIR)/cpu \
	$(FBA_BURN_DIR)/snd \
	$(FBA_BURN_DIR)/devices \
	$(FBA_INTERFACE_DIR) \
	$(FBA_INTERFACE_DIR)/input \
	$(FBA_INTERFACE_DIR)/cd \
	$(FBA_INTERFACE_DIR)/audio \
	$(FBA_BURNER_DIR) \
	$(FBA_CPU_DIR) \
	$(FBA_CPU_DIR)/i8039 \
	$(FBA_CPU_DIR)/i8051 \
	$(FBA_LIB_DIR)/zlib \
	$(FBA_BURN_DIR)/drv/capcom \
	$(FBA_BURN_DIR)/drv/konami \
	$(FBA_BURN_DIR)/drv/dataeast \
	$(FBA_BURN_DIR)/drv/cave \
	$(FBA_BURN_DIR)/drv/neogeo \
	$(FBA_BURN_DIR)/drv/psikyo \
	$(FBA_BURN_DIR)/drv/sega \
	$(FBA_BURN_DIR)/drv/toaplan \
	$(FBA_BURN_DIR)/drv/taito \
	$(FBA_GENERATED_DIR) \
	$(FBA_LIB_DIR)

LOCAL_LDLIBS += -lz

ENDIANNESS_DEFINES := -DLSB_FIRST
GLOBAL_DEFINES := -DWANT_NEOGEOCD -Dlog2\(x\)=\(log\(x\)/log\(2\)\) $(ENDIANNESS_DEFINES)

LOCAL_CXXFLAGS += -O2 -std=gnu++11 -DNDEBUG -DUSE_SPEEDHACKS -D__LIBRETRO_OPTIMIZATIONS__ -D__LIBRETRO__ -Wno-write-strings -DANDROID -DFRONTEND_SUPPORTS_RGB565 $(GLOBAL_DEFINES)
LOCAL_CFLAGS = -O2 -std=gnu99 -DNDEBUG -DUSE_SPEEDHACKS -D__LIBRETRO_OPTIMIZATIONS__ -D__LIBRETRO__ -Wno-write-strings -DANDROID -DFRONTEND_SUPPORTS_RGB565 $(GLOBAL_DEFINES)

LOCAL_DISABLE_FORMAT_STRING_CHECKS := true

LOCAL_MODULE    := libretro

include $(BUILD_SHARED_LIBRARY)
