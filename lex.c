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
	T_RSH_EQ,
	T_DOT_DOT_DOT
};

enum states {
	S_INITIAL = 0,
	S_SPACE,
	S_ID,
	S_INT,
	S_FLOAT,
	S_SC_OP,
	S_CHAR,
	S_STRING,
	S_EQ,
	S_EQUABLE,
	S_PLUS,
	S_MINUS,
	S_STAR,
	S_SLASH,
	S_AND,
	S_OR,
	S_GREATER,
	S_LESSER,
	S_SL_COMMENT,
	S_SL_COMMENT_END,
	S_ML_COMMENT,
	S_ML_COMMENT_END,
	S_DOT,
	S_DOT_DOT,
	S_LSH,
	S_RSH,

	S_FINAL_ID,
	S_END_OF_FILE,
	S_ERROR,
	S_FINAL_INT,
	S_FINAL_FLOAT,
	S_FINAL_STRING,
	S_FINAL_CHAR,
	S_FINAL_SC,
	S_FINAL_TC,

	S_DOT_DOT_DOT,
	S_LSH_EQUALS,
	S_RSH_EQUALS,
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
	EC_GREATER = 15,
	EC_LESSER = 16,
	EC_QUOTE = 17,
	EC_DOUBLE_QUOTES = 18,
};

const u8 equivalence_class[] = {
	EC_NULL, 0, 0, 0, 0, 0, 0, 0,
	0, EC_SPACE, EC_LF, 0, 0, EC_SPACE, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	EC_SPACE, EC_EQUABLE, EC_DOUBLE_QUOTES, EC_SC_OP, 0, EC_EQUABLE, EC_AND, EC_QUOTE,
	EC_SC_OP, EC_SC_OP, EC_STAR, EC_PLUS, EC_SC_OP, EC_MINUS, EC_DOT, EC_SLASH,
	EC_DIGIT, EC_DIGIT, EC_DIGIT, EC_DIGIT, EC_DIGIT, EC_DIGIT, EC_DIGIT, EC_DIGIT,
	EC_DIGIT, EC_DIGIT, EC_SC_OP, EC_SC_OP, EC_LESSER, EC_EQUALS, EC_GREATER, EC_SC_OP,
	0, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA,
	EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA,
	EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA,
	EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_SC_OP, EC_SPACE, EC_SC_OP, EC_EQUABLE, EC_ALPHA,
	0, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA,
	EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA,
	EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_ALPHA,
	EC_ALPHA, EC_ALPHA, EC_ALPHA, EC_SC_OP, EC_OR, EC_SC_OP, EC_EQUABLE, 0,
};

/* transition table from [state][equivalence class] -> new state */
const u8 transitions[26][20] = {
	/* initial */
	{ S_END_OF_FILE, S_SPACE, S_SPACE, S_ID, S_INT, S_DOT, S_SC_OP, S_EQUABLE, S_EQ, S_PLUS,
	  S_MINUS, S_STAR, S_SLASH, S_AND, S_OR, S_GREATER, S_LESSER, S_CHAR, S_STRING},
	/* space */
	{ S_END_OF_FILE, S_SPACE, S_SPACE, S_ID, S_INT, S_DOT, S_SC_OP,  S_EQUABLE, S_EQ, S_PLUS,
	  S_MINUS, S_STAR, S_SLASH, S_AND, S_OR, S_GREATER, S_LESSER, S_CHAR, S_STRING},
	/* id */
	{ S_FINAL_ID, S_FINAL_ID, S_FINAL_ID, S_ID, S_ID, S_FINAL_ID, S_FINAL_ID, S_FINAL_ID, S_FINAL_ID, S_FINAL_ID,
	  S_FINAL_ID, S_FINAL_ID, S_FINAL_ID, S_FINAL_ID, S_FINAL_ID, S_FINAL_ID, S_FINAL_ID, S_FINAL_ID, S_FINAL_ID},
	/* int */
	{ S_FINAL_INT, S_FINAL_INT, S_FINAL_INT, S_FINAL_INT, S_INT, S_FLOAT, S_FINAL_INT, S_FINAL_INT, S_FINAL_INT, S_FINAL_INT,
	  S_FINAL_INT, S_FINAL_INT, S_FINAL_INT, S_FINAL_INT, S_FINAL_INT, S_FINAL_INT, S_FINAL_INT, S_FINAL_INT, S_FINAL_INT},
	/* float */
	{ S_FINAL_FLOAT, S_FINAL_FLOAT, S_FINAL_FLOAT, S_FINAL_FLOAT, S_FLOAT,
	  S_ERROR, S_FINAL_FLOAT, S_FINAL_FLOAT, S_FINAL_FLOAT, S_FINAL_FLOAT,
	  S_FINAL_FLOAT, S_FINAL_FLOAT, S_FINAL_FLOAT, S_FINAL_FLOAT, S_FINAL_FLOAT,
	  S_FINAL_FLOAT, S_FINAL_FLOAT, S_FINAL_FLOAT, S_FINAL_FLOAT},
	/* single char operator */
	{ S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC,
	  S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC},
	  /* char */
	{ S_ERROR, S_CHAR, S_ERROR, S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR,
	  S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_CHAR, S_FINAL_CHAR, S_CHAR},
	/* string */
	{ S_FINAL_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING,
	  S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_STRING, S_FINAL_STRING},
	/* equals */
	{ S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_TC, S_FINAL_SC,
	  S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC},
	/* equable */
	{ S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_TC, S_FINAL_SC,
	  S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC},
	  /* plus */
	{ S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_TC, S_FINAL_TC,
	  S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC},
	  /* minus */
	{ S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_TC, S_FINAL_SC,
	  S_FINAL_TC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_TC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC},
	  /* star: */
	{ S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_TC, S_FINAL_SC,
	  S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC},
	  /* slash */
	{ S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_TC, S_FINAL_SC,
	  S_FINAL_SC, S_ML_COMMENT, S_SL_COMMENT, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC},
	  /* and */
	{ S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_TC, S_FINAL_SC,
	  S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_TC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC},
	  /* or */
	{ S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_TC, S_FINAL_SC,
	  S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_TC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC},
	  /* greater */
	{ S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_TC, S_FINAL_SC,
	  S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_RSH, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC},
	  /* lesser */
	{ S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_TC, S_FINAL_SC,
	  S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_LSH, S_FINAL_SC, S_FINAL_SC},
	  /* single line comment */
	{ S_END_OF_FILE, S_SL_COMMENT, S_SL_COMMENT_END, S_SL_COMMENT, S_SL_COMMENT, S_SL_COMMENT, S_SL_COMMENT, S_SL_COMMENT, S_SL_COMMENT, S_SL_COMMENT,
	  S_SL_COMMENT, S_SL_COMMENT, S_SL_COMMENT, S_SL_COMMENT, S_SL_COMMENT, S_SL_COMMENT, S_SL_COMMENT, S_SL_COMMENT, S_SL_COMMENT},
	  /* single line comment end */
	{ S_END_OF_FILE, S_SPACE, S_SPACE, S_SPACE, S_SPACE, S_SPACE, S_SPACE, S_SPACE, S_SPACE, S_SPACE,
	  S_SPACE, S_SPACE, S_SPACE, S_SPACE, S_SPACE, S_SPACE, S_SPACE, S_SPACE, S_SPACE},
	  /* multi line comment */
	{ S_END_OF_FILE, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT,
	  S_ML_COMMENT, S_ML_COMMENT_END, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT},
	  /* multi line comment possible end */
	{ S_END_OF_FILE, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT,
	  S_ML_COMMENT, S_ML_COMMENT, S_SPACE, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT, S_ML_COMMENT},
	/* dot */
	{ S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_DOT_DOT, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC,
	  S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC, S_FINAL_SC},
	  /* dot dot */
	{ S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_DOT_DOT_DOT, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC,
	  S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC},
	  /* left shift */
	{ S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_LSH_EQUALS, S_FINAL_TC,
	  S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC},
	  /* right shift */
	{ S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_RSH_EQUALS, S_FINAL_TC,
	  S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC, S_FINAL_TC},
};


