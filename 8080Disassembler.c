#include <stdio.h>
#include <stdlib.h>
#include "8080DisassembleLib.h"


int main(int argc, char* argv[])
{
	if(argc > 2) {
		printf("Only 1 arg expected.");
		exit(1);
	} else if(argc == 1) {
		printf("Please specify a file to disassemble.");
		exit(1);
	}

	File file = loadf(argv[1]);

	int pos = 0;
	char size[24];
	sprintf(size, "%ld", file.size-1);
	printf("%s\n", size);

	// file.size-1 is there because we add on a \0 when reading in the file just in case something goes wrong
	while(pos < (file.size-1))
	{
		//printf("\n\n%d\n", pos);
		pos += disassemble8080(file.src, pos);
	}

	free(file.src);
}
