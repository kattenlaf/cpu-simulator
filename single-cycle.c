/**
* Gedare Bloom
* single-cycle.c
*
* Drives the simulation of a single-cycle processor
*/

#include "cpu.h"
#include "memory.h"
#include "syscall.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#pragma warning (disable : 4996)
#define DEBUG

enum ASCIICONSTANTSI { AUVALUE = 65, BUVALUE, CUVALUE, DUVALUE, EUVALUE, FUVALUE, GUVALUE, HUVALUE, IUVALUE, JUVALUE, KUVALUE, LUVALUE, MUVALUE, NUVALUE, OUVALUE, PUVALUE, QUVALUE, RUVALUE, SUVALUE, TUVALUE, UUVALUE, VUVALUE, WUVALUE, XUVALUE, YUVALUE, ZUVALUE };

enum ASCIICONSTANTSII { ALVALUE = 97, BLVALUE, CLVALUE, DLVALUE, ELVALUE, FLVALUE, GLVALUE, HLVALUE, ILVALUE, JLVALUE, KLVALUE, LLVALUE, MLVALUE, NLVALUE, OLVALUE, PLVALUE, QLVALUE, RLVALUE, SLVALUE, TLVALUE, ULVALUE, VLVALUE, WLVALUE, XLVALUE, YLVALUE, ZLVALUE };

enum ASCIICONSTANTSIII { TABVALUE = 9, APOSTROPHEVALUE = 39, OPENBRACKETVALUE, CLOSEDBRACKETVALUE, ASTERISKVALUE, PLUSVALUE, COMMAVALUE, DASHVALUE, PERIODVALUE, BACKSLASHVALUE };

enum ASCIICONSTANTSIV { COLONVALUE = 58, SEMICOLONVALUE, GREATERTHANVALUE, EQUALVALUE, LESSTHANVALUE, QUESTIONMARKVALUE, ATVALUE };

enum ASCIICONSTANTSV { ACVALUE = 1, BCVALUE, CCVALUE, DCVALUE, ECVALUE, FCVALUE, GCVALUE, HCVALUE, ICVALUE, JCVALUE, KCVALUE, LCVALUE, MCVALUE, NCVALUE, OCVALUE, PCVALUE, QCVALUE, RCVALUE, SCVALUE, TCVALUE, UCVALUE, VCVALUE, WCVALUE, XCVALUE, YCVALUE, ZCVALUE };

enum ASCIINAVIGATIONI { BACKSPACE = 8, ENTER = 13, LCHARLIMIT = 32, UCHARLIMIT = 128 };

enum ASCIINAVIGATIONII { ARROW = 224, LEFT = 75, RIGHT = 77, UP = 72, DOWN = 80, SLEFT = 115, SRIGHT = 116, SUP = 141, SDOWN = 145 };

