# Makefile for C clock application (port of main.py)
# Depends on: ../waveshare_epd (libwaveshare_epd.a)
# Platform: Raspberry Pi (Linux, ARM)
#
# Prerequisites:
#   cd ../waveshare_epd && make       # build library first
#   sudo apt install libfreetype6-dev
#
# Usage:
#   make          - build clock
#   make clean    - remove build artifacts
#   make run      - build and run

CC       = gcc
CFLAGS   = -Wall -Wextra -O2
FT_CFLAGS = $(shell pkg-config --cflags freetype2 2>/dev/null)
FT_LIBS   = $(shell pkg-config --libs   freetype2 2>/dev/null)

# Paths relative to this Makefile
LIB_DIR   = ../waveshare_epd
LIB_INC   = $(LIB_DIR)/include
LIB_FILE  = $(LIB_DIR)/lib/libwaveshare_epd.a

BUILD_DIR = build
TARGET    = $(BUILD_DIR)/epd_clock

.PHONY: all clean run clock

all: dirs $(TARGET)

clock: all

dirs:
	@mkdir -p $(BUILD_DIR)

# Ensure library is built
$(LIB_FILE):
	@echo "  Building library..."
	@$(MAKE) -C $(LIB_DIR)

$(TARGET): clock.c $(LIB_FILE)
	@echo "  CC    clock.c"
	@$(CC) $(CFLAGS) $(FT_CFLAGS) -I$(LIB_INC) clock.c -L$(LIB_DIR)/lib -lwaveshare_epd $(FT_LIBS) -lpthread -o $@
	@echo "  Build complete: $@"
	@echo "  Run: sudo ./$@"

clean:
	@echo "  CLEAN"
	@rm -rf $(BUILD_DIR)

run: $(TARGET)
	sudo ./$(TARGET)
