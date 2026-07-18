# Makefile for C clock application (port of main.py)
# Platform: Raspberry Pi (Linux, ARM)
# Zero external dependencies (uses stb_truetype.h)
#
# Usage:
#   make          - build clock
#   make clean    - remove build artifacts
#   make run      - build and run

CC       = gcc
CFLAGS   = -Wall -Wextra -O2

# ---- waveshare_epd library ----
LIB_DIR   = ../waveshare_epd
LIB_INC   = $(LIB_DIR)/include
LIB_FILE  = $(LIB_DIR)/lib/libwaveshare_epd.a

BUILD_DIR = build
TARGET    = $(BUILD_DIR)/epd_clock

SRCS      = clock.c lunar.c
OBJS      = $(SRCS:%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean run clock

all: dirs $(TARGET)

clock: all

dirs:
	@mkdir -p $(BUILD_DIR)

$(LIB_FILE):
	@echo "  Building waveshare_epd library..."
	@$(MAKE) -C $(LIB_DIR)

$(TARGET): $(OBJS) stb_truetype.h $(LIB_FILE)
	@echo "  LD    $@"
	@$(CC) $(CFLAGS) -o $@ $(OBJS) \
		-L$(LIB_DIR)/lib -lwaveshare_epd -lm
	@echo "  Build complete: $@"
	@echo "  Run: sudo ./$@"

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) -I$(LIB_INC) -c $< -o $@

clean:
	@echo "  CLEAN"
	rm -rf $(BUILD_DIR) *.o gen_lunar.py lunar_data.txt

run: $(TARGET)
	sudo ./$(TARGET)
