#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <errno.h>

#include "CPU.h"
#include "disassembler.h"

#define memSize 100

void interpreter(CPU *cpu, char *text, int m);
char *findCode(char *text, int *sizeCode);
char *memoryAccess(char *text,int *sizeDisa);

int main(int argc, char *argv[])
{
	int fileIndex = 1;
	int m = 0;
	// Verification of the existence and opening of the file.
	if (argc == 1)
	{
		printf("Please enter the name of a file to execute.\n");
		return 1;
	}

	if (argc > 2)
	{
		if (!strcmp(argv[1],"-m"))
		{
			m = 1;
			fileIndex = 2;
		}
	}

	FILE *file;
	file = fopen(argv[fileIndex],"r");
	if (file == NULL)
	{
		printf("The file \"%s\" was not found or opened, please check if you used the right name.\n", argv[1]);
		return 1;
	}

	// Reading the file into a string, and 
	char fileContent[memSize];
	char *code = calloc(1,sizeof(char));
	int size = 1;
	int r;
	while ((r = fread(fileContent,sizeof(char), memSize, file)))
	{
		code = realloc(code,(size+r)*sizeof(char));	
		for (int i = size-1; i < (size-1) + r; i++)
			code[i] = fileContent[i-(size-1)];
		size = size + r;
		code[size-1] = 0;
	}
	fclose(file);
	
	// Getting the information from the code and the data of the given executable 
	int sizeCode;
	char *text = findCode(code, &sizeCode);
	int sizeData = sizeCode;
	char *data = memoryAccess(code,&sizeData);

	// Initialing the CPU and the memory of the interpreter
	char *env = "PATH=/usr:/usr/bin";
	CPU cpu;
	cpu_init(&cpu,argc,argv,env,code,sizeCode,sizeData);

	// Interpreting the given executable
	interpreter(&cpu,text,m);
	cpu_free(&cpu);


	// Freeing the Result array and returning a successfull result
	free(code);
	free(text);
	free(data);
	return 0;
}



//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

char **segCut(char *line, char separ);

int findReg(char *text);

short findValue(CPU *cpu, char* text);
void setValue(CPU *cpu, char*text, short value, char* src);

short getMemAddr(CPU *cpu, char *text);

int findInstIndex(char *text, char **Insts);

int HexStrToUInt(char* s);

int useData(CPU* cpu, int type, uint16_t offset, int m, int *quit);

void oldValueDisplay(CPU *cpu,char *text, int useOtherSeg);
void printMemCases(CPU *cpu, char *text, short value, int Case, int m);
void printMemory(CPU *cpu, uint16_t start, uint16_t end);


