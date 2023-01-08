#pragma once

// Adds assertions that stack operations never cause
// the stack pointer to overflow or underflow. This
// is useful for ensuring the stack usage is valid.
#define ASSUME_NO_STACK_OVERFLOW true
// Adds assertions that all memory operations in page 1
// are inside of the active stack area, since anything outside
// of it could be overwritten by an interrupt.
#define ASSUME_VALID_STACK_USAGE true
// Adds assertions that the arguments to ADC and SBC are valid
// BCD when the decimal flag is set.
#define ASSUME_VALID_BCD true
// Adds assertions that any references to TEMP only access inside
// of TEMP, even when using ,x and ,y.
#define ASSUME_VALID_TEMP_USAGE true
#define TEMP_SIZE 16
// When using the hash machine, the number of reads and writes
// to record
#define NUM_ADDRESSES_TO_REMEMBER 8

// Set this to PROCESSOR_2a03, PROCESSOR_65c02, or PROCESSOR_NMOS_6502
#define PROCESSOR_2a03


#ifdef PROCESSOR_2a03
// The processor on the NES is an NMOS 6502,
// but the decimal mode is cut for patent reasons.

#define DECIMAL_ENABLED        false
#define HAS_JUMP_INDIRECT_BUG  true
#define USE_65c02_DECIMAL      false
#define FASTER_RMW             false

#elif defined(PROCESSOR_65c02)
// The 65c02 family of processors includes
// extra instructions and fixes some bugs.

#define DECIMAL_ENABLED        true
#define HAS_JUMP_INDIRECT_BUG  false
#define USE_65c02_DECIMAL      true
#define FASTER_RMW             true

#elif defined(PROCESSOR_NMOS_6502)
// The classic 6502

#define DECIMAL_ENABLED        true
#define HAS_JUMP_INDIRECT_BUG  true
#define USE_65c02_DECIMAL      false
#define FASTER_RMW             false

#endif
