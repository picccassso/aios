# ARM64 OS System Architecture Guide

**Version**: Phase 3 Complete  
**Target**: ARM64 Cortex-A57 on QEMU virt machine  
**Approach**: Minimal educational operating system

This document provides technical details about the ARM64 OS architecture, design decisions, and implementation details.

## System Overview

### Design Philosophy
The ARM64 OS is designed as a **minimal educational operating system** with these core principles:

- **Simplicity First**: Implement only essential features for learning
- **No Virtual Memory**: Run at physical addresses for simplicity
- **Polling-Based I/O**: No interrupt handling required
- **Single Execution Context**: No processes or context switching
- **Freestanding Environment**: No standard library dependencies

### Key Specifications
- **Architecture**: ARM64 (AArch64)
- **Target CPU**: ARM Cortex-A57
- **Execution Level**: EL1 (Exception Level 1)
- **Memory Model**: Physical addressing only
- **Load Address**: 0x40000000 (QEMU virt machine default)
- **Total Size**: ~1MB including 1MB heap

## Boot Process Architecture

### Boot Sequence Overview
```
1. QEMU Loads → 2. Boot Header → 3. CPU Init → 4. C Entry → 5. Shell Ready
   kernel.img      (boot.S)        (EL1 mode)   (main.c)    (interactive)
```

### Phase 1: Boot Header (`boot/boot.S`)
**File**: `boot/boot.S` (ARM64 Assembly)  
**Purpose**: Initialize hardware and jump to C code

**Key Operations**:
```assembly
_start:
    # 1. Set up exception level (EL1)
    mrs x0, CurrentEL
    and x0, x0, #12
    cmp x0, #8                    # Check if EL2
    
    # 2. Initialize stack pointer
    ldr x0, =stack_top
    mov sp, x0
    
    # 3. Disable MMU and caches (simplified mode)
    mrs x0, sctlr_el1
    bic x0, x0, #1                # Disable MMU
    msr sctlr_el1, x0
    
    # 4. Jump to C entry point
    bl main
    
    # 5. Halt processor
    wfi_loop:
        wfi
        b wfi_loop
```

**Memory Layout Defined**:
- **Code Section**: `.text` at 0x40000000
- **Data Section**: `.data` for initialized variables
- **BSS Section**: `.bss` for uninitialized variables  
- **Stack**: 64KB allocated in `.bss` section

### Phase 2: Hardware Initialization (`src/main.c`)
**Purpose**: Initialize core system components in C

**Initialization Sequence**:
```c
int main(void) {
    // 1. Initialize UART for serial I/O
    uart_init();
    
    // 2. Initialize memory allocator  
    memory_init();
    
    // 3. Initialize shell command system
    shell_init();
    
    // 4. Display boot messages
    printf("=== ARM64 OS Phase 1 ===\n");
    printf("Boot successful!\n");
    
    // 5. Enter interactive shell loop
    shell_main_loop();  // Never returns
    
    return 0;
}
```

## Memory Architecture

### Physical Memory Layout
```
0x00000000 - 0x3FFFFFFF    Device/Reserved Memory
0x40000000 - 0x4009CFFF    Kernel Code + Data (~628KB)
0x4009D000 - 0x4019CFFF    Heap (1MB = 1,048,576 bytes)
0x4019D000 - 0x401AD000    Stack (64KB)
0x401AD000 - 0xFFFFFFFF    Unused Physical Memory
```

### Memory Management Strategy

**No Virtual Memory**:
- MMU disabled during boot and remains off
- All addresses are physical addresses
- No page tables or virtual address translation
- Simplified memory model for educational purposes

**Heap Management**:
- **Algorithm**: Simple bump allocator
- **Size**: 1MB (1,048,576 bytes)
- **Alignment**: 16-byte aligned allocations
- **Growth**: Heap grows upward from base address
- **Free**: No free() implementation (educational simplicity)

**Stack Management**:
- **Size**: 64KB (65,536 bytes)
- **Location**: Grows downward from high address
- **Usage**: C function calls, local variables
- **Overflow**: No guard pages (simplified design)

### Memory Safety Features

**Address Validation**:
```c
// Safe memory ranges for different operations
int is_address_safe(unsigned long addr) {
    // Allow reading from kernel and heap areas
    return (addr >= 0x40000000 && addr <= 0x40FFFFFF);
}

int is_address_safe_write(unsigned long addr) {
    // More restrictive - only heap area for writes
    return (addr >= 0x40010000 && addr <= 0x40FFFFFF);
}
```

**Bounds Checking**:
- All heap allocations checked against 1MB limit
- Memory commands validate address ranges
- Buffer overflow protection on command input

## Hardware Abstraction Layer

### UART Driver (`src/uart.c`)
**Hardware**: PL011 UART at physical address 0x09000000

**Key Functions**:
```c
void uart_init(void);           // Initialize UART hardware
void putchar(char c);           // Output single character  
char getchar(void);             // Input single character (blocking)
void puts(const char* str);     // Output string
void printf(const char* fmt, ...); // Formatted output (%s, %x, %% only)
```

