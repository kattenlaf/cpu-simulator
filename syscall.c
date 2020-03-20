/**
* Gedare Bloom
* syscall.c
*
* Implementation of the system calls
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h> 
#include "memory.h"
#include "cpu.h"

typedef struct
{
	char letter1;
	char letter2;
	char letter3;
	char letter4;
}hexChars;

hexChars getAllLetters(hexChars myLetters, uint32_t hexNum)
{
	myLetters.letter1 = (hexNum & 4278190080) >> 24; //4278190080 == 11111111 00000000 00000000 000000000, Isolate the top 8 bits and then shift 24 to the right
	myLetters.letter2 = (hexNum & 16711680) >> 16; //16711680 == 00000000 11111111 00000000 00000000, Isolate top middle 8 and then shift 16 to the right
	myLetters.letter3 = (hexNum & 65280) >> 8; //65280 == 00000000 000000000 11111111 00000000, Isolate bottom middle 8 and then shift 8 to the right
	myLetters.letter4 = (hexNum & 255); // Isolate last 8
	return myLetters;
}

int sys_exit() {
	printf("Exiting\n");
	exit(0);
}


//Exit

int syscall(ID_EX_buffer *ID_EX, struct cpu_context CPU) //num is the v0 register in CPU, CPU.GPR[2] since v0 is 00010
{
	//num is the v0 register in CPU, CPU.GPR[2] since v0 is 00010

	uint32_t num = CPU.GPR[2]; //Place the value from v0 in num

							   //Print int 

	if (num == 1)
	{
		int integer1 = CPU.GPR[4]; //$a0 is 00100 or in this case CPU.GPR[4]
		printf("%d\n", integer1);
		setAllSignalsToZero(ID_EX);
		return 0;
	}
	//Print String

	if (num == 4)
	{
		uint32_t CharAddr = CPU.GPR[4]; //$a0 in the case of a string stores the address of the first character in the string
										// Assuming that the address index will convert by design
		CharAddr = convertMemoryAddressToMemoryIndex(CharAddr); //Making this be the address of the characters in data_mem assuming it will convert by design

		char letter = data_memory[CharAddr];

		while (1)
		{
			letter = data_memory[CharAddr];
			printf("%c", letter);
			CharAddr++;
			if (letter == '\0')
			{
				break;
			}
		}
		setAllSignalsToZero(ID_EX);
		return 0;
	}

	if (num == 10)
	{
		sys_exit();
		setAllSignalsToZero(ID_EX);
		return 0;
	}

	return 0; //error case
}

uint32_t setAllSignalsToZero(ID_EX_buffer* ID_EX)
{
	//Made the signals of ID_EX.CUSignalType be 0
	ID_EX->signals.exPhase.ALUSrc = 0;
	ID_EX->signals.exPhase.RegDst = 0;
	ID_EX->signals.memPhase.MemRead = 0;
	ID_EX->signals.memPhase.MemWrite = 0;
	ID_EX->signals.writeBackPhase.MemtoReg = 0;
	ID_EX->signals.writeBackPhase.RegWrite = 0;
	ID_EX->signals.dePhase.Branch = 0;
	ID_EX->signals.dePhase.Jump = 0;
	return 0;
}

uint32_t loadImmediate(uint32_t value)
{
	uint32_t upper = (value & 4294901760); //isolate the top 16 bits
	uint16_t lower = (value & 65535);
	return upper + lower;
}

uint32_t loadUnsignedImmediate(uint32_t value)
{
	uint32_t upper = (value & 4294901760); //isolate the top 16 bits
	return upper;
}

uint16_t ORI(uint16_t value)
{
	uint16_t lower = (value & 65535);
	return lower;
}