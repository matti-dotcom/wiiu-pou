# Makefile for Nintendo Wii U (Pou.rpx)
# Requires devkitPPC, WUT, and wiiu-portlibs

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro")
endif

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

TARGET      := Pou
BUILD       := build
SOURCES     := src src/core src/entities src/rooms src/games src/ui
OUTPUT      := code/$(TARGET).rpx

# Compiler & Tools
PREFIX      := $(DEVKITPPC)/bin/powerpc-eabi-
CC          := $(PREFIX)gcc
CXX         := $(PREFIX)g++
AR          := $(PREFIX)ar
OBJCOPY     := $(PREFIX)objcopy
ELF2RPX     := elf2rpx

# Architecture Flags
ARCH        := -mcpu=750 -meabi -mhard-float

# Include directories
PORTLIBS    ?= $(DEVKITPRO)/portlibs/wiiu
WUT_ROOT    ?= $(DEVKITPRO)/wut

INCLUDES    := -Isrc \
               -I$(PORTLIBS)/include \
               -I$(PORTLIBS)/include/SDL2 \
               -I$(WUT_ROOT)/include

# Compiler Flags
CFLAGS      := $(ARCH) -O2 -Wall -D__WIIU__ -D__WUT__ $(INCLUDES)
CXXFLAGS    := $(CFLAGS) -std=gnu++17 -fno-rtti -fno-exceptions

# Linker Flags
LIBDIRS     := -L$(PORTLIBS)/lib -L$(WUT_ROOT)/lib
LIBS        := -lSDL2_mixer -lSDL2_image -lSDL2_ttf -lSDL2 -lpng -ljpeg -lfreetype -lz -lvorbisidec -logg -lwhb -lwut -lm

LDFLAGS     := $(ARCH) $(LIBDIRS) -specs=$(WUT_ROOT)/share/wut.specs $(LIBS)

# Collect Source Files
CPPFILES    := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp))
CFILES      := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
OFILES      := $(CPPFILES:%.cpp=$(BUILD)/%.o) $(CFILES:%.c=$(BUILD)/%.o)

.PHONY: all clean package

all: $(OUTPUT)

$(OUTPUT): $(BUILD)/$(TARGET).elf
	@mkdir -p code
	@echo "Creating RPX: $@"
	$(ELF2RPX) $< $@

$(BUILD)/$(TARGET).elf: $(OFILES)
	@mkdir -p $(dir $@)
	@echo "Linking ELF: $@"
	$(CXX) $^ $(LDFLAGS) -o $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling C++: $<"
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "Compiling C: $<"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD) $(OUTPUT)

package: all
	@echo "Packaging WUP title with NUSPacker..."
	@bash tools/package_wup.sh . Pou_WUP_Output
