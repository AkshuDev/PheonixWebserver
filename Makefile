# === Dirs ===
libDir := ~/.clibs
mongooseDir := $(libDir)/mongoose
buildDir := bin
srcDir := src

# === Files ===
SRCS := $(shell find $(srcDir)/*.c)

# === Tools ===
CC := gcc
CFLAGS := -w -I $(libDir) -I $(mongooseDir)

# === Rules and TARGET
TARGET := $(buildDir)/wserver-ggis

all: $(TARGET)

$(TARGET): $(SRCS)
	@echo "Building..."
	@mkdir -p $(buildDir)
	$(CC) $(CFLAGS) $(mongooseDir)/mongoose.c $(SRCS) -o $@
	@echo "Done!"

clean:
	@echo "Cleaning..."
	@rm -f $(TARGET)
	@echo "Done!"
