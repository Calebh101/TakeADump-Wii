#-------------------------------
# Minimal libogc2 Wii Makefile
#-------------------------------

ifndef DEVKITPRO
$(error DEVKITPRO is not set. Please run in devkitPro environment)
endif

# Directories
SRC     := source
LIB     := lib
INC     := include
BUILD   := build
TARGET  := $(notdir $(CURDIR))

# Toolchain
CC      := powerpc-eabi-gcc
CXX     := powerpc-eabi-g++
LD      := $(CXX)

# Compiler flags
MACHDEP := -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float
CFLAGS  := -O2 -Wall $(MACHDEP) -I$(SRC) -I$(INC)/libogc2 -I$(INC)/ppc-eabi
CXXFLAGS:= $(CFLAGS)
LDFLAGS := -O2 $(MACHDEP) -L$(LIB) -lwiiuse -lbte -logc -lfat -lm

# Find sources
CPPFILES := $(wildcard $(SRC)/*.cpp)
OBJECTS  := $(patsubst $(SRC)/%.cpp,$(BUILD)/%.o,$(CPPFILES))

ifeq ($(DEBUG),1)
    CXXFLAGS += -DDEBUG
endif

export PATH := $(PORTLIBS_PATH)/wii/bin:$(PORTLIBS_PATH)/ppc/bin:$(DEVKITPRO)/tools/bin:$(PATH)

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
