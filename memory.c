/**
 * Gedare Bloom
 * memory.c
 *
 * Implementation of the memory.
 */

#include "memory.h"

uint32_t instruction_memory[1024];
uint32_t data_memory[2048];
uint32_t stack_memory[1024];

block L1_instruction_cache[128];

AssociativeBlock L1_data_cache[128];


