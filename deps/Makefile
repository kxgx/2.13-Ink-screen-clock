# Build FreeType locally for the clock application
# Run: make -C deps

FREETYPE_VER  = 2.13.3
FREETYPE_TAR  = freetype-$(FREETYPE_VER).tar.gz
FREETYPE_URL  = https://download.savannah.gnu.org/releases/freetype/$(FREETYPE_TAR)
FREETYPE_DIR  = freetype-$(FREETYPE_VER)
BUILD_DIR     = build/freetype
ABS_BUILD     = $(shell pwd)/$(BUILD_DIR)

.PHONY: all clean distclean

all: $(BUILD_DIR)/lib/libfreetype.a

# Download
$(FREETYPE_TAR):
	@echo "  DOWNLOAD $(FREETYPE_TAR)..."
	@wget -q $(FREETYPE_URL) || curl -sLO $(FREETYPE_URL)

# Extract
$(FREETYPE_DIR)/configure: $(FREETYPE_TAR)
	@echo "  EXTRACT $(FREETYPE_TAR)..."
	@tar xzf $(FREETYPE_TAR)
	@touch $(FREETYPE_DIR)/configure

# Build with autotools (static lib, no external deps)
$(BUILD_DIR)/lib/libfreetype.a: $(FREETYPE_DIR)/configure
	@echo "  BUILD  freetype..."
	@mkdir -p $(BUILD_DIR)
	@cd $(FREETYPE_DIR) && ./configure --prefix=$(ABS_BUILD) \
		--disable-shared --enable-static \
		--without-bzip2 --without-png --without-zlib \
		--without-harfbuzz --without-brotli \
		> /dev/null 2>&1
	@$(MAKE) -C $(FREETYPE_DIR) -j$$(nproc) > /dev/null 2>&1
	@$(MAKE) -C $(FREETYPE_DIR) install > /dev/null 2>&1
	@echo "  DONE   $(BUILD_DIR)/lib/libfreetype.a"

clean:
	rm -rf $(BUILD_DIR)

distclean: clean
	rm -rf $(FREETYPE_DIR) $(FREETYPE_TAR)