int main(int argc, char *argv[])
{
	data_metrics.stallCount = 0;
	data_metrics.stallAmount = 0;
	data_metrics.instructionCacheHit = 0;
	data_metrics.instructionCacheMiss = 0;
	data_metrics.dataCacheHit = 0;
	data_metrics.dataCacheMiss = 0;

	FILE *loadManager;
	struct IF_ID_buffer if_id;
	ID_EX_buffer id_ex;
	struct EX_MEM_buffer ex_mem;
	struct MEM_WB_buffer mem_wb;
	int i;
	char lowerHalfWord[5];
	char fullWord[9];

	initializeDataCache();
	initializeInstructionCache();

	/* Initialize registers and memory to 0 */
	cpu_ctx.PC = 0x20000000; // 0x20000000
	for (i = 0; i < 32; i++) {
		cpu_ctx.GPR[i] = 0;
	}

	for (i = 0; i < 1024; i++) {
		instruction_memory[i] = 0;
		stack_memory[i] = 0;
	}
	for (i = 0; i < 1024; i++)
	{
		data_memory[i] = 0;
	}


	/* Read memory from the input file */
	loadManager = fopen("program.sim", "r");

	for (i = 0; i < 1024; i++) {
		fscanf(loadManager, "%s", fullWord);
		fscanf(loadManager, "%s", lowerHalfWord);
		strcat(fullWord, lowerHalfWord);
		instruction_memory[i] = changeToDecimal(fullWord, 16);


#if defined(DEBUG)
		if (instruction_memory[i] != 0) {
			printf("%X\n", instruction_memory[i]);
		}
#endif
	}

	for (i = 0; i < 1024; i++) {
		fscanf(loadManager, "%s", fullWord);
		fscanf(loadManager, "%s", lowerHalfWord);
		strcat(fullWord, lowerHalfWord);
		data_memory[i] = changeToDecimal(fullWord, 16);


#if defined(DEBUG)
		if (data_memory[i] != 0) {
			printf("%X\n", data_memory[i]);
		}
#endif
	}

	fclose(loadManager);

	cpu_ctx.GPR[29] = 0x20000000; //set sp to base of stack

	while (1) {
#if defined(DEBUG)
		printf("FETCH from PC=%X\n", cpu_ctx.PC);
#endif

#if defined(ENABLE_L1_CACHES)
		if (data_metrics.stallAmount > 0)
		{
			data_metrics.stallCount++;
			data_metrics.stallAmount--;
			continue;
		}
		fetch_with_cache(&if_id);
		if (if_id.instruction == 0) {
			break;
		}
		printf("INSTRUCTION: %X\n", if_id.instruction);
		decode(&if_id, &id_ex);
		execute(&id_ex, &ex_mem);
		memory_with_cache(&ex_mem, &mem_wb);
		writeback(&mem_wb);
		cpu_ctx.PC = if_id.next_pc;
}
	printf("-------------------------\nInstructions Executed: %d\n------------------\n", data_metrics.totalInstructions);
	printf("Instruction Cache:\n------------------\n");
	printf("Cache Accesses: %d\n", (data_metrics.instructionCacheHit + data_metrics.instructionCacheMiss));
	printf("Cache Hits: %d\n", data_metrics.instructionCacheHit);
	printf("Cache Misses: %d\n-----------\n", data_metrics.instructionCacheMiss);
	printf("Data Cache:\n-----------\n");
	printf("Cache Accesses: %d\n", (data_metrics.dataCacheHit + data_metrics.dataCacheMiss));
	printf("Cache Hits: %d\n", data_metrics.dataCacheHit);
	printf("Cache Misses: %d\n", data_metrics.dataCacheMiss);

	printf("CPI: %f\n", ((float)(data_metrics.instructionCacheHit + data_metrics.stallCount) / (float)data_metrics.totalInstructions));
	// getchar();
	return 0;
#else
		fetch_no_cache(&if_id);
		if (if_id.instruction == 0) break;
		decode(&if_id, &id_ex);
		execute(&id_ex, &ex_mem);
		memory_no_cache(&ex_mem, &mem_wb);
		writeback(&mem_wb);
		cpu_ctx.PC = if_id.next_pc;
#endif
	return 0;
}

uint32_t signExtendValue(uint16_t passedValue)
{
	uint32_t extendedValue;
	extendedValue = passedValue;
	if ((passedValue - 32768) >= 0) //if lestmost bit is 1
	{
		extendedValue += 4294901760; //make upper half all 1s to make value still negative
	}
	return extendedValue;
}

/***************************************************************************************************************/

InstructionComponents separateComponents(uint32_t instruction)
{
	InstructionComponents currentInstruction;

	currentInstruction.opCode = (instruction & 4227858432) >> 26; //11111100000000000000000000000000 & Isolate then shift the opCode
	currentInstruction.read1Address = (instruction & 65011712) >> 21; //00000011111000000000000000000000 & Isolate then shift the readAddress1
	currentInstruction.read2Address = (instruction & 2031616) >> 16; //00000000000111110000000000000000 & Isolate then shift the readAddress2
	currentInstruction.writeAddressRType = (instruction & 63488) >> 11; //00000000000000001111100000000000 & Isolate then shift the R Type writeAddress
	currentInstruction.writeAddressIType = currentInstruction.read2Address;
	currentInstruction.shamt = (instruction & 1984) >> 6; //00000000000000000000011111000000 & Isolate then shift the shamt
	currentInstruction.funct = (instruction & 63); //00000000000000000000000000111111 & Isolate the funct
	currentInstruction.immediate = (instruction & 65535); //00000000000000001111111111111111 & Isolate the 16-bit immediate
	currentInstruction.immediate_26 = (instruction & 67108863); //00000011111111111111111111111111 & Isolate the last 26-bit immediate
	return currentInstruction;
}

