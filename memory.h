/**
 * Gedare Bloom
 * memory.h
 *
 * Definitions for the memory.
 */

#include <stdint.h>

extern uint32_t instruction_memory[1024]; /* 0x400000 - 0x401000 */
extern uint32_t data_memory[1024]; /* 0x10000000 - 0x10001000 */
extern uint32_t stack_memory[1024]; /* 0x20000000 - 0x20001000 */

//Struct for Instruction Cache
typedef struct
{
	int validBit;
	uint32_t word1;
	uint32_t tag1;

	uint32_t word2;
	uint32_t tag2;

	uint32_t word3;
	uint32_t tag3;

	uint32_t word4;
	uint32_t tag4;
}block;

typedef struct
{
	uint32_t validBit;
	uint32_t lruTimer;
	uint32_t dirtyBit;
	uint32_t tag;
	uint32_t wordContent;
	uint32_t offset;
}AssociativeWord;


typedef struct
{
	AssociativeWord words[4];
}AssociativeBlock;


extern block L1_instruction_cache[128]; /* 2 KiB = 128 * 128bits */

extern AssociativeBlock L1_data_cache[128]; /* 2 KiB = 128 sets * 4*32bit addresses */


AssociativeWord convertMemoryAddresstoSetComponents(AssociativeWord passedWord, uint32_t memAddress);
uint32_t convertSetComponentsToMemoryAddress(AssociativeWord passedWord, uint32_t setNumber);
uint32_t findDataCacheValue(uint32_t memAddress, int *isFound);
void addToSetCache(uint32_t memAddress, uint32_t memValue);
void evictWord(uint32_t memAddress);
uint32_t dataCacheReadMiss(uint32_t memAddress);
uint32_t dataCacheReadHit(uint32_t memAddress);
void dataCacheWriteHit(uint32_t memAddress, uint32_t updatedValue);
void dataCacheWriteMiss(uint32_t memAddress, uint32_t valueToStore);