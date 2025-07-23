# ARM64 OS Command Reference Manual

**Version**: Phase 3 Complete  
**Date**: Day 21 - Final Documentation  
**Total Commands**: 17

This comprehensive reference covers all 17 commands available in the ARM64 OS interactive shell.

## Quick Command Index

### Basic Commands
- [`help`](#help) - Display command help and usage information
- [`echo`](#echo) - Display text to console  
- [`clear`](#clear) - Clear screen with enhanced ANSI sequences
- [`about`](#about) - Display OS information and credits

### Memory Management Commands
- [`meminfo`](#meminfo) - Detailed heap statistics and memory map
- [`peek`](#peek) - Read memory at address (hex/decimal)
- [`poke`](#poke) - Write memory (byte/word/long)
- [`dump`](#dump) - Hex dump with ASCII representation

### System Control Commands
- [`reboot`](#reboot) - System restart with confirmation
- [`color`](#color) - Toggle color output
- [`sysinfo`](#sysinfo) - Comprehensive system information  
- [`uptime`](#uptime) - System uptime and status

### Utility Commands
- [`calc`](#calc) - Basic calculator (+, -, *, /, %)
- [`history`](#history) - Display command history
- [`errors`](#errors) - Show error log
- [`stats`](#stats) - Performance monitoring statistics
- [`alias`](#alias) - Command aliases

---

## Detailed Command Reference

### `help`
**Purpose**: Display command help and usage information  
**Syntax**: `help [command]`

**Examples**:
```
help                    # Show all available commands
help echo              # Show detailed help for echo command
help peek              # Show memory peek command help
```

**Options**:
- No arguments: Lists all 17 commands with brief descriptions
- With command name: Shows detailed usage and examples for specific command

**Error Conditions**:
- Unknown command name: Returns error message with suggestion

---

### `echo`
**Purpose**: Display text to console  
**Syntax**: `echo [text...]`

**Examples**:
```
echo Hello World       # Output: Hello World
echo "Multiple words"  # Output: Multiple words  
echo                   # Output: (blank line)
```

**Features**:
- Supports multiple arguments
- Handles quoted strings
- Automatic space separation between arguments

---

### `clear`
**Purpose**: Clear screen with enhanced ANSI sequences  
**Syntax**: `clear [mode]`  

**Examples**:
```
clear                  # Clear entire screen and reset cursor
clear screen          # Same as clear
clear line            # Clear current line only
clear eol             # Clear from cursor to end of line
```

**Modes**:
- `screen` (default): Full screen clear with cursor reset
- `line`: Current line clear
- `eol`: Clear to end of line

---

### `about` 
**Purpose**: Display OS information and credits  
**Syntax**: `about`

**Output**: Shows OS version, phase completion status, feature summary, and development credits.

---

### `meminfo`
**Purpose**: Detailed heap statistics and memory map  
**Syntax**: `meminfo`

**Information Displayed**:
- Heap start address and total size
- Bytes allocated and available
- Allocation efficiency percentage
- Memory fragmentation analysis
- Stack and kernel memory estimates
- Complete memory layout with address ranges

---

### `peek`
**Purpose**: Read memory at address (hex/decimal)  
**Syntax**: `peek <address>`

**Examples**:
```
peek 0x40094000       # Read from hex address
peek 4096             # Read from decimal address  
peek 0x1000          # 4KB boundary address
```

**Address Formats**:
- Hexadecimal: `0x40000000`, `0x1000`
- Decimal: `4096`, `1048576`

**Safety Features**:
- Address validation against safe memory ranges
- Protection from hardware register access
- Clear error messages for invalid addresses

**Output Format**:
- Address and value in hexadecimal
- Decimal equivalent
- ASCII representation if printable

---

### `poke`
**Purpose**: Write memory (byte/word/long)  
**Syntax**: `poke <address> <value> [size]`

**Examples**:
```
poke 0x1000 0x12345678     # Write 32-bit value (default)
poke 0x1000 255 byte       # Write 8-bit value  
poke 0x1000 65535 word     # Write 16-bit value
poke 0x1000 0x1234 word    # Write hex 16-bit value
```

**Size Options**:
- `byte`: 8-bit write (0-255)
- `word`: 16-bit write (0-65535)
- `long`: 32-bit write (default)

**Safety Features**:
- Write address validation (more restrictive than peek)
- Alignment requirement enforcement
- Value range checking per size
- Write verification with readback confirmation

---

### `dump`
**Purpose**: Hex dump with ASCII representation  
**Syntax**: `dump <address> <length>`

**Examples**:
```
dump 0x40094000 64        # Dump 64 bytes from hex address
dump 1073811456 128       # Dump 128 bytes from decimal address
dump 0x1000 32           # Dump 32 bytes, small region
```

**Output Format**:
- 16 bytes per line in hex format
- Address column showing line start address
- ASCII representation column (printable chars only)
- Clean alignment and spacing

**Limits**:
- Maximum dump size: 1024 bytes
- Automatic 16-byte boundary alignment
- Safe memory range validation

---

### `reboot`
**Purpose**: System restart with confirmation  
**Syntax**: `reboot`

**Process**:
1. Displays confirmation prompt with system cleanup message
2. Requires 'y' or 'Y' confirmation to proceed
3. 'n', 'N', or any other key cancels operation
4. On confirmation: displays shutdown message and restarts system

**Safety**: Always asks for confirmation to prevent accidental reboots

---

### `color`
**Purpose**: Toggle color output  
**Syntax**: `color [on|off]`

**Examples**:
```
color                     # Show current color status
color on                 # Enable colored output
color off                # Disable colored output  
```

**Features**:
- Affects prompt colors (blue "OS>" when enabled)
- Controls error/success message coloring
- Maintains setting across commands

---

### `sysinfo`
**Purpose**: Comprehensive system information  
**Syntax**: `sysinfo`

**Information Categories**:
- **OS Details**: Name, version, phase completion status
- **Hardware**: CPU type (ARM Cortex-A57), execution level, MMU status
- **Memory**: Load address, heap configuration, stack size
- **Build**: Compiler, optimization level, build configuration
- **Boot Status**: Initialization status for all major subsystems

---

### `uptime`
**Purpose**: System uptime and status  
**Syntax**: `uptime`

**Displays**:
- Time since boot (simple counter-based)
- Boot completion status
- System initialization status
- Memory subsystem status

---

### `calc`
**Purpose**: Basic calculator (+, -, *, /, %)  
**Syntax**: `calc <expression>`

**Examples**:
```
calc 2 + 3               # Addition: Result = 5
calc 10 - 4              # Subtraction: Result = 6
calc 5 * 6               # Multiplication: Result = 30
calc 20 / 4              # Division: Result = 5
calc 100 % 7             # Modulo: Result = 2
```

**Supported Operations**:
- `+`: Addition
- `-`: Subtraction  
- `*`: Multiplication
- `/`: Division (integer)
- `%`: Modulo (remainder)

**Format**: `number operator number` (space-separated)

---

### `history`
**Purpose**: Display command history  
**Syntax**: `history`

**Features**:
- Shows last 20 commands in numbered list
- Circular buffer with automatic overflow handling
- Excludes duplicate consecutive commands
- Compatible with arrow key navigation

**Output Format**:
```
Command History (X entries):
  1: help
  2: echo Hello
  3: meminfo
  ...
```

---

### `errors`
**Purpose**: Show error log  
**Syntax**: `errors`

**Features**:
- Displays last 10 errors in circular buffer
- Color-coded error messages (when colors enabled)
- Shows error context and command that caused error
- Automatic error classification

**Output Format**: Chronological list with error types and descriptions

---

### `stats`
**Purpose**: Performance monitoring statistics  
**Syntax**: `stats`

**Displays**:
- Total commands executed
- Individual command execution counts
- Most frequently used commands
- System performance metrics
- Command usage patterns

**Note**: Performance timing temporarily disabled for system stability

---

### `alias`
**Purpose**: Command aliases  
**Syntax**: 
- `alias` - List all aliases
- `alias <name>` - Show specific alias  
- `alias <name> <command>` - Create/update alias

**Examples**:
```
alias                    # List all aliases
alias test               # Show 'test' alias definition
alias test "echo Works"  # Create alias: test -> echo Works
alias ll "dump"          # Create alias: ll -> dump
```

**Built-in Aliases**:
- `ll` → `dump` (memory listing)
- `cls` → `clear` (Windows-style clear)
- `mem` → `meminfo` (memory status)
- `h` → `help` (quick help)
- `?` → `help` (alternative help)

---

## Advanced Features

### Batch Commands
Execute multiple commands in sequence:

**Operators**:
- `;` - Sequential execution (always run next command)
- `&&` - Conditional execution (run next only if previous succeeded)
- `||` - Alternative execution (run next only if previous failed)

**Examples**:
```
echo First; echo Second                    # Run both commands
echo Success && echo "Previous worked"     # Second runs only if first succeeds
invalid_cmd || echo "Recovery worked"      # Second runs only if first fails
```

### Interactive Features
- **Arrow Keys**: Up/Down for history navigation, Left/Right for cursor movement
- **Tab Completion**: Auto-complete partial command names
- **Line Editing**: Insert/delete at cursor, Home/End keys, backspace/delete
- **Command History**: 20-command circular buffer with duplicate filtering

### Error Handling
- Comprehensive error messages with context
- Error logging and retrieval with `errors` command
- Color-coded error display
- Graceful handling of invalid input

---

## Error Codes and Troubleshooting

### Common Error Messages

**"Unknown command"**: 
- Check spelling with `help` to see available commands
- Use tab completion to verify command names

**"Invalid address format"**:
- Use hex format: `0x40000000` or decimal: `1073741824`
- Ensure address is within safe ranges

**"Address outside safe memory range"**:
- Use `meminfo` to see valid address ranges
- Stick to heap area (typically 0x40000000 and above)

**"Invalid argument count"**:
- Check command syntax with `help <command>`
- Verify required vs optional arguments

### Memory Safety
- `peek` allows reading from most kernel areas
- `poke` restricts writes to safe heap areas only
- `dump` validates entire requested range before output
- All memory commands prevent hardware register access

### Performance Notes
- Commands execute instantly (no noticeable delay)
- Memory operations include safety validation overhead
- Tab completion searches all 17 commands efficiently
- History navigation optimized for 20-entry buffer

---

## Command Categories Summary

| Category | Commands | Count |
|----------|----------|-------|
| Basic | help, echo, clear, about | 4 |
| Memory | meminfo, peek, poke, dump | 4 |
| System | reboot, color, sysinfo, uptime | 4 |
| Utility | calc, history, errors, stats, alias | 5 |
| **Total** | | **17** |

---

**End of Command Reference Manual**  
*For additional documentation, see: QUICK-START.md, KEYBINDINGS.md, ARCHITECTURE.md*