uint32_t generateAluOp(uint32_t opCode, uint32_t funct)
{
	switch (opCode)
	{
	case 0: //R-Type
		switch (funct)
		{
		case 32: //100000 - ADD
			return 2; //0010
			break;

		case 36: //100100 - AND
			return 0; //0000
			break;

		case 37: //100101 - OR
			return 1; //0001
			break;

		case 42: //101010 - SLT
			return 7; //0111
			break;

		case 0: //000000 - SLL
			return 6; //0110 NEED TO DOUBLE CHECK
			break;

		case 2: //000010 - SRL
			return 6; //0110 NEED TO DOUBLE CHECK
			break;

		case 34: //100010 - SUB
			return 6; //0110
			break;

		case 3: //000011 - SRA
			return 6; //0110 NEED TO DOUBLE CHECK
			break;

		case 38: //100110 - XOR
			return 6; //UNKNOWN FOR NOW NEED TO DOUBLE CHECK
			break;

		default:
			return 0;
			break;
		}
		break;


	case 8: //001000 - ADDI
		return 2; //0010
		break;

	case 12: //001100 - ANDI
		return 0; //0000
		break;

	case 4: //000100 - BEQ
		return 6; //0110 
		break;

	case 5: //000101 - BNE
		return 6; //0100 UNKNOWN FOR NOW NEED TO DOUBLE CHECK
		break;

	case 15: //001111 - LUI
		return 6; //0100 UNKNOWN FOR NOW NEED TO DOUBLE CHECK
		break;

	case 35: //100011 - LW
		return 2; //0010
		break;

	case 13: //001101 - ORI
		return 1; //0001 
		break;

	case 10: //001010 - SLTI
		return 7; //0111 
		break;

	case 43: //101011 - SW
		return 2; //0010
		break;

	case 14: //001110 - XORI
		return 6; //0110 NEED TO DOUBLE CHECK
		break;

	case 2: //000010 - J
		return 6; //0110 NEED TO DOUBLE CHECK
		break;

	case 3: //000011 - JAL
		return 6; //0110 NEED TO DOUBLE CHECK
		break;

	default:
		return 0;
		break;
	}
}

CUSignalType generateControlSignals(uint32_t opCode)
{
	CUSignalType signals;

	signals.exPhase.ALUSrc = 1;
	signals.exPhase.RegDst = 0;
	signals.memPhase.MemRead = 0;
	signals.memPhase.MemWrite = 0;
	signals.writeBackPhase.MemtoReg = 0;
	signals.writeBackPhase.RegWrite = 1; //temporary
	signals.dePhase.Branch = 0;
	signals.dePhase.Jump = 0;

	switch (opCode)
	{
	case 0: //R-Type
		signals.exPhase.ALUSrc = 0;
		signals.exPhase.RegDst = 1;
		break;

	case 43: //101011 - SW
		signals.memPhase.MemWrite = 1;
		signals.writeBackPhase.RegWrite = 0; //temporary
		break;

	case 35: //100011 - LWe
		signals.writeBackPhase.MemtoReg = 1;
		signals.memPhase.MemRead = 1;
		signals.writeBackPhase.RegWrite = 1; //temporary
		break;

	case 4: //000100 - BEQ
		signals.dePhase.Branch = 1;
		signals.writeBackPhase.RegWrite = 0; //temporary
		break;

	case 2: //000010 - J
		signals.dePhase.Jump = 1;
		break;

	case 3: //000011 - JAL
		signals.dePhase.Jump = 1;
		break;
	}

	return signals;
}

ID_EX_buffer updateReadValues(InstructionComponents passedComponents, ID_EX_buffer outBuffer, uint32_t GPR[])
{
	outBuffer.read1Cont = GPR[passedComponents.read1Address];

	outBuffer.read2Cont = GPR[passedComponents.read2Address];
	return outBuffer;
}

/***********************ALU FUNCTIONS*************************/


