/*
 * Shell command structure and framework
 * Phase 2 Day 10: Command processing foundation
 */

#ifndef SHELL_H
#define SHELL_H

#define MAX_ARGS 16
#define MAX_INPUT_SIZE 128
#define MAX_TOKEN_SIZE 32

// Command handler function pointer type
// Takes argc (argument count) and argv (argument array)
// Returns 0 on success, non-zero on error
typedef int (*command_handler_t)(int argc, char* argv[]);

// Command structure for command table
typedef struct {
    char* name;                 // Command name (e.g., "help", "echo")
    char* description;          // Brief description for help
    command_handler_t handler;  // Function pointer to command implementation
} shell_command_t;

// Tokenization result structure
typedef struct {
    int argc;                   // Number of tokens found
    char* argv[MAX_ARGS];       // Array of token pointers (non-const for command handlers)
    char tokens[MAX_ARGS][MAX_TOKEN_SIZE];  // Storage for tokens
} token_result_t;

// Shell function declarations
void shell_init(void);
int shell_read_line(char* buffer, int max_size);
int shell_tokenize(const char* input, token_result_t* result);
shell_command_t* shell_find_command(const char* name);
int shell_execute_command(const token_result_t* tokens);
int shell_parse_and_execute(const char* input);

// Command implementations
int cmd_help(int argc, char* argv[]);
int cmd_echo(int argc, char* argv[]);
int cmd_clear(int argc, char* argv[]);
int cmd_meminfo(int argc, char* argv[]);
int cmd_about(int argc, char* argv[]);
int cmd_uptime(int argc, char* argv[]);
int cmd_calc(int argc, char* argv[]);

#endif // SHELL_H