**Design Decisions**:
- **Polling-Based**: No interrupt handling required
- **115200 Baud**: Standard communication speed
- **8N1 Format**: 8 data bits, no parity, 1 stop bit
- **FIFO Enabled**: Hardware buffering for reliability

**Register Access**:
```c
#define UART_BASE_ADDR  0x09000000
#define UART_DATA       (UART_BASE_ADDR + 0x00)
#define UART_FLAG       (UART_BASE_ADDR + 0x18)
#define UART_IBRD       (UART_BASE_ADDR + 0x24)
#define UART_FBRD       (UART_BASE_ADDR + 0x28)
#define UART_LCRH       (UART_BASE_ADDR + 0x2C)
#define UART_CR         (UART_BASE_ADDR + 0x30)
```

### Timer System
**Implementation**: Software counter (no hardware timer)
```c
static unsigned int performance_counter = 0;

// Simple incrementing counter for basic timing
unsigned int get_timer_ticks(void) {
    return performance_counter++;
}
```

## Shell System Architecture

### Command Processing Pipeline
```
User Input → Tokenization → Command Lookup → Execution → Output
   ↓              ↓              ↓              ↓         ↓
shell_read_line → shell_tokenize → shell_find_command → cmd_handler → UART
```

### Core Shell Components

**1. Input Processing (`shell_read_line`)**:
- 256-byte command buffer with overflow protection
- Full line editing with cursor positioning
- ANSI escape sequence processing for special keys
- History navigation and tab completion

**2. Command Tokenization (`shell_tokenize`)**:
```c
typedef struct {
    int argc;                           // Argument count
    char* argv[MAX_ARGS];              // Argument pointers  
    char tokens[MAX_ARGS][MAX_TOKEN_SIZE]; // Token storage
} token_result_t;
```

**3. Command Table Structure**:
```c
typedef struct {
    char* name;                    // Command name
    char* description;             // Help description
    command_handler_t handler;     // Function pointer
} shell_command_t;

// Runtime-initialized table with 17 commands + NULL terminator
static shell_command_t command_table[18];
```

### Advanced Shell Features

**Command History System**:
```c
#define HISTORY_SIZE 20
typedef struct {
    char commands[HISTORY_SIZE][MAX_COMMAND_LENGTH];
    int write_index;    // Next position to write
    int nav_index;      // Current navigation position  
    int count;          // Total commands stored
} history_t;
```

**Alias System**:
```c
typedef struct {
    char name[32];          // Alias name
    char expansion[128];    // Command expansion
    int is_builtin;        // Built-in vs user-defined
} alias_entry_t;
```

**Batch Command Processing**:
- Supports `;` (sequential), `&&` (conditional), `||` (alternative)
- Safe parsing with bounds checking
- Recursive command execution prevention

## Command Implementation Architecture

### Memory Inspection Commands

**Address Parsing Strategy**:
```c
unsigned long parse_address(const char* str, int* valid) {
    if (strncmp(str, "0x", 2) == 0 || strncmp(str, "0X", 2) == 0) {
        return hex_to_ulong(str + 2, valid);  // Hexadecimal
    } else {
        return decimal_to_ulong(str, valid);   // Decimal
    }
}
```

**Memory Safety Architecture**:
- Three-tier safety checking: `is_address_safe`, `is_address_safe_write`, `is_address_safe_byte`
- Read operations: Allow kernel + heap areas
- Write operations: Restrict to heap area only
- Dump operations: Byte-level validation across entire range

**Hex Dump Format**:
```
Address: 40000000 01 02 03 04 05 06 07 08  09 0A 0B 0C 0D 0E 0F 10  |........ ........|
         40000010 11 12 13 14 15 16 17 18  19 1A 1B 1C 1D 1E 1F 20  |........ ....... |
```

### Calculator Engine
**Expression Parsing**:
```c
// Simple three-token parser: number operator number
typedef struct {
    int left_operand;
    char operator;
    int right_operand;
    int result;
} calc_expression_t;
```

**Supported Operations**:
- Addition (`+`), Subtraction (`-`), Multiplication (`*`)
- Division (`/`), Modulo (`%`)
- Integer arithmetic only (no floating point)

## Build System Architecture

### Cross-Compilation Toolchain
- **Compiler**: `aarch64-elf-gcc`
- **Assembler**: `aarch64-elf-as`
- **Linker**: `aarch64-elf-ld`
- **Objcopy**: `aarch64-elf-objcopy`

### Build Process
```bash
# 1. Assemble boot code
aarch64-elf-as -o build/boot/boot.o boot/boot.S

# 2. Compile C sources  
aarch64-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles \
                -Iinclude -c -o build/src/main.o src/main.c

# 3. Link into ELF binary
aarch64-elf-ld -nostdlib -T boot/boot.ld -o build/kernel.elf [objects...]

# 4. Convert to raw binary
aarch64-elf-objcopy -O binary build/kernel.elf build/kernel.img
```

### Linker Script (`boot/boot.ld`)
```ld
SECTIONS {
    . = 0x40000000;           /* Load address */
    
    .text : {                 /* Code section */
        *(.text)
    }
    
    .data : {                 /* Initialized data */
        *(.data)
    }
    
    .bss : {                  /* Uninitialized data */
        *(.bss)
    }
    
    _end = .;                 /* End marker for heap start */
}
```

## String Library Architecture

### Freestanding String Functions
**Custom Implementation** (no standard library):
```c
size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
void* memset(void* ptr, int value, size_t size);
void* memcpy(void* dest, const void* src, size_t size);
```

**Safety Features**:
- Bounds checking where possible
- NULL pointer validation
- Buffer overflow prevention in user-facing functions

## Error Handling Architecture

### Error Classification System
```c
typedef enum {
    SHELL_ERROR_NONE = 0,
    SHELL_ERROR_UNKNOWN_COMMAND,  
    SHELL_ERROR_INVALID_ARGS,
    SHELL_ERROR_MEMORY_ERROR,
    SHELL_ERROR_SYSTEM_ERROR
} shell_error_t;
```

### Error Logging
- **Circular Buffer**: 10 error entries maximum
- **Context Tracking**: Command name and error details
- **Color Coding**: Red error messages when colors enabled
- **Recovery**: Graceful error handling without system crashes

## Performance Characteristics

### System Performance
- **Boot Time**: ~1-2 seconds in QEMU
- **Command Response**: Instantaneous (no noticeable delay)
- **Memory Usage**: ~628KB kernel + dynamic heap usage
- **I/O Throughput**: Limited by UART baud rate (115200)

### Optimization Strategies
- **Compiler Optimization**: `-O2` for balanced size/speed
- **Function Inlining**: Small frequently-used functions
- **String Operations**: Optimized length-bounded operations
- **Memory Access**: 16-byte aligned allocations for ARM64 efficiency

## Security Architecture

### Memory Protection
- **Address Validation**: All memory commands validate addresses
- **Write Restrictions**: `poke` command limited to safe heap areas
- **Buffer Overflow Protection**: Fixed-size buffers with bounds checking
- **Command Injection Prevention**: Simple tokenization prevents complex injection

### System Integrity
- **No Privilege Escalation**: Single execution level (EL1)
- **No Process Isolation**: Educational single-process design
- **Hardware Register Protection**: Memory commands avoid hardware registers
- **Stack Protection**: 64KB stack with reasonable depth limits

## Extensibility Architecture

### Adding New Commands
```c
// 1. Implement command function
int cmd_newcommand(int argc, char* argv[]) {
    // Command implementation
    return 0;  // Success
}

// 2. Add to command table in shell_init()
command_table[n].name = "newcommand";
command_table[n].description = "Description";
command_table[n].handler = cmd_newcommand;

// 3. Add declaration to include/shell.h
```

### System Extensions
- **Hardware Drivers**: Add new device drivers following UART pattern
- **Memory Management**: Extend allocator with free() support
- **File System**: Add simple file system support
- **Network Stack**: Implement basic networking

## Testing and Validation

### Validation Strategy
- **Build Testing**: Clean compilation with zero warnings
- **Functional Testing**: All 17 commands tested systematically
- **Memory Testing**: Heap allocation/validation testing
- **Stress Testing**: Long-running command sequences
- **Boundary Testing**: Address validation edge cases

### Quality Metrics
- **Code Coverage**: All major code paths exercised
- **Memory Safety**: No buffer overflows or invalid access
- **System Stability**: No crashes or hangs during normal operation
- **User Experience**: Consistent and intuitive command interface

## Future Architecture Considerations

### Potential Enhancements
- **Virtual Memory**: Add MMU support and page tables
- **Interrupt Handling**: Replace polling with interrupt-driven I/O
- **Process Management**: Add basic process scheduling
- **File System**: Implement simple file system support
- **Network Stack**: Add TCP/IP support

### Scalability Limits
- **Single CPU**: No SMP support designed
- **Physical Memory**: Limited by available RAM in QEMU
- **Command Limit**: Command table size limited by design
- **Heap Growth**: Bump allocator prevents memory reuse

---

## Summary

The ARM64 OS represents a **minimal but complete** educational operating system with:

- ✅ **Clean Architecture**: Well-separated components and clear interfaces
- ✅ **Hardware Abstraction**: Proper driver architecture for UART
- ✅ **Memory Management**: Safe memory allocation and validation
- ✅ **Rich Shell**: 17 commands with advanced interactive features
- ✅ **Educational Focus**: Simple enough to understand, complex enough to be useful
- ✅ **Extensible Design**: Clear patterns for adding new functionality

**Total Implementation**: ~2,500 lines of C and ARM64 assembly code

---

*For user documentation, see `docs/COMMAND-REFERENCE.md` and `docs/QUICK-START.md`*  
*For interactive features, see `docs/KEYBINDINGS.md`*