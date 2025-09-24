# 'DEBUG=1': Build a debug binary
# 'EXCEPTIONS=1': Build with exceptions included in the runtime (if C++)

# Directories
SRC     := source
LIB     := lib
INC     := include
BIN   	:= bin
BUILD   := build
TARGET  := $(notdir $(CURDIR))
LIBOGC 	:= libogc

export DEVKITPRO := $(BIN)
export DEVKITPPC := $(DEVKITPRO)/devkitPPC
export PATH := $(DEVKITPRO)/tools/bin:$(DEVKITPPC)/bin:$(PATH)
#export PATH := /opt/devkitpro/tools/bin:/opt/devkitpro/devkitPPC/bin:$(PATH)

CC      := powerpc-eabi-gcc
CXX     := powerpc-eabi-g++
LD      := $(CXX)

MACHDEP := -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float
CFLAGS  := -O2 -Wall -w $(MACHDEP) -I$(SRC) -I$(SRC)/include -I$(INC)/$(LIBOGC) -I$(INC)/$(LIBOGC)/ogc/machine
CXXFLAGS:= $(CFLAGS)
LDFLAGS := -O2 $(MACHDEP) -L$(LIB)/$(LIBOGC) -lwiiuse -lbte -logc -lfat -lm

CPPFILES := $(wildcard $(SRC)/*.cpp)
OBJECTS  := $(patsubst $(SRC)/%.cpp,$(BUILD)/%.o,$(CPPFILES))

ifeq ($(DEBUG), 1)
    CXXFLAGS += -DDEBUG
    $(info Debug mode is enabled)
endif

ifeq ($(EXCEPTIONS), 1)
    CXXFLAGS += -fexceptions
    $(info Exceptions are enabled)
endif

ifeq ($(NTFS_SUPPORT), 1)
	CXXFLAGS += -DNTFS_SUPPORT
endif

# Default target
all: $(TARGET).dol

# Compile C++ sources
$(BUILD)/%.o: $(SRC)/%.cpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link ELF
$(TARGET).elf: $(OBJECTS)
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@

# Convert ELF -> DOL
$(TARGET).dol: $(TARGET).elf
	elf2dol $< $@

# Clean
clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).dol

# Flash/run on Wii (optional)
run: $(TARGET).dol
	wiiload $<
