#ifndef COMPILER_H
#define COMPILER_H

// Handle different compilers
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
// ARM Compiler 6 (ARMCLANG)
#define __packed __attribute__((packed))
#elif defined(__GNUC__)
// GCC
#define __packed __attribute__((packed))
#elif defined(__CC_ARM)
// ARMCC 5
#define __packed __packed
#else
#error "Unsupported compiler"
#endif

#endif // COMPILER_H