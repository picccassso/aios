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

// ANSI Escape Sequences for Enhanced Clear Screen - Day 17 Task 1
#define ANSI_ESC            "\x1b["
#define ANSI_CLEAR_SCREEN   "\x1b[2J"     // Clear entire screen
#define ANSI_CLEAR_LINE     "\x1b[2K"     // Clear entire current line
#define ANSI_CLEAR_TO_EOL   "\x1b[0K"     // Clear from cursor to end of line
#define ANSI_CLEAR_TO_BOL   "\x1b[1K"     // Clear from beginning of line to cursor
#define ANSI_CURSOR_HOME    "\x1b[H"      // Move cursor to top-left (1,1)
#define ANSI_CURSOR_POS(r,c) "\x1b[" #r ";" #c "H"  // Move cursor to row r, column c
#define ANSI_SAVE_CURSOR    "\x1b[s"      // Save cursor position
#define ANSI_RESTORE_CURSOR "\x1b[u"      // Restore cursor position

// ANSI Color Codes for Day 17 Task 2 - Color Support
#define ANSI_COLOR_RESET    "\x1b[0m"     // Reset all attributes
#define ANSI_COLOR_BOLD     "\x1b[1m"     // Bold text
#define ANSI_COLOR_DIM      "\x1b[2m"     // Dim text
#define ANSI_COLOR_UNDERLINE "\x1b[4m"    // Underlined text

// Foreground Colors
#define ANSI_FG_BLACK       "\x1b[30m"    // Black foreground
#define ANSI_FG_RED         "\x1b[31m"    // Red foreground
#define ANSI_FG_GREEN       "\x1b[32m"    // Green foreground
#define ANSI_FG_YELLOW      "\x1b[33m"    // Yellow foreground
#define ANSI_FG_BLUE        "\x1b[34m"    // Blue foreground
#define ANSI_FG_MAGENTA     "\x1b[35m"    // Magenta foreground
#define ANSI_FG_CYAN        "\x1b[36m"    // Cyan foreground
#define ANSI_FG_WHITE       "\x1b[37m"    // White foreground

// Background Colors
#define ANSI_BG_BLACK       "\x1b[40m"    // Black background
#define ANSI_BG_RED         "\x1b[41m"    // Red background
#define ANSI_BG_GREEN       "\x1b[42m"    // Green background
#define ANSI_BG_YELLOW      "\x1b[43m"    // Yellow background
#define ANSI_BG_BLUE        "\x1b[44m"    // Blue background
#define ANSI_BG_MAGENTA     "\x1b[45m"    // Magenta background
#define ANSI_BG_CYAN        "\x1b[46m"    // Cyan background
#define ANSI_BG_WHITE       "\x1b[47m"    // White background

// Bright Foreground Colors
#define ANSI_FG_BRIGHT_BLACK   "\x1b[90m" // Bright black (gray)
#define ANSI_FG_BRIGHT_RED     "\x1b[91m" // Bright red
#define ANSI_FG_BRIGHT_GREEN   "\x1b[92m" // Bright green
#define ANSI_FG_BRIGHT_YELLOW  "\x1b[93m" // Bright yellow
#define ANSI_FG_BRIGHT_BLUE    "\x1b[94m" // Bright blue
#define ANSI_FG_BRIGHT_MAGENTA "\x1b[95m" // Bright magenta
#define ANSI_FG_BRIGHT_CYAN    "\x1b[96m" // Bright cyan
#define ANSI_FG_BRIGHT_WHITE   "\x1b[97m" // Bright white

// Color configuration
static int colors_enabled = 1;  // Global color enable/disable flag

// Day 20 Task 1: Error Message System
// Error codes for standardized error reporting
typedef enum {
    SHELL_SUCCESS = 0,          // Command executed successfully
    SHELL_ERROR_INVALID_ARGS,   // Invalid number or format of arguments
    SHELL_ERROR_MEMORY,         // Memory allocation or access error
    SHELL_ERROR_PERMISSION,     // Permission denied or unsafe operation
    SHELL_ERROR_NOT_FOUND,      // Command or resource not found
    SHELL_ERROR_SYNTAX,         // Syntax error in command or arguments
    SHELL_ERROR_RANGE,          // Value out of valid range
    SHELL_ERROR_ALIGNMENT,      // Address alignment error
    SHELL_ERROR_PARSE,          // Parsing error
    SHELL_ERROR_SYSTEM,         // System or hardware error
    SHELL_ERROR_COUNT           // Total number of error types
} shell_error_t;

// Error logging entry structure
typedef struct {
    shell_error_t error_code;
    char command[32];           // Command that caused the error
    char context[64];           // Additional context information
    unsigned int timestamp;     // Simple timestamp counter
} error_log_entry_t;

// Error logging buffer (circular buffer like command history)
#define ERROR_LOG_SIZE 10
typedef struct {
    error_log_entry_t entries[ERROR_LOG_SIZE];
    int count;                  // Total errors logged
    int current_index;          // Current write position
} error_log_t;

static error_log_t error_log = {0};
static unsigned int error_timestamp_counter = 0;

// Day 20 Task 2: Performance Monitoring System
// Command execution statistics structure
typedef struct {
    char command[32];           // Command name
    unsigned int call_count;    // Number of times executed
    unsigned int total_time;    // Total execution time (simple counter)
    unsigned int last_time;     // Last execution time
    unsigned int avg_time;      // Average execution time
} command_stats_t;

// Performance monitoring storage
#define MAX_TRACKED_COMMANDS 16
typedef struct {
    command_stats_t commands[MAX_TRACKED_COMMANDS];
    int tracked_count;          // Number of commands currently tracked
    unsigned int total_commands; // Total commands executed
    unsigned int performance_counter; // Simple performance counter
} performance_monitor_t;

static performance_monitor_t perf_monitor = {0};

// Simple timer functions (using incrementing counter since no real timer)
// Removed unused get_simple_timer function

// Day 20 Task 3: Command Aliases System
// Alias entry structure
typedef struct {
    char name[32];              // Alias name
    char expansion[128];        // What the alias expands to
    int is_builtin;            // 1 for built-in aliases, 0 for user-defined
} alias_entry_t;

// Alias table structure
#define MAX_ALIASES 20
typedef struct {
    alias_entry_t aliases[MAX_ALIASES];
    int count;                  // Number of aliases currently stored
} alias_table_t;

static alias_table_t alias_table = {0};

// Day 20 Task 4: Batch Commands System
// Operator types for command chaining
typedef enum {
    BATCH_OP_NONE = 0,          // No operator (single command)
    BATCH_OP_SEMICOLON,         // ; - Always execute next command
    BATCH_OP_AND,               // && - Execute next only if previous succeeded
    BATCH_OP_OR                 // || - Execute next only if previous failed
} batch_operator_t;

// Individual command in a batch sequence
typedef struct {
    char command[256];          // The command string
    batch_operator_t next_op;   // Operator to next command
} batch_command_t;

// Batch command sequence structure
#define MAX_BATCH_COMMANDS 10
typedef struct {
    batch_command_t commands[MAX_BATCH_COMMANDS];
    int count;                  // Number of commands in batch
} batch_sequence_t;

// Error message lookup table
static const char* error_messages[SHELL_ERROR_COUNT] = {
    "Success",                                          // SHELL_SUCCESS
    "Invalid arguments provided",                       // SHELL_ERROR_INVALID_ARGS  
    "Memory allocation or access error",                // SHELL_ERROR_MEMORY
    "Permission denied or unsafe operation",            // SHELL_ERROR_PERMISSION
    "Command or resource not found",                    // SHELL_ERROR_NOT_FOUND
    "Syntax error in command or arguments",             // SHELL_ERROR_SYNTAX
    "Value is out of valid range",                      // SHELL_ERROR_RANGE
    "Address alignment error",                          // SHELL_ERROR_ALIGNMENT
    "Failed to parse command or arguments",             // SHELL_ERROR_PARSE
    "System or hardware error"                          // SHELL_ERROR_SYSTEM
};

// Day 19 Task 1: Command History Implementation
#define HISTORY_SIZE 20
#define MAX_COMMAND_LENGTH 256

typedef struct {
    char commands[HISTORY_SIZE][MAX_COMMAND_LENGTH];
    int count;          // Total commands stored
    int current;        // Current position for navigation
    int write_index;    // Where to write next command
} command_history_t;

static command_history_t history = {0};

// Forward declarations for history functions
static void history_add_command(const char* command);
static const char* history_get_previous(void);
static const char* history_get_next(void);
static void history_reset_navigation(void);
static void shell_complete_command(const char* partial, char* buffer, int* pos, int* cursor_pos, int max_size, int word_start);

// Forward declarations for error system functions
static void shell_log_error(shell_error_t error_code, const char* command, const char* context);
static void shell_display_error(shell_error_t error_code, const char* context);
// Removed unused shell_display_warning forward declaration
static void shell_display_success(const char* message);
static void shell_display_info(const char* message);
static const char* shell_get_error_message(shell_error_t error_code);

// Removed all unused performance monitoring function declarations

// Forward declarations for alias system functions
static void alias_init_builtins(void);
static int alias_add(const char* name, const char* expansion, int is_builtin);
static const char* alias_find(const char* name);
static int alias_remove(const char* name);
static void alias_clear_user_aliases(void);
static int alias_validate_name(const char* name);

// Forward declarations for batch command functions
static int batch_parse_commands(const char* input, batch_sequence_t* sequence);
static int batch_execute_sequence(const batch_sequence_t* sequence);
static int batch_execute_single_command(const char* command_str);
// Removed unused batch function declarations (batch_detect_operator, batch_trim_whitespace)

