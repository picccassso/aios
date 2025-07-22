/*
 * Shell command processing implementation
 * Phase 2 Day 10: Command structure and tokenization
 */

#include "shell.h"
#include "string.h"
#include "uart.h"
#include "memory.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

// Command table - Phase 2 Day 13 complete (runtime initialized)
static shell_command_t command_table[8];  // 7 commands + NULL terminator

void shell_init(void)
{
    // Initialize command table at runtime
    command_table[0].name = "help";
    command_table[0].description = "Show available commands";
    command_table[0].handler = cmd_help;
    
    command_table[1].name = "echo";
    command_table[1].description = "Display text";
    command_table[1].handler = cmd_echo;
    
    command_table[2].name = "clear";
    command_table[2].description = "Clear screen";
    command_table[2].handler = cmd_clear;
    
    command_table[3].name = "meminfo";
    command_table[3].description = "Show memory information";
    command_table[3].handler = cmd_meminfo;
    
    command_table[4].name = "about";
    command_table[4].description = "Display OS information";
    command_table[4].handler = cmd_about;
    
    command_table[5].name = "uptime";
    command_table[5].description = "Show system uptime";
    command_table[5].handler = cmd_uptime;
    
    command_table[6].name = "calc";
    command_table[6].description = "Simple calculator";
    command_table[6].handler = cmd_calc;
    
    // Terminator
    command_table[7].name = NULL;
    command_table[7].description = NULL;
    command_table[7].handler = NULL;
}

