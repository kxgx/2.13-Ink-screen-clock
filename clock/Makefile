# Makefile for C clock application (port of main.py)
# Platform: Raspberry Pi (Linux, ARM)
#
# Usage:
#   make          - build clock (auto-builds deps if needed)
#   make clean    - remove build artifacts
#   make run      - build and run

CC       = gcc
CFLAGS   = -Wall -Wextra -O2

# ---- FreeType: use system if available, else build from deps/ ----
SYS_FT_CFLAGS := $(shell pkg-config --cflags freetype2 2>/dev/null)
SYS_FT_LIBS   := $(shell pkg-config --libs   freetype2 2>/dev/null)

DEPS_DIR    = ../deps
LOCAL_FT    = $(DEPS_DIR)/build/freetype
LOCAL_FT_A  = $(LOCAL_FT)/lib/libfreetype.a
LOCAL_FT_INC = $(LOCAL_FT)/include/freetype2

ifneq ($(SYS_FT_CFLAGS),)
  # System FreeType found
  FT_CFLAGS = $(SYS_FT_CFLAGS)
  FT_LIBS   = $(SYS_FT_LIBS)
  FT_DEPS   =
else
  # Build FreeType locally
  FT_CFLAGS = -I$(LOCAL_FT_INC)
  FT_LIBS   = -L$(LOCAL_FT)/lib -lfreetype
  FT_DEPS   = $(LOCAL_FT_A)
endif

# ---- waveshare_epd library ----
LIB_DIR   = ../waveshare_epd
LIB_INC   = $(LIB_DIR)/include
LIB_FILE  = $(LIB_DIR)/lib/libwaveshare_epd.a

BUILD_DIR = build
TARGET    = $(BUILD_DIR)/epd_clock

.PHONY: all clean run clock deps

all: dirs deps $(TARGET)

clock: all

dirs:
	@mkdir -p $(BUILD_DIR)

# Build local FreeType if needed
deps: $(FT_DEPS)

$(LOCAL_FT_A):
	@echo "  Building FreeType locally..."
	@$(MAKE) -C $(DEPS_DIR)

# Ensure waveshare_epd library is built
$(LIB_FILE):
	@echo "  Building waveshare_epd library..."
	@$(MAKE) -C $(LIB_DIR)

$(TARGET): clock.c $(LIB_FILE) $(FT_DEPS)
	@echo "  CC    clock.c"
	@$(CC) $(CFLAGS) $(FT_CFLAGS) -I$(LIB_INC) clock.c \
		-L$(LIB_DIR)/lib -lwaveshare_epd $(FT_LIBS) -lpthread -o $@
	@echo "  Build complete: $@"
	@echo "  Run: sudo ./$@"

clean:
	@echo "  CLEAN"
	@rm -rf $(BUILD_DIR)

run: $(TARGET)
	sudo ./$(TARGET)
