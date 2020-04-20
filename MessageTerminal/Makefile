CC   = gcc
CXX  = g++

# --std=c++11     : required to use nullptr
# -g              : generate debug information
# -O2             : enable moderate optimization
# -Wall           : enable all warning messages
CFLAGS = -std=c++11 -g -O2 -Wall

TARGET = MessageTerminal

BUILD_DIR = build

LIB_SHARED = ../shared
LIB_SHARED_LINUX = ../shared_linux
LIB_FIRMWARE = ../firmware

LIB_TSC = $(LIB_SHARED)/TestStandComm
LIB_TSCH = $(LIB_SHARED_LINUX)/TestStandCommHost
LIB_LSD = $(LIB_SHARED_LINUX)/LinuxSerialDevice
LIB_GANTRY = $(LIB_FIRMWARE)/lib/Gantry/include
LIB_TEMP = $(LIB_FIRMWARE)/lib/TemperatureDAQ/include

INCS = -I. -I$(LIB_SHARED) -I$(LIB_TSC) -I$(LIB_TSCH) -I$(LIB_LSD) -I$(LIB_FIRMWARE)/include -I$(LIB_GANTRY) -I$(LIB_TEMP)

SRCS = MessageTerminal.cxx                                                               \
       $(addprefix $(LIB_TSC)/, SerialSession.cxx SerialTransport.cxx TestStandComm.cxx) \
       $(addprefix $(LIB_TSCH)/, TestStandCommHost.cxx)                                  \
       $(addprefix $(LIB_LSD)/, LinuxSerialDevice.cxx)

DEFS = -DPLATFORM_MIDAS

OBJS = $(patsubst %.cxx, $(BUILD_DIR)/%.o, $(notdir $(SRCS)))

VPATH := $(dir $(SRCS))

$(BUILD_DIR)/$(TARGET) : $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(INCS) $(DEFS) -o $@ $^

$(BUILD_DIR)/%.o : %.cxx
	@mkdir -p $(@D)
	$(CXX) $(CFLAGS) $(INCS) $(DEFS) -c $< -o $@

.PHONY: clean

clean:
	@rm -rf $(BUILD_DIR)
