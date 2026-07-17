# Makefile for Waveshare e-Paper C Library
# Target: libwaveshare_epd.a (static library)
# Platform: Raspberry Pi (Linux, ARM)
#
# Usage:
#   make          - build static library
#   make demo     - build demo program
#   make clean    - remove build artifacts

CC      = gcc
AR      = ar
CFLAGS  = -Wall -Wextra -O2 -Iinclude
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

.PHONY: all clean demo dirs

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

# Demo program (library usage example)
demo: dirs $(LIB_DIR)/$(LIB_NAME) app/demo.c
	@echo "  CC    app/demo.c"
	@$(CC) $(CFLAGS) -Iinclude app/demo.c -L$(LIB_DIR) -lwaveshare_epd $(LDFLAGS) -o $(BUILD_DIR)/epd_demo
	@echo "  Build complete: $(BUILD_DIR)/epd_demo"

# Clean
clean:
	@echo "  CLEAN"
	@rm -rf $(BUILD_DIR) $(LIB_DIR)

# Help
help:
	@echo "Waveshare e-Paper C Library"
	@echo ""
	@echo "Targets:"
	@echo "  make        Build static library (lib/$(LIB_NAME))"
	@echo "  make demo   Build demo program"
	@echo "  make clean  Remove build artifacts"
	@echo "  make help   Show this help"
