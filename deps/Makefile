# Build FreeType locally for the clock application
# Run: make -C deps

FREETYPE_VER  = 2.13.3
FREETYPE_TAR  = freetype-$(FREETYPE_VER).tar.gz
FREETYPE_URL  = https://download.savannah.gnu.org/releases/freetype/$(FREETYPE_TAR)
FREETYPE_DIR  = freetype-$(FREETYPE_VER)
BUILD_DIR     = build/freetype

.PHONY: all clean distclean

all: $(BUILD_DIR)/lib/libfreetype.a

# Download
$(FREETYPE_TAR):
	@echo "  DOWNLOAD $(FREETYPE_TAR)..."
	@wget -q $(FREETYPE_URL) || curl -sLO $(FREETYPE_URL)

# Extract
$(FREETYPE_DIR)/CMakeLists.txt: $(FREETYPE_TAR)
	@echo "  EXTRACT $(FREETYPE_TAR)..."
	@tar xzf $(FREETYPE_TAR)
	@touch $(FREETYPE_DIR)/CMakeLists.txt

# Build with CMake (static lib, no deps)
$(BUILD_DIR)/lib/libfreetype.a: $(FREETYPE_DIR)/CMakeLists.txt
	@echo "  BUILD  freetype..."
	@mkdir -p $(BUILD_DIR)
	@cd $(FREETYPE_DIR) && cmake -B ../$(BUILD_DIR) \
		-D CMAKE_BUILD_TYPE=Release \
		-D BUILD_SHARED_LIBS=OFF \
		-D FT_DISABLE_BZIP2=ON \
		-D FT_DISABLE_PNG=ON \
		-D FT_DISABLE_HARFBUZZ=ON \
		-D FT_DISABLE_BROTLI=ON \
		> /dev/null 2>&1
	@cmake --build $(BUILD_DIR) --target freetype -j$$(nproc) > /dev/null 2>&1
	@echo "  DONE   $(BUILD_DIR)/lib/libfreetype.a"

clean:
	rm -rf $(BUILD_DIR)

distclean: clean
	rm -rf $(FREETYPE_DIR) $(FREETYPE_TAR)
