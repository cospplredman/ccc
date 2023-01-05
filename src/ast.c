#include"ast.h"
#include<stdlib.h>
#include<stddef.h>
#include<stdio.h>

/* util functions */

void
freeAST(AST *ast)
{
	if(ast){
		switch(ast->type){
			case A_IDENT:
			case A_INTLIT:
			break;
			default:
			freeAST(ast->left);
			freeAST(ast->right);
		}
		free(ast);
	}
}

void
freeASTLeaves(AST *ast)
{
	if(ast){
		switch(ast->type){
			case A_IDENT:
			case A_INTLIT:
			break;
			default:
			freeAST(ast->left);
			freeAST(ast->right);
		}
	}
}

AST*
allocAST()
{
	//TODO make code more robust
	//return (AST*)malloc(sizeof(AST));
	return (AST*)calloc(1, sizeof(AST));
}

AST*
copyAST(AST ast)
{
	AST *lamp = allocAST();
	*lamp = ast;
	return lamp;
}

static int
scanToken(Token **tok, int token)
{
	if((*tok)->token == token){
		*tok = (*tok)->next;
		return 1;
	}

	return 0;
}

static int
binExpr(int (*prev)(Token**, AST*), int (*oper)(int), Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;
	int type;

	if(prev(&tmp, &tl)){
		while(1){
			type = oper(tmp->token);

			Token *ttmp = tmp->next;
			if(type < 0 || !prev(&ttmp, &tr))
				break;
			
			tl = (AST){
				type,
				.left = copyAST(tl),
				.right = copyAST(tr)
			};
			tmp = ttmp;
		}

		*ast = tl;
		*tok = tmp;
		return 1;
	}
	
	return 0;
}

static const char* astName[] = {
	"A_IDENT", "A_INTLIT", "A_STRLIT",

	"A_MEMBER", "A_ARROW", "A_PINC", "A_PDEC", "A_INC", "A_DEC", "A_SIZEOF",
	"A_ADDR", "A_UDEREF", "A_DEREF", "A_UADD", "A_USUB", "A_BNEG", "A_NEG",
	"A_ADD", "A_SUB", "A_MUL", "A_DIV", "A_MOD", "A_CAST", "A_LSHIFT", "A_RSHIFT",

	"A_LT", "A_GT", "A_LTEQ", "A_GTEQ",

	"A_EQUALS", "A_NOTEQUALS",

	"A_AND",

	"A_XOR",

	"A_OR",

	"A_BAND",

	"A_BOR",

	"A_ELVIS",
	"A_CALL",
	"A_EXPRLIST",

	"A_EQ", "A_MULEQ", "A_DIVEQ", "A_MODEQ", "A_ADDEQ", "A_SUBEQ", "A_LSHIFTEQ", "A_RSHIFTEQ", "A_ANDEQ", "A_XOREQ", "A_OREQ",
	
	"A_COND", "A_CONDRES",
	"A_DECLARATION", "A_DECLSPEC", "A_DECLSPECLIST", "A_DECLLIST", "A_DECLARATOR", "A_INITIALIZER",
	"A_STORAGESPEC", "A_TYPESPEC", "A_TYPEQUAL", "A_FUNCSPEC", "A_STRUCT", "A_UNION",
	"A_SPECQUALLIST", "A_STRUCTDECLR", "A_STRUCTDECLRLIST", "A_STRUCTDECL", "A_STRUCTDECLLIST",
	"A_ENUMERATOR", "A_ENUMLIST", "A_ENUMSPEC"
};

void
printAST(AST *ast)
{
	//if(ast)
	//	printf("\n %p %s %p %p\n", ast, astName[ast->type], ast->left, ast->right);
	
	if(ast == NULL){
		printf("_");
		return;
	}
	
	printf("(");
	if(ast->type == A_INTLIT){
		printf("%lld", ast->intValue);
		printf(")");
		return;
	}
	
	if(ast->type == A_IDENT){
		printf("%.*s", (int)(ast->end - ast->start), ast->start);
		printf(")");
		return;
	}

	if(ast->type == A_STORAGESPEC){
		printf("%lld", ast->intValue);
		printf(")");
		return;
	}

	printf("%s", astName[ast->type]);
	printAST(ast->left);
	printAST(ast->right);
	printf(")");
}

/* ast generation functions */

