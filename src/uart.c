/*
 * PL011 UART Driver
 * Phase 1: Serial output implementation
 */

// UART base address for QEMU virt machine
#define UART_BASE    0x09000000

// PL011 UART register offsets
#define UARTDR       0x000  // Data register
#define UARTFR       0x018  // Flag register
#define UARTIBRD     0x024  // Integer baud rate divisor
#define UARTFBRD     0x028  // Fractional baud rate divisor
#define UARTLCR_H    0x02C  // Line control register
#define UARTCR       0x030  // Control register

// Flag register bits
#define UART_FR_TXFF (1 << 5)  // Transmit FIFO full
#define UART_FR_RXFE (1 << 4)  // Receive FIFO empty

// Control register bits
#define UART_CR_UARTEN (1 << 0)  // UART enable
#define UART_CR_TXE    (1 << 8)  // Transmit enable
#define UART_CR_RXE    (1 << 9)  // Receive enable

// Line control register bits (8N1 configuration)
#define UART_LCR_H_WLEN_8 (3 << 5)  // 8 data bits
#define UART_LCR_H_FEN    (1 << 4)   // Enable FIFOs

// Memory-mapped I/O functions
static inline void mmio_write(unsigned long addr, unsigned int value)
{
    *(volatile unsigned int*)addr = value;
}

static inline unsigned int mmio_read(unsigned long addr)
{
    return *(volatile unsigned int*)addr;
}

/*
 * Initialize PL011 UART
 * Configure for 115200 baud, 8N1
 */
void uart_init(void)
{
    // Disable UART during configuration
    mmio_write(UART_BASE + UARTCR, 0);
    
    // Set baud rate to 115200
    // Assuming 24MHz UART clock: divisor = 24000000 / (16 * 115200) = 13.02
    // Integer part = 13, fractional part = 0.02 * 64 = 1.28 ≈ 1
    mmio_write(UART_BASE + UARTIBRD, 13);
    mmio_write(UART_BASE + UARTFBRD, 1);
    
    // Configure line control: 8 data bits, no parity, 1 stop bit, FIFOs enabled
    mmio_write(UART_BASE + UARTLCR_H, UART_LCR_H_WLEN_8 | UART_LCR_H_FEN);
    
    // Enable UART, transmitter, and receiver
    mmio_write(UART_BASE + UARTCR, UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE);
}

/*
 * Send a single character
 * Wait if transmit FIFO is full (polling)
 */
void putchar(char c)
{
    // Wait while transmit FIFO is full
    while (mmio_read(UART_BASE + UARTFR) & UART_FR_TXFF) {
        // Polling - do nothing
    }
    
    // Send character
    mmio_write(UART_BASE + UARTDR, c);
    
    // Handle newline: send carriage return too
    if (c == '\n') {
        while (mmio_read(UART_BASE + UARTFR) & UART_FR_TXFF) {
            // Wait again
        }
        mmio_write(UART_BASE + UARTDR, '\r');
    }
}

/*
 * Send a string
 * Loop through string until null terminator, add automatic newline
 */
void puts(const char* str)
{
    // Send each character until null terminator
    while (*str) {
        putchar(*str);
        str++;
    }
    
    // Add automatic newline
    putchar('\n');
}

/*
 * Helper function: convert number to hex string
 */
static void put_hex(unsigned long value)
{
    char hex_chars[] = "0123456789abcdef";
    char buffer[17]; // 16 hex digits + null terminator
    int i = 15;
    int start_pos;
    
    buffer[16] = '\0';
    
    // Handle zero case
    if (value == 0) {
        putchar('0');
        putchar('x');
        putchar('0');
        return;
    }
    
    // Convert to hex digits (reverse order)
    while (value > 0 && i >= 0) {
        buffer[i] = hex_chars[value & 0xf];
        value >>= 4;
        i--;
    }
    
    // Record where the actual hex digits start
    start_pos = i + 1;
    
    // Print 0x prefix
    putchar('0');
    putchar('x');
    
    // Print only the actual hex digits (not uninitialized buffer)
    for (int j = start_pos; j < 16; j++) {
        putchar(buffer[j]);
    }
}

/*
 * Simple printf implementation
 * Supports %s (string) and %x (hexadecimal)
 */
void printf(const char* format, ...)
{
    // Simple variable arguments using builtin functions
    __builtin_va_list args;
    __builtin_va_start(args, format);
    
    const char* p = format;
    
    while (*p) {
        if (*p == '%' && *(p + 1)) {
            p++; // Skip '%'
            
            switch (*p) {
                case 's': {
                    const char* str = __builtin_va_arg(args, const char*);
                    if (str) {
                        while (*str) {
                            putchar(*str);
                            str++;
                        }
                    } else {
                        putchar('(');
                        putchar('n');
                        putchar('u');
                        putchar('l');
                        putchar('l');
                        putchar(')');
                    }
                    break;
                }
                case 'x': {
                    unsigned long value = __builtin_va_arg(args, unsigned long);
                    put_hex(value);
                    break;
                }
                case '%':
                    putchar('%');
                    break;
                default:
                    // Unknown format - just print it
                    putchar('%');
                    putchar(*p);
                    break;
            }
        } else {
            putchar(*p);
        }
        p++;
    }
    
    __builtin_va_end(args);
}

/*
 * Receive a single character
 * Wait if receive FIFO is empty (polling)
 */
char getchar(void)
{
    // Wait while receive FIFO is empty
    while (mmio_read(UART_BASE + UARTFR) & UART_FR_RXFE) {
        // Polling - do nothing
    }
    
    // Read character from data register
    return (char)mmio_read(UART_BASE + UARTDR);
}

/*
 * Receive a line of input
 * Read characters until newline, store in buffer with null terminator
 */
void gets(char* buffer, int max_size)
{
    int i = 0;
    char c;
    
    while (i < max_size - 1) {  // Leave space for null terminator
        c = getchar();
        
        if (c == '\r' || c == '\n') {
            // End of line
            break;
        } else if (c == '\b' || c == 0x7F) {
            // Backspace handling
            if (i > 0) {
                i--;
                // Visual feedback: backspace, space, backspace
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        } else if (c >= 32 && c <= 126) {
            // Printable character
            buffer[i] = c;
            putchar(c);  // Echo character
            i++;
        }
        // Ignore other non-printable characters
    }
    
    buffer[i] = '\0';  // Add null terminator
    putchar('\n');     // Move to next line
}