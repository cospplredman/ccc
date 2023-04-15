#ifndef CCC_PP_H
#define CCC_PP_H
#include"ast.h"

Token *genPPTokens(char *);
AST *genPPAST(Token *);
//TODO better name
Token *ppttt(Token *);

#endif
