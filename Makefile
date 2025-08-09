# === Dirs ===
libDir := ~/.clibs
buildDir := bin
srcDir := src
incDir := $(srcDir)/inc
vosDir := $(incDir)/vos

# === Files ===
SRCS := $(shell find $(srcDir)/*.c)
VOSSRCS := $(shell find $(srcDir)/vos/*.c)

# === Tools ===
CC := gcc
CFLAGS := -g -w -I $(libDir) -I $(vosDir) -I $(incDir)

# === Rules and TARGET
TARGET := $(buildDir)/ps-webserver

all: $(TARGET)

$(TARGET): $(SRCS)
	@echo "Building..."
	@mkdir -p $(buildDir)
	$(CC) $(CFLAGS) $(SRCS) $(VOSSRCS) -o $@
	@echo "Done!"

clean:
	@echo "Cleaning..."
	@rm -f $(TARGET)
	@echo "Done!"

linux:
	@echo "Building..."
	@mkdir -p $(buildDir)
	$(CC) $(CFLAGS) $(SRCS) $(VOSSRCS) -o $@
	@echo "Done!"

windows:
	@echo "NOTE: this program is kinda unsupported in Windows but it should build, if it has any issues during runtime, know that it was never developed for Windows."
	@echo "Building..."
	@mkdir -p $(buildDir)
	$(CC) $(CFLAGS) $(SRCS) $(VOSSRCS) -o $@
	@echo "Done!"
