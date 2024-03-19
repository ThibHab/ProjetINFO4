RACK_DIR ?= ../..


# FLAGS += -w
# Add .cpp and .c files to the build
# SOURCES += $(wildcard src/dep/*.cpp)
SOURCES += $(wildcard src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin is automatically added.
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard art/*.art)
DISTRIBUTABLES += $(wildcard art/svg/*)
DISTRIBUTABLES += $(wildcard font/*)
DISTRIBUTABLES += $(wildcard manual/*)

DISTRIBUTABLES += README.md
# DISTRIBUTABLES += $(wildcard src/shader/*.json)

include $(RACK_DIR)/plugin.mk

# ifdef ARCH_WIN
# # extra dist target for Azure CI Windows build, as there is only 7zip available and no zip command
# azure-win-dist: all
# 	mkdir -p dist/$(SLUG)
# 	@# Strip and copy plugin binary
# 	cp $(TARGET) dist/$(SLUG)/
# 	$(STRIP) -s dist/$(SLUG)/$(TARGET)
# 	@# Copy distributables
# 	cp -R $(DISTRIBUTABLES) dist/$(SLUG)/
# 	@# Create ZIP package
# 	cd dist && 7z a -tzip -mx=9 $(SLUG)-$(VERSION)-$(ARCH).zip -r $(SLUG)
# endif
