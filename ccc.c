#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include"src/io.h"
#include"src/pp.h"

int main(int argc, char **argv){
	argc--, argv++;

	for(int i = 0; i < argc; i++){
		int len;
		char *str = readFile(argv[i], &len);
		//TODO use len

		//Token *tok = genPPTokens(str);
		//printToken(tok);
		//printf("\n============\n");
		//AST *ast = genPPAST(tok);
		//printAST(ast);

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
