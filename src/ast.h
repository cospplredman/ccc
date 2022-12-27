#include"token.h"

enum{
	A_IDENT, A_INTLIT, A_STRLIT,
	A_ADDR, A_DEREF, A_UADD, A_USUB, A_BNEG, A_NEG,
	A_ADD, A_SUB, A_MUL, A_DIV,
};

typedef struct AST{
	int type;
	union{
		long long intValue;
		double floatValue;
		struct{
			struct AST *left, *right;
		};
		struct{
			char *start, *end;
		};
	};
} AST;

void freeAST(AST *);
AST *allocAST();
AST *genAST(Token **);
