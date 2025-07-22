# Simplified ARM64 OS Plan - Minimal Shell in 2-3 Weeks

## Overview
**Duration**: 2-3 weeks  
**Goal**: Fastest path to a working shell on ARM64 QEMU

## Core Simplifications
- **No virtual memory** - Run with MMU disabled initially
- **No filesystem** - Commands built into kernel
- **No user space** - Shell runs in kernel mode
- **No interrupts** - Use polling for I/O
- **No process management** - Single execution flow
- **Minimal features** - Just enough to type commands

## Week 1: Boot to Hello World

### Day 1: Setup
#### Morning
- Install toolchain: `brew tap messense/macos-cross-toolchains && brew install aarch64-unknown-linux-gnu`
- Install QEMU: `brew install qemu`
- Create basic project structure (boot/, Makefile, run.sh)

#### Afternoon
- Write minimal Makefile
- Create QEMU run script
- Test toolchain works

### Day 2-3: Minimal Boot
#### Tasks
- Create boot.S with ARM64 header
- Initialize stack pointer
- Jump to C code
- Create boot.ld linker script
- Write main() that loops forever
- Verify it boots without crashing

### Day 4-5: Serial Output
#### Tasks
- Initialize PL011 UART at 0x09000000
- Implement putchar() in C using polling
- Implement puts() for strings
- Print "Hello ARM64 OS!" 
- Add printf() with just %s, %x support

### Day 6-7: Basic Input
#### Tasks
- Implement getchar() using UART polling
- Create gets() for line input
- Echo characters as they're typed
- Handle backspace
- Test interactive input/output

## Week 2: Memory and Basic Shell

### Day 8-9: Simple Memory
#### Tasks
- Create super simple bump allocator
- Implement malloc() that just increments a pointer
- No free() needed initially
- Add memory bounds checking
- Test allocations work

### Day 10-11: Command Parser
#### Tasks
- Create command structure (name + function pointer)
- Parse space-separated commands
- Implement strcmp() for matching
- Create command table
- Add "help" command that lists commands

### Day 12-14: Basic Shell Loop
#### Tasks
- Create shell prompt
- Read line from user
- Parse into command + arguments
- Look up command in table
- Execute command function
- Loop forever

## Week 3: Essential Commands (Optional but recommended)

### Day 15-16: Memory Commands
#### Tasks
- `peek` - Read memory address
- `poke` - Write memory address  
- `dump` - Display memory region
- `meminfo` - Show allocation info

### Day 17-18: System Commands
#### Tasks
- `clear` - Clear screen (ANSI escape codes)
- `echo` - Print arguments
- `reboot` - Restart QEMU
- `uptime` - Show timer ticks

### Day 19-21: Polish
#### Tasks
- Add command history (up/down arrows)
- Improve error messages
- Add colors to prompt
- Create demo script
- Write basic documentation

## Minimal Implementation Structure

```
myos/
├── boot.S          # Boot code and UART driver
├── boot.ld         # Linker script
├── main.c          # Main kernel and shell
├── string.c        # Basic string functions  
├── memory.c        # Bump allocator
├── commands.c      # Built-in commands
├── Makefile        # Build system
└── run.sh          # QEMU launcher
```

## Critical Success Path

### Must Have (Week 1)
- [ ] Boots in QEMU
- [ ] Serial I/O working
- [ ] Can type and see echo

### Must Have (Week 2)  
- [ ] Command prompt appears
- [ ] Can type commands
- [ ] At least 3 commands work
- [ ] Shell loop doesn't crash

### Nice to Have (Week 3)
- [ ] 8-10 useful commands
- [ ] Command history
- [ ] Clean error handling
- [ ] Some visual polish

## Key Simplifications Made

1. **No MMU/Virtual Memory**
   - Run at physical addresses
   - No page tables needed
   - No memory protection

2. **No Processes**
   - Shell is part of kernel
   - Commands are function calls
   - No fork/exec needed

3. **No Filesystem**
   - Commands compiled into kernel
   - No file I/O needed
   - No VFS layer

4. **No Interrupts**
   - Poll UART for input
   - Simple delay loops
   - No context switching

5. **Minimal Error Handling**
   - Basic bounds checking
   - Simple error messages
   - No recovery needed

## Development Tips

### Start Simple
- Get each piece working before moving on
- Don't add features until basics work
- Test constantly

### Debugging
- Add printf debugging everywhere
- Use QEMU monitor for inspection
- Keep backup of working code

### Common Issues
- Stack alignment (keep 16-byte aligned)
- UART not initialized properly
- Forgetting instruction barriers
- String termination bugs

## Sample Commands to Implement

### Essential (Week 2)
- `help` - List all commands
- `echo` - Print arguments
- `clear` - Clear screen

### Useful (Week 3)
- `peek <addr>` - Read memory
- `poke <addr> <val>` - Write memory
- `dump <addr> <len>` - Display memory
- `meminfo` - Memory statistics
- `reboot` - Restart system
- `uptime` - System timer

### Fun (If time permits)
- `banner` - ASCII art
- `calc` - Simple calculator
- `snake` - Tiny snake game
- `edit` - Line editor

## Final Result

After 2-3 weeks, you'll have:
- OS that boots in QEMU
- Interactive shell prompt
- Can type commands and see results
- 5-10 working commands
- ~1000 lines of code total

This is a **real OS** - just extremely minimal. Perfect for learning and can be expanded later!

## Next Steps After Success

Once your minimal shell works, you can add:
1. Interrupts for better I/O
2. Multiple processes
3. Virtual memory
4. Real filesystem
5. User/kernel separation

But these can wait - get the shell working first!