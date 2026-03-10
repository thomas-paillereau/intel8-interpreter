#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "CPU.h"


void cpu_init(CPU *cpu, int argc, char **argv, char *env, char *code, int sizeText, int sizeData)
{
	// Init
	cpu->lastReg = calloc(1,sizeof(char));

	for (int i = 0; i <= 0xffff; i++)
		cpu->memory[i] = 0;
	// Putting data in memory
	for (int i = 0x20 + sizeText; i < 0x20 + sizeText + sizeData; i++)
		cpu->memory[i - 0x20 - sizeText] = (uint8_t) code[i];

	cpu->cs = 0;
	cpu->ds = 0;
	cpu->ss = sizeData;
	cpu->es = 0xfffe;

	/// Putting the stack begining
	// Setting argv and argc
	cpu->sp = 0x0000;
	argv++;
	argc--;	
	while (*argv && *(*argv) == '-')
	{
		argv++;
		argc--;
	}
	
	// Calculating the len of env + argv, and rounding
	uint16_t argAddrList[0x8000];
	int indexArgAddrList = 0;

	int n = strlen(env) + 1;
	for (int i = 0; i < argc; i++)
		n += strlen(argv[i]) + 1;
	
	if (n % 2 == 1)
		cpu->sp++;

	// Entering env in memory
	cpu->memory[--(cpu->sp)] = '\0'; 
	for (int i = strlen(env) -1; i >= 0; i--)
		cpu->memory[--(cpu->sp)] = env[i]; 

	uint16_t addrEnv = cpu->sp;
	
	// Entering argv in memory
	for (int i = argc - 1; i >= 0; i--)
	{
		cpu->memory[--(cpu->sp)] = '\0'; 
		for (int j = strlen(argv[i]) - 1; j >= 0; j--)
			cpu->memory[--(cpu->sp)] = argv[i][j];
		argAddrList[indexArgAddrList] = cpu->sp;
		indexArgAddrList++;
	}

	argAddrList[indexArgAddrList] = 0x0;

	// Adding null 2 bytes
	cpu->memory[--(cpu->sp)] = 0; 
	cpu->memory[--(cpu->sp)] = 0;

	// Adding env addr
	cpu->memory[--(cpu->sp)] = addrEnv >> 8; 
	cpu->memory[--(cpu->sp)] = addrEnv;

	// Adding null 2 bytes
	cpu->memory[--(cpu->sp)] = 0; 
	cpu->memory[--(cpu->sp)] = 0;

	// Adding argv addr
	for (int i = 0; argAddrList[i]; i++)
	{
		cpu->memory[--(cpu->sp)] = argAddrList[i] >> 8; 
		cpu->memory[--(cpu->sp)] = argAddrList[i];
	}

	// Adding argc
	cpu->memory[--(cpu->sp)] = argc >> 8;
	cpu->memory[--(cpu->sp)] = argc;
}

void cpu_free(CPU *cpu)
{
	free(cpu->lastReg);
}

// Automaticly detect the register and update the value of it to the given argument
void set(CPU *cpu, int indReg, uint16_t value)
{
	if (indReg < 8)
		cpu->regs[indReg] = value % 0x10000;
	else if (8 <= indReg && indReg < 12)
		switch (indReg){
			case 8: cpu->al = (value << 8) >> 8; break;
			case 9: cpu->bl = (value << 8) >> 8; break;
			case 10: cpu->cl = (value << 8) >> 8; break;
			case 11: cpu->dl = (value << 8) >> 8; break;
			default: return;
		}
	else if (12 <= indReg && indReg < 16)
		switch (indReg){
			case 12: cpu->ah = (value >> 8); break;
			case 13: cpu->bh = (value >> 8); break;
			case 14: cpu->ch = (value >> 8); break;
			case 15: cpu->dh = (value >> 8); break;
			default: return;
		}
	else
		return;
}

// Automaticly detect the register and get the value of it
uint16_t get(CPU *cpu, int indReg)
{
	if (indReg < 8)
		return cpu->regs[indReg];
	else if (8 <= indReg && indReg < 12)
		switch (indReg){
			case 8: return cpu->al;
			case 9: return cpu->bl;
			case 10: return cpu->cl;
			case 11: return cpu->dl;
			default: return -1;
		}
	else if (12 <= indReg && indReg < 16)
		switch (indReg){
			case 12: return cpu->ah;
			case 13: return cpu->bh;
			case 14: return cpu->ch;
			case 15: return cpu->dh;
			default: return -1;
		}
	else
		return -1;
}

// Get the 16 bits value in the memory 
uint16_t getMem(CPU* cpu, uint16_t offset)
{
	uint16_t value = cpu->memory[offset];
	value += cpu->memory[offset+1] << 8;
	return value;
}

