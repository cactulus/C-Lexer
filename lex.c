#include <nk_types.h>

#define NK_CSTD_IMPLEMENTATION
#include <nk_cstd.h>

typedef struct {
	char *current;
} LexerContext;

typedef struct {
	u8 type;

	union {
		CString lexeme;
		s64 int_value;
		f64 float_value;
		s64 keyword;
	};
} Token;

enum token_keyword {
	TK_AUTO = 0,
	TK_BREAK,
	TK_CASE,
	TK_CHAR,
	TK_CONST,
	TK_CONTINUE,
	TK_DEFAULT,
	TK_DO,
	TK_DOUBLE,
	TK_ELSE,
	TK_ENUM,
	TK_EXTERN,
	TK_FLOAT,
	TK_FOR,
	TK_GOTO,
	TK_IF,
	TK_INT,
	TK_LONG,
	TK_REGISTER,
	TK_RETURN,
	TK_SHORT,
	TK_SIGNED,
	TK_SIZEOF,
	TK_STATIC,
	TK_STRUCT,
	TK_SWITCH,
	TK_TYPEDEF,
	TK_UNION,
	TK_UNSIGNED,
	TK_VOID,
	TK_VOLATILE,
	TK_WHILE
};

/* note: for single char operators, the ascii code is used.
Care to not overlap char codes for single char operators
IMPORTANT: DON'T CHANGE ORDER OF THESE!
*/
enum token_type {
	T_ERROR = 0,
	T_END_OF_FILE,
	T_IDENTIFIER,
	T_INT_LITERAL,
	T_FLOAT_LITERAL,
	T_SINGLE_CHAR_OPERATOR,
	T_TWO_CHAR_OPERATOR,
	T_KEYWORD,
	T_CHAR,
	T_STRING,

	/* two char operators */
	T_EQ_EQ,
	T_AND_AND,
	T_OR_OR,
	T_LSH,
	T_RSH,

	T_ARROW,
	T_PLUS_PLUS,
	T_MINUS_MINUS,

	T_PLUS_EQ,
	T_MINUS_EQ,
	T_STAR_EQ,
	T_SLASH_EQ,
	T_PERCENT_EQ,
	T_XOR_EQ,
	T_TILDE_EQ,
	T_AND_EQ,
	T_OR_EQ,
	T_NOT_EQ,
	T_GREATER_EQ,
	T_LESS_EQ,

	/* three char operators*/
	T_LSH_EQ,
	T_RSH_EQ
};

enum states {
	S_INITIAL = 0,
	S_FINAL = 1,
	S_ERROR = 2,
	S_SPACE = 3,
	S_ID = 4,
	S_INT = 5,
	S_FLOAT = 6,
	S_SC_OP = 7,
	S_CHAR = 8,
	S_STRING = 9,
	S_CHAR_LAST = 10,
	S_STRING_LAST = 11,
	S_EQ = 12,
	S_EQ_EQ = 13,
	S_EQUABLE = 14,
	/* any char, folowed by = */
	S_EQ_POST = 15,
};

enum equivalence_classes {
	EC_NULL = 0,
	EC_SPACE = 1,
	EC_LF = 2,
	EC_ALPHA = 3,
	EC_DIGIT = 4,
	EC_DOT = 5,
	EC_SC_OP = 6,
	EC_EQUABLE = 7,
	EC_EQUALS = 8,
	EC_PLUS = 9,
	EC_MINUS = 10,
	EC_STAR = 11,
	EC_SLASH = 12,
	EC_AND = 13,
	EC_OR = 14,
	EC_HASH = 15,
	EC_GREATER = 16,
	EC_LESSER = 17,
	EC_QUOTE = 18,
	EC_DOUBLE_QUOTES = 19,
};

const u8 equivalence_class[] = {
	EC_NULL, 0, 0, 0, 0, 0, 0, 0,
	0, EC_SPACE, EC_LF, 0, 0, EC_SPACE, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	EC_SPACE, EC_EQUABLE, EC_DOUBLE_QUOTES, EC_HASH, 0, EC_EQUABLE, EC_AND, EC_QUOTE,
	EC_SC_OP, EC_SC_OP, EC_STAR, EC_PLUS, EC_SC_OP, EC_MINUS, EC_DOT, EC_SLASH,
	EC_DIGIT, EC_DIGIT, EC_DIGIT, EC_DIGIT, EC_DIGIT, EC_DIGIT, EC_DIGIT, EC_DIGIT,
	EC_DIGIT, EC_DIGIT, EC_SC_OP, EC_SC_OP, EC_LESSER, EC_EQUALS, EC_GREATER, EC_SC_OP,
	0, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA,
	EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA,
	EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA,
	EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_SC_OP, 0, EC_SC_OP, EC_EQUABLE, EC_ALPHA,
	0, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA,
	EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA,
	EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA,
	EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_SC_OP, EC_OR, EC_SC_OP, EC_EQUABLE, 0,
};


