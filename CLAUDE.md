# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a minimal ARM64 operating system project designed to run on QEMU. The project follows a simplified architecture with no virtual memory, filesystem, user space, interrupts, or process management - just enough to boot and provide an interactive shell foundation.

**Current Status**: Phase 2 Complete - Memory allocation and interactive shell fully implemented with 7 working commands
**Next Phase**: Phase 3 available for development - Additional shell commands and polish

## Development Workflow and Philosophy

This project follows a **Conservative, Conscientious, and Fearful of Failure** approach:

- **Be Conservative**: Implement only what is explicitly requested, avoid assumptions
- **Be Conscientious**: Include the human in every significant decision and milestone  
- **Be Fearful of Failure**: Get approval before proceeding, use review and confirmation processes

### Task Management Process

1. **Always create CURRENT-TASK.md** for active work tracking
2. **Get human approval** before moving between phases/major steps
3. **Include human test checkpoints** - specify what should be tested and verified
4. **Never proceed without explicit permission** to the next step or phase
5. **Reference context/simplified-phase-X-boot.md** outlines when implementing features
6. **Update CURRENT-TASK.md continuously** with progress (✅ complete, ⚠️ in progress)

## Build Commands

**Primary Commands:**
```bash
make            # Build kernel image (default)
make clean      # Remove all build artifacts
./run.sh        # Run kernel in QEMU (Ctrl+A, X to quit)
```

**Additional Commands:**
```bash
make debug      # Build with debug symbols
make size       # Show kernel size information  
make listing    # Generate disassembly listing
make help       # Show all available targets
```

## Architecture and Technical Details

### Cross-Compilation Setup
- **Toolchain**: `aarch64-elf-gcc` (installed via Homebrew)
- **Target**: ARM64 Cortex-A57 on QEMU virt machine
- **Load Address**: 0x40000000 (QEMU virt machine default)
- **Compiler Flags**: `-ffreestanding -nostdinc -nostdlib -nostartfiles` for freestanding environment
- **Build Artifacts**: Organized by type in `build/boot/`, `build/src/`
- **Final Outputs**: `kernel.elf` (linked binary), `kernel.img` (raw binary for QEMU)

### Boot Process
1. **boot/boot.S**: ARM64 boot header + CPU initialization (EL1 setup, MMU/cache disable)
2. **boot/boot.ld**: Linker script defining memory layout starting at 0x40000000
3. **src/main.c**: C entry point with UART initialization and interactive test

### Hardware Abstraction
- **PL011 UART**: Located at 0x09000000 on QEMU virt machine
- **UART Configuration**: 115200 baud, 8N1, FIFO enabled, polling-based I/O
- **Serial I/O**: Full bidirectional support with polling (getchar, putchar, gets, puts, printf)
- **Memory Layout**: Code starts at 0x40000000, heap after kernel `_end` symbol, 64KB stack
- **Memory Management**: Bump allocator implemented with 1MB heap, 16-byte alignment
- **Memory Utilities**: memset, memcpy, memmove, strdup functions available

### Project Structure
```
src/            # C source files
include/        # Header files  
boot/           # ARM64 boot code and linker script
build/          # All build artifacts (organized by source type)
context/        # Project documentation and phase plans
```

### Key Constraints
- **No standard library**: Uses `ffreestanding`, `nostdlib`, `nostartfiles`
- **No virtual memory**: MMU disabled, runs at physical addresses
- **No interrupts**: Uses polling for all I/O operations
- **Single execution flow**: No processes or context switching

## Phase Development Plan

The project is structured in phases documented in `context/simplified-phase-X-*.md`:

- **Phase 1** ✅: Boot to Hello World with serial I/O (COMPLETED)
- **Phase 2** ⚠️: Memory allocation and basic shell loop (IN PROGRESS - Memory complete, shell pending)
- **Phase 3**: Essential shell commands and polish

When implementing new phases, always reference the corresponding outline document and follow the conservative development approach outlined in `context/INSTRUCTIONS.md`.

## Current Implementation Status

### Completed Components
- **Boot System**: ARM64 boot loader, CPU initialization, serial console
- **UART Driver** (`src/uart.c`, `include/uart.h`): Full I/O with polling
- **Memory Allocator** (`src/memory.c`, `include/memory.h`): Bump allocator with statistics
- **Shell System** (`src/shell.c`, `include/shell.h`): Interactive command interpreter with 7 built-in commands
- **String Utilities** (`src/string.c`, `include/string.h`): Freestanding string functions

### Memory System Details
- **Heap Location**: Starts after kernel `_end` symbol from linker script
- **Heap Size**: 1MB with bounds checking and allocation tracking
- **Alignment**: 16-byte aligned allocations for ARM64 requirements  
- **Statistics**: Tracks allocations, usage, remaining space via `memory_info()`
- **Utilities**: Standard memory functions without stdlib dependency

### Shell System Architecture
- **Command Processing**: Function pointer table with argc/argv style arguments
- **Tokenization**: Space-delimited parsing with buffer overflow protection
- **Command Set**: 7 built-in commands (help, echo, clear, meminfo, about, uptime, calc)
- **Input Handling**: Line editing with backspace support, 256-byte command buffer
- **Error System**: Helpful error messages with usage hints and suggestions
- **Interactive Loop**: Persistent "OS> " prompt with continuous command execution

### Key APIs Available
```c
// Memory allocation
void* malloc(size_t size);
void memory_info(void);

// Memory utilities  
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t size);
void* memmove(void* dest, const void* src, size_t size);
char* strdup(const char* str);

// UART I/O
void putchar(char c);
void puts(const char* str);
void printf(const char* format, ...);  // Supports %s, %x
char getchar(void);
void gets(char* buffer, int max_size);

// Shell system
int shell_read_line(char* buffer, int max_size);
int shell_parse_and_execute(const char* input);

// String utilities (freestanding implementations)
size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
```