void interpreter(CPU *cpu, char *code, int m)
{
	//printMemory(cpu,0xFF00,0xFFFF);

	if (m)
		printf(" AX   BX   CX   DX   SP   BP   SI   DI  FLAGS IP\n");
	

	int quit = 0;
	int halt = 0;
	int offset = 0;
	while (!quit)
	{
		// Setting up the instructions to be interpreted
		int size = 1;
		char *line = getInstruction(code, offset, &size);
		for (int j = 0; line[j]; j++)
			if (line[j] == '\n')
				line[j] = 0;	
		int increaseOffset = 1;

		char **segs = segCut(line+18,' ');


		char *inst = segs[0];
		char **seg = segs+1;

		char OF = cpu->of == 1?'O':'-';
		char SF = cpu->sf == 1?'S':'-';
		char ZF = cpu->zf == 1?'Z':'-';
		char CF = cpu->cf == 1?'C':'-';

		if (!strcmp(inst,"int"))
			halt = 0;

		if (m && !halt)
			printf("%04hx %04hx %04hx %04hx %04hx %04hx %04hx %04hx %c%c%c%c %s",cpu->ax,cpu->bx,cpu->cx,cpu->dx,cpu->sp,cpu->bp,cpu->si,cpu->di,OF,SF,ZF,CF,line);

		//int sht = 0;
		int byte = 0;
		if (*seg && (!strcmp(*seg,"short") || !(strcmp(*seg,"byte"))))
		{
			/*if (!strcmp(*seg,"short"))
				sht = 1;*/
			if (!strcmp(*seg,"byte"))
				byte = 1;
			seg++;
		}
		if (m && !halt)
		{
			if (*seg && strchr(seg[0],'['))
			{
				if (*(seg+1))
					oldValueDisplay(cpu,seg[0], byte || (8 <= sFindReg(seg[1]) && sFindReg(seg[1]) < 16));
				else	
					oldValueDisplay(cpu,seg[0], byte);
			}
			if (*seg && *(seg+1) && strchr(seg[1],'['))
				oldValueDisplay(cpu,seg[1], 8 <= sFindReg(seg[0]) && sFindReg(seg[0]) < 16);
			printf("\n");
		}





		// Interpreting the instructions
		if (halt) 
		{

		}
		else if (!strcmp(inst,"mov"))
		{
			short value = findValue(cpu,seg[1]);
			setValue(cpu,seg[0],value,seg[1]);
			UpdateLastReg(cpu,seg[0]);
			//Flags
		}
		else if (!strcmp(inst,"push"))
		{
			short value = findValue(cpu,seg[0]);
			push(cpu,value);
			UpdateLastReg(cpu,seg[0]);
			//Flags
		}
		else if (!strcmp(inst,"pop"))
		{
			short value = pop(cpu);
			setValue(cpu,seg[0],value,"");
			UpdateLastReg(cpu,seg[0]);
			//Flags
		}
		else if (!strcmp(inst,"xchg"))
		{
			short value1 = findValue(cpu,seg[0]);
			short value2 = findValue(cpu,seg[1]);
			setValue(cpu,seg[1],value1,seg[0]);
			setValue(cpu,seg[0],value2,seg[1]);
		}
		//in
		//out
		//xlat
		else if (!strcmp(inst,"lea"))
		{
			short value = getMemAddr(cpu,seg[1]);
			setValue(cpu,seg[0],value,"");
			UpdateLastReg(cpu,seg[0]);
			//Flags
		}
		//lds
		//les
		//lahf
		//sahf
		//pushf
		//popf
		
		else if (!strcmp(inst,"add"))
		{
			short value1 = findValue(cpu,seg[0]);
			short value2 = findValue(cpu,seg[1]);
			setValue(cpu,seg[0],value1+value2,seg[1]);
			UpdateLastReg(cpu,seg[0]);
			UpdateOF(cpu,value1,value2,value1+value2,"add");
			UpdateSF(cpu,value1+value2);
			UpdateZF(cpu,value1+value2);
			UpdateCF(cpu,value1,value2,"add");
		}
		else if (!strcmp(inst,"adc"))
		{
			short value1 = findValue(cpu,seg[0]);
			short value2 = findValue(cpu,seg[1]);
			short value3 = findValue(cpu,cpu->lastReg);
			setValue(cpu,seg[0],value1+value2+value3,seg[1]);
			UpdateLastReg(cpu,seg[0]);
			UpdateOF(cpu,value1,value2+value3,value1+value2+value3,"add");
			UpdateSF(cpu,value1+value2);
			UpdateZF(cpu,value1+value2);
			UpdateCF(cpu,value1,value2+value3,"add");
		}
		else if (!strcmp(inst,"inc"))
		{
			short value = findValue(cpu,seg[0]);
			if ((uint16_t)value == 0xFFFF)
				value = 0x0000;
			else
				value += 1;
			setValue(cpu,seg[0],value,"");
			UpdateLastReg(cpu,seg[0]);
			UpdateSF(cpu,value);
			UpdateZF(cpu,value);
			cpu->of = (value-1 == 0x7FFF);
		}
		//aaa
		//baa
		else if (!strcmp(inst,"sub"))
		{
			short value1 = findValue(cpu,seg[0]);
			short value2 = findValue(cpu,seg[1]);
			setValue(cpu,seg[0],value1-value2,seg[1]);
			UpdateLastReg(cpu,seg[0]);
			UpdateOF(cpu,value1,value2,value1-value2,"sub");
			UpdateSF(cpu,value1-value2);
			UpdateZF(cpu,value1-value2);
			UpdateCF(cpu,value1,value2,"sub");
		}
		else if (!strcmp(inst,"sbb"))
		{
			short value1 = findValue(cpu,seg[0]);
			short value2 = findValue(cpu,seg[1]);
			short value3 = findValue(cpu,cpu->lastReg);
			setValue(cpu,seg[0],value1-value2-value3,seg[1]);
			UpdateLastReg(cpu,seg[0]);
			UpdateOF(cpu,value1,value2+value3,value1-value2-value3,"sub");
			UpdateSF(cpu,value1-value2);
			UpdateZF(cpu,value1-value2);
			UpdateCF(cpu,value1,-value2-value3,"sub");
		}
		else if (!strcmp(inst,"dec"))
		{
			short value = findValue(cpu,seg[0]);
			setValue(cpu,seg[0],value-1,"");
			UpdateLastReg(cpu,seg[0]);
			UpdateSF(cpu,value-1);
			UpdateZF(cpu,value-1);
			cpu->of = (value+1 == 0x8000);
		}
		else if (!strcmp(inst,"neg"))
		{
			short value = findValue(cpu,seg[0]);
			setValue(cpu,seg[0],-value,"");
			UpdateLastReg(cpu,seg[0]);
			UpdateSF(cpu,-value);
			UpdateZF(cpu,-value);
			cpu->of = (value == (short)0x8000);
			cpu->cf = (value != 0);
		}
		else if (!strcmp(inst,"cmp"))
		{
			short value1 = findValue(cpu,seg[0]);
			short value2 = findValue(cpu,seg[1]);
			short value = value1 - value2;
			UpdateLastReg(cpu,seg[0]);
			UpdateOF(cpu,value1,value2,value1-value2,"sub");
			if (byte)
			{
				UpdateZF(cpu,(uint8_t)value1 - (uint8_t) value2);
				UpdateSF(cpu,(uint8_t)value1 - (uint8_t) value2);
			}
			else
			{
				UpdateZF(cpu,value);
				UpdateSF(cpu,value);
			}
			UpdateCF(cpu,value1,value2,"sub");
		}
		//aas
		//das
		//mul
		//imul
		//aam
		else if (!strcmp(inst,"div"))
		{
			short value = findValue(cpu,seg[0]);
			if (value != 0)
			{
				uint32_t dividend = ((uint32_t)cpu->dx << 16) | cpu->ax;
    				uint16_t quotient = dividend / value;
    				uint16_t remainder = dividend % value;

				cpu->ax = quotient;
				cpu->dx = remainder;
			}
		}
		//idiv
		//aad
		else if (!strcmp(inst,"cbw"))
		{
			short value = findValue(cpu,cpu->lastReg);
			int tmpReg = sFindReg(cpu->lastReg);
			if (8 <= tmpReg && tmpReg < 12)
			{
				value = (value << 8) >> 8;
				short tmp = value >> 7;
				if (tmp)
					setValue(cpu,iFindSReg(tmpReg-8),0xFF00 + value,"");
				else
					setValue(cpu,iFindSReg(tmpReg-8),0x0000 + value,"");
			}
		}
		else if (!strcmp(inst,"cwd"))
		{
			if (cpu->ax & 0x8000) {
        			cpu->dx = 0xFFFF;
    			} else {
        			cpu->dx = 0x0000;
    			}	
		}
		
		//not
		else if (!strcmp(inst,"shl"))
		{
			short value1 = findValue(cpu,seg[0]);
			short value2 = findValue(cpu,seg[1]);
			short value = value1 << value2;
			setValue(cpu,seg[0],value,"");
			UpdateLastReg(cpu,seg[0]);
			cpu->cf = (value1 >> (16 - value2)) & 1;
			UpdateSF(cpu,value);
			UpdateZF(cpu,value);
		}
		//shr
		else if (!strcmp(inst,"sar"))
		{
			short value1 = findValue(cpu,seg[0]);
			short value2 = findValue(cpu,seg[1]);
			short value = value1;

			for (short i = 0; i < value2; i++)
			{
				value >>= 1;
				short tmp = (value >> 14) << 15;
				value += tmp;
			}

			setValue(cpu,seg[0],value,"");
			UpdateLastReg(cpu,seg[0]);
			cpu->cf = (value1 >> (value2 - 1)) & 1;
			UpdateSF(cpu,value);
			UpdateZF(cpu,value);
		}
		//rol
		//ror
		//rcl
		//rcr
		else if (!strcmp(inst,"and"))
		{
			short value = findValue(cpu,seg[0]) & findValue(cpu,seg[1]);
			setValue(cpu,seg[0],value,"");
			UpdateLastReg(cpu,seg[0]);
			UpdateSF(cpu,value);
			UpdateZF(cpu,value);
			cpu->of = 0;
			cpu->cf = 0;
		}
		else if (!strcmp(inst,"test"))
		{
			short value = findValue(cpu,seg[0]) & findValue(cpu,seg[1]);
			UpdateLastReg(cpu,seg[0]);
			UpdateSF(cpu,value);
			UpdateZF(cpu,value);
			cpu->cf = 0;
		}
		else if (!strcmp(inst,"or"))
		{
			short value = findValue(cpu,seg[0]) | findValue(cpu,seg[1]);
			setValue(cpu,seg[0],value,"");
			UpdateLastReg(cpu,seg[0]);
			UpdateSF(cpu,value);
			UpdateZF(cpu,value);
			cpu->of = 0;
			cpu->cf = 0;
		}
		else if (!strcmp(inst,"xor"))
		{
			short value = findValue(cpu,seg[0]) ^ findValue(cpu,seg[1]);
			setValue(cpu,seg[0],value,"");
			UpdateLastReg(cpu,seg[0]);
			UpdateSF(cpu,value);
			UpdateZF(cpu,value);
			cpu->of = 0;
			cpu->cf = 0;
		}
		//rep
		//movs
		//cmps
		//scas
		//lods
		//stos	
		else if (!strcmp(inst,"call"))
		{
			line[4] = 0;
			short value = HexStrToUInt(line);
			line[4] = ':';
			push(cpu,value+size);
			offset = findValue(cpu,seg[0]);
			increaseOffset = 0;
		}
		
		else if (!strcmp(inst,"jmp"))
		{
			offset = findValue(cpu,seg[0]);
			increaseOffset = 0;
		}
		
		else if (!strcmp(inst,"ret"))
		{
			
			short value = pop(cpu);
			char *sValue;
			asprintf(&sValue,"%04hx",value);
			offset = HexStrToUInt(sValue);
			free(sValue);
			if (seg[0])
			{
				short diff = findValue(cpu,seg[0]);
				cpu->sp = (uint16_t) ((short) cpu->sp) + diff;
			}
			increaseOffset = 0;
		}
		else if (!strcmp(inst,"je"))
		{
			if (cpu->zf)
			{
				offset = HexStrToUInt(seg[0]);
				increaseOffset = 0;
			}
		}
		else if (!strcmp(inst,"jl"))
		{
			if (cpu->sf != cpu->of)
			{
				offset = HexStrToUInt(seg[0]);
				increaseOffset = 0;
			}
		}
		else if (!strcmp(inst,"jle"))
		{
			if (cpu->zf || cpu->sf != cpu->of)
			{
				offset = HexStrToUInt(seg[0]);
				increaseOffset = 0;
			}
		}
		else if (!strcmp(inst,"jb"))
		{
			if (cpu->cf)
			{
				offset = HexStrToUInt(seg[0]);
				increaseOffset = 0;
			}
		}
		else if (!strcmp(inst,"jbe"))
		{
			if (cpu->cf || cpu->zf)
			{
				offset = HexStrToUInt(seg[0]);
				increaseOffset = 0;
			}
		}
		//jp
		//jo
		//js
		else if (!strcmp(inst,"jne"))
		{
			if (!cpu->zf)
			{
				offset = HexStrToUInt(seg[0]);
				increaseOffset = 0;
			}
		}
		else if (!strcmp(inst,"jnl"))
		{
			if (cpu->sf == cpu->of)
			{
				offset = HexStrToUInt(seg[0]);
				increaseOffset = 0;
			}
		}
		else if (!strcmp(inst,"jnle"))
		{
			if (!cpu->zf && cpu->sf == cpu->of)
			{
				offset = HexStrToUInt(seg[0]);
				increaseOffset = 0;
			}
		}
		else if (!strcmp(inst,"jnb"))
		{
			if (!cpu->cf)
			{
				offset = HexStrToUInt(seg[0]);
				increaseOffset = 0;
			}
		}
		else if (!strcmp(inst,"jnbe"))
		{
			if (!cpu->cf && !cpu->zf)
			{
				offset = HexStrToUInt(seg[0]);
				increaseOffset = 0;
			}
		}
		//jnp
		//jno
		//jns
		//loop
		//loopnz
		//jcxz
		else if (!strcmp(inst,"int"))
		{
			short index = get(cpu,sFindReg(cpu->lastReg));
			short type = findValue(cpu,seg[0]);
			useData(cpu,type,index,m,&quit);
			halt = 0;
			cpu->ax = 0x0000;
		}
		//into
		//iret
		
		//clc
		//cmc
		//stc
		//cld
		//std
		//cli
		//sti
		else if (!strcmp(inst,"hlt"))
		{
			halt = 1;
		}
		//wait
		//esc
		//lock
		else if (!strcmp(inst,"(undefined)")) {}
		else 
		{
			quit = 1;
			printf("Undefined\n");
		}

		// End of the instruction reading
		for (char **p = segs; *p; p++)
			free(*p);
		free(segs);
		free(line);

		if (increaseOffset)
			offset += size;
		if (offset < cpu->cs)
			quit = 1;
	}
}

