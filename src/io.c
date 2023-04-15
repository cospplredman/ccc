#include<stdio.h>
#include<stdlib.h>
#include"io.h"


char* readFile(char *fn, int *len)
{
	FILE *f = fopen(fn, "r");
	if(f && !fseek(f, 0, SEEK_END)){
		long int length = ftell(f);
		if(length != -1){
			rewind(f);
			char *buffer = malloc(length + 1);
			if(buffer && fread(buffer, 1, length, f) == length) {
				buffer[length] = 0;
				fclose(f);
				*len = length;
				return buffer;
			}
			free(buffer);
		}
		fclose(f);
	}

	printf("failed to read file: \"%s\"\n", fn);
	*len = 0;
	return NULL;
}


