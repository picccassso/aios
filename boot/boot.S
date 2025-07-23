/*
 * ARM64 OS Boot Code
 * Phase 1: Minimal boot with ARM64 header
 */

.section .text.boot
.global _start

/*
 * ARM64 Linux boot header (64 bytes)
 * Required for QEMU and real hardware
 */
_start:
    b       boot_entry          // Branch to boot code
    .long   0                   // Reserved
    .quad   0x80000             // Image load offset (512KB)
    .quad   _end - _start       // Image size  
    .quad   0x0                 // Flags
    .quad   0                   // Reserved
    .quad   0                   // Reserved
    .quad   0                   // Reserved
    .ascii  "ARM\x64"           // Magic number
    .long   0                   // Reserved

/*
 * Boot entry point
 * CPU initialization and setup
 */
boot_entry:
    // Check current exception level
    mrs     x0, CurrentEL
    lsr     x0, x0, #2          // Extract EL bits
    cmp     x0, #1              // Are we at EL1?
    b.eq    el1_setup           // Yes, continue
    
    // If at EL2, drop to EL1
    cmp     x0, #2              // Are we at EL2?
    b.ne    hang                // Unexpected EL, hang
    
    // Drop from EL2 to EL1
    mov     x0, #0x3c5          // EL1h mode with interrupts disabled
    msr     spsr_el2, x0
    adr     x0, el1_setup
    msr     elr_el2, x0
    eret                        // Exception return to EL1

el1_setup:
    // Disable MMU and caches
    mrs     x0, sctlr_el1
    bic     x0, x0, #1          // Disable MMU (M bit)
    bic     x0, x0, #4          // Disable D-cache (C bit)  
    bic     x0, x0, #0x1000     // Disable I-cache (I bit)
    msr     sctlr_el1, x0
    
    // Clear interrupt masks
    msr     daifclr, #0xf       // Enable all interrupts for now
    
    // Set up stack pointer
    // Stack grows downward from high memory
    ldr     x0, =stack_top
    mov     sp, x0
    
    // Clear BSS section
    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
    sub     x1, x1, x0          // Calculate BSS size
    cbz     x1, bss_cleared     // Skip if no BSS
    
clear_bss:
    str     xzr, [x0], #8       // Clear 8 bytes and increment
    subs    x1, x1, #8
    b.gt    clear_bss

bss_cleared:
    // Jump to C main function
    bl      main
    
    // If main returns, hang
hang:
    wfe                         // Wait for event (low power)
    b       hang

/*
 * Stack space (64KB)
 * Placed at end of image
 */
.section .bss
.align 4                        // 16-byte alignment
stack_bottom:
.space 0x10000                  // 64KB stack
stack_top:
