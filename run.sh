#!/bin/bash

# QEMU run script for ARM64 OS
# Configure for ARM64 virt machine with serial console

KERNEL_IMG="build/kernel.img"

# Check if kernel image exists
if [ ! -f "$KERNEL_IMG" ]; then
    echo "Error: Kernel image not found at $KERNEL_IMG"
    echo "Run 'make' to build the kernel first"
    exit 1
fi

# QEMU command
echo "Starting ARM64 OS in QEMU..."
echo "Use Ctrl+A, X to quit QEMU"
echo

qemu-system-aarch64 \
    -machine virt \
    -cpu cortex-a57 \
    -kernel "$KERNEL_IMG" \
    -m 128M \
    -nographic \
    -serial mon:stdio