// Segments a string for each time there is the given character "separ". If the character is '+', the '-' character is also counted
char **segCut(char *line, char separ)
{
	char **res = calloc(1,sizeof(char*));
	char *tmp = calloc(1,sizeof(char));
	int size = 0;
	int tmpSize = 0;
	for (int i = 0; line[i]; i++)
	{
		if (line[i] != separ && !(separ == '+' && line[i] == '-') && line[i] != ',' && line[i] != '\n' && line[i] != ']')
		{
			tmpSize++;
			tmp = realloc(tmp,(tmpSize+1)*sizeof(char));
			tmp[tmpSize-1] = line[i];
			tmp[tmpSize] = 0;
		}
		else if (line[i] != ',' && line[i] != '\n' && line[i] != ']')
		{
			size++;
			res = realloc(res,(size+1)*sizeof(char*));
			res[size-1] = tmp;
			res[size] = NULL;
			if (separ == '+' && line[i] == '-')	
			{
				tmp = calloc(2,sizeof(char));
				tmp[0] = '-';
				tmpSize = 1;
			}
			else
			{
				tmp = calloc(1,sizeof(char));
				tmpSize = 0;
			}
		}
	}
	size++;	
	res = realloc(res,(size+1)*sizeof(char*));
	res[size-1] = tmp;
	res[size] = NULL;

	return res;
}

