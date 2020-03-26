#####################################################################
#
#  Name:         Makefile
#
#####################################################################
#
#--------------------------------------------------------------------
# The MIDASSYS should be defined prior the use of this Makefile
ifndef MIDASSYS
missmidas::
	@echo "...";
	@echo "Missing definition of environment variable 'MIDASSYS' !";
	@echo "...";
endif

#--------------------------------------------------------------------
# The following lines contain specific switches for different UNIX
# systems. Find the one which matches your OS and outcomment the 
# lines below.

#-----------------------------------------
# This is for Linux
ifeq ($(OSTYPE),Linux)
OSTYPE = linux
endif

#ifeq ($(OSTYPE),linux)

OS_DIR = linux-m64
OSFLAGS = -DOS_LINUX
CFLAGS = -std=c++11 -g -O2 -Wall -fpermissive
LIBS = -lm -lz -lutil -lnsl -lpthread -lrt
#endif

#-----------------------
# MacOSX/Darwin is just a funny Linux
#
ifeq ($(OSTYPE),Darwin)
OSTYPE = darwin
endif

ifeq ($(OSTYPE),darwin)
OS_DIR = darwin
FF = cc
OSFLAGS = -DOS_LINUX -DOS_DARWIN -DHAVE_STRLCPY -DAbsoftUNIXFortran -fPIC -Wno-unused-function
LIBS = -lpthread -lrt
SPECIFIC_OS_PRG = $(BIN_DIR)/mlxspeaker
NEED_STRLCPY=
NEED_RANLIB=1
NEED_SHLIB=
NEED_RPATH=

endif

#-------------------------------------------------------------------
# The following lines define directories. Adjust if necessary
#
MIDAS_INC = $(MIDASSYS)/include
MIDAS_LIB = $(MIDASSYS)/lib
MIDAS_SRC = $(MIDASSYS)/src
MIDAS_DRV = $(MIDASSYS)/drivers/vme

BUILD_DIR = build

# Hardware driver can be (camacnul, kcs2926, kcs2927, hyt1331)
#

ARDUINO_BUILD_DIR = $(BUILD_DIR)/arduino
ARDUINO_DEFS = -DPLATFORM_MIDAS

ARDUINO_LIB_SHARED = ../shared
ARDUINO_LIB_SHARED_LINUX = ../shared_linux

ARDUINO_LIB_UTIL = $(ARDUINO_LIB_SHARED)/Util
ARDUINO_LIB_TSC = $(ARDUINO_LIB_SHARED)/TestStandComm
ARDUINO_LIB_TSCH = $(ARDUINO_LIB_SHARED_LINUX)/TestStandCommHost
ARDUINO_LIB_LSD = $(ARDUINO_LIB_SHARED_LINUX)/LinuxSerialDevice

ARDUINO_INCS += -I$(ARDUINO_LIB_TSC) -I$(ARDUINO_LIB_TSCH) -I$(ARDUINO_LIB_LSD) -I$(ARDUINO_LIB_UTIL) -I$(ARDUINO_LIB_SHARED)

ARDUINO_SRCS = $(addprefix $(ARDUINO_LIB_TSC)/, SerialSession.cxx SerialTransport.cxx TestStandComm.cxx) \
               $(addprefix $(ARDUINO_LIB_TSCH)/, TestStandCommHost.cxx) \
               $(addprefix $(ARDUINO_LIB_LSD)/, LinuxSerialDevice.cxx) \
               motorCalculation.cxx

ARDUINO_OBJS = $(patsubst %.cxx, $(ARDUINO_BUILD_DIR)/%.o, $(notdir $(ARDUINO_SRCS)))

VPATH += $(dir $(ARDUINO_SRCS))

DRIVERS = $(ARDUINO_OBJS)

#-------------------------------------------------------------------
# Frontend code name defaulted to frontend in this example.
# comment out the line and run your own frontend as follow:
# gmake UFE=my_frontend
#
UFE = feArduino

####################################################################
# Lines below here should not be edited
####################################################################
#
# compiler
CC   = gcc
CXX  = g++
#
# MIDAS library
LIBMIDAS = -L$(MIDAS_LIB) -lmidas
#
#
# All includes
INCS = -I. -I$(MIDAS_INC) -I$(MIDAS_DRV) 
all: $(UFE).exe

$(UFE).exe: $(LIB) $(MIDAS_LIB)/mfe.o $(DRIVERS) $(BUILD_DIR)/$(UFE).o
	$(CXX) $(CFLAGS) $(OSFLAGS) $(INCS) -o $(UFE).exe $(BUILD_DIR)/$(UFE).o $(DRIVERS) \
	$(MIDAS_LIB)/mfe.o $(LIBMIDAS) $(LIBS)

$(BUILD_DIR)/feArduino.o: feArduino.cxx
	$(CXX) $(CFLAGS) $(INCS) $(ARDUINO_INCS) $(OSFLAGS) -o $@ -c $<

$(ARDUINO_BUILD_DIR)/%.o : %.cxx
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(ARDUINO_INCS) $(ARDUINO_DEFS) -o $@ -c $<

$(MIDAS_LIB)/mfe.o:
	@cd $(MIDASSYS) && make

# %.o: %.cxx
# 	$(CXX) $(USERFLAGS) $(CFLAGS) $(OSFLAGS) $(INCS) -o $@ -c $<

clean::
	@rm -f *.exe *.o *~ \#*
	@rm -rf $(BUILD_DIR)

#end file
