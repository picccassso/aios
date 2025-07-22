# ARM64 OS Makefile
# Cross-compilation toolchain
CROSS = aarch64-elf-
CC = $(CROSS)gcc
AS = $(CROSS)as
LD = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy

# Directories
SRCDIR = src
INCLUDEDIR = include
BOOTDIR = boot
BUILDDIR = build

# Compiler flags
CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -I$(INCLUDEDIR)
CFLAGS_DEBUG = $(CFLAGS) -g -DDEBUG
ASFLAGS = 
LDFLAGS = -nostdlib

# Source files
ASM_SOURCES = $(BOOTDIR)/boot.S
C_SOURCES = $(SRCDIR)/main.c $(SRCDIR)/uart.c $(SRCDIR)/memory.c $(SRCDIR)/string.c $(SRCDIR)/shell.c

# Object files (output to build subdirectories)
ASM_OBJECTS = $(ASM_SOURCES:$(BOOTDIR)/%.S=$(BUILDDIR)/boot/%.o)
C_OBJECTS = $(C_SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/src/%.o)
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

# Output
KERNEL = $(BUILDDIR)/kernel.elf
KERNEL_IMG = $(BUILDDIR)/kernel.img

# Default target
all: $(KERNEL_IMG)

# Build kernel image
$(KERNEL_IMG): $(KERNEL)
	$(OBJCOPY) -O binary $< $@
	@echo "Built kernel image: $@"

# Link kernel
$(KERNEL): $(OBJECTS)
	@mkdir -p $(BUILDDIR)
	$(LD) $(LDFLAGS) -T $(BOOTDIR)/boot.ld -o $@ $^
	@echo "Linked kernel: $@"

# Compile assembly files
$(BUILDDIR)/boot/%.o: $(BOOTDIR)/%.S
	@mkdir -p $(BUILDDIR)/boot
	$(AS) $(ASFLAGS) -o $@ $<

# Compile C files  
$(BUILDDIR)/src/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)/src
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean build files
clean:
	rm -rf $(BUILDDIR)
	@echo "Cleaned build files"

# Debug build
debug: CFLAGS := $(CFLAGS_DEBUG)
debug: $(KERNEL_IMG)

# Size reporting
size: $(KERNEL)
	@echo "=== Kernel Size Report ==="
	$(CROSS)size $<
	@echo "=== Binary Size ==="
	@ls -lh $(KERNEL_IMG)

# Generate listing
listing: $(KERNEL)
	$(CROSS)objdump -d $< > $(BUILDDIR)/kernel.lst
	@echo "Generated listing: $(BUILDDIR)/kernel.lst"

# Show help
help:
	@echo "ARM64 OS Build System"
	@echo "Targets:"
	@echo "  all     - Build kernel image (default)"
	@echo "  debug   - Build with debug symbols"
	@echo "  size    - Show kernel size information"
	@echo "  listing - Generate disassembly listing"
	@echo "  clean   - Remove build files"
	@echo "  help    - Show this help"

.PHONY: all debug size listing clean help