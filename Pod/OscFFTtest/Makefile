# Project Name
TARGET = OscFFTtest

# Sources
CPP_SOURCES = OscFFTtest.cpp

C_INCLUDES = -Istmlib/

LDFLAGS += -u _printf_float

# Library Locations
LIBDAISY_DIR = ../../../DaisyExamples/libDaisy/
DAISYSP_DIR = ../../../DaisyExamples/DaisySP/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
