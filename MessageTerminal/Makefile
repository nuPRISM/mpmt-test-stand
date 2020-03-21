CC   = gcc
CXX  = g++

CFLAGS = -g -O2 -Wall -fpermissive

TARGET = MessageTerminal

BUILD_DIR = build

LIB_TSC = ../shared/TestStandComm
LIB_TSCH = ../shared_linux/TestStandCommHost
LIB_LSD = ../shared_linux/LinuxSerialDevice
LIB_UTIL = ../shared/Util

INCS = -I. -I$(LIB_TSC) -I$(LIB_TSCH) -I$(LIB_LSD) -I$(LIB_UTIL)

SRCS = MessageTerminal.cxx                                                               \
       $(addprefix $(LIB_TSC)/, SerialSession.cxx SerialTransport.cxx TestStandComm.cxx) \
       $(addprefix $(LIB_TSCH)/, TestStandCommHost.cxx)                                  \
       $(addprefix $(LIB_LSD)/, LinuxSerialDevice.cxx)

DEFS = -DPLATFORM_MIDAS

OBJS = $(patsubst %.cxx, $(BUILD_DIR)/%.o, $(notdir $(SRCS)))

VPATH := $(dir $(SRCS))

$(BUILD_DIR)/$(TARGET) : $(OBJS)
	@mkdir -p $(@D)
	@$(CXX) $(CFLAGS) $(INCS) $(DEFS) -o $@ $^

$(BUILD_DIR)/%.o : %.cxx
	@mkdir -p $(@D)
	@$(CXX) $(CFLAGS) $(INCS) $(DEFS) -c $< -o $@

.PHONY: clean

clean:
	@rm -rf $(BUILD_DIR)