/* transition table from [state][equivalence class] -> new state */
const u8 transitions[16][20] = {
	/* initial */
	{ S_FINAL, S_SPACE, S_SPACE, S_ID, S_INT, S_ERROR, S_SC_OP, S_EQUABLE, S_EQ, -1,
	  -1, -1, -1, -1, -1, -1, -1, -1, S_CHAR, S_STRING},
	/* final */
	{ S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR,
	  S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR},
	/* error */
	{ S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR,
	  S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR, S_ERROR},
	/* space */
	{ S_FINAL, S_SPACE, S_SPACE, S_ID, S_INT, S_ERROR, S_SC_OP,  S_EQUABLE, S_EQ, S_FINAL,
	  S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_CHAR, S_STRING},
	/* id */
	{ S_FINAL, S_FINAL, S_FINAL, S_ID, S_ID, S_ERROR, S_FINAL, S_FINAL, S_FINAL, S_FINAL,
	  S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL},
	/* int */
	{ S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_INT, S_FLOAT, S_FINAL, S_FINAL, S_FINAL, S_FINAL,
	  S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL},
	/* float */
	{ S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FLOAT, S_ERROR, S_FINAL, S_FINAL, S_FINAL, S_FINAL,
	  S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL},
	/* single char operator */
	{ S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL,
	  S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL},
	  /* char */
	{ S_ERROR, S_CHAR, S_ERROR, S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR,
	  S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR_LAST, S_CHAR},
	/* string */
	{ S_ERROR, S_STRING, S_ERROR, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING,
	  S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING_LAST},
	/* char last */
	{ S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL,
	  S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL},
	/* string last */
	{ S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL,
	  S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL},
	/* equals */
	{ S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_EQ_EQ, S_FINAL,
	S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL},
	/* equals equals */
	{ S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL,
	  S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL},
	/* equable */
	{ S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_EQ_POST, S_FINAL,
		S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL},
	/* post equals */
	{ S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL,
	  S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL, S_FINAL},
};

const u8 in_token[] = {
	0, 0, 1, 0,
	1, 1, 1, 1,
	1, 1, 1, 1,
	1, 1, 1, 1
};

const u8 state_to_token_type[] = {
	T_END_OF_FILE,
	T_END_OF_FILE,
	T_ERROR,
	T_END_OF_FILE,
	T_IDENTIFIER,
	T_INT_LITERAL,
	T_FLOAT_LITERAL,
	T_SINGLE_CHAR_OPERATOR,
	T_CHAR,
	T_STRING,
	T_CHAR,
	T_STRING,
	T_SINGLE_CHAR_OPERATOR,
	T_EQ_EQ,
	T_SINGLE_CHAR_OPERATOR,
	T_TWO_CHAR_OPERATOR
};

const char *keywords[] = {
	"auto", "break", "case", "char",
	"const", "continue", "default",
	"do", "double", "else", "enum",
	"extern", "float", "for", "goto",
	"if", "int", "long", "register",
	"return", "short", "signed",
	"sizeof", "static", "struct",
	"switch", "typedef", "union",
	"unsigned", "void", "volatile",
	"while"
};
const int KEYWORD_COUNT = 32;

const char *two_char_operators[] = {
	"==", "&&", "||", "<<", ">>", "->", "++", "--",
	"+=", "-=", "*=", "/=", "%=", "^=", "~=", "&=", "|=",
	"!=", ">=", "<="
};
const int TWO_CHAR_OPERATOR_COUNT = 20;

