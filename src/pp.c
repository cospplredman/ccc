#include<stdio.h>
#include<stdlib.h>
#include"pp.h"

#define ISDIGIT(a) ((a) >= '0' && (a) <= '9')
#define ISOCT(a)   ((a) >= '0' && (a) <= '7')
#define ISHEX(a)   (((a) >= '0' && (a) <= '9') || (((a) | 32) >= 'a' && ((a) | 32) <= 'f'))
#define HEXVAL(b)  (ISDIGIT(b) ? ((b) - '0') : (((b) | 32) - 'a' + 10))
#define ISALPHA(a) (((a) | 32) >= 'a' && ((a) | 32) <= 'z')

int identifier(char **, Token **);
int charConst(char **, Token **);
int stringLit(char **, Token **);
int punctuator(char **, Token **);
Token *initToken(int, char *, char *);
int Sequence(int (*)(Token **, AST **), int, Token **, AST **);

/* util functions */

void
printPPToken(PPToken *)
{
	//TODO
}

PPToken*
allocPPToken()
{
	return allocToken();
}

void
freePPToken(PPToken *tok)
{
	freeToken(tok);
}

static PPToken *
initPPToken(int type, char *s, char *e)
{
	return initToken(type, s, e);
}

int scanChar(char **str, char ch);
int scanCharLower(char **str, char ch);

/* pptoken stuff */

// iso c99 64
static int
headerName(char **str, PPToken **tok)
{
	char *cur = *str;
	if(scanChar(&cur, '<')){
		while(*cur != '>'){
			if(*cur == '\n')
				return 0;
			cur++;
		}
	}else if(scanChar(&cur, '"')){
		while(*cur != '"'){
			if(*cur == '\n')
				return 0;
			cur++;
		}
	}else{
		return 0;
	}

	*tok = initPPToken(PPT_PPHEADER, *str, cur);
	*str = cur;
	return 1;
}

// iso c99 65
static int
ppnumber(char **str, PPToken **tok)
{
	char *cur = *str;

	if(ISDIGIT(*cur) || scanChar(&cur, '.') && ISDIGIT(*cur)){
		cur++;
		while(1){
			if(ISDIGIT(*cur) || ISALPHA(*cur)){
				cur++;
				continue;
			}

			if(scanChar(&cur, '_') || scanCharLower(&cur, 'e') || scanCharLower(&cur, 'p') || scanChar(&cur, '.'))
				continue;

			break;
		}

		*tok = initPPToken(PPT_PPNUMBER, *str, cur);
		*str = cur;
		return 1;
	}

	return 0;
}

// iso c99 49
static int
ppextra(char **str, PPToken **tok)
{
	if(*str){
		*tok = initPPToken(PPT_PPEXTRA, *str, *str + 1);
		*str = *str + 1;
		return 1;
	}

	return 0;
}

// iso c99 49
static int
pptoken(char **str, PPToken **tok)
{
	return identifier(str, tok) || ppnumber(str, tok) || charConst(str, tok) || stringLit(str, tok) || punctuator(str, tok) || headerName(str, tok);	
}

static int
pptokens(char **str, PPToken **tok)
{
	PPToken *tl = NULL;
	char *cur = *str;
	if(pptoken(&cur, &tl)){
		if(pptokens(&cur, &(tl->next)) || ppextra(&cur, &(tl->next))){
			*tok = tl;
			*str = cur;
			return 1;
		}
		tl->next = NULL;
		freeToken(tl);
	}

	return 0;
}

PPToken *genPPToken(char *)
{
	PPToken *tl;
	char *cur;	

	if(cur && pptokens(&cur, &tl))
		return tl;

	printf("could not parse pptokens\n");
	tl = initPPToken(PPT_PPEXTRA, NULL, NULL);
	tl->next = NULL;
	return tl;
}


/* ppast stuff */

AST *initNode(int, AST *, AST *);
AST *initUNode(int, AST *);
int scanToken(Token **, int);

void 
printPPAST(PPAST *)
{
	//TODO
}

