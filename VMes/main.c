#include <stdio.h> 
#include <stdlib.h>

// stack will have fixed size
#define STACK_SIZE 100
#define PUSH(vm, v) vm->stack[++vm->sp] = v // push value on top of the stack
#define POP(vm)     vm->stack[vm->sp--]     // pop value from top of the stack
#define NCODE(vm)   vm->code[vm->pc++]      // get next bytecode

typedef struct {
	int* locals;    // local scoped data
	int* code;      // array od byte codes to be executed
	int* stack;     // virtual stack
	int pc;         // program counter (aka. IP - instruction pointer)
	int sp;         // stack pointer
	int fp;         // frame pointer (for local scope)
} VM;

VM* newVM(int* code,    // pointer to table containing a bytecode to be executed  
	int pc,             // address of instruction to be invoked as first one - entrypoint/main func
	int datasize)
{      // total locals size required to perform a program operations
	VM* vm = (VM*)malloc(sizeof(VM));
	vm->code = code;
	vm->pc = pc;
	vm->fp = 0;
	vm->sp = -1;
	vm->locals = (int*)malloc(sizeof(int) * datasize);
	vm->stack = (int*)malloc(sizeof(int) * STACK_SIZE);

	return vm;
}

void delVM(VM* vm) {
	free(vm->locals);
	free(vm->stack);
	free(vm);
}

enum {
	ADD_I32 = 1,    // int add
	SUB_I32 = 2,    // int sub
	MUL_I32 = 3,    // int mul
	LT_I32 = 4,     // int less than
	EQ_I32 = 5,     // int equal
	JMP = 6,        // branch
	JMPT = 7,       // branch if true
	JMPF = 8,       // branch if false
	CONST_I32 = 9,  // push constant integer
	LOAD = 10,      // load from local
	GLOAD = 11,     // load from global
	STORE = 12,     // store in local
	GSTORE = 13,    // store in global memory
	PRINT = 14,     // print value on top of the stack
	POP = 15,       // throw away top of the stack
	HALT = 16,      // stop program
	CALL = 17,      // call procedure
	RET = 18,      // return from procedure
	PAUSE = 19		// pause program
};

