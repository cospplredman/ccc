#include"token.h"
#include<stddef.h>
#include<stdlib.h>

#define ISDIGIT(a) ((a) >= '0' && (a) <= '9')
#define ISOCT(a)   ((a) >= '0' && (a) <= '7')
#define ISHEX(a)   ((a) >= '0' && (a) <= '9' || ((a) | 32) >= 'a' && ((a) | 32) <= 'f')
#define HEXVAL(b)  (ISDIGIT(b) ? ((b) - '0') : (((b) | 32) - 'a' + 10))
#define ISALPHA(a) (((a) | 32) >= 'a' && ((a) | 32) <= 'z')

static void
skipIntSufix(char **str)
{
	char *cur = *str;
	if((*cur | 32) == 'u')
		cur++;
	if((*cur | 32) == 'l')
		cur++;
	if((*cur | 32) == 'l')
		cur++;
	if((**str | 32) != 'u' && (*cur | 32) == 'u')
		cur++;
	if(cur != *str)
		*str = cur;
}

static int
decIntConst(char **str, Token **tok)
{
	long long v = 0;
	char *cur = *str;
	
	if(*cur < '1' || *cur > '9')
		return 0;

	do 
		v = v*10 + (*cur++ - '0');
	while(ISDIGIT(*cur));
	skipIntSufix(&cur);
	
	**tok = (Token){T_INTEGERCONST, NULL, *str, cur, .intValue = v};
	*str = cur;
	return 1;
}

static int
octIntConst(char **str, Token **tok)
{
	long long v = 0;
	char *cur = *str;
	if(*cur != '0')
		return 0;

	do
		v = v*8 + (*cur++ - '0');
	while(ISOCT(*cur));
	skipIntSufix(&cur);
	
	**tok = (Token){T_INTEGERCONST, NULL, *str, cur, .intValue = v};
	*str = cur;
	return 1;
}

static int
hexIntConst(char **str, Token **tok)
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

	**tok = (Token){T_INTEGERCONST, NULL, *str, cur, .intValue = v};
	*str = cur;
	return 1;
}

// iso c99 54
static int
intConst(char **str, Token **tok)
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
floatConst(char **str, Token **tok)
{
	char *cur = *str;
	if(*cur == '0' && (cur[1] | 32) == 'x'){
		cur += 2;
		if(hexDigitSeq(&cur) && *cur == '.'){
			cur++;
			hexDigitSeq(&cur);
		}else if((-1)[++cur] != '.' || !hexDigitSeq(&cur)){
			return 0;
		}

		if((*cur++ | 32) == 'p'){
			if(*cur == '+' || *cur == '-')
				cur++;
			
			digitSeq(&cur);
		}


	}else{
		if(digitSeq(&cur) && *cur == '.'){
			cur++;
			digitSeq(&cur);
		}else if((-1)[++cur] != '.' || !digitSeq(&cur)){
			return 0;
		}

		if((*cur++ | 32) == 'e'){
			if(*cur == '+' || *cur == '-')
				cur++;

			digitSeq(&cur);
		}	
	}

	if((*cur | 32) == 'l' || (*cur | 32) == 'f')
		cur++;

	(*tok)->token = T_FLOATCONST;
	(*tok)->start = *str;
	(*tok)->end = cur;
	(*tok)->floatValue = strtof((*tok)->start, NULL);
	*str = cur;
	return 1;
}

// iso c99 53
static int
universalCharacterName(char **str, Token **tok){
	char *cur = *str;
	if(*cur != '\\' || (cur[1] | 32) != 'u')
		return 0;

	cur += 2;
	
	(*tok)->intValue = 0;
	int j, i;
	for(j = 0; j < 2; j++){
		for(i = 0; i < 4; i++){
			if(!ISHEX(*cur))
				goto ret;

			(*tok)->intValue = (*tok)->intValue * 16 + HEXVAL(*cur);
			cur++;
		}
	}	

ret:
	if(j == 0)
		return 0;

	if(j == 1){
		(*tok)->intValue >>= i*4;
		cur -= i;
	}

	(*tok)->start = *str;
	(*tok)->end = cur;
	*str = cur;
	return 1;
}