uint32_t ALU(ID_EX_buffer *in, uint32_t Reg2)
{
	/* ADD */
	if (in->aluOP == 2)
	{
		return (in->read1Cont + Reg2);
	}
	/* SUB */
	if (in->aluOP == 6)
	{
		return (in->read1Cont - Reg2);
	}
	/* AND */
	if (in->aluOP == 0)
	{
		return (in->read1Cont & Reg2);
	}
	/* OR */
	if (in->aluOP == 1)
	{
		return (in->read1Cont | Reg2);
	}
	/* Set on Less Than */
	if (in->aluOP == 7)
	{
		if (in->read1Cont < Reg2)
		{
			return 1;
		}
		return 0;
	}
	else
	{
		return 0;
	}
}

uint32_t Adder(ID_EX_buffer *in, uint32_t Reg2)
{
	return (in->read1Cont + Reg2);
}

uint32_t Multiplexor(ID_EX_buffer *in)
{
	if (in->signals.exPhase.ALUSrc == 1)
	{
		return in->extendedImmediate;
	}
	return in->read2Cont;
}

uint32_t PC_Counter(uint32_t PC)
{
	return (PC + 4);
}

/*************************************************************/

uint32_t rw_memory_with_cache(uint32_t ALUresult, uint32_t data2, uint32_t MemWrite, uint32_t MemRead, uint32_t *memdata, uint32_t Mem[])
{
	int isFound = 1;
	if ((MemWrite == 1 || MemRead == 1) && ALUresult % 4 != 0) // If address is bad, then return a halt condition
	{
		//Memory call out of range
		return 1;
	}
	//checks if MemWrite is 1. If it is, it sets memory of ALUresult to data2
	if (MemWrite == 1)
	{
		findDataCacheValue(ALUresult, &isFound);
		if (isFound == 1)
		{
			data_metrics.dataCacheHit++;
			dataCacheWriteHit(ALUresult, data2);
		}
		else
		{
			data_metrics.stallAmount++;
			data_metrics.dataCacheMiss++;
			dataCacheWriteMiss(ALUresult, data2);
		}
		//convertMemoryAddressToMemoryIndex(ALUresult);
		//Mem[convertMemoryAddressToMemoryIndex(ALUresult)] = data2;
	}
	//checks if MemRead is 1. If it is, it sets the memory data to memory of ALUresult shifted 2-bits
	if (MemRead == 1)
	{
		findDataCacheValue(ALUresult, &isFound);
		if (isFound == 1)
		{
			*memdata = dataCacheReadHit(ALUresult);
		}
		else
		{
			*memdata = dataCacheReadMiss(ALUresult);
		}
		*memdata = Mem[convertMemoryAddressToMemoryIndex(ALUresult)];
	}

	return 0;
}

uint32_t rw_memory_no_cache(uint32_t ALUresult, uint32_t data2, uint32_t MemWrite, uint32_t MemRead, uint32_t *memdata, uint32_t Mem[])
{
	if ((MemWrite == 1 || MemRead == 1) && ALUresult % 4 != 0) // If address is bad, then return a halt condition
	{
		//Memory call out of range
		return 1;
	}
	//checks if MemWrite is 1. If it is, it sets memory of ALUresult to data2
	if (MemWrite == 1)
	{
		convertMemoryAddressToMemoryIndex(ALUresult);
		Mem[convertMemoryAddressToMemoryIndex(ALUresult)] = data2;
	}

	//checks if MemRead is 1. If it is, it sets the memory data to memory of ALUresult shifted 2-bits
	if (MemRead == 1)
	{
		*memdata = Mem[convertMemoryAddressToMemoryIndex(ALUresult)];
	}
	return 0;

}

//Write Register
void write_register(uint32_t r2, uint32_t r3, uint32_t memdata, uint32_t ALUresult, uint32_t RegWrite, uint32_t RegDst, uint32_t MemtoReg, uint32_t *Reg)
{
	if (RegWrite == 1)
	{
		if (MemtoReg == 1)
		{
			if (RegDst == 1)
				Reg[r3] = memdata; //Write memdata to rd
			else
				Reg[r2] = memdata; // Write memdata to rt
		}
		else
		{
			if (RegDst == 1)
				Reg[r3] = ALUresult; // Write ALU results to rd
			else
				Reg[r2] = ALUresult; // Write ALU results to rt
		}
	}
}

