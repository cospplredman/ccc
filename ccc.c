#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include"src/ast.h"

const char *tokenName[] = {
	"T_AUTO", "T_ENUM", "T_RESTRICT", "T_UNSIGNED", "T_BREAK", "T_EXTERN", "T_RETURN", "T_VOID", "T_CASE", "T_FLOAT", "T_SHORT", "T_VOLATILE", "T_CHAR", "T_FOR", "T_SIGNED", "T_WHILE", "T_CONST", "T_GOTO", "T_SIZEOF", "T_BOOL", "T_CONTINUE", "T_IF", "T_STATIC", "T_COMPLEX", "T_DEFAULT", "T_INLINE", "T_STRUCT", "T_IMAGINARY", "T_DO", "T_INT", "T_SWITCH", "T_DOUBLE", "T_LONG", "T_TYPEDEF", "T_ELSE", "T_REGISTER", "T_UNION",
	"T_IDENTIFIER",
	
	"T_INTEGERCONST",
	"T_FLOATCONST",
	"T_CHARCONST",

	"T_STRINGLIT",

	"T_LBRACE", "T_RBRACE", "T_LPAREN", "T_RPAREN", "T_LCURLY", "T_RCURLY", "T_DOT", "T_ARROW", "T_INC", "T_DEC", "T_AND", "T_STAR", "T_PLUS", "T_MINUS", "T_TILDE", "T_BANG", "T_SLASH", "T_PERCENT", "T_LSHIFT", "T_RSHIFT", "T_LT", "T_GT", "T_LTEQ", "T_GTEQ", "T_EQUAL", "T_NOTEQUAL", "T_XOR", "T_OR", "T_BAND", "T_BOR", "T_QMARK", "T_COLON", "T_SEMI", "T_ELIPSIS", "T_EQUALS", "T_EQ", "T_MULEQ", "T_DIVEQ", "T_MODEQ", "T_ADDEQ", "T_SUBEQ", "T_LSHIFTEQ", "T_RSHIFTEQ", "T_ANDEQ", "T_XOREQ", "T_OREQ", "T_COMMA", "T_POUND", "T_PPCONCAT",

	"T_PPHEADER",

	"T_WS",
	"T_PPEXTRA"
};


char* readfile(FILE *f) {
  // f invalid? fseek() fail?
  if (f == NULL || fseek(f, 0, SEEK_END)) {
    return NULL;
  }

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

		Token *tmp = tok;
		while(tmp->token != T_PPEXTRA){
			printf("%-15s: %d %lf \"%.*s\"\n", tokenName[tmp->token], tmp->intValue, tmp->floatValue, tmp->end - tmp->start, tmp->start);
			tmp = tmp->next;
		}

		AST *ast = genAST(&tok);
		if(!ast)
			printf("could not generate ast\n");
		
		
		//TODO free tok
		//TODO free ast
		free(str);
		fclose(cf);
	}
	return 0;
}