// iso c99 60
static int
escapeSequence(char **str, Token **tok)
{

	if(universalCharacterName(str, tok))
		return 1;

	char *cur = *str;
	if(*cur++ != '\\')
		return 0;

	switch(*cur++){
		case '\\':
			(*tok)->intValue = '\\';
		break;
		case 'a':
			(*tok)->intValue = '\a';
		break;
		case 'b':
			(*tok)->intValue = '\b';
		break;
		case 'f':
			(*tok)->intValue = '\f';
		break;
		case 'n':
			(*tok)->intValue = '\n';
		break;
		case 'r':
			(*tok)->intValue = '\r';
		break;
		case 't':
			(*tok)->intValue = '\t';
		break;
		case 'v':
			(*tok)->intValue = '\v';
		break;
		case '\'':
			(*tok)->intValue = '\'';
		break;
		case '\"':
			(*tok)->intValue = '\"';
		break;
		case '?':
			(*tok)->intValue = '\?';
		break;
		case 'x':
			(*tok)->intValue = 0;
			if(!ISHEX(*cur))
				return 0;

			do{
				(*tok)->intValue = (*tok)->intValue * 16 + HEXVAL(*cur);
				cur++;
			}while(ISHEX(*cur));
		break;
		default:
			cur--;
			if(ISOCT(*cur)){
				(*tok)->intValue = *cur - '0';
				cur++;

				if(ISOCT(*cur)){
					(*tok)->intValue = (*tok)->intValue * 8 + *cur - '0';
					cur++;
				}

				if(ISOCT(*cur)){
					(*tok)->intValue = (*tok)->intValue * 8 + *cur - '0';
					cur++;
				}
			}else{
				return 0;
			}
	}

	(*tok)->start = *str;
	(*tok)->end = cur;
	*str = cur;
	return 1;
}

// iso c99 59
static int
charConst(char **str, Token **tok)
{
	char *cur = *str;
	if(*cur == 'L')
		cur++;

	if(*cur++ != '\'')
		return 0;

	if(!escapeSequence(&cur, tok)){
		if(*cur == '\'' || *cur == '\\' || *cur == '\n')
			return 0;
		(*tok)->intValue = *cur;
		cur++;
	}

	if(*cur++ != '\'')
		return 0;

	(*tok)->token = T_CHARCONST;
	(*tok)->start = *str;
	(*tok)->end = cur;
	*str = cur;
	return 1;
}

//iso c99 54
static int
constant(char **str, Token **tok)
{
	return floatConst(str, tok) || intConst(str, tok) || charConst(str, tok);
}

// iso c99 62
static int
stringLit(char **str, Token **tok)
{
	char *cur = *str;
	if(*cur == 'L')
		cur++;

	if(*cur++ != '\"')
		return 0;

	while(*cur != '"'){
		if(*cur == '\n')
			return 0;
		if(!escapeSequence(&cur, tok)){
			if(*cur == '\\')
				return 0;
			cur++;
		}
	}

	if(*cur++ != '\"')
		return 0;

	(*tok)->start = *str;
	(*tok)->end = cur;
	*str = cur;
	(*tok)->token = T_STRINGLIT;
	return 1;
}

//iso c99 51
static int
identifier(char **str, Token **tok)
{
	char *cur = *str;
	if(!ISALPHA(*cur) && *cur != '_' && !universalCharacterName(&cur, tok))
		return 0;

	do cur++;
	while(ISALPHA(*cur) || universalCharacterName(&cur, tok) || ISDIGIT(*cur) || *cur == '_');
	
	(*tok)->token = T_IDENTIFIER;
	(*tok)->start = *str;
	(*tok)->end = cur;
	*str = cur;
	return 1;
}

