#include"ast.h"
#include<stdlib.h>
#include<stddef.h>
#include<stdio.h>

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

// iso c99 69
static int
identifier(Token **tok, AST *ast)
{
	if((*tok)->token == T_IDENTIFIER){
		*ast = (AST){
			A_IDENT,
			.start = (*tok)->start,
			.end = (*tok)->end
		};

		*tok = (*tok)->next;
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
	if((*tok)->token == T_STRINGLIT){
		//TODO do escape sequences and whatnot (alredy implimented just need to make a string)
		*ast = (AST){
			A_STRLIT,
			.start = (*tok)->start,
			.end = (*tok)->end,
		};

		*tok = (*tok)->next;
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

	if((*tok)->token == T_LPAREN){
		Token *tmp = (*tok)->next;
		if(expr(&tmp, ast) && tmp->token == T_RPAREN){
			*tok = tmp->next;
			return 1;
		}
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
	AST *lamp, *ramp = NULL;
	Token *tmp = *tok;
	
	if(assignmentExpr(&tmp, &tl)){
		Token *ttmp = tmp->next;
		
		if(tmp->token == T_COMMA && exprList(&ttmp, &tr)){
			lamp = allocAST();
			ramp = allocAST();
			*lamp = tl;
			*ramp = tr;
			
			*ast = (AST){
				A_EXPRLIST,
				.left = lamp,
				.right = ramp
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
	AST *lamp, *ramp;
	Token *tmp = *tok;
	int type = -1;
	
	if(primaryExpr(&tmp, &tl)){
		while(1){
			Token *ttmp = tmp;
			lamp = allocAST();
			*lamp = tl;
			tl.left = lamp;

			type = -1;
			switch(tmp->token){
				case T_DOT: type = A_MEMBER; break;
				case T_ARROW: type = A_ARROW; break;
			}

			ttmp = tmp->next;
			if(type >= 0 && identifier(&ttmp, &tr)){
				tmp = ttmp;
				ramp = allocAST();
				*ramp = tr;

				tl = (AST){
					type,
					.left = tl.left,
					.right = ramp
				};
				continue;
			}

			ttmp = tmp->next;
			if(tmp->token == T_LBRACE && expr(&ttmp, &tr)){
				if(ttmp->token == T_RBRACE){
					tmp = ttmp->next;
					ramp = allocAST();
					*ramp = tr;

					tl = (AST){
						A_DEREF,
						.left = tl.left,
						.right = ramp
					};
					continue;
				}
				freeASTLeaves(&tr);
			}
			
			ttmp = tmp->next;
			if(tmp->token == T_LPAREN){
			       	if(!argumentExprList(&ttmp, &tr))
					tr = (AST){.left = NULL, .right = NULL};
				if(ttmp->token == T_RPAREN){
				freeAST(tr.right);
					tmp = ttmp->next;
					ramp = allocAST();
					*ramp = tr;

					tl = (AST){
						A_CALL,
						.left = tl.left,
						.right = ramp
					};
					continue;
				}
				freeASTLeaves(&tr);
			}

			type = -1;
			switch(tmp->token){
				case T_INC: type = A_PINC; break;
				case T_DEC: type = A_PDEC; break;
			}

			if(type < 0)
				break;

			tl = (AST){
				type,
				.left = tl.left,
				.right = NULL
			};
				
		}

		*tok = tmp;
		lamp = tl.left;
		
		*ast = *lamp;
		lamp->left = lamp->right = NULL;
		freeAST(lamp);
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
		AST *lamp = allocAST();
		*lamp = tl;

		*ast = (AST){
			type,
			.left = lamp,
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
	// todo
	if((*tok)->token == T_LPAREN){
		Token *tmp = (*tok)->next;
		AST *lamp = allocAST();
		AST *ramp = allocAST();
		if(typeName(&tmp, lamp)){
			if(tmp->token == T_RPAREN){
				tmp = tmp->next;
				if(castExpr(&tmp, ramp)){
					*tok = tmp;
					ast->left = lamp;
					ast->right = ramp;
					ast->type = A_CAST;
					return 1;
				}
			}
		}	
		freeAST(lamp);
		freeAST(ramp);
	}
	return unaryExpr(tok, ast);
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
			if(type < 0 || !prev(&tmp->next, &tr))
				break;
			tmp = tmp->next;

			AST *lamp = allocAST(),
			    *ramp = allocAST();
			*lamp = tl;
			*ramp = tr;
			
			tl = (AST){
				type,
				.left = lamp,
				.right = ramp
			};
		};

		*ast = tl;
		*tok = tmp;
		return 1;
	}
	
	return 0;
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
	AST *lamp, *mamp, *ramp;
	Token *tmp = *tok;

	if(borExpr(&tmp, &tl)){
		Token *ttmp = tmp;
		if(ttmp->token == T_QMARK){
			ttmp = ttmp->next;
			if(expr(&ttmp, &tm) && ttmp->token == T_COLON){
				ttmp = ttmp->next;
				if(elvisExpr(&ttmp, &tr)){
					lamp = allocAST();
					mamp = allocAST();
					ramp = allocAST();
					*mamp = tm;
					*ramp = tr;

					*lamp = (AST){
						A_CONDRES,
						.left = mamp,
						.right = ramp
					};

					ramp = allocAST();
					*ramp = tl;

					*ast = (AST){
						A_COND,
						.left = ramp,
						.right = lamp
					};

					*tok = ttmp;
					return 1;
				}
				freeASTLeaves(&tm);
			}

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
	AST *lamp, *ramp;
	Token *tmp = *tok;
	int type;

	if(unaryExpr(&tmp, &tl)){
		type = assignmentOper(tmp->token);
		tmp = tmp->next;
		if(type >= 0 && assignmentExpr(&tmp, &tr)){
			lamp = allocAST();
			ramp = allocAST();
			*lamp = tl;
			*ramp = tr;

			*ast = (AST){
				type,
				.left = lamp,
				.right = ramp
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
	AST *lamp, *ramp;
	Token *tmp = *tok;
	if(declarator(&tmp, &tl)){
		if(tmp->token == T_EQUALS){
			tmp = tmp->next;
			if(initializer(&tmp, &tr)){
				lamp = allocAST();
				ramp = allocAST();
				*lamp = tl;
				*ramp = tr;
				
				*ast = (AST){
					A_INITIALIZER,
					.left = lamp,
					.right = ramp
				};

				*tok = tmp;
				return 1;
			}
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
	AST *lamp, *ramp = NULL;
	Token *tmp = *tok;
	
	if(initDeclarator(&tmp, &tl)){
		Token *ttmp = tmp->next;
		
		if(tmp->token == T_COMMA && initDeclarator(&ttmp, &tr)){
			lamp = allocAST();
			ramp = allocAST();
			*lamp = tl;
			*ramp = tr;
			
			*ast = (AST){
				A_DECLLIST,
				.left = lamp,
				.right = ramp
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

// iso c99 101
static int
structOrUnionSpecifier(Token **tok, AST *ast)
{
	//TODO
	return 0;
}

// iso c99 99
static int
enumSpecifier(Token **tok, AST *ast)
{
	//TODO
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
	AST *lamp, *ramp;
	Token *tmp = *tok;

	if(   storageClassSpecifier(&tmp, &tl) || typeSpecifier(&tmp, &tl) 
	   || typeQualifier(&tmp, &tl) || functionSpecifier(&tmp, &tl)     ){
		if(declarationSpecifiers(&tmp, &tr)){
			lamp = allocAST();
			ramp = allocAST();
			*lamp = tl;
			*ramp = tr;
			
			*ast = (AST){
				A_DECLSPECLIST,
				.left = lamp,
				.right = ramp
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
	AST *lamp, *ramp = NULL;
	Token *tmp = *tok;

	if(declarationSpecifiers(&tmp, &tl)){
		int res = initDeclaratorList(&tmp, &tr);
		if(tmp->token == T_SEMI){
			tmp = tmp->next;
			if(res){
				ramp = allocAST();
				*ramp = tr;
			}
			lamp = allocAST();
			*lamp = tl;
	
			*ast = (AST){
				A_DECLARATION,
				.left = lamp,
				.right = ramp
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
	"A_DECLARATION", "A_DECLSPEC", "A_DECLSPECLIST", "A_DECLLIST", "A_DECLARATOR", "A_INITIALIZER"
	"A_STORAGESPEC", "A_TYPESPEC", "A_TYPEQUAL", "A_FUNCSPEC"
};

void
printAST(AST *ast)
{
	if(ast == NULL)
		return;
	
	printf("(");
	if(ast->type == A_INTLIT){
		printf("%d", ast->intValue);
		printf(")");
		return;
	}
	
	if(ast->type == A_IDENT){
		printf("%.*s", ast->end - ast->start, ast->start);
		printf(")");
		return;
	}

	printf("%s", astName[ast->type]);
	printAST(ast->left);
	printAST(ast->right);
	printf(")");
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
