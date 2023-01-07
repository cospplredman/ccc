#include"ast.h"
#include<stdlib.h>
#include<stddef.h>
#include<stdio.h>

/* util functions */

void
freeAST(AST *ast)
{
	if(ast){	
		if(ast->flags & AF_NODE){
			AST **data = (AST**)ast->data;
			for(int i = 0; i < ast->length; i++)
				freeAST(data[i]);
		}

		if(ast->flags & AF_OWNSCOPE){
			freeHashTable(ast->scope);
		}

		free(ast);	
	}
}


AST*
allocAST(int entries)
{
	return (AST*)malloc(sizeof(AST) + sizeof(void*)*entries);
}

static AST*
initNode(int type, AST *l, AST *r)
{
	AST *tl = allocAST(2);
	*tl = (AST){
		.ASTtype = type,
		.length = 2,
		.type = NULL,
		.flags = AF_NODE,
	};
	tl->data[0] = l;
	tl->data[1] = r;

	return tl;
}

static AST*
initUNode(int type, AST *l)
{
	AST *tl = allocAST(1);
	*tl = (AST){
		.ASTtype = type,
		.length = 1,
		.type = NULL,
		.flags = AF_NODE,
	};
	tl->data[0] = l;

	return tl;
}

static AST*
initStrNode(int type, char *s, char *e)
{
	AST *tl = allocAST(2);
	*tl = (AST){
		.ASTtype = type,
		.length = 2,
		.type = NULL,
		.flags = AF_STRNODE,
	};
	tl->data[0] = s;
	tl->data[1] = e;

	return tl;
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
binExpr(int (*prev)(Token**, AST**), int (*oper)(int), Token **tok, AST **ast)
{
	AST *tl, *tr;
	Token *tmp = *tok;
	int type;

	if(prev(&tmp, &tl)){
		while(1){
			type = oper(tmp->token);

			Token *ttmp = tmp->next;
			if(type < 0 || !prev(&ttmp, &tr))
				break;
			
			tl = initNode(type, tl, tr);
			tmp = ttmp;
		}

		*ast = tl;
		*tok = tmp;
		return 1;
	}
	
	return 0;
}

static int
List_(int (*of)(Token **, AST**), int type, Token **tok, AST **ast, int c)
{
	AST *tl, *tr;
	Token *tmp = *tok;
	
	if(of(&tmp, &tl)){
		Token *ttmp = tmp;
		if(scanToken(&ttmp, T_COMMA) && List_(of, type, &ttmp, &tr, c+1)){
			tr->data[c] = tl;
			*ast = tr;
			*tok = ttmp;
			return 1;
		}
		
		tr = allocAST(c + 1);
		*ast = tr;
		*tr = (AST){
			.ASTtype = type,
			.flags = AF_NODE,
			.type = NULL,
			.length = c + 1
		};
		tr->data[c] = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

static int
List(int (*of)(Token **, AST**), int type, Token **tok, AST **ast)
{
	return List_(of, type, tok, ast, 0);
}

static int
Sequence_(int (*of)(Token**, AST**), int type, Token **tok, AST **ast, int c)
{
	AST *tl, *tr;
	Token *tmp = *tok;

	if(of(&tmp, &tl)){
		if(Sequence_(of, type, &tmp, &tr, c+1)){
			tr->data[c] = tl;
			*ast = tr;
			*tok = tmp;
			return 1;
		}

		tr = allocAST(c + 1);
		*ast = tr;
		*tr = (AST){
			.ASTtype = type,
			.flags = AF_NODE,
			.type = NULL,
			.length = c + 1
		};
		tr->data[c] = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

static int
Sequence(int (*of)(Token**, AST**), int type, Token **tok, AST **ast)
{
	return Sequence_(of, type, tok, ast, 0);
}

static int
Capsule(int (*of)(Token**, AST**), const int lrpair[2], Token **tok, AST **ast)
{
	AST *tl;
	Token *tmp = *tok;

	if(scanToken(&tmp, lrpair[0]) && of(&tmp, &tl)){
		if(scanToken(&tmp, lrpair[1])){
			*ast = tl;
			*tok = tmp;
			return 1;
		}
		freeAST(tl);
	}

	return 0;
}

static int
CapsuleWComma(int (*of)(Token**, AST**), const int lrpair[2], Token **tok, AST **ast)
{
	AST *tl;
	Token *tmp = *tok;

	if(scanToken(&tmp, lrpair[0]) && of(&tmp, &tl)){
		scanToken(&tmp, T_COMMA);
		if(scanToken(&tmp, lrpair[1])){
			*ast = tl;
			*tok = tmp;
			return 1;
		}
		freeAST(tl);
	}

	return 0;
}

static const int PAREN[] = {T_LPAREN, T_RPAREN};
static const int CURLY[] = {T_LCURLY, T_RCURLY};
static const int BRACE[] = {T_LBRACE, T_RBRACE};

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
	"A_ENUMERATOR", "A_ENUMLIST", "A_ENUMSPEC", "A_DESINIT", "A_DESIGNATOR", "A_DESIGNATORLIST", "A_INITLIST",
	"A_POINTER", "A_IDENTLIST", "A_PARAMLIST", "A_PARAMDECL", "A_PARAMTYPELIST", "A_DIRECTDECLARATOR", "A_QUALLIST",
	"A_DIRECTABSTRACTDECL", "A_ABSTRACTDECLARATOR", "A_TYPENAME",
	"A_LABELSTATEMENT", "A_FOR", "A_WHILE", "A_DOWHILE", "A_FORLIST", "A_BLOCK", "A_RETURN", "A_CONTINUE", "A_BREAK", "A_SWITCH", "A_GOTO", "A_IFELSE", "A_IF", "A_BLOCKLIST", "A_EMPTYSTATEMENT",
	"A_DECLARATIONLIST", "A_FUNCDECL", "A_TRANSLATIONUNIT", "A_DVLA", "A_VLA", "A_EPL"
};

static void
printAST_(AST *ast, int depth)
{
	for(int i = 0; i < depth; i++)
		printf("   ");

	if(ast == NULL){
		printf("%p\n", NULL);
		return;
	}
	
	printf("%s:\n", astName[ast->ASTtype]);
	switch(ast->flags){	
		case AF_NODE:
			for(int i = 0; i < ast->length; i++)
				printAST_((AST*)ast->data[i], depth + 1);
			break;
		case AF_STRNODE:
			for(int i = 0; i < depth; i++)
				printf("   ");
			printf("   (%.*s)\n", (int)((char*)ast->data[1] - (char*)ast->data[0]), (char*)ast->data[0]);
			break;
		case AF_TOKNODE:
			for(int i = 0; i < depth; i++)
				printf("   ");
			printf("   ");
			printToken((Token*)ast->data[0]);
			break;
	}
}

void
printAST(AST *ast)
{
	printAST_(ast, 0);
}

/* ast generation functions */

// iso c99 69
static int
identifier(Token **tok, AST **ast)
{
	Token *tmp = *tok;
	if(scanToken(&tmp, T_IDENTIFIER)){
		*ast = initStrNode(A_IDENT, (*tok)->start, (*tok)->end);
		*tok = tmp;
		return 1;
	}
		
	return 0;
}

// iso c99 69
// string literals are parsed in the constant section aswell
static int
constant(Token **tok, AST **ast)
{
	switch((*tok)->token){
		case T_INTEGERCONST:
		case T_FLOATCONST:
		case T_CHARCONST:
		case T_STRINGLIT:
			*ast = initStrNode(A_INTLIT, (*tok)->start, (*tok)->end);
			*tok = (*tok)->next;
			return 1;
		break;
	}
	
	return 0;
}


static int expr(Token**, AST**);

// iso c99 69
static int
primaryExpr(Token **tok, AST **ast)
{
	return identifier(tok, ast) || constant(tok, ast) || Capsule(expr, PAREN, tok, ast);
}

static int typeName(Token **, AST **);
static int initializerList(Token **, AST **);
static int assignmentExpr(Token **, AST **);

// iso c99 70
static int
argumentExprList(Token **tok, AST **ast)
{
	return List(assignmentExpr, A_EXPRLIST, tok, ast);
}

// iso c99 69
static int
postfixExpr(Token **tok, AST **ast)
{

	AST *tl, *tr;
	Token *tmp = *tok, *ttmp;
	int type = -1;
	
	ttmp = *tok;

	if(Capsule(typeName, PAREN, &ttmp, &tl)){
		if(CapsuleWComma(initializerList, CURLY, &ttmp, &tr)){
			*ast = initNode(A_DESINIT, tl, tr);
			*tok = ttmp;
			return 1;
		}
		freeAST(tl);
	}

	if(primaryExpr(&tmp, &tl)){
		while(1){
			ttmp = tmp;
			if(Capsule(expr, BRACE, &ttmp, &tr)){
				tl = initNode(A_DEREF, tl, tr);
				tmp = ttmp;
				continue;
			}
			
			if(Capsule(argumentExprList, PAREN, &ttmp, &tr)){
				tl = initNode(A_CALL, tl, tr);
				tmp = ttmp;
				continue;
			}

			if(scanToken(&ttmp, T_LPAREN) && scanToken(&ttmp, T_RPAREN)){
				tl = initUNode(A_CALL, tl);
				tmp = ttmp;
				continue;
			}
			
			type = -1;
			switch(tmp->token){
				case T_DOT: type = A_MEMBER; break;
				case T_ARROW: type = A_ARROW; break;
			}
			ttmp = tmp->next;

			if(type >= 0 && identifier(&ttmp, &tr)){
				tl = initNode(type, tl, tr);
				tmp = ttmp;
				continue;
			}

			type = -1;
			switch(tmp->token){
				case T_INC: type = A_PINC; break;
				case T_DEC: type = A_PDEC; break;
			}
			ttmp = tmp->next;

			if(type >= 0){
				tl = initUNode(type, tl);
				tmp = ttmp;
				continue;
			}

			break;
		}

		*ast = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

static int castExpr(Token **, AST **);

// iso c99 78
static int
unaryExpr(Token **tok, AST **ast)
{
	AST *tl;
	Token *tmp = *tok, *ttmp;
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
		case T_SIZEOF:
			ttmp = tmp->next;
			if(unaryExpr(&ttmp, &tl) || Capsule(typeName, PAREN, &ttmp, &tl)){
				*ast = initUNode(A_SIZEOF, tl);
				*tok = ttmp;
				return 1;
			}
	}

	tmp = tmp->next;
	if(type >= 0 && castExpr(&tmp, &tl)){
		*ast = initUNode(type, tl);
		*tok = tmp;
		return 1;
	}

	return postfixExpr(tok, ast);
}

// iso c99 78
static int
castExpr(Token **tok, AST **ast){
	AST *tl, *tr;
	Token *tmp = *tok;

	if(Capsule(typeName, PAREN, &tmp, &tl)){
		if(castExpr(&tmp, &tr)){
			*ast = initNode(A_CAST, tl, tr);
			*tok = tmp;
			return 1;
		}
		freeAST(tl);
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
mulExpr(Token **tok, AST **ast)
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
addExpr(Token **tok, AST **ast)
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
shiftExpr(Token **tok, AST **ast)
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
relExpr(Token **tok, AST **ast)
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
eqExpr(Token **tok, AST **ast)
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
andExpr(Token **tok, AST **ast)
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
xorExpr(Token **tok, AST **ast)
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
orExpr(Token **tok, AST **ast)
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
bandExpr(Token **tok, AST **ast)
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
borExpr(Token **tok, AST **ast)
{
	return binExpr(bandExpr, borOper, tok, ast);
}

// iso c99 90
static int
elvisExpr(Token **tok, AST **ast)
{
	static const int lrpair[] = {T_QMARK, T_COLON};
	AST *tl, *tm, *tr;
	Token *tmp = *tok;

	if(borExpr(&tmp, &tl)){
		Token *ttmp = tmp;
		if(Capsule(expr, lrpair, &ttmp, &tm)){
			if(elvisExpr(&ttmp, &tr)){
				*ast = initNode(A_COND, tl, initNode(A_CONDRES, tm, tr));
				*tok = ttmp;
				return 1;
			}
			freeAST(tm);
		}

		*ast = tl;
		*tok = tmp;
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
assignmentExpr(Token **tok, AST **ast)
{
	AST *tl, *tr;
	Token *tmp = *tok;
	int type;

	if(unaryExpr(&tmp, &tl)){
		type = assignmentOper(tmp->token);
		tmp = tmp->next;

		if(type >= 0 && assignmentExpr(&tmp, &tr)){
			*ast = initNode(type, tl, tr);
			*tok = tmp;
			return 1;
		}
		
		freeAST(tl);
	}

	return elvisExpr(tok, ast);
}

// iso c99 94 
static int
expr(Token **tok, AST **ast)
{
	return List(assignmentExpr, A_EXPRLIST, tok, ast);
}

static int qualifierList(Token **, AST **);

static int
pointerEntry(Token **tok, AST **ast)
{
	AST *tl;
	Token *tmp = *tok;

	if(scanToken(&tmp, T_STAR)){
		if(qualifierList(&tmp, &tl)){
			*ast = tl;
			*tok = tmp;
			return 1;
		}

		//TODO not null
		*ast = initUNode(A_TYPEQUAL, NULL);
		*tok = tmp;
		return 1;
	}

	return 0;
}


// iso c99 114
static int
pointer(Token **tok, AST **ast)
{
	return Sequence(pointerEntry, A_POINTER, tok, ast);	
}

// iso c99 114
static int
identifierList(Token **tok, AST **ast)
{
	return List(identifier, A_IDENTLIST, tok, ast);
}

static int declarationSpecifiers(Token **, AST **);
static int declarator(Token **, AST **);
static int abstractDeclarator(Token **, AST **);
static int paramTypeList(Token **, AST **);

static int
directAbstractSequence(Token **tok, AST **ast)
{
	Token *tmp = *tok;

	if(scanToken(&tmp, T_RBRACE) && scanToken(&tmp, T_STAR) && scanToken(&tmp, T_RBRACE)){
		*ast = initUNode(A_DVLA, NULL);
		*tok = tmp;	
		return 1;
	}

	tmp = *tok;
	if(scanToken(&tmp, T_LBRACE) && scanToken(&tmp, T_RBRACE)){
		//TODO better asttype
		*ast = initUNode(A_VLA, NULL);
		*tok = tmp;	
		return 1;
	}

	tmp = *tok;
	if(scanToken(&tmp, T_LPAREN) && scanToken(&tmp, T_RPAREN)){
		*ast = initUNode(A_EPL, NULL);
		*tok = tmp;	
		return 1;
	}

	
	return Capsule(assignmentExpr, BRACE, tok, ast) || Capsule(paramTypeList, PAREN, tok, ast);

}

// iso c99 122
static int
directAbstractDeclarator(Token **tok, AST **ast)
{
	AST *tl = NULL, *tr = NULL;
	Token *tmp = *tok;
	printf("\nhi\n");

	Capsule(abstractDeclarator, PAREN, &tmp, &tl);
	Sequence(directAbstractSequence, A_DIRECTABSTRACTDECL, &tmp, &tr);

	if(tl || tr){
		*ast = initNode(A_DIRECTABSTRACTDECL, tl, tr);
		*tok = tmp;
		return 1;
	}

	return 0;
}

static int pointer(Token **,  AST **);

// iso c99 122
static int
abstractDeclarator(Token **tok, AST **ast)
{
	AST *tl = NULL, *tr;
	Token *tmp = *tok;

	pointer(&tmp, &tl);

	if(directAbstractDeclarator(&tmp, &tr)){
		*ast = tl ? initNode(A_ABSTRACTDECLARATOR, tl, tr) : tr;
		*tok = tmp;
		return 1;
	}

	if(tl){
		*ast = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

static int specifierQualifierList(Token **, AST **);

// iso c99 122
static int
typeName(Token **tok, AST **ast)
{
	AST *tl, *tr = NULL;
	Token *tmp = *tok;

	if(specifierQualifierList(&tmp, &tl)){
		abstractDeclarator(&tmp, &tr);

		*ast = initNode(A_TYPENAME, tl, tr);
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 114
static int
paramDecl(Token **tok, AST **ast)
{
	AST *tl, *tr;
	Token *tmp = *tok;

	if(declarationSpecifiers(&tmp, &tl)){
		if(declarator(&tmp, &tr) || abstractDeclarator(&tmp, &tr)){
			*ast = initNode(A_PARAMDECL, tl, tr);
			*tok = tmp;
			return 1;
		}

		*ast = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 114
static int
paramList(Token **tok, AST **ast)
{
	return List(paramDecl, A_PARAMLIST, tok, ast);
}

// iso c99 114
static int
paramTypeList(Token **tok, AST **ast)
{
	return List(paramList, A_PARAMTYPELIST, tok, ast);
}

static int typeQualifier(Token **, AST **);
// iso c99 114
static int
typeQualifierList(Token **tok, AST **ast){
	return Sequence(typeQualifier, A_QUALLIST, tok, ast);
}

static int
directSequence(Token **tok, AST **ast)
{
	AST *tl, *tm, *tr;
	Token *tmp = *tok;

	if(scanToken(&tmp, T_LPAREN) && scanToken(&tmp, T_RPAREN)){
		*ast = initUNode(A_VLA, NULL);
		*tok = tmp;
		return 1;
	}

	tl = tm = tr = NULL;
	tmp = *tok;
	if(scanToken(&tmp, T_LBRACE)){
		typeQualifierList(&tmp, &tm);
		if(scanToken(&tmp, T_STAR) && scanToken(&tmp, T_RBRACE)){
			//TODO less jank
			tl = initNode(A_DIRECTDECLARATOR, tm, NULL);
			*tok = tmp;
			return 1;
		}	
		freeAST(tm);
	}

	tl = tm = tr = NULL;
	tmp = *tok;
	if(scanToken(&tmp, T_LBRACE)){
		typeQualifierList(&tmp, &tm);
		assignmentExpr(&tmp, &tr);
		if(scanToken(&tmp, T_RBRACE)){
			tl = initNode(A_DIRECTDECLARATOR, tl, initNode(A_DIRECTDECLARATOR, tm, tr));
			*tok = tmp;
			return 1;
		}
		freeAST(tm);
		freeAST(tr);
	}

	tmp = *tok;
	if(scanToken(&tmp, T_LBRACE)){
		int opt = scanToken(&tmp, T_STATIC);
		if(typeQualifierList(&tmp, &tm)){
			if((opt || scanToken(&tmp, T_STATIC)) && assignmentExpr(&tmp, &tr)){
				tl = initNode(A_DIRECTDECLARATOR, tl, initNode(A_DIRECTDECLARATOR, tm, tr));
				*tok = tmp;
				return 1;
			}
			freeAST(tm);
		}else if(opt && assignmentExpr(&tmp, &tr)){
			tl = initNode(A_DIRECTDECLARATOR, tl, tr);
			*tok = tmp;
			return 1;
		}
	}
	
	return Capsule(paramTypeList, PAREN, tok, ast) || Capsule(identifierList, PAREN, tok, ast);
}

// iso c99 114
static int
directDeclarator(Token **tok, AST **ast)
{
	AST *tl = NULL, *tr = NULL;

	if(identifier(tok, &tl) || Capsule(declarator, PAREN, tok, &tl)){
		Sequence(directSequence, A_DIRECTDECLARATOR, tok, &tr);
		*ast = initNode(A_DIRECTDECLARATOR, tl, tr);
		return 1;
	}
	
	return 0;
}

// iso c99 114
static int
declarator(Token **tok, AST **ast)
{
	AST *tl = NULL, *tr;
	Token *tmp = *tok;
	pointer(&tmp, &tl);

	if(directDeclarator(&tmp, &tr)){
		*ast = tl ? initNode(A_DECLARATOR, tl, tr) : tr;
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 125
static int
designator(Token **tok, AST **ast)
{
	AST *tl;
	Token *tmp = *tok;

	//constexpr
	if(Capsule(elvisExpr, BRACE, &tmp, &tl) || (scanToken(&tmp, T_DOT) && identifier(&tmp, &tl))){
		*ast = initUNode(A_DESIGNATOR, tl);
		*tok = tmp;
		return 1;
	}
	
	return 0;
}

// iso c99 125
static int
designatorList(Token **tok, AST **ast)
{
	return List(designator, A_DESIGNATORLIST, tok, ast);
}

// iso c99 125
static int
designation(Token **tok, AST **ast)
{
	AST *tl = NULL;
	Token *tmp = *tok;

	if(designatorList(&tmp, &tl)){
		if(scanToken(&tmp, T_EQ)){
			*ast = tl;
			*tok = tmp;
			return 1;
		}
		freeAST(tl);
	}

	return 0;
}

static int initializer(Token **tok, AST **ast);

static int
initializerEntry(Token **tok, AST **ast)
{
	AST *tl = NULL, *tr;
	Token *tmp = *tok;
	designation(&tmp, &tl);

	if(initializer(&tmp, &tr)){
		*ast = tl ? initNode(A_INITIALIZER, tl, tr) : tr;
		*tok = tmp;
		return 1;
	}

	freeAST(tl);

	return 0;
}

// iso c99 125
static int
initializerList(Token **tok, AST **ast)
{
	return List(initializerEntry, A_INITLIST, tok, ast);
}

// iso c99 125
static int
initializer(Token **tok, AST **ast)
{
	return assignmentExpr(tok, ast) || CapsuleWComma(initializerList, CURLY, tok, ast);
}

// iso c99 97
static int
initDeclarator(Token **tok, AST **ast)
{
	AST *tl, *tr;
	Token *tmp = *tok;
	if(declarator(&tmp, &tl)){
		Token *ttmp = tmp;
		if(scanToken(&ttmp, T_EQ) && initializer(&ttmp, &tr)){
			*ast = initNode(A_INITIALIZER, tl, tr);
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
initDeclaratorList(Token **tok, AST **ast)
{
	return List(initDeclarator, A_INITLIST, tok, ast);
}

// iso c99 98
static int
storageClassSpecifier(Token **tok, AST **ast)
{
	Token *tmp = *tok;
	
	switch(tmp->token){
		case T_TYPEDEF:
		case T_EXTERN:
		case T_STATIC:
		case T_AUTO:
		case T_REGISTER:
			//TODO not null
			*ast = initUNode(A_STORAGESPEC, NULL);
			//	.intValue = tmp->token
			tmp = tmp->next;
			*tok = tmp;
			return 1;
	}

	return 0;
}

static int typeSpecifier(Token **, AST **);
static int typeQualifier(Token **, AST **);

static int
specifierQualifierEntry(Token **tok, AST **ast){
	return typeSpecifier(tok, ast) || typeQualifier(tok, ast);
}

// iso c99 101
static int
specifierQualifierList(Token **tok, AST **ast)
{
	return Sequence(specifierQualifierEntry, A_SPECQUALLIST, tok, ast);
}

// iso c99 114
static int
qualifierList(Token **tok, AST **ast)
{
	return Sequence(typeQualifier, A_QUALLIST, tok, ast);
}

// iso c99 101
static int
structDeclarator(Token **tok, AST **ast)
{
	AST *tl = NULL, *tr;
	Token *tmp = *tok;
	declarator(&tmp, &tl);

	Token *ttmp = tmp;
	//constexpr
	if(scanToken(&ttmp, T_COLON) && elvisExpr(&ttmp, &tr)){
		*ast = initNode(A_STRUCTDECLR, tl, tr);
		*tok = ttmp;
		return 1;
	}

	if(tl){
		*ast = tl;
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 101
static int
structDeclaratorList(Token **tok, AST **ast)
{
	return List(structDeclarator, A_STRUCTDECLLIST, tok, ast);
}

// iso c99 101
static int
structDeclaration(Token **tok, AST **ast)
{
	AST *tl, *tr;
	Token *tmp = *tok;

	if(specifierQualifierList(&tmp, &tl)){
		if(structDeclaratorList(&tmp, &tr)){
			if(scanToken(&tmp, T_SEMI)){
				*ast = initNode(A_STRUCTDECL, tl, tr);
				*tok = tmp;
				return 1;	
			}
			freeAST(tr);
		}
		freeAST(tl);
	}
	return 0;
}

// iso c99 101
static int
structDeclarationList(Token **tok, AST **ast){
	return Sequence(structDeclaration, A_STRUCTDECLLIST, tok, ast);
}

// iso c99 101
static int
structOrUnionSpecifier(Token **tok, AST **ast)
{
	AST *tl = NULL, *tr;
	Token *tmp = *tok;
	int type = -1;

	switch(tmp->token){
		case T_STRUCT: type = A_STRUCT; break;
		case T_UNION: type = A_UNION; break;
	}
	tmp = tmp->next;

	if(type >= 0){
		identifier(&tmp, &tl);
		if(Capsule(structDeclarationList, CURLY, &tmp, &tr)){
			*ast = initNode(type, tl, tr);
			*tok = tmp;
			return 1;
		}

		if(tl){
			*ast = initUNode(type, tl);
			*tok = tmp;
			return 1;
		}
	}
	return 0;
}

// iso c99 105
static int
enumerator(Token **tok, AST **ast)
{
	AST *tl, *tr;
	Token *tmp = *tok;

	//enumconst
	if(identifier(&tmp, &tl)){
		Token *ttmp = tmp;
		//constexpr
		if(scanToken(&ttmp, T_EQ) && elvisExpr(&ttmp, &tr)){
			*ast = initNode(A_ENUMERATOR, tl, tr);
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
enumeratorList(Token **tok, AST **ast)
{
	return List(enumerator, A_ENUMLIST, tok, ast);
}

// iso c99 104
static int
enumSpecifier(Token **tok, AST **ast)
{
	AST *tl = NULL, *tr;
	Token *tmp = *tok;

	if(scanToken(&tmp, T_ENUM)){
		identifier(&tmp, &tl);
		if(CapsuleWComma(enumeratorList, CURLY, &tmp, &tr)){
			*ast = initNode(A_ENUMSPEC, tl, tr);
			*tok = tmp;
			return 1;
		}

		if(tl){
			*ast = initUNode(A_ENUMSPEC, tl);
			*tok = tmp;
			return 1;
		}

	}
	
	return 0;
}

// iso c99 99
static int
typedefName(Token **tok, AST **ast){
	//TODO verify name
	
	return 0;
	return identifier(tok, ast);
}

// iso c99 99
static int
typeSpecifier(Token **tok, AST **ast)
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
			//TODO not null
			*ast = initUNode(A_TYPESPEC, NULL);
			//	.intValue = tmp->token
			tmp = tmp->next;
			*tok = tmp;	
			return 1;
	}

	return structOrUnionSpecifier(tok, ast) || enumSpecifier(tok, ast) || typedefName(tok, ast);
}

// iso c99 97
static int
typeQualifier(Token **tok, AST **ast)
{
	Token *tmp = *tok;

	switch(tmp->token){
		case T_CONST:
		case T_RESTRICT:
		case T_VOLATILE:
			//TODO not null
			*ast = initUNode(A_TYPEQUAL, NULL);
			//	.intValue = tmp->token
			tmp = tmp->next;
			*tok = tmp;
			return 1;	
	}

	return 0;
}

// iso c99 97
static int
functionSpecifier(Token **tok, AST **ast)
{
	Token *tmp = *tok;
	
	if(scanToken(&tmp, T_INLINE)){
		//TODO not null
		*ast = initUNode(A_FUNCSPEC, NULL);
		//	.intValue = (*tok)->token
		*tok = tmp;
		return 1;
	}
	
	return 0;
}

static int
declarationSpecifier(Token **tok, AST **ast)
{
	return storageClassSpecifier(tok, ast) || typeSpecifier(tok, ast) || typeQualifier(tok, ast) || functionSpecifier(tok, ast);
}

// iso c99 97
static int
declarationSpecifiers(Token **tok, AST **ast)
{
	return Sequence(declarationSpecifier, A_DECLSPECLIST, tok, ast);
}

// iso c99 97
static int
declaration(Token **tok, AST **ast)
{
	AST *tl, *tr;
	Token *tmp = *tok;

	if(declarationSpecifiers(&tmp, &tl)){
		int res = initDeclaratorList(&tmp, &tr);
		if(scanToken(&tmp, T_SEMI)){
			*ast = initNode(A_DECLARATION, res ? tr : NULL, tl);
			*tok = tmp;
			return 1;
		}
		if(res)
			freeAST(tr);
		freeAST(tl);
	}
	return 0;

}

static int statement(Token **, AST **);

// iso c99 136
static int
jumpStatement(Token **tok, AST **ast)
{
	static const int LRGOTO[] = {T_GOTO, T_SEMI}, LRRETURN[] = {T_RETURN, T_SEMI};
	AST *tl;
	Token *tmp = *tok;

	if(Capsule(identifier, LRGOTO, &tmp, &tl)){
		*ast = initUNode(A_GOTO, tl);
		*tok = tmp;
		return 1;
	}

	if(Capsule(expr, LRRETURN, &tmp, &tl)){
		*ast = initUNode(A_RETURN, tl);
		*tok = tmp;
		return 1;
	}

	if(scanToken(&tmp, T_CONTINUE) && scanToken(&tmp, T_SEMI)){
		*ast = initUNode(A_CONTINUE, NULL);
		*tok = tmp;
		return 1;
	}

	tmp = *tok;
	if(scanToken(&tmp, T_BREAK) && scanToken(&tmp, T_SEMI)){
		*ast = initUNode(A_BREAK, NULL);
		*tok = tmp;
		return 1;
	}

	return 0;
}

static int
forList(Token **tok, AST **ast){
	AST *tl = NULL, *tm = NULL, *tr = NULL;
	Token *tmp = *tok;

	expr(&tmp, &tl);
	if(scanToken(&tmp, T_SEMI)){
		expr(&tmp, &tm);
		if(scanToken(&tmp, T_SEMI)){
			expr(&tmp, &tr);
			*ast = allocAST(3);
			**ast = (AST){
				.ASTtype = A_FORLIST,
				.flags = AF_NODE,
				.type = NULL,
				.length = 3
			};

			(*ast)->data[0] = tl;
			(*ast)->data[1] = tm;
			(*ast)->data[2] = tr;
			*tok = tmp;
			return 1;
		}
		freeAST(tm);
	}
	freeAST(tl);

	tl = tm = tr = NULL;
	tmp = *tok;
	if(declaration(&tmp, &tl)){
		expr(&tmp, &tm);
		if(scanToken(&tmp, T_SEMI)){
			expr(&tmp, &tr);
			*ast = allocAST(3);
			**ast = (AST){
				.type = NULL,
				.ASTtype = A_FORLIST,
				.flags = AF_NODE,
				.length = 3
			};

			(*ast)->data[0] = tl;
			(*ast)->data[1] = tm;
			(*ast)->data[2] = tr;
			*tok = tmp;
			return 1;
		}
		freeAST(tm);
		freeAST(tl);
	}

	return 0;
}

// iso c99 135
static int
iterationStatement(Token **tok, AST **ast)
{
	AST *tl, *tr;
	Token *tmp = *tok;
	if(scanToken(&tmp, T_WHILE) && Capsule(expr, PAREN, &tmp, &tl)){
		if(statement(&tmp, &tr)){
			*ast = initNode(A_WHILE, tl, tr);
			*tok = tmp;
			return 1;
		}
		freeAST(tl);
	}

	tmp = *tok;
	if(scanToken(&tmp, T_DO) && statement(&tmp, &tl)){
		if(scanToken(&tmp, T_WHILE) && Capsule(expr, PAREN, &tmp, &tr)){
			if(scanToken(&tmp, T_SEMI)){
				*ast = initNode(A_DOWHILE, tr, tl);
				*tok = tmp;
				return 1;
			}
			freeAST(tr);
		}
		freeAST(tl);
	}

	tmp = *tok;
	if(scanToken(&tmp, T_FOR) && Capsule(forList, PAREN, &tmp, &tl)){
		if(statement(&tmp, &tr)){
			*ast = initNode(A_FOR, tl, tr);
			*tok = tmp;
			return 1;
		}
		freeAST(tl);
	}

	return 0;
}

// iso c99 133
static int
selectionStatement(Token **tok, AST **ast)
{
	AST *tl, *tm, *tr;
	Token *tmp = *tok;

	if(scanToken(&tmp, T_IF) && Capsule(expr, PAREN, &tmp, &tl)){
		if(statement(&tmp, &tm)){
			*ast = initNode(A_IF, tl, tm);
			Token *ttmp = tmp;
			if(scanToken(&ttmp, T_ELSE) && statement(&ttmp, &tr)){
				*ast = initNode(A_IFELSE, *ast, tr);
				*tok = ttmp;
				return 1;
			}

			
			*tok = tmp;
			return 1;
		}
		freeAST(tl);
	}

	tmp = *tok;
	if(scanToken(&tmp, T_SWITCH) && Capsule(expr, PAREN, &tmp, &tl)){
		if(statement(&tmp, &tr)){
			*ast = initNode(A_SWITCH, tl, tr);
			*tok = tmp;
			return 1;
		}
		freeAST(tl);
	}

	return 0;
}

// iso c99 132
static int
exprStatement(Token **tok, AST **ast)
{
	AST *tl = NULL;
	Token *tmp = *tok;
	expr(&tmp, &tl);

	if(scanToken(&tmp, T_SEMI)){
		*ast = tl ? tl : initUNode(A_EMPTYSTATEMENT, NULL);
		*tok = tmp;
		return 1;
	}

	freeAST(tl);

	return 0;
}

// iso c99 132
static int
blockItem(Token **tok, AST **ast)
{
	return declaration(tok, ast) || statement(tok, ast);
}

// iso c99 132
static int
blockItemList(Token **tok, AST **ast)
{
	return Sequence(blockItem, A_BLOCKLIST, tok, ast);
}

// iso c99 132
static int
compoundStatement(Token **tok, AST **ast)
{
	Token *tmp = *tok;

	if(scanToken(&tmp, T_LCURLY) && scanToken(&tmp, T_RCURLY)){
		*ast = initUNode(A_BLOCK, NULL);
		*tok = tmp;
		return 1;
	}

	return Capsule(blockItemList, CURLY, tok, ast);
}

// iso c99 131
static int
labelStatement(Token **tok, AST **ast)
{
	static const int LRCASE[] = {T_CASE, T_COLON};
	Token *tmp = *tok;
	AST *tl, *tr;

	if(identifier(&tmp, &tl)){
		if(scanToken(&tmp, T_COLON) && statement(&tmp, &tr)){
			*ast = initNode(A_LABELSTATEMENT, tl, tr);
			*tok = tmp;
			return 1;
		}
		freeAST(tl);
	}

	tmp = *tok;
	if(Capsule(elvisExpr, LRCASE, &tmp, &tl)){
		if(statement(&tmp, &tr)){
			*ast = initNode(A_LABELSTATEMENT, tl, tr);
			*tok = tmp;
			return 1;
		
		}
		freeAST(tl);
	}


	tmp = *tok;
	if(scanToken(&tmp, T_DEFAULT) && scanToken(&tmp, T_COLON) && statement(&tmp, &tr)){
		*ast = initNode(A_LABELSTATEMENT, NULL, tr);
		*tok = tmp;
		return 1;
	}

	return 0;
}

// iso c99 131
static int
statement(Token **tok, AST **ast)
{
	return labelStatement(tok, ast) || iterationStatement(tok, ast) || exprStatement(tok, ast) || selectionStatement(tok, ast) || jumpStatement(tok, ast) || compoundStatement(tok, ast);
}

// iso c99 141
static int
declarationList(Token **tok, AST **ast)
{
	return Sequence(declaration, A_DECLARATIONLIST, tok, ast);
}

// iso c99 141
static int
functionDeclaration(Token **tok, AST **ast)
{
	AST *tl, *tml, *tmr = NULL, *tr;
	Token *tmp = *tok;

	if(declarationSpecifiers(&tmp, &tl)){
		if(declarator(&tmp, &tml)){
			declarationList(&tmp, &tmr);
			if(compoundStatement(&tmp, &tr)){
				*ast = allocAST(4);
				**ast = (AST){
					.type = NULL,
					.ASTtype = A_FUNCDECL,
					.flags = AF_NODE,
					.length = 4
				};

				(*ast)->data[0] = tl;
				(*ast)->data[1] = tml;
				(*ast)->data[2] = tmr;
				(*ast)->data[3] = tr;
				*tok = tmp;
				return 1;
			}
			freeAST(tmr);
			freeAST(tml);
		}
		freeAST(tl);
	}

	return 0;
}

// iso c99 140
static int
externalDeclaration(Token **tok, AST **ast)
{
	return functionDeclaration(tok, ast) || declaration(tok, ast);
}

// iso c99 140
static int
translationUnit(Token **tok, AST **ast)
{
	return Sequence(externalDeclaration, A_TRANSLATIONUNIT, tok, ast);
}

AST*
genAST(Token *tok)
{
	AST *lt;
	Token *tmp = tok;
	if(translationUnit(&tmp, &lt)){
		if(tmp->token == T_EOF){
			return lt;
		}
		freeAST(lt);
	}

	printf("could not generate ast\n");
	lt = allocAST(0);
	*lt = (AST){
		.ASTtype = A_TRANSLATIONUNIT,
		.flags = AF_NODE,
		.length = 0
	};

	return lt;
}
