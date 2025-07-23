# ARM64 OS Quick Start Guide

**Version**: Phase 3 Complete  
**Difficulty**: Beginner  
**Time to Complete**: 10 minutes

Welcome to the ARM64 OS! This guide will get you up and running with the interactive shell in just a few minutes.

## Prerequisites

### Required Tools
- **QEMU**: ARM64 system emulator
- **Cross-compiler**: aarch64-elf-gcc toolchain
- **Terminal**: Any terminal with ANSI support

### macOS Installation
```bash
# Install QEMU
brew install qemu

# Install ARM64 cross-compiler
brew tap messense/macos-cross-toolchains
brew install aarch64-unknown-linux-gnu
```

## Quick Boot

### 1. Build the System
```bash
cd /path/to/aios
make clean && make
```

**Expected Output**:
```
Built kernel image: build/kernel.img
```

### 2. Start the OS
```bash
./run.sh
```

**Boot Sequence You'll See**:
```
Starting ARM64 OS in QEMU...
Use Ctrl+A, X to quit QEMU

Memory allocator initialized
Heap start: 0x4009d000
Heap size: 0x100000 bytes (1MB)
=== ARM64 OS Phase 1 ===
Boot successful!

Hello ARM64 OS!
UART initialized at 0x9000000
System ready - Phase 1 complete

=== ARM64 OS Interactive Shell ===
Phase 2 complete - Shell ready!

Welcome to ARM64 OS!
This is a minimal educational operating system
Features: Memory management, interactive shell, 17 commands

Available commands: help, echo, clear, meminfo, about, uptime, calc, peek, poke, dump, color, reboot, sysinfo, history, errors, stats, alias
Type 'help' for detailed command information
Type 'about' for system information

OS> 
```

### 3. Exit QEMU
Press `Ctrl+A`, then `X` to quit QEMU and return to your terminal.

## First Commands Tutorial

### Start with Help
```
OS> help
```
This shows all 17 available commands with brief descriptions.

### System Information
```
OS> about
```
Displays OS version, features, and credits.

```
OS> sysinfo
```
Shows comprehensive system information including hardware details.

### Memory Exploration
```
OS> meminfo
```
Displays detailed memory statistics and layout.

```
OS> peek 0x40000000
```
Read memory at the kernel load address.

```
OS> dump 0x40000000 64
```
Show a hex dump of the first 64 bytes of kernel memory.

### Interactive Features Test
```
OS> echo Hello ARM64 OS!
```
Basic text output.

```
OS> calc 15 + 27
```
Simple calculator test.

```
OS> color on
```
Enable colored output (if not already enabled).

```
OS> clear
```
Clear the screen.

## Essential Features Overview

### 17 Built-in Commands

**Basic Commands**:
- `help` - Command help and documentation
- `echo` - Text output  
- `clear` - Screen clearing
- `about` - OS information

**Memory Management**:
- `meminfo` - Memory statistics
- `peek` - Read memory addresses
- `poke` - Write to memory
- `dump` - Hex dump memory regions

**System Control**:
- `reboot` - System restart
- `color` - Toggle colored output
- `sysinfo` - System information
- `uptime` - Boot time and status

**Utilities**:
- `calc` - Basic calculator
- `history` - Command history
- `errors` - Error log display
- `stats` - Performance statistics  
- `alias` - Command aliases

### Advanced Shell Features

**Command History**:
- Use ↑/↓ arrow keys to navigate through last 20 commands
- Automatic duplicate filtering

**Line Editing**:
- ←/→ arrow keys for cursor movement
- Home/End keys for line start/end
- Backspace/Delete for character removal
- Insert characters at cursor position

**Tab Completion**:
- Press Tab to auto-complete command names
- Shows multiple matches if ambiguous

**Batch Commands**:
- `;` - Run multiple commands: `echo Hello; echo World`
- `&&` - Conditional execution: `echo Success && echo "Previous worked"`
- `||` - Alternative execution: `invalid_cmd || echo "Recovery"`

## Common Usage Patterns

### Memory Investigation
```bash
# Check system memory status
OS> meminfo

# Examine specific memory location
OS> peek 0x40094000

# Inspect kernel code area  
OS> dump 0x40000000 128

# Write test value and verify
OS> poke 0x40094000 0x12345678
OS> peek 0x40094000
```

### System Monitoring
```bash
# Check system information
OS> sysinfo
OS> uptime

# Monitor command usage
OS> stats
OS> history

# Check for any errors
OS> errors
```

### Customization
```bash
# Set up aliases for common commands
OS> alias mem "meminfo"
OS> alias ll "dump"

# Test your aliases
OS> mem
OS> ll 0x40000000 64

# Enable colors for better visibility
OS> color on
```

## Troubleshooting

### Common Issues

**"Command not found"**:
- Type `help` to see all available commands
- Use tab completion to verify command names
- Check for typos

**"QEMU won't start"**:
- Verify QEMU is installed: `qemu-system-aarch64 --version`
- Check that `./run.sh` has execute permissions
- Ensure `build/kernel.img` exists (run `make` first)

**"Build fails"**:
- Verify cross-compiler: `aarch64-elf-gcc --version`
- Run `make clean` then `make`
- Check that all source files are present

**"Memory access errors"**:
- Use `meminfo` to see safe address ranges
- Stick to heap area (typically 0x40000000+)
- Use hex format: `0x40000000` not just `40000000`

### Getting Help

**Within the OS**:
- `help` - List all commands
- `help <command>` - Detailed help for specific command
- `about` - OS version and feature information

**Documentation**:
- `docs/COMMAND-REFERENCE.md` - Complete command documentation  
- `docs/KEYBINDINGS.md` - Keyboard shortcuts
- `docs/ARCHITECTURE.md` - Technical details

## What's Next?

### Explore Advanced Features
1. **Try batch commands**: Combine multiple operations
2. **Use aliases**: Create shortcuts for common tasks
3. **Explore memory**: Use peek/poke/dump to understand the system
4. **Monitor performance**: Use stats and history commands

### Learn the Architecture
- Read `docs/ARCHITECTURE.md` for technical details
- Examine the source code in `src/` directory
- Understand the boot process in `boot/` directory

### Extend the System
- Add new commands (modify `src/shell.c`)
- Enhance existing features
- Study the memory management implementation
- Experiment with the UART driver

## Quick Reference Card

| Command | Purpose | Example |
|---------|---------|---------|
| `help` | Show commands | `help calc` |
| `echo` | Display text | `echo Hello` |
| `clear` | Clear screen | `clear` |
| `meminfo` | Memory stats | `meminfo` |
| `peek` | Read memory | `peek 0x40000000` |
| `dump` | Hex dump | `dump 0x1000 64` |
| `calc` | Calculator | `calc 10 + 5` |
| `history` | Command history | `history` |
| `alias` | Create aliases | `alias ll "dump"` |
| `reboot` | Restart system | `reboot` |

### Keyboard Shortcuts
- `↑/↓` - Command history
- `←/→` - Cursor movement  
- `Tab` - Command completion
- `Home/End` - Line start/end
- `Ctrl+A, X` - Quit QEMU

---

## Success!

You now have a working ARM64 operating system with:
- ✅ Interactive shell with 17 commands
- ✅ Memory management and inspection tools
- ✅ Advanced line editing and history
- ✅ Batch command processing
- ✅ Comprehensive help system

**Happy exploring!** 🚀

---

*For complete command documentation, see `docs/COMMAND-REFERENCE.md`*  
*For keyboard shortcuts, see `docs/KEYBINDINGS.md`*  
*For technical details, see `docs/ARCHITECTURE.md`*