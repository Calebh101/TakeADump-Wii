#-------------------------------
# Minimal libogc2 Wii Makefile
#-------------------------------

ifndef DEVKITPRO
$(error DEVKITPRO is not set. Please run in devkitPro environment)
endif

# Directories
SRC     := src
BUILD   := build
TARGET  := $(notdir $(CURDIR))

# Toolchain
CC      := powerpc-eabi-gcc
CXX     := powerpc-eabi-g++
LD      := $(CXX)

# Compiler flags
MACHDEP := -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float
CFLAGS  := -O2 -Wall $(MACHDEP) -I$(DEVKITPRO)/libogc2/include -I$(DEVKITPRO)/libogc2/include/ogc -I$(SRC)
CXXFLAGS:= $(CFLAGS)
LDFLAGS := -O2 $(MACHDEP) -L$(DEVKITPRO)/libogc2/lib/wii -lwiiuse -lbte -logc -lfat -lm

# Find sources
CPPFILES := $(wildcard $(SRC)/*.cpp)
OBJECTS  := $(patsubst $(SRC)/%.cpp,$(BUILD)/%.o,$(CPPFILES))

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
