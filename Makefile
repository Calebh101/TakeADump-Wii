ifndef DEVKITPRO
$(error DEVKITPRO is not set. Please run in a devkitPro environment.)
endif

ifndef DEVKITPPC
$(error DEVKITPPC is not set. Please run in a devkitPro environment.)
endif

# Directories
SRC     := source
LIB     := lib
INC     := include
BUILD   := build
TARGET  := $(notdir $(CURDIR))
LIBOGC 	:= libogc

# Toolchain
CC      := powerpc-eabi-gcc
CXX     := powerpc-eabi-g++
LD      := $(CXX)

# Compiler flags
MACHDEP := -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float
CFLAGS  := -O2 -Wall -w $(MACHDEP) -I$(SRC) -I$(INC)/takeadump -I$(INC)/$(LIBOGC) -I$(INC)/$(LIBOGC)/ogc/machine
CXXFLAGS:= $(CFLAGS)
LDFLAGS := -O2 $(MACHDEP) -L$(LIB)/$(LIBOGC) -lwiiuse -lbte -logc -lfat -lm

# Find sources
CPPFILES := $(wildcard $(SRC)/*.cpp)
OBJECTS  := $(patsubst $(SRC)/%.cpp,$(BUILD)/%.o,$(CPPFILES))
PORTLIBS_PATH := $(DEVKITPRO)/portlibs

ifeq ($(DEBUG),1)
    CXXFLAGS += -DDEBUG
    $(info Debug mode is enabled)
endif

ifeq ($(EXCEPTIONS),1)
    CXXFLAGS += -fexceptions
    $(info Exceptions are enabled)
endif

export PATH := $(PORTLIBS_PATH)/wii/bin:$(PORTLIBS_PATH)/ppc/bin:$(DEVKITPRO)/tools/bin:$(DEVKITPPC)/bin:$(PATH)

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
