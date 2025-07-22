# Simplified Phase 2: Memory and Basic Shell - Detailed Plan

## Overview
**Duration**: 1 week (7 days)  
**Goal**: Implement memory allocation and create a working command shell

## Day 8-9: Simple Memory Allocator

### Day 8 Tasks
1. **Design Bump Allocator**
   - Plan memory layout after kernel code/data
   - Define heap start and end addresses
   - Design allocation strategy (just increment pointer)
   - Plan alignment requirements

2. **Memory Constants Setup**
   - Define heap base address (after kernel end)
   - Set heap size limit (e.g., 1MB initially)
   - Create current allocation pointer
   - Define allocation alignment (8 or 16 bytes)

3. **Basic Malloc Implementation**
   - Create malloc function signature
   - Round requested size up to alignment
   - Check if enough space remains
   - Increment allocation pointer
   - Return pointer to allocated memory

4. **Bounds Checking**
   - Verify allocation won't exceed heap limit
   - Return NULL on allocation failure
   - Add total allocated counter
   - Track number of allocations

### Day 9 Tasks
1. **Memory Utilities**
   - Implement memset for zeroing memory
   - Create memcpy for data copying
   - Add memmove for overlapping copies
   - Test with various sizes

2. **Allocation Testing**
   - Test small allocations
   - Test large allocations
   - Verify alignment is maintained
   - Test allocation failure case

3. **Debug Features**
   - Create memory info function
   - Show total heap size
   - Display bytes allocated
   - Report number of allocations
   - Show remaining space

4. **String Allocation**
   - Create strdup using malloc
   - Implement string allocation helper
   - Test string operations
   - Verify no corruption

## Day 10-11: Command Parser

### Day 10 Tasks
1. **Command Structure Design**
   - Define command structure with name field
   - Add function pointer for command handler
   - Include help text field
   - Plan for argument passing

2. **String Functions**
   - Implement strcmp for command matching
   - Create strncmp for partial matching
   - Add strlen for length calculation
   - Implement strcpy for string copying

3. **Input Tokenization**
   - Design token parsing approach
   - Split input on spaces
   - Handle multiple spaces
   - Store tokens in array
   - Count number of tokens

4. **Command Table**
   - Create static array of commands
   - Define command handler function type
   - Populate with initial test command
   - Add table terminator

### Day 11 Tasks
1. **Parser Implementation**
   - Read line into buffer
   - Tokenize into command and arguments
   - Look up command in table
   - Handle command not found
   - Pass arguments to handler

2. **Argument Handling**
   - Design argument passing convention
   - Create argument count variable
   - Build argument array
   - Handle quoted arguments (optional)
   - Validate argument counts

3. **Help System**
   - Implement help command
   - List all available commands
   - Show command descriptions
   - Format output nicely
   - Add usage examples

4. **Error Handling**
   - Handle empty input
   - Detect buffer overflow
   - Report unknown commands
   - Show helpful error messages
   - Recover gracefully

## Day 12-14: Basic Shell Loop

### Day 12 Tasks
1. **Shell Prompt Design**
   - Create prompt string
   - Add color support (ANSI codes)
   - Include system name
   - Show current status
   - Make configurable

2. **Main Shell Loop**
   - Display prompt
   - Read line from user
   - Parse command
   - Execute if found
   - Handle errors
   - Loop forever

3. **Built-in Commands**
   - Implement echo command
   - Create clear screen command
   - Add exit/quit command (restart)
   - Implement help command
   - Test each thoroughly

4. **Shell State**
   - Track last command status
   - Store last command executed
   - Count commands run
   - Track shell uptime
   - Display in prompt (optional)

### Day 13 Tasks
1. **Enhanced Input Handling**
   - Improve line editing
   - Better backspace support
   - Handle arrow keys (skip for now)
   - Add input validation
   - Prevent buffer overflows

2. **Command Execution**
   - Improve command dispatcher
   - Add pre/post command hooks
   - Handle command interruption
   - Improve error reporting
   - Add command timing

3. **More Built-in Commands**
   - Add memory info command
   - Create system info command
   - Implement simple calculator
   - Add string manipulation commands
   - Test all commands

4. **Polish Shell Experience**
   - Add welcome banner
   - Improve error messages
   - Add command suggestions
   - Format output consistently
   - Create shell configuration

### Day 14 Tasks
1. **Integration Testing**
   - Test all commands work
   - Verify no memory leaks
   - Check error handling
   - Test edge cases
   - Stress test with long inputs

2. **Performance Testing**
   - Measure command execution time
   - Check memory usage
   - Monitor allocation patterns
   - Optimize hot paths
   - Remove debug code

3. **Documentation**
   - Document all commands
   - Create user guide
   - Write developer notes
   - Add inline comments
   - Create demo script

4. **Stabilization**
   - Fix any remaining bugs
   - Clean up code
   - Improve consistency
   - Final testing
   - Prepare for next phase

## Deliverables Checklist

### Must Complete
- [ ] Working memory allocator
- [ ] Command parser functional
- [ ] Shell prompt appears
- [ ] Can execute commands
- [ ] Help command works
- [ ] Echo command works

### Should Complete
- [ ] 5+ working commands
- [ ] Good error messages
- [ ] Memory info available
- [ ] Clean command output
- [ ] No crashes

### Nice to Have
- [ ] Colored prompt
- [ ] Command timing
- [ ] Advanced commands
- [ ] Configuration options
- [ ] Easter eggs

## Common Issues and Solutions

1. **Parser Problems**
   - Start with single-word commands
   - Add arguments incrementally
   - Handle edge cases carefully
   - Test with malformed input

2. **Memory Issues**
   - Track all allocations
   - Verify alignment maintained
   - Check bounds carefully
   - Monitor heap usage

3. **Shell Crashes**
   - Add defensive programming
   - Validate all inputs
   - Handle NULL pointers
   - Recover from errors

4. **Command Bugs**
   - Test each command thoroughly
   - Handle missing arguments
   - Validate parameters
   - Provide good feedback

## Success Criteria

By end of Phase 2:
- Shell prompt displays
- Can type commands
- Commands execute properly
- Help system works
- Memory allocation functional
- No crashes during normal use

## Commands to Implement

### Essential
- **help** - List all commands
- **echo** - Print arguments
- **clear** - Clear screen
- **meminfo** - Show memory usage

### Recommended
- **exit/quit** - Restart system
- **about** - Show OS information
- **uptime** - Display system uptime
- **calc** - Simple arithmetic

### Optional
- **peek** - Read memory address
- **poke** - Write memory address
- **dump** - Display memory region
- **test** - Run system tests

## Next Phase Preview

Phase 3 will add:
- More system commands
- Memory inspection tools
- System utilities
- Polish and optimization