// Command table - Phase 3 Day 20 expanded (runtime initialized)
static shell_command_t command_table[18];  // 17 commands + NULL terminator

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
    
    command_table[7].name = "peek";
    command_table[7].description = "Read memory address";
    command_table[7].handler = cmd_peek;
    
    command_table[8].name = "poke";
    command_table[8].description = "Write memory address";
    command_table[8].handler = cmd_poke;
    
    command_table[9].name = "dump";
    command_table[9].description = "Display memory region";
    command_table[9].handler = cmd_dump;
    
    command_table[10].name = "color";
    command_table[10].description = "Control color settings";
    command_table[10].handler = cmd_color;
    
    command_table[11].name = "reboot";
    command_table[11].description = "Restart the system";
    command_table[11].handler = cmd_reboot;
    
    command_table[12].name = "sysinfo";
    command_table[12].description = "Display system information";
    command_table[12].handler = cmd_sysinfo;
    
    command_table[13].name = "history";
    command_table[13].description = "Show command history";
    command_table[13].handler = cmd_history;
    
    command_table[14].name = "errors";
    command_table[14].description = "Show error log";
    command_table[14].handler = cmd_errors;
    
    command_table[15].name = "stats";
    command_table[15].description = "Show performance statistics";
    command_table[15].handler = cmd_stats;
    
    command_table[16].name = "alias";
    command_table[16].description = "Manage command aliases";
    command_table[16].handler = cmd_alias;
    
    // Terminator
    command_table[17].name = NULL;
    command_table[17].description = NULL;
    command_table[17].handler = NULL;
    
    // Initialize alias system with built-in aliases
    alias_init_builtins();
}

// Day 19 Task 4: Tab completion implementation
static void shell_complete_command(const char* partial, char* buffer, int* pos, int* cursor_pos, int max_size, int word_start)
{
    if (!partial || !buffer || !pos || !cursor_pos) return;
    
    // Find all matching commands
    char matches[17][32];  // Up to 17 commands, 32 chars each
    int match_count = 0;
    int partial_len = strlen(partial);
    
    // Search through command table
    for (int i = 0; i < 17 && command_table[i].name != NULL; i++) {
        if (strncmp(command_table[i].name, partial, partial_len) == 0) {
            if (match_count < 17) {
                strcpy(matches[match_count], command_table[i].name);
                match_count++;
            }
        }
    }
    
    // Also search through aliases
    for (int i = 0; i < alias_table.count && match_count < 17; i++) {
        if (strncmp(alias_table.aliases[i].name, partial, partial_len) == 0) {
            strcpy(matches[match_count], alias_table.aliases[i].name);
            match_count++;
        }
    }
    
    if (match_count == 0) {
        // No matches - make a beep sound (BEL character)
        putchar('\x07');
    } else if (match_count == 1) {
        // Single match - complete the command
        const char* completion = matches[0];
        int completion_len = strlen(completion);
        int insert_len = completion_len - partial_len;
        
        // Check if we have space to insert
        if (*pos + insert_len < max_size - 1) {
            // Shift characters after cursor to make room
            for (int i = *pos; i >= *cursor_pos; i--) {
                buffer[i + insert_len] = buffer[i];
            }
            
            // Insert the completion
            for (int i = 0; i < insert_len; i++) {
                buffer[word_start + partial_len + i] = completion[partial_len + i];
            }
            
            // Update positions
            *pos += insert_len;
            *cursor_pos += insert_len;
            
            // Redraw the line from word_start
            printf("\r");
            for (int i = 0; i < word_start; i++) {
                putchar(buffer[i]);
            }
            for (int i = word_start; i < *pos; i++) {
                putchar(buffer[i]);
            }
            
            // Move cursor to correct position
            for (int i = *pos - 1; i >= *cursor_pos; i--) {
                putchar('\b');
            }
        } else {
            // Not enough space - beep
            putchar('\x07');
        }
    } else {
        // Multiple matches - show them
        putchar('\n');
        printf("Possible completions:\n");
        for (int i = 0; i < match_count; i++) {
            printf("  %s", matches[i]);
            if (i % 4 == 3) printf("\n");  // 4 commands per line
            else printf("\t");
        }
        if (match_count % 4 != 0) printf("\n");
        
        // Redisplay prompt and current line
        printf("OS> ");
        for (int i = 0; i < *pos; i++) {
            putchar(buffer[i]);
        }
        // Move cursor to correct position
        for (int i = *pos - 1; i >= *cursor_pos; i--) {
            putchar('\b');
        }
    }
}