// iso c99 69
static int
identifier(Token **tok, AST *ast)
{
	Token *tmp = *tok;
	if(scanToken(&tmp, T_IDENTIFIER)){
		*ast = (AST){
			A_IDENT,
			.start = (*tok)->start,
			.end = (*tok)->end
		};

		*tok = tmp;
		return 1;
	}
		
	return 0;
}

// iso c99 69
static int
constant(Token **tok, AST *ast)
{
	switch((*tok)->token){
		case T_INTEGERCONST:
		case T_FLOATCONST:
		case T_CHARCONST:
		case T_STRINGLIT:
			//TODO deal with strings properly
			ast->type = A_INTLIT;
			ast->intValue = (*tok)->intValue;
			*tok = (*tok)->next;
			return 1;
		break;
	}
	
	return 0;
}

// iso c99 69
static int
stringLit(Token **tok, AST *ast)
{
	Token *tmp = *tok;
	if(scanToken(&tmp, T_STRINGLIT)){
		//TODO do escape sequences and whatnot (alredy implimented just need to make a string)
		*ast = (AST){
			A_STRLIT,
			.start = (*tok)->start,
			.end = (*tok)->end,
		};

		*tok = tmp;
		return 1;
	}

	return 0;
}

static int expr(Token**, AST*);

// iso c99 69
static int
primaryExpr(Token **tok, AST *ast)
{
	if(identifier(tok, ast) || constant(tok, ast) || stringLit(tok, ast))
		return 1;

	Token *tmp = *tok;
	AST tl;
	if(scanToken(&tmp, T_LPAREN)){
		if(expr(&tmp, &tl) && scanToken(&tmp, T_RPAREN)){
			*ast = tl;
			*tok = tmp;
			return 1;
		}

		freeASTLeaves(&tl);
	}

	return 0;
}

// iso c99 69
static int
typeName(Token **tok, AST *ast)
{
	//TODO
	return 0;

	switch((*tok)->token){
		case T_IDENTIFIER:
		case T_BOOL:
		case T_CONST:
		case T_COMPLEX:
		case T_STRUCT:
		case T_UNION:
		case T_CHAR:
		case T_SHORT:
		case T_FLOAT:
		case T_DOUBLE:
		case T_SIGNED:
		case T_UNSIGNED:
		case T_IMAGINARY:
		case T_INT:
		case T_LONG:
		case T_VOID:
		break;
	}
}

// iso c99 69
static int
initializerList(Token **tok, AST *ast)
{
	//TODO
	return 0;
}

static int assignmentExpr(Token **, AST *);

