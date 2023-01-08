#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include"src/ast.h"

char* readFile(char *fn) {
	FILE *f = fopen(fn, "r");
	if(f && !fseek(f, 0, SEEK_END)){
		long int length = ftell(f);
		if (length != -1){
			rewind(f);
			char *buffer = malloc(length + 1);
			if (buffer && fread(buffer, 1, length, f) == length) {
				buffer[length] = 0;
				fclose(f);
				return buffer;
			}
			free(buffer);
		}
		fclose(f);
	}

	printf("failed to read file: \"%s\"\n", fn);
	return NULL;
}

int main(int argc, char **argv){
	argc--, argv++;

	for(int i = 0; i < argc; i++){
		char *str = readFile(argv[i]);
		Token *tok = genTokens(str);
		AST *ast = genAST(tok);
	
		printToken(tok);	
		printAST(ast);

		free(str);
		freeToken(tok);
		freeAST(ast);
	}
	return 0;
}
