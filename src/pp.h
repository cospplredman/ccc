#include"ast.h"

enum{
	/* pptokens iso c99 49 */
	PPT_PPNUMBER = T_EOF + 1,
	PPT_PPHEADER,
	PPT_PPEXTRA
};

typedef Token PPToken;

void printPPToken(PPToken *);
PPToken *allocPPToken();
void freePPToken(PPToken *);
PPToken *genPPToken(char *);
//TODO better name

enum{
	PPA_PPFILE, PPA_PPGROUP, PPA_PPELIFGROUP, PPA_PPIFSECTION
};

typedef AST PPAST;

void printPPAST(PPAST *);
PPAST *allocPPAST(int);
void freePPAST(PPAST *);
PPAST *genPPAST(PPToken *);

Token *ppttt(PPToken *);
