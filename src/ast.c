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

static int
identifier(Token **tok, AST *ast)
{
	//TODO
	return 0;
}

static int
intlit(Token **tok, AST *ast)
{
	//TODO
	return 0;
}

static int
primaryexpr(Token **tok, AST *ast)
{
	//TODO
	return 0;
}

static int
postfixexpr(Token **tok, AST *ast)
{
	//TODO
	return 0;
}

static int
unaryoper(Token **tok, AST *ast)
{
	//TODO
	return 0;
}

static int
unaryexpr(Token **tok, AST *ast)
{
	//TODO
	return 0;
}

static int
muloper(Token **tok, AST *ast)
{
	//TODO
	return 0;
}

static int
expr(Token **tok, AST *ast)
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
	if(expr(tok, ast)){
		printAST(ast);
		printf("\n");
		return ast;
	}
	return NULL;
}
