all : myInterpreter

myInterpreter: myInterpreter.o CPU.o disassembler.o
	gcc myInterpreter.o CPU.o disassembler.o -o myInterpreter -Wall -Wextra -g

CPU: CPU.o
	gcc CPU.o -o CPU -fsanitize=address -Wall -Wextra

myInterpreter.o: myInterpreter.c CPU.h
	gcc -c myInterpreter.c -o myInterpreter.o -Wall -Wextra

CPU.o : CPU.c CPU.h
	gcc -c CPU.c -o CPU.o -Wall -Wextra

disassembler.o: disassembler.c
	gcc -c disassembler.c -o disassembler.o -Wall -Wextra

clean :
	rm -f myInterpreter CPU myInterpreter.o CPU.o disassembler.o
