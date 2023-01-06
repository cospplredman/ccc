#include"token.h"
#include<stdio.h>
#include<stddef.h>
#include<stdlib.h>

#define ISDIGIT(a) ((a) >= '0' && (a) <= '9')
#define ISOCT(a)   ((a) >= '0' && (a) <= '7')
#define ISHEX(a)   (((a) >= '0' && (a) <= '9') || (((a) | 32) >= 'a' && ((a) | 32) <= 'f'))
#define HEXVAL(b)  (ISDIGIT(b) ? ((b) - '0') : (((b) | 32) - 'a' + 10))
#define ISALPHA(a) (((a) | 32) >= 'a' && ((a) | 32) <= 'z')

/* util functions */

static const char *tokenName[] = {
	"T_AUTO", "T_ENUM", "T_RESTRICT", "T_UNSIGNED", "T_BREAK", "T_EXTERN", "T_RETURN", "T_VOID", "T_CASE", "T_FLOAT", "T_SHORT", "T_VOLATILE", "T_CHAR", "T_FOR", "T_SIGNED", "T_WHILE", "T_CONST", "T_GOTO", "T_SIZEOF", "T_BOOL", "T_CONTINUE", "T_IF", "T_STATIC", "T_COMPLEX", "T_DEFAULT", "T_INLINE", "T_STRUCT", "T_IMAGINARY", "T_DO", "T_INT", "T_SWITCH", "T_DOUBLE", "T_LONG", "T_TYPEDEF", "T_ELSE", "T_REGISTER", "T_UNION",
	"T_IDENTIFIER",
	
	"T_INTEGERCONST",
	"T_FLOATCONST",
	"T_CHARCONST",

	"T_STRINGLIT",

	"T_LBRACE", "T_RBRACE", "T_LPAREN", "T_RPAREN", "T_LCURLY", "T_RCURLY", "T_DOT", "T_ARROW", "T_INC", "T_DEC", "T_AND", "T_STAR", "T_PLUS", "T_MINUS", "T_TILDE", "T_BANG", "T_SLASH", "T_PERCENT", "T_LSHIFT", "T_RSHIFT", "T_LT", "T_GT", "T_LTEQ", "T_GTEQ", "T_EQUAL", "T_NOTEQUAL", "T_XOR", "T_OR", "T_BAND", "T_BOR", "T_QMARK", "T_COLON", "T_SEMI", "T_ELIPSIS", "T_EQUALS", "T_EQ", "T_MULEQ", "T_DIVEQ", "T_MODEQ", "T_ADDEQ", "T_SUBEQ", "T_LSHIFTEQ", "T_RSHIFTEQ", "T_ANDEQ", "T_XOREQ", "T_OREQ", "T_COMMA", "T_POUND", "T_PPCONCAT",

	"T_PPHEADER",

	"T_WS",
	"T_PPEXTRA"
};

void
printToken(Token *tok)
{
	printf("%-15s: %lld %lf \"%.*s\"\n", tokenName[tok->token], tok->intValue, tok->floatValue, (int)(tok->end - tok->start), tok->start);
}

void
printTokens(Token *tok)
{
	printToken(tok);
	if(tok->next)
		printTokens(tok->next);
}

Token*
allocToken()
{
	return (Token*)malloc(sizeof(Token));
}

void
freeToken(Token *tok)
{
	if(tok->next)
		freeToken(tok->next);
	free(tok);
}

static int
scanChar(char **str, char ch)
{
	if(**str == ch){
		(*str)++;
		return 1;
	}

	return 0;
}

static int
scanCharLower(char **str, char ch)
{
	if((**str | 32) == ch){
		(*str)++;
		return 1;
	}

	return 0;
}

static void
skipIntSufix(char **str)
{
	char *cur = *str;
	int opt = scanCharLower(&cur, 'u');
	scanCharLower(&cur, 'l');
	scanCharLower(&cur, 'l');
	!opt && scanCharLower(&cur, 'u');
	*str = cur;
}

/* token parsing */

static int
decIntConst(char **str, Token *tok)
{
	long long v = 0;
	char *cur = *str;
	
	if(*cur < '1' || *cur > '9')
		return 0;

	do 
		v = v*10 + (*cur++ - '0');
	while(ISDIGIT(*cur));
	skipIntSufix(&cur);

	*tok = (Token){
		T_INTEGERCONST,
		*str, 
		cur,
		.intValue = v
	};

	*str = cur;
	return 1;
}

