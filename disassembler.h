#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

char** disassembler(char *text,int *sizeDisa);
char *getInstruction(char *text, int offset, int *returnSize);

#endif