// The function making the "interupt" instruction of the interpreter
int useData(CPU* cpu, int type, uint16_t offset, int m, int *quit)
{
	//printMemory(cpu,0xFF00,0xFFFF);
	if (type != 0x20)
		return -1;

	uint8_t *data = cpu->memory + offset;
	uint16_t fd = getMem(cpu,offset+4);
	uint16_t type2 = data[3] * 256 + data[2];
	uint16_t len = getMem(cpu,offset+6);
	uint16_t coordinates = getMem(cpu,offset+10);
	if (type2 == 1)
	{
		if (m)
			printf("<exit(%u)>\n",fd);
		*quit = 1;
		return 0;
	}
	else if (type2 == 4)
	{
		uint8_t *textData = cpu->memory + coordinates;
		if (m)
			printf("<write(%i, 0x%04hx, %i)",fd,coordinates,len);
		fflush(stdout);
		write(fd,textData,len);
		if (m)
			printf(" => %i>\n",len);
		setMem(cpu,offset+2,len);
		return len;
	}
	else if (type2 == 17)
	{
		uint16_t addr = getMem(cpu,offset+10);
		if (m)
			printf("<brk(0x%04hx) => 0>\n",addr);

		setMem(cpu, offset+2,0);
		setMem(cpu,offset+18,addr);

	}
	else if (type2 == 54)
	{	
		errno = EINVAL;
		uint16_t value1 = data[9] * 256 + data[8];
		uint16_t value2 = data[0x13] * 256 + data[0x12];
		if (m)
			printf("<ioctl(%i, 0x%04hx, 0x%04hx)>\n",fd,value1,value2);
		setMem(cpu,offset+2,-errno);
	}
	return 0;
}

