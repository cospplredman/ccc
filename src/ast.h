#include"token.h"

enum{
	A_IDENT, A_INTLIT, A_STRLIT,

	A_MEMBER, A_ARROW, A_PINC, A_PDEC, A_INC, A_DEC, A_SIZEOF,
	A_ADDR, A_UDEREF, A_DEREF, A_UADD, A_USUB, A_BNEG, A_NEG,
	A_ADD, A_SUB, A_MUL, A_DIV, A_MOD, A_CAST, A_LSHIFT, A_RSHIFT,

	A_LT, A_GT, A_LTEQ, A_GTEQ,

	A_EQUALS, A_NOTEQUALS,

	A_AND,

	A_XOR,

	A_OR,

	A_BAND,

	A_BOR,

	A_ELVIS
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
void printAST(AST *);
AST *genAST(Token **);