static int
octIntConst(char **str, Token *tok)
{
	long long v = 0;
	char *cur = *str;
	if(*cur != '0')
		return 0;

	do
		v = v*8 + (*cur++ - '0');
	while(ISOCT(*cur));
	skipIntSufix(&cur);
	
	*tok = (Token){
		T_INTEGERCONST,
		*str,
		cur,
		.intValue = v
	};

	*str = cur;
	return 1;
}

static int
hexIntConst(char **str, Token *tok)
{
	long long v = 0;
	char *cur = *str;
	if(*cur != '0' || (cur[1] | 32) != 'x' || !ISHEX(cur[2]))
		return 0;

	cur += 2;
	do{
		v = v*16 + HEXVAL(*cur);
		cur++;
	}while(ISHEX(*cur));
	skipIntSufix(&cur);

	*tok = (Token){
		T_INTEGERCONST,
		*str,
		cur,
		.intValue = v
	};

	*str = cur;
	return 1;
}

// iso c99 54
static int
intConst(char **str, Token *tok)
{
	return decIntConst(str, tok) || hexIntConst(str, tok) || octIntConst(str, tok);
}

static int
hexDigitSeq(char **str)
{
	char *cur = *str;
	if(!ISHEX(*cur))
		return 0;

	do
		cur++;
	while(ISHEX(*cur));

	*str = cur;
	return 1;
}

static int
digitSeq(char **str)
{
	char *cur = *str;
	if(!ISDIGIT(*cur))
		return 0;

	do 
		cur++;
	while(ISDIGIT(*cur));

	*str = cur;
	return 1;
}

// iso c99 57
static int
floatConst(char **str, Token *tok)
{
	char *cur = *str;
	if(scanChar(&cur, '0') && scanCharLower(&cur, 'x')){
		int opt = hexDigitSeq(&cur);
		if(scanChar(&cur, '.')){
			int opt1 = hexDigitSeq(&cur);
			if(opt || opt1){
				char *ccur = cur;
				if(scanCharLower(&ccur, 'p')){
					scanChar(&ccur, '+') || scanChar(&ccur, '-');
					if(digitSeq(&ccur))
						cur = ccur;
				}
				goto end;
			}
		}
	}

	cur = *str;
	int opt = digitSeq(&cur);
	if(scanChar(&cur, '.')){
		int opt1 = digitSeq(&cur);
		if(opt || opt1){
			char *ccur = cur;
			if(scanCharLower(&ccur, 'e')){
				scanChar(&ccur, '+') || scanChar(&ccur, '-');
				if(digitSeq(&ccur))
					cur = ccur;
			}
			goto end;
		}
	}

	return 0;

end:
	scanCharLower(&cur, 'l') || scanCharLower(&cur, 'f');

	*tok = (Token){
		T_FLOATCONST,
		*str,
		cur,
		//TODO do it myself >:3
		.floatValue = strtof(*str, NULL)
	};

	*str = cur;
	return 1;
}

// iso c99 53
static int
universalCharacterName(char **str, Token *tok){
	char *cur = *str;
	int j, i;
	if(scanChar(&cur, '\\') && scanCharLower(&cur, 'u')){
		tok->intValue = 0;
		for(j = 0; j < 2; j++){
			for(i = 0; i < 4; i++){
				if(!ISHEX(*cur))
					goto ret;
	
				tok->intValue = (tok->intValue << 4) + HEXVAL(*cur);
				cur++;
			}
		}
	}

	return 0;
ret:
	if(j == 0)
		return 0;

	if(j == 1){
		tok->intValue >>= i*4;
		cur -= i;
	}

	*tok = (Token){
		T_PPEXTRA, //B/C it shouldn't be a token
		*str,
		cur,
		.intValue = tok->intValue //TODO find a use for this?
	};
	
	*str = cur;
	return 1;
}

