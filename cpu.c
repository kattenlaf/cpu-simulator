/**
* Ruel Gordon
* Matthew King
* LeAnn Lewis
* Gedare Bloom
* cpu.c
*
* Implementation of simulated processor.
*/

#include "cpu.h"
#include "memory.h"

struct cpu_context cpu_ctx;
int instructionExists;

int fetch(struct IF_ID_buffer *out)
{
	//ACCESS CACHE FIRST, does cache store instruction or address to the instruction
	if (doesInstructionExist(cpu_ctx.PC) == 1)
	{
		totalInstructions++;
		instructionCacheHit++;
		// TODO: GET INSTRUCTION
		(*out).instruction = getInstruction(cpu_ctx.PC);
		//printf("The instruction is : %u\n", getInstruction(cpu_ctx.PC));
		//printf("Instruction Grabbed\n");
		(*out).next_pc = cpu_ctx.PC + 4;
	}
	else
	{
		stallCount++;
		instructionCacheMiss++;
		//STALL AND PLACE BLOCK? HOWEVER YOU WANT IT
		placeBlock(cpu_ctx.PC);
		//(*out).instruction = instruction_memory[convertPCToInstructionIndex(cpu_ctx.PC)];
		//Stall 
		(*out).next_pc = cpu_ctx.PC;
	}

	//(*out).instruction = instruction_memory[convertPCToInstructionIndex(cpu_ctx.PC)];
	//printf("The instruction is %u\n", (*out).instruction);
	//(*out).next_pc = cpu_ctx.PC + 4;
	return 0;
}

int fetch2(struct IF_ID_buffer *out)
{
	(*out).instruction = instruction_memory[convertPCToInstructionIndex(cpu_ctx.PC)];
	(*out).next_pc = cpu_ctx.PC + 4;
	return 0;
}

int decode(struct IF_ID_buffer *in, ID_EX_buffer *out)
{
	setSignalsToZeroh(out);
	InstructionComponents currentInstruction;
	currentInstruction = separateComponents((*in).instruction);
	(*out).writeAddressIType = currentInstruction.writeAddressIType;
	(*out).writeAddressRType = currentInstruction.writeAddressRType;
	(*out).extendedImmediate = signExtendValue(currentInstruction.immediate);
	(*out).signals = generateControlSignals(currentInstruction.opCode);
	*out = updateReadValues(currentInstruction, *out, cpu_ctx.GPR);
	(*out).opCode = currentInstruction.opCode;
	(*out).funct = currentInstruction.funct;

	if (currentInstruction.read1Address == 31 && currentInstruction.funct == 8) // jr $ra
	{
		jumpRegister(in, out);
	}

	if (out->signals.dePhase.Jump == 1) //J or Jal
	{
		if (currentInstruction.opCode == 2)
		{
			jump(in, currentInstruction, out);
		}
		else if (currentInstruction.opCode == 3)
		{
			jumpAndLink(in, currentInstruction, out);
		}
	}

	if (currentInstruction.opCode == 0 && currentInstruction.funct == 12) //Syscall
	{
		syscall(out, cpu_ctx);
	}
	return 0;
}

int execute(ID_EX_buffer *in, struct EX_MEM_buffer *out)
{
	uint32_t secondValue = 0;
	in->aluOP = generateAluOp(in->opCode, in->funct);
	//Responsible for handling what value should be passed to ALU
	secondValue = Multiplexor(in);
	(*out).writeAddressIType = (*in).writeAddressIType;
	(*out).writeAddressRType = (*in).writeAddressRType;
	out->address = ALU(in, secondValue);
	(*out).writeData = (*in).read2Cont;
	(*out).signals = (*in).signals;
	return 0;
}



int memory(struct EX_MEM_buffer* in, struct MEM_WB_buffer* out)
{
	out->ALUresult = in->address; //Address is stored in the MEM_
	(*out).memdata = (*in).address; // not sure if a fix
	rw_memory(in->address, in->writeData, in->signals.memPhase.MemWrite, in->signals.memPhase.MemRead, &out->memdata, data_memory);
	(*out).writeAddressIType = (*in).writeAddressIType;
	(*out).writeAddressRType = (*in).writeAddressRType;
	(*out).signals = (*in).signals;
	return 0;
}

int memory2(struct EX_MEM_buffer* in, struct MEM_WB_buffer* out)
{
	out->ALUresult = in->address; //Address is stored in the MEM_
	(*out).memdata = (*in).address; // not sure if a fix
	rw_memory2(in->address, in->writeData, in->signals.memPhase.MemWrite, in->signals.memPhase.MemRead, &out->memdata, data_memory);
	(*out).writeAddressIType = (*in).writeAddressIType;
	(*out).writeAddressRType = (*in).writeAddressRType;
	(*out).signals = (*in).signals;
	return 0;
}

int writeback(struct MEM_WB_buffer *in)
{
	write_register((*in).writeAddressIType, (*in).writeAddressRType, in->memdata, in->ALUresult, in->signals.writeBackPhase.RegWrite, in->signals.exPhase.RegDst, in->signals.writeBackPhase.MemtoReg, cpu_ctx.GPR);

	return 0;
}