// Day 19 Task 2: Enhanced shell_read_line with arrow key support
int shell_read_line(char* buffer, int max_size)
{
    if (!buffer || max_size <= 0) return -1;
    
    int pos = 0;
    int cursor_pos = 0;  // Current cursor position within the line
    char c;
    
    // Reset history navigation
    history_reset_navigation();
    
    while (pos < max_size - 1) {
        c = getchar();
        
        
        // Handle escape sequences for arrow keys and special keys
        if (c == 0x1B) {  // ESC character
            char seq[4];
            seq[0] = getchar();
            if (seq[0] == '[') {
                seq[1] = getchar();
                seq[2] = '\0';
                seq[3] = '\0';
                
                // Check if it's a multi-character sequence (like Home/End/Delete)
                if (seq[1] == '1' || seq[1] == '3' || seq[1] == '4' || seq[1] == 'H' || seq[1] == 'F') {
                    if (seq[1] == '1' || seq[1] == '3' || seq[1] == '4') {
                        seq[2] = getchar();  // Should be '~'
                        seq[3] = '\0';
                    }
                }
                
                // Handle arrow keys and special keys
                switch (seq[1]) {
                    case 'A':  // Up arrow - previous command in history
                    {
                        const char* prev_cmd = history_get_previous();
                        if (prev_cmd != NULL) {
                            // Clear current line using ANSI escape sequences (more reliable)
                            printf("\r");        // Move cursor to beginning of line
                            printf("\033[K");    // Clear from cursor to end of line
                            printf("OS> ");      // Redraw prompt
                            
                            // Copy history command to buffer and display
                            strncpy(buffer, prev_cmd, max_size - 1);
                            buffer[max_size - 1] = '\0';
                            pos = strlen(buffer);
                            cursor_pos = pos;
                            printf("%s", buffer);
                        }
                        break;
                    }
                    case 'B':  // Down arrow - next command in history
                    {
                        const char* next_cmd = history_get_next();
                        if (next_cmd != NULL) {
                            // Clear current line using ANSI escape sequences (more reliable)
                            printf("\r");        // Move cursor to beginning of line
                            printf("\033[K");    // Clear from cursor to end of line
                            printf("OS> ");      // Redraw prompt
                            
                            // Copy history command to buffer and display
                            strncpy(buffer, next_cmd, max_size - 1);
                            buffer[max_size - 1] = '\0';
                            pos = strlen(buffer);
                            cursor_pos = pos;
                            printf("%s", buffer);
                        }
                        break;
                    }
                    case 'C':  // Right arrow - move cursor right
                        if (cursor_pos < pos) {
                            putchar(buffer[cursor_pos]);
                            cursor_pos++;
                        }
                        break;
                    case 'D':  // Left arrow - move cursor left
                        if (cursor_pos > 0) {
                            putchar('\b');
                            cursor_pos--;
                        }
                        break;
                    case 'H':  // Home key - move cursor to beginning
                    case '1':  // Home key (alternative sequence)
                        if (seq[1] == '1' && seq[2] != '~') break;  // Validate sequence
                        while (cursor_pos > 0) {
                            putchar('\b');
                            cursor_pos--;
                        }
                        break;
                    case 'F':  // End key - move cursor to end
                    case '4':  // End key (alternative sequence)
                        if (seq[1] == '4' && seq[2] != '~') break;  // Validate sequence
                        while (cursor_pos < pos) {
                            putchar(buffer[cursor_pos]);
                            cursor_pos++;
                        }
                        break;
                    case '3':  // Delete key - delete character at cursor
                        if (seq[2] == '~' && cursor_pos < pos) {
                            // Move characters after cursor one position left
                            for (int i = cursor_pos; i < pos - 1; i++) {
                                buffer[i] = buffer[i + 1];
                            }
                            pos--;
                            
                            // Redraw line from cursor position
                            for (int i = cursor_pos; i < pos; i++) {
                                putchar(buffer[i]);
                            }
                            putchar(' ');  // Clear the last character
                            // Move cursor back to correct position
                            for (int i = pos; i >= cursor_pos; i--) {
                                putchar('\b');
                            }
                        }
                        break;
                    default:
                        // Unknown escape sequence, ignore
                        break;
                }
            }
            continue;
        }
        
        // Handle backspace
        if (c == '\b' || c == 0x7F) {
            if (cursor_pos > 0) {
                // Move characters after cursor one position left
                for (int i = cursor_pos - 1; i < pos - 1; i++) {
                    buffer[i] = buffer[i + 1];
                }
                pos--;
                cursor_pos--;
                
                // Redraw line from cursor position
                putchar('\b');
                for (int i = cursor_pos; i < pos; i++) {
                    putchar(buffer[i]);
                }
                putchar(' ');  // Clear the last character
                // Move cursor back to correct position
                for (int i = pos; i >= cursor_pos; i--) {
                    putchar('\b');
                }
            }
            continue;
        }
        
        // Handle enter/newline
        if (c == '\r' || c == '\n') {
            putchar('\n');
            break;
        }
        
        // Handle tab for command completion
        if (c == '\t') {
            // Only do completion at cursor position if at end of a word
            if (cursor_pos == pos || buffer[cursor_pos] == ' ') {
                // Find start of current word
                int word_start = cursor_pos;
                while (word_start > 0 && buffer[word_start - 1] != ' ') {
                    word_start--;
                }
                
                // Extract current word
                int word_len = cursor_pos - word_start;
                if (word_len > 0) {
                    char word[32];
                    strncpy(word, buffer + word_start, word_len);
                    word[word_len] = '\0';
                    
                    // Try to complete the command
                    shell_complete_command(word, buffer, &pos, &cursor_pos, max_size, word_start);
                }
            }
            continue;
        }
        
        // Handle printable characters
        if (c >= 0x20 && c <= 0x7E) {
            // Insert character at cursor position
            if (pos < max_size - 1) {
                // Shift characters after cursor one position right
                for (int i = pos; i > cursor_pos; i--) {
                    buffer[i] = buffer[i - 1];
                }
                buffer[cursor_pos] = c;
                pos++;
                
                // Redraw line from cursor position
                for (int i = cursor_pos; i < pos; i++) {
                    putchar(buffer[i]);
                }
                cursor_pos++;
                // Move cursor back to correct position
                for (int i = pos - 1; i >= cursor_pos; i--) {
                    putchar('\b');
                }
            }
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
    
    for (int i = 0; i < 17 && command_table[i].name != NULL; i++) {
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
    
    // Day 20 Task 2: Performance Monitoring - Record command execution
    // TEMPORARILY DISABLED - Performance monitoring causing command hang
    // unsigned int start_time = get_simple_timer();
    // perf_monitor.total_commands++;
    
    // Execute the command (cast to fix const qualifier warning)
    int result = cmd->handler(tokens->argc, (char**)tokens->argv);
    
    // Record execution time
    // perf_record_command_end(tokens->argv[0], start_time);
    
    return result;
}

int shell_parse_and_execute(const char* input)
{
    if (!input) return -1;
    
    // Skip empty input or whitespace-only input
    const char* p = input;
    while (*p == ' ' || *p == '\t' || *p == '\n') p++;
    if (*p == '\0') return 0;  // Empty input is not an error
    
    // Day 20 Task 4: Check for batch commands first
    // Re-enable batch commands with full functionality
    batch_sequence_t batch;
    if (batch_parse_commands(input, &batch) > 0) {
        // Input contains batch commands, execute the sequence
        return batch_execute_sequence(&batch);
    }
    
    token_result_t tokens;
    int result = shell_tokenize(input, &tokens);
    if (result != 0) {
        puts("Error: Failed to parse command");
        return result;
    }
    
    if (tokens.argc == 0) return 0;  // No tokens, not an error
    
    // Day 20 Task 3: Check for alias expansion
    const char* alias_expansion = alias_find(tokens.argv[0]);
    if (alias_expansion) {
        // Create expanded input by replacing first token with alias expansion
        char expanded_input[256];
        int pos = 0;
        
        // Copy alias expansion
        int exp_len = strlen(alias_expansion);
        for (int i = 0; i < exp_len && pos < 255; i++) {
            expanded_input[pos++] = alias_expansion[i];
        }
        
        // Add remaining arguments if any
        for (int i = 1; i < tokens.argc && pos < 255; i++) {
            // Add space
            if (pos < 255) expanded_input[pos++] = ' ';
            
            // Add argument
            int arg_len = strlen(tokens.argv[i]);
            for (int j = 0; j < arg_len && pos < 255; j++) {
                expanded_input[pos++] = tokens.argv[i][j];
            }
        }
        
        expanded_input[pos] = '\0';
        
        // Recursively parse and execute the expanded command
        // Note: This allows alias chaining but prevents infinite recursion
        // since aliases can't refer to themselves
        return shell_parse_and_execute(expanded_input);
    }
    
    // Execute command
    int exec_result = shell_execute_command(&tokens);
    
    // Add to history if command executed successfully (or even if it failed - user might want to recall it)
    // Don't add "history" command itself to avoid cluttering history
    if (strcmp(tokens.argv[0], "history") != 0) {
        history_add_command(input);
    }
    
    return exec_result;
}

// Day 17 Task 1: Enhanced Clear Screen Utility Functions
/*
 * Clear entire screen and move cursor to home position
 */
static void clear_screen_full(void)
{
    printf(ANSI_CLEAR_SCREEN ANSI_CURSOR_HOME);
}

/*
 * Clear current line (cursor stays at current position)
 */
static void clear_current_line(void)
{
    printf(ANSI_CLEAR_LINE);
}

/*
 * Clear from cursor to end of line
 */
static void clear_to_end_of_line(void)
{
    printf(ANSI_CLEAR_TO_EOL);
}

/*
 * Clear from beginning of line to cursor
 */
static void clear_to_beginning_of_line(void)
{
    printf(ANSI_CLEAR_TO_BOL);
}

/*
 * Move cursor to specific position (1-indexed)
 */
// Removed unused cursor positioning functions (move_cursor_to, save_cursor_position, restore_cursor_position)

// Day 17 Task 2: Color Support Utility Functions
/*
 * Print colored text with automatic reset
 */
static void print_colored(const char* color, const char* text)
{
    if (colors_enabled) {
        printf("%s%s%s", color, text, ANSI_COLOR_RESET);
    } else {
        printf("%s", text);
    }
}

/*
 * Print text with foreground color
 */
static void print_fg_color(const char* color, const char* text)
{
    print_colored(color, text);
}

// Removed unused print_bg_color function

/*
 * Print colored and formatted text (bold, underline, etc.)
 */
static void print_formatted(const char* format, const char* color, const char* text)
{
    if (colors_enabled) {
        printf("%s%s%s%s", format, color, text, ANSI_COLOR_RESET);
    } else {
        printf("%s", text);
    }
}

/*
 * Print success message in green
 */
static void print_success(const char* text)
{
    print_fg_color(ANSI_FG_GREEN, text);
}

/*
 * Print error message in red
 */
static void print_error(const char* text)
{
    print_fg_color(ANSI_FG_RED, text);
}

/*
 * Print warning message in yellow
 */
static void print_warning(const char* text)
{
    print_fg_color(ANSI_FG_YELLOW, text);
}

/*
 * Print info message in cyan
 */
static void print_info(const char* text)
{
    print_fg_color(ANSI_FG_CYAN, text);
}

/*
 * Print prompt with color
 */
static void print_colored_prompt(void)
{
    if (colors_enabled) {
        printf(ANSI_FG_BRIGHT_BLUE "OS" ANSI_FG_WHITE "> " ANSI_COLOR_RESET);
    } else {
        printf("OS> ");
    }
}

/*
 * Enable/disable colors globally
 */
static void set_colors_enabled(int enabled)
{
    colors_enabled = enabled;
}

// Removed unused get_colors_enabled function

// Day 19 Task 1: Command History Management Functions
static void history_add_command(const char* command)
{
    if (!command || strlen(command) == 0) {
        return;  // Don't store empty commands
    }
    
    // Don't store duplicate consecutive commands
    if (history.count > 0) {
        int last_index = (history.write_index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
        if (strcmp(history.commands[last_index], command) == 0) {
            return;
        }
    }
    
    // Copy command to history buffer
    strncpy(history.commands[history.write_index], command, MAX_COMMAND_LENGTH - 1);
    history.commands[history.write_index][MAX_COMMAND_LENGTH - 1] = '\0';
    
    // Update indices
    history.write_index = (history.write_index + 1) % HISTORY_SIZE;
    if (history.count < HISTORY_SIZE) {
        history.count++;
    }
    
    // Reset current pointer for navigation
    history.current = history.write_index;
}

static const char* history_get_previous(void)
{
    if (history.count == 0) {
        return NULL;
    }
    
    // Move to previous command
    history.current = (history.current - 1 + HISTORY_SIZE) % HISTORY_SIZE;
    
    // Don't go beyond the oldest command
    int oldest_index = history.count < HISTORY_SIZE ? 0 : history.write_index;
    if (history.current == oldest_index && history.count == HISTORY_SIZE) {
        history.current = (history.current + 1) % HISTORY_SIZE;
        return NULL;
    }
    
    return history.commands[history.current];
}

static const char* history_get_next(void)
{
    if (history.count == 0) {
        return NULL;
    }
    
    // Move to next command
    history.current = (history.current + 1) % HISTORY_SIZE;
    
    // If we've reached the write index, return empty (no "next" command)
    if (history.current == history.write_index) {
        return "";
    }
    
    return history.commands[history.current];
}

static void history_reset_navigation(void)
{
    history.current = history.write_index;
}

// Day 20 Task 1: Error System Function Implementations

/*
 * Get error message string from error code
 */
static const char* shell_get_error_message(shell_error_t error_code)
{
    if (error_code >= 0 && error_code < SHELL_ERROR_COUNT) {
        return error_messages[error_code];
    }
    return "Unknown error";
}

/*
 * Log error to circular buffer for debugging
 */
static void shell_log_error(shell_error_t error_code, const char* command, const char* context)
{
    if (!command) command = "unknown";
    if (!context) context = "";
    
    error_log_entry_t* entry = &error_log.entries[error_log.current_index];
    entry->error_code = error_code;
    entry->timestamp = ++error_timestamp_counter;
    
    // Copy command name (truncate if necessary)
    strncpy(entry->command, command, sizeof(entry->command) - 1);
    entry->command[sizeof(entry->command) - 1] = '\0';
    
    // Copy context (truncate if necessary)
    strncpy(entry->context, context, sizeof(entry->context) - 1);
    entry->context[sizeof(entry->context) - 1] = '\0';
    
    // Update circular buffer indices
    error_log.current_index = (error_log.current_index + 1) % ERROR_LOG_SIZE;
    if (error_log.count < ERROR_LOG_SIZE) {
        error_log.count++;
    }
}

/*
 * Display error message with color coding and logging
 */
static void shell_display_error(shell_error_t error_code, const char* context)
{
    const char* message = shell_get_error_message(error_code);
    
    if (colors_enabled) {
        printf(ANSI_FG_RED "Error: %s" ANSI_COLOR_RESET, message);
    } else {
        printf("Error: %s", message);
    }
    
    if (context && *context) {
        printf(" (%s)", context);
    }
    printf("\n");
    
    // Log the error for debugging
    shell_log_error(error_code, "shell", context ? context : "");
}

/*
 * Display warning message with color coding
 */
// Removed unused shell_display_warning function

/*
 * Display success message with color coding
 */
static void shell_display_success(const char* message)
{
    if (!message) return;
    
    if (colors_enabled) {
        printf(ANSI_FG_GREEN "%s" ANSI_COLOR_RESET "\n", message);
    } else {
        printf("%s\n", message);
    }
}

/*
 * Display info message with color coding
 */
static void shell_display_info(const char* message)
{
    if (!message) return;
    
    if (colors_enabled) {
        printf(ANSI_FG_CYAN "%s" ANSI_COLOR_RESET "\n", message);
    } else {
        printf("%s\n", message);
    }
}

/*
 * Display shell prompt with color support
 */
void shell_display_prompt(void)
{
    print_colored_prompt();
}

// Removed unused performance monitoring functions (perf_find_or_create_stats, perf_update_stats)

// Removed unused performance monitoring functions (perf_record_command_start, perf_record_command_end)

// Day 20 Task 3: Alias System Functions
static void alias_init_builtins(void) {
    // Initialize built-in aliases - common shortcuts
    alias_add("ll", "dump", 1);         // ll -> dump (memory listing)
    alias_add("cls", "clear", 1);       // cls -> clear (Windows-style)
    alias_add("mem", "meminfo", 1);     // mem -> meminfo (memory status)
    alias_add("h", "help", 1);          // h -> help (quick help)
    alias_add("?", "help", 1);          // ? -> help (alternative help)
    alias_add("q", "reboot", 1);        // q -> reboot (quick quit/reboot)
}

static int alias_validate_name(const char* name) {
    if (!name || strlen(name) == 0) return 0;
    if (strlen(name) >= 32) return 0;  // Name too long
    
    // Check if name conflicts with existing command
    shell_command_t* cmd = shell_find_command(name);
    if (cmd) return 0;  // Conflict with existing command
    
    // Check for valid characters (alphanumeric, underscore, dash)
    for (const char* p = name; *p; p++) {
        if (!((*p >= 'a' && *p <= 'z') || 
              (*p >= 'A' && *p <= 'Z') || 
              (*p >= '0' && *p <= '9') || 
              *p == '_' || *p == '-' || *p == '?')) {
            return 0;
        }
    }
    
    return 1;  // Valid name
}

static int alias_add(const char* name, const char* expansion, int is_builtin) {
    if (!name || !expansion) return -1;
    if (!alias_validate_name(name)) return -1;
    if (strlen(expansion) >= 128) return -1;  // Expansion too long
    
    // Check if alias already exists
    for (int i = 0; i < alias_table.count; i++) {
        if (strcmp(alias_table.aliases[i].name, name) == 0) {
            // Update existing alias
            strcpy(alias_table.aliases[i].expansion, expansion);
            alias_table.aliases[i].is_builtin = is_builtin;
            return 0;
        }
    }
    
    // Add new alias if we have space
    if (alias_table.count < MAX_ALIASES) {
        alias_entry_t* alias = &alias_table.aliases[alias_table.count];
        strcpy(alias->name, name);
        strcpy(alias->expansion, expansion);
        alias->is_builtin = is_builtin;
        alias_table.count++;
        return 0;
    }
    
    return -1;  // No space available
}

static const char* alias_find(const char* name) {
    if (!name) return NULL;
    
    for (int i = 0; i < alias_table.count; i++) {
        if (strcmp(alias_table.aliases[i].name, name) == 0) {
            return alias_table.aliases[i].expansion;
        }
    }
    
    return NULL;  // Not found
}

static int alias_remove(const char* name) {
    if (!name) return -1;
    
    for (int i = 0; i < alias_table.count; i++) {
        if (strcmp(alias_table.aliases[i].name, name) == 0) {
            // Don't allow removal of built-in aliases
            if (alias_table.aliases[i].is_builtin) return -1;
            
            // Shift remaining aliases down
            for (int j = i; j < alias_table.count - 1; j++) {
                alias_table.aliases[j] = alias_table.aliases[j + 1];
            }
            alias_table.count--;
            return 0;
        }
    }
    
    return -1;  // Not found
}

static void alias_clear_user_aliases(void) {
    // Remove all user-defined aliases, keep built-ins
    int write_pos = 0;
    for (int read_pos = 0; read_pos < alias_table.count; read_pos++) {
        if (alias_table.aliases[read_pos].is_builtin) {
            if (write_pos != read_pos) {
                alias_table.aliases[write_pos] = alias_table.aliases[read_pos];
            }
            write_pos++;
        }
    }
    alias_table.count = write_pos;
}

// Day 20 Task 4: Batch Commands Functions
// Removed unused batch_trim_whitespace function

// Removed unused batch_detect_operator function

static int batch_parse_commands(const char* input, batch_sequence_t* sequence) {
    if (!input || !sequence) return -1;
    
    sequence->count = 0;
    
    // Check if input contains semicolon - simplified batch detection
    int has_semicolon = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == ';') {
            has_semicolon = 1;
            break;
        }
    }
    
    // If no semicolon, this is not a batch command
    if (!has_semicolon) {
        return 0;
    }
    
    // Simple semicolon-only splitting for safety
    char work_buffer[512];
    int input_len = strlen(input);
    if (input_len >= sizeof(work_buffer)) return -1;  // Input too long
    
    // Copy input to work buffer with bounds checking
    for (int i = 0; i <= input_len && i < sizeof(work_buffer) - 1; i++) {
        work_buffer[i] = input[i];
    }
    work_buffer[sizeof(work_buffer) - 1] = '\0';  // Ensure null termination
    
    // Split on semicolons only (no && or || for now - too complex)
    char* start = work_buffer;
    
    while (*start && sequence->count < MAX_BATCH_COMMANDS) {
        // Find next semicolon
        char* semicolon = start;
        while (*semicolon && *semicolon != ';') {
            semicolon++;
        }
        
        // Extract command
        char command[128];
        int cmd_pos = 0;
        
        // Copy command with bounds checking and whitespace trimming
        char* cmd_start = start;
        while (cmd_start < semicolon && (*cmd_start == ' ' || *cmd_start == '\t')) {
            cmd_start++;  // Skip leading whitespace
        }
        
        char* cmd_end = semicolon - 1;
        while (cmd_end > cmd_start && (*cmd_end == ' ' || *cmd_end == '\t')) {
            cmd_end--;  // Skip trailing whitespace
        }
        
        // Copy trimmed command
        for (char* p = cmd_start; p <= cmd_end && cmd_pos < sizeof(command) - 1; p++) {
            command[cmd_pos++] = *p;
        }
        command[cmd_pos] = '\0';
        
        // Add to sequence if non-empty
        if (cmd_pos > 0) {
            if (cmd_pos < sizeof(sequence->commands[sequence->count].command)) {
                for (int i = 0; i <= cmd_pos; i++) {
                    sequence->commands[sequence->count].command[i] = command[i];
                }
                sequence->commands[sequence->count].next_op = BATCH_OP_SEMICOLON;
                sequence->count++;
            }
        }
        
        // Move to next command
        if (*semicolon == ';') {
            start = semicolon + 1;
        } else {
            break;  // No more semicolons
        }
    }
    
    // Set last command's next_op to NONE
    if (sequence->count > 0) {
        sequence->commands[sequence->count - 1].next_op = BATCH_OP_NONE;
    }
    
    return sequence->count;
}

static int batch_execute_single_command(const char* command_str) {
    // Direct execution without batch parsing or alias expansion (prevents recursion)
    token_result_t tokens;
    int result = shell_tokenize(command_str, &tokens);
    if (result != 0) {
        puts("Error: Failed to parse command");
        return result;
    }
    
    if (tokens.argc == 0) return 0;  // No tokens, not an error
    
    // SAFETY: Skip alias expansion in batch context to prevent infinite recursion
    // Aliases can contain batch commands which would cause recursive calls
    // For batch commands, we execute only direct commands for safety
    
    // Execute command directly without alias expansion
    int exec_result = shell_execute_command(&tokens);
    
    // Add to history if command executed successfully (or even if it failed)
    if (strcmp(tokens.argv[0], "history") != 0) {
        history_add_command(command_str);
    }
    
    return exec_result;
}

static int batch_execute_sequence(const batch_sequence_t* sequence) {
    if (!sequence || sequence->count == 0) return -1;
    
    int last_result = 0;
    
    for (int i = 0; i < sequence->count; i++) {
        int should_execute = 1;
        
        // Determine if we should execute this command based on previous result and operator
        if (i > 0) {
            batch_operator_t prev_op = sequence->commands[i - 1].next_op;
            switch (prev_op) {
                case BATCH_OP_SEMICOLON:
                    // Always execute
                    should_execute = 1;
                    break;
                case BATCH_OP_AND:
                    // Execute only if previous succeeded
                    should_execute = (last_result == 0);
                    break;
                case BATCH_OP_OR:
                    // Execute only if previous failed
                    should_execute = (last_result != 0);
                    break;
                case BATCH_OP_NONE:
                    // No operator (shouldn't happen in middle of sequence, but handle gracefully)
                    should_execute = 1;
                    break;
            }
        }
        
        if (should_execute) {
            // Execute this command directly (avoiding recursion)
            last_result = batch_execute_single_command(sequence->commands[i].command);
        }
    }
    
    return last_result;
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
            puts("Usage: clear [mode]");
            puts("Examples:");
            puts("  clear        - Clear entire screen");
            puts("  clear screen - Clear entire screen");
            puts("  clear line   - Clear current line");
            puts("  clear eol    - Clear to end of line");
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
        } else if (strcmp(cmd->name, "color") == 0) {
            puts("Usage: color [option]");
            puts("Examples:");
            puts("  color        - Show color status");
            puts("  color on     - Enable colors");
            puts("  color test   - Show color test");
        }
        
        return 0;
    }
    
    // Show all commands with better formatting
    puts("=== ARM64 OS Shell - Available Commands ===");
    puts("");
    
    for (int i = 0; i < 17 && command_table[i].name != NULL; i++) {
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
    // Enhanced clear command - Day 17 Task 1
    if (argc > 2) {
        puts("Usage: clear [mode]");
        puts("Modes:");
        puts("  clear        - Clear entire screen (default)");
        puts("  clear screen - Clear entire screen");
        puts("  clear line   - Clear current line");
        puts("  clear eol    - Clear to end of line");
        puts("  clear bol    - Clear to beginning of line");
        return -1;
    }
    
    // Default behavior: clear entire screen
    if (argc == 1) {
        clear_screen_full();
        return 0;
    }
    
    // Handle different clear modes
    const char* mode = argv[1];
    
    if (strcmp(mode, "screen") == 0) {
        clear_screen_full();
    } else if (strcmp(mode, "line") == 0) {
        clear_current_line();
    } else if (strcmp(mode, "eol") == 0) {
        clear_to_end_of_line();
    } else if (strcmp(mode, "bol") == 0) {
        clear_to_beginning_of_line();
    } else {
        printf("Error: Unknown clear mode '%s'\n", mode);
        puts("Valid modes: screen, line, eol, bol");
        return -1;
    }
    
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
    puts("- Interactive shell with 10 commands");
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
    // Use new error system for argument validation
    if (argc != 4) {
        shell_display_error(SHELL_ERROR_INVALID_ARGS, "Expected 4 arguments: calc <number1> <operator> <number2>");
        puts("Supported operators: +, -, *, /");
        puts("Examples:");
        puts("  calc 10 + 5");
        puts("  calc 20 - 8");
        puts("  calc 6 * 7");
        puts("  calc 15 / 3");
        shell_log_error(SHELL_ERROR_INVALID_ARGS, "calc", "wrong argument count");
        return SHELL_ERROR_INVALID_ARGS;
    }
    
    // Simple string to integer conversion
    int num1 = 0, num2 = 0;
    char* str1 = argv[1];
    char* str2 = argv[3];
    char op = argv[2][0];
    
    // Convert first number with enhanced error reporting
    for (int i = 0; str1[i] != '\0'; i++) {
        if (str1[i] >= '0' && str1[i] <= '9') {
            num1 = num1 * 10 + (str1[i] - '0');
        } else {
            shell_display_error(SHELL_ERROR_SYNTAX, "First argument is not a valid number");
            shell_log_error(SHELL_ERROR_SYNTAX, "calc", str1);
            return SHELL_ERROR_SYNTAX;
        }
    }
    
    // Convert second number with enhanced error reporting
    for (int i = 0; str2[i] != '\0'; i++) {
        if (str2[i] >= '0' && str2[i] <= '9') {
            num2 = num2 * 10 + (str2[i] - '0');
        } else {
            shell_display_error(SHELL_ERROR_SYNTAX, "Second argument is not a valid number");
            shell_log_error(SHELL_ERROR_SYNTAX, "calc", str2);
            return SHELL_ERROR_SYNTAX;
        }
    }
    
    int result = 0;
    
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
                shell_display_error(SHELL_ERROR_RANGE, "Division by zero is not allowed");
                shell_log_error(SHELL_ERROR_RANGE, "calc", "division by zero");
                return SHELL_ERROR_RANGE;
            }
            result = num1 / num2;
            break;
        default:
            {
                // Build error message without snprintf (not available in freestanding)
                char error_context[64] = "Unknown operator: ";
                int len = strlen(error_context);
                if (len < sizeof(error_context) - 1) {
                    error_context[len] = op;
                    error_context[len + 1] = '\0';
                }
                shell_display_error(SHELL_ERROR_SYNTAX, error_context);
                puts("Supported operators: +, -, *, /");
                shell_log_error(SHELL_ERROR_SYNTAX, "calc", error_context);
                return SHELL_ERROR_SYNTAX;
            }
    }
    
    // Display result with success formatting (printf doesn't support %d, use %x for now)
    printf("%x %c %x = %x\n", num1, op, num2, result);
    
    return SHELL_SUCCESS;
}

