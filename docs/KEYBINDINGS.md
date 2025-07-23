# ARM64 OS Key Bindings & Shortcuts

**Version**: Phase 3 Complete  
**Interactive Features**: Full line editing, history navigation, tab completion

This document covers all keyboard shortcuts and interactive features available in the ARM64 OS shell.

## Overview

The ARM64 OS shell provides a rich interactive experience with:
- **Command History**: Navigate through last 20 commands
- **Line Editing**: Full cursor control with insert/delete
- **Tab Completion**: Auto-complete command names  
- **Special Keys**: Home, End, Delete, and function keys
- **ANSI Support**: Full terminal escape sequence processing

## Navigation Keys

### Arrow Key Navigation

| Key | Function | Description |
|-----|----------|-------------|
| `↑` | Previous Command | Navigate backward through command history |
| `↓` | Next Command | Navigate forward through command history |
| `←` | Move Cursor Left | Move cursor one character to the left |
| `→` | Move Cursor Right | Move cursor one character to the right |

**Example Usage**:
```
OS> echo Hello          # Type a command
OS> meminfo             # Type another command  
OS> ↑                   # Shows: meminfo
OS> ↑                   # Shows: echo Hello
OS> ↓                   # Shows: meminfo
```

### Home and End Keys

| Key | Function | Description |
|-----|----------|-------------|
| `Home` | Move to Line Start | Jump cursor to beginning of command line |
| `End` | Move to Line End | Jump cursor to end of command line |

**ANSI Sequences Supported**:
- `Home`: `ESC[H`, `ESC[1~`
- `End`: `ESC[F`, `ESC[4~`

### Page Navigation Keys

| Key | Function | Status |
|-----|----------|--------|
| `Page Up` | Not implemented | Reserved for future use |
| `Page Down` | Not implemented | Reserved for future use |

## Editing Keys

### Character Manipulation

| Key | Function | Description |
|-----|----------|-------------|
| `Backspace` | Delete Previous Character | Remove character to the left of cursor |
| `Delete` | Delete Next Character | Remove character at cursor position |
| `Enter` | Execute Command | Submit current command line for execution |
| `Tab` | Command Completion | Auto-complete partial command names |

**Delete Key Support**:
- ANSI sequence: `ESC[3~`
- Deletes character at cursor position (forward delete)
- Automatically redraws line after deletion

### Advanced Editing

| Key Combination | Function | Description |
|-----------------|----------|-------------|
| `Ctrl+A` | Not implemented | Reserved (conflicts with QEMU) |
| `Ctrl+E` | Not implemented | Reserved for future use |
| `Ctrl+L` | Not implemented | Reserved for clear screen |

**Note**: Basic Ctrl combinations are not implemented to avoid conflicts with QEMU control sequences.

## Command History System

### History Navigation
- **Buffer Size**: 20 commands maximum
- **Storage**: Circular buffer with automatic overflow
- **Filtering**: Duplicate consecutive commands are not stored
- **Persistence**: History cleared on system restart

### History Behavior
```
Command Entry:           History State:
echo Hello          →    [echo Hello]
meminfo            →    [echo Hello, meminfo]
meminfo            →    [echo Hello, meminfo]         # Duplicate not stored
calc 1 + 1         →    [echo Hello, meminfo, calc 1 + 1]
```

### Navigation Details
- **Up Arrow**: Move backward through history (newer to older)
- **Down Arrow**: Move forward through history (older to newer)
- **End of History**: Stays at oldest/newest command
- **Line Replacement**: Selected history command completely replaces current line

## Tab Completion

### Command Completion
- **Trigger**: Press `Tab` key
- **Scope**: All 17 built-in commands
- **Behavior**: 
  - Single match: Auto-completes immediately
  - Multiple matches: Shows all possibilities
  - No matches: Beep sound (BEL character)

### Completion Examples

**Single Match**:
```
OS> cal<Tab>        # Auto-completes to: calc
```

**Multiple Matches**:
```
OS> c<Tab>          # Shows: calc, clear, color
Available commands starting with 'c':
calc    clear   color
```

**No Match**:
```
OS> xyz<Tab>        # Beep sound, no completion
```

### Smart Completion Features
- **Word Boundary Detection**: Only completes at word boundaries
- **Partial Matching**: Matches from the beginning of command names
- **Case Sensitive**: Exact case matching required
- **Multiple Display**: Shows up to 4 commands per line when listing matches

## Special Key Sequences

### ANSI Escape Sequence Processing

The shell processes standard ANSI escape sequences:

| Sequence | Key | Function |
|----------|-----|----------|
| `ESC[A` | Up Arrow | Previous command |
| `ESC[B` | Down Arrow | Next command |
| `ESC[C` | Right Arrow | Cursor right |
| `ESC[D` | Left Arrow | Cursor left |
| `ESC[H` | Home | Line start |
| `ESC[F` | End | Line end |
| `ESC[1~` | Home (alt) | Line start |
| `ESC[3~` | Delete | Forward delete |
| `ESC[4~` | End (alt) | Line end |

