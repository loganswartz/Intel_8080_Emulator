#ifndef DISASSEMBLELIB8080_H
#define DISASSEMBLELIB8080_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "v8080.h"

void ReadFileIntoMemoryAt(CPUState8080* state, char* filename, uint32_t offset);
int Disassemble8080(CPUState8080* state);

#endif