// iso c99 60
static int
escapeSequence(char **str, Token *tok)
{
	char *cur = *str;

	if(universalCharacterName(str, tok))
		return 1;

	if(!scanChar(&cur, '\\'))
		return 0;

	switch((*cur)++){
		case '\\': tok->intValue = '\\'; break;
		case 'a': tok->intValue = '\a'; break;
		case 'b': tok->intValue = '\b'; break;
		case 'f': tok->intValue = '\f'; break;
		case 'n': tok->intValue = '\n'; break;
		case 'r': tok->intValue = '\r'; break;
		case 't': tok->intValue = '\t'; break;
		case 'v': tok->intValue = '\v'; break;
		case '\'': tok->intValue = '\''; break;
		case '\"': tok->intValue = '\"'; break; 
		case '?': tok->intValue = '\?'; break;
		case 'x': tok->intValue = 0;
			if(!ISHEX(*cur))
				return 0;

			do{
				tok->intValue = tok->intValue * 16 + HEXVAL(*cur);
				cur++;
			}while(ISHEX(*cur));
		break;
		default:
			cur--;
			if(ISOCT(*cur)){
				tok->intValue = *cur - '0';
				cur++;

				if(ISOCT(*cur)){
					tok->intValue = tok->intValue * 8 + *cur - '0';
					cur++;
				}

				if(ISOCT(*cur)){
					tok->intValue = tok->intValue * 8 + *cur - '0';
					cur++;
				}
			}else{
				return 0;
			}
	}

	*tok = (Token){
		T_PPEXTRA, //b/c shouldn't be a token
		*str,
		cur,
		.intValue = tok->intValue //TODO use to generate actual strings
	};

	*str = cur;
	return 1;
	
}

// iso c99 59
static int
charConst(char **str, Token *tok)
{
	char *cur = *str;
	//TODO wide strings?
	scanChar(&cur, 'L');

	if(scanChar(&cur, '\'')){
		if(!escapeSequence(&cur, tok)){
			if(*cur == '\'' || *cur == '\\' || *cur == '\n')
				return 0;
			tok->intValue = *cur;
			cur++;
		}

		if(scanChar(&cur, '\'')){
			*tok = (Token){
				T_CHARCONST,
				*str,
				cur,
				.intValue = tok->intValue
			};

			*str = cur;
			return 1;
		}
	}

	return 0;
}

//iso c99 54
static int
constant(char **str, Token *tok)
{
	return floatConst(str, tok) || intConst(str, tok) || charConst(str, tok);
}

// iso c99 62
static int
stringLit(char **str, Token *tok)
{
	char *cur = *str;
	scanChar(&cur, 'L');

	if(scanChar(&cur, '"')){
		while(!scanChar(&cur, '"')){
			if(scanChar(&cur, '\n') || (*cur == '\\' && !escapeSequence(&cur, tok)))
				return 0;

			cur++;
		}

		*tok = (Token){
			T_STRINGLIT,
			*str,
			cur
		};

		*str = cur;
		return 1;
	}

	return 0;
}

//iso c99 51
static int
identifier(char **str, Token *tok)
{
	char *cur = *str;
	if(!ISALPHA(*cur) && *cur != '_' && !universalCharacterName(&cur, tok))
		return 0;

	do cur++;
	while(ISALPHA(*cur) || universalCharacterName(&cur, tok) || ISDIGIT(*cur) || *cur == '_');

	*tok = (Token){	
		T_IDENTIFIER,
		*str,
		cur
	};

	*str = cur;
	return 1;
}

