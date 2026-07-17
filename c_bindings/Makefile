# Makefile for Waveshare e-Paper C Library
# Target: libwaveshare_epd.a (static library)
# Platform: Raspberry Pi (Linux, ARM)
#
# Usage:
#   make              - build static library
#   make example      - build example program
#   make clean        - remove build artifacts

CC      = gcc
AR      = ar
CFLAGS  = -Wall -Wextra -O2 -Iinclude
FT_CFLAGS = $(shell pkg-config --cflags freetype2 2>/dev/null)
FT_LIBS   = $(shell pkg-config --libs   freetype2 2>/dev/null)
LDFLAGS =

# Source directories
HAL_DIR     = src/hal
DRIVERS_DIR = src/drivers

# Output
LIB_DIR  = lib
LIB_NAME = libwaveshare_epd.a

# Collect all source files
HAL_SRCS     = $(wildcard $(HAL_DIR)/*.c)
DRIVER_SRCS  = $(wildcard $(DRIVERS_DIR)/*.c)
ALL_SRCS     = $(HAL_SRCS) $(DRIVER_SRCS)

# Object files (mirror source tree in build/)
BUILD_DIR    = build
HAL_OBJS     = $(patsubst $(HAL_DIR)/%.c, $(BUILD_DIR)/hal/%.o, $(HAL_SRCS))
DRIVER_OBJS  = $(patsubst $(DRIVERS_DIR)/%.c, $(BUILD_DIR)/drivers/%.o, $(DRIVER_SRCS))
ALL_OBJS     = $(HAL_OBJS) $(DRIVER_OBJS)

# Build directories needed
BUILD_DIRS = $(sort $(dir $(ALL_OBJS)))

.PHONY: all clean example dirs

all: dirs $(LIB_DIR)/$(LIB_NAME)

# Create necessary directories
dirs:
	@mkdir -p $(BUILD_DIRS) $(LIB_DIR)

# Build static library
$(LIB_DIR)/$(LIB_NAME): $(ALL_OBJS)
	@echo "  AR    $@"
	@$(AR) rcs $@ $^

# Compile HAL sources
$(BUILD_DIR)/hal/%.o: $(HAL_DIR)/%.c
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile driver sources
$(BUILD_DIR)/drivers/%.o: $(DRIVERS_DIR)/%.c
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Example program
example: dirs $(LIB_DIR)/$(LIB_NAME) example/main.c
	@echo "  CC    example/main.c"
	@$(CC) $(CFLAGS) -Iinclude example/main.c -L$(LIB_DIR) -lwaveshare_epd $(LDFLAGS) -o $(BUILD_DIR)/epd_example
	@echo "  Build complete: $(BUILD_DIR)/epd_example"

# Clock application (C port of main.py with full/partial refresh)
# Requires: sudo apt install libfreetype6-dev
clock: dirs $(LIB_DIR)/$(LIB_NAME) example/clock.c
	@echo "  CC    example/clock.c"
	@$(CC) $(CFLAGS) $(FT_CFLAGS) -Iinclude example/clock.c -L$(LIB_DIR) -lwaveshare_epd $(FT_LIBS) -lpthread $(LDFLAGS) -o $(BUILD_DIR)/epd_clock
	@echo "  Build complete: $(BUILD_DIR)/epd_clock"
	@echo "  Run: sudo ./build/epd_clock"

# Clean
clean:
	@echo "  CLEAN"
	@rm -rf $(BUILD_DIR) $(LIB_DIR)

# Help
help:
	@echo "Waveshare e-Paper C Library"
	@echo ""
	@echo "Targets:"
	@echo "  make          Build static library (lib/$(LIB_NAME))"
	@echo "  make example  Build example program"
	@echo "  make clock    Build clock application (C port of main.py)"
	@echo "  make clean    Remove build artifacts"
	@echo "  make help     Show this help"
