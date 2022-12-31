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
	return malloc(sizeof(AST));
}

// iso c99 69
static int
identifier(Token **tok, AST **ast)
{
	if((*tok)->token == T_IDENTIFIER){
		(*ast)->type = A_IDENT;
		(*ast)->start = (*tok)->start;
		(*ast)->end = (*tok)->end;
		*tok = (*tok)->next;
		return 1;
	}
		
	return 0;
}

// iso c99 69
static int
constant(Token **tok, AST **ast)
{
	switch((*tok)->token){
		case T_INTEGERCONST:
		case T_FLOATCONST:
		case T_CHARCONST:
		case T_STRINGLIT:
			(*ast)->type = A_INTLIT;
			(*ast)->intValue = (*tok)->intValue;
			*tok = (*tok)->next;
			return 1;
		break;
	}
	
	return 0;
}

// iso c99 69
static int
stringLit(Token **tok, AST **ast)
{
	if((*tok)->token == T_STRINGLIT){
		(*ast)->type = A_STRLIT;
		//TODO do escape sequences and whatnot (alredy implimented just need to make a string)
		(*ast)->start = (*tok)->start;
		(*ast)->end = (*tok)->end;
		*tok = (*tok)->next;
		return 1;
	}

	return 0;
}

static int expr(Token**, AST**);

// iso c99 69
static int
primaryExpr(Token **tok, AST **ast)
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
typeName(Token **tok, AST **ast)
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

static int
assignmentExpr(Token **tok, AST *ast)
{
	//TODO
	return 0;
}

// iso c99 70
static int
argumentExprList(Token **tok, AST **ast)
{
	//TODO
	return 0;
}