static int
exprList(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;
	
	if(assignmentExpr(&tmp, &tl)){
		Token *ttmp = tmp;
		if(scanToken(&ttmp, T_COMMA) && exprList(&ttmp, &tr)){
			*ast = (AST){
				A_EXPRLIST,
				.left = copyAST(tl),
				.right = copyAST(tr)
			};
			*tok = ttmp;
			return 1;
		}
		
		*ast = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 70
static int
argumentExprList(Token **tok, AST *ast)
{
	return exprList(tok, ast);
}

// iso c99 69
static int
postfixExpr(Token **tok, AST *ast)
{
	if((*tok)->token == T_LPAREN){
		//TODO
	}

	AST tl, tr;
	Token *tmp = *tok, *ttmp;
	int type = -1;
	
	if(primaryExpr(&tmp, &tl)){
		while(1){
			tl.left = copyAST(tl);
			tl.right = NULL;

			ttmp = tmp;
			if(scanToken(&ttmp, T_LBRACE) && expr(&ttmp, &tr)){
				if(scanToken(&ttmp, T_RBRACE)){
					tl = (AST){
						A_DEREF,
						.left = tl.left,
						.right = copyAST(tr)
					};
					tmp = ttmp;
					continue;
				}
				freeASTLeaves(&tr);
			}
			
			ttmp = tmp;
			if(scanToken(&ttmp, T_LPAREN)){
			       	int opt = argumentExprList(&ttmp, &tr);
				if(scanToken(&ttmp, T_RPAREN)){
					tl = (AST){
						A_CALL,
						.left = tl.left,
						.right = opt ? copyAST(tr) : NULL
					};
					tmp = ttmp;
					continue;
				}
				if(opt)
					freeASTLeaves(&tr);
			}
			
			type = -1;
			switch(tmp->token){
				case T_DOT: type = A_MEMBER; break;
				case T_ARROW: type = A_ARROW; break;
			}
			ttmp = tmp->next;

			if(type >= 0 && identifier(&ttmp, &tr)){
				tl = (AST){
					type,
					.left = tl.left,
					.right = copyAST(tr)
				};
				tmp = ttmp;
				continue;
			}

			type = -1;
			switch(tmp->token){
				case T_INC: type = A_PINC; break;
				case T_DEC: type = A_PDEC; break;
			}
			ttmp = tmp->next;

			if(type < 0)
				break;

			tl = (AST){
				type,
				.left = tl.left,
				.right = NULL
			};
			tmp = ttmp;
		}

		*ast = *tl.left;
		tl.left->left = tl.left->right = NULL;
		freeAST(tl.left);
		*tok = tmp;
		return 1;
	}

	return 0;
}

static int castExpr(Token **, AST *);

// iso c99 78
static int
unaryExpr(Token **tok, AST *ast)
{
	AST tl;
	Token *tmp = *tok;
	int type = -1;

	switch(tmp->token){
		case T_INC: type = A_INC; break;
		case T_DEC: type = A_DEC; break;
		case T_AND: type = A_ADDR; break;
		case T_STAR: type = A_UDEREF; break;
		case T_PLUS: type = A_UADD; break;
		case T_MINUS: type = A_USUB; break;
		case T_TILDE: type = A_NEG; break;
		case T_BANG: type = A_BNEG; break;
		/*
		case T_SIZEOF:
			if(unaryExpr(&tmp, lamp)){
				//TODO generate int literal
			}else if(tmp->token == T_LPAREN){
				tmp = tmp->next;
				if(typeName(&tmp, &lamp)){
					if(tmp->token == T_RPAREN){
						tmp = tmp->next;
					}
				}
			}
			break;
		*/
	}

	tmp = tmp->next;
	if(type >= 0 && castExpr(&tmp, &tl)){
		*ast = (AST){
			type,
			.left = copyAST(tl),
			.right = NULL
		};

		*tok = tmp;
		return 1;
	}

	return postfixExpr(tok, ast);
}

// iso c99 78
static int
castExpr(Token **tok, AST *ast){
	AST tl, tr;
	Token *tmp = *tok;

	if(scanToken(&tmp, T_LPAREN) && typeName(&tmp, &tl)){
		if(scanToken(&tmp, T_RPAREN) && castExpr(&tmp, &tr)){
			*ast = (AST){
				A_CAST,
				.left = copyAST(tl),
				.right = copyAST(tr)
			};
			*tok = tmp;
			return 1;
		}
		freeASTLeaves(&tl);
	}	

	return unaryExpr(tok, ast);
}

static int
mulOper(int token)
{
	int type = -1;
	switch(token){
		case T_STAR: type = A_MUL; break;
		case T_SLASH: type = A_DIV; break;
		case T_PERCENT: type = A_MOD; break;
	}

	return type;
}

// iso c99 82
static int
mulExpr(Token **tok, AST *ast)
{
	return binExpr(castExpr, mulOper, tok, ast);
}

static int
addOper(int token)
{
	int type = -1;
	switch(token){
		case T_PLUS: type = A_ADD; break;
		case T_MINUS: type = A_SUB; break;
	}

	return type;
}

// iso c99 82
static int
addExpr(Token **tok, AST *ast)
{
	return binExpr(mulExpr, addOper, tok, ast);
}

static int
shiftOper(int token)
{
	int type = -1;
	switch(token){
		case T_LSHIFT: type = A_LSHIFT; break;
		case T_RSHIFT: type = A_RSHIFT; break;	       
	}

	return type;
}

// iso c99 84
static int
shiftExpr(Token **tok, AST *ast)
{
	return binExpr(addExpr, shiftOper, tok, ast);
}

static int
relOper(int token)
{
	int type = -1;
	switch(token){
		case T_LT: type = A_LT; break;
		case T_GT: type = A_GT; break;
		case T_LTEQ: type = A_LTEQ; break;
		case T_GTEQ: type = A_GTEQ; break;
	}

	return type;
}
// iso c99 85
static int
relExpr(Token **tok, AST *ast)
{
	return binExpr(shiftExpr, relOper, tok, ast);
}

static int
eqOper(int token)
{
	int type = -1;
	switch(token){
		case T_EQUAL: type = A_EQUALS; break;
		case T_NOTEQUAL: type = A_NOTEQUALS; break;
	}

	return type;
}
// iso c99 86
static int
eqExpr(Token **tok, AST *ast)
{
	return binExpr(relExpr, eqOper, tok, ast);
}

static int
andOper(int token)
{
	int type = -1;
	switch(token){
		case T_AND: type = A_AND; break;
	}

	return type;
}
// iso c99 87
static int
andExpr(Token **tok, AST *ast)
{
	return binExpr(eqExpr, andOper, tok, ast);
}

static int
xorOper(int token)
{
	int type = -1;
	switch(token){
		case T_XOR: type = A_XOR; break;
	}

	return type;
}
// iso c99 88
static int
xorExpr(Token **tok, AST *ast)
{
	return binExpr(andExpr, xorOper, tok, ast);
}

static int
orOper(int token)
{
	int type = -1;
	switch(token){
		case T_OR: type = A_OR; break;
	}

	return type;
}
// iso c99 88
static int
orExpr(Token **tok, AST *ast)
{
	return binExpr(xorExpr, orOper, tok, ast);
}


static int
bandOper(int token)
{
	int type = -1;
	switch(token){
		case T_BAND: type = A_BAND; break;
	}

	return type;
}

// iso c99 89
static int
bandExpr(Token **tok, AST *ast)
{
	return binExpr(orExpr, bandOper, tok, ast);
}

static int
borOper(int token)
{
	int type = -1;
	switch(token){
		case T_BOR: type = A_BOR; break;
	}

	return type;
}

// iso c99 89
static int 
borExpr(Token **tok, AST *ast)
{
	return binExpr(bandExpr, borOper, tok, ast);
}

// iso c99 90
static int
elvisExpr(Token **tok, AST *ast)
{
	AST tl, tm, tr;
	Token *tmp = *tok;

	if(borExpr(&tmp, &tl)){
		Token *ttmp = tmp;
		if(scanToken(&ttmp, T_QMARK) && expr(&ttmp, &tm) ){
			if(scanToken(&ttmp, T_COLON) && elvisExpr(&ttmp, &tr)){
				*ast = (AST){
					A_COND,
					.left = copyAST(tl),
					.right = copyAST((AST){
						A_CONDRES,
						.left = copyAST(tm),
						.right = copyAST(tr)
					})
				};

				*tok = ttmp;
				return 1;
			}
			freeASTLeaves(&tm);
		}

		*tok = tmp;
		*ast = tl;
		return 1;
	}
	
	return 0;
}

static int
assignmentOper(int token){
	int type = -1;
	switch(token){
		case T_EQ: type = A_EQ; break;
		case T_MULEQ: type = A_MULEQ; break;
		case T_DIVEQ: type = A_DIVEQ; break;
		case T_MODEQ: type = A_MODEQ; break;
		case T_ADDEQ: type = A_ADDEQ; break;
		case T_SUBEQ: type = A_SUBEQ; break;
		case T_LSHIFTEQ: type = A_LSHIFTEQ; break;
		case T_RSHIFTEQ: type = A_RSHIFTEQ; break;
		case T_ANDEQ: type = A_ANDEQ; break;
		case T_XOREQ: type = A_XOREQ; break;
		case T_OREQ: type = A_OREQ; break;
	}

	return type;
}

// iso c99 91
static int
assignmentExpr(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;
	int type;

	if(unaryExpr(&tmp, &tl)){
		type = assignmentOper(tmp->token);
		tmp = tmp->next;

		if(type >= 0 && assignmentExpr(&tmp, &tr)){
			*ast = (AST){
				type,
				.left = copyAST(tl),
				.right = copyAST(tr)
			};

			*tok = tmp;
			return 1;
		}
		
		freeASTLeaves(&tl);
	}

	if(elvisExpr(tok, ast))
		return 1;
	
	return 0;
}

// iso c99 94 
static int
expr(Token **tok, AST *ast)
{
	return exprList(tok, ast);
}

// iso c99 97
static int
declarator(Token **tok, AST *ast)
{
	//TODO A_DECLARATOR
	return 0;
}

// iso c99 97
static int
initializer(Token **tok, AST *ast)
{
	//TODO A_INITIALIZER
	return 0;
}

// iso c99 97
static int
initDeclarator(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;
	if(declarator(&tmp, &tl)){
		Token *ttmp = tmp;
		if(scanToken(&ttmp, T_EQUALS) && initializer(&ttmp, &tr)){
			*ast = (AST){
				A_INITIALIZER,
				.left = copyAST(tl),
				.right = copyAST(tr)
			};

			*tok = ttmp;
			return 1;
		}

		*ast = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 97
static int
initDeclaratorList(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;
	
	if(initDeclarator(&tmp, &tl)){
		Token *ttmp = tmp;
		if(scanToken(&ttmp, T_COMMA) && initDeclarator(&ttmp, &tr)){
			*ast = (AST){
				A_DECLLIST,
				.left = copyAST(tl),
				.right = copyAST(tr)
			};

			*tok = ttmp;
			return 1;
		}
		
		*ast = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 98
static int
storageClassSpecifier(Token **tok, AST *ast)
{
	Token *tmp = *tok;
	
	switch(tmp->token){
		case T_TYPEDEF:
		case T_EXTERN:
		case T_STATIC:
		case T_AUTO:
		case T_REGISTER:
			*ast = (AST){
				A_STORAGESPEC,
				.intValue = tmp->token
			};
			tmp = tmp->next;
			*tok = tmp;
			return 1;
	}

	return 0;
}

static int typeSpecifier(Token **, AST *);
static int typeQualifier(Token **, AST *);

// iso c99 101
static int
specifierQualifierList(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;

	if(typeSpecifier(&tmp, &tl) || typeQualifier(&tmp, &tl)){
		if(specifierQualifierList(&tmp, &tr)){
			*ast = (AST){
				A_SPECQUALLIST,
				.left = copyAST(tl),
				.right = copyAST(tr)
			};

			*tok = tmp;	
			return 1;
		}

		*ast = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 101
static int
structDeclarator(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;
	int opt = declarator(&tmp, &tl);

	Token *ttmp = tmp;
	//constexpr
	if(scanToken(&ttmp, T_COLON) && elvisExpr(&ttmp, &tr)){
		*ast = (AST){
			A_STRUCTDECLR,
			.left = opt ? copyAST(tl) : NULL,
			.right = copyAST(tr)
		};

		*tok = ttmp;
		return 1;
	}

	if(opt){
		*ast = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 101
static int
structDeclaratorList(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;
	if(structDeclarator(&tmp, &tl)){
		Token *ttmp = tmp;
		if(scanToken(&ttmp, T_COMMA) && structDeclaratorList(&ttmp, &tr)){
			*ast = (AST){
				A_STRUCTDECLRLIST,
				.left = copyAST(tl),
				.right = copyAST(tr)
			};

			*tok = ttmp;
			return 1;
		}

		*ast = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 101
static int
structDeclaration(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;

	if(specifierQualifierList(&tmp, &tl)){
		if(structDeclaratorList(&tmp, &tr)){
			if(scanToken(&tmp, T_SEMI)){
				*ast = (AST){
					A_STRUCTDECL,
					.left = copyAST(tl),
					.right = copyAST(tr)
				};

				*tok = tmp;
				return 1;	
			}
			freeASTLeaves(&tr);
		}
		freeASTLeaves(&tl);
	}
	return 0;
}

// iso c99 101
static int
structDeclarationList(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;

	if(structDeclaration(&tmp, &tl)){
		Token *ttmp = tmp;
		if(structDeclarationList(&ttmp, &tr)){
			*ast = (AST){
				A_STRUCTDECLLIST,
				.left = copyAST(tl),
				.right = copyAST(tr)
			};

			*tok = ttmp;
			return 1;
		}

		*ast = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 101
static int
structOrUnionSpecifier(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;
	int idflag = 0;
	int type = A_STRUCT;

	if(scanToken(&tmp, T_STRUCT) || (scanToken(&tmp, T_UNION) ? (type = A_UNION, 1) : 0)){
		idflag = identifier(&tmp, &tl);

		Token *ttmp = tmp;
		if(scanToken(&ttmp, T_LCURLY) && structDeclarationList(&ttmp, &tr)){
		       if(scanToken(&ttmp,  T_RCURLY)){
				*ast = (AST){
					type,
					.left = idflag ? copyAST(tl) : NULL,
					.right = copyAST(tr)
				};
				*tok = ttmp;
				return 1;
			}
			freeASTLeaves(&tr);
		}

		if(idflag){
			*ast = (AST){
				type,
				.left = copyAST(tl),
				.right = NULL
			};
			*tok = tmp;
			return 1;
		}
	}
	return 0;
}

// iso c99 105
static int
enumerator(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;

	//enumconst
	if(identifier(&tmp, &tl)){
		Token *ttmp = tmp;
		//constexpr
		if(scanToken(&ttmp, T_EQ) && elvisExpr(&ttmp, &tr)){
			*ast = (AST){
				A_ENUMERATOR,
				.left = copyAST(tl),
				.right = copyAST(tr)
			};

			*tok = ttmp;
			return 1;
		}

		*ast = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 104
static int
enumeratorList(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;

	if(enumerator(&tmp, &tl)){
		Token *ttmp = tmp;
		if(scanToken(&ttmp, T_COMMA), enumeratorList(&ttmp, &tr)){
			*ast = (AST){
				A_ENUMLIST,
				.left = copyAST(tl),
				.right = copyAST(tr)
			};

			*tok = ttmp;
			return 1;
		}

		*ast = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 104
static int
enumSpecifier(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;

	if(scanToken(&tmp, T_ENUM)){
		int opt = identifier(&tmp, &tl);
		
		Token *ttmp = tmp;
		if(scanToken(&ttmp, T_LCURLY) && enumeratorList(&ttmp, &tr)){
			scanToken(&ttmp, T_COMMA);
			if(scanToken(&ttmp, T_RCURLY)){
				*ast = (AST){
					A_ENUMSPEC,
					.left = opt ? copyAST(tl) : NULL,
					.right = copyAST(tr)
				};

				*tok = ttmp;
				return 1;
			}
			freeASTLeaves(&tr);
		}

		if(opt){
			*ast = (AST){
				A_ENUMSPEC,
				.left = copyAST(tl),
				.right = NULL
			};

			*tok = tmp;
			return 1;
		}

	}
	
	return 0;
}

// iso c99 99
static int
typedefName(Token **tok, AST *ast){
	//TODO
	return 0;
}

// iso c99 99
static int
typeSpecifier(Token **tok, AST *ast)
{
	Token *tmp = *tok;

	switch(tmp->token){
		case T_VOID:
		case T_CHAR:
		case T_SHORT:
		case T_INT:
		case T_LONG:
		case T_FLOAT:
		case T_DOUBLE:
		case T_SIGNED:
		case T_UNSIGNED:
		case T_BOOL:
		case T_COMPLEX:
		case T_IMAGINARY:
			*ast = (AST){
				A_TYPESPEC,
				.intValue = tmp->token
			};
			tmp = tmp->next;
			*tok = tmp;	
			return 1;
	}

	return structOrUnionSpecifier(tok, ast) || enumSpecifier(tok, ast) || typedefName(tok, ast);
}

// iso c99 97
static int
typeQualifier(Token **tok, AST *ast)
{
	//TODO A_TYPEQUAL
	return 0;
}

// iso c99 97
static int
functionSpecifier(Token **tok, AST *ast)
{
	//TODO A_FUNCSPEC
	return 0;
}

// iso c99 97
static int
declarationSpecifiers(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;

	if(   storageClassSpecifier(&tmp, &tl) 
	   || typeSpecifier(&tmp, &tl) 
	   || typeQualifier(&tmp, &tl)
	   || functionSpecifier(&tmp, &tl) ){
		if(declarationSpecifiers(&tmp, &tr)){
			*ast = (AST){
				A_DECLSPECLIST,
				.left = copyAST(tl),
				.right = copyAST(tr)
			};

			*tok = tmp;
			return 1;
		}

		*ast = tl;
		*tok = tmp;
		return 1;
	}
	return 0;
}

// iso c99 97
static int
declaration(Token **tok, AST *ast)
{
	AST tl, tr;
	Token *tmp = *tok;

	if(declarationSpecifiers(&tmp, &tl)){
		int res = initDeclaratorList(&tmp, &tr);
		if(scanToken(&tmp, T_SEMI)){
			*ast = (AST){
				A_DECLARATION,
				.left = res ? copyAST(tr) : NULL,
				.right = copyAST(tl)
			};

			*tok = tmp;
			return 1;
		}
		if(res)
			freeASTLeaves(&tr);
		freeASTLeaves(&tl);
	}
	return 0;

}



AST*
genAST(Token **tok)
{
	AST *ast = allocAST();
	//if(expr(tok, ast))
	if(declaration(tok, ast))
		return ast;
	freeAST(ast);
	return NULL;
}
