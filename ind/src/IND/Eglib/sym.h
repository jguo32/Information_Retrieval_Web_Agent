
typedef union  {
	Context  *tk_context;
	int	 tk_int;
	char 	 *tk_string;
	} YYSTYPE;
extern YYSTYPE yylval;
# define IDENTIFIER 257
# define STRING 258
# define CONTINUOUS 259
# define AND 260
# define ONLYIF 261
# define NEVER 262
# define CONTEXTS 263
# define STRATIFY 264
# define UTILITIES 265
# define ASFOR 266
# define PRIOR 267