uint32_t changeToDecimal(char passedVal[], int valBase)
{
	uint32_t returnValue;
	int multiplier;
	returnValue = 0;

	for (int generalCycler = 0; generalCycler < (int) strlen(passedVal); generalCycler++)
	{
		multiplier = (int) pow(16, (strlen(passedVal) - generalCycler - 1));
		returnValue += (modifyCharacter(passedVal[generalCycler]) * multiplier);
	}
	return returnValue;
}

int modifyCharacter(char passedCharacter)
{
	if (passedCharacter > 57) //if greater than '9'
	{
		if (passedCharacter > FUVALUE) //if greater than 'F'
		{
			if (passedCharacter > FLVALUE) //if greater than 'f'
			{
				return -1; //error case
			}
			else
			{
				return (passedCharacter - ALVALUE + 10); //take away 'A' then add 10
			}
		}
		return (passedCharacter - AUVALUE + 10); //take away 'A' then add 10
	}
	else
	{
		return passedCharacter - 48;
	}
}

int convertPCToInstructionIndex(int givenPCValue)
{
	int index = (givenPCValue - 536870912) / 4; //instruction starts at 0x20000000
	return index;
}

int convertMemoryAddressToMemoryIndex(uint32_t passedAddress)
{
	int index = (passedAddress - 0x10000000) / 4; //mem starts at 0x10000000
	return index;
}

int determineTag(uint32_t memoryAddress) //Li_D
{
	//DIRECT MAPPED TAG HAS TO BE MEMORY ADDRESS MODULUS LENGTH OF CACHE
	//SLIDE 17 OF PART TWO
	int tag = memoryAddress % 512;
	return tag;
}

int jump(struct IF_ID_buffer *in, InstructionComponents Instructions, ID_EX_buffer* out)
{
	uint32_t last28bits = Instructions.immediate_26 << 2; //Shifts immediate two to the left
	uint32_t first4bits = (in->next_pc & 4026531840); // PC and 11110000000000000000000000000000
	uint32_t newAddress = (last28bits | first4bits);
	in->next_pc = newAddress;
	setAllSignalsToZero(out);
	return 0;
}

int jumpAndLink(struct IF_ID_buffer *in, InstructionComponents Instructions, ID_EX_buffer* out)
{
	cpu_ctx.GPR[31] = in->next_pc + 4;
	jump(in, Instructions, out);
	return 0;
}

int jumpRegister(struct IF_ID_buffer* in, ID_EX_buffer* out)
{
	if (cpu_ctx.GPR[31] != 0)
	{
		in->next_pc = cpu_ctx.GPR[31];
		setAllSignalsToZero(out);
	}
	cpu_ctx.GPR[31] = 0;
	return 0;
}

//CACHE STUFF

uint32_t findByteOffset(uint32_t address)
{
	return (address & 15); // number and 000000*001111 since last four bits are byte offset
}

uint32_t findIndexbits(uint32_t address)
{
	return (address & 2032); // number and 0000*11111110000 since index bits are 7
}

uint32_t findTag(uint32_t address)
{
	return (address & 4294965248); // number and 11111111111111111111100000000000 since tag bits are first 21
}

uint32_t findBlockNumber(uint32_t address)
{
	uint32_t blockNum = 0;
	blockNum = ((address - 536870912) / 16) % 128; //Subtract where PC starts, divide by block size and mod size of block to get block number
	return blockNum;
}

uint32_t findWhichWord(uint32_t address)
{
	uint32_t wordNum = 0;
	wordNum = address % 4;
	return wordNum;
}

uint32_t findSetNumber(uint32_t address)
{
	return ((address & 2032) >> 4);
}

// PLACE ONE FULL BLOCK AT A TIME, that is 4 instructions
//METADATA BITS = tag bits + 1 valid bit