// Detects where should the value be gotten (memory / register / immediate) and returns the gotten value
short findValue(CPU *cpu, char* text)
{
	int verif = 0;
	if (text[0] == '[')
	{
		text++;
		verif = 1;
	}

	char **seg = segCut(text,'+');
	int result = 0;
	if (verif)
	{
		for (int i = 0; seg[i]; i++)
		{
			int tmp = sFindReg(seg[i]);
			if (tmp == 16)
			{
				result += HexStrToUInt(seg[i]);
			}
			else 
				result += get(cpu,tmp);
		}	
		result = getMem(cpu,result);
	}
	else 
	{
		for (int i = 0; seg[i]; i++)
		{
			int tmp = sFindReg(seg[i]);
			if (tmp == 16)
			{
				result += HexStrToUInt(seg[i]);
			}
			else 
				result += get(cpu,tmp);
		}
	}
	for (int i = 0; seg[i]; i++)
		free(seg[i]);
	free(seg);
	return result;
}

// Sets the value, automaticly finding where to put the value (register / memory)
void setValue(CPU *cpu, char*text, short value, char* src)
{
	int srcIndex = sFindReg(src);
	int bit8 = (8 <= srcIndex && srcIndex < 16);
	int verif = 0;
	if (text[0] == '[')
	{
		text++;
		verif = 1;
	}

	if (verif)
	{
		char **seg = segCut(text,'+');
		int result = 0;
		for (int i = 0; seg[i]; i++)
		{
			int tmp = sFindReg(seg[i]);
			if (tmp == 16)
				result += HexStrToUInt(seg[i]);
			else 
				result += get(cpu,tmp);
		}
		for (int i = 0; seg[i]; i++)
			free(seg[i]);
		free(seg);
		if (bit8)
			setMem8(cpu,result,value);
		else
			setMem(cpu,result,value);
	}
	else
	{	
		int tmp = sFindReg(text);
		if (tmp == 16)
			printf("What????\n");
		else
			set(cpu,tmp,value);
	}
}