// iso c99 63
static int
punctuator(char **str, Token *tok)
{
	// iso c99 50 EXAMPLE 2
	static const char *words[] = {
		"[", "]", "(", ")", "{", "}", "...", ".", "->", "++", "--", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|=", "<<", ">>", "<=", ">=", "<:", ":>", "<%", "%>", "%:%:", "%:",  "<", ">", "==", "=", "!=", "!", "&&", "||", "&", "*", "+", "-", "~", ":", ";", ",", "##", "#", "/", "^", "%", "|",
	
		//TODO pre processor stuff
		//"\?\?=\?\?=", "\?\?=", "\?\?(", "\?\?)", "\?\?<", "\?\?>", "\?\?/", "\?\?\'", "\?\?!", "\?\?-",

		"?", NULL

	};

	static int tokens[] = {
		T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN, T_LCURLY, T_RCURLY, T_ELIPSIS, T_DOT, T_ARROW, T_INC, T_DEC, T_MULEQ, T_DIVEQ, T_MODEQ, T_ADDEQ, T_SUBEQ, T_LSHIFTEQ, T_RSHIFTEQ, T_ANDEQ, T_XOREQ, T_OREQ, T_LSHIFT, T_RSHIFT, T_LTEQ, T_GTEQ, T_LBRACE, T_RBRACE, T_LCURLY, T_RCURLY, T_PPCONCAT, T_POUND, T_LT, T_GT, T_EQUAL, T_EQ, T_NOTEQUAL, T_BANG, T_BAND, T_BOR, T_AND, T_STAR, T_PLUS, T_MINUS, T_TILDE, T_COLON, T_SEMI, T_COMMA, T_PPCONCAT, T_POUND, T_SLASH, T_XOR, T_PERCENT, T_OR,
		
		//T_PPCONCAT, T_POUND, T_LBRACE, T_RBRACE, T_LCURLY, T_RCURLY, T_SLASH, T_XOR ,T_OR, T_TILDE,

		T_QMARK
	};
	
	const char **word = words;
	while(*word){
		int index = 0;
		while(index[*word]){
			if(index[*word] != index[*str])
			       	goto end;
			index++;
		}
	
		*tok = (Token){
			tokens[word - words],
			*str,
			*str + index
		};

		*str = *str + index;
		return 1;
	end:
	    word++;
	}
	return 0;
}

// iso c99 50
static int
keyword(char **str, Token *tok)
{
	static const char *words[] = {
		"auto", "enum", "restrict", "unsigned", "break", "extern", "return", "void", "case", "float", "short", "volatile", "char", "for", "signed", "while", "const", "goto", "sizeof", "_Bool", "continue", "if", "static", "_Complex", "default", "inline", "struct", "_Imaginary", "do", "int", "switch", "double", "long", "typedef", "else", "register", "union", NULL
	};

	static int tokens[] = {
		T_AUTO, T_ENUM, T_RESTRICT, T_UNSIGNED, T_BREAK, T_EXTERN, T_RETURN, T_VOID, T_CASE, T_FLOAT, T_SHORT, T_VOLATILE, T_CHAR, T_FOR, T_SIGNED, T_WHILE, T_CONST, T_GOTO, T_SIZEOF, T_BOOL, T_CONTINUE, T_IF, T_STATIC, T_COMPLEX, T_DEFAULT, T_INLINE, T_STRUCT, T_IMAGINARY, T_DO, T_INT, T_SWITCH, T_DOUBLE, T_LONG, T_TYPEDEF, T_ELSE, T_REGISTER, T_UNION
	};

	const char **word = words;
	while(*word){
		int index = 0;
		while(index[*word]){
			if(index[*word] != index[*str])
			       	goto end;
			index++;
		}
	
		*tok = (Token){	
			tokens[word - words],
			*str,
			*str + index
		};

		*str = *str + index;
		return 1;
	end:
	    word++;
	}

	return 0;
}

static int
ppExtra(char **str, Token *tok)
{
	*tok = (Token){
		T_PPEXTRA,
		*str,
		*str + 1
	};

	(*str)++;
	return 1;
}

// iso c99 66 iso c99 184
static int
whiteSpace(char **str, Token *tok)
{
	char *cur = *str;

	while(1){
		switch(*cur){
			case '\n': case '\f': case '\r': case '\v': case '\t': case ' ': cur++; continue;
			case '/':
				cur++;
				if(scanChar(&cur, '/')){
					while(!scanChar(&cur, '\n'))
						cur++;
					continue;
				}
				
				if(scanChar(&cur, '*')){
					while(!(scanChar(&cur, '*') && scanChar(&cur, '/')))
						cur++;
					continue;
				}
				cur--;
		}

		*tok = (Token){
			T_WS,
			*str,
			cur
		};

		*str = cur;
		return 1;
	}
}

// iso c99 49
static int
token(char **str, Token *tok)
{
	//TODO preprocessor
	whiteSpace(str, tok);
	return keyword(str, tok) || identifier(str, tok) || constant(str, tok) || stringLit(str, tok) || punctuator(str, tok) || ppExtra(str, tok);
}

Token*
genTokens(char **str)
{
	Token *tok = allocToken();
	Token *ctok = tok;
	while(1){
		token(str, ctok);
		if(ctok->token == T_PPEXTRA)
			return tok;
		ctok->next = allocToken();
		ctok = ctok->next; 
	}
}

