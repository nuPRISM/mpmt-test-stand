CC   = gcc
CXX  = g++

CFLAGS = -g -O2 -Wall -fpermissive

TARGET = MessageTerminal

BUILD_DIR = build

INCS = -I.
SRCS = MessageTerminal.cxx
DEFS = -DPLATFORM_MIDAS

# TestStandComm
LIB_TSC = ../shared/TestStandComm
INCS += -I$(LIB_TSC)
SRCS += $(addprefix $(LIB_TSC)/, SerialSession.cxx SerialTransport.cxx TestStandComm.cxx)

# TestStandCommHost
LIB_TSCH = ../shared_linux/TestStandCommHost
INCS += -I$(LIB_TSCH)
SRCS += $(addprefix $(LIB_TSCH)/, TestStandCommHost.cxx)

# LinuxSerialDevice
LIB_LSD = ../shared_linux/LinuxSerialDevice
INCS += -I$(LIB_LSD)
SRCS += $(addprefix $(LIB_LSD)/, LinuxSerialDevice.cxx)

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