// Gets the address from a string (ex: "[bx+10]")
short getMemAddr(CPU *cpu, char *text)
{
	if (text[0] == '[')
	{
		text++;
		for (int i = 0; text[i]; i++)
			if (text[i] == ']')
				text[i] = 0;
	}

	char **seg = segCut(text,'+');
	int result = 0;
	for (int i = 0; seg[i]; i++)
	{
		int tmp = sFindReg(seg[i]);
		if (tmp == 16)
		{
			result += HexStrToUInt(seg[i]);
		}
		else 
			result += get(cpu,tmp);
	}

	for (int i = 0; seg[i]; i++)
		free(seg[i]);
	free(seg);
	
	return result;
}


int findInstIndex(char *text, char **Insts)
{
	int i = 0;
	for (; Insts[i]; i++)
	{
		int v1 = text[0] == Insts[i][0];
		int v2 = text[1] == Insts[i][1];
		int v3 = text[2] == Insts[i][2];
		int v4 = text[3] == Insts[i][3];
		if (v1 && v2 && v3 && v4)
			return i-1;
	}
	return i-1;
}

// Takes a string of an hexadecimal number and returns the corresponding interger
int HexStrToUInt(char* s)
{
	int minus = 1;
	if (*s == '-')
	{
		minus = -1;
		s++;
	}
	int result = 0;
	for (int i = 0; s[i]; i++)
	{
		result *= 16;
		if ('a' <= s[i] && s[i] <= 'f')
			result += s[i] - 'a' + 10;
		else
			result += s[i] - '0';
	}
	return minus*result;
}

