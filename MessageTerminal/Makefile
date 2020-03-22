CC   = gcc
CXX  = g++

CFLAGS = -g -O2 -Wall -fpermissive

TARGET = MessageTerminal

BUILD_DIR = build

LIB_SHARED = ../shared
LIB_SHARED_LINUX = ../shared_linux

LIB_TSC = $(LIB_SHARED)/TestStandComm
LIB_UTIL = $(LIB_SHARED)/Util
LIB_TSCH = $(LIB_SHARED_LINUX)/TestStandCommHost
LIB_LSD = $(LIB_SHARED_LINUX)/LinuxSerialDevice

INCS = -I. -I$(LIB_SHARED) -I$(LIB_UTIL) -I$(LIB_TSC) -I$(LIB_TSCH) -I$(LIB_LSD)

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