int shell_read_line(char* buffer, int max_size)
{
    if (!buffer || max_size <= 0) return -1;
    
    int pos = 0;
    char c;
    
    while (pos < max_size - 1) {
        c = getchar();
        
        // Handle backspace
        if (c == '\b' || c == 0x7F) {
            if (pos > 0) {
                pos--;
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
            continue;
        }
        
        // Handle enter/newline
        if (c == '\r' || c == '\n') {
            putchar('\n');
            break;
        }
        
        // Handle printable characters
        if (c >= 0x20 && c <= 0x7E) {
            buffer[pos++] = c;
            putchar(c);  // Echo character
        }
    }
    
    buffer[pos] = '\0';
    return pos;
}

int shell_tokenize(const char* input, token_result_t* result)
{
    if (!input || !result) return -1;
    
    result->argc = 0;
    int i = 0;          // Input position
    int token_idx = 0;  // Current token index
    int char_idx = 0;   // Character position in current token
    
    // Skip leading whitespace
    while (input[i] == ' ' || input[i] == '\t' || input[i] == '\n') {
        i++;
    }
    
    while (input[i] != '\0' && token_idx < MAX_ARGS) {
        char_idx = 0;
        
        // Read characters until whitespace or end
        while (input[i] != '\0' && input[i] != ' ' && input[i] != '\t' && 
               input[i] != '\n' && char_idx < (MAX_TOKEN_SIZE - 1)) {
            result->tokens[token_idx][char_idx++] = input[i++];
        }
        
        // Null terminate the token
        result->tokens[token_idx][char_idx] = '\0';
        
        // Set argv pointer to this token
        result->argv[token_idx] = result->tokens[token_idx];
        token_idx++;
        
        // Skip whitespace before next token
        while (input[i] == ' ' || input[i] == '\t' || input[i] == '\n') {
            i++;
        }
    }
    
    result->argc = token_idx;
    return 0;
}

shell_command_t* shell_find_command(const char* name)
{
    if (!name) return NULL;
    
    for (int i = 0; command_table[i].name != NULL; i++) {
        if (strcmp(name, command_table[i].name) == 0) {
            return &command_table[i];
        }
    }
    
    return NULL;
}

int shell_execute_command(const token_result_t* tokens)
{
    if (!tokens || tokens->argc == 0) return -1;
    
    shell_command_t* cmd = shell_find_command(tokens->argv[0]);
    if (!cmd) {
        printf("Unknown command: '%s'\n", tokens->argv[0]);
        puts("Type 'help' to see available commands, or 'about' for system info.");
        
        // Suggest similar commands for common mistakes
        if (strcmp(tokens->argv[0], "ls") == 0) {
            puts("Hint: This OS has no filesystem. Try 'help' instead.");
        } else if (strcmp(tokens->argv[0], "exit") == 0 || strcmp(tokens->argv[0], "quit") == 0) {
            puts("Hint: This OS runs indefinitely. Use Ctrl+A, X to quit QEMU.");
        } else if (strcmp(tokens->argv[0], "cat") == 0 || strcmp(tokens->argv[0], "more") == 0) {
            puts("Hint: No filesystem available. Try 'meminfo' to see memory status.");
        }
        
        return -1;
    }
    
    return cmd->handler(tokens->argc, tokens->argv);
}

int shell_parse_and_execute(const char* input)
{
    if (!input) return -1;
    
    // Skip empty input or whitespace-only input
    const char* p = input;
    while (*p == ' ' || *p == '\t' || *p == '\n') p++;
    if (*p == '\0') return 0;  // Empty input is not an error
    
    token_result_t tokens;
    int result = shell_tokenize(input, &tokens);
    if (result != 0) {
        puts("Error: Failed to parse command");
        return result;
    }
    
    if (tokens.argc == 0) return 0;  // No tokens, not an error
    
    return shell_execute_command(&tokens);
}

// Command implementations for Day 11
int cmd_help(int argc, char* argv[])
{
    // If specific command requested
    if (argc > 1) {
        shell_command_t* cmd = shell_find_command(argv[1]);
        if (!cmd) {
            printf("Unknown command: '%s'\n", argv[1]);
            return -1;
        }
        
        printf("Command: %s\n", cmd->name);
        printf("Description: %s\n", cmd->description);
        
        // Add usage examples for specific commands
        if (strcmp(cmd->name, "echo") == 0) {
            puts("Usage: echo <text>");
            puts("Example: echo Hello World");
        } else if (strcmp(cmd->name, "help") == 0) {
            puts("Usage: help [command]");
            puts("Examples:");
            puts("  help        - Show all commands");
            puts("  help echo   - Show help for echo command");
        } else if (strcmp(cmd->name, "clear") == 0) {
            puts("Usage: clear");
            puts("Example: clear");
        } else if (strcmp(cmd->name, "meminfo") == 0) {
            puts("Usage: meminfo");
            puts("Example: meminfo");
        } else if (strcmp(cmd->name, "about") == 0) {
            puts("Usage: about");
            puts("Example: about");
        } else if (strcmp(cmd->name, "uptime") == 0) {
            puts("Usage: uptime");
            puts("Example: uptime");
        } else if (strcmp(cmd->name, "calc") == 0) {
            puts("Usage: calc <number1> <operator> <number2>");
            puts("Examples: calc 10 + 5, calc 20 - 8");
        }
        
        return 0;
    }
    
    // Show all commands with better formatting
    puts("=== ARM64 OS Shell - Available Commands ===");
    puts("");
    
    for (int i = 0; command_table[i].name != NULL; i++) {
        printf("%s - %s\n", command_table[i].name, command_table[i].description);
    }
    
    puts("");
    puts("Type 'help <command>' for detailed information about a command.");
    puts("Example: help echo");
    
    return 0;
}

int cmd_echo(int argc, char* argv[])
{
    if (argc < 2) {
        puts("Usage: echo <text>");
        puts("Example: echo Hello World");
        return -1;
    }
    
    for (int i = 1; i < argc; i++) {
        printf("%s", argv[i]);
        if (i < argc - 1) printf(" ");
    }
    printf("\n");
    return 0;
}

int cmd_clear(int argc, char* argv[])
{
    if (argc > 1) {
        puts("Usage: clear");
        puts("The clear command takes no arguments.");
        return -1;
    }
    
    // Clear screen by sending escape sequences
    // ESC[2J clears entire screen, ESC[H moves cursor to top-left
    printf("\x1b[2J\x1b[H");
    return 0;
}

int cmd_meminfo(int argc, char* argv[])
{
    if (argc > 1) {
        puts("Usage: meminfo");
        puts("The meminfo command takes no arguments.");
        return -1;
    }
    
    // Use the existing memory_info function which displays comprehensive stats
    memory_info();
    return 0;
}

int cmd_about(int argc, char* argv[])
{
    if (argc > 1) {
        puts("Usage: about");
        puts("The about command takes no arguments.");
        return -1;
    }
    
    puts("=== ARM64 OS Information ===");
    puts("");
    puts("Operating System: ARM64 OS");
    puts("Version: Phase 2 Complete");
    puts("Architecture: ARM64 (AArch64)");
    puts("Target Platform: QEMU virt machine");
    puts("Memory Management: Bump allocator with 1MB heap");
    puts("Shell: Interactive command processor");
    puts("");
    puts("Features:");
    puts("- Serial I/O with PL011 UART");
    puts("- Memory allocation and utilities");
    puts("- Interactive shell with 7 commands");
    puts("- String processing functions");
    puts("- Command parsing and execution");
    puts("");
    printf("Build target: %s\n", "aarch64-elf");
    puts("No filesystem, no virtual memory, no interrupts");
    puts("Designed for educational purposes");
    
    return 0;
}

int cmd_uptime(int argc, char* argv[])
{
    if (argc > 1) {
        puts("Usage: uptime");
        puts("The uptime command takes no arguments.");
        return -1;
    }
    
    // Simple uptime placeholder - no real timer implemented
    puts("System uptime: Unknown (timer not implemented)");
    puts("Note: This is a minimal OS without timer hardware support");
    
    return 0;
}

int cmd_calc(int argc, char* argv[])
{
    if (argc != 4) {
        puts("Usage: calc <number1> <operator> <number2>");
        puts("Supported operators: +, -, *, /");
        puts("Examples:");
        puts("  calc 10 + 5");
        puts("  calc 20 - 8");
        puts("  calc 6 * 7");
        puts("  calc 15 / 3");
        return -1;
    }
    
    // Simple string to integer conversion
    int num1 = 0, num2 = 0;
    char* str1 = argv[1];
    char* str2 = argv[3];
    char op = argv[2][0];
    
    // Convert first number
    for (int i = 0; str1[i] != '\0'; i++) {
        if (str1[i] >= '0' && str1[i] <= '9') {
            num1 = num1 * 10 + (str1[i] - '0');
        } else {
            puts("Error: First argument is not a valid number");
            return -1;
        }
    }
    
    // Convert second number  
    for (int i = 0; str2[i] != '\0'; i++) {
        if (str2[i] >= '0' && str2[i] <= '9') {
            num2 = num2 * 10 + (str2[i] - '0');
        } else {
            puts("Error: Second argument is not a valid number");
            return -1;
        }
    }
    
    int result = 0;
    int valid = 1;
    
    switch (op) {
        case '+':
            result = num1 + num2;
            break;
        case '-':
            result = num1 - num2;
            break;
        case '*':
            result = num1 * num2;
            break;
        case '/':
            if (num2 == 0) {
                puts("Error: Division by zero");
                return -1;
            }
            result = num1 / num2;
            break;
        default:
            printf("Error: Unknown operator '%c'\n", op);
            puts("Supported operators: +, -, *, /");
            valid = 0;
            break;
    }
    
    if (valid) {
        printf("%d %c %d = %d\n", num1, op, num2, result);
    }
    
    return valid ? 0 : -1;
}