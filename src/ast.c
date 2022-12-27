#include"ast.h"
#include<stdlib.h>
#include<stddef.h>
#include<stdio.h>

void
freeAST(AST *ast)
{
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

static int
postfixExpr(Token **tok, AST **ast)
{
	//TODO
	return 0;
}

static int
unaryOper(Token **tok, AST **ast)
{
	//TODO
	return 0;
}

static int
unaryExpr(Token **tok, AST **ast)
{
	//TODO
	return 0;
}

static int
mulOper(Token **tok, AST **ast)
{
	//TODO
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
	if(primaryExpr(tok, &ast)){
		printAST(ast);
		printf("\n");
		return ast;
	}
	return NULL;
}
