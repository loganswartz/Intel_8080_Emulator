#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "v8080.h"
#include "8080DisassembleLib.h"



int main(int argc, char* argv[])
{
	/*
	if(argc > 2) {
		printf("Only 1 arg expected.");
		exit(1);
	} else if(argc == 1) {
		printf("Please specify a file to run.");
		exit(1);
	}
	*/

	int halt = 0;
	int stepsize = 1548;
	int cycles = 0;
	int i = 1;
	CPUState8080* state = Init8080();
	ReadFileIntoMemoryAt(state, "games/space_invaders/invaders.h", 0);
	ReadFileIntoMemoryAt(state, "games/space_invaders/invaders.g", 0x800);
	ReadFileIntoMemoryAt(state, "games/space_invaders/invaders.f", 0x1000);
	ReadFileIntoMemoryAt(state, "games/space_invaders/invaders.e", 0x1800);

	while(halt == 0)
	{
		printf("\nOpcode = %x\n", state->memory[state->pc]);
		Disassemble8080(state);
		halt = Emulate8080(state);
		cycles++;
		printf("\tC=%d,P=%d,S=%d,Z=%d\n", state->cc.cy, state->cc.p, state->cc.s, state->cc.z);
		printf("\tA $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n", state->a, state->b, state->c, state->d, state->e, state->h, state->l, state->sp);
		if(i >= stepsize) {
			printf("Cycle count = %d\n", cycles);
			getchar();
			stepsize = 5;
			i = 1;
		} else {
			i++;
		}
	}

	exit(0);
}


int parity(int x, int size)
{
	int i;
	int p = 0;
	x = (x & ((1<<size)-1));
	for (i=0; i<size; i++)
	{
		if (x & 0x1) p++;
		x = x >> 1;
	}
	return (0 == (p & 0x1));
}


void UnimplementedInstruction(CPUState8080* state) {
	printf("Error: instruction not implemented");
	exit(1);
}

