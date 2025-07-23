# ARM64 Operating System

A minimal ARM64 operating system for QEMU with an interactive shell and 17 built-in commands.

## Project Overview

This project is a **proof of concept of AI capabilities** - a collaborative effort between human research and AI implementation, with all code written by **Claude Code** (claude.ai/code).

## Quick Start

```bash
# Build and run
make
./run.sh

# Exit QEMU: Ctrl+A, X
```

Try these commands in the shell:
```bash
help           # List all commands
about          # OS information  
meminfo        # Memory statistics
calc 2 + 3     # Built-in calculator
```

## Commands (17 total)

**Basic:** help, echo, clear, about  
**Memory:** meminfo, peek, poke, dump  
**System:** reboot, color, sysinfo, uptime  
**Utilities:** calc, history, errors, stats, alias

## Documentation

See `docs/` for complete information:
- **QUICK-START.md** - Setup and tutorial
- **COMMAND-REFERENCE.md** - All commands with examples
- **ARCHITECTURE.md** - Technical details
- **KEYBINDINGS.md** - Interactive features
- **ACCOMPLISHMENTS.md** - Project achievements

---

*Demonstrates AI capabilities in systems programming - human-guided research with AI implementation.*


## 🚫 License: Limited Permission

You can clone and run this project, but you **cannot** reuse, republish, or modify it without permission.