// Set the 16 bits value in the memory to the given argument 
void setMem(CPU *cpu, uint16_t offset, uint16_t value)
{
	cpu->memory[offset] = value;
	cpu->memory[offset+1] = value >> 8;
}

// Set the 8 bits value in the memory to the given argument
void setMem8(CPU *cpu, uint16_t offset, uint16_t value)
{
	cpu->memory[offset] = (value << 8) >> 8;
}

// Push the given argument (16 bits) in the memory and moves the SP register back 
void push(CPU *cpu, uint16_t value)
{
	cpu->sp -= 2;
	cpu->memory[cpu->sp] = (uint16_t) value % 0x100;
	cpu->memory[cpu->sp+1] = (uint16_t) value >> 8;
}

// Pop the given argument (16 bits) in the memory and moves the SP register forward
uint16_t pop(CPU *cpu)
{
	short value = cpu->memory[cpu->sp] % 0x100;
	value += cpu->memory[cpu->sp+1] << 8;
	cpu->sp += 2;
	return value;
}

// Gets the id of a register from an int
enum RegIndex iFindReg(int n)
{
	switch (n)
	{
		case 0: return AX;
		case 1: return BX;
		case 2: return CX;
		case 3: return DX;
		case 4: return SP;
		case 5: return BP;
		case 6: return SI;
		case 7: return DI;

		case 8: return AL;
		case 9: return BL;
		case 10: return CL;
		case 11: return DL;
		case 12: return AH;
		case 13: return BH;
		case 14: return CH;
		case 15: return DH;

		default: return NONE;
	}
}

// Gets the id of a register from a string
enum RegIndex sFindReg(char *text)
{
	if (text[0] && text[0] == '-')
		text++;
	if (!strcmp(text,"ax"))
		return AX;
	else if (!strcmp(text,"bx"))
		return BX;
	else if (!strcmp(text,"cx"))
		return CX;
	else if (!strcmp(text,"dx"))
		return DX;
	else if (!strcmp(text,"sp"))
		return SP;
	else if (!strcmp(text,"bp"))
		return BP;
	else if (!strcmp(text,"si"))
		return SI;
	else if (!strcmp(text,"di"))
		return DI;
	else if (!strcmp(text,"al"))
		return AL;
	else if (!strcmp(text,"bl"))
		return BL;
	else if (!strcmp(text,"cl"))
		return CL;
	else if (!strcmp(text,"dl"))
		return DL;
	else if (!strcmp(text,"ah"))
		return AH;
	else if (!strcmp(text,"bh"))
		return BH;
	else if (!strcmp(text,"ch"))
		return CH;
	else if (!strcmp(text,"dh"))
		return DH;
	else
		return 16;
}

// Gets the string of a register from an id of register
char *iFindSReg(int n)
{
	switch (n)
	{
		case 0: return "ax";
		case 1: return "ax";
		case 2: return "ax";
		case 3: return "ax";
		case 4: return "ax";
		case 5: return "ax";
		case 6: return "ax";
		case 7: return "ax";
		case 8: return "al";
		case 9: return "ax";
		case 10: return "ax";
		case 11: return "ax";
		case 12: return "ah";
		case 13: return "ax";
		case 14: return "ax";
		case 15: return "ax";
		default: return "";
	}
}

// Updates the "Last register" flag
void UpdateLastReg(CPU *cpu, char *s)
{
	char *lastReg = cpu->lastReg;
	free(lastReg);
	asprintf(&lastReg,"%s",s);
	cpu->lastReg = lastReg;
}

// Updates the Overflow flag
void UpdateOF(CPU *cpu, short dst, short src, short res, char *operation)
{
	if (!strcmp(operation,"add"))
	{
		if (dst < 0 && src < 0 && res >= 0)
			cpu->of = 1;	
		if (dst >= 0 && src >= 0 && res < 0)
			cpu->of = 1;	
		cpu->of = 0;
	}
	else if (!strcmp(operation,"sub"))
	{
		src = -src;
		if (dst < 0 && src < 0 && res >= 0)
			cpu->of = 1;	
		if (dst >= 0 && src >= 0 && res < 0)
			cpu->of = 1;	
		cpu->of = 0;
	}
}

// Updates the Sign flag
void UpdateSF(CPU *cpu, short res)
{
	cpu->sf = (res < 0);
}

// Updates the Zero flag
void UpdateZF(CPU *cpu, short res)
{
	cpu->zf = (res == 0);
}

// Updates the Carry flag
void UpdateCF(CPU *cpu, uint16_t dst, uint16_t src, char *op)
{
	if (!strcmp(op,"sub")) {
        	cpu->cf = (dst < src);
    	} else {
        	uint32_t result = (uint32_t)dst + (uint32_t)src;
        	cpu->cf = (result > 0xFFFF);
   	}
}

