# The TARGET variable determines what target system the application is 
# compiled for. It either refers to an XN file in the source directories
# or a valid argument for the --target option when compiling.

TARGET = xk-audio-316-mc.xn
APP_NAME =

# The flags passed to xcc when building the application
# The EXTRA_BUILD_FLAGS variable can be used on the xmake command line to add options
BUILD_FLAGS    = $(EXTRA_BUILD_FLAGS) -fcomment-asm -O3 -report -lquadflash -g -fxscope -DUSB_TILE=tile[0] -DADAT_TX_USE_SHARED_BUFF=1 -DQUAD_SPI_FLASH=1

# The USED_MODULES variable lists other module used by the application. These
# modules will extend the SOURCE_DIRS, INCLUDE_DIRS and LIB_DIRS variables. 
# Modules are expected to be in the directory above the BASE_DIR directory.
USED_MODULES = lib_xua lib_i2c lib_dsp

#
# Supported build configs (Note these make use of the defaults in xua_conf.h)
# All Audio Class 2, Async, I2S Master,  no Input, 2xOutput
#   USB_THREAD    DSP inside the USB thread
#   SINGLE_THREAD DSP in a separate thread
#   MULTI_THREAD  DSP in multiple parallel threads
#   PIPELINE      DSP in a pipeline
#
INCLUDE_ONLY_IN_PIPELINE =
XCC_FLAGS_PIPELINE = $(BUILD_FLAGS)        -DI2S_CHANS_DAC=2 \
                                           -DI2S_CHANS_ADC=0 \
									       -DDSP_PIPELINE

INCLUDE_ONLY_IN_MULTI_THREAD =
XCC_FLAGS_MULTI_THREAD = $(BUILD_FLAGS)    -DI2S_CHANS_DAC=2 \
                                           -DI2S_CHANS_ADC=0 \
									       -DDSP_MULTI_THREAD

INCLUDE_ONLY_IN_SINGLE_THREAD =
XCC_FLAGS_SINGLE_THREAD = $(BUILD_FLAGS)   -DI2S_CHANS_DAC=2 \
                                           -DI2S_CHANS_ADC=0 \
									       -DDSP_SINGLE_THREAD

INCLUDE_ONLY_IN_USB_THREAD =
XCC_FLAGS_USB_THREAD = $(BUILD_FLAGS)      -DI2S_CHANS_DAC=2 \
                                           -DI2S_CHANS_ADC=0 \
									       -DDSP_USB_THREAD


#=============================================================================
# The following part of the Makefile includes the common build infrastructure
# for compiling XMOS applications. You should not need to edit below here.

XMOS_MAKE_PATH ?= ../..
include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
