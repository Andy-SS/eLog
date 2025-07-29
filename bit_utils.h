#ifndef BIT_UTILS_H
#define BIT_UTILS_H

/* Utility macros for bit manipulation */
#ifndef SET_BIT
#define SET_BIT(reg, bit)   ((reg) |= (1 << (bit)))  // Set a specific bit in a register
#endif

#ifndef CLEAR_BIT
#define CLEAR_BIT(reg, bit) ((reg) &= ~(1 << (bit))) // Clear a specific bit in a register
#endif

#ifndef TOGGLE_BIT
#define TOGGLE_BIT(reg, bit) ((reg) ^= (1 << (bit))) // Toggle a specific bit in a register
#endif

#ifndef READ_BIT
#define READ_BIT(reg, bit)  ((reg) & (1 << (bit)))   // Read a specific bit from a register
#endif

#ifndef BIT
#define BIT(x) (1 << (x)) // Create a bit mask
#endif

#endif /* BIT_UTILS_H */
