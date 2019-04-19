#include <stdio.h>
#include <stdlib.h>

struct File {
	size_t size;
	char* src;
};


struct File loadf(char path[])
{
	struct File infile;

	char* source = NULL;
	FILE* fp = fopen(path, "r");
	if (fp != NULL) {
		/* Go to the end of the file and get position with ftell(). */
		if (fseek(fp, 0L, SEEK_END) == 0) {
			/* Get the size of the file. */
			long bufsize = ftell(fp);
			if (bufsize == -1) { /* Error */ }

			/* Allocate our buffer to that size. */
			source = malloc(sizeof(char) * (bufsize + 1));

			/* Go back to the start of the file. */
			if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

			/* Read the entire file into memory and get total size of read contents. */
			infile.size = fread(source, sizeof(char), bufsize, fp);
			if ( ferror( fp ) != 0 ) {
				fputs("Error reading file", stderr);
			} else {
				source[infile.size++] = '\0'; /* Just to be safe. */
			}
		}
		fclose(fp);
	}
	
	infile.src = source;
	//free(source); /* Don't forget to call free() later! */
	return infile;
}

int disassemble8080(char* buffer, int pos)
{
	/* create pointer to character @ buffer[pos] */
	unsigned char* opcode = &buffer[pos];
	int bytes = 1;

	printf("%04x ", pos);
	switch(*opcode)
	{
		case 0x00: printf("NOP"); break;
		case 0x01: printf("LXI   B,#$%02x%02x", opcode[2], opcode[1]); bytes=3; break;
		case 0x02: printf("STAX   B"); break;
		case 0x03: printf("INX   B"); break;
		case 0x04: printf("INR   B"); break;
		case 0x05: printf("DCR   B"); break;
		case 0x06: printf("MVI   B,#$%02x", opcode[1]); bytes=2; break;
		case 0x07: printf("RLC"); break;
		case 0x08: printf("NOP"); break;
		/* ........ */
		case 0x3e: printf("MVI   A,#0x%02x", opcode[1]); bytes = 2; break;
		/* ........ */
		case 0xc3: printf("JMP   $%02x%02x",opcode[2],opcode[1]); bytes = 3; break;
		/* ........ */
	}

	printf("\n");
	return bytes;
}


void main()
{
	char inpath[] = "/home/logans/git-repos/projects/Intel_8080_Emulator/invaders.h";
	struct File file = loadf(inpath);

	int pos = 0;
	char size[24];
	sprintf(size, "%ld", file.size);
	printf("%s\n", size);
	while(pos < file.size)
	{
		//printf("\n\n%d\n", pos);
		pos += disassemble8080(file.src, pos);
	}

	free(file.src);
}
