# Simplified Phase 1: Boot to Hello World - Detailed Plan

## Overview
**Duration**: 1 week (7 days)  
**Goal**: Get a bootable system with serial I/O working

## Day 1: Development Environment Setup

### Morning Tasks (2-3 hours)
1. **Install Required Tools**
   - Install Homebrew if not already installed
   - Add the cross-compilation toolchain tap
   - Install ARM64 cross-compiler toolchain
   - Install QEMU for ARM64 emulation
   - Verify all tools are accessible from terminal

2. **Create Project Structure**
   - Create main project directory
   - Create subdirectory for boot code
   - Create directory for build outputs
   - Initialize version control

3. **Verify Tool Installation**
   - Check cross-compiler version and path
   - Verify QEMU ARM64 binary works
   - Test that tools can be invoked
   - Document tool versions for reference

### Afternoon Tasks (2-3 hours)
1. **Create Build System**
   - Write Makefile with variables for cross-compilation tools
   - Define source and build directories
   - Create rules for assemblin2g boot code
   - Add linking rules for final binary
   - Create clean target

2. **Create QEMU Run Script**
   - Write shell script to launch QEMU
   - Configure for ARM64 virt machine
   - Set up for serial console output (no graphics)
   - Add convenient command-line options
   - Make script executable

3. **Test Build Environment**
   - Create empty assembly file
   - Verify Makefile can process it
   - Test that QEMU script runs
   - Fix any path or permission issues

## Day 2-3: Minimal Boot Implementation

### Day 2 Tasks
1. **Study ARM64 Boot Requirements**
   - Research ARM64 boot header format
   - Understand required 64-byte header structure
   - Learn about exception levels (EL0-EL3)
   - Identify minimum initialization needed

2. **Create Boot Assembly File**
   - Implement ARM64 boot header with magic number
   - Add branch instruction to boot code
   - Fill required header fields (image size, etc.)
   - Create entry point label

3. **Basic CPU Initialization**
   - Check current exception level
   - Ensure running at EL1 (kernel level)
   - Disable MMU for simplicity
   - Disable caches (I-cache and D-cache)
   - Clear interrupt masks

4. **Stack Setup**
   - Define stack location in memory
   - Ensure 16-byte alignment (ARM64 requirement)
   - Set stack pointer register
   - Reserve adequate space (suggest 64KB)

### Day 3 Tasks
1. **Create Linker Script**
   - Define memory layout starting at 0x40000000 (QEMU default)
   - Place boot code at start
   - Define sections for code, data, and BSS
   - Export symbols for stack and BSS locations
   - Ensure proper alignment

2. **C Environment Preparation**
   - Create C entry point function
   - Clear BSS section (zero uninitialized data)
   - Set up function call from assembly to C
   - Create infinite loop to prevent crash

3. **Build and Test**
   - Compile boot assembly code
   - Link with linker script
   - Generate binary file
   - Test in QEMU (should boot without crashing)
   - Add debugging output to build process

## Day 4-5: Serial Output Implementation

### Day 4 Tasks
1. **Study PL011 UART Hardware**
   - Learn UART register layout
   - Understand QEMU virt UART location (0x09000000)
   - Study initialization sequence
   - Document control and data registers

2. **UART Initialization**
   - Define UART base address
   - Create initialization sequence
   - Disable UART during configuration
   - Set baud rate registers
   - Configure for 8 data bits, no parity, 1 stop bit
   - Enable transmitter

3. **Character Output Function**
   - Implement putchar functionality
   - Check transmit FIFO status
   - Wait if FIFO is full (polling)
   - Write character to data register
   - Handle special characters (newline)

### Day 5 Tasks
1. **String Output Functions**
   - Implement puts for string output
   - Loop through string until null terminator
   - Call putchar for each character
   - Add automatic newline

2. **Basic Printf Implementation**
   - Create simple printf with format string parsing
   - Support %s for strings
   - Support %x for hexadecimal
   - Handle basic formatting
   - Use variable arguments

3. **Test Messages**
   - Print boot banner
   - Display "Hello ARM64 OS!" message
   - Show initialization progress
   - Verify all output functions work

## Day 6-7: Basic Input and Polish

### Day 6 Tasks
1. **UART Input Configuration**
   - Enable UART receiver
   - Configure receive FIFO
   - Set up control registers
   - Test receive functionality

2. **Character Input Function**
   - Implement getchar using polling
   - Check receive FIFO status
   - Wait if FIFO is empty
   - Read character from data register
   - Return character value

3. **Line Input Function**
   - Implement gets for line input
   - Read characters until newline
   - Store in provided buffer
   - Add null terminator
   - Handle buffer size limits

4. **Echo Implementation**
   - Echo characters as typed
   - Handle special keys (backspace)
   - Update screen appropriately
   - Provide visual feedback

### Day 7 Tasks
1. **Input/Output Testing**
   - Test character echo
   - Verify line input works
   - Test backspace handling
   - Check special character handling

2. **Code Cleanup**
   - Organize code structure
   - Add comments and documentation
   - Remove debugging code
   - Ensure consistent style

3. **Build System Polish**
   - Add debug symbols option
   - Create release build option
   - Generate listing files
   - Add size reporting

4. **Prepare for Next Phase**
   - Document current functionality
   - List successful features
   - Note any limitations
   - Plan memory allocator design

## Deliverables Checklist

### Must Complete
- [ ] Boots successfully in QEMU every time
- [ ] Shows "Hello ARM64 OS!" on screen
- [ ] Can type characters and see them echoed
- [ ] Can input entire lines of text
- [ ] No crashes or hangs

### Should Complete
- [ ] Clean code organization
- [ ] Basic printf with %s and %x
- [ ] Backspace handling
- [ ] Build completes without warnings

### Nice to Have
- [ ] Boot timing information
- [ ] Memory size detection
- [ ] Different build configurations
- [ ] Comprehensive documentation

## Common Issues and Solutions

1. **Boot Failures**
   - Verify boot header is correct
   - Check linker script addresses
   - Ensure stack is aligned
   - Add early serial output for debugging

2. **UART Not Working**
   - Double-check UART base address
   - Verify initialization sequence
   - Check QEMU serial configuration
   - Try simpler test first

3. **Input Not Echoing**
   - Verify receiver is enabled
   - Check polling logic
   - Test with single character first
   - Add debug output

4. **Stack Issues**
   - Always maintain 16-byte alignment
   - Don't exceed allocated size
   - Check stack pointer setup
   - Monitor stack usage

## Success Criteria

By end of Phase 1:
- System boots reliably
- Can output text to console
- Can receive keyboard input
- Echo works properly
- Ready for shell development

## Next Phase Preview

Phase 2 will add:
- Simple memory allocator
- Command parsing
- Basic shell loop
- First working commands