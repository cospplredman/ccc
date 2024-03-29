#include"token.h"
#include<stdio.h>
#include<stddef.h>
#include<stdlib.h>

/* util functions */

static const char *tokenName[] = {
	"T_AUTO", "T_ENUM", "T_RESTRICT", "T_UNSIGNED", "T_BREAK", "T_EXTERN", "T_RETURN", "T_VOID", "T_CASE", "T_FLOAT", "T_SHORT", "T_VOLATILE", "T_CHAR", "T_FOR", "T_SIGNED", "T_WHILE", "T_CONST", "T_GOTO", "T_SIZEOF", "T_BOOL", "T_CONTINUE", "T_IF", "T_STATIC", "T_COMPLEX", "T_DEFAULT", "T_INLINE", "T_STRUCT", "T_IMAGINARY", "T_DO", "T_INT", "T_SWITCH", "T_DOUBLE", "T_LONG", "T_TYPEDEF", "T_ELSE", "T_REGISTER", "T_UNION",
	"T_IDENTIFIER",
	
	"T_INTEGERCONST",
	"T_FLOATCONST",
	"T_CHARCONST",

	"T_STRINGLIT",

	"T_LBRACE", "T_RBRACE", "T_LPAREN", "T_RPAREN", "T_LCURLY", "T_RCURLY", "T_DOT", "T_ARROW", "T_INC", "T_DEC", "T_AND", "T_STAR", "T_PLUS", "T_MINUS", "T_TILDE", "T_BANG", "T_SLASH", "T_PERCENT", "T_LSHIFT", "T_RSHIFT", "T_LT", "T_GT", "T_LTEQ", "T_GTEQ", "T_EQUAL", "T_NOTEQUAL", "T_XOR", "T_OR", "T_BAND", "T_BOR", "T_QMARK", "T_COLON", "T_SEMI", "T_ELIPSIS", "T_EQUALS", "T_EQ", "T_MULEQ", "T_DIVEQ", "T_MODEQ", "T_ADDEQ", "T_SUBEQ", "T_LSHIFTEQ", "T_RSHIFTEQ", "T_ANDEQ", "T_XOREQ", "T_OREQ", "T_COMMA", "T_POUND", "T_PPCONCAT",

	"T_EOF",

	"PPT_PPNUMBER",
	"PPT_PPHEADER",
	"PPT_PPEXTRA",
	"T_WS"
};