// iso c99 63
static int
punctuator(char **str, Token **tok)
{
	// iso c99 50 EXAMPLE 2
	static const char *words[] = {
		"[", "]", "(", ")", "{", "}", "...", ".", "->", "++", "--", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|=", "<<", ">>", "<=", ">=", "<:", ":>", "<%", "%>", "%:%:", "%:",  "<", ">", "==", "=", "!=", "!", "&&", "||", "&", "*", "+", "-", "~", ":", ";", ",", "##", "#", "/", "^", "%", "|",
		
		"\?\?=\?\?=", "\?\?=", "\?\?(", "\?\?)", "\?\?<", "\?\?>", "\?\?/", "\?\?\'", "\?\?!", "\?\?-",

		"?", NULL

	};

	static int tokens[] = {
		T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN, T_LCURLY, T_RCURLY, T_ELIPSIS, T_DOT, T_ARROW, T_INC, T_DEC, T_MULEQ, T_DIVEQ, T_MODEQ, T_ADDEQ, T_SUBEQ, T_LSHIFTEQ, T_RSHIFTEQ, T_ANDEQ, T_XOREQ, T_OREQ, T_LSHIFT, T_RSHIFT, T_LTEQ, T_GTEQ, T_LBRACE, T_RBRACE, T_LCURLY, T_RCURLY, T_PPCONCAT, T_POUND, T_LT, T_GT, T_EQUAL, T_EQ, T_NOTEQUAL, T_BANG, T_BAND, T_BOR, T_AND, T_STAR, T_PLUS, T_MINUS, T_TILDE, T_COLON, T_SEMI, T_COMMA, T_PPCONCAT, T_POUND, T_SLASH, T_XOR, T_PERCENT, T_OR,
		
		T_PPCONCAT, T_POUND, T_LBRACE, T_RBRACE, T_LCURLY, T_RCURLY, T_SLASH, T_XOR ,T_OR, T_TILDE,

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
		
		(*tok)->start = *str;
		(*tok)->end = *str + index;
		(*tok)->token = tokens[word - words];
		*str = *str + index;
		return 1;
	end:;
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

	const char **word = words;
	while(*word){
		int index = 0;
		while(index[*word]){
			if(index[*word] != index[*str])
			       	goto end;
			index++;
		}
		
		(*tok)->start = *str;
		(*tok)->end = *str + index;
		(*tok)->token = tokens[word - words];
		*str = *str + index;
		return 1;
	end:;
	    word++;
	}

	return 0;
}

//TODO preprocessor
static int
ppExtra(char **str, Token **tok)
{
	(*tok)->start = *str;
	*str++;
	(*tok)->end = *str;
	(*tok)->token = T_PPEXTRA;
	return 1;
}

// iso c99 66 iso c99 184
static int
whiteSpace(char **str, Token **tok)
{
	char *cur = *str;

	while(1){
		switch(*cur){
			case '\n':
			case '\f':
			case '\r':
			case '\v':
			case '\t':
			case ' ':
				cur++;
			break;
			case '/':
				cur++;
				if(*cur == '/'){
					while(*cur != '\n'){
						cur++;
						if(*cur == '\\' && cur[1] == '\n')
							cur += 2;
					}
				}else if(*cur == '*'){
					while(*cur != '*' || cur[1] != '/') cur++;
					cur += 2;
				}else{
					cur--;
					(*tok)->start = *str;
					(*tok)->end = cur;
					*str = cur;
					(*tok)->token = T_WS;
					return 1;
				}	
			break;
			default:
				if(cur == *str)
					return 0;

				(*tok)->start = *str;
				(*tok)->end = cur;
				*str = cur;
				(*tok)->token = T_WS;
				return 1;
		}
	}
}

// iso c99 49
static int
token(char **str, Token **tok)
{
	whiteSpace(str, tok);
	return /* whiteSpace(str, tok) || */ keyword(str, tok) || identifier(str, tok) || constant(str, tok) || stringLit(str, tok) || punctuator(str, tok) || ppExtra(str, tok);
}

/*               *
 * END OF TOKENS *
 *               */

// iso c99 64
static int
headerName(char **str, Token **tok)
{
	char *cur = *str;
	if(*cur == '<'){
		cur++;
		while(*cur != '>'){
			if(*cur == '\n')
				return 0;
			cur++;
		}
	}else if(*cur == '"'){
		cur++;
		while(*cur != '"'){
			if(*cur == '\n')
				return 0;
			cur++;
		}
	}else{
		return 0;
	}

	(*tok)->start = *str;
	(*tok)->end = cur;
	*str = cur;
	(*tok)->token = T_PPHEADER;
}

// iso c99 49
static int
ppToken(char **str, Token **tok)
{
	return identifier(str, tok) || stringLit(str, tok) || constant(str, tok) || punctuator(str, tok) || whiteSpace(str, tok) || ppExtra(str, tok);
}

// iso c99 145
static int
nonDirective(char **str, Token **tok)
{
	char *cur = *str;
	if(!punctuator(str, tok) || (*tok)->token != T_POUND)
		return 0;

	Token t, *q = &t;
	while(ppToken(&cur, &q))
		if(t.token == T_WS && t.intValue)
			return 1;
}

// iso c99 145
static int
textLine(char **str, Token **tok)
{
	while(ppToken(str, tok)){
		if((*tok)->token == T_WS && (*tok)->intValue)
			return 1;
		*tok = (*tok)->next = malloc(sizeof(Token));
	}
	return 0;
}

// iso c99 145
static int
controlLine(char **str, Token **tok)
{
	return 0;
}

// iso c99 145
static int
ifSection(char **str, Token **tok)
{
	return 0;
}

// iso c99 145
static int
groupPart(char **str, Token **tok)
{
	return ifSection(str, tok) || controlLine(str, tok) || nonDirective(str, tok) || textLine(str, tok);
}

// iso c99 145
static int
group(char **str, Token **tok)
{
	//TODO
	return 0;
}


/*               *
 * END OF PPTOKENS *
 *               */

Token*
genTokens(char **str)
{
	Token *tok = malloc(sizeof(Token));
	Token *ctok = tok;
	while(1){
		token(str, &ctok);
		if(ctok->token == T_PPEXTRA)
			return tok;
		ctok->next = malloc(sizeof(Token));
		ctok = ctok->next; 
	}
}