void placeBlock(uint32_t address)
{
	uint32_t blockNum = findBlockNumber(address);
	switch (findWhichWord(address >> 2)) //LAST TWO BITS ARE ALWAYS 00, so shift to find where to place instruction
	{
	case(0):
		L1_instruction_cache[blockNum].word1 = instruction_memory[convertPCToInstructionIndex(address)];
		L1_instruction_cache[blockNum].word2 = instruction_memory[convertPCToInstructionIndex(address + 4)];
		L1_instruction_cache[blockNum].word3 = instruction_memory[convertPCToInstructionIndex(address + 8)];
		L1_instruction_cache[blockNum].word4 = instruction_memory[convertPCToInstructionIndex(address + 12)];
		//SET TAGS
		L1_instruction_cache[blockNum].tag1 = findTag(address);
		L1_instruction_cache[blockNum].tag2 = findTag(address + 4);
		L1_instruction_cache[blockNum].tag3 = findTag(address + 8);
		L1_instruction_cache[blockNum].tag4 = findTag(address + 12);
		break;
	case(1):
		L1_instruction_cache[blockNum].word1 = instruction_memory[convertPCToInstructionIndex(address - 4)];
		L1_instruction_cache[blockNum].word2 = instruction_memory[convertPCToInstructionIndex(address)];
		L1_instruction_cache[blockNum].word3 = instruction_memory[convertPCToInstructionIndex(address + 4)];
		L1_instruction_cache[blockNum].word4 = instruction_memory[convertPCToInstructionIndex(address + 8)];
		//SET TAGS
		L1_instruction_cache[blockNum].tag1 = findTag(address - 4);
		L1_instruction_cache[blockNum].tag2 = findTag(address);
		L1_instruction_cache[blockNum].tag3 = findTag(address + 4);
		L1_instruction_cache[blockNum].tag4 = findTag(address + 8);
		break;
	case(2):
		L1_instruction_cache[blockNum].word1 = instruction_memory[convertPCToInstructionIndex(address - 8)];
		L1_instruction_cache[blockNum].word2 = instruction_memory[convertPCToInstructionIndex(address - 4)];
		L1_instruction_cache[blockNum].word3 = instruction_memory[convertPCToInstructionIndex(address)];
		L1_instruction_cache[blockNum].word4 = instruction_memory[convertPCToInstructionIndex(address + 4)];
		//SET TAGS
		L1_instruction_cache[blockNum].tag1 = findTag(address - 8);
		L1_instruction_cache[blockNum].tag2 = findTag(address - 4);
		L1_instruction_cache[blockNum].tag3 = findTag(address);
		L1_instruction_cache[blockNum].tag4 = findTag(address + 4);
		break;
	case(3):
		L1_instruction_cache[blockNum].word1 = instruction_memory[convertPCToInstructionIndex(address - 12)];
		L1_instruction_cache[blockNum].word2 = instruction_memory[convertPCToInstructionIndex(address - 8)];
		L1_instruction_cache[blockNum].word3 = instruction_memory[convertPCToInstructionIndex(address - 4)];
		L1_instruction_cache[blockNum].word4 = instruction_memory[convertPCToInstructionIndex(address)];
		//SET TAGS
		L1_instruction_cache[blockNum].tag1 = findTag(address - 12);
		L1_instruction_cache[blockNum].tag2 = findTag(address - 8);
		L1_instruction_cache[blockNum].tag3 = findTag(address - 4);
		L1_instruction_cache[blockNum].tag4 = findTag(address);
		break;
	}
	L1_instruction_cache[blockNum].validBit = 1;
	//printBlock(blockNum);
}

void printBlock(uint32_t blockNum)
{
	printf("Instruction One %u\n", L1_instruction_cache[blockNum].word1);
	printf("Instruction Two %u\n", L1_instruction_cache[blockNum].word2);
	printf("Instruction Three %u\n", L1_instruction_cache[blockNum].word3);
	printf("Instruction Four %u\n", L1_instruction_cache[blockNum].word4);
}

int doesInstructionExist(uint32_t address)
{
	uint32_t blockNum = findBlockNumber(address);
	if (L1_instruction_cache[blockNum].validBit == 1)
	{
		//CHECK IF FIRST 21 bits and last 2 bits if they are the same
		//Instead of checking the instruction themselves are similar, check the tag of the address stored with tag of the address
		switch (findWhichWord(address >> 2))
		{
		case(0):
			if (findTag(address) == L1_instruction_cache[blockNum].tag1)
			{
				return 1;
				break;
			}
		case(1):
			if (findTag(address) == L1_instruction_cache[blockNum].tag2)
			{
				return 1;
				break;
			}
		case(2):
			if (findTag(address) == L1_instruction_cache[blockNum].tag3)
			{
				return 1;
				break;
			}
		case(3):
			if (findTag(address) == L1_instruction_cache[blockNum].tag4)
			{
				return 1;
				break;
			}
		}
	}
	return 0;
}