// iso c99 69
static int
postfixExpr(Token **tok, AST **ast)
{
	if((*tok)->token == T_LPAREN){
		//TODO
	}

	AST *lamp = allocAST();
	Token *tmp = *tok;
	
	if(primaryExpr(&tmp, &lamp)){
		AST *amp = lamp, *ramp;
		
		//TODO fix
		while(1){
			AST *tamp = allocAST();
			tamp->left = amp;
			amp = tamp;

			switch(tmp->token){
				case T_LBRACE:
					tmp = tmp->next;
					ramp = allocAST();
					if(expr(&tmp, &ramp) && tmp->token == T_RBRACE){
						tmp = tmp->next;
						amp->right = ramp;
						amp->type = A_DEREF;
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
					if(identifier(&tmp, &ramp)){
						amp->right = ramp;
						amp->type = A_MEMBER;
					}else{
						freeAST(ramp);
					}
					ramp = NULL;
					break;
				case T_ARROW:
					tmp = tmp->next;
					ramp = allocAST();
					if(identifier(&tmp, &ramp)){
						amp->right = ramp;
						amp->type = A_ARROW;
					}else{
						freeAST(ramp);
					}
					ramp = NULL;
					break;
				case T_INC:
					tmp = tmp->next;
					freeAST(ramp);
					amp->right = NULL;
					amp->type = A_PINC;
					break;
				case T_DEC:
					tmp = tmp->next;
					freeAST(ramp);
					amp->right = NULL;
					amp->type = A_PDEC;
					break;
				break;
				default:
					*tok = tmp;
					amp = tamp->left;
					tamp->left = NULL;
					freeAST(tamp);

					**ast = *amp;
					amp->left = amp->right = NULL;
					freeAST(amp);
					return 1;
				break;
			}
		}
	}

	freeAST(lamp);
	return 0;
}

static int unaryExpr(Token **, AST **);

// iso c99 78
static int
castExpr(Token **tok, AST **ast){
	// todo
	if((*tok)->token == T_LPAREN){
		Token *tmp = (*tok)->next;
		AST *lamp = allocAST();
		AST *ramp = allocAST();
		if(typeName(&tmp, &lamp)){
			if(tmp->token == T_RPAREN){
				tmp = tmp->next;
				if(castExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_CAST;
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
unaryExpr(Token **tok, AST **ast)
{
	AST *lamp = allocAST();
	Token *tmp = (*tok)->next;
	switch((*tok)->token){
		case T_INC:
			if(unaryExpr(&tmp, &lamp)){
				*tok = tmp;
				(*ast)->left = lamp;
				(*ast)->right = NULL;
				(*ast)->type = A_INC;
				return 1;
			}
			break;
		case T_DEC:
			if(unaryExpr(&tmp, &lamp)){
				*tok = tmp;
				(*ast)->left = lamp;
				(*ast)->right = NULL;
				(*ast)->type = A_DEC;
				return 1;
			}
			break;
		/*
		case T_SIZEOF:
			if(unaryExpr(&tmp, &lamp)){
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
			if(castExpr(&tmp, &lamp)){
				*tok = tmp;
				(*ast)->left = lamp;
				(*ast)->right = NULL;
				(*ast)->type = A_ADDR;
				return 1;
			}
			break;
		case T_STAR:
			if(castExpr(&tmp, &lamp)){
				*tok = tmp;
				(*ast)->left = lamp;
				(*ast)->right = NULL;
				(*ast)->type = A_UDEREF;
				return 1;
			}
			break;
		case T_PLUS:
			if(castExpr(&tmp, &lamp)){
				*tok = tmp;
				(*ast)->left = lamp;
				(*ast)->right = NULL;
				(*ast)->type = A_UADD;
				return 1;
			}
			break;
		case T_MINUS:
			if(castExpr(&tmp, &lamp)){
				*tok = tmp;
				(*ast)->left = lamp;
				(*ast)->right = NULL;
				(*ast)->type = A_USUB;
				return 1;
			}
			break;
		case T_TILDE:
			if(castExpr(&tmp, &lamp)){
				*tok = tmp;
				(*ast)->left = lamp;
				(*ast)->right = NULL;
				(*ast)->type = A_NEG;
				return 1;
			}
			break;
		case T_BANG:
			if(castExpr(&tmp, &lamp)){
				*tok = tmp;
				(*ast)->left = lamp;
				(*ast)->right = NULL;
				(*ast)->type = A_BNEG;
				return 1;
			}
			break;
	}

	freeAST(lamp);
	return postfixExpr(tok, ast);
}

static int
mulExpr(Token **tok, AST **ast)
{
	AST *lamp = allocAST();
	AST *ramp = allocAST();
	Token *tmp = *tok;

	if(castExpr(&tmp, &lamp)){
		switch(tmp->token){
			case T_STAR:
				tmp = tmp->next;
				if(castExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_MUL;
					return 1;
				}
			break;
			case T_SLASH:
				tmp = tmp->next;
				if(castExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_DIV;
					return 1;
				}
			break;
			case T_PERCENT:
				tmp = tmp->next;
				if(castExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_MOD;
					return 1;
				}
			break;
		}

		*tok = tmp;
		**ast = *lamp;
		lamp->left = lamp->right = NULL;
		freeAST(lamp);
		freeAST(ramp);
		return 1;
	}
	
	freeAST(lamp);
	freeAST(ramp);

	return 0;
}

static int
addExpr(Token **tok, AST **ast)
{
	AST *lamp = allocAST();
	AST *ramp = allocAST();
	Token *tmp = *tok;

	if(mulExpr(&tmp, &lamp)){
		switch(tmp->token){
			case T_PLUS:
				tmp = tmp->next;
				if(mulExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_ADD;
					return 1;
				}
			break;
			case T_MINUS:
				tmp = tmp->next;
				if(mulExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_SUB;
					return 1;
				}
			break;
		}

		*tok = tmp;
		**ast = *lamp;
		lamp->left = lamp->right = NULL;
		freeAST(lamp);
		freeAST(ramp);
		return 1;
	}
	
	freeAST(lamp);
	freeAST(ramp);

	return 0;
}

static int
shiftExpr(Token **tok, AST **ast)
{
	AST *lamp = allocAST();
	AST *ramp = allocAST();
	Token *tmp = *tok;

	if(addExpr(&tmp, &lamp)){
		switch(tmp->token){
			case T_LSHIFT:
				tmp = tmp->next;
				if(addExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_LSHIFT;
					return 1;
				}
			break;
			case T_RSHIFT:
				tmp = tmp->next;
				if(addExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_RSHIFT;
					return 1;
				}
			break;
		}

		*tok = tmp;
		**ast = *lamp;
		lamp->left = lamp->right = NULL;
		freeAST(lamp);
		freeAST(ramp);
		return 1;
	}
	
	freeAST(lamp);
	freeAST(ramp);

	return 0;
}

static int
relExpr(Token **tok, AST **ast)
{
	AST *lamp = allocAST();
	AST *ramp = allocAST();
	Token *tmp = *tok;

	if(shiftExpr(&tmp, &lamp)){
		switch(tmp->token){
			case T_LT:
				tmp = tmp->next;
				if(shiftExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_LT;
					return 1;
				}
			break;
			case T_GT:
				tmp = tmp->next;
				if(shiftExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_GT;
					return 1;
				}
			break;
			case T_LTEQ:
				tmp = tmp->next;
				if(shiftExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_LTEQ;
					return 1;
				}
			break;
			case T_GTEQ:
				tmp = tmp->next;
				if(shiftExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_GTEQ;
					return 1;
				}
			break;
		}

		*tok = tmp;
		**ast = *lamp;
		lamp->left = lamp->right = NULL;
		freeAST(lamp);
		freeAST(ramp);
		return 1;
	}
	
	freeAST(lamp);
	freeAST(ramp);

	return 0;
}
	
static int
eqExpr(Token **tok, AST **ast)
{
	AST *lamp = allocAST();
	AST *ramp = allocAST();
	Token *tmp = *tok;

	if(relExpr(&tmp, &lamp)){
		switch(tmp->token){
			case T_EQUAL:
				tmp = tmp->next;
				if(relExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_EQUALS;
					return 1;
				}
			break;
			case T_NOTEQUAL:
				tmp = tmp->next;
				if(relExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_NOTEQUALS;
					return 1;
				}
			break;
		}

		*tok = tmp;
		**ast = *lamp;
		lamp->left = lamp->right = NULL;
		freeAST(lamp);
		freeAST(ramp);
		return 1;
	}
	
	freeAST(lamp);
	freeAST(ramp);

	return 0;
}

static int
andExpr(Token **tok, AST **ast){
	AST *lamp = allocAST();
	AST *ramp = allocAST();
	Token *tmp = *tok;

	if(eqExpr(&tmp, &lamp)){
		switch(tmp->token){
			case T_AND:
				tmp = tmp->next;
				if(eqExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_AND;
					return 1;
				}
			break;
		}

		*tok = tmp;
		**ast = *lamp;
		lamp->left = lamp->right = NULL;
		freeAST(lamp);
		freeAST(ramp);
		return 1;
	}
	
	freeAST(lamp);
	freeAST(ramp);

	return 0;
}

static int
xorExpr(Token **tok, AST **ast){
	AST *lamp = allocAST();
	AST *ramp = allocAST();
	Token *tmp = *tok;

	if(andExpr(&tmp, &lamp)){
		switch(tmp->token){
			case T_XOR:
				tmp = tmp->next;
				if(andExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_XOR;
					return 1;
				}
			break;
		}

		*tok = tmp;
		**ast = *lamp;
		lamp->left = lamp->right = NULL;
		freeAST(lamp);
		freeAST(ramp);
		return 1;
	}
	
	freeAST(lamp);
	freeAST(ramp);

	return 0;
}

static int
orExpr(Token **tok, AST **ast){
	AST *lamp = allocAST();
	AST *ramp = allocAST();
	Token *tmp = *tok;

	if(xorExpr(&tmp, &lamp)){
		switch(tmp->token){
			case T_OR:
				tmp = tmp->next;
				if(xorExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_OR;
					return 1;
				}
			break;
		}

		*tok = tmp;
		**ast = *lamp;
		lamp->left = lamp->right = NULL;
		freeAST(lamp);
		freeAST(ramp);
		return 1;
	}
	
	freeAST(lamp);
	freeAST(ramp);

	return 0;
}

static int
bandExpr(Token **tok, AST **ast){
	AST *lamp = allocAST();
	AST *ramp = allocAST();
	Token *tmp = *tok;

	if(orExpr(&tmp, &lamp)){
		switch(tmp->token){
			case T_BAND:
				tmp = tmp->next;
				if(orExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_BAND;
					return 1;
				}
			break;
		}

		*tok = tmp;
		**ast = *lamp;
		lamp->left = lamp->right = NULL;
		freeAST(lamp);
		freeAST(ramp);
		return 1;
	}
	
	freeAST(lamp);
	freeAST(ramp);

	return 0;
}

static int
borExpr(Token **tok, AST **ast){
	AST *lamp = allocAST();
	AST *ramp = allocAST();
	Token *tmp = *tok;

	if(bandExpr(&tmp, &lamp)){
		switch(tmp->token){
			case T_BOR:
				tmp = tmp->next;
				if(bandExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_BOR;
					return 1;
				}
			break;
		}

		*tok = tmp;
		**ast = *lamp;
		lamp->left = lamp->right = NULL;
		freeAST(lamp);
		freeAST(ramp);
		return 1;
	}
	
	freeAST(lamp);
	freeAST(ramp);

	return 0;
}

static int
elvisExpr(Token **tok, AST **ast){
	AST *lamp = allocAST();
	AST *ramp = allocAST();
	Token *tmp = *tok;

	if(borExpr(&tmp, &lamp)){
		/*
		//TODO
		switch(tmp->token){
			case T_BAND:
				tmp = tmp->next;
				if(borExpr(&tmp, &ramp)){
					*tok = tmp;
					(*ast)->left = lamp;
					(*ast)->right = ramp;
					(*ast)->type = A_BAND;
					return 1;
				}
			break;
		}
		*/
		*tok = tmp;
		**ast = *lamp;
		lamp->left = lamp->right = NULL;
		freeAST(lamp);
		freeAST(ramp);
		return 1;
	}
	
	freeAST(lamp);
	freeAST(ramp);

	return 0;
}

static int
expr(Token **tok, AST **ast)
{
	//TODO
	return 0;
}

void
printAST(AST *ast)
{
	if(ast == NULL)
		return;
	if(ast->type == A_INTLIT){
		printf("%d ", ast->intValue);
		return;
	}
	
	if(ast->type == A_IDENT){
		printf("%.*s ", ast->end - ast->start, ast->start);
		return;
	}

	printf("%d ", ast->type);
	printf("(");
	printAST(ast->left);
	printf(") (");
	printAST(ast->right);
	printf(")");
}

AST*
genAST(Token **tok)
{
	AST *ast = allocAST();
	if(relExpr(tok, &ast)){
		printAST(ast);
		printf("\n");
		return ast;
	}
	return NULL;
}
