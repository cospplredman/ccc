enum{
	/* tokens iso c99 49 */
	//keywords iso c99 50
	T_AUTO, T_ENUM, T_RESTRICT, T_UNSIGNED, T_BREAK, T_EXTERN, T_RETURN, T_VOID, T_CASE, T_FLOAT, T_SHORT, T_VOLATILE, T_CHAR, T_FOR, T_SIGNED, T_WHILE, T_CONST, T_GOTO, T_SIZEOF, T_BOOL, T_CONTINUE, T_IF, T_STATIC, T_COMPLEX, T_DEFAULT, T_INLINE, T_STRUCT, T_IMAGINARY, T_DO, T_INT, T_SWITCH, T_DOUBLE, T_LONG, T_TYPEDEF, T_ELSE, T_REGISTER, T_UNION,

	//identifier iso c99 51
	T_IDENTIFIER,
	
	//constant iso c99 54
	T_INTEGERCONST,
	T_FLOATCONST,
	// T_ENUMCONST, //is an identifier
	T_CHARCONST,

	//string literal iso c99 62
	T_STRINGLIT,

	//punctuator iso c99 63
	T_LBRACE, T_RBRACE, T_LPAREN, T_RPAREN, T_LCURLY, T_RCURLY, T_DOT, T_ARROW, T_INC, T_DEC, T_AND, T_STAR, T_PLUS, T_MINUS, T_TILDE, T_BANG, T_SLASH, T_PERCENT, T_LSHIFT, T_RSHIFT, T_LT, T_GT, T_LTEQ, T_GTEQ, T_EQUAL, T_NOTEQUAL, T_XOR, T_OR, T_BAND, T_BOR, T_QMARK, T_COLON, T_SEMI, T_ELIPSIS, T_EQUALS, T_EQ, T_MULEQ, T_DIVEQ, T_MODEQ, T_ADDEQ, T_SUBEQ, T_LSHIFTEQ, T_RSHIFTEQ, T_ANDEQ, T_XOREQ, T_OREQ, T_COMMA, T_POUND, T_PPCONCAT,

	/* pptokens iso c99 49 */
	//header
	T_PPHEADER,

	//identifier
	//pp-number
	//character const
	//string literal
	//punctuator

	//whitespace
	T_WS,

	//everything else
	T_PPEXTRA
};

typedef struct Token{
	int token;
	char *start, *end;
	
	union{
		long long int intValue;
		double floatValue;
		char *stringValue;
	};

	struct Token *next;
} Token;

Token *allocToken();
void freeToken(Token *tok);

Token *genTokens(char **str);
Token *genPPTokens(char **str);
void printToken(Token *tok);
void printTokens(Token *tok);