uint32_t getInstruction(uint32_t address)
{
	uint32_t blockNum = findBlockNumber(address);
	switch (findWhichWord(address >> 2))
	{
	case 0:
		if ( findTag(address) == L1_instruction_cache[blockNum].tag1 )
		{
			return L1_instruction_cache[blockNum].word1;
			break;
		}
	case(1):
		if (findTag(address) == L1_instruction_cache[blockNum].tag2)
		{
			return L1_instruction_cache[blockNum].word2;
			break;
		}
	case(2):
		if (findTag(address) == L1_instruction_cache[blockNum].tag3)
		{
			return L1_instruction_cache[blockNum].word3;
			break;
		}
	case(3):
		if (findTag(address) == L1_instruction_cache[blockNum].tag4)
		{
			return L1_instruction_cache[blockNum].word4;
			break;
		}
	default:
		return 0; //case of error
		break;
	}
}

void initializeDataCache()
{
	for (int setCycler = 0; setCycler < 1; setCycler++)
	{
		//lru timers are set with word1 being the first to be replaced
		for (int wordCycler = 0; wordCycler < 4; wordCycler++)
		{
			//all dirty bits are set to invalid
			L1_data_cache[setCycler].words[wordCycler].dirtyBit = 0;
			//all valid bits are set to invalid
			L1_data_cache[setCycler].words[wordCycler].validBit = 0;

			switch (wordCycler)
			{
			case 0:
				L1_data_cache[setCycler].words[wordCycler].lruTimer = wordCycler;
				break;
			case 1:
				L1_data_cache[setCycler].words[wordCycler].lruTimer = wordCycler;
				break;
			case 2:
				L1_data_cache[setCycler].words[wordCycler].lruTimer = wordCycler;
				break;
			case 3:
				L1_data_cache[setCycler].words[wordCycler].lruTimer = wordCycler;
				break;

			default: //error case
				break;
			}
		}
	}
}

void initializeInstructionCache()
{
	for (int lineCycler = 0; lineCycler < 128; lineCycler++)
	{
		L1_instruction_cache[lineCycler].validBit = 0;
	}
}

uint32_t dataCacheReadMiss(uint32_t memAddress)
{
	uint32_t memValue;
	memValue = data_memory[convertMemoryAddressToMemoryIndex(memAddress)];
	evictWord(memAddress);
	addToSetCache(memAddress, memValue);
	return memValue;
}

void evictWord(uint32_t memAddress)
{
	int evictedWord = 0;
	for (int wordCycler = 0; wordCycler < 4; wordCycler++)
	{
		if (L1_data_cache[findSetNumber(memAddress)].words[wordCycler].lruTimer == 0)
		{
			evictedWord = wordCycler;
		}
	}

	if (L1_data_cache[findSetNumber(memAddress)].words[evictedWord].lruTimer == 0)
	{
		if (L1_data_cache[findSetNumber(memAddress)].words[evictedWord].dirtyBit == 1)
		{
			data_memory[convertMemoryAddressToMemoryIndex(memAddress)] = L1_data_cache[findSetNumber(memAddress)].words[evictedWord].wordContent;
			L1_data_cache[findSetNumber(memAddress)].words[evictedWord].dirtyBit = 0;
			L1_data_cache[findSetNumber(memAddress)].words[evictedWord].validBit = 0;
		}
	}
}

AssociativeWord convertMemoryAddresstoSetComponents(AssociativeWord passedWord, uint32_t memAddress)
{
	passedWord.offset = (memAddress & 15);
	passedWord.tag = ((memAddress & 4294965248) >> 11);

	return passedWord;
}

uint32_t convertSetComponentsToMemoryAddress(AssociativeWord passedWord, uint32_t setNumber) 
{
	uint32_t memoryAddress;
	memoryAddress = 0;
	memoryAddress = memoryAddress ^ passedWord.offset;
	memoryAddress = memoryAddress ^ (setNumber << 4);
	memoryAddress = memoryAddress ^ (passedWord.tag << 11);
	return memoryAddress;
}