const s8 in_token[] = {
	0, 0, 1, 1,
	1, 1, 1, 1,
	1, 1, 1, 1,
	1, 1, 1, 1,
	1, 1, 0, -1,
	0, -1, 1, 1,
	1, 1, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 1,
	1, 1
};

const u8 state_to_token_type[] = {
	T_IDENTIFIER,
	T_END_OF_FILE,
	T_ERROR,
	T_INT_LITERAL,
	T_FLOAT_LITERAL,
	T_STRING,
	T_CHAR,
	T_SINGLE_CHAR_OPERATOR,
	T_TWO_CHAR_OPERATOR,
	T_DOT_DOT_DOT,
	T_LSH_EQ,
	T_RSH_EQ
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

	do {
		u8 c = *p++;
		u8 eq_cls = equivalence_class[c];
		state = transitions[state][eq_cls];
		token_len += in_token[state];
	} while (state < S_FINAL_ID);

	char *token_start = p - token_len - 1;

	Token tk = {0};
	tk.type = state_to_token_type[state - S_FINAL_ID];

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
		/* skip closing ' */
		p++;
	} break;
	case T_STRING: {
		tk.lexeme = cstring(token_start + 1, token_len - 1);
		/* skip closing " */
		p++;
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
	case T_DOT_DOT_DOT:
	case T_LSH_EQ:
	case T_RSH_EQ:
		p++;
		break;
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
	case T_DOT_DOT_DOT: return "dot dot dot";
	default: "";
	}

	return "";
}

void print_token(Token token) {
	switch (token.type) {
	case T_INT_LITERAL:
		printf("%s: %lld\n", token_type_to_str(token.type), token.int_value);
		break;
	case T_FLOAT_LITERAL:
		printf("%s: %f\n", token_type_to_str(token.type), token.float_value);
		break;
	case T_IDENTIFIER:
	case T_STRING:
		printf("%s: '%.*s'\n", token_type_to_str(token.type), (u32)token.lexeme.length, token.lexeme.ptr);
		break;
	case T_KEYWORD:
		printf("%s: %d\n", token_type_to_str(token.type), token.keyword);
		break;
	case T_CHAR:
		printf("%s: '%c'\n", token_type_to_str(token.type), token.int_value);
		break;
	default:
		if ((token.type >= T_EQ_EQ && token.type <= T_DOT_DOT_DOT) || token.type == T_ERROR) {
			printf("%s\n", token_type_to_str(token.type));
		} else {
			printf("%c\n", token.type);
		}
		break;
	}
}

char *read_entire_file(const char *file_name) {
	FILE *f = fopen(file_name, "rb");

	fseek(f, 0, SEEK_END);
	size_t len = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *text = malloc(len + 1);
	fread(text, 1, len, f);
	text[len] = 0;

	fclose(f);

	return text;
}

s32 main() {
	char *input = read_entire_file("E:\\Development\\c\\crow\\crow\\lex.c");

	LexerContext ctx;
	ctx.current = input;

	size_t num_tokens = 0;

	Token token;
	while ((token = lex(&ctx)).type != T_END_OF_FILE) {
		print_token(token);
	}


	return 0;
}