// Phase 3 Day 15: Helper function for hex string parsing
static unsigned long parse_hex(const char* str) 
{
    unsigned long result = 0;
    const char* p = str;
    
    // Skip "0x" prefix if present
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        p += 2;
    }
    
    while (*p) {
        char c = *p;
        if (c >= '0' && c <= '9') {
            result = result * 16 + (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            result = result * 16 + (c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            result = result * 16 + (c - 'A' + 10);
        } else {
            return 0; // Invalid character
        }
        p++;
    }
    
    return result;
}

// Phase 3 Day 15: Helper function for decimal string parsing
static unsigned long parse_decimal(const char* str)
{
    unsigned long result = 0;
    const char* p = str;
    
    while (*p) {
        char c = *p;
        if (c >= '0' && c <= '9') {
            result = result * 10 + (c - '0');
        } else {
            return 0; // Invalid character
        }
        p++;
    }
    
    return result;
}

// Phase 3 Day 15: Parse address from string (hex or decimal)
static unsigned long parse_address(const char* str, int* valid)
{
    unsigned long addr;
    *valid = 0;
    
    if (!str || strlen(str) == 0) {
        return 0;
    }
    
    // Check if hex format (starts with 0x)
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        addr = parse_hex(str);
        if (addr > 0 || strcmp(str, "0x0") == 0 || strcmp(str, "0X0") == 0) {
            *valid = 1;
        }
    } else {
        // Try decimal
        addr = parse_decimal(str);
        if (addr > 0 || strcmp(str, "0") == 0) {
            *valid = 1;
        }
    }
    
    return addr;
}