// Finds the value in the memory before the instruction is applied to it
void oldValueDisplay(CPU *cpu,char *text, int useOtherSeg)
{
	int verif = 0;
	if (text[0] == '[')
	{
		text++;
		for (int i = 0; text[i]; i++)
			if (text[i] == ']')
				text[i] = 0;
		verif = 1;
	}
	
	char **seg = segCut(text,'+');
	short result = 0;
	for (int i = 0; seg[i]; i++)
	{
		short tmp = get(cpu,sFindReg(seg[i]));
		if (tmp == -1)
		{
			result += HexStrToUInt(seg[i]);
		}
		else 
			result += tmp;
	}
	for (int i = 0; seg[i]; i++)
		free(seg[i]);
	free(seg);
	if (!verif)
		return;

	if (verif)
	{
		short addr = result;
		result = getMem(cpu,result);
		if (useOtherSeg)
			printf(" ;[%04hx]%02hx",addr,result % 0x100);
		else
			printf(" ;[%04hx]%04hx",addr,result);
	}
	else 
	{
		printf(" ;%s %04x",text,result);
	}
}

void printMemCases(CPU *cpu, char *text, short value, int Case, int m)
{
	if (!m)
		return;
	if (Case == 1)
		oldValueDisplay(cpu,text, 0);
	else if (Case == 2)
	{
		int verif = 0;
		if (text[0] == '[')
		{
			text++;
			for (int i = 0; text[i]; i++)
				if (text[i] == ']')
					text[i] = 0;
			verif = 1;
		}
		

		char **seg = segCut(text,'+');
		short result = 0;
		for (int i = 0; seg[i]; i++)
		{
			short tmp = get(cpu,sFindReg(seg[i]));
			if (tmp == -1)
			{
				result += HexStrToUInt(seg[i]);
			}
			else 
				result += tmp;
		}
		for (int i = 0; seg[i]; i++)
			free(seg[i]);
		free(seg);

		if (verif)
		{
			printf(" ;[%04hx]%04hx",result,value);
		}
	}
	printf("\n");
}

// Prints memory, from the starting address to the ending address
void printMemory(CPU *cpu, uint16_t start, uint16_t end)
{
	for (uint16_t i = start; i < end; i++)
	{
		if (i % 0x10 == 0)
			printf("\n%04hx ", i);
		else if (i % 0x08 == 0)
			printf(" ");
		printf("%02hx ",cpu->memory[i]);
	}
	printf("%02hx \n",cpu->memory[end]);
}



//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

void getHexChar(char c, int* c1, int* c2);

// Gets the code of the executable
char *findCode(char *text, int *sizeCode)
{
	//Getting the size of the code (in byte)
	int size = 0;
	int power = 1;
	for (int i = 0; i < 4; i++)
	{
		size += ((text[8+i]%256)+256)%256 * power;
		power *= 256;
	}

	char *list = calloc(size+1,sizeof(char));
	//Finding all pieces of code
	text += 0x20;
	*sizeCode = size;
	int i = 0;
	while (i < size)
	{
		list[i] = text[i];
		i++;
	}
	return list;
}

// Gets the data of the executable
char *memoryAccess(char *text,int *sizeData)
{
	//Getting the size of the data (in byte)
	int size = 0;
	int power = 1;
	for (int i = 0; i < 4; i++)
	{
		size += ((text[12+i]%256)+256)%256 * power;
		power *= 256;
	}

	char *list = calloc(size+1,sizeof(char));
	//Finding all pieces of data
	text += 32 + *sizeData;
	*sizeData = size;
	int i = 0;
	while (i < size)
	{
		list[i] = text[i];
		i++;
	}
	return list;
}