void
printToken(Token *tok)
{
	if(tok){
		printf("%-15s: \"%.*s\"\n", tokenName[tok->token], (int)(tok->end - tok->start), tok->start);
		printToken(tok->next);
	}
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

Token*
initToken(int type, char *s, char *e)
{
	Token *tl = allocToken();
	*tl = (Token){
		.token = type,
		.start = s,
		.end = e,
	};
	return tl;
}

int
scanChar(char **str, char ch)
{
	if(**str == ch){
		(*str)++;
		return 1;
	}

	return 0;
}

int
scanCharLower(char **str, char ch)
{
	if((**str | 32) == ch){
		(*str)++;
		return 1;
	}

	return 0;
}

int
scanLut(char **str, char *lut)
{
	if(lut[**str]){
		(*str)++;
		return 1;
	}

	return 0;	
}

//TODO everything unsigned char
int
scanRange(char **str, char l, char r)
{
	if(**str >= l && **str <= r){
		(*str)++;
		return 1;
	}

	return 0;
}

int
scanAlpha(char **str)
{
	return scanRange(str, 'a', 'z') || scanRange(str, 'A', 'Z');
}

int
scanDigit(char **str)
{
	return scanRange(str, '0', '9');
}

int
scanOctDigit(char **str)
{
	return scanRange(str, '0', '7');
}

int
scanHexDigit(char **str)
{
	return scanDigit(str) || scanRange(str, 'a', 'f') || scanRange(str, 'A', 'F');
}

static void
scanIntSufix(char **str)
{
	char *cur = *str;
	int opt = scanCharLower(&cur, 'u');
	scanCharLower(&cur, 'l');
	scanCharLower(&cur, 'l');
	opt && scanCharLower(&cur, 'u');
	*str = cur;
}

static int
hexDigitSeq(char **str)
{
	if(scanHexDigit(str)){
		while(scanHexDigit(str));
		return 1;
	}	
	return 0;
}

static int
digitSeq(char **str)
{
	if(scanDigit(str)){
		while(scanDigit(str));
		return 1;
	}
	return 0;
}

int
scanString(char **a, const char *b)
{
	char *cur = *a;
	while(*b)
		if(*(cur++) != *(b++))
			return 0;

	*a = cur;
	return 1;
}

/* token parsing */

static int
decIntConst(char **str, Token **tok)
{
	char *cur = *str;
	if(digitSeq(&cur)){
		scanIntSufix(&cur);
		*tok = initToken(T_INTEGERCONST, *str, cur);
		*str = cur;
		return 1;
	}

	return 0;
}

static int
octIntConst(char **str, Token **tok)
{
	char *cur = *str;
	if(scanChar(&cur, '0')){
		while(scanOctDigit(&cur));
		scanIntSufix(&cur);
		*tok = initToken(T_INTEGERCONST, *str, cur);	
		*str = cur;
		return 1;
	}

	return 0;
}

static int
hexIntConst(char **str, Token **tok)
{
	char *cur = *str;
	if(scanChar(&cur, '0') && scanCharLower(&cur, 'x') && hexDigitSeq(&cur)){ 
		scanIntSufix(&cur);
		*tok = initToken(T_INTEGERCONST, *str, cur);
		*str = cur;
		return 1;
	}

	return 0;
}

// iso c99 54
static int
intConst(char **str, Token **tok)
{
	return decIntConst(str, tok) || hexIntConst(str, tok) || octIntConst(str, tok);
}

// iso c99 57
static int
floatConst(char **str, Token **tok)
{
	char *cur = *str, *ccur;
	int opt, opt1;
	if(scanChar(&cur, '0') && scanCharLower(&cur, 'x')){
		opt = hexDigitSeq(&cur);
		if(scanChar(&cur, '.')){
			opt1 = hexDigitSeq(&cur);
			if(opt || opt1){
				ccur = cur;
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
	opt = digitSeq(&cur);
	if(scanChar(&cur, '.')){
		opt1 = digitSeq(&cur);
		if(opt || opt1){
			ccur = cur;
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

	*tok = initToken(T_FLOATCONST, *str, cur);
	*str = cur;
	return 1;
}

// iso c99 53
static int
universalCharacterName(char **str, Token **tok){
	char *cur = *str;
	int j, i;
	if(scanChar(&cur, '\\') && scanCharLower(&cur, 'u')){
		for(j = 0; j < 2; j++){
			for(i = 0; i < 4; i++){
				if(!scanHexDigit(&cur))
					goto ret;
			}
		}
	}

	return 0;
ret:
	if(j == 0)
		return 0;

	if(j == 1)
		cur -= i;

	//*tok = initToken(T_EOF, *str, cur);
	*str = cur;
	return 1;
}

// iso c99 60
static int
escapeSequence(char **str, Token **tok)
{
	char *cur = *str;

	if(universalCharacterName(str, tok))
		return 1;

	if(scanChar(&cur, '\\')){
		//TODO get char
		switch((*cur)++){
			case '\\':case 'a': case 'b': case 'f': case 'n': case 'r': case 't': case 'v': case '\'': case '\"': case '?': break;
			case 'x': 
				if(!hexDigitSeq(&cur))
					return 0;
			break;
			default:
				cur--;
				if(scanOctDigit(&cur)){
					scanOctDigit(&cur) && scanOctDigit(&cur);
					break;
				}
				return 0;
		}

		*tok = initToken(T_EOF, *str, cur);
		*str = cur;
		return 1;
	}
	return 0;
	
}

// iso c99 59
int
charConst(char **str, Token **tok)
{
	char *cur = *str;
	//TODO wide strings?
	scanChar(&cur, 'L');

	if(scanChar(&cur, '\'')){
		if(!escapeSequence(&cur, tok) && (scanChar(&cur, '\'') || scanChar(&cur, '\\') || scanChar(&cur, '\n')))
				return 0;
		cur++;
		if(scanChar(&cur, '\'')){
			*tok = initToken(T_CHARCONST, *str, cur);
			*str = cur;
			return 1;
		}
	}

	return 0;
}

//iso c99 54
static int
constant(char **str, Token **tok)
{
	return floatConst(str, tok) || intConst(str, tok) || charConst(str, tok);
}

// iso c99 62
int
stringLit(char **str, Token **tok)
{
	Token *tl;
	char *cur = *str;
	scanChar(&cur, 'L');

	if(scanChar(&cur, '"')){
		while(!scanChar(&cur, '"')){
			if(scanChar(&cur, '\n'))
				return 0;

			//TODO gross
			if(*cur == '\\'){
				if(escapeSequence(&cur, &tl)){
					freeToken(tl);
					continue;
				}
				return 0;
			}
			cur++;
		}

		*tok = initToken(T_STRINGLIT, *str, cur);
		*str = cur;
		return 1;
	}

	return 0;
}

//iso c99 51
static int
identifier(char **str, Token **tok)
{
	char *cur = *str;
	if(scanAlpha(&cur) || scanChar(&cur, '_') || universalCharacterName(&cur, tok)){
		//TODO gross
		while(scanAlpha(&cur) || universalCharacterName(&cur, tok) || scanDigit(&cur) || scanChar(&cur, '_'));

		*tok = initToken(T_IDENTIFIER, *str, cur);
		*str = cur;
		return 1;
	}
	return 0;
}

// iso c99 63
int
punctuator(char **str, Token **tok)
{
	// iso c99 50 EXAMPLE 2
	static const char *words[] = {
		"[", "]", "(", ")", "{", "}", "...", ".", "->", "++", "--", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|=", "<<", ">>", "<=", ">=", 
		//"<:", ":>", "<%", "%>", "%:%:", "%:", 
		"<", ">", "==", "=", "!=", "!", "&&", "||", "&", "*", "+", "-", "~", ":", ";", ",", "##", "#", "/", "^", "%", "|",
	
		//TODO pre processor stuff
		//"\?\?=\?\?=", "\?\?=", "\?\?(", "\?\?)", "\?\?<", "\?\?>", "\?\?/", "\?\?\'", "\?\?!", "\?\?-",

		"?", NULL

	};

	static int tokens[] = {
		T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN, T_LCURLY, T_RCURLY, T_ELIPSIS, T_DOT, T_ARROW, T_INC, T_DEC, T_MULEQ, T_DIVEQ, T_MODEQ, T_ADDEQ, T_SUBEQ, T_LSHIFTEQ, T_RSHIFTEQ, T_ANDEQ, T_XOREQ, T_OREQ, T_LSHIFT, T_RSHIFT, T_LTEQ, T_GTEQ, 
		//T_LBRACE, T_RBRACE, T_LCURLY, T_RCURLY, T_PPCONCAT, T_POUND,

	       	T_LT, T_GT, T_EQUAL, T_EQ, T_NOTEQUAL, T_BANG, T_BAND, T_BOR, T_AND, T_STAR, T_PLUS, T_MINUS, T_TILDE, T_COLON, T_SEMI, T_COMMA, T_PPCONCAT, T_POUND, T_SLASH, T_XOR, T_PERCENT, T_OR,
		
		//T_PPCONCAT, T_POUND, T_LBRACE, T_RBRACE, T_LCURLY, T_RCURLY, T_SLASH, T_XOR ,T_OR, T_TILDE,

		T_QMARK
	};

	char *cur = *str;	
	const char **word = words;
	while(*word){
		if(scanString(&cur, *word)){
			*tok = initToken(tokens[word - words], *str, cur);
			*str = cur;
			return 1;
		}
		word++;
	}

	return 0;
}

// iso c99 50
static int
keyword(char **str, Token **tok)
{
	static const char *words[] = {
		"auto", "enum", "restrict", "unsigned", "break", "extern", "return", "void", "case", "float", "short", "volatile", "char", "for", "signed", "while", "const", "goto", "sizeof", "_Bool", "continue", "if", "static", "_Complex", "default", "inline", "struct", "_Imaginary", "do", "int", "switch", "double", "long", "typedef", "else", "register", "union", NULL
	};

	static int tokens[] = {
		T_AUTO, T_ENUM, T_RESTRICT, T_UNSIGNED, T_BREAK, T_EXTERN, T_RETURN, T_VOID, T_CASE, T_FLOAT, T_SHORT, T_VOLATILE, T_CHAR, T_FOR, T_SIGNED, T_WHILE, T_CONST, T_GOTO, T_SIZEOF, T_BOOL, T_CONTINUE, T_IF, T_STATIC, T_COMPLEX, T_DEFAULT, T_INLINE, T_STRUCT, T_IMAGINARY, T_DO, T_INT, T_SWITCH, T_DOUBLE, T_LONG, T_TYPEDEF, T_ELSE, T_REGISTER, T_UNION
	};

	char *cur = *str;
	const char **word = words;
	while(*word){
		if(scanString(&cur, *word)){
			*tok = initToken(tokens[word - words], *str, cur);
			*str = cur;
			return 1;
		}
		word++;
	}

	return 0;
}

// iso c99 66 iso c99 184
int
whiteSpace(char **str, Token **tok)
{
	//TODO preprocessor
	char *cur = *str;
	int ret = 0;
	while(1){
		switch(*cur){
			case '\n': ret = 1; case '\f': case '\r': case '\v': case '\t': case ' ': cur++; continue;
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

		break;
	}

	if(ret){
		*tok = initToken(T_WS, *str, cur);
		*str = cur;
		return 1;
	}

	*str = cur;
	return 0;
}

// iso c99 49
static int
token(char **str, Token **tok)
{
	Token *tl = NULL;
	//TODO gross
	if(whiteSpace(str, &tl))
		freeToken(tl);
	return keyword(str, tok) || identifier(str, tok) || constant(str, tok) || stringLit(str, tok) || punctuator(str, tok);
}

static int
eof(char **str, Token **tok)
{
	//TODO gross
	if(whiteSpace(str, tok))
		freeToken(*tok);
	if(*str == NULL || scanChar(str, 0)){
		*tok = initToken(T_EOF, NULL, NULL);
		(*tok)->next = NULL;
		return 1;
	}

	return 0;
}

static int
tokens(char **str, Token **tok)
{
	Token *tl = NULL;
	char *cur = *str;
	if(token(&cur, &tl)){
		if(tokens(&cur, &(tl->next)) || eof(&cur, &(tl->next))){
			*tok = tl;
			*str = cur;
			return 1;
		}
		tl->next = NULL;
		freeToken(tl);
	}

	return 0;
}

Token*
genTokens(char *str)
{
	Token *tl;
	char *cur = str;

	if(cur && tokens(&cur, &tl))
		return tl;

	printf("could not parse tokens\n");
	tl = initToken(T_EOF, NULL, NULL);
	tl->next = NULL;
	return tl;
}

/* pp tokens */

// iso c99 65
static int
ppnumber(char **str, Token **tok)
{
	char *cur = *str;

	if(scanDigit(&cur) || scanChar(&cur, '.') && scanDigit(&cur)){
		while(1){
			if(scanDigit(&cur) || scanAlpha(&cur) || scanChar(&cur, '_') || scanCharLower(&cur, 'e') || scanCharLower(&cur, 'p') || scanChar(&cur, '.'))
				continue;
			break;
		}

		*tok = initToken(PPT_PPNUMBER, *str, cur);
		*str = cur;
		return 1;
	}

	return 0;
}

// iso c99 49
static int
ppextra(char **str, Token **tok)
{
	if(*str){
		*tok = initToken(PPT_PPEXTRA, *str, *str + 1);
		*str = *str + 1;
		return 1;
	}

	return 0;
}

// iso c99 49
static int
pptoken(char **str, Token **tok)
{
	return whiteSpace(str, tok) || identifier(str, tok) || ppnumber(str, tok) || charConst(str, tok) || stringLit(str, tok) || punctuator(str, tok);
}

static int
pptokens(char **str, Token **tok)
{
	Token *tl = NULL;
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

Token *genPPTokens(char *str)
{
	Token *tl;
	char *cur = str;	

	if(cur && pptokens(&cur, &tl))
		return tl;

	printf("could not parse pptokens\n");
	tl = initToken(PPT_PPEXTRA, NULL, NULL);
	tl->next = NULL;
	return tl;
}


