/*
 * ARM64 OS Main Entry Point
 * Phase 1: Boot to Hello World with serial output
 */

#include "uart.h"
#include "memory.h"
#include "string.h"
#include "shell.h"

void main(void)
{
    // Initialize UART for serial output
    uart_init();
    
    // Initialize memory allocator for Phase 2
    memory_init();
    
    // Initialize shell command table
    shell_init();
    
    // Print boot banner
    puts("=== ARM64 OS Phase 1 ===");
    puts("Boot successful!");
    puts("");
    
    // Display "Hello ARM64 OS!" message as required
    puts("Hello ARM64 OS!");
    
    // Show initialization progress
    printf("UART initialized at %x\n", 0x09000000);
    printf("System ready - Phase %s complete\n", "1");
    puts("");
    
    // Phase 2 Day 13: Enhanced Interactive Shell with Welcome Banner
    puts("=== ARM64 OS Interactive Shell ===");
    puts("Phase 2 complete - Shell ready!");
    puts("");
    puts("Welcome to ARM64 OS!");
    puts("This is a minimal educational operating system");
    puts("Features: Memory management, interactive shell, 17 commands");
    puts("");
    puts("Available commands: help, echo, clear, meminfo, about, uptime, calc, peek, poke, dump, color, reboot, sysinfo, history, errors, stats, alias");
    puts("Type 'help' for detailed command information");
    puts("Type 'about' for system information");
    puts("");
    
    // Shell main loop
    char command_buffer[256];
    
    while (1) {
        // Display shell prompt with color support
        shell_display_prompt();
        
        // Read command line
        int len = shell_read_line(command_buffer, sizeof(command_buffer));
        
        
        // Skip empty input
        if (len <= 0) {
            continue;
        }
        
        // Execute command
        shell_parse_and_execute(command_buffer);
    }
}