PPAST*
allocPPAST(int size)
{
	return allocAST(size);
}

void
freePPAST(PPAST *ast)
{
	return freeAST(ast);
}

static PPAST*
initPPNode(int type, PPAST* l, PPAST* r)
{
	return initNode(type, l, r);
}

static PPAST*
initUPPNode(int type, PPAST* l)
{
	return initUNode(type, l);
}

static int
scanPPToken(PPToken **tok, int type)
{
	return scanToken(tok, type);
}

//TODO figure out what stays
// iso c99 146
static int
pptoken___(PPToken **tok, PPAST **ast)
{
	//TODO
	return 0;
}

// iso c99 146
static int
replacementList(PPToken **tok, PPAST **ast)
{
	//TODO
	return 0;
}

// iso c99 146
static int
lparen(PPToken **tok, PPAST **ast)
{
	//TODO
	return 0;
}

// iso c99 146
static int
textLine(PPToken **tok, PPAST **ast)
{
	//TODO
	return 0;
}

// iso c99 146
static int
controlLine(PPToken **tok, PPAST **ast)
{
	//TODO
	return 0;
}

// iso c99 145
static int
endifLine(PPToken **tok, PPAST **ast)
{
	//TODO
	return 0;
}

// iso c99 145
static int
elseGroup(PPToken **tok, PPAST **ast)
{
	//TODO
	return 0;
}

// iso c99 145
static int
elifGroup(PPToken **tok, PPAST **ast)
{
	//TODO
	return 0;
}

// iso c99 145
static int
elifGroups(PPToken **tok, PPAST **ast)
{
	return Sequence(elifGroup, PPA_PPELIFGROUP, tok, ast);
}

// iso c99 145
static int
ifGroup(PPToken **tok, PPAST **ast)
{
	//TODO
	return 0;
}

// iso c99 145
static int
ifSection(PPToken **tok, PPAST **ast)
{
	PPAST *tl, *tml = NULL, *tmr = NULL, *tr;
	PPToken *tmp = *tok;
	if(ifGroup(&tmp, &tl)){
		elifGroups(&tmp, &tml);
		elseGroup(&tmp, &tmr);
		if(endifLine(&tmp, &tr)){
			*ast = allocPPAST(4);
			**ast = (PPAST){
				.ASTtype = PPA_PPIFSECTION,
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
		freePPAST(tl);
		freePPAST(tml);
		freePPAST(tmr);
	}
	return 0;
}

// iso c99 145
static int
nonDirective(PPToken **tok, PPAST **ast)
{
	//TODO
	return 0;
}

// iso c99 145
static int
groupPart(PPToken **tok, PPAST **ast)
{
	PPAST *lt;
	PPToken *tmp = *tok;
	if(scanPPToken(&tmp, T_POUND) && nonDirective(&tmp, &lt)){
		*ast = lt;	
		*tok = tmp;
		return 1;
	}

	return ifSection(tok, ast) || controlLine(tok, ast) || textLine(tok, ast);
}

// iso c99 145
static int
group(PPToken **tok, PPAST **ast)
{
	return Sequence(groupPart, PPA_PPGROUP, tok, ast);
}

// iso c99 145
static int
preprocessingFile(PPToken **tok, PPAST **ast)
{
	if(Sequence(group, PPA_PPFILE, tok, ast))
		return 1;
	
	*ast = initUPPNode(PPA_PPFILE, NULL);
	return 1;
}

PPAST*
genPPAST(PPToken *tok)
{
	PPAST *lt;
	PPToken *tmp = tok;
	if(preprocessingFile(&tmp, &lt)){
		if(tmp->token == PPT_PPEXTRA)
			return lt;
		freeAST(lt);
	}

	printf("could not generate ppast\n");
	lt = allocPPAST(0);
	*lt = (PPAST){
		.ASTtype = PPA_PPFILE,
		.flags = AF_NODE,
		.length = 0	
	};

	return lt;
}

Token *ppttt(PPToken *);