void addToSetCache(uint32_t memAddress, uint32_t memValue)
{
	int targetWord = 0;

	for (int wordCycler = 0; wordCycler < 4; wordCycler++)
	{
		if (L1_data_cache[findSetNumber(memAddress)].words[wordCycler].lruTimer == 0)
		{
			targetWord = wordCycler;
		}
	}

	L1_data_cache[findSetNumber(memAddress)].words[targetWord].wordContent = memValue;
	L1_data_cache[findSetNumber(memAddress)].words[targetWord] = convertMemoryAddresstoSetComponents(L1_data_cache[findSetNumber(memAddress)].words[targetWord], memAddress);
	L1_data_cache[findSetNumber(memAddress)].words[targetWord].lruTimer = 4;
	L1_data_cache[findSetNumber(memAddress)].words[targetWord].validBit = 1;

	for (int wordCycler = 0; wordCycler < 4; wordCycler++)
	{
		L1_data_cache[findSetNumber(memAddress)].words[wordCycler].lruTimer--;
	}
}

void updateDataCacheWord(uint32_t memAddress)
{
	int preservationValue = 0;
	for (int wordCycler = 0; wordCycler < 4; wordCycler++)
	{
		if ((L1_data_cache[findSetNumber(memAddress)].words[wordCycler].tag == ((memAddress & 4294965248) >> 11)) &&
			(L1_data_cache[findSetNumber(memAddress)].words[wordCycler].offset == (memAddress & 15)))
		{
			preservationValue = L1_data_cache[findSetNumber(memAddress)].words[wordCycler].lruTimer;
			L1_data_cache[findSetNumber(memAddress)].words[wordCycler].lruTimer = 4;
			break;
		}
	}
	for (int wordCycler = 0; wordCycler < 4; wordCycler++)
	{
		if ((int) L1_data_cache[findSetNumber(memAddress)].words[wordCycler].lruTimer > preservationValue)
		{
			L1_data_cache[findSetNumber(memAddress)].words[wordCycler].lruTimer--;
		}
	}

}

uint32_t findDataCacheValue(uint32_t memAddress, int *isFound)
{
	for (int wordCycler = 0; wordCycler < 4; wordCycler++)
	{
		if ((L1_data_cache[findSetNumber(memAddress)].words[wordCycler].tag == ((memAddress & 4294965248) >> 11)) &&
			(L1_data_cache[findSetNumber(memAddress)].words[wordCycler].offset == (memAddress & 15)) &&
			(L1_data_cache[findSetNumber(memAddress)].words[wordCycler].validBit == 1))
		{
			(*isFound) = 0;
			return (L1_data_cache[findSetNumber(memAddress)].words[wordCycler].wordContent);
		}
	}
	(*isFound) = 0;
	return -1;
}

uint32_t dataCacheReadHit(uint32_t memAddress)
{
	int isFound = 1;
	uint32_t cacheValue;
	updateDataCacheWord(memAddress);
	cacheValue = findDataCacheValue(memAddress, &isFound);
	return cacheValue;
}

int modifyDataCacheWord(uint32_t memAddress, uint32_t updatedValue)
{
	for (int wordCycler = 0; wordCycler < 4; wordCycler++)
	{
		if ((L1_data_cache[findSetNumber(memAddress)].words[wordCycler].tag == ((memAddress & 4294965248) >> 11)) &&
			(L1_data_cache[findSetNumber(memAddress)].words[wordCycler].offset == (memAddress & 15)))
		{
			L1_data_cache[findSetNumber(memAddress)].words[wordCycler].wordContent = updatedValue;
			L1_data_cache[findSetNumber(memAddress)].words[wordCycler].dirtyBit = 1;
		}
	}
	return 0;
}

void dataCacheWriteHit(uint32_t memAddress, uint32_t updatedValue)
{
	updateDataCacheWord(memAddress);
	modifyDataCacheWord(memAddress, updatedValue);
}

void dataCacheWriteMiss(uint32_t memAddress, uint32_t valueToStore)
{
	//Write allocate so data is stored in memory and in cache simultaneously, evicting the word at the postion
	data_memory[convertMemoryAddressToMemoryIndex(memAddress)] = valueToStore;
	evictWord(memAddress);
	addToSetCache(memAddress, valueToStore);
}