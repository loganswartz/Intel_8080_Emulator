#ifndef V8080_H
#define V8080_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct ConditionCodes {
	uint8_t    z:1; // zero flag (result is zero)
	uint8_t    s:1; // sign flag (indicates if most significant bit of result is set)
	uint8_t    p:1; // parity flag (set if result is even, not set if odd)
	uint8_t    cy:1; // carry flag (set if instruction resulted in a carry on most significant bit)
	uint8_t    ac:1; // auxiliary carry flag (used in BCD math)
	uint8_t    pad:3; // something???
} ConditionCodes;

typedef struct CPUState8080 {
	uint8_t    a; // Accumulator register
	uint8_t    b;
	uint8_t    c;
	uint8_t    d;
	uint8_t    e;
	uint8_t    h;
	uint8_t    l;
	uint16_t   sp;
	uint16_t   pc; // program counter
	uint8_t    *memory;
	struct     ConditionCodes cc;
	uint8_t    enable;
} CPUState8080;


int Parity(uint8_t input);
int Parity16(uint16_t input);
void UnimplementedInstruction(CPUState8080* state);
int Emulate8080(CPUState8080* state);
CPUState8080* Init8080();
void EvalFlags(CPUState8080* state, uint8_t result);
void EvalFlags16(CPUState8080* state, uint16_t result);

#endif