### Multi-Character Sequence Handling
The shell properly handles multi-character sequences by:
1. Detecting ESC character (`0x1B`)
2. Reading follow-up characters
3. Parsing complete sequence before acting
4. Handling timeouts and incomplete sequences

## Interactive Features

### Line Editing Capabilities

**Real-time Editing**:
- Insert characters at any cursor position
- Delete characters from any position  
- Visual cursor positioning
- Automatic line redraw after modifications

**Buffer Management**:
- **Maximum Line Length**: 256 characters
- **Overflow Handling**: Prevents buffer overflow
- **Character Encoding**: ASCII only
- **Line Termination**: Carriage return (`\r`) or newline (`\n`)

### Cursor Positioning

**Visual Feedback**:
- Cursor position always visible
- Real-time cursor movement
- Proper positioning after history navigation
- Accurate cursor tracking during editing

**Position Tracking**:
- Maintains separate cursor position and line length
- Handles cursor boundaries (cannot move beyond line start/end)
- Updates position after character insertion/deletion

## QEMU-Specific Keys

### System Control

| Key Combination | Function | Description |
|-----------------|----------|-------------|
| `Ctrl+A, X` | Quit QEMU | Exit the emulator (most important!) |
| `Ctrl+A, C` | QEMU Monitor | Access QEMU command console |
| `Ctrl+A, H` | Help | Show QEMU key combinations |

**Critical**: `Ctrl+A, X` is the standard way to exit QEMU. Always use this sequence to properly close the emulator.

### QEMU Monitor Access
```
(qemu) info registers      # Show CPU registers
(qemu) info memory         # Show memory mapping
(qemu) quit               # Alternative way to exit
```

## Troubleshooting Key Issues

### Common Problems

**Arrow keys not working**:
- Verify terminal supports ANSI escape sequences
- Check terminal type (TERM environment variable)
- Some terminals may use different escape sequences

**Tab completion not responding**:
- Ensure you're typing at the command prompt (`OS> `)
- Verify the partial command name is valid
- Check for extra spaces or characters

**History navigation issues**:
- History is cleared on each boot
- Only last 20 commands are stored
- Duplicate consecutive commands are filtered

**Backspace behavior**:
- Should delete character to the left of cursor
- Automatically redraws line
- Works at any cursor position

### Terminal Compatibility

**Fully Supported**:
- xterm
- QEMU console
- Most modern terminal emulators

**Limited Support**:
- Very old terminals (limited ANSI support)
- Terminals without escape sequence support

**Testing Keyboard Features**:
```bash
# Test basic editing
OS> echo test                    # Type normally
# Use backspace to delete 'test'
# Use arrow keys to navigate

# Test history
OS> help
OS> meminfo  
OS> ↑                           # Should show meminfo
OS> ↑                           # Should show help

# Test tab completion
OS> cal<Tab>                    # Should complete to calc
OS> c<Tab>                      # Should show: calc, clear, color
```

## Advanced Usage Tips

### Efficient Command Entry
1. **Use Tab Completion**: Faster than typing full command names
2. **Leverage History**: Use ↑ to repeat similar commands
3. **Edit in Place**: Use arrow keys to modify previous commands
4. **Home/End Navigation**: Quick movement to line boundaries

### Power User Shortcuts
```bash
# Quick command repetition with modification
OS> peek 0x40000000             # Original command
OS> ↑                           # Recall command
OS> Home                        # Jump to start  
OS> → → → → →                   # Move to address
OS> End                         # Jump to end
OS> Backspace Backspace 04     # Change to 0x40000004
```

### Batch Command Editing
```bash
# Use history and editing for complex batch commands
OS> echo First; echo Second; calc 1 + 1
OS> ↑                           # Recall batch command
# Edit individual parts using cursor movement
OS> → → → → → → → → → → →        # Navigate to specific section
# Modify as needed
```

## Summary

The ARM64 OS provides a fully-featured interactive shell with:
- ✅ **Full Line Editing**: Insert, delete, cursor movement
- ✅ **20-Command History**: With duplicate filtering
- ✅ **Tab Completion**: For all 17 commands
- ✅ **Arrow Key Navigation**: History and cursor control
- ✅ **Special Key Support**: Home, End, Delete
- ✅ **ANSI Compatibility**: Standard terminal sequences

**Most Important Keys to Remember**:
- `↑/↓` - Command history navigation
- `Tab` - Command completion  
- `Home/End` - Line start/end
- `Ctrl+A, X` - Exit QEMU

---

*For complete command documentation, see `docs/COMMAND-REFERENCE.md`*  
*For getting started, see `docs/QUICK-START.md`*  
*For technical details, see `docs/ARCHITECTURE.md`*