# Project Name
TARGET = MathsTest

# Sources
CPP_SOURCES = MathsTest.cpp

LDFLAGS += -u _printf_float

C_SOURCES = \
$(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Source/ComplexMathFunctions/arm_cmplx_mag_f32.c \
$(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Source/FastMathFunctions/arm_sin_f32.c \
$(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Source/FastMathFunctions/arm_cos_f32.c \
$(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Source/CommonTables/arm_common_tables.c

C_INCLUDES += \
-I. \
-Istmlib \
-I../../../ChowDSP/include/math_approx

# Library Locations
LIBDAISY_DIR = ../../../DaisyExamples/libDaisy/
DAISYSP_DIR = ../../../DaisyExamples/DaisySP/
STMLIB_DIR = ../../../DaisyExamples/stmlib

#CC sources
CC_SOURCES += $stmlib/atan.cc

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
