#include<stdio.h>
#include<stdlib.h>
#include"pp.h"

/* ppast stuff */

AST *initNode(int, AST *, AST *);
AST *initUNode(int, AST *);
AST *initStrNode(int, char *, char *);
int scanToken(Token **, int);
int elvisExpr(Token **, AST **);

int scanString(char **, const char *);

int Sequence(int (*)(Token **, AST **), int, Token **, AST **);
int identifier(Token **, AST **);

static int
scanAST(int (*of)(Token **, AST **), AST **ast, char *s, char *e)
{
	//TODO make this thread safe	
	char temp = *e;
	*e = 0;
	Token *t = genTokens(s), *tt = t;
	*e = temp;
	AST *tl;
	if(of(&tt, &tl)){
		if(tt->token == T_EOF){
			freeToken(t);
			*ast = tl;
			return 1;
		}
		freeAST(tl);
	}
	freeToken(t);
	return 0;
}

// iso c99 146
static int
replacementList(Token **tok, AST **ast)
{
	//TODO
	return 0;
}

// iso c99 146
static int
textLine(Token **tok, AST **ast)
{
	Token *tmp = *tok, *ttmp;
	if(!scanToken(&tmp, T_POUND)){
		while(tmp && tmp->token != T_WS)
			tmp = tmp->next;

		ttmp = tmp;
		if(scanToken(&ttmp, T_WS)){
			char *s = (*tok)->start, *e = tmp->start;
			*tok = ttmp;
			*ast = initStrNode(PPA_PPTEXTLINE, s, e);
			return 1;
		}
	}
	return 0;
}

// iso c99 146
static int
controlLine(Token **tok, AST **ast)
{
	AST *tl, *tm, *tr;
	Token *tmp = *tok, *ttmp;
	if(scanToken(&tmp, T_POUND)){
		char *s = tmp->start;
		if(scanString(&s, "include") && s == tmp->end){
			tmp = tmp->next;
			ttmp = tmp;

			if(scanToken(&ttmp, T_STRINGLIT)){
				if(scanToken(&ttmp, T_WS)){
					*ast = initStrNode(PPA_PPINCLUDE, tmp->start, tmp->end);
					*tok = ttmp;
					return 1;	
				}
			}

			ttmp = tmp;
			char *s = ttmp->start;
			if(scanToken(&ttmp, T_LT)){
				while(ttmp && ttmp->token != T_GT){
					ttmp = ttmp->next;
				}

				char *e = ttmp->end;
				if(scanToken(&ttmp, T_GT) && scanToken(&ttmp, T_WS)){
					*ast = initStrNode(PPA_PPINCLUDE, s, e);
					*tok = ttmp;
					return 1;
				}
			}	
		}
		
		s = tmp->start;
		if(scanString(&s, "define") && s == tmp->end){
			ttmp = tmp->next;
			char *s = ttmp->start, *e = ttmp->end;
			if(scanToken(&ttmp, T_IDENTIFIER)){
				char *q = ttmp->end;
				if(scanToken(&ttmp, T_LPAREN) && ttmp->start == q){
					//TODO
				}
			}
		}
		
		s = tmp->start;
		if(scanString(&s, "undef") && s == tmp->end){
			ttmp = tmp->next;
			if(identifier(&ttmp, &tl)){
				if(scanToken(&ttmp, T_WS)){
					*ast = initUNode(PPA_PPUNDEF, tl);
					*tok = ttmp;
					return 1;
				}
				freeAST(tl);
			}

		}
		
		s = tmp->start;
		if(scanString(&s, "line") && s == tmp->end){
			ttmp = tmp->next;
		}
		
		s = tmp->start;
		if(scanString(&s, "error") && s == tmp->end){
			char *s = tmp->end;
			ttmp = tmp->next;
			if(textLine(&ttmp, &tl)){
				tl->ASTtype = PPA_PPERROR;
				*ast = tl;
				*tok = ttmp;
				return 1;
			}
		}
		
		s = tmp->start;
		if(scanString(&s, "pragma") && s == tmp->end){
			ttmp = tmp->next;	
		}

		if(scanToken(&tmp, T_WS)){
			*tok = tmp;
			*ast = NULL;
			return 1;
		}
	}

	return 0;
}

// iso c99 145
static int
endifLine(Token **tok, AST **ast)
{
	Token *tmp = *tok;
	if(scanToken(&tmp, T_POUND)){
		char *s = tmp->start;
		if(scanString(&s, "endif") && s == tmp->end){
			tmp = tmp->next;
			if(scanToken(&tmp, T_WS)){
				*tok = tmp;
				return 1;
			}
		}
	}
	return 0;
}

static int group(Token **tok, AST **ast);

// iso c99 145
static int
elseGroup(Token **tok, AST **ast)
{
	AST *tl = NULL;
	Token *tmp = *tok;
	if(scanToken(&tmp, T_POUND)){
		char *s = tmp->start;
		if(scanString(&s, "else") && s == tmp->end){
			tmp = tmp->next;
			if(scanToken(&tmp, T_WS)){
				group(&tmp, &tl);
				*ast = initUNode(PPA_PPELSEGROUP, tl);
				*tok = tmp;
				return 1;
			}
		}
	}
	return 0;
}