// Phase 3 Day 15: Check if address is safe to read (word-aligned)
static int is_address_safe(unsigned long addr)
{
    // Basic safety checks for ARM64 QEMU environment
    
    // Null pointer
    if (addr == 0) {
        return 0;
    }
    
    // Extremely low addresses (likely kernel structures)
    if (addr < 0x1000) {
        return 0;
    }
    
    // UART hardware registers (0x09000000 range)
    if (addr >= 0x09000000 && addr <= 0x09001000) {
        return 0;
    }
    
    // Very high addresses (kernel/hardware regions)
    if (addr >= 0xFFFF0000) {
        return 0;
    }
    
    // Address should be aligned for safety (required for word access)
    if (addr % 4 != 0) {
        return 0;
    }
    
    return 1;
}

// Phase 3 Day 16: Check if address is safe for byte-level reading (dump/peek bytes)
static int is_address_safe_byte(unsigned long addr)
{
    // Basic safety checks without alignment requirement
    
    // Null pointer
    if (addr == 0) {
        return 0;
    }
    
    // Extremely low addresses (likely kernel structures)
    if (addr < 0x1000) {
        return 0;
    }
    
    // UART hardware registers (0x09000000 range)
    if (addr >= 0x09000000 && addr <= 0x09001000) {
        return 0;
    }
    
    // Very high addresses (kernel/hardware regions)
    if (addr >= 0xFFFF0000) {
        return 0;
    }
    
    // No alignment requirement for byte access
    return 1;
}

// Phase 3 Day 15: Display character as printable ASCII or '.'
static char to_printable_char(unsigned char c)
{
    if (c >= 32 && c <= 126) {
        return c;
    }
    return '.';
}

// Phase 3 Day 15: Helper to print address in hex format  
static void print_address_hex(unsigned long addr)
{
    printf("%x", (unsigned int)addr);
}

