#define ARRAY 257
#define ASSIGNMENT 258
#define CASE 259
#define COLON 260
#define COMMA 261
#define CONST 262
#define DO 263
#define DOT 264
#define DOTDOT 265
#define DOWNTO 266
#define ELSE 267
#define END 268
#define EXTERNAL 269
#define BFALSE 270
#define FOR 271
#define FORWARD 272
#define FUNCTION 273
#define GOTO 274
#define IF 275
#define LBRAC 276
#define LPAREN 277
#define MINUS 278
#define NIL 279
#define NOT 280
#define OF 281
#define OR 282
#define OTHERWISE 283
#define PACKED 284
#define PBEGIN 285
#define PFILE 286
#define PLUS 287
#define PROCEDURE 288
#define PROGRAM 289
#define RBRAC 290
#define RECORD 291
#define REPEAT 292
#define RPAREN 293
#define SEMICOLON 294
#define SET 295
#define SLASH 296
#define STAR 297
#define STARSTAR 298
#define THEN 299
#define TO 300
#define BTRUE 301
#define TYPE 302
#define UNTIL 303
#define VAR 304
#define WHILE 305
#define WITH 306
#define IDENTIFIER 307
#define CHARACTER_STRING 308
#define UPARROW 309
#define DIGSEQ 310
#define LABEL 311
#define REALNUMBER 312
#define EQUAL 313
#define NOTEQUAL 314
#define LT 315
#define GT 316
#define LE 317
#define GE 318
#define IN 319
#define DIV 320
#define MOD 321
#define AND 322
#define EQ 323
#define NE 324
#define UMINUS 325
#define UPLUS 326
#define HIGHER 327
#define LOWER 328
typedef union {
  int   IntVal;
  struct Token
  {
     char* tokenStart;
     int   tokenLength;
  } Text;  
  int ObjRef;
  int RelOp;
  int ArithmeticOp;
  double RealVal;
} YYSTYPE;
extern YYSTYPE yylval;
