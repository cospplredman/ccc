#ifndef CCC_AST_H
#define CCC_AST_H

#include"token.h"
#include"hashtable.h"

/* AST types */
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

	A_ELVIS,
	A_CALL,
	A_EXPRLIST,

	A_EQ, A_MULEQ, A_DIVEQ, A_MODEQ, A_ADDEQ, A_SUBEQ, A_LSHIFTEQ, A_RSHIFTEQ, A_ANDEQ, A_XOREQ, A_OREQ,

	A_COND, A_CONDRES,
	A_DECLARATION, A_DECLSPEC, A_DECLSPECLIST, A_DECLLIST, A_DECLARATOR, A_INITIALIZER,
	A_STORAGESPEC, A_TYPESPEC, A_TYPEQUAL, A_FUNCSPEC, A_STRUCT, A_UNION,
	A_SPECQUALLIST, A_STRUCTDECLR, A_STRUCTDECLRLIST, A_STRUCTDECL, A_STRUCTDECLLIST,
	A_ENUMERATOR, A_ENUMLIST, A_ENUMSPEC, A_DESINIT, A_DESIGNATOR, A_DESIGNATORLIST, A_INITLIST,
	A_POINTER, A_IDENTLIST, A_PARAMLIST, A_PARAMDECL, A_PARAMTYPELIST, A_DIRECTDECLARATOR, A_QUALLIST,
	A_DIRECTABSTRACTDECL, A_ABSTRACTDECLARATOR, A_TYPENAME,
	A_LABELSTATEMENT, A_FOR, A_WHILE, A_DOWHILE, A_FORLIST, A_BLOCK, A_RETURN, A_CONTINUE, A_BREAK, A_SWITCH, A_GOTO, A_IFELSE, A_IF, A_BLOCKLIST, A_EMPTYSTATEMENT,
	A_DECLARATIONLIST, A_FUNCDECL, A_TRANSLATIONUNIT, A_DVLA, A_VLA, A_EPL,

	
	PPA_PPFILE, PPA_PPGROUP, PPA_PPELIFGROUP, PPA_PPIFSECTION, PPA_PPINCLUDE, PPA_PPUNDEF, PPA_PPELSEGROUP, PPA_PPIFGROUP, PPA_PPELIFGROUPS, PPA_PPERROR, PPA_PPIFNDEFGROUP, PPA_PPIFDEFGROUP, PPA_PPTEXTLINE, PPA_PPNONDIRECTIVE
};

enum{
	SC_VOID, SC_CHAR, SC_SHORT, SC_INT, SC_LONG, SC_FLOAT, SC_DOUBLE, SC_SIGNED, SC_UNSIGNED, SC_BOOL, SC_COMPLEX, SC_IMAGINARY, SC_NONBASE, SC_IDENTIFIER
};

enum{
	Q_CONST = 1, Q_RESTRICT = 1 << 1, Q_VOLATILE = 1 << 2, Q_INLINE = 1 << 3, Q_STATIC = 1 << 4, Q_POINTER = 1 << 5, Q_VARLIST = 1 << 6
};

typedef struct StrEntry{
	size_t hash;
	char *str;
} StrEntry;

typedef struct scopeEntry{
	char type, qual;
	char *start, *end;
	size_t length, size;
	struct scopeEntry *ptype[];
} scopeEntry;

/* AST flags */
enum{
	//TODO rename 
	AF_NODE = 1, AF_TOKNODE = 1 << 1, AF_STRNODE = 1 << 3, AF_OWNSCOPE = 1 << 4
};

typedef struct AST{
	int ASTtype, length;
	HashTable *scope;
	scopeEntry *type;
	char flags;
	void* data[];
} AST;

AST *allocAST(int);
void freeAST(AST *);
void printAST(AST *);
AST *genAST(Token *);

#endif

