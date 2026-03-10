#define MEMORY_SIZE 0xFFFF

#ifndef CPU_H
#define CPU_H

typedef struct {
	uint16_t ip;

	union {
        	uint16_t regs[8];
        
        	struct {
            		union { uint16_t ax; struct { uint8_t al, ah; }; };
            		union { uint16_t bx; struct { uint8_t bl, bh; }; };
            		union { uint16_t cx; struct { uint8_t cl, ch; }; };
            		union { uint16_t dx; struct { uint8_t dl, dh; }; };
            
            		uint16_t sp, bp, si, di;
			uint16_t cs, ds, ss, es;
        	};
    	};
	union {
		uint8_t flags[4];
		struct {
        		uint8_t of : 1;  // Overflow Flag&
        		uint8_t sf : 1;  // Sign Flag
        		uint8_t zf : 1;  // Zero Flag
        		uint8_t cf : 1;  // Carry Flag
		};
	};
	uint8_t memory[MEMORY_SIZE];
	char *lastReg;
}CPU;

enum RegIndex {
    AX, BX, CX, DX,
    SP, BP, SI, DI,
    AL, BL, CL, DL,
    AH, BH, CH, DH,
    NONE
};



void cpu_init(CPU *cpu, int argc, char **argv, char *env, char *code, int sizeText, int sizeData);
void cpu_free(CPU *cpu);
void push(CPU *cpu, uint16_t value);
uint16_t pop(CPU *cpu);
uint16_t get(CPU *cpu, int indReg);
void set(CPU *cpu, int indReg, uint16_t value);
void setMem(CPU *cpu, uint16_t offset, uint16_t value);
void setMem8(CPU *cpu, uint16_t offset, uint16_t value);
uint16_t getMem(CPU* cpu, uint16_t offset);

enum RegIndex iFindReg(int n);
enum RegIndex sFindReg(char *text);
char *iFindSReg(int n);

void UpdateLastReg(CPU *cpu, char *s);
void UpdateOF(CPU *cpu, short dst, short src, short res, char *operation);
void UpdateSF(CPU *cpu, short res);
void UpdateZF(CPU *cpu, short res);
void UpdateCF(CPU *cpu, uint16_t dst, uint16_t src, char *op);

#endif
