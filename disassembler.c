#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>

#define memSize 100
#define nSpaceAfterCode 20

char* binStrToHexStr(char *bin);
char intToHexChar(int n);
void removeStartZero(char *data1, char *data2);
int startsBy(char *string, char*start);
char* getBinChar(char c);
int BinStrToInt(char* data1, char* data2);
int HexStrToInt(char* data1, char* data2, int s);
int hexNegative(char* data1, char* data2);
int contains(char *s, char c);
void getValue(char *org, char**res, int pos, int size);
void getData(char *org, char **res, int pos);
void changeByteSize(char *code, int index, char **s, int newByteSize);

char *getReg(char *string, int w);
char *getEA(char *rm, char *mod, int *addi);
char *getInst(char* bin, char **printedString);
void getInst2(char* bin, char **printedString, char **sInst, int size);


char *getInstruction(char *text, int offset, int *returnSize)
{
	char *deft = "";
	char **ind = &deft;
	// Creating the return variables
	char *sInst = "";
	char *sReg = "";
	char *sEA = "";
	char *sEAAddi1 = "";
	char *sEAAddi2 = "";
	char *sData = "";
	char *sWData = "";
	char *sPos2 = "";
	char *sPos3 = "";
	// Getting the first byte of the instruction
	char *bin = getBinChar(*(text+offset));
	char *s2 = getInst(bin, &sInst);
	
	// Set up of the variables
	int w = 1;
	int d = 1;
	int s = -1;
	int n = 0;
	int t = 0;
	int a = 0;
	int A = 0;
	int f = 0;
	int F = 0;
	int o = 0;
	int O = 0;
	int q = 0;
	int Q = 0;
	int j = 0;
	int J = 0;
	int v = -1;
	int p = 0;
	int z = 0;

	char *mod = calloc(3,sizeof(char));
	char *reg = calloc(4,sizeof(char)); 
	char *rm = calloc(4,sizeof(char));
	char *data = calloc(1,sizeof(char));
	char *wdata = calloc(1,sizeof(char));
	char *disp1 = calloc(1,sizeof(char));
	char *disp2 = calloc(1,sizeof(char));
	char *Pos2 = calloc(1,sizeof(char));
	char *Pos3 = calloc(1,sizeof(char));

	// Checking the size that takes the instructions (in bytes)
	int nSpace = 0;
	for (int j = 0; s2[j]; j++)
		if (s2[j] == ' ')
			nSpace++;
		// Updating the variables and the size of the instructions
	n = contains(s2,'n') != -1;
	t = contains(s2,'t') != -1;
	a = contains(s2,'a') != -1;
	A = contains(s2,'A') != -1;
	f = contains(s2,'f') != -1;
	F = contains(s2,'F') != -1;
	o = contains(s2,'o') != -1;
	O = contains(s2,'O') != -1;
	q = contains(s2,'q') != -1;
	Q = contains(s2,'Q') != -1;
	j = contains(s2,'j') != -1;
	J = contains(s2,'J') != -1;
	p = contains(s2,'p') != -1;
	z = contains(s2,'z') != -1;
	
	int r;
	if ((r = contains(s2,'w')) != -1)
		w = bin[r] - '0';
	if ((r = contains(s2,'d')) != -1)
		d = bin[r] - '0';
	if ((r = contains(s2,'v')) != -1)
		v = bin[r] - '0';
	if (contains(s2,'i') != -1)
		if (!w)
			nSpace--;
	if (contains(s2,'I') != -1)
		if (!(w && !s))
			nSpace--;
	
	changeByteSize(text, offset, &bin, nSpace+1);

	// Making sure it is the good instructions with the new bytes
	getInst2(bin, &s2, &sInst, nSpace+1);
	nSpace = 0;
	for (int j = 0; s2[j]; j++)
		if (s2[j] == ' ')
			nSpace++;
	if ((r = contains(s2,'w')) != -1)
		w = bin[r] - '0';
	if ((r = contains(s2,'d')) != -1)
		d = bin[r] - '0';
	if ((r = contains(s2,'s')) != -1)
		s = bin[r] - '0';
	if (contains(s2,'i') != -1)
		if (!w)
			nSpace--;
	if (contains(s2,'I') != -1)
		if (!(w && !s))
			nSpace--;
	


	// Finding the mod, reg and r/m
	if ((r = contains(s2,'m')) != -1)
		getValue(bin,&mod,r,2);
	if ((r = contains(s2,'r')) != -1)
		getValue(bin,&reg,r,3);
	if ((r = contains(s2,'R')) != -1)
		getValue(bin,&rm,r,3);
	int posDisp = r+3;
		// Check for mod to see if further growth of the size is necessary
	if (!strcmp(mod,"01"))
		nSpace++;
	else if (!strcmp(mod,"10"))
		nSpace+=2;
	else if (!strcmp(mod,"00") && !strcmp(rm,"110"))
		nSpace+=2;
	if (z)
		nSpace++;
	changeByteSize(text, offset, &bin, nSpace+1);

	// Finding the data
	if ((r = contains(s2,'i')) != -1 && w)
		getData(bin,&wdata,nSpace*8);
	else if ((r = contains(s2,'I')) != -1 && w && !s)
		getData(bin,&wdata,nSpace*8);
	if ((r = contains(s2,'D')) != -1)
		getData(bin,&data,!strcmp(wdata,"")?nSpace*8:(nSpace-1)*8);
	// Specific cases
	if (n || t || a || f || o || q || j || p || z)
		getData(bin,&Pos2,8);
	if (A || F || O || Q || J)
		getData(bin,&Pos3,16);

	

	/// Interpreting the values
	char* brack1 = "[";
	char* brack2 = "]";
	char* byte = "byte ";
	int addi = 0;
	char* plus = "";
	sData = binStrToHexStr(data);
	sWData = binStrToHexStr(wdata);
	sPos2 = binStrToHexStr(Pos2);
	sPos3 = binStrToHexStr(Pos3);
	// Mod
	if (!strcmp(mod,"11"))
	{
		sEA = getReg(rm,w);
		brack1 = "";
		brack2 = "";
		byte = "";
	}
	else
	{
		sEA = getEA(rm,mod,&addi);
		if (addi && strcmp(mod,"00"))
		{
			plus = "+";
			getData(bin,&disp1,posDisp);
			if (!strcmp(mod,"10"))
			{
				getData(bin,&disp2,posDisp+8);
				if (disp2[0] == '1')
				{
					plus = "-";
					int verif = 0;
					for (int j = 7; j >= 0; j--)
					{
						if (verif)
						{
							if (disp1[j] == '1')
								disp1[j] = '0';
							else
								disp1[j] = '1';
						}
						else
							if (disp1[j] == '1')
								verif = 1;
					}
					for (int j = 7; j >= 0; j--)
					{
						if (verif)
						{
							if (disp2[j] == '1')
								disp2[j] = '0';
							else
								disp2[j] = '1';
						}
						else
							if (disp2[j] == '1')
								verif = 1;
					}


				}
			}
			else if (disp1[0] == '1')
			{
				plus = "-";
				int verif = 0;
				for (int j = 7; j >= 0; j--)
				{
					if (verif)
					{
						if (disp1[j] == '1')
							disp1[j] = '0';
						else
							disp1[j] = '1';
					}
					else
						if (disp1[j] == '1')
							verif = 1;
				}
				}
		}
	}
	
	if (!strcmp(sEA,"d"))
	{
		getData(bin,&disp1,posDisp);
		getData(bin,&disp2,posDisp+8);
	}
	sEAAddi1 = binStrToHexStr(disp1);
	sEAAddi2 = binStrToHexStr(disp2);
	
	// Reg
	sReg = getReg(reg,w);
	
	// Printing
	if (!strcmp(sEA,""))
	{
		brack1 = "";
		brack2 = "";
		byte = "";
	}
		if (w != 0)
		byte = "";

	
	if (!strcmp(sEA,"d"))
		sEA = "";
		
	if (startsBy(bin,"1100011___000")
			|| startsBy(bin,"100000")
			|| startsBy(bin,"1111011")
			|| startsBy(bin,"1010100"))
	{
		if (!w)
			removeStartZero(sData, sWData);		
	}	

	if (strcmp(mod,"00") || strcmp(rm,"110"))
		removeStartZero(sEAAddi1,sEAAddi2);
		
		
	char *ogCode = binStrToHexStr(bin);
	char *pre;
	asprintf(&pre,"%04x:%-13s",offset,ogCode);	

	if (strcmp(reg,"") && strcmp(rm,"")) // Inst with reg && r/m
	{
		if (d && strcmp(sInst,"xchg"))
			asprintf(ind,"%s%s %s, %s%s%s%s%s%s\n",pre,sInst,sReg,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2);
		else
			asprintf(ind,"%s%s %s%s%s%s%s%s, %s\n",pre,sInst,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,sReg);
	}
	else if (strcmp(rm,"")) // Inst with r/m only
	{	
		if (s != -1)
		{
			int tmp = HexStrToInt(sData,sWData,s);
			if (s)
				asprintf(ind,"%s%s %s%s%s%s%s%s, %s%x\n",pre,sInst,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,tmp<0?"-":"",abs(tmp));
			else
			{
				if ((startsBy(bin,"1100011") || startsBy(bin,"100000") || startsBy(bin,"1111011") || startsBy(bin,"1010100")) && !strcmp(sWData,"") && tmp < 16)
					asprintf(ind,"%s%s %s%s%s%s%s%s%s, %01x\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,abs(tmp));
				else if ((startsBy(bin,"1100011") || startsBy(bin,"100000") || startsBy(bin,"1111011") || startsBy(bin,"1010100")) && !strcmp(sWData,""))
					asprintf(ind,"%s%s %s%s%s%s%s%s%s, %02x\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,abs(tmp));
				else if (startsBy(bin,"1100011") || startsBy(bin,"100000") || startsBy(bin,"1111011") || startsBy(bin,"1010100"))
					asprintf(ind,"%s%s %s%s%s%s%s%s%s, %04x\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,abs(tmp));
				else 
					asprintf(ind,"%s%s %s%s%s%s%s%s%s, %x\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,tmp);
			}
		}
		else if (v != -1)
			if (v)
				asprintf(ind,"%s%s %s%s%s%s%s%s%s, cl\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2);
			else
				asprintf(ind,"%s%s %s%s%s%s%s%s%s, 1\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2);
		else if (!strcmp(data,""))
			asprintf(ind,"%s%s %s%s%s%s%s%s%s\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2);
		else
			if (d)
				asprintf(ind,"%s%s %s%s%s%s%s%s%s, %s%s\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,sWData,sData);
			else
				asprintf(ind,"%s%s %s%s%s, %s%s%s%s%s%s\n",pre,sInst,byte,sWData,sData,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2);
		}
	else if (strcmp(reg,"")) // Inst with reg only
	{
		if (startsBy(bin,"10010"))
			asprintf(ind,"%s%s %s, ax\n",pre,sInst,sReg);
		else if (!strcmp(data,""))
			asprintf(ind,"%s%s %s\n",pre,sInst, sReg);
		else
			asprintf(ind,"%s%s %s, %s%s\n",pre,sInst, sReg, sWData, sData);
	}
	else // Inst without reg nor r/m
	{
		if (t)
			asprintf(ind,"%s%s %s\n",pre,sInst,sPos2);
		else if (n || f || o || q) 
		{
			//asprintf(ind,"%s ",sInst);
			char* sShort = "";
		 	if (!strcmp(sInst,"jmp") && n) // short jmp
				sShort = " short";
			int j = BinStrToInt(Pos2,Pos3);
			if (f)
				j+=1;
			//printf("%04x\n",i+j+2);
			
				asprintf(ind, "%s%s%s %04x\n",pre,sInst,sShort,offset+j+2);
		}
		else if (j) 
		{	
			asprintf(ind,"%s%s %s%s\n",pre,sInst,sPos3,sPos2);
		}
		else if (p)
		{
			asprintf(ind,"%s%s %s, %s\n",pre,sInst,w?"ax":"al",sPos2);
		}
		else if (z)
		{
			char *tmp = "";
			char *tmp2 = getInst(Pos2,&tmp);
			int w2 = -1;
			if ((r = contains(tmp2,'w')))
				w2 = Pos2[r] - '0';
			char *sW2 = "";
			if (w2==1)
				sW2 = "w";
			if (w2==0)
				sW2 = "b";
			asprintf(ind,"%s%s %s%s\n",pre,sInst,tmp,sW2);
		}
			else if (!strcmp(sInst,"in"))
			asprintf(ind,"%s%s %s, dx\n",pre,sInst,w?"ax":"al");

		else if (strcmp(sWData,"") || strcmp(sData,""))
			asprintf(ind,"%s%s %s, %s%s\n",pre,sInst,w?"ax":"al",sWData,sData);
		else
			asprintf(ind,"%s%s\n",pre,sInst);
	}
	free(sData);
	free(sWData);
	free(sEAAddi1);
	free(sEAAddi2);
	free(sPos2);
	free(sPos3);
	free(mod);
	free(reg);
	free(rm);
	
	free(ogCode);
	free(pre);
	free(data);
	free(wdata);
	free(Pos2);
	free(Pos3);
	free(disp1);
	free(disp2);

	free(bin);
	*returnSize = 1 + nSpace;
	return *ind;
}

char** disassembler(char *text,int *sizeDisa)
{
	//Getting the size of the code (in byte)
	int size = 0;
	int power = 1;
	for (int i = 0; i < 4; i++)
	{
		size += ((text[8+i]%256)+256)%256 * power;
		power *= 256;
	}
	*sizeDisa = size;


	//Finding all pieces of code
	text += 32;	
	char **list = malloc(0);
	int i = 0;
	char **resList = calloc(1,sizeof(char*));
	int resListSize = 0;
	while (i < size)
	{
		resListSize++;
		resList = realloc(resList,(resListSize+1)*sizeof(char*));
		resList[resListSize] = 0;

		char **ind = resList + resListSize - 1;

		// Creating the return variables
		char *sInst = "";
		char *sReg = "";
		char *sEA = "";
		char *sEAAddi1 = "";
		char *sEAAddi2 = "";
		char *sData = "";
		char *sWData = "";

		char *sPos2 = "";
		char *sPos3 = "";

		// Getting the first byte of the instruction
		char *bin = getBinChar(text[i]);
		if (i == size-1)
		{
			bin[4] = 0;
			bin[5] = 0;
			bin[6] = 0;
			bin[7] = 0;
		}
		char *s2 = getInst(bin, &sInst);
		
		// Set up of the variables
		int w = 1;
		int d = 1;
		int s = -1;

		int n = 0;
		int t = 0;
		int a = 0;
		int A = 0;
		int f = 0;
		int F = 0;
		int o = 0;
		int O = 0;
		int q = 0;
		int Q = 0;
		int j = 0;
		int J = 0;
		int v = -1;
		int p = 0;
		int z = 0;


		char *mod = calloc(3,sizeof(char));
		char *reg = calloc(4,sizeof(char)); 
		char *rm = calloc(4,sizeof(char));
		char *data = calloc(1,sizeof(char));
		char *wdata = calloc(1,sizeof(char));
		char *disp1 = calloc(1,sizeof(char));
		char *disp2 = calloc(1,sizeof(char));
		char *Pos2 = calloc(1,sizeof(char));
		char *Pos3 = calloc(1,sizeof(char));


		// Checking the size that takes the instructions (in bytes)
		int nSpace = 0;
		for (int j = 0; s2[j] && i+nSpace < size; j++)
			if (s2[j] == ' ')
				nSpace++;

		// Updating the variables and the size of the instructions
		n = contains(s2,'n') != -1;
		t = contains(s2,'t') != -1;
		a = contains(s2,'a') != -1;
		A = contains(s2,'A') != -1;
		f = contains(s2,'f') != -1;
		F = contains(s2,'F') != -1;
		o = contains(s2,'o') != -1;
		O = contains(s2,'O') != -1;
		q = contains(s2,'q') != -1;
		Q = contains(s2,'Q') != -1;
		j = contains(s2,'j') != -1;
		J = contains(s2,'J') != -1;
		p = contains(s2,'p') != -1;
		z = contains(s2,'z') != -1;
		
		int r;
		if ((r = contains(s2,'w')) != -1)
			w = bin[r] - '0';
		if ((r = contains(s2,'d')) != -1)
			d = bin[r] - '0';
		if ((r = contains(s2,'v')) != -1)
			v = bin[r] - '0';
		if (contains(s2,'i') != -1 && i+nSpace < size)
			if (!w)
				nSpace--;
		if (contains(s2,'I') != -1 && i+nSpace < size)
			if (!(w && !s))
				nSpace--;
		
		changeByteSize(text, i, &bin, nSpace+1);


		// Making sure it is the good instructions with the new bytes

		getInst2(bin, &s2, &sInst, nSpace+1);

		nSpace = 0;
		for (int j = 0; s2[j] && i+nSpace < size; j++)
			if (s2[j] == ' ')
				nSpace++;
		if ((r = contains(s2,'w')) != -1)
			w = bin[r] - '0';
		if ((r = contains(s2,'d')) != -1)
			d = bin[r] - '0';
		if ((r = contains(s2,'s')) != -1)
			s = bin[r] - '0';
		if (contains(s2,'i') != -1)
			if (!w)
				nSpace--;
		if (contains(s2,'I') != -1)
			if (!(w && !s))
				nSpace--;
		




		// Finding the mod, reg and r/m
		if ((r = contains(s2,'m')) != -1)
			getValue(bin,&mod,r,2);
		if ((r = contains(s2,'r')) != -1)
			getValue(bin,&reg,r,3);
		if ((r = contains(s2,'R')) != -1)
			getValue(bin,&rm,r,3);
		int posDisp = r+3;

		// Check for mod to see if further growth of the size is necessary
		if (!strcmp(mod,"01") && i+nSpace < size)
			nSpace++;
		else if (!strcmp(mod,"10") && i+nSpace < size)
			nSpace+=2;
		else if (!strcmp(mod,"00") && !strcmp(rm,"110") && i+nSpace < size)
			nSpace+=2;

		if (z)
			nSpace++;

		changeByteSize(text, i, &bin, nSpace+1);


		// Finding the data
		if ((r = contains(s2,'i')) != -1 && w)
			getData(bin,&wdata,nSpace*8);
		else if ((r = contains(s2,'I')) != -1 && w && !s)
			getData(bin,&wdata,nSpace*8);
		if ((r = contains(s2,'D')) != -1)
			getData(bin,&data,!strcmp(wdata,"")?nSpace*8:(nSpace-1)*8);

		// Specific cases
		if (n || t || a || f || o || q || j || p || z)
			getData(bin,&Pos2,8);
		if (A || F || O || Q || J)
			getData(bin,&Pos3,16);
	
		



		/// Interpreting the values

		char* brack1 = "[";
		char* brack2 = "]";
		char* byte = "byte ";
		int addi = 0;
		char* plus = "";
		sData = binStrToHexStr(data);
		sWData = binStrToHexStr(wdata);
		sPos2 = binStrToHexStr(Pos2);
		sPos3 = binStrToHexStr(Pos3);
		// Mod
		if (!strcmp(mod,"11"))
		{
			sEA = getReg(rm,w);
			brack1 = "";
			brack2 = "";
			byte = "";
		}
		else
		{
			sEA = getEA(rm,mod,&addi);
			if (addi && strcmp(mod,"00"))
			{
				plus = "+";
				getData(bin,&disp1,posDisp);
				if (!strcmp(mod,"10"))
				{
					getData(bin,&disp2,posDisp+8);
					if (disp2[0] == '1')
					{
						plus = "-";

						int verif = 0;
						for (int j = 7; j >= 0; j--)
						{
							if (verif)
							{
								if (disp1[j] == '1')
									disp1[j] = '0';
								else
									disp1[j] = '1';
							}
							else
								if (disp1[j] == '1')
									verif = 1;
						}
						for (int j = 7; j >= 0; j--)
						{
							if (verif)
							{
								if (disp2[j] == '1')
									disp2[j] = '0';
								else
									disp2[j] = '1';
							}
							else
								if (disp2[j] == '1')
									verif = 1;
						}




					}
				}
				else if (disp1[0] == '1')
				{
					plus = "-";
					int verif = 0;
					for (int j = 7; j >= 0; j--)
					{
						if (verif)
						{
							if (disp1[j] == '1')
								disp1[j] = '0';
							else
								disp1[j] = '1';
						}
						else
							if (disp1[j] == '1')
								verif = 1;
					}

				}
			}
		}
		
		if (!strcmp(sEA,"d"))
		{
			getData(bin,&disp1,posDisp);
			getData(bin,&disp2,posDisp+8);
		}

		sEAAddi1 = binStrToHexStr(disp1);
		sEAAddi2 = binStrToHexStr(disp2);
		
		// Reg

		sReg = getReg(reg,w);
		
		// Printing
		if (!strcmp(sEA,""))
		{
			brack1 = "";
			brack2 = "";
			byte = "";
		}

		if (w != 0)
			byte = "";
	
		
		if (!strcmp(sEA,"d"))
			sEA = "";

		

		if (startsBy(bin,"1100011___000")
				|| startsBy(bin,"100000")
				|| startsBy(bin,"1111011")
				|| startsBy(bin,"1010100"))
		{
			if (!w)
				removeStartZero(sData, sWData);		
		}	


		if (strcmp(mod,"00") || strcmp(rm,"110"))
			removeStartZero(sEAAddi1,sEAAddi2);

		

		
		char *ogCode = binStrToHexStr(bin);
		char *pre;
		asprintf(&pre,"%04x:%-13s",i,ogCode);	


		if (strcmp(reg,"") && strcmp(rm,"")) // Inst with reg && r/m
		{
			if (d && strcmp(sInst,"xchg"))
				asprintf(ind,"%s%s %s, %s%s%s%s%s%s\n",pre,sInst,sReg,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2);
			else
				asprintf(ind,"%s%s %s%s%s%s%s%s, %s\n",pre,sInst,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,sReg);
		}
		else if (strcmp(rm,"")) // Inst with r/m only
		{	
			if (s != -1)
			{
				int tmp = HexStrToInt(sData,sWData,s);
				if (s)
					asprintf(ind,"%s%s %s%s%s%s%s%s, %s%x\n",pre,sInst,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,tmp<0?"-":"",abs(tmp));
				else
				{
					if ((startsBy(bin,"1100011") || startsBy(bin,"100000") || startsBy(bin,"1111011") || startsBy(bin,"1010100")) && !strcmp(sWData,"") && tmp < 16)
						asprintf(ind,"%s%s %s%s%s%s%s%s%s, %01x\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,abs(tmp));
					else if ((startsBy(bin,"1100011") || startsBy(bin,"100000") || startsBy(bin,"1111011") || startsBy(bin,"1010100")) && !strcmp(sWData,""))
						asprintf(ind,"%s%s %s%s%s%s%s%s%s, %02x\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,abs(tmp));
					else if (startsBy(bin,"1100011") || startsBy(bin,"100000") || startsBy(bin,"1111011") || startsBy(bin,"1010100"))
						asprintf(ind,"%s%s %s%s%s%s%s%s%s, %04x\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,abs(tmp));
					else 
						asprintf(ind,"%s%s %s%s%s%s%s%s%s, %x\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,tmp);
				}
			}
			else if (v != -1)
				if (v)
					asprintf(ind,"%s%s %s%s%s%s%s%s%s, cl\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2);
				else
					asprintf(ind,"%s%s %s%s%s%s%s%s%s, 1\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2);
			else if (!strcmp(data,""))
				asprintf(ind,"%s%s %s%s%s%s%s%s%s\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2);
			else
				if (d)
					asprintf(ind,"%s%s %s%s%s%s%s%s%s, %s%s\n",pre,sInst,byte,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2,sWData,sData);
				else
					asprintf(ind,"%s%s %s%s%s, %s%s%s%s%s%s\n",pre,sInst,byte,sWData,sData,brack1,sEA,plus,sEAAddi2,sEAAddi1,brack2);

		}
		else if (strcmp(reg,"")) // Inst with reg only
		{
			if (startsBy(bin,"10010"))
				asprintf(ind,"%s%s %s, ax\n",pre,sInst,sReg);
			else if (!strcmp(data,""))
				asprintf(ind,"%s%s %s\n",pre,sInst, sReg);
			else
				asprintf(ind,"%s%s %s, %s%s\n",pre,sInst, sReg, sWData, sData);
		}
		else // Inst without reg nor r/m
		{
			if (t)
				asprintf(ind,"%s%s %s\n",pre,sInst,sPos2);
			else if (n || f || o || q) 
			{
				//asprintf(ind,"%s ",sInst);
				char* sShort = "";
			 	if (!strcmp(sInst,"jmp") && n) // short jmp
					sShort = " short";
				int j = BinStrToInt(Pos2,Pos3);
				if (f)
					j+=1;
				//printf("%04x\n",i+j+2);
				

				asprintf(ind, "%s%s%s %04x\n",pre,sInst,sShort,i+j+2);
			}
			else if (j) 
			{	
				asprintf(ind,"%s%s %s%s\n",pre,sInst,sPos3,sPos2);
			}
			else if (p)
			{
				asprintf(ind,"%s%s %s, %s\n",pre,sInst,w?"ax":"al",sPos2);
			}
			else if (z)
			{
				char *tmp = "";
				char *tmp2 = getInst(Pos2,&tmp);
				int w2 = -1;
				if ((r = contains(tmp2,'w')))
					w2 = Pos2[r] - '0';
				char *sW2 = "";
				if (w2==1)
					sW2 = "w";
				if (w2==0)
					sW2 = "b";
				asprintf(ind,"%s%s %s%s\n",pre,sInst,tmp,sW2);
			}

			else if (!strcmp(sInst,"in"))
				asprintf(ind,"%s%s %s, dx\n",pre,sInst,w?"ax":"al");
	
			else if (strcmp(sWData,"") || strcmp(sData,""))
				asprintf(ind,"%s%s %s, %s%s\n",pre,sInst,w?"ax":"al",sWData,sData);
			else
				asprintf(ind,"%s%s\n",pre,sInst);
		}

		free(sData);
		free(sWData);
		free(sEAAddi1);
		free(sEAAddi2);
		free(sPos2);
		free(sPos3);

		free(mod);
		free(reg);
		free(rm);
		
		free(ogCode);
		free(pre);
		free(data);
		free(wdata);
		free(Pos2);
		free(Pos3);
		free(disp1);
		free(disp2);

		free(bin);
		
		i += 1 + nSpace;
	}

	free(list);
	return resList;
}

char* binStrToHexStr(char *bin)
{
	int size = 0;
	for (int i = 0; bin[i]; i+=4)
		size++;
	char* res = calloc(size+1,sizeof(char));

	for (int i = 0; bin[i]; i+=4)
	{
		int n = (bin[i]-'0') * 8;
		n += (bin[i+1]-'0') * 4;
		n += (bin[i+2]-'0') *2;
		n += (bin[i+3]-'0');
		int c = n>9?n+'a'-10:n+'0';
		res[i/4] = c;
	}
	return res;
}

int same(char* s1, char *s2)
{
	return !strcmp(s1,s2);
}

char intToHexChar(int n)
{
	if (n < 0)
		n += 16;
	if (n <= 9)
		return n + '0';
	return n + 'a' - 10;
}

int hexNegative(char* data1, char* data2)
{
	if (strcmp(data2,""))
		return data2[0]-'0';
	if (strcmp(data1,""))
		return data1[0]-'0';
	return 0;
}

int BinStrToInt(char* data1, char* data2)
{
	int m = 1;
	int res = 0;
	if (strcmp(data2,"") && data2[0] == '1')
	{
		m = -1;
		int verif = 0;
		for (int j = 7; j >= 0; j--)
		{
			if (verif)
			{
				if (data1[j] == '1')
					data1[j] = '0';
				else
					data1[j] = '1';
			}
			else if (data1[j] == '1')
				verif = 1;
		}
		for (int j = 7; j >= 0; j--)
		{
			if (verif)
			{
				if (data2[j] == '1')
					data2[j] = '0';
				else
					data2[j] = '1';
			}
			else
				if (data2[j] == '1')
					verif = 1;
		}
	}
	else if (!strcmp(data2,"") && data1[0] == '1')
	{
		m = -1;
		int verif = 0;
		for (int j = 7; j >= 0; j--)
		{
			if (verif)
			{
				if (data1[j] == '1')
					data1[j] = '0';
				else
					data1[j] = '1';
			}
			else if (data1[j] == '1')
				verif = 1;
		}
	}
	int p = 1;
	for (int j = 7; j >= 0; j--)
	{
		res += (data1[j]-'0') * p;
		p *= 2;
	}
	if (strcmp(data2,""))
	{
		for (int j = 7; j >= 0; j--)
		{
			res += (data2[j]-'0') * p;
			p *= 2;
		}
	}
	
	return res*m;
}

int BinStrToUnsignedInt(char* data1, char* data2)
{
	int res = 0;
	int p = 1;
	for (int j = 7; j >= 0; j--)
	{
		res += (data1[j]-'0') * p;
		p *= 2;
	}
	if (strcmp(data2,""))
	{
		for (int j = 7; j >= 0; j--)
		{
			res += (data2[j]-'0') * p;
			p *= 2;
		}
	}
	return res;
}


int HexStrToInt(char* data1, char* data2, int s)
{
	int res = 0;
	char *tmp1 = calloc(9,sizeof(char));
	char *tmp2 = calloc(9,sizeof(char));
	if (strcmp(data1,"") || strcmp(data2,""))
	{
		if (strcmp(data2,""))
		{
			int c1 = data2[0]>='a'?data2[0]-'a'+10:data2[0]-'0';
			int c2 = data2[1]>='a'?data2[1]-'a'+10:data2[1]-'0';
			if (data2[1] == 0)
			{
				tmp2[0] = '0';
				tmp2[1] = '0';
				tmp2[2] = '0';
				tmp2[3] = '0';
				tmp2[4] = (c1/8)%2 +'0';
				tmp2[5] = (c1/4)%2 +'0';
				tmp2[6] = (c1/2)%2 +'0';
				tmp2[7] = (c1)%2 +'0';
			}
			else {
				tmp2[0] = (c1/8)%2 +'0';
				tmp2[1] = (c1/4)%2 +'0';
				tmp2[2] = (c1/2)%2 +'0';
				tmp2[3] = (c1)%2 +'0';
				tmp2[4] = (c2/8)%2 +'0';
				tmp2[5] = (c2/4)%2 +'0';
				tmp2[6] = (c2/2)%2 +'0';
				tmp2[7] = (c2)%2 +'0';
			}
		}
		int c3 = data1[0]>='a'?data1[0]-'a'+10:data1[0]-'0';
		int c4 = data1[1]>='a'?data1[1]-'a'+10:data1[1]-'0';
		if (data1[1] == 0)
		{
			tmp1[0] = '0';
			tmp1[1] = '0';
			tmp1[2] = '0';
			tmp1[3] = '0';
			tmp1[4] = (c3/8)%2 +'0';
			tmp1[5] = (c3/4)%2 +'0';
			tmp1[6] = (c3/2)%2 +'0';
			tmp1[7] = (c3)%2 + '0';
		}
		else {
			tmp1[0] = (c3/8)%2 +'0';
			tmp1[1] = (c3/4)%2 +'0';
			tmp1[2] = (c3/2)%2 + '0';
			tmp1[3] = (c3)%2 +'0';
			tmp1[4] = (c4/8)%2 +'0';
			tmp1[5] = (c4/4)%2 +'0';
			tmp1[6] = (c4/2)%2 +'0';
			tmp1[7] = (c4)%2 + '0';
		}
		if (s)
			res = BinStrToInt(tmp1,tmp2);
		else
			res = BinStrToUnsignedInt(tmp1,tmp2);
	}
	free(tmp1);
	free(tmp2);
	return res;
}

void removeStartZero(char *data1, char *data2)
{
	if (strcmp(data2,""))
	{
		if (!strcmp(data2,"00"))
		{
			data2[0] = 0;
			data2[1] = 0;
		}
		else if (data2[0] != '0')
			return;
		else 
		{
			data2[0] = data2[1];
			data2[1] = 0;
			return;
		}
	}
	if (!strcmp(data1,"00") && strcmp(data2,""))
	{
		data1[0] = 0;
		data1[1] = 0;
	}
	else if (data1[0] != '0')
		return;
	else 
	{
		data1[0] = data1[1];
		data1[1] = 0;
	}
}

// Function to check if a string starts by another string given in the arguments.
// The use of the character '_' permits to skip some in the original string
int startsBy(char *string, char*start)
{
	int i = 0;
	for (; string[i] && start[i]; i++)
	{
		if (string[i] != start[i] && start[i] != '_')
			return 0;
	}
	if (start[i] == 0)
		return 1;
	if (string[i] == 0)
		return 0;
	return 1;
}

// Function to get the 2 hexodecimal number of the ascii of a character
void getHexChar(char c, int* c1, int* c2)
{
	*c1 = (c<0?256+c:c) / 16;
	*c2 = (c<0?256+c:c) % 16;	
}

// Function to get the binary number of the ascii of a character
char* getBinChar(char c)
{
	int c1;
	int c2;
	getHexChar(c,&c1,&c2);
	char *string;
	asprintf(&string,"%04b%04b",c1,c2);
	return string;
}

// Check if a character is present in the string, made to decrypt the architecture
// of an instruction
int contains(char *s, char c)
{
	int n = 0;
	for (int i = 0; s[i]; i++)
	{
		if (s[i] == c)
			return n;
		if (s[i] == ' ')
			n--;
		else if (s[i] == 'm')
			n += 1;
		else if (s[i] == 'r' || s[i] == 'R')
			n += 2;
		else if (s[i] == 'i' || s[i] == 'I' || s[i] == 'D' || s[i] == 'n' 
				|| s[i] == 'f' || s[i] == 'F' || s[i] == 'p' || s[i] == 't'
				|| s[i] == 'a' || s[i] == 'A' || s[i] == 'o' || s[i] == 'O'
				|| s[i] == 'q' || s[i] == 'Q' || s[i] == 'j' || s[i] == 'J')
			n += 7;
		n++;
	}
	return -1;
}

// Functions that returns a part of a string 
void getValue(char *org, char**res, int pos, int size)
{
	for (int i = pos; i < pos + size; i++)
		(*res)[i-pos] = org[i];
	(*res)[size] = 0;
}

// Functions that gets the data
void getData(char *org, char **res, int pos)
{
	char *tmp = calloc(9,sizeof(char));
	tmp[0] = org[pos];
	tmp[1] = org[pos+1];
	tmp[2] = org[pos+2];
	tmp[3] = org[pos+3];
	tmp[4] = org[pos+4];
	tmp[5] = org[pos+5];
	tmp[6] = org[pos+6];
	tmp[7] = org[pos+7];
	free(*res);
	*res = tmp;
}

// Function to change the size of an instructions to take all the bytes necessary
void changeByteSize(char *code, int index, char **s, int newByteSize)
{
	free(*s);
	char *tmp = calloc(newByteSize*8+1,sizeof(char));
	for (int i = index; i < index + newByteSize; i++)
	{
		char *binChar = getBinChar(code[i]);
		tmp[(i-index)*8] = binChar[0];
		tmp[(i-index)*8+1] = binChar[1];
		tmp[(i-index)*8+2] = binChar[2];
		tmp[(i-index)*8+3] = binChar[3];
		tmp[(i-index)*8+4] = binChar[4];
		tmp[(i-index)*8+5] = binChar[5];
		tmp[(i-index)*8+6] = binChar[6];
		tmp[(i-index)*8+7] = binChar[7];
		free(binChar);
	}
	*s = tmp;
}



/// Definition of all the reg, inst, and parameters of inst

char *getReg(char *string, int w)
{
	if (!strcmp(string,"000"))
		return w?"ax":"al";
	if (!strcmp(string,"001"))
		return w?"cx":"cl";
	if (!strcmp(string,"010"))
		return w?"dx":"dl";
	if (!strcmp(string,"011"))
		return w?"bx":"bl";
	if (!strcmp(string,"100"))
		return w?"sp":"ah";
	if (!strcmp(string,"101"))
		return w?"bp":"ch";
	if (!strcmp(string,"110"))
		return w?"si":"dh";
	if (!strcmp(string,"111"))
		return w?"di":"bh";

	if (!strcmp(string,"01"))
		return "cs";
	if (!strcmp(string,"10"))
		return "ss";
	if (!strcmp(string,"11"))
		return "ds";
	if (!strcmp(string,"00"))
		return "es";
	return "";
}

char *getEA(char *rm, char *mod, int *addi)
{
	*addi = 1;
	if (!strcmp(rm,"110") && !strcmp(mod,"00"))
		return("d");

	if (!strcmp(rm,"000"))
		return("bx+si");
	if (!strcmp(rm,"001"))
		return("bx+di");
	if (!strcmp(rm,"010"))
		return("bp+si");
	if (!strcmp(rm,"011"))
		return("bp+di");
	if (!strcmp(rm,"100"))
		return("si");
	if (!strcmp(rm,"101"))
		return("di");
	if (!strcmp(rm,"110"))
		return("bp");
	if (!strcmp(rm,"111"))
		return("bx");

	*addi = 0;
	return "";
}






// d : 				d
// w : 				w
// mod : 			m
// reg :			r
// r/m : 			R
// data : 			D
// data if w = 1 : 		i
// data if w = 01 : 		I
// s : 				s
// v : 				v
// port : 			p
// addr-low : 			a
// addr-high : 			A
// disp: 			n
// disp_low: 			f
// disp_high: 			F
// offset_low: 			o
// offset_high: 		O
// seg_low : 			q
// seg_high : 			Q
// data_low : 			j
// data_high : 			J
// x : 				x
// type : 			t

char *getInst(char* bin, char **printedString)
{
	/*if (bin[2] == 0)
		*printedString = "(undeclared)";
		return "00";*/
	// Move                X
	if (startsBy(bin,"100010")){
		*printedString = "mov"; 
		return ("100010dw mrR");}
	if (startsBy(bin,"1100011")) {
		*printedString = "mov"; 
		return ("1100011w m000R D i");}
	if (startsBy(bin,"1011")){
		*printedString = "mov"; 
		return ("1011wr D i");}
	if (startsBy(bin,"1010000")){
		*printedString = "mov"; 
		return ("1010000w a A");}
	if (startsBy(bin,"1010001")){
		*printedString = "mov"; 
		return ("1010001w a A");}
	if (startsBy(bin,"10001110")){
		*printedString = "mov"; 
		return ("10001110 m0rR");}
	if (startsBy(bin,"10001100")){
		*printedString = "mov"; 
		return ("10001100 m0rR");}
	// Push                 X
	if (startsBy(bin,"11111111")){
		*printedString = "push"; 
		return ("11111111 m110R");}
	if (startsBy(bin,"01010")){
		*printedString = "push"; 
		return ("01010r");}
	if (startsBy(bin,"000__110")){
		*printedString = "push"; 
		return ("000r110");}
	// Pop                  X
	if (startsBy(bin,"10001111")){
		*printedString = "pop"; 
		return ("10001111 m000r");}
	if (startsBy(bin,"01011")){
		*printedString = "pop"; 
		return ("01011r");}
	if (startsBy(bin,"000__111")){
		*printedString = "pop"; 
		return ("000r111");}
	// Exchange
	if (startsBy(bin,"1000011")){
		*printedString = "xchg"; 
		return ("1000011w mrR");}
	if (startsBy(bin,"10010")){
		*printedString = "xchg"; 
		return ("10010r");}

	// Input from           X
	if (startsBy(bin,"1110010")){
		*printedString = "in"; 
		return ("1110010w p");}
	if (startsBy(bin,"1110110")){
		*printedString = "in"; 
		return ("1110110w");}
	// Output from
	if (startsBy(bin,"1110011")){
		*printedString = "out"; 
		return ("1110011w p");}
	if (startsBy(bin,"1110111")){
		*printedString = "out"; 
		return ("1110111w");}

	// XLAT
	if (startsBy(bin,"11010111")){
		*printedString = "xlat"; 
		return ("11010111");}
	// LEA                  X
	if (startsBy(bin,"10001101")){
		*printedString = "lea"; 
		return ("10001101 mrR");}
	// LDS
	if (startsBy(bin,"11000101")){
		*printedString = "lds"; 
		return ("11000101 mrR");}
	// LES
	if (startsBy(bin,"11000100")){
		*printedString = "les"; 
		return ("11000100 mrR");}
	// LAHF
	if (startsBy(bin,"10011111")){
		*printedString = "lahf"; 
		return ("10011111");}
	// SAHF
	if (startsBy(bin,"10011110")){
		*printedString = "sahf"; 
		return ("10011110");}
	// PUSHF
	if (startsBy(bin,"10011100")){
		*printedString = "pushf"; 
		return ("10011100");}
	// POPF
	if (startsBy(bin,"10011101")){
		*printedString = "popf"; 
		return ("10011101");}

	
	// Add                  X
	if (startsBy(bin,"000000")){
		*printedString = "add"; 
		return ("000000dw mrR");}
	if (startsBy(bin,"100000")){
		*printedString = "add"; 
		return ("100000sw m000R D I");}
	if (startsBy(bin,"0000010")){
		*printedString = "add"; 
		return ("0000010w D i");}
	// Add with carry
	if (startsBy(bin,"000100")){
		*printedString = "adc"; 
		return ("000100dw mrR");}
	if (startsBy(bin,"100000")){
		*printedString = "adc"; 
		return ("100000sw m010R D I");}
	if (startsBy(bin,"0001010")){
		*printedString = "adc"; 
		return ("0001010w D I");}
	// Increment
	if (startsBy(bin,"1111111")){
		*printedString = "inc"; 
		return ("1111111w m000R");}
	if (startsBy(bin,"01000")){
		*printedString = "inc"; 
		return ("01000r");}
	// AAA
	if (startsBy(bin,"00110111")){
		*printedString = "aaa"; 
		return ("00110111");}
	// BAA
	if (startsBy(bin,"00100111")){
		*printedString = "baa"; 
		return ("00100111");}
	// Subtract  	           X
	if (startsBy(bin,"001010")){
		*printedString = "sub"; 
		return ("001010dw mrR");}
	if (startsBy(bin,"100000")){
		*printedString = "sub"; 
		return ("100000sw m101R D I");}
	if (startsBy(bin,"0010110")){
		*printedString = "sub"; 
		return ("0010110w D i");}
	// Suctract with borrow X
	if (startsBy(bin,"000110")){
		*printedString = "sbb"; 
		return ("000110dw mrR");}
	if (startsBy(bin,"100000")){
		*printedString = "sbb"; 
		return ("100000sw m011R D I");}
	if (startsBy(bin,"000111")){
		*printedString = "sbb"; 
		return ("000111w D I");}
	// Decrement            X
	if (startsBy(bin,"1111111")){
		*printedString = "dec"; 
		return ("1111111w m001R");}
	if (startsBy(bin,"01001")){
		*printedString = "dec"; 
		return ("01001r");}
	// NEG                  X
	if (startsBy(bin,"1111011")){
		*printedString = "neg"; 
		return ("1111011w m011R");}
	// Compare              X
	if (startsBy(bin,"001110")){
		*printedString = "cmp"; 
		return ("001110dw mrR");}
	if (startsBy(bin,"100000")){
		*printedString = "cmp"; 
		return ("100000sw m111R D I");}
	if (startsBy(bin,"0011110")){
		*printedString = "cmp"; 
		return ("0011110 D i");}
	// AAS
	if (startsBy(bin,"00111111")){
		*printedString = "aas"; 
		return ("00111111");}
	// DAS
	if (startsBy(bin,"00101111")){
		*printedString = "das"; 
		return ("00101111");}
	// MUL
	if (startsBy(bin,"1111011")){
		*printedString = "mul"; 
		return ("1111011w m100R");}
	// IMUL
	if (startsBy(bin," 111011")){
		*printedString = "imul"; 
		return ("1111011w m101R");}
	// AAM
	if (startsBy(bin,"11010100")){
		*printedString = "amm"; 
		return ("1101010000001010");}
	// DIV
	if (startsBy(bin,"1111011")){
		*printedString = "div"; 
		return ("1111011w m110R");}
	// IDIV
	if (startsBy(bin,"1111011")){
		*printedString = "idiv"; 
		return ("1111011w m111R");}
	// AAD
	if (startsBy(bin,"11010101")){
		*printedString = "add"; 
		return ("1101010100001010");}
	// CBW
	if (startsBy(bin,"10011000")){
		*printedString = "cbw"; 
		return ("10011000");}
	// CWD
	if (startsBy(bin,"10011001")){
		*printedString = "cwd"; 
		return ("10011001");}

	
	// NOT
	if (startsBy(bin,"1111011")){
		*printedString = "not"; 
		return ("1111011w m010R");}

	// SHL/SAL              X
	if (startsBy(bin,"110100")){
		*printedString = "shl"; 
		return ("110100vw m100R");}
	// SHR
	if (startsBy(bin,"110100")){
		*printedString = "shr"; 
		return ("110100vw m101R");}
	// SAR
	if (startsBy(bin,"110100")){
		*printedString = "sar"; 
		return ("110100vw m111R");}
	// ROL
	if (startsBy(bin,"110100")){
		*printedString = "rol"; 
		return ("110100vw m000R");}
	// ROR
	if (startsBy(bin,"110100")){
		*printedString = "ror"; 
		return ("110100vw m001R");}
	// RCL
	if (startsBy(bin,"110100")){
		*printedString = "rcl"; 
		return ("110100vw m010R");}
	// RCR
	if (startsBy(bin,"110100")){
		*printedString = "rcr"; 
		return ("110100vw m011R");}
	// And                  X
	if (startsBy(bin,"001000")){
		*printedString = "and"; 
		return ("001000dw mrR");}
	if (startsBy(bin,"1000000")){
		*printedString = "and"; 
		return ("1000000w m100R D i");}
	if (startsBy(bin,"0010010")){
		*printedString = "and"; 
		return ("0010010w D i");}
	// TEST                 X
	if (startsBy(bin,"1000010")){
		*printedString = "test"; 
		return ("1000010w mrR");}
	if (startsBy(bin,"1111011")){
		*printedString = "test"; 
		return ("1111011w m000R D i");}
	if (startsBy(bin,"1010100")){
		*printedString = "test"; 
		return ("1010100w D i");}
	// Or                   X
	if (startsBy(bin,"000010")){
		*printedString = "or"; 
		return ("000010dw mrR");}
	if (startsBy(bin,"1000000")){
		*printedString = "or"; 
		return ("1000000w m001R D i");}
	if (startsBy(bin,"0000110")){
		*printedString = "or"; 
		return ("0000110w D i");}
	// Exclusive or         X
	if (startsBy(bin,"001100")){
		*printedString = "xor"; 
		return ("001100dw mrR");}
	if (startsBy(bin,"1000000")){
		*printedString = "xor"; 
		return ("1000000w m110R D i");}
	if (startsBy(bin,"0011010")){
		*printedString = "xor"; 
		return ("0011010w D i");}
	// REP
	if (startsBy(bin,"1111001")){
		*printedString = "rep"; 
		return ("1111001z");}
	// MOVS
	if (startsBy(bin,"1010010")){
		*printedString = "movs"; 
		return ("1010010w");}
	// CMPS
	if (startsBy(bin,"1010011")){
		*printedString = "cmps"; 
		return ("1010011w");}
	// SCAS
	if (startsBy(bin,"1010111")){
		*printedString = "scas"; 
		return ("1010111w");}
	// LODS
	if (startsBy(bin,"1010110")){
		*printedString = "lods"; 
		return ("1010110w");}
	// STOS
	if (startsBy(bin,"1010101")){
		*printedString = "stos"; 
		return ("1010101w");}
	// CALL                 X
	if (startsBy(bin,"11101000")){
		*printedString = "call"; 
		return ("11101000 f F");}
	if (startsBy(bin,"11111111")){
		*printedString = "call"; 
		return ("11111111 m010R");}
	if (startsBy(bin,"10011010")){
		*printedString = "call"; 
		return ("10011010 (o O)/(q Q)");}
	if (startsBy(bin,"11111111")){
		*printedString = "call"; 
		return ("11111111 m011R");}
	
	// Unconditional Jump   X
	if (startsBy(bin,"11101001")){
		*printedString = "jmp"; 
		return ("11101001 f F");}
	if (startsBy(bin,"11101011")){
		*printedString = "jmp"; 
		return ("11101011 n");}
	if (startsBy(bin,"11111111")){
		*printedString = "jmp"; 
		return ("11111111 m100R");}
	if (startsBy(bin,"11101010")){
		*printedString = "jmp"; 
		return ("11101010 (o O)/(q Q)");}
	if (startsBy(bin,"11111111")){
		*printedString = "jmp"; 
		return ("11111111 m101R");}
	// Return from call     X
	if (startsBy(bin,"11000011")){
		*printedString = "ret"; 
		return ("11000011");}
	if (startsBy(bin,"11000010")){
		*printedString = "ret"; 
		return ("11000010 j J");}
	if (startsBy(bin,"11001011")){
		*printedString = "ret"; 
		return ("11001011");}
	if (startsBy(bin,"11001010")){
		*printedString = "ret"; 
		return ("11001010 j J");}
	// JE/JZ                X
	if (startsBy(bin,"01110100")){
		*printedString = "je"; 
		return ("01110100 n");}
	// JL/JNGE              X
	if (startsBy(bin,"01111100")){
		*printedString = "jl"; 
		return ("01111100 n");}
	// JLE/JNG
	if (startsBy(bin,"01111110")){
		*printedString = "jle"; 
		return ("01111110 n");}
	// JB/JNAE
	if (startsBy(bin,"01110010")){
		*printedString = "jb"; 
		return ("01110010 n");}
	// JBE/JNA
	if (startsBy(bin,"01110110")){
		*printedString = "jbe"; 
		return ("01110110 n");}
	// JP/JPE
	if (startsBy(bin,"01111010")){
		*printedString = "jp"; 
		return ("01111010 n");}
	// JO
	if (startsBy(bin,"01110000")){
		*printedString = "jo"; 
		return ("01110000 n");}
	// JS
	if (startsBy(bin,"01111000")){
		*printedString = "js"; 
		return ("01111000 n");}
	// JNE/JNZ              X
	if (startsBy(bin,"01110101")){
		*printedString = "jne"; 
		return ("01110101 n");}
	// JNL/JGE              X
	if (startsBy(bin,"01111101")){
		*printedString = "jnl"; 
		return ("01111101 n");}
	// JNLE/JG
	if (startsBy(bin,"01111111")){
		*printedString = "jnle"; 
		return ("01111111 n");}
	// JNB/JAE              X
	if (startsBy(bin,"01110011")){
		*printedString = "jnb"; 
		return ("01110011 n");}
	// JNBE/JA
	if (startsBy(bin,"01110111")){
		*printedString = "jnbe"; 
		return ("01110111 n");}
	// JNP/JPO
	if (startsBy(bin,"01111011")){
		*printedString = "jnp"; 
		return ("01111011 n");}
	// JNO
	if (startsBy(bin,"01110001")){
		*printedString = "jno"; 
		return ("01110001 n");}
	// JNS
	if (startsBy(bin,"01111001")){
		*printedString = "jns"; 
		return ("01111001 n");}
	// LOOP
	if (startsBy(bin,"11100010")){
		*printedString = "loop"; 
		return ("11100010 n");}
	// LOOPZ/LOOPE
	if (startsBy(bin,"11100001")){
		*printedString = "loopz"; 
		return ("11100001 n");}
	// LOOPNZ/LOOPNE
	if (startsBy(bin,"11100000")){
		*printedString = "loopnz"; 
		return ("11100000 n");}
	// JCXZ
	// Interrupt
	if (startsBy(bin,"11001101")){
		*printedString = "int"; 
		return ("11001101 t");}
	if (startsBy(bin,"11001100")){
		*printedString = "int"; 
		return ("11001100");}
	// INTO
	if (startsBy(bin,"11001110")){
		*printedString = "into"; 
		return ("11001110");}
	// IRET
	if (startsBy(bin,"11001111")){
		*printedString = "iret"; 
		return ("11001111");}
	
	// CLC
	if (startsBy(bin,"11111000")){
		*printedString = "clc"; 
		return ("11111000");}
	// CMC
	if (startsBy(bin,"11110101")){
		*printedString = "cmc"; 
		return ("11110101");}
	// STC
	if (startsBy(bin,"11111001")){
		*printedString = "stc"; 
		return ("11111001");}
	// CLD
	if (startsBy(bin,"11111100")){
		*printedString = "cld"; 
		return ("11111100");}
	// STD
	if (startsBy(bin,"11111101")){
		*printedString = "std"; 
		return ("11111101");}
	// CLI
	if (startsBy(bin,"11111010")){
		*printedString = "cli"; 
		return ("11111010");}
	// STI
	if (startsBy(bin,"11111011")){
		*printedString = "sti"; 
		return ("11111011");}
	// HLT                 X
	if (startsBy(bin,"11110100")){
		*printedString = "hlt"; 
		return ("11110100");}
	// WAIT
	if (startsBy(bin,"10011011")){
		*printedString = "wait"; 
		return ("10011011");}
	// ESC
	if (startsBy(bin,"11011")){
		*printedString = "esc"; 
		return ("11011xxx mxxxR");}
	// LOCK
	if (startsBy(bin,"11110000")){
		*printedString = "lock"; 
		return ("11110000");}
	// (Undefinied)
	*printedString = "(undefined)";
	return "00";
}

void getInst2(char* bin, char **printedString, char **sInst, int size)
{
	if (!(same(*sInst,"push")
			|| same(*sInst,"add")
			|| same(*sInst,"sub")
			|| same(*sInst,"ssb")
			|| same(*sInst,"dec")
			|| same(*sInst,"neg")
			|| same(*sInst,"cmp")
			|| same(*sInst,"shl")
			|| same(*sInst,"and")
			|| same(*sInst,"test")
			|| same(*sInst,"or")
			|| same(*sInst,"xor")
			|| same(*sInst,"call")
			|| same(*sInst,"jmp")))
		return;
	
	//1111111_
	if (startsBy(bin, "1111111_") && size > 1)
	{
		if      (startsBy(bin,"1111111___000"))
		{
			*sInst = "inc"; 
			*printedString = "1111111w m000R";
		}
		else if (startsBy(bin,"1111111___001"))
		{
			*sInst = "dec"; 
			*printedString = "1111111w m001R";
		}
		else if (startsBy(bin,"11111111__010"))
		{
			*sInst = "call"; 
			*printedString = "11111111 m010R";
		}
		else if (startsBy(bin,"11111111__011"))
		{
			*sInst = "call"; 
			*printedString = "11111111 m011R";
		}
		else if (startsBy(bin,"11111111__100"))
		{
			*sInst = "jmp"; 
			*printedString = "11111111 m100R";
		}
		else if (startsBy(bin,"11111111__101"))
		{
			*sInst = "jmp"; 
			*printedString = "11111111 m101R";
		}
		else if (startsBy(bin,"11111111__110"))
		{
			*sInst = "push"; 
			*printedString = "11111111 m110R";
		}
	}

	//10000__
	else if (startsBy(bin, "100000__") && size > 1)
	{
		if      (startsBy(bin,"100000____000"))
		{
			*sInst= "add"; 
			*printedString = "100000sw m000R D I";
		}
		else if (startsBy(bin,"1000000___001"))
		{
			*sInst = "or"; 
			*printedString = "1000000w m001R D i";
		}
		else if (startsBy(bin,"1000000___010"))
		{
			*sInst = "adc"; 
			*printedString = "100000sw m010R D i";
		}
		else if (startsBy(bin,"100000____011"))
		{
			*sInst = "sbb"; 
			*printedString = "100000sw m011R D I";
		}
		else if (startsBy(bin,"1000000___100"))
		{
			*sInst = "and"; 
			*printedString =  "1000000w m100R D i";
		}	
		else if (startsBy(bin,"100000____101"))
		{
			*sInst = "sub"; 
			*printedString = "100000sw m101R D I";
		}
		else if (startsBy(bin,"1000000___110"))
		{
			*sInst = "xor"; 
			*printedString = "1000000w m110R D i";
		}
		else if (startsBy(bin,"100000____111"))
		{
			*sInst = "cmp"; 
			*printedString = "100000sw m111R D I";
		}
	}

	//1111011_
	else if (startsBy(bin, "1111011_") && size > 1)
	{
		if      (startsBy(bin,"1111011___000"))
		{
			*sInst = "test"; 
			*printedString = "1111011w m000R D i";
		}
		// ?                  "1111011___001"
		else if (startsBy(bin,"1111011___010"))
		{
			*sInst = "not"; 
			*printedString = "1111011w m010R";
		}
		else if (startsBy(bin,"1111011___011"))
		{
			*sInst = "neg"; 
			*printedString = "1111011w m011R";
		}
		else if (startsBy(bin,"1111011___100"))
		{
			*sInst = "mul"; 
			*printedString = "1111011w m100R";
		}
		else if (startsBy(bin,"1111011___101"))
		{
			*sInst = "imul"; 
			*printedString = "1111011w m101R";
		}
		else if (startsBy(bin,"1111011___110"))
		{
			*sInst = "div"; 
			*printedString = "1111011w m110R";
		}
		else if (startsBy(bin,"1111011___111"))
		{
			*sInst = "idiv"; 
			*printedString = "1111011w m111R";
		}
	}

	//110100__
	else if (startsBy(bin,"110100__") && size > 1)
	{
		if      (startsBy(bin,"110100____000"))
		{
			*sInst = "rol"; 
			*printedString = "110100vw m000R";
		}
		else if (startsBy(bin,"110100____001"))
		{
			*sInst = "ror"; 
			*printedString = "110100vw m001R";
		}
		else if (startsBy(bin,"110100____010"))
		{
			*sInst = "rcl"; 
			*printedString = "110100vw m010R";
		}
		else if (startsBy(bin,"110100____011"))
		{
			*sInst = "rcr"; 
			*printedString = "110100vw m011R";
		}
		else if (startsBy(bin,"110100____100"))
		{
			*sInst = "shl"; 
			*printedString = "110100vw m100R";
		}
		else if (startsBy(bin,"110100____101"))
		{
			*sInst = "shr"; 
			*printedString = "110100vw m101R";
		}
		// ?                   110100____110
		else if (startsBy(bin,"110100____111"))
		{
			*sInst = "sar"; 
			*printedString = "110100vw m111R";
		}
	}
}
