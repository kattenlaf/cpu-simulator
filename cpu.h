#pragma once
/**
* Ruel Gordon
* Matthew King
* LeAnn Lewis
* Gedare Bloom
* cpu.h
*
* Definitions for the processor.
*/

#include <stdint.h> 
#include <inttypes.h>
#include <stdio.h>
#define ENABLE_L1_CACHES

struct cpu_context {
	uint32_t PC;
	uint32_t GPR[32];
};

extern struct cpu_context cpu_ctx;

struct IF_ID_buffer {
	uint32_t instruction;
	uint32_t next_pc;
};


//start of ID component

typedef struct
{
	uint32_t ALUSrc;
	uint32_t RegDst;
}ExecuteSignals;

typedef struct
{
	//uint32_t Branch;
	uint32_t MemRead;
	uint32_t MemWrite;
}MemorySignals;

typedef struct
{
	uint32_t MemtoReg;
	uint32_t RegWrite;
}WriteBackSignals;

typedef struct
{
	uint32_t Branch;
	uint32_t Jump;
}DecodeSignals;
typedef struct
{
	DecodeSignals dePhase;
	ExecuteSignals exPhase;
	MemorySignals memPhase;
	WriteBackSignals writeBackPhase;
}CUSignalType;

typedef struct
{
	CUSignalType signals;
	uint32_t read1Cont;
	uint32_t read2Cont;
	uint32_t opCode;
	uint32_t funct;
	uint32_t aluOP;
	uint32_t extendedImmediate;
	uint32_t jumpInstruction;
	uint32_t writeAddressRType;
	uint32_t writeAddressIType;
}ID_EX_buffer;

typedef struct
{
	uint32_t opCode;
	uint32_t read1Address;
	uint32_t read2Address;
	uint32_t writeAddressRType;
	uint32_t writeAddressIType;
	uint32_t shamt;
	uint32_t funct;
	uint16_t immediate;
	uint32_t immediate_26;
}InstructionComponents;

//end of ID component

struct EX_MEM_buffer {
	CUSignalType signals;
	uint32_t address;
	uint32_t writeData;
	uint32_t writeAddressRType;
	uint32_t writeAddressIType;
};

struct MEM_WB_buffer {
	CUSignalType signals;
	uint32_t memdata;
	uint32_t ALUresult;
	uint32_t writeAddressRType;
	uint32_t writeAddressIType;
};

typedef struct {
	int stallCount;
	int stallAmount;
	int instructionCacheHit;
	int instructionCacheMiss;
	int dataCacheHit;
	int dataCacheMiss;
	int totalInstructions;

} data_metric_handling;

// Variables to use
data_metric_handling data_metrics;

/* Function prototypes
** ------------------------
 */

int fetch_with_cache(struct IF_ID_buffer *out);
int fetch_no_cache(struct IF_ID_buffer *out);
int decode(struct IF_ID_buffer *in, ID_EX_buffer *out);
int execute(ID_EX_buffer *in, struct EX_MEM_buffer *out);
int memory_with_cache(struct EX_MEM_buffer *in, struct MEM_WB_buffer *out);
int memory_no_cache(struct EX_MEM_buffer *in, struct MEM_WB_buffer *out);
int writeback(struct MEM_WB_buffer *in);
InstructionComponents separateComponents(uint32_t instruction);
uint32_t generateAluOp(uint32_t opCode, uint32_t funct);
CUSignalType generateControlSignals(uint32_t opCode);
ID_EX_buffer updateReadValues(InstructionComponents passedComponents, ID_EX_buffer outBuffer, uint32_t GPR[]);
uint32_t signExtendValue(uint16_t passedValue);
uint32_t rw_memory_with_cache(uint32_t ALUresult, uint32_t data2, uint32_t MemWrite, uint32_t MemRead, uint32_t *memdata, uint32_t Mem[]);
uint32_t rw_memory_no_cache(uint32_t ALUresult, uint32_t data2, uint32_t MemWrite, uint32_t MemRead, uint32_t *memdata, uint32_t Mem[]);
void write_register(uint32_t r2, uint32_t r3, uint32_t memdata, uint32_t ALUresult, uint32_t RegWrite, uint32_t RegDst, uint32_t MemtoReg, uint32_t *Reg);
uint32_t ALU(ID_EX_buffer *in, uint32_t Reg2);
uint32_t Adder(ID_EX_buffer *in, uint32_t Reg2);
uint32_t Multiplexor(ID_EX_buffer *in);
uint32_t PC_Counter(uint32_t PC);
uint32_t changeToDecimal(char passedVal[], int valBase);
int modifyCharacter(char passedCharacter);
int convertPCToInstructionIndex(int givenPCValue);
int convertMemoryAddressToMemoryIndex(uint32_t passedAddress);
int syscall(ID_EX_buffer *ID_EX, struct cpu_context CPU);
int jump(struct IF_ID_buffer *in, InstructionComponents Instructions, ID_EX_buffer* out);
int jumpAndLink(struct IF_ID_buffer *in, InstructionComponents Instructions, ID_EX_buffer* out);
int jumpRegister(struct IF_ID_buffer *in, ID_EX_buffer* out);
uint32_t setAllSignalsToZero(ID_EX_buffer* ID_EX);
uint32_t findByteOffset(uint32_t address);
uint32_t findIndexbits(uint32_t address);
uint32_t findTag(uint32_t address);
uint32_t findBlockNumber(uint32_t address);
uint32_t findWhichWord(uint32_t address);
void placeBlock(uint32_t address);
int doesInstructionExist(uint32_t address);
uint32_t getInstruction(uint32_t address);
void printBlock(uint32_t blockNum);
void initializeDataCache();
void initializeInstructionCache();
uint32_t findSetNumber(uint32_t address);
void evictWord(uint32_t memAddress);
/* 
* Functions for quality of life printing
*/
void formatInstructionTelemetry(InstructionComponents instruction);