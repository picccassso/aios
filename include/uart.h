/*
 * UART Function Declarations
 * Phase 1: Serial output
 */

#ifndef UART_H
#define UART_H

void uart_init(void);
void putchar(char c);
void puts(const char* str);
void printf(const char* format, ...);
char getchar(void);
void gets(char* buffer, int max_size);

#endif