// iso c99 145
static int
elifGroup(Token **tok, AST **ast)
{
	AST *tl, *tr = NULL; 
	Token *tmp = *tok;
	if(scanToken(&tmp, T_POUND)){
		char *s = tmp->start;
		if(scanString(&s, "elif") && s == tmp->end){
			tmp = tmp->next;
			
			s = tmp->start;
			while(tmp && tmp->token != T_WS)
				tmp = tmp->next;
			char *e = tmp->start;

			if(scanAST(elvisExpr, &tl, s, e)){
				if(scanToken(&tmp, T_WS)){
					group(&tmp, &tr);
					*ast = initNode(PPA_PPELIFGROUP, tl, tr);
					*tok = tmp;
					return 1;
				}
				freeAST(tl);
			}
		}
	}

	return 0;
}

// iso c99 145
static int
elifGroups(Token **tok, AST **ast)
{
	return Sequence(elifGroup, PPA_PPELIFGROUPS, tok, ast);
}

// iso c99 145
static int
ifGroup(Token **tok, AST **ast)
{
	AST *tl, *tr = NULL;
	Token *tmp = *tok, *ttmp;
	if(scanToken(&tmp, T_POUND)){
		if(tmp->token == T_IDENTIFIER){
			char *s = tmp->start;
			if(scanString(&s, "if") && s == tmp->end){
				ttmp = tmp->next;

				/* this is really gross but will work for now :)*/
				while(ttmp && ttmp->token != T_WS)
					ttmp = ttmp->next;
				char *e = ttmp->start;

				//TODO defined unary operator
				if(scanAST(elvisExpr, &tl, s, e)){
					if(scanToken(&ttmp, T_WS)){
						group(&ttmp, &tr);
						printAST(tr);
						*ast = initNode(PPA_PPIFGROUP, tl, tr);
						*tok = ttmp;
						return 1;
					}
					freeAST(tl);
				}
			}

			s = tmp->start;
			if(scanString(&s, "ifndef") && s == tmp->end){
				ttmp = tmp->next;
				if(identifier(&ttmp, &tl)){
					if(scanToken(&ttmp, T_WS)){
						group(&ttmp, &tr);
						*ast = initNode(PPA_PPIFNDEFGROUP, tl, tr);
						*tok = ttmp;
						return 1;
					}
					freeAST(tl);
				}
			}

			s = tmp->start;
			if(scanString(&s, "ifdef") && s == tmp->end){
				ttmp = tmp->next;
				if(identifier(&ttmp, &tl)){
					if(scanToken(&ttmp, T_WS)){
						group(&ttmp, &tr);
						*ast = initNode(PPA_PPIFDEFGROUP, tl, tr);
						*tok = ttmp;
						return 1;
					}
					freeAST(tl);
				}
			}
		}
	}

	return 0;
}

// iso c99 145
static int
ifSection(Token **tok, AST **ast)
{
	AST *tl, *tml = NULL, *tmr = NULL, *tr;
	Token *tmp = *tok;
	if(ifGroup(&tmp, &tl)){
		elifGroups(&tmp, &tml);
		elseGroup(&tmp, &tmr);
		if(endifLine(&tmp, &tr)){
			*ast = allocAST(3);
			**ast = (AST){
				.ASTtype = PPA_PPIFSECTION,
				.flags = AF_NODE,
				.length = 3
			};

			(*ast)->data[0] = tl;
			(*ast)->data[1] = tml;
			(*ast)->data[2] = tmr;
			freeAST(tr);
			*tok = tmp;
			return 1;
		}
		freeAST(tl);
		freeAST(tml);
		freeAST(tmr);
	}
	return 0;
}

// iso c99 145
static int
nonDirective(Token **tok, AST **ast)
{
	static const char *direc[] = {"include", "define", "if", "ifdef", "ifndef", "error", "pragma", "undef", "elif", "else", "endif", NULL};

	Token *tmp = *tok;
	char *s = tmp->start, *e = tmp->end;
	if(scanToken(&tmp, T_IDENTIFIER)){
		const char **cur = direc;
		while(*cur){
			if(scanString(&s, *cur))
				return 0;
			cur++;
		}

	}
	
	if(textLine(tok, ast)){
		(*ast)->ASTtype = PPA_PPNONDIRECTIVE;
		return 1;
	}

	return 0;
}

// iso c99 145
static int
groupPart(Token **tok, AST **ast)
{
	AST *lt;
	Token *tmp = *tok;
	
	if(ifSection(tok, ast) || controlLine(tok, ast) || textLine(tok, ast))
		return 1;

	if(scanToken(&tmp, T_POUND) && nonDirective(&tmp, &lt)){
		*ast = lt;	
		*tok = tmp;
		return 1;
	}
	return 0;

}

// iso c99 145
static int
group(Token **tok, AST **ast)
{
	return Sequence(groupPart, PPA_PPGROUP, tok, ast);
}

// iso c99 145
static int
preprocessingFile(Token **tok, AST **ast)
{
	return Sequence(group, PPA_PPFILE, tok, ast);
}

AST*
genPPAST(Token *tok)
{
	AST *lt;
	Token *tmp = tok;
	if(preprocessingFile(&tmp, &lt)){
		//if(tmp->token == PPT_PPEXTRA)
			return lt;
		//printAST(lt);
		//freeAST(lt);
	}

	printf("could not generate ppast\n");
	lt = allocAST(0);
	*lt = (AST){
		.ASTtype = PPA_PPFILE,
		.flags = AF_NODE,
		.length = 0	
	};

	return lt;
}