int Emulate8080(CPUState8080* state) {
	uint8_t* opcode = &state->memory[state->pc];
	int halt = 0;

	switch(*opcode) {
		case 0x00: break; // NOP
		case 0x01: // LXI B,word
		{
				state->b = opcode[2];
				state->c = opcode[1];
				state->pc += 2; // advance program counter past our 2 data bytes
				break;
		}
		case 0x02: UnimplementedInstruction(state); break; // STAX   B
		case 0x03: UnimplementedInstruction(state); break; // INX    B
		case 0x04: state->b += 1; break; // INR    B
		case 0x05:// DCR    B
			{
				EvalFlags(state, state->b-1);
				state->b -= 1;
				break;
			}
		case 0x06: // MVI    B,<data>
			{
				state->b = opcode[1];
				state->pc += 1;
				break;
			}
		case 0x07: UnimplementedInstruction(state); break; // RLC
		case 0x08: break; // NOP
		case 0x09: // DAD    B
			{
				uint16_t hldouble = (state->h<<8) | (state->l); // append l to h
				uint16_t rpdouble = (state->b<<8) | (state->c); // append l to h
				uint32_t result = (uint32_t) hldouble + (uint32_t) rpdouble;
				state->h = (result >> 8) & 0xff;
				state->l = result & 0xff;
				state->cc.cy = (result > 0xffff);
				break;
			}
		case 0x0a: // LDAX   B
		{
				uint16_t memaddress = (state->b<<8) | (state->c); // append l to h
				state->a = state->memory[memaddress];
				break;
		}
		case 0x0b: UnimplementedInstruction(state); break; // DCX    B
		case 0x0c: state->c += 1; break; // INR    C
		case 0x0d: state->c -= 1; break; // DCR    C
			{
				EvalFlags(state, state->c-1);
				state->c -= 1;
				break;
			}
		case 0x0e: // MVI    C,<data>
			{
				state->c = opcode[1];
				state->pc += 1;
				break;
			}
		case 0x0f: // RRC
			{
				state->cc.cy = ((state->a>>7) & 1); // extract MSB
				state->a = state->a >> 1;
				state->a |= (uint8_t) state->cc.cy << 7; // set MSB to 1 if cy is 1 (which is 1 if the LSB *was* 1 before the bitshifting)
				break;
			}
		case 0x10: break; // NOP
		case 0x11: // LXI    D,word
		{
				state->d = opcode[2];
				state->e = opcode[1];
				state->pc += 2; // advance program counter past our 2 data bytes
				break;
		}
		case 0x12: UnimplementedInstruction(state); break; // STAX   D
		case 0x13: // INX    D
			{
				state->e++;
				if (state->e == 0)
					state->d++;
				break;
			}
		case 0x14: state->d += 1; break; // INR    D
		case 0x15: state->d -= 1; break; // DCR    D
			{
				EvalFlags(state, state->d-1);
				state->d -= 1;
				break;
			}
		case 0x16: UnimplementedInstruction(state); break; // MVI    D,<data>
		case 0x17: UnimplementedInstruction(state); break; // RAL
		case 0x18: break; // NOP
		case 0x19: // DAD    D
			{
				uint16_t hldouble = (state->h<<8) | (state->l); // append l to h
				uint16_t rpdouble = (state->d<<8) | (state->e); // append l to h
				uint32_t result = (uint32_t) hldouble + (uint32_t) rpdouble;
				state->h = (result >> 8) & 0xff;
				state->l = result & 0xff;
				state->cc.cy = (result > 0xffff);
				break;
			}
		case 0x1a: // LDAX   D
		{
				uint16_t memaddress = (state->d<<8) | state->e; // append l to h
				state->a = state->memory[memaddress];
				break;
		}
		case 0x1b: UnimplementedInstruction(state); break; // DCX    D
		case 0x1c: state->e += 1; break; // INR    E
		case 0x1d: state->e -= 1; break; // DCR    E
			{
				EvalFlags(state, state->e-1);
				state->e -= 1;
				break;
			}
		case 0x1e: // MVI    E,<data>
			{
				state->e = opcode[1];
				state->pc += 1;
				break;
			}
		case 0x1f: UnimplementedInstruction(state); break; // RAR
		case 0x20: break; // NOP
		case 0x21: // LXI    H,word
		{
				state->h = opcode[2];
				state->l = opcode[1];
				state->pc += 2; // advance program counter past our 2 data bytes
				break;
		}
		case 0x22: UnimplementedInstruction(state); break; // SHLD   $%02x%02x
		case 0x23: // INX    H
			{
				state->l++;
				if (state->l == 0)
					state->h++;
				break;
			}
		case 0x24: state->h += 1; break; // INR    H
		case 0x25: state->h -= 1; break; // DCR    H
			{
				EvalFlags(state, state->h-1);
				state->h -= 1;
				break;
			}
		case 0x26: // MVI    H,<data>
			{
				state->h = opcode[1];
				state->pc += 1;
				break;
			}
		case 0x27: UnimplementedInstruction(state); break; // DAA
		case 0x28: break; // NOP
		case 0x29: // DAD    H
			{
				uint16_t hldouble = (state->h<<8) | (state->l); // append l to h
				uint32_t result = (uint32_t) hldouble + (uint32_t) hldouble;
				state->h = (result >> 8) & 0xff;
				state->l = result & 0xff;
				state->cc.cy = (result > 0xffff);
				break;
			}
		case 0x2a: UnimplementedInstruction(state); break; // LHLD   $%02x%02x
		case 0x2b: UnimplementedInstruction(state); break; // DCX    H
		case 0x2c: state->l += 1; break; // INR    L
		case 0x2d: state->l -= 1; break; // DCR    L
			{
				EvalFlags(state, state->l-1);
				state->l -= 1;
				break;
			}
		case 0x2e: // MVI    L,<data>
			{
				state->l = opcode[1];
				state->pc += 1;
				break;
			}
		case 0x2f: state->a = ~state->a; break; // CMA
		case 0x30: break; // NOP
		case 0x31: // LXI    SP,word
		{
				state->sp = (opcode[2] << 8) | (opcode[1]); // append 2 bytes
				state->pc += 2; // advance program counter past our 2 data bytes
				break;
		}
		case 0x32: // STA    adr((h)(l))
			{
				uint16_t memaddress = (opcode[2]<<8) | (opcode[1]);
				state->memory[memaddress] = state->a;
				state->pc += 2;
				break;
			}
		case 0x33: UnimplementedInstruction(state); break; // INX    SP
		case 0x34: UnimplementedInstruction(state); break; // INR    M
		case 0x35: // DCR    M
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				EvalFlags(state, state->memory[memaddress]-1);
				state->memory[memaddress] -= 1;
				break;
			}
		case 0x36: // MVI    M,<data>
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->memory[memaddress] = opcode[1];
				state->pc += 1;
				break;
			}
		case 0x37: UnimplementedInstruction(state); break; // STC
		case 0x38: break; // NOP
		case 0x39: UnimplementedInstruction(state); break; // DAD    SP
		case 0x3a: // LDA    adr((h)(l))
			{
				uint16_t memaddress = (opcode[2] << 8) | (opcode[1]); // append 2 bytes
				state->a = state->memory[memaddress];
				state->pc += 2; // advance program counter past our 2 data bytes
				break;
			}
		case 0x3b: UnimplementedInstruction(state); break; // DCX    SP
		case 0x3c: state->a += 1; break; // INR    A
		case 0x3d: state->a -= 1; break; // DCR    A
			{
				EvalFlags(state, state->a-1);
				state->a -= 1;
				break;
			}
		case 0x3e: // MVI    A,<data>
			{
				state->a = opcode[1];
				state->pc += 1;
				break;
			}
		case 0x3f: UnimplementedInstruction(state); break; // CMC
		case 0x40: UnimplementedInstruction(state); break; // MOV    B,B
		case 0x41: state->b = state->c; break; // MOV    B,C
		case 0x42: state->b = state->d; break; // MOV    B,D
		case 0x43: state->b = state->e; break; // MOV    B,E
		case 0x44: state->b = state->h; break; // MOV    B,H
		case 0x45: state->b = state->l; break; // MOV    B,L
		case 0x46: // MOV    B,M
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->b = state->memory[memaddress];
				break;
			}
		case 0x47: state->b = state->a; break; // MOV    B,A
		case 0x48: state->c = state->b; break; // MOV    C,B
		case 0x49: UnimplementedInstruction(state); break; // MOV    C,C
		case 0x4a: state->c = state->d; break; // MOV    C,D
		case 0x4b: state->c = state->e; break; // MOV    C,E
		case 0x4c: state->c = state->h; break; // MOV    C,H
		case 0x4d: state->c = state->l; break; // MOV    C,L
		case 0x4e: // MOV    C,M
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->c = state->memory[memaddress];
				break;
			}
		case 0x4f: state->c = state->a; break; // MOV    C,A
		case 0x50: state->d = state->b; break; // MOV    D,B
		case 0x51: state->d = state->c; break; // MOV    D,C
		case 0x52: UnimplementedInstruction(state); break; // MOV    D,D
		case 0x53: state->d = state->e; break; // MOV    D.E
		case 0x54: state->d = state->h; break; // MOV    D,H
		case 0x55: state->d = state->l; break; // MOV    D,L
		case 0x56: // MOV    D,M
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->d = state->memory[memaddress];
				break;
			}
		case 0x57: state->d = state->a; break; // MOV    D,A
		case 0x58: state->e = state->b; break; // MOV    E,B
		case 0x59: state->e = state->c; break; // MOV    E,C
		case 0x5a: state->e = state->d; break; // MOV    E,D
		case 0x5b: UnimplementedInstruction(state); break; // MOV    E,E
		case 0x5c: state->e = state->h; break; // MOV    E,H
		case 0x5d: state->e = state->l; break; // MOV    E,L
		case 0x5e: // MOV    E,M
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->d = state->memory[memaddress];
				break;
			}
		case 0x5f: state->e = state->a; break; // MOV    E,A
		case 0x60: state->h = state->b; break; // MOV    H,B
		case 0x61: state->h = state->c; break; // MOV    H,C
		case 0x62: state->h = state->d; break; // MOV    H,D
		case 0x63: state->h = state->e; break; // MOV    H.E
		case 0x64: UnimplementedInstruction(state); break; // MOV    H,H
		case 0x65: state->h = state->l; break; // MOV    H,L
		case 0x66: // MOV    H,M
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->d = state->memory[memaddress];
				break;
			}
		case 0x67: state->h = state->a; break; // MOV    H,A
		case 0x68: state->l = state->b; break; // MOV    L,B
		case 0x69: state->l = state->c; break; // MOV    L,C
		case 0x6a: state->l = state->d; break; // MOV    L,D
		case 0x6b: state->l = state->e; break; // MOV    L,E
		case 0x6c: state->l = state->h; break; // MOV    L,H
		case 0x6d: UnimplementedInstruction(state); break; // MOV    L,L
		case 0x6e: // MOV    L,M
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->l = state->memory[memaddress];
				break;
			}
		case 0x6f: state->l = state->a; break; // MOV    L,A
		case 0x70: // MOV    M,B
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->memory[memaddress] = state->b;
				break;
			}
		case 0x71: // MOV    M,C
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->memory[memaddress] = state->c;
				break;
			}
		case 0x72: // MOV    M,D
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->memory[memaddress] = state->d;
				break;
			}
		case 0x73: // MOV    M.E
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->memory[memaddress] = state->e;
				break;
			}
		case 0x74: // MOV    M,H
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->memory[memaddress] = state->h;
				break;
			}
		case 0x75: // MOV    M,L
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->memory[memaddress] = state->l;
				break;
			}
		case 0x76: halt = 1; break; // HLT
		case 0x77: // MOV    M,A
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->memory[memaddress] = state->a;
				break;
			}
		case 0x78: state->a = state->b; break; // MOV    A,B
		case 0x79: state->a = state->c; break; // MOV    A,C
		case 0x7a: state->a = state->d; break; // MOV    A,D
		case 0x7b: state->a = state->e; break; // MOV    A,E
		case 0x7c: state->a = state->h; break; // MOV    A,H
		case 0x7d: state->a = state->l; break; // MOV    A,L
		case 0x7e: // MOV    A,M
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				state->a = state->memory[memaddress];
				break;
			}
		case 0x7f: UnimplementedInstruction(state); break; // MOV    A,A
		case 0x80: // ADD    B
			{
				// perform addition as uint16_t to catch carry
				uint16_t result = (uint16_t) state->a + (uint16_t) state->b;
				
				// check/set zero flag
				if(result & 0xff == 0) {
					state->cc.z = 1;
				} else {
					state->cc.z = 0;
				}

				// check/set sign flag
				if(result & 0x80) {
					state->cc.s = 1;
				} else {
					state->cc.s = 0;
				}

				// check/set carry flag
				if(result > 0xff) {
					state->cc.cy = 1;
				} else {
					state->cc.cy = 0;
				}

				// check/set parity flag
				state->cc.p = parity(result, 8);
				state->a = result & 0xff;

				break;
			}
		case 0x81: // ADD    C
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) state->c;
				EvalFlags(state, result);
				state->a = result & 0xff;
				break;
			}
		case 0x82: // ADD    D
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) state->d;
				EvalFlags16(state, result);
				state->a = result & 0xff;
				break;
			}
		case 0x83: // ADD    E
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) state->e;
				EvalFlags16(state, result);
				state->a = result & 0xff;
				break;
			}
		case 0x84: // ADD    H
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) state->h;
				EvalFlags16(state, result);
				state->a = result & 0xff;
				break;
			}
		case 0x85: // ADD    L
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) state->l;
				EvalFlags16(state, result);
				state->a = result & 0xff;
				break;
			}
		case 0x86: // ADD    M
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				uint16_t result = (uint16_t) state->a + state->memory[memaddress];
				EvalFlags16(state, result);
				state->a = result & 0xff;
				break;
			}
		case 0x87: // ADD    A
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) state->a;
				EvalFlags16(state, result);
				state->a = result & 0xff;
				break;
			}
		case 0x88: // ADC    B
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) state->b + (uint16_t) state->cc.cy;
				state->cc.z = ((result & 0xff) == 0);
				state->cc.s = ((result & 0x80) != 0);
				state->cc.cy = (result > 0xff);
				state->cc.p = parity(result, 8);
				state->a = result & 0xff;
				break;
			}
		case 0x89: // ADC    C
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) state->c + (uint16_t) state->cc.cy;
				state->cc.z = ((result & 0xff) == 0);
				state->cc.s = ((result & 0x80) != 0);
				state->cc.cy = (result > 0xff);
				state->cc.p = parity(result, 8);
				state->a = result & 0xff;
				break;
			}
		case 0x8a: // ADC    D
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) state->d + (uint16_t) state->cc.cy;
				state->cc.z = ((result & 0xff) == 0);
				state->cc.s = ((result & 0x80) != 0);
				state->cc.cy = (result > 0xff);
				state->cc.p = parity(result, 8);
				state->a = result & 0xff;
				break;
			}
		case 0x8b: // ADC    E
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) state->e + (uint16_t) state->cc.cy;
				state->cc.z = ((result & 0xff) == 0);
				state->cc.s = ((result & 0x80) != 0);
				state->cc.cy = (result > 0xff);
				state->cc.p = parity(result, 8);
				state->a = result & 0xff;
				break;
			}
		case 0x8c: // ADC    H
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) state->h + (uint16_t) state->cc.cy;
				state->cc.z = ((result & 0xff) == 0);
				state->cc.s = ((result & 0x80) != 0);
				state->cc.cy = (result > 0xff);
				state->cc.p = parity(result, 8);
				state->a = result & 0xff;
				break;
			}
		case 0x8d: // ADC    L
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) state->l + (uint16_t) state->cc.cy;
				state->cc.z = ((result & 0xff) == 0);
				state->cc.s = ((result & 0x80) != 0);
				state->cc.cy = (result > 0xff);
				state->cc.p = parity(result, 8);
				state->a = result & 0xff;
				break;
			}
		case 0x8e: // ADC    M
			{
				uint16_t memaddress = (state->h<<8) | (state->l); // append l to h
				uint16_t result = (uint16_t) state->a + state->memory[memaddress] + (uint16_t) state->cc.cy;
				state->cc.z = ((result & 0xff) == 0);
				state->cc.s = ((result & 0x80) != 0);
				state->cc.cy = (result > 0xff);
				state->cc.p = parity(result, 8);
				state->a = result & 0xff;
				break;
			}
		case 0x8f: // ADC    A
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) state->b + (uint16_t) state->cc.cy;
				state->cc.z = ((result & 0xff) == 0);
				state->cc.s = ((result & 0x80) != 0);
				state->cc.cy = (result > 0xff);
				state->cc.p = parity(result, 8);
				state->a = result & 0xff;
				break;
			}
		case 0x90: UnimplementedInstruction(state); break; // SUB    B
		case 0x91: UnimplementedInstruction(state); break; // SUB    C
		case 0x92: UnimplementedInstruction(state); break; // SUB    D
		case 0x93: UnimplementedInstruction(state); break; // SUB    E
		case 0x94: UnimplementedInstruction(state); break; // SUB    H
		case 0x95: UnimplementedInstruction(state); break; // SUB    L
		case 0x96: UnimplementedInstruction(state); break; // SUB    M
		case 0x97: UnimplementedInstruction(state); break; // SUB    A
		case 0x98: UnimplementedInstruction(state); break; // SBB    B
		case 0x99: UnimplementedInstruction(state); break; // SBB    C
		case 0x9a: UnimplementedInstruction(state); break; // SBB    D
		case 0x9b: UnimplementedInstruction(state); break; // SBB    E
		case 0x9c: UnimplementedInstruction(state); break; // SBB    H
		case 0x9d: UnimplementedInstruction(state); break; // SBB    L
		case 0x9e: UnimplementedInstruction(state); break; // SBB    M
		case 0x9f: UnimplementedInstruction(state); break; // SBB    A
		case 0xa0: UnimplementedInstruction(state); break; // ANA    B
		case 0xa1: UnimplementedInstruction(state); break; // ANA    C
		case 0xa2: UnimplementedInstruction(state); break; // ANA    D
		case 0xa3: UnimplementedInstruction(state); break; // ANA    E
		case 0xa4: UnimplementedInstruction(state); break; // ANA    H
		case 0xa5: UnimplementedInstruction(state); break; // ANA    L
		case 0xa6: UnimplementedInstruction(state); break; // ANA    M
		case 0xa7: // ANA    A
			{
				// A & A == A, so no need to waste time computing this, nothing changes
				// we do have to clear the cy flag though as per the specs of the 8080
				state->cc.cy = 0;
			}
		case 0xa8: UnimplementedInstruction(state); break; // XRA    B
		case 0xa9: UnimplementedInstruction(state); break; // XRA    C
		case 0xaa: UnimplementedInstruction(state); break; // XRA    D
		case 0xab: UnimplementedInstruction(state); break; // XRA    E
		case 0xac: UnimplementedInstruction(state); break; // XRA    H
		case 0xad: UnimplementedInstruction(state); break; // XRA    L
		case 0xae: UnimplementedInstruction(state); break; // XRA    M
		case 0xaf: // XRA    A
			{
				// A ^ A == 0, so we set register A to 0
				// we also reset the cy and ac flags
				state->a = 0;
				state->cc.cy = 0;
				state->cc.ac = 0;
				break;
			}
		case 0xb0: UnimplementedInstruction(state); break; // ORA    B
		case 0xb1: UnimplementedInstruction(state); break; // ORA    C
		case 0xb2: UnimplementedInstruction(state); break; // ORA    D
		case 0xb3: UnimplementedInstruction(state); break; // ORA    E
		case 0xb4: UnimplementedInstruction(state); break; // ORA    H
		case 0xb5: UnimplementedInstruction(state); break; // ORA    L
		case 0xb6: UnimplementedInstruction(state); break; // ORA    M
		case 0xb7: UnimplementedInstruction(state); break; // ORA    A
		case 0xb8: UnimplementedInstruction(state); break; // CMP    B
		case 0xb9: UnimplementedInstruction(state); break; // CMP    C
		case 0xba: UnimplementedInstruction(state); break; // CMP    D
		case 0xbb: UnimplementedInstruction(state); break; // CMP    E
		case 0xbc: UnimplementedInstruction(state); break; // CMP    H
		case 0xbd: UnimplementedInstruction(state); break; // CMP    L
		case 0xbe: UnimplementedInstruction(state); break; // CMP    M
		case 0xbf: UnimplementedInstruction(state); break; // CMP    A
		case 0xc0: UnimplementedInstruction(state); break;
		case 0xc1: // POP   B
			{
				state->c = state->memory[(state->sp)+1]; // get memory contents from stack pointer location
				state->b = state->memory[(state->sp)+2];
				state->sp += 2;
				break;
			}
		case 0xc2: // JNZ adr
			{
				if(!(state->cc.z)) {
					// -1 is to counteract the universal pc increment from throwing off our jump
					state->pc = (opcode[2] << 8) | (opcode[1]) - 1; // append 2 bytes
					break;
				} else {
					state->pc += 2;
					break;
				}
			}
		case 0xc3: state->pc = (opcode[2] << 8) | (opcode[1]) - 1; break; // JMP adr
		case 0xc4: UnimplementedInstruction(state); break;
		case 0xc5: // PUSH B
			{
				state->memory[(state->sp)-1] = state->b;
				state->memory[(state->sp)-2] = state->c;
				state->sp -= 2;
				break;
			}
		case 0xc6: // ADI  A   (ADD Immediate to A)
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) opcode[1];
				EvalFlags16(state, result);
				state->a = result & 0xff;
				state->pc += 1;
				break;
			}
		case 0xc7: UnimplementedInstruction(state); break;
		case 0xc8: UnimplementedInstruction(state); break;
		case 0xc9: // RET
			{
				state->pc = state->memory[state->sp] | (state->memory[state->sp+1] << 8); // grab and put sp and sp+1 into pc
				state->pc += 2;
				break;
			}
		case 0xca: // JZ adr
			{
				if(state->cc.z) {
					state->pc = (opcode[2] << 8) | (opcode[1]) - 1; // append 2 bytes
					break;
				} else {
					state->pc += 2;
					break;
				}
			}
		case 0xcb: UnimplementedInstruction(state); break;
		case 0xcc: UnimplementedInstruction(state); break;
		case 0xcd: // CALL adr
			{
				state->memory[(state->sp)-1] = (state->pc>>8) & 0xff;
				state->memory[(state->sp)-2] = state->pc & 0xff;
				state->sp -= 2;
				state->pc = (uint16_t) (opcode[2]<<8) | (uint16_t) opcode[1];
				state->pc -= 1; // counteract standard increment from throwing off our subprocess call address
				break;
			}
		case 0xce: // ACI
			{
				uint16_t result = (uint16_t) state->a + (uint16_t) opcode[1] + (uint16_t) state->cc.cy;
				EvalFlags(state, result);
				state->a = result & 0xff;
				state->pc += 1;
				break;
			}
		case 0xcf: UnimplementedInstruction(state); break;
		case 0xd0: UnimplementedInstruction(state); break;
		case 0xd1: // POP D
			{
				state->d = state->memory[(state->sp)+1]; // get memory contents from stack pointer location
				state->e = state->memory[(state->sp)+2];
				state->sp += 2;
				break;
			}
		case 0xd2: UnimplementedInstruction(state); break;
		case 0xd3: state->pc++; break; // OUT d8 ---------> unknown implementation
		case 0xd4: UnimplementedInstruction(state); break;
		case 0xd5: // PUSH D
			{
				state->memory[(state->sp)-1] = state->d;
				state->memory[(state->sp)-2] = state->e;
				state->sp -= 2;
				break;
			}
		case 0xd6: UnimplementedInstruction(state); break;
		case 0xd7: UnimplementedInstruction(state); break;
		case 0xd8: UnimplementedInstruction(state); break;
		case 0xd9: UnimplementedInstruction(state); break;
		case 0xda: UnimplementedInstruction(state); break;
		case 0xdb: UnimplementedInstruction(state); break;
		case 0xdc: UnimplementedInstruction(state); break;
		case 0xdd: UnimplementedInstruction(state); break;
		case 0xde: UnimplementedInstruction(state); break;
		case 0xdf: UnimplementedInstruction(state); break;
		case 0xe0: UnimplementedInstruction(state); break;
		case 0xe1: // POP H
			{
				state->h = state->memory[(state->sp)+1]; // get memory contents from stack pointer location
				state->l = state->memory[(state->sp)+2];
				state->sp += 2;
				break;
			}
		case 0xe2: UnimplementedInstruction(state); break;
		case 0xe3: UnimplementedInstruction(state); break;
		case 0xe4: UnimplementedInstruction(state); break;
		case 0xe5: // PUSH H
			{
				state->memory[(state->sp)-1] = state->h;
				state->memory[(state->sp)-2] = state->l;
				state->sp -= 2;
				break;
			}
		case 0xe6: // ANI
			{
				state->a = state->a & opcode[1];
				state->cc.cy = 0;
				state->cc.ac = 0;
				state->pc += 1;
				break;
			}
		case 0xe7: UnimplementedInstruction(state); break;
		case 0xe8: UnimplementedInstruction(state); break;
		case 0xe9: UnimplementedInstruction(state); break;
		case 0xea: UnimplementedInstruction(state); break;
		case 0xeb: // XCNG
			{
				uint8_t htemp = state->h;
				uint8_t ltemp = state->l;
				state->h = state->d;
				state->l = state->e;
				state->d = htemp;
				state->e = ltemp;
				break;
			}
		case 0xec: UnimplementedInstruction(state); break;
		case 0xed: UnimplementedInstruction(state); break;
		case 0xee: UnimplementedInstruction(state); break;
		case 0xef: UnimplementedInstruction(state); break;
		case 0xf0: UnimplementedInstruction(state); break;
		case 0xf1: // POP PSW
			{
				state->cc.cy = (state->memory[state->sp]) & 1;
				state->cc.p = (state->memory[state->sp]>>1) & 1;
				state->cc.ac = (state->memory[state->sp]>>3) & 1;
				state->cc.z = (state->memory[state->sp]>>5) & 1;
				state->cc.s = (state->memory[state->sp]>>6) & 1;
				state->a = state->memory[(state->sp)+1];
				state->sp += 2;
				break;
			}
		case 0xf2: UnimplementedInstruction(state); break;
		case 0xf3: UnimplementedInstruction(state); break;
		case 0xf4: UnimplementedInstruction(state); break;
		case 0xf5: // PUSH PSW
			{
				uint8_t cy = (state->cc.cy) & 1;
				uint8_t p = (state->cc.cy) & (uint8_t) (1<<2);
				uint8_t ac = (state->cc.cy) & (uint8_t) (1<<4);
				uint8_t z = (state->cc.cy) & (uint8_t) (1<<6);
				uint8_t s = (state->cc.cy) & (uint8_t) (1<<7);
				uint8_t psw = cy | p | ac | z | s;
				state->memory[(state->sp)-1] = state->a;
				state->memory[(state->sp)-2] = psw;
				state->sp -= 2;
				break;
			}
		case 0xf6: UnimplementedInstruction(state); break;
		case 0xf7: UnimplementedInstruction(state); break;
		case 0xf8: UnimplementedInstruction(state); break;
		case 0xf9: UnimplementedInstruction(state); break;
		case 0xfa: UnimplementedInstruction(state); break;
		case 0xfb: state->pc++; break; // EI (enable interrupts) ------> not implemented, just skip for now
		case 0xfc: UnimplementedInstruction(state); break;
		case 0xfd: UnimplementedInstruction(state); break;
		case 0xfe: // CPI d8
			{
				EvalFlags(state, state->a-opcode[1]);
				state->cc.z = (state->a == opcode[1]);
				state->cc.cy = (state->a < opcode[1]);
				state->a -= opcode[1];
				state->pc += 1;
				break;
			}
		case 0xff: UnimplementedInstruction(state); break;
	}
	state->pc += 1; // increment program counter
	return halt; // halt = 0 means continue CPU emulation, halt = 1 means quit
}

CPUState8080* Init8080(void)
{
	CPUState8080* state = calloc(1,sizeof(CPUState8080));
	state->memory = malloc(0x10000);  //16K
	return state;
}

void EvalFlags(CPUState8080* state, uint8_t result)
{
	state->cc.z = (result == 0);
	state->cc.s = ((result >> 7) == 1);
	state->cc.cy = (result > 0xff);
	state->cc.p = parity(result, 8);
}

void EvalFlags16(CPUState8080* state, uint16_t result)
{
	state->cc.z = (result == 0);
	state->cc.s = ((result >> 7) == 1);
	state->cc.cy = (result > 0xff);
	state->cc.p = parity(result, 8);
}
