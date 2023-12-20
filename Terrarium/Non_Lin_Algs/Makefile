# Project Name
TARGET = Non_Lin_Algs

# Library Locations
LIBDAISY_DIR = ../../../DaisyExamples/libDaisy/
DAISYSP_DIR = ../../../DaisyExamples/DaisySP/

# Sources
CPP_SOURCES = Non_Lin_Algs.cpp highshelf.cpp \
	
C_SOURCES = \
	$(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_interpolate_f32.c \
	$(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_interpolate_init_f32.c  \
	$(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_decimate_f32.c  \
	$(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Source/FilteringFunctions/arm_fir_decimate_init_f32.c 

C_INCLUDES += \
-I$(LIBDAISY_DIR)/Drivers/CMSIS/DSP/Include 

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
