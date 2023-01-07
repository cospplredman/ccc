#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include"src/ast.h"

char* readfile(FILE *f) {
  // f invalid? fseek() fail?
  if(f == NULL || fseek(f, 0, SEEK_END))
    return NULL;

  long length = ftell(f);
  rewind(f);
  // Did ftell() fail?  Is the length too long?
  if (length == -1 || (unsigned long) length >= SIZE_MAX) {
    return NULL;
  }

  // Convert from long to size_t
  size_t ulength = (size_t) length;
  char *buffer = malloc(ulength + 1);
  // Allocation failed? Read incomplete?
  if (buffer == NULL || fread(buffer, 1, ulength, f) != ulength) {
    free(buffer);
    return NULL;
  }
  buffer[ulength] = '\0'; // Now buffer points to a string

  return buffer;
}

int main(int argc, char **argv){
	argc--;
	argv++;

	for(int i = 0; i < argc; i++){
		FILE * cf = fopen(argv[i], "r");
		char *str = readfile(cf), *strp = str;

		Token *tok = genTokens(&strp);
		AST *ast = genAST(&tok);

		if(!ast)
			printf("could not generate ast\n");
		else
			printAST(ast);
		
		freeToken(tok);
		freeAST(ast);
		free(str);
		fclose(cf);
	}
	return 0;
}