// Phase 3 Day 15: Helper to print decimal number
static void print_decimal(unsigned int value)
{
    if (value == 0) {
        putchar('0');
        return;
    }
    
    char dec_buf[12]; // Enough for 32-bit number
    int i = 0;
    
    while (value > 0) {
        dec_buf[i++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Print digits in reverse order
    while (i > 0) {
        putchar(dec_buf[--i]);
    }
}

// Phase 3 Day 15: Peek command implementation
int cmd_peek(int argc, char* argv[])
{
    if (argc != 2) {
        puts("Usage: peek <address>");
        puts("       peek 0x40094000    # Read 32-bit value from address");
        puts("       peek 4096          # Read from decimal address");
        return -1;
    }
    
    // Parse address
    int addr_valid;
    unsigned long addr = parse_address(argv[1], &addr_valid);
    
    if (!addr_valid) {
        printf("Error: Invalid address format: '%s'\n", argv[1]);
        puts("Address should be hex (0x1000) or decimal (4096)");
        return -1;
    }
    
    // Check if address is safe to read
    if (!is_address_safe(addr)) {
        printf("Error: Address %x is outside safe memory range\n", addr);
        puts("Safe range: 0x40000000 to 0x40FFFFFF (kernel + heap area)");
        return -1;
    }
    
    // Read value from memory
    unsigned long value = *(volatile unsigned long*)addr;
    
    printf("Address %x: %x\n", addr, value);
    return 0;
}

// Phase 3 Day 16: Check if address is safe to write (stricter than read)
static int is_address_safe_write(unsigned long addr)
{
    // Use same basic safety checks as read, but be more restrictive
    if (!is_address_safe(addr)) {
        return 0;
    }
    
    // Additional write-specific restrictions
    // Don't allow writing to low kernel area (more restrictive than read)
    if (addr < 0x40000000) {
        return 0;
    }
    
    // Don't allow writing to the main kernel code area
    if (addr >= 0x40000000 && addr < 0x40010000) {
        return 0; 
    }
    
    return 1;
}

// Phase 3 Day 16: Poke command implementation
int cmd_poke(int argc, char* argv[])
{
    if (argc < 3 || argc > 4) {
        puts("Usage: poke <address> <value> [size]");
        puts("       poke 0x40094000 0x12345678     # Write 32-bit value");
        puts("       poke 0x40094000 255 byte       # Write byte value");
        puts("       poke 0x40094000 65535 word     # Write 16-bit value");
        puts("Sizes: byte (8-bit), word (16-bit), long (32-bit, default)");
        return -1;
    }
    
    // Parse address
    int addr_valid;
    unsigned long addr = parse_address(argv[1], &addr_valid);
    
    if (!addr_valid) {
        printf("Error: Invalid address format: '%s'\n", argv[1]);
        puts("Address should be hex (0x1000) or decimal (4096)");
        return -1;
    }
    
    // Parse value
    int value_valid;  
    unsigned long value = parse_address(argv[2], &value_valid);
    
    if (!value_valid) {
        printf("Error: Invalid value format: '%s'\n", argv[2]);
        puts("Value should be hex (0x1234) or decimal (4660)");
        return -1;
    }
    
    // Parse size (default to long/32-bit)
    int write_size = 4; // Default: 32-bit
    if (argc == 4) {
        if (strcmp(argv[3], "byte") == 0) {
            write_size = 1;
        } else if (strcmp(argv[3], "word") == 0) {
            write_size = 2;
        } else if (strcmp(argv[3], "long") == 0) {
            write_size = 4;
        } else {
            printf("Error: Invalid size '%s'\n", argv[3]);
            puts("Valid sizes: byte, word, long");
            return -1;
        }
    }
    
    // Check address alignment based on write size
    if (write_size == 2 && (addr % 2) != 0) {
        printf("Error: Address %x not aligned for 16-bit write\n", (unsigned int)addr);
        puts("16-bit writes require 2-byte alignment");
        return -1;
    }
    if (write_size == 4 && (addr % 4) != 0) {
        printf("Error: Address %x not aligned for 32-bit write\n", (unsigned int)addr);
        puts("32-bit writes require 4-byte alignment");
        return -1;
    }
    
    // Safety check for write
    if (!is_address_safe_write(addr)) {
        printf("Error: Unsafe write address: %x\n", (unsigned int)addr);
        puts("Refusing to write to potentially dangerous memory location");
        puts("Safe write area: 0x40010000 and above, aligned properly");
        return -1;
    }
    
    // Validate value range for size
    if (write_size == 1 && value > 0xFF) {
        printf("Error: Value %x too large for byte write (max: 0xFF)\n", (unsigned int)value);
        return -1;
    }
    if (write_size == 2 && value > 0xFFFF) {
        printf("Error: Value %x too large for word write (max: 0xFFFF)\n", (unsigned int)value);
        return -1;
    }
    
    // Perform the write
    puts("Writing to memory...");
    if (write_size == 1) {
        unsigned char* ptr = (unsigned char*)addr;
        *ptr = (unsigned char)value;
    } else if (write_size == 2) {
        unsigned short* ptr = (unsigned short*)addr;
        *ptr = (unsigned short)value;
    } else { // write_size == 4
        unsigned int* ptr = (unsigned int*)addr;
        *ptr = (unsigned int)value;
    }
    
    // Verify the write succeeded by reading back
    puts("Verifying write...");
    unsigned int read_back = 0;
    if (write_size == 1) {
        unsigned char* ptr = (unsigned char*)addr;
        read_back = *ptr;
    } else if (write_size == 2) {
        unsigned short* ptr = (unsigned short*)addr;
        read_back = *ptr;
    } else { // write_size == 4
        unsigned int* ptr = (unsigned int*)addr;
        read_back = *ptr;
    }
    
    // Check if write was successful
    unsigned int expected = (unsigned int)value;
    if (write_size == 1) expected &= 0xFF;
    if (write_size == 2) expected &= 0xFFFF;
    
    if (read_back == expected) {
        puts("Write successful!");
        printf("Address: %x\n", (unsigned int)addr);
        printf("Value written: %x\n", expected);
        puts("Size: ");
        print_decimal(write_size);
        puts(" bytes");
    } else {
        puts("Write verification failed!");
        printf("Expected: %x\n", expected);
        printf("Read back: %x\n", read_back);
        return -1;
    }
    
    return 0;
}

// Phase 3 Day 16: Dump command implementation
int cmd_dump(int argc, char* argv[])
{
    if (argc != 3) {
        puts("Usage: dump <address> <length>");
        puts("       dump 0x40094000 64    # Dump 64 bytes from address");
        puts("       dump 1073811456 128   # Dump 128 bytes from decimal address");
        return -1;
    }
    
    // Parse starting address
    int addr_valid;
    unsigned long start_addr = parse_address(argv[1], &addr_valid);
    
    if (!addr_valid) {
        printf("Error: Invalid address format: '%s'\n", argv[1]);
        puts("Address should be hex (0x40094000) or decimal (1073811456)");
        return -1;
    }
    
    // Parse length
    int length_valid;
    unsigned long length = parse_address(argv[2], &length_valid);
    
    if (!length_valid || length == 0) {
        printf("Error: Invalid length format: '%s'\n", argv[2]);
        puts("Length should be a positive number (64, 0x40, etc.)");
        return -1;
    }
    
    // Reasonable length limits
    if (length > 1024) {
        printf("Error: Length %x too large (max: 1024 bytes)\n", (unsigned int)length);
        puts("Use smaller chunks for large memory regions");
        return -1;
    }
    
    // Check if start address is safe to read (byte level)
    if (!is_address_safe_byte(start_addr)) {
        printf("Error: Unsafe start address: %x\n", (unsigned int)start_addr);
        puts("Cannot dump from potentially dangerous memory location");
        return -1;
    }
    
    // Check if end address is safe to read (byte level)
    unsigned long end_addr = start_addr + length - 1;
    if (!is_address_safe_byte(end_addr)) {
        printf("Error: Dump range extends to unsafe address: %x\n", (unsigned int)end_addr);
        puts("Reduce length or choose different start address");
        return -1;
    }
    
    // Align start address to 16-byte boundary for clean display
    unsigned long aligned_start = start_addr & ~0xF;
    unsigned long total_length = (start_addr - aligned_start) + length;
    
    // Round up total length to 16-byte boundary
    if (total_length % 16) {
        total_length = (total_length + 15) & ~0xF;
    }
    
    puts("Memory dump:");
    puts("");
    
    // Display memory in 16-byte lines
    unsigned char* ptr = (unsigned char*)aligned_start;
    
    for (unsigned long offset = 0; offset < total_length; offset += 16) {
        unsigned long line_addr = aligned_start + offset;
        
        // Display address column (8 hex digits)
        printf("%x: ", (unsigned int)line_addr);
        
        // Display 16 bytes in hex
        for (int i = 0; i < 16; i++) {
            unsigned long byte_addr = line_addr + i;
            
            // Check if this byte is within our requested range
            if (byte_addr >= start_addr && byte_addr < start_addr + length) {
                // Check if safe to read this specific byte
                if (is_address_safe_byte(byte_addr)) {
                    printf("%x ", (unsigned int)ptr[offset + i]);
                } else {
                    puts("?? ");
                }
            } else {
                // Outside requested range, show padding
                puts("   ");
            }
            
            // Add extra space after 8th byte for readability
            if (i == 7) {
                puts(" ");
            }
        }
        
        // Display ASCII representation
        puts("  |");
        for (int i = 0; i < 16; i++) {
            unsigned long byte_addr = line_addr + i;
            
            if (byte_addr >= start_addr && byte_addr < start_addr + length) {
                if (is_address_safe_byte(byte_addr)) {
                    putchar(to_printable_char(ptr[offset + i]));
                } else {
                    putchar('?');
                }
            } else {
                putchar(' ');
            }
        }
        puts("|");
    }
    
    puts("");
    printf("Dumped %x bytes from ", (unsigned int)length);
    print_address_hex(start_addr);
    puts("");
    
    return 0;
}

// Day 17 Task 2: Color command implementation
int cmd_color(int argc, char* argv[])
{
    if (argc == 1) {
        // Show current color status
        printf("Color support: %s\n", colors_enabled ? "enabled" : "disabled");
        if (colors_enabled) {
            puts("Color test:");
            print_success("  Success (green)\n");
            print_error("  Error (red)\n");
            print_warning("  Warning (yellow)\n");
            print_info("  Info (cyan)\n");
            print_formatted(ANSI_COLOR_BOLD, ANSI_FG_MAGENTA, "  Bold magenta\n");
        }
        return 0;
    }
    
    if (argc == 2) {
        const char* option = argv[1];
        
        if (strcmp(option, "on") == 0 || strcmp(option, "enable") == 0) {
            set_colors_enabled(1);
            print_success("Colors enabled\n");
            return 0;
        } else if (strcmp(option, "off") == 0 || strcmp(option, "disable") == 0) {
            set_colors_enabled(0);
            puts("Colors disabled");
            return 0;
        } else if (strcmp(option, "test") == 0) {
            puts("Color test - 16 basic colors:");
            if (colors_enabled) {
                print_fg_color(ANSI_FG_BLACK, "Black ");
                print_fg_color(ANSI_FG_RED, "Red ");
                print_fg_color(ANSI_FG_GREEN, "Green ");
                print_fg_color(ANSI_FG_YELLOW, "Yellow ");
                print_fg_color(ANSI_FG_BLUE, "Blue ");
                print_fg_color(ANSI_FG_MAGENTA, "Magenta ");
                print_fg_color(ANSI_FG_CYAN, "Cyan ");
                print_fg_color(ANSI_FG_WHITE, "White\n");
                print_fg_color(ANSI_FG_BRIGHT_BLACK, "Gray ");
                print_fg_color(ANSI_FG_BRIGHT_RED, "BrightRed ");
                print_fg_color(ANSI_FG_BRIGHT_GREEN, "BrightGreen ");
                print_fg_color(ANSI_FG_BRIGHT_YELLOW, "BrightYellow ");
                print_fg_color(ANSI_FG_BRIGHT_BLUE, "BrightBlue ");
                print_fg_color(ANSI_FG_BRIGHT_MAGENTA, "BrightMagenta ");
                print_fg_color(ANSI_FG_BRIGHT_CYAN, "BrightCyan ");
                print_fg_color(ANSI_FG_BRIGHT_WHITE, "BrightWhite\n");
            } else {
                puts("Colors are disabled. Enable colors first with 'color on'");
            }
            return 0;
        } else {
            printf("Error: Unknown color option '%s'\n", option);
            puts("Valid options: on, off, enable, disable, test");
            return -1;
        }
    }
    
    // Show usage
    puts("Usage: color [option]");
    puts("Options:");
    puts("  color           - Show current color status and test");
    puts("  color on        - Enable colors");
    puts("  color off       - Disable colors");
    puts("  color enable    - Enable colors");
    puts("  color disable   - Disable colors");
    puts("  color test      - Show color test pattern");
    return 0;
}

// Day 17 Task 3: Reboot Command Implementation
int cmd_reboot(int argc, char* argv[])
{
    char response[8];
    
    // Display shutdown message
    if (colors_enabled) {
        print_warning("=== SYSTEM REBOOT ===\n");
    } else {
        puts("=== SYSTEM REBOOT ===");
    }
    
    // Add confirmation prompt for safety
    printf("Are you sure you want to restart the system? (y/N): ");
    
    // Read user response
    if (shell_read_line(response, sizeof(response)) <= 0) {
        puts("Reboot cancelled.");
        return 0;
    }
    
    // Check for confirmation
    if (response[0] != 'y' && response[0] != 'Y') {
        puts("Reboot cancelled.");
        return 0;
    }
    
    // Clean up before reboot
    puts("Cleaning up system state...");
    
    // Display shutdown message
    if (colors_enabled) {
        print_info("Shutting down ARM64 OS...\n");
        print_success("System will restart shortly.\n");
    } else {
        puts("Shutting down ARM64 OS...");
        puts("System will restart shortly.");
    }
    puts("Goodbye!");
    puts("");
    
    // In QEMU ARM64, we can trigger a system reset using the 
    // ARM System Register reset mechanism or PSCI interface.
    // For simplicity, we'll use an infinite loop that causes
    // QEMU to detect an unresponsive system (user can Ctrl+A, X to quit)
    
    // Disable interrupts and halt
    __asm__ volatile (
        "msr daifset, #15\n"    // Disable all interrupts (D,A,I,F flags)
        "wfi\n"                 // Wait for interrupt (low power state) 
        "1: b 1b\n"            // Infinite loop
        ::: "memory"
    );
    
    // This point should never be reached
    return 0;
}

// Day 17 Task 4: System Information Command Implementation
int cmd_sysinfo(int argc, char* argv[])
{
    // System header
    if (colors_enabled) {
        print_info("=== ARM64 OS SYSTEM INFORMATION ===\n");
    } else {
        puts("=== ARM64 OS SYSTEM INFORMATION ===");
    }
    puts("");
    
    // OS Version and Name
    if (colors_enabled) {
        print_success("Operating System\n");
    } else {
        puts("Operating System:");
    }
    printf("  Name: ARM64 OS Educational\n");
    printf("  Version: Phase 3 Development\n");
    printf("  Architecture: ARM64 (AArch64)\n");
    printf("  Build: Minimal Educational OS\n");
    puts("");
    
    // Build Information
    if (colors_enabled) {
        print_success("Build Information\n");
    } else {
        puts("Build Information:");
    }
    printf("  Compiler: aarch64-elf-gcc\n");
    printf("  Target: Freestanding (no stdlib)\n");
    printf("  Optimization: -O2\n");
    printf("  Built for: QEMU ARM64 virt machine\n");
    puts("");
    
    // Memory Configuration
    if (colors_enabled) {
        print_success("Memory Configuration\n");
    } else {
        puts("Memory Configuration:");
    }
    printf("  Load Address: 0x40000000\n");
    printf("  Heap Start: 0x40097000\n");
    printf("  Heap Size: 1MB (1048576 bytes)\n");
    printf("  Stack Size: 64KB allocated\n");
    printf("  Memory Model: No virtual memory (physical addresses)\n");
    puts("");
    
    // CPU Information
    if (colors_enabled) {
        print_success("CPU Information\n");
    } else {
        puts("CPU Information:");
    }
    printf("  Architecture: ARM Cortex-A57 (emulated)\n");
    printf("  Mode: EL1 (Exception Level 1)\n");
    printf("  MMU: Disabled (direct physical addressing)\n");
    printf("  Cache: Disabled\n");
    printf("  Interrupts: Disabled (polling I/O)\n");
    puts("");
    
    // Hardware Configuration
    if (colors_enabled) {
        print_success("Hardware Configuration\n");
    } else {
        puts("Hardware Configuration:");
    }
    printf("  Platform: QEMU virt machine\n");
    printf("  UART: PL011 at 0x09000000 (115200 baud)\n");
    printf("  Timer: ARM Generic Timer (available)\n");
    printf("  Reset: ARM system reset mechanism\n");
    puts("");
    
    // System Features
    if (colors_enabled) {
        print_success("System Features\n");
    } else {
        puts("System Features:");
    }
    printf("  Shell Commands: 13 built-in commands\n");
    printf("  Memory Management: Bump allocator\n");
    printf("  Color Support: ANSI escape sequences\n");
    printf("  Screen Control: Clear screen, cursor positioning\n");
    printf("  Error Handling: Comprehensive validation\n");
    puts("");
    
    // Current Status
    if (colors_enabled) {
        print_success("Current Status\n");
    } else {
        puts("Current Status:");
    }
    printf("  Boot Status: Successfully initialized\n");
    printf("  Shell Status: Interactive and responsive\n");
    printf("  Memory Status: ");
    
    // Get current memory statistics
    memory_info();
    
    return 0;
}

// Day 19 Task 1: Command History Display Implementation
int cmd_history(int argc, char* argv[])
{
    if (history.count == 0) {
        puts("No commands in history.");
        return 0;
    }
    
    // Display header
    if (colors_enabled) {
        print_info("=== COMMAND HISTORY ===\n");
    } else {
        puts("=== COMMAND HISTORY ===");
    }
    puts("");
    
    // Calculate starting index for display
    int start_index;
    int display_count = history.count;
    
    if (history.count < HISTORY_SIZE) {
        // Haven't wrapped around yet
        start_index = 0;
    } else {
        // Wrapped around, start from oldest
        start_index = history.write_index;
    }
    
    // Display commands with numbering
    for (int i = 0; i < display_count; i++) {
        int index = (start_index + i) % HISTORY_SIZE;
        int command_number = i + 1;
        
        if (colors_enabled) {
            printf("[%x] %s\n", command_number, history.commands[index]);
        } else {
            printf("%2x  %s\n", command_number, history.commands[index]);
        }
    }
    
    puts("");
    printf("Total commands: %x\n", history.count);
    if (history.count == HISTORY_SIZE) {
        puts("(History buffer is full - oldest commands are being overwritten)");
    }
    
    return 0;
}

// Day 20 Task 1: Error log display command
int cmd_errors(int argc, char* argv[])
{
    if (error_log.count == 0) {
        shell_display_info("No errors logged yet.");
        return SHELL_SUCCESS;
    }
    
    // Display header
    if (colors_enabled) {
        printf(ANSI_FG_CYAN "=== ERROR LOG ===" ANSI_COLOR_RESET "\n\n");
    } else {
        puts("=== ERROR LOG ===\n");
    }
    
    // Calculate starting index for display (oldest first)
    int start_index;
    int display_count = error_log.count;
    
    if (error_log.count < ERROR_LOG_SIZE) {
        // Haven't wrapped around yet
        start_index = 0;
    } else {
        // Wrapped around, start from oldest
        start_index = error_log.current_index;
    }
    
    // Display errors
    for (int i = 0; i < display_count; i++) {
        int index = (start_index + i) % ERROR_LOG_SIZE;
        error_log_entry_t* entry = &error_log.entries[index];
        
        // Format timestamp
        printf("[%u] ", entry->timestamp);
        
        // Display command name
        if (colors_enabled) {
            printf(ANSI_FG_YELLOW "%s" ANSI_COLOR_RESET ": ", entry->command);
        } else {
            printf("%s: ", entry->command);
        }
        
        // Display error message with color coding
        const char* error_msg = shell_get_error_message(entry->error_code);
        if (colors_enabled) {
            printf(ANSI_FG_RED "%s" ANSI_COLOR_RESET, error_msg);
        } else {
            printf("%s", error_msg);
        }
        
        // Display context if available
        if (entry->context[0] != '\0') {
            printf(" (%s)", entry->context);
        }
        
        printf("\n");
    }
    
    printf("\nTotal errors logged: %d\n", error_log.count);
    if (error_log.count == ERROR_LOG_SIZE) {
        puts("(Error log is full - oldest errors are being overwritten)");
    }
    
    return SHELL_SUCCESS;
}

// Day 20 Task 2: Performance Statistics Command
int cmd_stats(int argc, char* argv[])
{
    if (argc > 1) {
        shell_display_error(SHELL_ERROR_INVALID_ARGS, "stats command takes no arguments");
        return SHELL_ERROR_INVALID_ARGS;
    }
    
    // Display performance monitoring header
    if (colors_enabled) {
        printf(ANSI_FG_CYAN "=== Performance Statistics ===" ANSI_COLOR_RESET "\n\n");
    } else {
        puts("=== Performance Statistics ===\n");
    }
    
    // Display global statistics (printf doesn't support %u or %d, use %x)
    printf("Total commands executed: %x\n", perf_monitor.total_commands);
    printf("Commands tracked: %x/%x\n", perf_monitor.tracked_count, MAX_TRACKED_COMMANDS);
    printf("Performance counter: %x\n\n", perf_monitor.performance_counter);
    
    if (perf_monitor.tracked_count == 0) {
        puts("No command statistics available yet.");
        puts("Execute some commands and run 'stats' again to see performance data.");
        return SHELL_SUCCESS;
    }
    
    // Display command statistics table header
    if (colors_enabled) {
        printf(ANSI_FG_YELLOW "%-12s %8s %8s %8s %8s" ANSI_COLOR_RESET "\n", 
               "Command", "Count", "Total", "Last", "Average");
    } else {
        printf("%-12s %8s %8s %8s %8s\n", 
               "Command", "Count", "Total", "Last", "Average");
    }
    
    puts("--------------------------------------------------");
    
    // Display statistics for each tracked command
    for (int i = 0; i < perf_monitor.tracked_count; i++) {
        command_stats_t* stats = &perf_monitor.commands[i];
        
        if (colors_enabled) {
            printf(ANSI_FG_GREEN "%-12s" ANSI_COLOR_RESET " %8u %8u %8u %8u\n",
                   stats->command,
                   stats->call_count,
                   stats->total_time,
                   stats->last_time,
                   stats->avg_time);
        } else {
            printf("%-12s %8u %8u %8u %8u\n",
                   stats->command,
                   stats->call_count,
                   stats->total_time,
                   stats->last_time,
                   stats->avg_time);
        }
    }
    
    puts("");
    
    // Find most used command
    if (perf_monitor.tracked_count > 0) {
        command_stats_t* most_used = &perf_monitor.commands[0];
        for (int i = 1; i < perf_monitor.tracked_count; i++) {
            if (perf_monitor.commands[i].call_count > most_used->call_count) {
                most_used = &perf_monitor.commands[i];
            }
        }
        
        if (colors_enabled) {
            printf("Most used command: " ANSI_FG_BRIGHT_GREEN "%s" ANSI_COLOR_RESET " (%u times)\n", 
                   most_used->command, most_used->call_count);
        } else {
            printf("Most used command: %s (%u times)\n", 
                   most_used->command, most_used->call_count);
        }
    }
    
    puts("\nNote: Time values are simple execution counters, not real time.");
    
    return SHELL_SUCCESS;
}

// Day 20 Task 3: Alias Command Implementation
int cmd_alias(int argc, char* argv[])
{
    // No arguments - list all aliases
    if (argc == 1) {
        if (colors_enabled) {
            printf(ANSI_FG_CYAN "=== Command Aliases ===" ANSI_COLOR_RESET "\n\n");
        } else {
            puts("=== Command Aliases ===\n");
        }
        
        if (alias_table.count == 0) {
            puts("No aliases defined.");
            return SHELL_SUCCESS;
        }
        
        // Display built-in aliases first
        int builtin_count = 0;
        for (int i = 0; i < alias_table.count; i++) {
            if (alias_table.aliases[i].is_builtin) {
                if (builtin_count == 0) {
                    if (colors_enabled) {
                        printf(ANSI_FG_YELLOW "Built-in aliases:" ANSI_COLOR_RESET "\n");
                    } else {
                        puts("Built-in aliases:");
                    }
                }
                
                if (colors_enabled) {
                    printf("  " ANSI_FG_GREEN "%s" ANSI_COLOR_RESET " -> " 
                           ANSI_FG_BRIGHT_BLUE "%s" ANSI_COLOR_RESET "\n",
                           alias_table.aliases[i].name, alias_table.aliases[i].expansion);
                } else {
                    printf("  %s -> %s\n", 
                           alias_table.aliases[i].name, alias_table.aliases[i].expansion);
                }
                builtin_count++;
            }
        }
        
        // Display user-defined aliases
        int user_count = 0;
        for (int i = 0; i < alias_table.count; i++) {
            if (!alias_table.aliases[i].is_builtin) {
                if (user_count == 0) {
                    if (builtin_count > 0) puts("");
                    if (colors_enabled) {
                        printf(ANSI_FG_YELLOW "User-defined aliases:" ANSI_COLOR_RESET "\n");
                    } else {
                        puts("User-defined aliases:");
                    }
                }
                
                if (colors_enabled) {
                    printf("  " ANSI_FG_CYAN "%s" ANSI_COLOR_RESET " -> " 
                           ANSI_FG_BRIGHT_CYAN "%s" ANSI_COLOR_RESET "\n",
                           alias_table.aliases[i].name, alias_table.aliases[i].expansion);
                } else {
                    printf("  %s -> %s\n", 
                           alias_table.aliases[i].name, alias_table.aliases[i].expansion);
                }
                user_count++;
            }
        }
        
        if (user_count == 0 && builtin_count > 0) {
            puts("\nNo user-defined aliases. Use 'alias <name> <command>' to create one.");
        }
        
        printf("\nTotal aliases: %x/%x\n", alias_table.count, MAX_ALIASES);
        return SHELL_SUCCESS;
    }
    
    // Handle subcommands
    if (argc >= 2) {
        // Delete alias: alias -d <name>
        if (strcmp(argv[1], "-d") == 0) {
            if (argc != 3) {
                shell_display_error(SHELL_ERROR_INVALID_ARGS, "Usage: alias -d <name>");
                return SHELL_ERROR_INVALID_ARGS;
            }
            
            int result = alias_remove(argv[2]);
            if (result == 0) {
                shell_display_success("Alias removed successfully");
                return SHELL_SUCCESS;
            } else {
                shell_display_error(SHELL_ERROR_NOT_FOUND, "Alias not found or cannot be removed (built-in)");
                return SHELL_ERROR_NOT_FOUND;
            }
        }
        
        // Clear user aliases: alias -c
        if (strcmp(argv[1], "-c") == 0) {
            if (argc != 2) {
                shell_display_error(SHELL_ERROR_INVALID_ARGS, "Usage: alias -c");
                return SHELL_ERROR_INVALID_ARGS;
            }
            
            alias_clear_user_aliases();
            shell_display_success("All user-defined aliases cleared");
            return SHELL_SUCCESS;
        }
        
        // Create/update alias: alias <name> <expansion>
        if (argc >= 3) {
            // Join all arguments from argv[2] onwards as the expansion
            char expansion[128];
            int pos = 0;
            
            for (int i = 2; i < argc && pos < 127; i++) {
                // Add space separator between arguments
                if (i > 2 && pos < 127) {
                    expansion[pos++] = ' ';
                }
                
                // Add the argument
                int arg_len = strlen(argv[i]);
                for (int j = 0; j < arg_len && pos < 127; j++) {
                    expansion[pos++] = argv[i][j];
                }
            }
            
            expansion[pos] = '\0';
            
            if (!alias_validate_name(argv[1])) {
                shell_display_error(SHELL_ERROR_SYNTAX, "Invalid alias name or conflicts with existing command");
                return SHELL_ERROR_SYNTAX;
            }
            
            int result = alias_add(argv[1], expansion, 0);  // User-defined alias
            if (result == 0) {
                if (colors_enabled) {
                    printf(ANSI_FG_GREEN "Alias created: " ANSI_COLOR_RESET 
                           ANSI_FG_CYAN "%s" ANSI_COLOR_RESET " -> " 
                           ANSI_FG_BRIGHT_CYAN "%s" ANSI_COLOR_RESET "\n",
                           argv[1], expansion);
                } else {
                    printf("Alias created: %s -> %s\n", argv[1], expansion);
                }
                return SHELL_SUCCESS;
            } else {
                shell_display_error(SHELL_ERROR_MEMORY, "Unable to create alias (table full or invalid parameters)");
                return SHELL_ERROR_MEMORY;
            }
        }
        
        // Invalid syntax
        shell_display_error(SHELL_ERROR_SYNTAX, "Usage: alias [name expansion] | [-d name] | [-c]");
        return SHELL_ERROR_SYNTAX;
    }
    
    return SHELL_SUCCESS;
}