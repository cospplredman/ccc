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

// iso c99 70
static int
argumentExprList(Token **tok, AST *ast)
{
	//TODO
	return 0;
}

// iso c99 69
static int
postfixExpr(Token **tok, AST *ast)
{
	if((*tok)->token == T_LPAREN){
		//TODO
	}

	AST *lamp = allocAST();
	AST *ramp;
	Token *tmp = *tok;
	
	if(primaryExpr(&tmp, lamp)){
		
		//TODO fix
		while(1){
			AST *tamp = allocAST();
			tamp->left = lamp;
			lamp = tamp;

			switch(tmp->token){
				case T_LBRACE:
					tmp = tmp->next;
					ramp = allocAST();

					if(expr(&tmp, ramp) && tmp->token == T_RBRACE){
						tmp = tmp->next;
						lamp->right = ramp;
						lamp->type = A_DEREF;
					}else{
						freeAST(ramp);
					}

					ramp = NULL;
					break;
				/*
				case T_LPAREN:
					ramp = allocAST();
					if(argumentExprList(&tmp, &ramp)){
						//todo a_call	
					}
					break;
				*/
				case T_DOT:
					tmp = tmp->next;
					ramp = allocAST();
					if(identifier(&tmp, ramp)){
						lamp->right = ramp;
						lamp->type = A_MEMBER;
					}else{
						freeAST(ramp);
					}
					ramp = NULL;
					break;
				case T_ARROW:
					tmp = tmp->next;
					ramp = allocAST();
					if(identifier(&tmp, ramp)){
						lamp->right = ramp;
						lamp->type = A_ARROW;
					}else{
						freeAST(ramp);
					}
					ramp = NULL;
					break;
				case T_INC:
					tmp = tmp->next;
					freeAST(ramp);
					lamp->right = NULL;
					lamp->type = A_PINC;
					break;
				case T_DEC:
					tmp = tmp->next;
					freeAST(ramp);
					lamp->right = NULL;
					lamp->type = A_PDEC;
					break;
				default:
					*tok = tmp;
					lamp = tamp->left;
					tamp->left = NULL;
					freeAST(tamp);
					
					*ast = *lamp;
					lamp->left = lamp->right = NULL;
					freeAST(lamp);
					return 1;
					break;
			}
		}
	}

	freeAST(lamp);
	return 0;
}

static int unaryExpr(Token **, AST *);

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

// iso c99 78
static int
unaryExpr(Token **tok, AST *ast)
{
	AST *lamp = allocAST();
	Token *tmp = (*tok)->next;
	switch((*tok)->token){
		case T_INC:
			if(unaryExpr(&tmp, lamp)){
				*tok = tmp;
				ast->left = lamp;
				ast->right = NULL;
				ast->type = A_INC;
				return 1;
			}
			break;
		case T_DEC:
			if(unaryExpr(&tmp, lamp)){
				*tok = tmp;
				ast->left = lamp;
				ast->right = NULL;
				ast->type = A_DEC;
				return 1;
			}
			break;
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
		case T_AND:
			if(castExpr(&tmp, lamp)){
				*tok = tmp;
				ast->left = lamp;
				ast->right = NULL;
				ast->type = A_ADDR;
				return 1;
			}
			break;
		case T_STAR:
			if(castExpr(&tmp, lamp)){
				*tok = tmp;
				ast->left = lamp;
				ast->right = NULL;
				ast->type = A_UDEREF;
				return 1;
			}
			break;
		case T_PLUS:
			if(castExpr(&tmp, lamp)){
				*tok = tmp;
				ast->left = lamp;
				ast->right = NULL;
				ast->type = A_UADD;
				return 1;
			}
			break;
		case T_MINUS:
			if(castExpr(&tmp, lamp)){
				*tok = tmp;
				ast->left = lamp;
				ast->right = NULL;
				ast->type = A_USUB;
				return 1;
			}
			break;
		case T_TILDE:
			if(castExpr(&tmp, lamp)){
				*tok = tmp;
				ast->left = lamp;
				ast->right = NULL;
				ast->type = A_NEG;
				return 1;
			}
			break;
		case T_BANG:
			if(castExpr(&tmp, lamp)){
				*tok = tmp;
				ast->left = lamp;
				ast->right = NULL;
				ast->type = A_BNEG;
				return 1;
			}
			break;
	}

	freeAST(lamp);
	return postfixExpr(tok, ast);
}

static int
Expr(int (*prev)(Token**, AST*), int (*oper)(int), Token **tok, AST *ast)
{
	AST tl, tr, *lamp = &tl, *ramp = &tr;
	Token *tmp = *tok;
	int type;

	if(prev(&tmp, lamp)){
		type = oper(tmp->token);

		if(type < 0 || !prev(&tmp->next, ramp)){
			*ast = tl;
		}else{
			lamp = allocAST();
			ramp = allocAST();
			tmp = tmp->next;
			*lamp = tl;
			*ramp = tr;
			
			*ast = (AST){
				type,
				.left = lamp,
				.right = ramp
			};
		}
		
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
	return Expr(castExpr, mulOper, tok, ast);
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
	return Expr(mulExpr, addOper, tok, ast);
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
	return Expr(addExpr, shiftOper, tok, ast);
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
	return Expr(shiftExpr, relOper, tok, ast);
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
	return Expr(relExpr, eqOper, tok, ast);
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
	return Expr(eqExpr, andOper, tok, ast);
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
	return Expr(andExpr, xorOper, tok, ast);
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
	return Expr(xorExpr, orOper, tok, ast);
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
	return Expr(orExpr, bandOper, tok, ast);
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
	return Expr(bandExpr, borOper, tok, ast);
}

// iso c99 90
static int
elvisExpr(Token **tok, AST *ast)
{
	AST *lamp = allocAST();
	AST *ramp = allocAST();
	Token *tmp = *tok;

	if(borExpr(&tmp, lamp)){
		/*
		//TODO
		switch(tmp->token){
			case T_BAND:
				tmp = tmp->next;
				if(borExpr(&tmp, ramp)){
					*tok = tmp;
					ast->left = lamp;
					ast->right = ramp;
					ast->type = A_BAND;
					return 1;
				}
			break;
		}
		*/
		*tok = tmp;
		*ast = *lamp;
		lamp->left = lamp->right = NULL;
		freeAST(lamp);
		freeAST(ramp);
		return 1;
	}
	
	freeAST(lamp);
	freeAST(ramp);

	return 0;
}

// iso c99 91
static int
assignmentExpr(Token **tok, AST *ast)
{
	return elvisExpr(tok, ast);
	//TODO
	return 0;
}

//TODO
// iso c99 ?? 
static int
expr(Token **tok, AST *ast)
{
	return assignmentExpr(tok, ast) || elvisExpr(tok, ast);
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

	"A_ELVIS"
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
	if(expr(tok, ast))
		return ast;
	return NULL;
}
