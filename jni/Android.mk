LOCAL_PATH := $(call my-dir)

ROOT_DIR     := $(LOCAL_PATH)/..
MAIN_FBA_DIR := $(ROOT_DIR)/src

INCLUDE_CPLUSPLUS11_FILES := 0
INCLUDE_7Z_SUPPORT        := 1
EXTERNAL_ZLIB             := 0
BUILD_X64_EXE             := 0
WANT_NEOGEOCD             := 0
HAVE_NEON                 := 0

CFLAGS      :=
CXXFLAGS    :=
LDFLAGS     :=
SOURCES_C   :=
SOURCES_CXX :=
FBA_DEFINES :=

include $(ROOT_DIR)/makefile.libretro_common

COMMON_FLAGS := -DUSE_SPEEDHACKS -D__LIBRETRO__ -DANDROID -DFRONTEND_SUPPORTS_RGB565 -Wno-write-strings -DLSB_FIRST $(FBA_DEFINES)

# Build shared library including static C module
include $(CLEAR_VARS)
LOCAL_MODULE       := retro
LOCAL_SRC_FILES    := $(SOURCES_C) $(SOURCES_S) $(SOURCES_CXX)
LOCAL_C_INCLUDES   := $(INCLUDE_DIRS)
LOCAL_CFLAGS       := $(CFLAGS) $(COMMON_FLAGS)
LOCAL_CPPFLAGS     := $(CXXFLAGS) $(COMMON_FLAGS)
LOCAL_LDFLAGS      := -Wl,-version-script=$(MAIN_FBA_DIR)/burner/libretro/link.T
LOCAL_LDLIBS       := $(LDFLAGS)
LOCAL_CPP_FEATURES := exceptions rtti
LOCAL_DISABLE_FORMAT_STRING_CHECKS := true
include $(BUILD_SHARED_LIBRARY)