void run(VM* vm) {
	do {
		int opcode = NCODE(vm);        // fetch
		int v, addr, offset, a, b, argc, rval;

		switch (opcode) {   // decode

		case HALT: 
			return;  // stop the program

		case CONST_I32:
			v = NCODE(vm);   // get next value from code ...
			PUSH(vm, v);     // ... and move it on top of the stack

			printf("\npushing constant int (%d)\n", v);

			break;

		case ADD_I32:
			b = POP(vm);        // get second value from top of the stack ...
			a = POP(vm);        // ... then get first value from top of the stack ...
			PUSH(vm, a + b);    // ... add those two values and put result on top of the stack

			printf("\nadding %d + %d (%d)\n", a, b, (a+b));

			break;

		case SUB_I32:
			b = POP(vm);        // get second value from top of the stack ...
			a = POP(vm);        // ... then get first value from top of the stack ...
			PUSH(vm, a - b);    // ... subtract those two values and put result on top of the stack

			printf("\nsubtracting %d - %d (%d)\n", a, b, (a-b));

			break;

		case MUL_I32:
			b = POP(vm);        // get second value from top of the stack ...
			a = POP(vm);        // ... then get first value from top of the stack ...
			PUSH(vm, a * b);    // ... multiply those two values and put result on top of the stack

			printf("\nmultiplying %d * %d (%d)\n", a, b, (a*b));

			break;

		case LT_I32:
			b = POP(vm);        // get second value from top of the stack ...
			a = POP(vm);        // ... then get first value from top of the stack ...
			PUSH(vm, (a<b) ? 1 : 0); // ... compare those two values, and put result on top of the stack

			printf("\n%d less than %d?", a, b);

			if (a < b)
			{
				printf(" (true)\n");
			}
			else
			{
				printf(" (false)\n");
			}

			break;

		case EQ_I32:
			b = POP(vm);        // get second value from top of the stack ...
			a = POP(vm);        // ... then get first value from top of the stack ...
			PUSH(vm, (a == b) ? 1 : 0); // ... compare those two values, and put result on top of the stack

			printf("\n%d equal to %d?", a, b);

			if (a == b)
			{
				printf(" (true)\n");
			}
			else
			{
				printf(" (false)\n");
			}

			break;

		case JMP:
			vm->pc = NCODE(vm);  // unconditionaly jump with program counter to provided address

			printf("\nbranch\n");

			break;

		case JMPT:
			addr = NCODE(vm);  // get address pointer from code ...
			if (POP(vm) == 1)
			{      // ... pop value from top of the stack, and if it's true ...
				vm->pc = addr; // ... jump with program counter to provided address

				printf("\nbranch true\n");
			}			

			break;

		case JMPF:
			addr = NCODE(vm);  // get address pointer from code ...
			if (POP(vm) == 0)
			{      // ... pop value from top of the stack, and if it's true ...
				vm->pc = addr; // ... jump with program counter to provided address

				printf("\nbranch false\n");
			}			

			break;

		case LOAD:                  // load local value or function arg  
			offset = NCODE(vm);     // get next value from code to identify local variables offset start on the stack
			PUSH(vm, vm->stack[vm->fp + offset]); // ... put on the top of the stack variable stored relatively to frame pointer

			printf("\nlocal load\n");

			break;

		case STORE:                 // store local value or function arg  
			v = POP(vm);            // get value from top of the stack ...
			offset = NCODE(vm);     // ... get the relative pointer address from code ...
			vm->locals[vm->fp + offset] = v;  // ... and store value at address received relatively to frame pointer

			printf("\nstore locally\n");

			break;

		case GLOAD:
			addr = POP(vm);             // get pointer address from code ...
			v = vm->locals[addr];         // ... load value from memory of the provided addres ...
			PUSH(vm, v);                // ... and put that value on top of the stack

			printf("\nload global\nvalue: %d\naddress: %d\n", v, addr);

			break;

		case GSTORE:
			v = POP(vm);                // get value from top of the stack ...
			addr = NCODE(vm);           // ... get pointer address from code ...
			vm->locals[addr] = v;         // ... and store value at address received

			printf("\nstore global\nvalue: %d\naddress: %d\n", v, addr);

			break;

		case CALL:
			// we expect all args to be on the stack
			addr = NCODE(vm); // get next instruction as an address of procedure jump ...
			argc = NCODE(vm); // ... and next one as number of arguments to load ...
			PUSH(vm, argc);   // ... save num args ...
			PUSH(vm, vm->fp); // ... save function pointer ...
			PUSH(vm, vm->pc); // ... save instruction pointer ...
			vm->fp = vm->sp;  // ... set new frame pointer ...
			vm->pc = addr;    // ... move instruction pointer to target procedure address

			printf("\nfunction call\n");

			break;

		case RET:
			rval = POP(vm);     // pop return value from top of the stack
			vm->sp = vm->fp;    // ... return from procedure address ...
			vm->pc = POP(vm);   // ... restore instruction pointer ...
			vm->fp = POP(vm);   // ... restore framepointer ...
			argc = POP(vm);     // ... hom many args procedure has ...
			vm->sp -= argc;     // ... discard all of the args left ...
			PUSH(vm, rval);     // ... leave return value on top of the stack

			printf("\nfunction return\n");

			break;

		case POP:
			--vm->sp;      // throw away value at top of the stack

			printf("\nthrowing away top of stack\n");

			break;

		case PRINT:
			v = POP(vm);        // pop value from top of the stack ...
			printf("\nPrinting top of stack: %d\n", v);  // ... and print it					

			break;

		case PAUSE:		
			do 
			{
				printf("\n\nPaused. Press enter to continue.\n"); // pause until enter is pressed
			} while (getchar() != '\n');

			break;

		default:
			break;
		}

	} while (1);
}

void main()
{
	const int state = 0;
	int program[] = {
		
		CONST_I32, 6,   
		GSTORE, state,
		CONST_I32, state,
		GLOAD,  
		PRINT, 
		CONST_I32, 69,
		GSTORE, state,
		CONST_I32, state,
		GLOAD,
		PRINT,
		PAUSE,			
		HALT      

	};

	// initialize virtual machine
	VM* vm = newVM(program,   // program to execute  
		0,    // start address of main function
		1);    // locals to be reserved
	run(vm);

	//delete vm
	delVM(vm);
}