Token lex(LexerContext *ctx) {
	char *p = ctx->current;

	u8 state = S_INITIAL;
	size_t token_len = 0;

	u8 last_state = state;
	do {
		u8 c = *p++;
		u8 eq_cls = equivalence_class[c];
		last_state = state;
		state = transitions[state][eq_cls];
		token_len += in_token[state];
	} while (state != S_FINAL);

	char *token_start = p - token_len - 1;

	Token tk = {0};
	tk.type = state_to_token_type[last_state];

	switch (tk.type) {
	case T_INT_LITERAL: {
		if (token_len >= 67) {
			tk.type = T_ERROR;
			return tk;
		}

		/* longest possible string is : 0b1..64 bits */
		char buf[67];
		memcpy(buf, token_start, token_len);
		buf[token_len + 1] = 0;

		tk.int_value = strtol(buf, 0, 10);
	} break;
	case T_FLOAT_LITERAL: {
		if (token_len >= 67) {
			tk.type = T_ERROR;
			return tk;
		}

		/* longest possible string is : 0b1..64 bits */
		char buf[67];
		memcpy(buf, token_start, token_len);
		buf[token_len + 1] = 0;

		tk.float_value = strtof(buf, 0);
	} break;
	case T_SINGLE_CHAR_OPERATOR: {
		tk.type = (u8) *token_start;
	} break;
	case T_TWO_CHAR_OPERATOR: {
		for (s32 i = 0; i < TWO_CHAR_OPERATOR_COUNT; ++i) {
			const char *op = two_char_operators[i];
			if (op[0] == *token_start && op[1] == *(token_start + 1)) {
				tk.type = T_EQ_EQ + i;
				break;
			}
		}
	}
	case T_CHAR: {
		tk.int_value = *(token_start + 1);
	} break;
	case T_STRING: {
		tk.lexeme = cstring(token_start + 1, token_len - 2);
	} break;
	case T_IDENTIFIER: {
		CString lexeme = cstring(token_start, token_len);

		s32 is_keyword = 0;
		for (s32 i = 0; i < KEYWORD_COUNT; ++i) {
			if (cstring_compare_s(lexeme, keywords[i])) {
				is_keyword = 1;
				tk.type = T_KEYWORD;
				tk.keyword = i;
				break;
			}
		}

		if (!is_keyword) {
			tk.lexeme = lexeme;
		}
	} break;
	default:
		break;
	}

	ctx->current = p - 1;
	return tk;
}

const char *token_type_to_str(u8 token_type) {
	switch (token_type) {
	case T_ERROR: return "error token";
	case T_END_OF_FILE: return "end of file";
	case T_IDENTIFIER: return "identifier";
	case T_INT_LITERAL: return "integer";
	case T_FLOAT_LITERAL: return "float";
	case T_SINGLE_CHAR_OPERATOR: return "single char operator";
	case T_TWO_CHAR_OPERATOR: return "two char operator";
	case T_KEYWORD: return "keyword";
	case T_CHAR: return "char";
	case T_STRING: return "string";
	case T_EQ_EQ: return "equals equals";
	case T_AND_AND: return "and and";
	case T_OR_OR: return "or or";
	case T_LSH: return "left shift";
	case T_RSH: return "right shift";
	case T_ARROW: return "arrow";
	case T_PLUS_PLUS: return "plus plus";
	case T_MINUS_MINUS: return "minus minus";
	case T_PLUS_EQ: return "plus equals";
	case T_MINUS_EQ: return "minus equals";
	case T_STAR_EQ: return "star equals";
	case T_SLASH_EQ: return "slash equals";
	case T_PERCENT_EQ: return "percent equals";
	case T_XOR_EQ: return "xor equals";
	case T_TILDE_EQ: return "tilde equals";
	case T_AND_EQ: return "and equals";
	case T_OR_EQ: return "or equals";
	case T_NOT_EQ: return "not equals";
	case T_GREATER_EQ: return "greater equals";
	case T_LESS_EQ: return "less equals";
	case T_LSH_EQ: return "left shift equals";
	case T_RSH_EQ: return "right shift equals";
	default: "";
	}

	return "";
}

s32 main() {
	const char *input = "= == ^= ^ =";
	
	LexerContext ctx;
	ctx.current = input;

	Token token;
	while ((token = lex(&ctx)).type != T_END_OF_FILE) {
		switch (token.type) {
		case T_INT_LITERAL:
			printf("%s: %lld\n", token_type_to_str(token.type), token.int_value);
			break;
		case T_FLOAT_LITERAL:
			printf("%s: %f\n", token_type_to_str(token.type), token.float_value);
			break;
		case T_IDENTIFIER:
		case T_STRING:
			printf("%s: '%.*s'\n", token_type_to_str(token.type), (u32) token.lexeme.length, token.lexeme.ptr);
			break;
		case T_KEYWORD:
			printf("%s: %d\n", token_type_to_str(token.type), token.keyword);
			break;
		case T_CHAR:
			printf("%s: '%c'\n", token_type_to_str(token.type), token.int_value);
			break;
		default:
			if (token.type >= T_EQ_EQ && token.type <= T_RSH_EQ) {
				printf("%s\n", token_type_to_str(token.type));
			} else {
				printf("%c\n", token.type);
			}
			break;
		}
	}

	return 0;
}