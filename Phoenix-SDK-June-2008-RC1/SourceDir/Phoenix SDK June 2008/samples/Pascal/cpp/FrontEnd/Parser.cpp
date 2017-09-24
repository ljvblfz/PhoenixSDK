#ifndef lint
/*static char yysccsid[] = "from: @(#)yaccpar	1.9 (Berkeley) 02/21/93";*/
static char yyrcsid[] = "$Id: skeleton.c,v 1.2 1997/06/23 02:51:17 tdukes Exp $";
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define YYPREFIX "yy"
/*
 * pascal.y
 *
 * Pascal grammar in Yacc format, based originally on BNF given
 * in "Standard Pascal -- User Reference Manual", by Doug Cooper.
 * This in turn is the BNF given by the ANSI and ISO Pascal standards,
 * and so, is PUBLIC DOMAIN. The grammar is for ISO Level 0 Pascal.
 * The grammar has been massaged somewhat to make it LALR, and added
 * the following extensions.
 *
 * constant expressions
 * otherwise statement in a case
 * productions to correctly match else's with if's
 * beginnings of a separate compilation facility
 *
 */
#pragma once

#include "stdafx.h"

#include <process.h>
#include <stdio.h>
 
/* Scanner interface.*/
#include "Scanner.h"

/* Parser interface.*/
#include "Parser.h"

/* Include definition of abstract class Node.*/
#include "Ast.h"

extern int   LineNum; /* current line in the scanner*/
extern char* yytext;  /* current token in the scanner*/
extern "C" int yyparse();

/* Short-hand for accessing the managed AST nodes.*/
static inline Node^ AstNode(int nodeRef)
{
   return (nodeRef == -1) ? nullptr : ParserGCRoots::astNodes[nodeRef];
}

/* Short-hand for accessing AST node lists.*/
static inline List<Node^>^ AstNodeList(int listRef)
{
   return (listRef == -1) ? nullptr : ParserGCRoots::astNodeLists[listRef];
}

/* Create a node reference for use in native datatypes, in particular in the union and array of unions*/
/* that implement $$ and $i.*/
static inline int AddNode(Node^ newNode)
{
   ParserGCRoots::astNodes->Add(newNode);
   return ParserGCRoots::astNodes->Count - 1;
}

/* Create a nodelist reference for use in yacc's native datatypes.*/
static inline int AddNodeList(int nodeRef =-1)
{
   /* Create a new list and record it in the GC roots.*/
   List<Node^>^ newNodeList = gcnew List<Node^>();
   ParserGCRoots::astNodeLists->Add(newNodeList);

   /* If a nodeRef was passed, add the node to the new list.*/
   if (nodeRef != -1)
   {
      newNodeList->Add(ParserGCRoots::astNodes[nodeRef]);
   }
   return ParserGCRoots::astNodeLists->Count - 1;
}

/* Some yacc implementations require yyerror to be declared.*/
void yyerror(char*);

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
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,    1,    3,    3,    6,    6,    9,    4,    2,
    7,    7,   36,   36,   35,    8,    8,   14,   14,   15,
   18,   18,   19,   19,   20,   20,   21,   21,   22,   22,
   23,   23,   23,   23,   42,   42,   42,   34,   34,   43,
   43,   43,   13,   13,  110,  110,  111,  138,  138,   92,
   92,   92,   94,   94,   95,   96,   98,   98,   97,   97,
   97,   97,   99,  106,  106,  105,   93,   93,  104,  100,
  100,  100,  107,  107,  108,  109,  109,  109,  112,  112,
  113,  113,  114,  114,  114,  117,  117,  118,  118,  115,
  116,  101,  121,  102,  103,  122,   11,   11,   12,   12,
   10,   10,   16,   16,   72,   72,   73,   73,   81,   81,
   82,   82,   90,   90,   74,   76,   76,   75,   75,   75,
   75,   77,   78,   79,   80,  124,   83,   84,   84,   84,
   85,   85,   91,   87,   86,   17,   44,   45,   45,   46,
   46,   47,   47,   48,   48,   50,   50,   50,   50,   50,
   50,   50,   50,   50,   50,   50,   49,   49,   49,   49,
   63,   57,   53,   58,   54,   55,   51,   56,   56,   52,
   59,   64,   64,   64,   64,  125,  127,  127,  128,  126,
   60,   60,  129,  130,  130,  131,  131,  131,   61,   62,
   62,   62,   62,  119,  123,  123,  120,  132,  132,   67,
   68,   70,   70,   69,   71,   71,   30,   24,   24,   25,
   25,   26,   26,   27,   27,   28,   28,   29,   29,   29,
   29,   29,   29,   38,   38,   38,   38,   39,   39,   41,
   40,   37,   37,   65,   66,   66,  134,  134,  135,  135,
   31,   31,   31,   32,   32,   32,   32,   32,   33,   33,
   33,   33,   33,   33,   33,    5,   89,   88,  136,  137,
  133,
};
short yylen[] = {                                         2,
    1,    1,    4,    2,    5,    3,    1,    1,    6,    4,
    3,    0,    3,    1,    1,    2,    0,    2,    1,    4,
    1,    3,    1,    3,    1,    3,    2,    1,    1,    3,
    1,    3,    1,    2,    1,    2,    1,    1,    1,    1,
    1,    1,    2,    0,    2,    1,    4,    1,    1,    1,
    1,    1,    1,    1,    3,    3,    1,    2,    1,    1,
    1,    1,    6,    3,    1,    1,    1,    1,    1,    3,
    5,    3,    3,    1,    3,    5,    4,    0,    3,    1,
    3,    1,    5,    7,    5,    3,    1,    1,    3,    1,
    1,    3,    1,    3,    2,    1,    3,    0,    3,    1,
    3,    3,    2,    0,    3,    1,    1,    1,    3,    3,
    1,    2,    1,    1,    3,    3,    1,    1,    1,    1,
    1,    3,    4,    1,    1,    2,    1,    3,    3,    3,
    4,    5,    1,    2,    1,    1,    3,    3,    1,    1,
    1,    3,    1,    3,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    0,    1,    1,    1,    1,
    4,    4,    4,    8,    8,    4,    4,    4,    6,    6,
    3,    1,    1,    1,    2,    4,    3,    1,    1,    3,
    2,    1,    3,    3,    1,    1,    3,    5,    2,    5,
    6,    8,    9,    1,    3,    1,    3,    1,    2,    1,
    1,    1,    1,    1,    3,    1,    1,    1,    3,    1,
    3,    1,    3,    2,    1,    1,    3,    1,    1,    1,
    1,    3,    2,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    2,    3,    2,    3,    1,    3,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,
};
short yydefred[] = {                                      0,
    0,    0,    0,    1,    2,    0,    0,  256,    0,    0,
   19,    0,  257,    0,    0,    0,    0,   18,    0,    0,
    0,    0,    0,    0,   46,  260,    0,    0,  233,    0,
   39,  261,    0,   38,  232,  225,  230,  231,   31,    0,
    0,    0,   25,   28,    0,    0,  227,   33,  224,  229,
  228,  226,    8,    0,    7,   15,   14,    0,    3,    0,
    0,   45,    0,    0,   10,    0,  106,  107,    0,  108,
    0,    0,    0,    0,    0,  100,    0,    0,   34,   20,
  242,  243,  241,  249,  250,  251,  252,  253,  254,  255,
    0,    0,  245,  244,  246,  247,  248,    0,    0,   27,
  259,    5,    0,   11,    0,    0,    0,    0,    0,    0,
    0,    0,   37,    0,   40,   42,    0,    0,    0,   35,
   49,   50,   53,   54,   57,   51,   59,   60,   61,   62,
   52,    0,    0,  126,    0,    0,    0,    0,    0,  112,
    0,    0,    0,   32,    0,    0,   26,   30,    6,   13,
    0,    0,    0,   58,    0,    0,    0,    0,   74,    0,
    0,   96,   95,   41,   36,    0,   47,    0,    0,  105,
  114,  113,  127,  110,  109,  135,  130,  128,  129,    0,
    0,    0,  117,    0,  118,  119,  120,  121,  124,  125,
  101,  102,   99,    0,    0,   66,   67,   65,    0,   55,
   94,   69,    0,    0,    0,   80,  258,    0,   70,    0,
   72,   93,   92,   56,  133,  131,    0,    0,    0,    0,
  115,    0,    0,    9,  136,    0,    0,    0,    0,   75,
   73,    0,  132,    0,  122,  116,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  149,    0,  139,  140,  141,
  143,  145,  152,  153,  154,  155,  157,  158,  159,  160,
  146,  147,  148,  150,  151,    0,  173,  174,    0,   64,
    0,    0,   82,    0,   87,   91,   79,   71,  123,    0,
    0,    0,    0,  194,    0,    0,  212,  215,    0,    0,
  219,    0,  220,  221,    0,  200,    0,  189,  207,    0,
    0,    0,  172,    0,    0,    0,  181,    0,  137,    0,
    0,    0,    0,  175,   63,    0,    0,    0,    0,  236,
    0,    0,  238,    0,  223,  234,    0,    0,    0,    0,
  214,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  185,  142,  144,  138,  171,  180,  179,    0,  178,   89,
   81,    0,   86,    0,  235,    0,  222,    0,    0,  213,
  217,    0,  196,    0,  201,    0,  168,    0,  161,  162,
  163,  166,  167,    0,    0,  183,    0,  176,    0,    0,
    0,  239,  237,    0,  190,    0,    0,  203,  202,    0,
    0,    0,  184,  177,   83,    0,   85,  197,  191,    0,
    0,  195,  204,    0,  169,  170,    0,    0,  199,    0,
    0,  188,   84,  192,    0,  164,  165,  193,
};
short yydgoto[] = {                                       3,
    4,    5,    6,  176,  283,  157,   22,    7,   55,   76,
   27,   77,   16,   10,   11,   65,  224,   40,   41,   42,
   43,   44,   45,  299,  285,  286,  287,  288,  289,  300,
   91,   98,   92,  290,  245,   58,   47,  291,   49,   50,
   51,  119,  120,  246,  247,  248,  249,  250,  251,  252,
  253,  254,  255,  256,  257,  258,  259,  260,  261,  262,
  263,  264,  265,  292,  293,  294,  297,  366,  404,  390,
  305,   66,   67,  169,  183,  184,  185,  186,  187,  188,
   68,   69,  174,   70,   71,  177,   72,  208,  310,  175,
  216,  121,  196,  122,  123,  124,  125,  126,  127,  128,
  129,  130,  131,  201,  198,  199,  158,  159,  160,   24,
   25,  204,  272,  273,  205,  206,  274,  275,  295,  363,
  213,  163,  364,   73,  267,  268,  348,  349,  307,  340,
  341,  401,   52,  322,  323,  103,   28,  202,
};
short yysindex[] = {                                   -209,
 -249, -249,    0,    0,    0, -201, -231,    0, -178, -249,
    0, -138,    0, -100, -249, -102,  648,    0, -249,  -48,
  -47,   15,  -67, -249,    0,    0, -129, -249,    0,  648,
    0,    0,  219,    0,    0,    0,    0,    0,    0, -201,
  666,  212,    0,    0,  -20,  648,    0,    0,    0,    0,
    0,    0,    0, -187,    0,    0,    0, -166,    0, -231,
  439,    0, -249, -249,    0, -201,    0,    0, -201,    0,
 -201, -201,  -22,   22,   43,    0, -201,   -4,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  648,  648,    0,    0,    0,    0,    0,  648,  219,    0,
    0,    0, -249,    0,  -48, -102,   23, -249, -208,   32,
 -223,   38,    0, -249,    0,    0,    0,  -59,   61,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0, -201,   -6,    0, -129, -193, -193, -100,  -32,    0,
  439,  439, -249,    0,  212,   47,    0,    0,    0,    0,
 -129,  458, -140,    0,  439, -249,   62,  -74,    0,   64,
  458,    0,    0,    0,    0,  -43,    0, -249,   78,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -249,
 -249,   83,    0,   52,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   70,    0,    0,    0,    0, -218,    0,
    0,    0,    0,   80,  112,    0,    0,  439,    0, -223,
    0,    0,    0,    0,    0,    0, -249,   -6,   99, -249,
    0,  -32,  484,    0,    0,   93,  458,  -43, -249,    0,
    0,   96,    0, -249,    0,    0,  622, -249,  -48,  622,
  484,  622, -249,  117,  138,    0,  -69,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0, -139,    0,    0,  439,    0,
  144, -201,    0,  109,    0,    0,    0,    0,    0,  607,
  622,  105,  117,    0,  666,  212,    0,    0,  149,  622,
    0, -210,    0,    0,  120,    0,  145,    0,    0,  125,
 -244,  171,    0, -210, -206,  622,    0,  503,    0,  484,
  622, -249,  622,    0,    0,  -43,  -43,  159,  -43,    0,
  183, -115,    0,  158,    0,    0,  622,  622,  622,  105,
    0,  -43,  622,  484,  622,  484,  484, -249,  193, -103,
    0,    0,    0,    0,    0,    0,    0, -109,    0,    0,
    0, -223,    0,  622,    0,  622,    0,  212,   47,    0,
    0,  119,    0,  -64,    0, -215,    0,  187,    0,    0,
    0,    0,    0, -210,  622,    0,  622,    0,  622,  126,
  164,    0,    0,  484,    0,  -75,  -43,    0,    0,  622,
  484,  203,    0,    0,    0, -223,    0,    0,    0,  204,
  484,    0,    0,  205,    0,    0,  622,  173,    0,  -58,
  484,    0,    0,    0,  201,    0,    0,    0,
};
short yyrindex[] = {                                     10,
    0,    0,    0,    0,    0,    0,   13,    0,    0,    6,
    0,  176,    0,  103,    0,    7,    0,    0,    0,    0,
    0,  -28,    0,   17,    0,    0,  472,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  139,  280,    0,    0,  505,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  104,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  148,  213,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  114,    0,    0,    0,    0,
  226,    0,    0,    0,    0,    0,   74,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  179,    0,    3,  103,  103,  103,    0,    0,
    0,    0,    8,    0,  658,  152,    0,    0,    0,    0,
  202,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   59,    0,    0,    0,    0,    0,
    0,    0, -136,    0,    0,    0,    0,    0,    0,  226,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  -21,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  -91,    0,    0,  -80,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  136,  -62,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  162,    0,  568,  351,    0,    0,  288,    0,
    0,  225,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  -11,    0,    0,    0, -118,    0, -137,
    0,    0,    0,    0,    0,    0,  -55,    0,    0,    0,
  -66,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0, -118,    0, -118, -118,    0,  -93,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  207,    0,    0,    0,    0,    0,  395,  579,    0,
    0,    0,    0,    0,    0,    0,    0, -105,    0,    0,
    0,    0,    0,   24,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  -21,    0,  393,    0,    0,    0,    0,
 -118,  -92,    0,    0,    0,  207,    0,    0,    0,  447,
  -21,    0,    0,    0,    0,    0,    0,    0,    0,    0,
 -118,    0,    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
    0,    0,    0,   12,   -1,   -7,    0,  473,  394,  358,
  396,    0,  444,    0,  495,  359,    0,  481,  421,  423,
  -16,  417,  497,   60,  189,  208, -243,  206,  255, -220,
 -239, -246,  265,   31,   -5,    0,    0,   35,    0,    0,
    0, -155,  434,  363,  318, -290, -302, -295,  252,  253,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   20,    0,    0,    0,    0,    0,    0,
    0,    0,  428,  491,  343,    0,    0,    0,    0,    0,
    0, -114,    0,    0, -108,  430,    0,    0,   -2,  435,
  354,    0,  415, -124,    0,    0,  468,    0,    0,    0,
    0,    0,    0,  310,  353,    0,  231, -191, -205,    0,
  562,    0,    0,  272,    0,  361, -299,  273,    0,  228,
    0,    0,    0,    0,    0,    0,    0,  221,  330,    0,
  243,    0,    0,    0,  266,  -56,    0,  -37,
};
#define YYTABLESIZE 985
short yytable[] = {                                       9,
   12,  105,  103,   14,  232,   16,   98,   97,    9,   17,
  214,   54,   44,   23,   57,   39,   43,   53,  231,  344,
   75,  302,   23,  132,  189,   21,   74,  197,   39,  100,
  190,   39,  362,  370,  372,  156,  197,   80,  368,  329,
  371,  373,  101,  367,   39,  327,  331,   46,  107,   13,
  388,   48,    1,  312,  101,  104,  337,    8,  335,  117,
   46,  133,  134,  135,   48,  313,  136,   48,  137,  138,
   15,  226,  271,  101,  143,  171,   46,  110,  172,    2,
   48,  147,  111,    8,  389,  360,  112,  362,  405,   39,
   39,  118,   13,  398,  101,  406,   39,   39,  314,  150,
  153,   53,  197,  191,  192,  102,   53,  189,  416,   53,
  410,  329,  162,  190,  369,  417,  164,   20,  311,  327,
  101,   46,   46,   90,  312,   48,   48,   13,   46,  167,
  156,  182,   48,   48,   17,   75,  313,   53,   19,  117,
  117,   74,  227,   63,   91,  101,  381,  173,  156,  156,
  195,  101,  200,  117,  203,  210,  156,  101,   64,  195,
  350,  271,  141,  271,  164,  156,  215,  186,  187,  314,
  230,  118,  118,  219,  355,  156,  271,  172,  218,   53,
  378,  222,  118,  172,  156,  118,  182,  182,  141,  376,
  408,  118,  399,  209,  240,  172,  118,  141,  309,  186,
  187,   26,  156,  385,  231,   77,  117,  400,   53,  414,
   20,  156,   76,  182,  182,  215,   59,  319,  235,   13,
   53,  244,  182,  240,   13,  195,  164,  276,  172,  386,
   77,  271,  279,  298,   31,  415,  296,   76,  118,  244,
  180,  303,  266,   34,   17,   61,  156,    8,  338,  206,
  115,  206,  116,  168,  139,   64,   17,  118,  118,   17,
  266,   56,  304,    8,  113,  356,  115,  117,  116,  317,
  139,  181,  156,   17,    8,   17,    1,   99,   16,   98,
   97,  141,   17,  377,  205,   44,  205,  103,  144,   43,
   16,  379,   97,   16,   98,   97,  284,   17,  152,  118,
   44,   43,  142,  101,   43,  319,  244,   16,  244,   16,
  346,   17,  155,   17,  164,  164,   44,  164,  161,   68,
   43,  207,  101,   41,   81,  166,   68,  266,   82,  266,
  164,  211,  244,   83,  244,  244,  303,  217,   41,  321,
  324,   48,  220,  101,  221,   13,  118,  118,   68,  118,
   53,   68,   68,  266,  223,  266,  266,  374,  234,  101,
  228,  387,  118,  278,   12,  339,   48,   48,  318,  101,
  345,  229,  347,  269,   29,   12,   44,  396,  384,  101,
  280,  281,  244,   32,  282,  164,   98,   12,   44,  244,
   12,   44,  365,  306,   53,   88,   88,  308,   98,  244,
  332,   98,  333,  266,   12,   35,   12,   44,  316,  244,
  266,    8,   36,  382,   37,  321,   38,  118,  395,   13,
  266,  172,  172,  334,  172,  172,  172,  172,  172,  172,
  266,   21,   21,  336,  392,  352,  339,  172,  347,  172,
  111,  111,  172,  172,   22,   22,  330,  354,  172,  403,
  357,  172,  375,  391,  172,  172,  397,  172,  172,  172,
  172,  172,  407,  409,  172,  413,  412,  411,  418,    4,
  172,  104,  134,    8,  172,  172,  172,  172,  172,  172,
  172,  172,  172,  172,  218,  218,  104,  218,   29,  218,
  218,  218,  218,   78,   60,   30,  149,   32,   33,   78,
  193,  151,  218,  106,   18,  218,  218,   93,   94,  194,
   78,  218,  146,  145,  218,  148,  359,  218,  218,   35,
  218,  218,  218,  218,  218,    8,   36,  218,   37,   79,
   38,   95,   96,   97,  358,  361,  325,  218,  218,  218,
  218,  218,  218,  218,  218,  218,  218,  216,  216,  328,
  216,  165,  216,  216,  216,  216,  225,   23,  301,  342,
  343,   23,  170,  140,  236,  216,   23,  179,  216,  216,
  233,  178,   23,   23,  216,  212,  154,  216,  315,  270,
  216,  216,  380,  216,  216,   62,  216,  216,  351,  277,
  216,  353,   23,   23,   23,   23,   23,   23,   23,  394,
  216,  216,  216,  216,  216,  216,  216,  216,  216,  216,
  210,  210,  326,  210,  402,  210,  210,  210,  210,  393,
    0,  383,    0,    0,    0,    0,    0,    0,  210,    0,
    0,  210,  210,    0,    0,    0,    0,  210,    0,    0,
  210,    0,    0,  210,  210,    0,    0,    0,    0,  210,
  210,    0,    0,  210,  211,  211,    0,  211,    0,  211,
  211,  211,  211,  210,  210,  210,  210,  210,  210,  210,
  257,    0,  211,    0,    0,  211,  211,    0,    0,  257,
    0,  211,    0,    0,  211,    0,    0,  211,  211,    0,
    0,    0,    0,  211,  211,  107,    0,  211,    0,  257,
  257,    0,  257,    0,  257,  198,    0,  211,  211,  211,
  211,  211,  211,  211,  198,  108,   31,  198,    0,    0,
  198,  198,  109,    0,  110,   34,    0,    0,    0,  111,
    0,  198,    0,  112,  108,   31,    0,    0,  198,    0,
  198,    0,  237,    0,   34,    8,  113,  114,  115,    0,
  116,  198,  198,  198,  238,    0,  198,  239,  240,    0,
    0,  237,    0,    0,    8,  113,    0,  115,  223,  116,
    0,    0,    0,  238,    0,  241,  239,  240,    0,    0,
    0,    0,   29,    0,    0,    0,   29,  223,  242,  243,
    8,   29,    0,   56,  241,    0,    0,   29,   29,    0,
   29,   29,    0,    0,    0,    0,    0,  242,  243,    8,
    0,    0,    0,    0,    0,    0,    0,   29,   29,   29,
   29,   29,   29,   29,   29,   29,   29,  208,  208,    0,
  208,    0,  208,  208,  208,  208,    0,    0,  209,  209,
    0,  209,    0,  209,  209,  209,  209,    0,  208,    0,
    0,    0,    0,    0,    0,    0,    0,  208,    0,  209,
  208,  208,    0,    0,    0,    0,  208,  208,  209,    0,
  208,  209,  209,    0,    0,    0,   29,  209,  209,    0,
    0,  209,  280,  281,   31,   32,  282,    0,    0,    0,
    0,   29,    0,   34,    0,    0,  320,  280,  281,   31,
   32,  282,    0,    0,    0,    0,    0,   35,   34,    0,
    0,    0,    0,    8,   36,    0,   37,   29,   38,    0,
    0,    0,   35,    0,   30,   31,   32,   33,    8,   36,
    0,   37,    0,   38,   34,   24,    0,    0,    0,   24,
    0,    0,    0,   81,   24,    0,    0,   82,   35,    0,
   24,   24,   83,    0,    8,   36,    0,   37,    0,   38,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   24,   24,   24,   24,   24,   24,   24,    0,   84,   85,
   86,   87,   88,   89,   90,
};
short yycheck[] = {                                       1,
    2,   58,    0,    6,  210,    0,    0,    0,   10,    0,
  166,   19,    0,   15,   20,   17,    0,   19,  210,  310,
   28,  242,   24,   61,  139,   14,   28,  152,   30,   46,
  139,   33,  332,  336,  337,  259,  161,   40,  334,  286,
  336,  337,  261,  334,   46,  285,  290,   17,  257,  294,
  266,   17,  262,  264,  261,   58,  263,  307,  303,   61,
   30,   63,   64,   66,   30,  276,   69,   33,   71,   72,
  302,  290,  228,  261,   77,  269,   46,  286,  272,  289,
   46,   98,  291,  307,  300,  329,  295,  387,  391,   91,
   92,   61,  294,  384,  261,  391,   98,   99,  309,  105,
  108,  103,  227,  141,  142,  293,  108,  222,  411,  111,
  401,  358,  114,  222,  335,  411,  118,  311,  258,  359,
  261,   91,   92,  260,  264,   91,   92,  294,   98,  132,
  268,  139,   98,   99,  313,  143,  276,  139,  277,  141,
  142,  143,  199,  273,  281,  261,  352,  136,  267,  268,
  152,  261,  293,  155,  156,  158,  294,  261,  288,  161,
  316,  317,  268,  319,  166,  303,  168,  261,  261,  309,
  208,  141,  142,  181,  290,  294,  332,  258,  180,  181,
  290,  184,  152,  264,  303,  155,  267,  268,  294,  293,
  396,  161,  268,  268,  261,  276,  166,  303,  268,  293,
  293,  304,  294,  268,  396,  268,  208,  283,  210,  268,
  311,  303,  268,  294,  222,  217,  264,  274,  220,  294,
  222,  223,  303,  290,  294,  227,  228,  229,  309,  294,
  293,  387,  234,  239,  278,  294,  238,  293,  208,  241,
  273,  243,  223,  287,  273,  313,  268,  307,  305,  261,
  310,  263,  312,  260,  277,  288,  285,  227,  228,  288,
  241,  310,  243,  307,  308,  322,  310,  269,  312,  272,
  277,  304,  294,  302,  307,  304,  262,  298,  273,  273,
  273,  260,  273,  340,  261,  273,  263,  285,  293,  273,
  285,  348,  285,  288,  288,  288,  237,  288,  276,  269,
  288,  285,  260,  261,  288,  362,  308,  302,  310,  304,
  312,  302,  281,  304,  316,  317,  304,  319,  281,  261,
  304,  260,  261,  265,  278,  265,  268,  308,  282,  310,
  332,  268,  334,  287,  336,  337,  338,  260,  265,  280,
  281,  268,  260,  261,  293,  294,  316,  317,  290,  319,
  352,  293,  294,  334,  285,  336,  337,  338,  260,  261,
  281,  364,  332,  268,  262,  306,  293,  294,  260,  261,
  311,  260,  313,  281,  270,  273,  273,  380,  260,  261,
  276,  277,  384,  279,  280,  387,  273,  285,  285,  391,
  288,  288,  333,  277,  396,  260,  261,  260,  285,  401,
  281,  288,  258,  384,  302,  301,  304,  304,  265,  411,
  391,  307,  308,  354,  310,  356,  312,  387,  293,  294,
  401,  260,  261,  299,  263,  264,  265,  266,  267,  268,
  411,  293,  294,  263,  375,  277,  377,  276,  379,  278,
  293,  294,  281,  282,  293,  294,  298,  265,  287,  390,
  293,  290,  260,  267,  293,  294,  293,  296,  297,  298,
  299,  300,  260,  260,  303,  293,  407,  263,  268,  294,
  309,    0,  294,  261,  313,  314,  315,  316,  317,  318,
  319,  320,  321,  322,  260,  261,  285,  263,  270,  265,
  266,  267,  268,  268,   22,  277,  103,  279,  280,  293,
  143,  106,  278,   60,   10,  281,  282,  296,  297,  151,
   30,  287,   92,   91,  290,   99,  328,  293,  294,  301,
  296,  297,  298,  299,  300,  307,  308,  303,  310,   33,
  312,  320,  321,  322,  327,  330,  282,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  260,  261,  285,
  263,  118,  265,  266,  267,  268,  194,  278,  241,  308,
  308,  282,  135,   73,  222,  278,  287,  138,  281,  282,
  217,  137,  293,  294,  287,  161,  109,  290,  269,  227,
  293,  294,  352,  296,  297,   24,  299,  300,  317,  229,
  303,  319,  313,  314,  315,  316,  317,  318,  319,  379,
  313,  314,  315,  316,  317,  318,  319,  320,  321,  322,
  260,  261,  283,  263,  387,  265,  266,  267,  268,  377,
   -1,  356,   -1,   -1,   -1,   -1,   -1,   -1,  278,   -1,
   -1,  281,  282,   -1,   -1,   -1,   -1,  287,   -1,   -1,
  290,   -1,   -1,  293,  294,   -1,   -1,   -1,   -1,  299,
  300,   -1,   -1,  303,  260,  261,   -1,  263,   -1,  265,
  266,  267,  268,  313,  314,  315,  316,  317,  318,  319,
  278,   -1,  278,   -1,   -1,  281,  282,   -1,   -1,  287,
   -1,  287,   -1,   -1,  290,   -1,   -1,  293,  294,   -1,
   -1,   -1,   -1,  299,  300,  257,   -1,  303,   -1,  307,
  308,   -1,  310,   -1,  312,  259,   -1,  313,  314,  315,
  316,  317,  318,  319,  268,  277,  278,  271,   -1,   -1,
  274,  275,  284,   -1,  286,  287,   -1,   -1,   -1,  291,
   -1,  285,   -1,  295,  277,  278,   -1,   -1,  292,   -1,
  294,   -1,  259,   -1,  287,  307,  308,  309,  310,   -1,
  312,  305,  306,  307,  271,   -1,  310,  274,  275,   -1,
   -1,  259,   -1,   -1,  307,  308,   -1,  310,  285,  312,
   -1,   -1,   -1,  271,   -1,  292,  274,  275,   -1,   -1,
   -1,   -1,  278,   -1,   -1,   -1,  282,  285,  305,  306,
  307,  287,   -1,  310,  292,   -1,   -1,  293,  294,   -1,
  296,  297,   -1,   -1,   -1,   -1,   -1,  305,  306,  307,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  313,  314,  315,
  316,  317,  318,  319,  320,  321,  322,  260,  261,   -1,
  263,   -1,  265,  266,  267,  268,   -1,   -1,  260,  261,
   -1,  263,   -1,  265,  266,  267,  268,   -1,  281,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  290,   -1,  281,
  293,  294,   -1,   -1,   -1,   -1,  299,  300,  290,   -1,
  303,  293,  294,   -1,   -1,   -1,  270,  299,  300,   -1,
   -1,  303,  276,  277,  278,  279,  280,   -1,   -1,   -1,
   -1,  270,   -1,  287,   -1,   -1,  290,  276,  277,  278,
  279,  280,   -1,   -1,   -1,   -1,   -1,  301,  287,   -1,
   -1,   -1,   -1,  307,  308,   -1,  310,  270,  312,   -1,
   -1,   -1,  301,   -1,  277,  278,  279,  280,  307,  308,
   -1,  310,   -1,  312,  287,  278,   -1,   -1,   -1,  282,
   -1,   -1,   -1,  278,  287,   -1,   -1,  282,  301,   -1,
  293,  294,  287,   -1,  307,  308,   -1,  310,   -1,  312,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
  313,  314,  315,  316,  317,  318,  319,   -1,  313,  314,
  315,  316,  317,  318,  319,
};
#define YYFINAL 3
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 328
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,"'*'","'+'",0,"'-'",0,"'/'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"ARRAY",
"ASSIGNMENT","CASE","COLON","COMMA","CONST","DO","DOT","DOTDOT","DOWNTO","ELSE",
"END","EXTERNAL","BFALSE","FOR","FORWARD","FUNCTION","GOTO","IF","LBRAC",
"LPAREN","MINUS","NIL","NOT","OF","OR","OTHERWISE","PACKED","PBEGIN","PFILE",
"PLUS","PROCEDURE","PROGRAM","RBRAC","RECORD","REPEAT","RPAREN","SEMICOLON",
"SET","SLASH","STAR","STARSTAR","THEN","TO","BTRUE","TYPE","UNTIL","VAR",
"WHILE","WITH","IDENTIFIER","CHARACTER_STRING","UPARROW","DIGSEQ","LABEL",
"REALNUMBER","EQUAL","NOTEQUAL","LT","GT","LE","GE","IN","DIV","MOD","AND","EQ",
"NE","UMINUS","UPLUS","HIGHER","LOWER",
};
char *yyrule[] = {
"$accept : file",
"file : program",
"file : module",
"program : program_heading semicolon block DOT",
"program_heading : PROGRAM identifier",
"program_heading : PROGRAM identifier LPAREN identifier_list RPAREN",
"identifier_list : identifier_list comma formal_parameter",
"identifier_list : formal_parameter",
"formal_parameter : identifier",
"block : label_declaration_part constant_definition_part type_definition_part variable_declaration_part procedure_and_function_declaration_part statement_part",
"module : constant_definition_part type_definition_part variable_declaration_part procedure_and_function_declaration_part",
"label_declaration_part : LABEL label_list semicolon",
"label_declaration_part :",
"label_list : label_list comma label",
"label_list : label",
"label : DIGSEQ",
"constant_definition_part : CONST constant_list",
"constant_definition_part :",
"constant_list : constant_list constant_definition",
"constant_list : constant_definition",
"constant_definition : identifier EQUAL cexpression semicolon",
"cexpression : csimple_expression",
"cexpression : csimple_expression relop csimple_expression",
"csimple_expression : cterm",
"csimple_expression : csimple_expression addop cterm",
"cterm : cfactor",
"cterm : cterm mulop cfactor",
"cfactor : sign cfactor",
"cfactor : cexponentiation",
"cexponentiation : cprimary",
"cexponentiation : cprimary STARSTAR cexponentiation",
"cprimary : identifier",
"cprimary : LPAREN cexpression RPAREN",
"cprimary : unsigned_constant",
"cprimary : NOT cprimary",
"constant : non_string",
"constant : sign non_string",
"constant : CHARACTER_STRING",
"sign : PLUS",
"sign : MINUS",
"non_string : DIGSEQ",
"non_string : identifier",
"non_string : REALNUMBER",
"type_definition_part : TYPE type_definition_list",
"type_definition_part :",
"type_definition_list : type_definition_list type_definition",
"type_definition_list : type_definition",
"type_definition : identifier EQUAL type_denoter semicolon",
"type_denoter : identifier",
"type_denoter : new_type",
"new_type : new_ordinal_type",
"new_type : new_structured_type",
"new_type : new_pointer_type",
"new_ordinal_type : enumerated_type",
"new_ordinal_type : subrange_type",
"enumerated_type : LPAREN identifier_list RPAREN",
"subrange_type : constant DOTDOT constant",
"new_structured_type : structured_type",
"new_structured_type : PACKED structured_type",
"structured_type : array_type",
"structured_type : record_type",
"structured_type : set_type",
"structured_type : file_type",
"array_type : ARRAY LBRAC index_list RBRAC OF component_type",
"index_list : index_list comma index_type",
"index_list : index_type",
"index_type : ordinal_type",
"ordinal_type : new_ordinal_type",
"ordinal_type : identifier",
"component_type : type_denoter",
"record_type : RECORD record_section_list END",
"record_type : RECORD record_section_list semicolon variant_part END",
"record_type : RECORD variant_part END",
"record_section_list : record_section_list semicolon record_section",
"record_section_list : record_section",
"record_section : identifier_list colon type_denoter",
"variant_part : CASE variant_selector OF variant_list semicolon",
"variant_part : CASE variant_selector OF variant_list",
"variant_part :",
"variant_selector : tag_field COLON tag_type",
"variant_selector : tag_type",
"variant_list : variant_list semicolon variant",
"variant_list : variant",
"variant : case_constant_list COLON LPAREN record_section_list RPAREN",
"variant : case_constant_list COLON LPAREN record_section_list semicolon variant_part RPAREN",
"variant : case_constant_list COLON LPAREN variant_part RPAREN",
"case_constant_list : case_constant_list comma case_constant",
"case_constant_list : case_constant",
"case_constant : constant",
"case_constant : constant DOTDOT constant",
"tag_field : identifier",
"tag_type : identifier",
"set_type : SET OF base_type",
"base_type : ordinal_type",
"file_type : PFILE OF component_type",
"new_pointer_type : UPARROW domain_type",
"domain_type : identifier",
"variable_declaration_part : var variable_declaration_list semicolon",
"variable_declaration_part :",
"variable_declaration_list : variable_declaration_list semicolon variable_declaration",
"variable_declaration_list : variable_declaration",
"variable_declaration : identifier COLON type_denoter",
"variable_declaration : identifier_list COLON type_denoter",
"procedure_and_function_declaration_part : proc_or_func_declaration_list semicolon",
"procedure_and_function_declaration_part :",
"proc_or_func_declaration_list : proc_or_func_declaration_list semicolon proc_or_func_declaration",
"proc_or_func_declaration_list : proc_or_func_declaration",
"proc_or_func_declaration : procedure_declaration",
"proc_or_func_declaration : function_declaration",
"procedure_declaration : procedure_heading semicolon directive",
"procedure_declaration : procedure_heading semicolon procedure_block",
"procedure_heading : procedure_identification",
"procedure_heading : procedure_identification formal_parameter_list",
"directive : FORWARD",
"directive : EXTERNAL",
"formal_parameter_list : LPAREN formal_parameter_section_list RPAREN",
"formal_parameter_section_list : formal_parameter_section_list semicolon formal_parameter_section",
"formal_parameter_section_list : formal_parameter_section",
"formal_parameter_section : value_parameter_specification",
"formal_parameter_section : variable_parameter_specification",
"formal_parameter_section : procedural_parameter_specification",
"formal_parameter_section : functional_parameter_specification",
"value_parameter_specification : identifier_list COLON identifier",
"variable_parameter_specification : VAR identifier_list COLON identifier",
"procedural_parameter_specification : procedure_heading",
"functional_parameter_specification : function_heading",
"procedure_identification : PROCEDURE identifier",
"procedure_block : block",
"function_declaration : function_heading semicolon directive",
"function_declaration : function_identification semicolon function_block",
"function_declaration : function_heading semicolon function_block",
"function_heading : FUNCTION identifier COLON result_type",
"function_heading : FUNCTION identifier formal_parameter_list COLON result_type",
"result_type : identifier",
"function_identification : FUNCTION identifier",
"function_block : block",
"statement_part : compound_statement",
"compound_statement : PBEGIN statement_sequence END",
"statement_sequence : statement_sequence semicolon statement",
"statement_sequence : statement",
"statement : open_statement",
"statement : closed_statement",
"open_statement : label COLON non_labeled_open_statement",
"open_statement : non_labeled_open_statement",
"closed_statement : label COLON non_labeled_closed_statement",
"closed_statement : non_labeled_closed_statement",
"non_labeled_closed_statement : assignment_statement",
"non_labeled_closed_statement : procedure_statement",
"non_labeled_closed_statement : goto_statement",
"non_labeled_closed_statement : compound_statement",
"non_labeled_closed_statement : case_statement",
"non_labeled_closed_statement : repeat_statement",
"non_labeled_closed_statement : closed_with_statement",
"non_labeled_closed_statement : closed_if_statement",
"non_labeled_closed_statement : closed_while_statement",
"non_labeled_closed_statement : closed_for_statement",
"non_labeled_closed_statement :",
"non_labeled_open_statement : open_with_statement",
"non_labeled_open_statement : open_if_statement",
"non_labeled_open_statement : open_while_statement",
"non_labeled_open_statement : open_for_statement",
"repeat_statement : REPEAT statement_sequence UNTIL boolean_expression",
"open_while_statement : WHILE boolean_expression DO open_statement",
"closed_while_statement : WHILE boolean_expression DO closed_statement",
"open_for_statement : FOR control_variable ASSIGNMENT initial_value direction final_value DO open_statement",
"closed_for_statement : FOR control_variable ASSIGNMENT initial_value direction final_value DO closed_statement",
"open_with_statement : WITH record_variable_list DO open_statement",
"closed_with_statement : WITH record_variable_list DO closed_statement",
"open_if_statement : IF boolean_expression THEN statement",
"open_if_statement : IF boolean_expression THEN closed_statement ELSE open_statement",
"closed_if_statement : IF boolean_expression THEN closed_statement ELSE closed_statement",
"assignment_statement : variable_access ASSIGNMENT expression",
"variable_access : identifier",
"variable_access : indexed_variable",
"variable_access : field_designator",
"variable_access : variable_access UPARROW",
"indexed_variable : variable_access LBRAC index_expression_list RBRAC",
"index_expression_list : index_expression_list comma index_expression",
"index_expression_list : index_expression",
"index_expression : expression",
"field_designator : variable_access DOT identifier",
"procedure_statement : identifier params",
"procedure_statement : identifier",
"params : LPAREN actual_parameter_list RPAREN",
"actual_parameter_list : actual_parameter_list comma actual_parameter",
"actual_parameter_list : actual_parameter",
"actual_parameter : expression",
"actual_parameter : expression COLON expression",
"actual_parameter : expression COLON expression COLON expression",
"goto_statement : GOTO label",
"case_statement : CASE case_index OF case_list_element_list END",
"case_statement : CASE case_index OF case_list_element_list SEMICOLON END",
"case_statement : CASE case_index OF case_list_element_list SEMICOLON otherwisepart statement END",
"case_statement : CASE case_index OF case_list_element_list SEMICOLON otherwisepart statement SEMICOLON END",
"case_index : expression",
"case_list_element_list : case_list_element_list semicolon case_list_element",
"case_list_element_list : case_list_element",
"case_list_element : case_constant_list COLON statement",
"otherwisepart : OTHERWISE",
"otherwisepart : OTHERWISE COLON",
"control_variable : identifier",
"initial_value : expression",
"direction : TO",
"direction : DOWNTO",
"final_value : expression",
"record_variable_list : record_variable_list comma variable_access",
"record_variable_list : variable_access",
"boolean_expression : expression",
"expression : simple_expression",
"expression : simple_expression relop simple_expression",
"simple_expression : term",
"simple_expression : simple_expression addop term",
"term : factor",
"term : term mulop factor",
"factor : sign factor",
"factor : exponentiation",
"exponentiation : primary",
"exponentiation : primary STARSTAR exponentiation",
"primary : variable_access",
"primary : unsigned_constant",
"primary : function_designator",
"primary : set_constructor",
"primary : LPAREN expression RPAREN",
"primary : NOT primary",
"unsigned_constant : unsigned_number",
"unsigned_constant : CHARACTER_STRING",
"unsigned_constant : nil",
"unsigned_constant : boolean_constant",
"unsigned_number : unsigned_integer",
"unsigned_number : unsigned_real",
"unsigned_integer : DIGSEQ",
"unsigned_real : REALNUMBER",
"boolean_constant : BTRUE",
"boolean_constant : BFALSE",
"function_designator : identifier params",
"set_constructor : LBRAC member_designator_list RBRAC",
"set_constructor : LBRAC RBRAC",
"member_designator_list : member_designator_list comma member_designator",
"member_designator_list : member_designator",
"member_designator : expression DOTDOT expression",
"member_designator : expression",
"addop : PLUS",
"addop : MINUS",
"addop : OR",
"mulop : STAR",
"mulop : SLASH",
"mulop : DIV",
"mulop : MOD",
"mulop : AND",
"relop : EQUAL",
"relop : NOTEQUAL",
"relop : LT",
"relop : GT",
"relop : LE",
"relop : GE",
"relop : IN",
"identifier : IDENTIFIER",
"semicolon : SEMICOLON",
"colon : COLON",
"comma : COMMA",
"var : VAR",
"nil : NIL",
};
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE
 
void yyerror(char *extended_error) 
{
   if (extended_error)
      Pascal::Output::ReportError(
         LineNum,
         Pascal::Error::ParseSyntaxError, 
         String::Format("{0} ({1})", gcnew String(yytext),	gcnew String(extended_error))
      );
   else
      Pascal::Output::ReportError(
         LineNum,
         Pascal::Error::ParseSyntaxError, 
         gcnew String(yytext)
      );
}

Ast::Node ^ Parse()
{
   ParserGCRoots::Initialize();
   int status = yyparse();
   Ast::Node ^ astRoot = ParserGCRoots::syntaxTree;
   ParserGCRoots::Cleanup();
   if (status == 0)
   {
      return astRoot;
   }
   else
   {
      return nullptr;
   }
}
#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
#ifdef __cplusplus
extern "C" { 
char * getenv(const char *);
int yylex();
int yyparse();
}

#endif
int
#if defined(__STDC__)
yyparse(void)
#else
yyparse()
#endif
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
#ifndef __cplusplus
    extern char *getenv();
#endif

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 1:
{ /*file*/ ParserGCRoots::syntaxTree = gcnew Ast::FileNode(LineNum, AstNode(yyvsp[0].ObjRef)); }
break;
case 2:
{ /*file*/ ParserGCRoots::syntaxTree = gcnew Ast::FileNode(LineNum, AstNode(yyvsp[0].ObjRef)); }
break;
case 3:
{ /*program*/ yyval.ObjRef = AddNode(gcnew Ast::ProgramNode(LineNum, AstNode(yyvsp[-3].ObjRef), AstNode(yyvsp[-1].ObjRef))); }
break;
case 4:
{ /*program_heading*/ yyval.ObjRef = AddNode(gcnew Ast::ProgramHeadingNode(LineNum, AstNode(yyvsp[0].ObjRef), 
		nullptr)); }
break;
case 5:
{ /*program_heading*/ yyval.ObjRef = AddNode(gcnew Ast::ProgramHeadingNode(LineNum, AstNode(yyvsp[-3].ObjRef), 
		gcnew IdentifierListNode(LineNum, AstNodeList(yyvsp[-1].ObjRef)))); }
break;
case 6:
{ /*identifier_list*/
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
	yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 7:
{ /*identifier_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 8:
{ /*formal_parameter*/ yyval.ObjRef = AddNode(gcnew Ast::FormalParameterNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 9:
{ /*block*/ 
	List<Node^>^ list = gcnew List<Node^>();
	list->Add(AstNode(yyvsp[-5].ObjRef));
	list->Add(AstNode(yyvsp[-4].ObjRef));
	list->Add(AstNode(yyvsp[-3].ObjRef));
	list->Add(AstNode(yyvsp[-2].ObjRef));
	list->Add(AstNode(yyvsp[-1].ObjRef));
	list->Add(AstNode(yyvsp[0].ObjRef));
	yyval.ObjRef = AddNode(gcnew Ast::BlockNode(LineNum, list)); 
 }
break;
case 10:
{ /*module*/ 
	List<Node^>^ list = gcnew List<Node^>();
	list->Add(AstNode(yyvsp[-3].ObjRef));
	list->Add(AstNode(yyvsp[-2].ObjRef));
	list->Add(AstNode(yyvsp[-1].ObjRef));
	list->Add(AstNode(yyvsp[0].ObjRef));	
	yyval.ObjRef = AddNode(gcnew Ast::ModuleNode(LineNum, list)); }
break;
case 11:
{ /*label_declaration_part*/ 
	yyval.ObjRef = AddNode(gcnew Ast::LabelDeclarationPartNode(LineNum, gcnew LabelListNode(LineNum, AstNodeList(yyvsp[-1].ObjRef)))); 
}
break;
case 12:
{ /*label_declaration_part*/ 
	yyval.ObjRef = AddNode(gcnew Ast::LabelDeclarationPartNode(LineNum, nullptr));
}
break;
case 13:
{ /*label_list*/ 
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
    yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 14:
{ /*label_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 15:
{ /*label*/ yyval.ObjRef = AddNode(gcnew Ast::LabelNode(LineNum, yyvsp[0].IntVal)); }
break;
case 16:
{ /*constant_definition_part*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ConstantDefinitionPartNode(LineNum, gcnew ConstantListNode(LineNum, AstNodeList(yyvsp[0].ObjRef)))); 
}
break;
case 17:
{ /*constant_definition_part*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ConstantDefinitionPartNode(LineNum, nullptr));
}
break;
case 18:
{ /*constant_list*/ 
	AstNodeList(yyvsp[-1].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
    yyval.ObjRef = yyvsp[-1].ObjRef; 
}
break;
case 19:
{ /*constant_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 20:
{  /*constant_definition*/
	yyval.ObjRef = AddNode(gcnew Ast::ConstantDefinitionNode(LineNum, AstNode(yyvsp[-3].ObjRef), AstNode(yyvsp[-1].ObjRef))); 
}
break;
case 21:
{ /*cexpression*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantExpressionNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 22:
{ /*cexpression*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantExpressionNode(LineNum, yyvsp[-1].ObjRef, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); }
break;
case 23:
{ /*csimple_expression*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantSimpleExpressionNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 24:
{ /*csimple_expression*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantSimpleExpressionNode(LineNum, yyvsp[-1].ObjRef, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); }
break;
case 25:
{ /*cterm*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantTermNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 26:
{ /*cterm*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantTermNode(LineNum, yyvsp[-1].ObjRef, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); }
break;
case 27:
{ /*cfactor*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantFactorNode(LineNum, yyvsp[-1].ObjRef, AstNode(yyvsp[0].ObjRef))); }
break;
case 28:
{ /*cfactor*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantFactorNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 29:
{ /*cexponentiation*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantExponentiationNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 30:
{ /*cexponentiation*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantExponentiationNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); }
break;
case 31:
{ /*cprimary*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantPrimaryNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 32:
{ /*cprimary*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantPrimaryNode(LineNum, AstNode(yyvsp[-1].ObjRef))); }
break;
case 33:
{ /*cprimary*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantPrimaryNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 34:
{ /*cprimary*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantPrimaryNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 35:
{ /*constant*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 36:
{ /*constant*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantNode(LineNum, yyvsp[-1].ObjRef, AstNode(yyvsp[0].ObjRef))); }
break;
case 37:
{ /*constant*/ yyval.ObjRef = AddNode(gcnew Ast::ConstantNode(LineNum, gcnew CharacterStringNode(LineNum, yyvsp[0].Text.tokenStart, yyvsp[0].Text.tokenLength))); }
break;
case 38:
{ yyval.ObjRef = PLUS; }
break;
case 39:
{ yyval.ObjRef = MINUS; }
break;
case 40:
{ /*non_string*/ yyval.ObjRef = AddNode(gcnew Ast::NonStringNode(LineNum, (int)yyvsp[0].IntVal)); }
break;
case 41:
{ /*non_string*/ yyval.ObjRef = AddNode(gcnew Ast::NonStringNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 42:
{ /*non_string*/ yyval.ObjRef = AddNode(gcnew Ast::NonStringNode(LineNum, (double)yyvsp[0].RealVal)); }
break;
case 43:
{ /*type_definition_part*/ yyval.ObjRef = AddNode(gcnew Ast::TypeDefinitionPartNode(LineNum, 
		gcnew TypeDefinitionListNode(LineNum, AstNodeList(yyvsp[0].ObjRef)))); }
break;
case 44:
{ /*type_definition_part*/ yyval.ObjRef = AddNode(gcnew Ast::TypeDefinitionPartNode(LineNum, 
		nullptr)); }
break;
case 45:
{ /*type_definition_list*/ 
	AstNodeList(yyvsp[-1].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
    yyval.ObjRef = yyvsp[-1].ObjRef; 
}
break;
case 46:
{ /*type_definition_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 47:
{ /*type_definition*/ yyval.ObjRef = AddNode(gcnew Ast::TypeDefinitionNode(LineNum, AstNode(yyvsp[-3].ObjRef), AstNode(yyvsp[-1].ObjRef))); }
break;
case 48:
{ /*type_denoter*/
	yyval.ObjRef = AddNode(gcnew Ast::TypeDenoterNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 49:
{ /*type_denoter*/
	yyval.ObjRef = AddNode(gcnew Ast::TypeDenoterNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 50:
{ /*new_type*/
	yyval.ObjRef = AddNode(gcnew Ast::NewTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 51:
{ /*new_type*/
	yyval.ObjRef = AddNode(gcnew Ast::NewTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 52:
{ /*new_type*/
	yyval.ObjRef = AddNode(gcnew Ast::NewTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 53:
{ /*new_type*/
	yyval.ObjRef = AddNode(gcnew Ast::NewOrdinalTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 54:
{ /*new_type*/
	yyval.ObjRef = AddNode(gcnew Ast::NewOrdinalTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 55:
{ /*enumerated_type*/
	yyval.ObjRef = AddNode(gcnew Ast::EnumeratedTypeNode(LineNum, gcnew IdentifierListNode(LineNum, AstNodeList(yyvsp[-1].ObjRef))));
}
break;
case 56:
{ /*subrange_type*/
	yyval.ObjRef = AddNode(gcnew Ast::SubrangeTypeNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef)));
}
break;
case 57:
{ /*new_structured_type*/
	yyval.ObjRef = AddNode(gcnew Ast::NewStructuredTypeNode(LineNum, AstNode(yyvsp[0].ObjRef), false));
}
break;
case 58:
{ /*new_structured_type*/
	yyval.ObjRef = AddNode(gcnew Ast::NewStructuredTypeNode(LineNum, AstNode(yyvsp[0].ObjRef), true));
}
break;
case 59:
{ /*structured_type*/
	yyval.ObjRef = AddNode(gcnew Ast::StructuredTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 60:
{ /*structured_type*/
	yyval.ObjRef = AddNode(gcnew Ast::StructuredTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 61:
{ /*structured_type*/
	yyval.ObjRef = AddNode(gcnew Ast::StructuredTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 62:
{ /*structured_type*/
	yyval.ObjRef = AddNode(gcnew Ast::StructuredTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 63:
{ /*array_type*/
	yyval.ObjRef = AddNode(gcnew Ast::ArrayTypeNode(LineNum, gcnew IndexListNode(LineNum, AstNodeList(yyvsp[-3].ObjRef)), AstNode(yyvsp[0].ObjRef)));
}
break;
case 64:
{ /*index_list*/
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
	yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 65:
{ /*index_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 66:
{ /*index_type*/
	yyval.ObjRef = AddNode(gcnew Ast::IndexTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 67:
{ /*ordinal_type*/
	yyval.ObjRef = AddNode(gcnew Ast::OrdinalTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 68:
{ /*ordinal_type*/
	yyval.ObjRef = AddNode(gcnew Ast::OrdinalTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 69:
{ /*component_type*/
	yyval.ObjRef = AddNode(gcnew Ast::ComponentTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 70:
{ /*record_type*/
	yyval.ObjRef = AddNode(gcnew Ast::RecordTypeNode(LineNum, gcnew RecordSectionListNode(LineNum, AstNodeList(yyvsp[-1].ObjRef)), nullptr));
}
break;
case 71:
{ /*record_type*/
	yyval.ObjRef = AddNode(gcnew Ast::RecordTypeNode(LineNum, gcnew RecordSectionListNode(LineNum, AstNodeList(yyvsp[-3].ObjRef)), AstNode(yyvsp[-1].ObjRef)));
}
break;
case 72:
{ /*record_type*/
	yyval.ObjRef = AddNode(gcnew Ast::RecordTypeNode(LineNum, nullptr, AstNode(yyvsp[-1].ObjRef)));
}
break;
case 73:
{ /*record_section_list*/
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
	yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 74:
{ /*record_section_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 75:
{ /*record_section*/
	yyval.ObjRef = AddNode(gcnew Ast::RecordSectionNode(LineNum, gcnew IdentifierListNode(LineNum, AstNodeList(yyvsp[-2].ObjRef)), AstNode(yyvsp[0].ObjRef)));
}
break;
case 76:
{ /*variant_part*/
	yyval.ObjRef = AddNode(gcnew Ast::VariantPartNode(LineNum, AstNode(yyvsp[-3].ObjRef), 
		gcnew VariantListNode(LineNum, AstNodeList(yyvsp[-1].ObjRef)), true));
}
break;
case 77:
{ /*variant_part*/
	yyval.ObjRef = AddNode(gcnew Ast::VariantPartNode(LineNum, AstNode(yyvsp[-2].ObjRef), 
		gcnew VariantListNode(LineNum, AstNodeList(yyvsp[0].ObjRef)), false));
}
break;
case 78:
{ /*variant_part*/
	yyval.ObjRef = AddNode(gcnew Ast::VariantPartNode(LineNum, nullptr, 
		nullptr, false));
}
break;
case 79:
{ /*variant_selector*/
	yyval.ObjRef = AddNode(gcnew Ast::VariantSelectorNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 80:
{ /*variant_selector*/
	yyval.ObjRef = AddNode(gcnew Ast::VariantSelectorNode(LineNum, nullptr, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 81:
{ /*variant_list*/
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
	yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 82:
{ /*variant_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 83:
{ /*variant*/
	yyval.ObjRef = AddNode(gcnew Ast::VariantNode(LineNum, gcnew CaseConstantListNode(LineNum, AstNodeList(yyvsp[-4].ObjRef)), 
		gcnew RecordSectionListNode(LineNum, AstNodeList(yyvsp[-1].ObjRef)), nullptr)); 
}
break;
case 84:
{ /*variant*/
	yyval.ObjRef = AddNode(gcnew Ast::VariantNode(LineNum, gcnew CaseConstantListNode(LineNum, AstNodeList(yyvsp[-6].ObjRef)), 
		gcnew RecordSectionListNode(LineNum, AstNodeList(yyvsp[-3].ObjRef)), AstNode(yyvsp[-1].ObjRef)));
}
break;
case 85:
{ /*variant*/
	yyval.ObjRef = AddNode(gcnew Ast::VariantNode(LineNum, gcnew CaseConstantListNode(LineNum, AstNodeList(yyvsp[-4].ObjRef)), 
		nullptr, AstNode(yyvsp[-1].ObjRef)));
}
break;
case 86:
{ /*case_constant_list*/
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
	yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 87:
{ /*case_constant_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 88:
{ /*case_constant*/
	yyval.ObjRef = AddNode(gcnew Ast::CaseConstantNode(LineNum, AstNode(yyvsp[0].ObjRef), nullptr)); 
}
break;
case 89:
{ /*case_constant*/
	yyval.ObjRef = AddNode(gcnew Ast::CaseConstantNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 90:
{ /*tag_field*/
	yyval.ObjRef = AddNode(gcnew Ast::TagFieldNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 91:
{ /*tag_type*/
	yyval.ObjRef = AddNode(gcnew Ast::TagTypeNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 92:
{ /*set_type*/
	yyval.ObjRef = AddNode(gcnew Ast::SetTypeNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 93:
{ /*base_type*/
	yyval.ObjRef = AddNode(gcnew Ast::BaseTypeNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 94:
{ /*file_type*/
	yyval.ObjRef = AddNode(gcnew Ast::FileTypeNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 95:
{ /*new_pointer_type*/
	yyval.ObjRef = AddNode(gcnew Ast::NewPointerTypeNode(LineNum, yyvsp[-1].Text.tokenStart[0], AstNode(yyvsp[0].ObjRef))); 
}
break;
case 96:
{ /*domain_type*/
	yyval.ObjRef = AddNode(gcnew Ast::DomainTypeNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 97:
{ /*variable_declaration_part*/ 
	yyval.ObjRef = AddNode(gcnew Ast::VariableDeclarationPartNode(LineNum, 
		gcnew VariableDeclarationListNode(LineNum, AstNodeList(yyvsp[-1].ObjRef))));
}
break;
case 98:
{ /*variable_declaration_part*/ 
	yyval.ObjRef = AddNode(gcnew Ast::VariableDeclarationPartNode(LineNum, 
		nullptr)); 
}
break;
case 99:
{ /*variable_declaration_list*/ 
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
	yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 100:
{ /*variable_declaration_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 101:
{ /*variable_declaration*/
	yyval.ObjRef = AddNode(gcnew Ast::VariableDeclarationNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 102:
{ /*variable_declaration*/
	yyval.ObjRef = AddNode(gcnew Ast::VariableDeclarationNode(LineNum, gcnew IdentifierListNode(LineNum, AstNodeList(yyvsp[-2].ObjRef)), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 103:
{ /*procedure_and_function_declaration_part*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ProcedureAndFunctionDeclarationPartNode(LineNum, 
		gcnew ProcedureOrFunctionDeclarationListNode(LineNum, AstNodeList(yyvsp[-1].ObjRef))));
}
break;
case 104:
{ /*procedure_and_function_declaration_part*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ProcedureAndFunctionDeclarationPartNode(LineNum, 
		nullptr));
}
break;
case 105:
{ /*proc_or_func_declaration_list*/ 
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
	yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 106:
{ /*proc_or_func_declaration_list*/ 
	yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); 
}
break;
case 107:
{ /*proc_or_func_declaration*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ProcedureOrFunctionDeclarationNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 108:
{ /*proc_or_func_declaration*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ProcedureOrFunctionDeclarationNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 109:
{ /*procedure_declaration*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ProcedureDeclarationNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 110:
{ /*procedure_declaration*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ProcedureDeclarationNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 111:
{ /*procedure_heading*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ProcedureHeadingNode(LineNum, AstNode(yyvsp[0].ObjRef), nullptr));
}
break;
case 112:
{ /*procedure_heading*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ProcedureHeadingNode(LineNum, AstNode(yyvsp[-1].ObjRef), AstNode(yyvsp[0].ObjRef)));
}
break;
case 113:
{ /*directive*/
	yyval.ObjRef = AddNode(gcnew Ast::DirectiveNode(LineNum, yyvsp[0].Text.tokenStart, yyvsp[0].Text.tokenLength));
}
break;
case 114:
{ /*directive*/
	yyval.ObjRef = AddNode(gcnew Ast::DirectiveNode(LineNum, yyvsp[0].Text.tokenStart, yyvsp[0].Text.tokenLength));
}
break;
case 115:
{ /*formal_parameter_list*/	
	yyval.ObjRef = AddNode(gcnew Ast::FormalParameterListNode(LineNum, 
		gcnew FormalParameterSectionListNode(LineNum, AstNodeList(yyvsp[-1].ObjRef)))); 
}
break;
case 116:
{ /*formal_parameter_section_list*/
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
	yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 117:
{ /*formal_parameter_section_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 118:
{ /*formal_parameter_section*/	
	yyval.ObjRef = AddNode(gcnew Ast::FormalParameterSectionNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 119:
{ /*formal_parameter_section*/	
	yyval.ObjRef = AddNode(gcnew Ast::FormalParameterSectionNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 120:
{ /*formal_parameter_section*/	
	yyval.ObjRef = AddNode(gcnew Ast::FormalParameterSectionNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 121:
{ /*formal_parameter_section*/	
	yyval.ObjRef = AddNode(gcnew Ast::FormalParameterSectionNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 122:
{ /*value_parameter_specification*/	
	yyval.ObjRef = AddNode(gcnew Ast::ValueParameterSpecificationNode(LineNum, 
		gcnew IdentifierListNode(LineNum, AstNodeList(yyvsp[-2].ObjRef)), AstNode(yyvsp[0].ObjRef)));
}
break;
case 123:
{ /*variable_parameter_specification*/	
	yyval.ObjRef = AddNode(gcnew Ast::VariableParameterSpecificationNode(LineNum, 
		gcnew IdentifierListNode(LineNum, AstNodeList(yyvsp[-2].ObjRef)), AstNode(yyvsp[0].ObjRef)));
}
break;
case 124:
{ /*procedural_parameter_specification*/	
	yyval.ObjRef = AddNode(gcnew Ast::ProceduralParameterSpecificationNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 125:
{ /*functional_parameter_specification*/	
	yyval.ObjRef = AddNode(gcnew Ast::FunctionalParameterSpecificationNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 126:
{ /*procedure_identification*/
	yyval.ObjRef = AddNode(gcnew Ast::ProcedureIdentificationNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 127:
{ /*procedure_block*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ProcedureBlockNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 128:
{ /*function_declaration*/ 
	yyval.ObjRef = AddNode(gcnew Ast::FunctionDeclarationNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 129:
{ /*function_declaration*/ 
	yyval.ObjRef = AddNode(gcnew Ast::FunctionDeclarationNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 130:
{ /*function_declaration*/ 
	yyval.ObjRef = AddNode(gcnew Ast::FunctionDeclarationNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 131:
{ /*function_heading*/ 
	yyval.ObjRef = AddNode(gcnew Ast::FunctionHeadingNode(LineNum, AstNode(yyvsp[-2].ObjRef), 
		nullptr, AstNode(yyvsp[0].ObjRef)));
}
break;
case 132:
{ /*function_heading*/ 
	yyval.ObjRef = AddNode(gcnew Ast::FunctionHeadingNode(LineNum, AstNode(yyvsp[-3].ObjRef), 
		AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef)));
}
break;
case 133:
{ /*result_type*/
	yyval.ObjRef = AddNode(gcnew Ast::ResultTypeNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 134:
{ /*function_identification*/ 
	yyval.ObjRef = AddNode(gcnew Ast::FunctionIdentificationNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 135:
{ /*function_block*/ 
	yyval.ObjRef = AddNode(gcnew Ast::FunctionBlockNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 136:
{ /*statement_part*/ 
	yyval.ObjRef = AddNode(gcnew Ast::StatementPartNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 137:
{ /*compound_statement*/ 
	yyval.ObjRef = AddNode(gcnew Ast::CompoundStatementNode(LineNum, gcnew StatementSequenceNode(LineNum, AstNodeList(yyvsp[-1].ObjRef)))); 
}
break;
case 138:
{ /*statement_sequence*/ 
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
    yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 139:
{ /*statement_sequence*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 140:
{ /*statement*/ yyval.ObjRef = AddNode(gcnew Ast::StatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 141:
{ /*statement*/ yyval.ObjRef = AddNode(gcnew Ast::StatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 142:
{ /*open_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::OpenStatementNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef)));
}
break;
case 143:
{ /*open_statement*/ 
	yyval.ObjRef = AddNode(gcnew Ast::OpenStatementNode(LineNum, nullptr, AstNode(yyvsp[0].ObjRef)));
}
break;
case 144:
{ /*closed_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::ClosedStatementNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef)));
}
break;
case 145:
{ /*closed_statement*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ClosedStatementNode(LineNum, nullptr, AstNode(yyvsp[0].ObjRef)));
}
break;
case 146:
{ /*non_labeled_closed_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 147:
{ /*non_labeled_closed_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 148:
{ /*non_labeled_closed_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 149:
{ /*non_labeled_closed_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 150:
{ /*non_labeled_closed_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 151:
{ /*non_labeled_closed_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 152:
{ /*non_labeled_closed_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 153:
{ /*non_labeled_closed_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 154:
{ /*non_labeled_closed_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 155:
{ /*non_labeled_closed_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 156:
{ /*non_labeled_closed_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, nullptr)); }
break;
case 157:
{ /*non_labeled_open_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledOpenStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 158:
{ /*non_labeled_open_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledOpenStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 159:
{ /*non_labeled_open_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledOpenStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 160:
{ /*non_labeled_open_statement*/ yyval.ObjRef = AddNode(gcnew Ast::NonLabeledOpenStatementNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 161:
{ /*repeat_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::RepeatStatementNode(LineNum, 
		gcnew StatementSequenceNode(LineNum, AstNodeList(yyvsp[-2].ObjRef)), AstNode(yyvsp[0].ObjRef)));
}
break;
case 162:
{ /*open_while_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::OpenWhileStatementNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef)));
}
break;
case 163:
{ /*closed_while_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::ClosedWhileStatementNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef)));
}
break;
case 164:
{ /*open_for_statement*/
	List<Node^>^ list = gcnew List<Node^>();
	list->Add(AstNode(yyvsp[-6].ObjRef));
	list->Add(AstNode(yyvsp[-4].ObjRef));
	list->Add(AstNode(yyvsp[-3].ObjRef));
	list->Add(AstNode(yyvsp[-2].ObjRef));
	list->Add(AstNode(yyvsp[0].ObjRef));
	yyval.ObjRef = AddNode(gcnew Ast::OpenForStatementNode(LineNum, list)); 
}
break;
case 165:
{ /*closed_for_statement*/
	List<Node^>^ list = gcnew List<Node^>();
	list->Add(AstNode(yyvsp[-6].ObjRef));
	list->Add(AstNode(yyvsp[-4].ObjRef));
	list->Add(AstNode(yyvsp[-3].ObjRef));
	list->Add(AstNode(yyvsp[-2].ObjRef));
	list->Add(AstNode(yyvsp[0].ObjRef));
	yyval.ObjRef = AddNode(gcnew Ast::ClosedForStatementNode(LineNum, list)); 
}
break;
case 166:
{ /*open_with_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::OpenWithStatementNode(LineNum, 
		gcnew Ast::RecordVariableListNode(LineNum, AstNodeList(yyvsp[-2].ObjRef)), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 167:
{ /*closed_with_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::ClosedWithStatementNode(LineNum, 
		gcnew Ast::RecordVariableListNode(LineNum, AstNodeList(yyvsp[-2].ObjRef)), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 168:
{ /*open_if_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::OpenIfStatementNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef), nullptr)); 
}
break;
case 169:
{ /*open_if_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::OpenIfStatementNode(LineNum, AstNode(yyvsp[-4].ObjRef), AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 170:
{ /*closed_if_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::ClosedIfStatementNode(LineNum, AstNode(yyvsp[-4].ObjRef), AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 171:
{ /*assignment_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::AssignmentStatementNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 172:
{ /*variable_access*/
	yyval.ObjRef = AddNode(gcnew Ast::VariableAccessNode(LineNum, 0, AstNode(yyvsp[0].ObjRef)));
}
break;
case 173:
{ /*variable_access*/
	yyval.ObjRef = AddNode(gcnew Ast::VariableAccessNode(LineNum, 0, AstNode(yyvsp[0].ObjRef)));
}
break;
case 174:
{ /*variable_access*/
	yyval.ObjRef = AddNode(gcnew Ast::VariableAccessNode(LineNum, 0, AstNode(yyvsp[0].ObjRef)));
}
break;
case 175:
{ /*variable_access*/
	yyval.ObjRef = AddNode(gcnew Ast::VariableAccessNode(LineNum, yyvsp[0].Text.tokenStart[0], AstNode(yyvsp[-1].ObjRef)));
}
break;
case 176:
{ /*indexed_variable*/ 
	yyval.ObjRef = AddNode(gcnew Ast::IndexedVariableNode(LineNum, AstNode(yyvsp[-3].ObjRef), 
		gcnew IndexExpressionListNode(LineNum, AstNodeList(yyvsp[-1].ObjRef))));
}
break;
case 177:
{ /*index_expression_list*/ 
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
    yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 178:
{ /*index_expression_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 179:
{ /*index_expression*/ 
	yyval.ObjRef = AddNode(gcnew Ast::IndexExpressionNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 180:
{ /*field_designator*/ 
	yyval.ObjRef = AddNode(gcnew Ast::FieldDesignatorNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef)));
}
break;
case 181:
{ /*procedure_statement*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ProcedureStatementNode(LineNum, AstNode(yyvsp[-1].ObjRef), AstNode(yyvsp[0].ObjRef)));
}
break;
case 182:
{ /*procedure_statement*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ProcedureStatementNode(LineNum, AstNode(yyvsp[0].ObjRef), nullptr));
}
break;
case 183:
{ /*params*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ParamsNode(LineNum, gcnew ActualParameterListNode(LineNum, AstNodeList(yyvsp[-1].ObjRef))));
}
break;
case 184:
{ /*actual_parameter_list*/ 
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
    yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 185:
{ /*actual_parameter_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 186:
{ /*actual_parameter*/ 	
    yyval.ObjRef = AddNode(gcnew Ast::ActualParameterNode(LineNum, AstNode(yyvsp[0].ObjRef), nullptr, nullptr));
}
break;
case 187:
{ /*actual_parameter*/ 	
    yyval.ObjRef = AddNode(gcnew Ast::ActualParameterNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef), nullptr));
}
break;
case 188:
{ /*actual_parameter*/ 	
    yyval.ObjRef = AddNode(gcnew Ast::ActualParameterNode(LineNum, AstNode(yyvsp[-4].ObjRef), AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef)));
}
break;
case 189:
{ /*goto_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::GotoStatementNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 190:
{ /*case_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::CaseStatementNode(LineNum, AstNode(yyvsp[-3].ObjRef), 
		gcnew CaseListElementListNode(LineNum, AstNodeList(yyvsp[-1].ObjRef)), false, nullptr, nullptr, false));
}
break;
case 191:
{ /*case_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::CaseStatementNode(LineNum, AstNode(yyvsp[-4].ObjRef), 
		gcnew CaseListElementListNode(LineNum, AstNodeList(yyvsp[-2].ObjRef)), true, nullptr, nullptr, false));
}
break;
case 192:
{ /*case_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::CaseStatementNode(LineNum, AstNode(yyvsp[-6].ObjRef), 
		gcnew CaseListElementListNode(LineNum, AstNodeList(yyvsp[-4].ObjRef)), true, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[-1].ObjRef), false));
}
break;
case 193:
{ /*case_statement*/
	yyval.ObjRef = AddNode(gcnew Ast::CaseStatementNode(LineNum, AstNode(yyvsp[-7].ObjRef), 
		gcnew CaseListElementListNode(LineNum, AstNodeList(yyvsp[-5].ObjRef)), true, AstNode(yyvsp[-3].ObjRef), AstNode(yyvsp[-2].ObjRef), true));
}
break;
case 194:
{ /*case_index*/
	yyval.ObjRef = AddNode(gcnew Ast::CaseIndexNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 195:
{ /*case_list_element_list*/ 
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
    yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 196:
{ /*case_list_element_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 197:
{ /*case_index*/
	yyval.ObjRef = AddNode(gcnew Ast::CaseListElementNode(LineNum, 
		gcnew CaseConstantListNode(LineNum, AstNodeList(yyvsp[-2].ObjRef)), AstNode(yyvsp[0].ObjRef)));
}
break;
case 198:
{ /*otherwisepart*/
	yyval.ObjRef = AddNode(gcnew Ast::OtherwisePartNode(LineNum, false));
}
break;
case 199:
{ /*otherwisepart*/
	yyval.ObjRef = AddNode(gcnew Ast::OtherwisePartNode(LineNum, true));
}
break;
case 200:
{ /*control_variable*/
	yyval.ObjRef = AddNode(gcnew Ast::ControlVariableNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 201:
{ /*initial_value*/
	yyval.ObjRef = AddNode(gcnew Ast::InitialValueNode(LineNum, AstNode(yyvsp[0].ObjRef)));
}
break;
case 202:
{ /*direction*/
	yyval.ObjRef = AddNode(gcnew Ast::DirectionNode(LineNum, yyvsp[0].Text.tokenStart, yyvsp[0].Text.tokenLength)); 
}
break;
case 203:
{ /*direction*/
	yyval.ObjRef = AddNode(gcnew Ast::DirectionNode(LineNum, yyvsp[0].Text.tokenStart, yyvsp[0].Text.tokenLength)); 
}
break;
case 204:
{ /*final_value*/
	yyval.ObjRef = AddNode(gcnew Ast::FinalValueNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 205:
{ /*record_variable_list*/ 
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
    yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 206:
{ /*record_variable_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 207:
{ /*boolean_expression*/ 
	yyval.ObjRef = AddNode(gcnew Ast::BooleanExpressionNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 208:
{ /*expression*/ 
	yyval.ObjRef = AddNode(gcnew Ast::ExpressionNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 209:
{ /*expression*/
	yyval.ObjRef = AddNode(gcnew Ast::ExpressionNode(LineNum, yyvsp[-1].ObjRef, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef)));
}
break;
case 210:
{ /*simple_expression*/ 
	yyval.ObjRef = AddNode(gcnew Ast::SimpleExpressionNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 211:
{ /*simple_expression*/
	yyval.ObjRef = AddNode(gcnew Ast::SimpleExpressionNode(LineNum, yyvsp[-1].ObjRef, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 212:
{ /*term*/ 
	yyval.ObjRef = AddNode(gcnew Ast::TermNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 213:
{ /*term*/
	yyval.ObjRef = AddNode(gcnew Ast::TermNode(LineNum, yyvsp[-1].ObjRef, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 214:
{ /*factor*/ yyval.ObjRef = AddNode(gcnew Ast::FactorNode(LineNum, yyvsp[-1].ObjRef, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 215:
{ /*factor*/ yyval.ObjRef = AddNode(gcnew Ast::FactorNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 216:
{ /*exponentiation*/
	yyval.ObjRef = AddNode(gcnew Ast::ExponentiationNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 217:
{ /*exponentiation*/
	yyval.ObjRef = AddNode(gcnew Ast::ExponentiationNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 218:
{ /*primary*/ yyval.ObjRef = AddNode(gcnew Ast::PrimaryNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 219:
{ /*primary*/ yyval.ObjRef = AddNode(gcnew Ast::PrimaryNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 220:
{ /*primary*/ yyval.ObjRef = AddNode(gcnew Ast::PrimaryNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 221:
{ /*primary*/ yyval.ObjRef = AddNode(gcnew Ast::PrimaryNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 222:
{ /*primary*/ yyval.ObjRef = AddNode(gcnew Ast::PrimaryNode(LineNum, AstNode(yyvsp[-1].ObjRef))); }
break;
case 223:
{ /*primary*/ yyval.ObjRef = AddNode(gcnew Ast::PrimaryNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 224:
{ /*unsigned_constant*/ yyval.ObjRef = AddNode(gcnew Ast::UnsignedConstantNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 225:
{ /*unsigned_constant*/ yyval.ObjRef = AddNode(gcnew Ast::UnsignedConstantNode(LineNum, gcnew CharacterStringNode(LineNum, yyvsp[0].Text.tokenStart, yyvsp[0].Text.tokenLength))); }
break;
case 226:
{ /*unsigned_constant*/ yyval.ObjRef = AddNode(gcnew Ast::UnsignedConstantNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 227:
{ /*unsigned_constant*/ yyval.ObjRef = AddNode(gcnew Ast::UnsignedConstantNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 228:
{ /*unsigned_number*/ yyval.ObjRef = AddNode(gcnew Ast::UnsignedNumberNode(LineNum, AstNode(yyvsp[0].ObjRef))); }
break;
case 229:
{ /*unsigned_number*/ 
	yyval.ObjRef = AddNode(gcnew Ast::UnsignedNumberNode(LineNum, AstNode(yyvsp[0].ObjRef))); 
}
break;
case 230:
{ /*unsigned_integer*/ 
	yyval.ObjRef = AddNode(gcnew Ast::UnsignedIntegerNode(LineNum, yyvsp[0].IntVal)); 
}
break;
case 231:
{ /*unsigned_real*/ 
	yyval.ObjRef = AddNode(gcnew Ast::UnsignedRealNode(LineNum, yyvsp[0].RealVal)); 
}
break;
case 232:
{ /*boolean_constant*/
	yyval.ObjRef = AddNode(gcnew Ast::BooleanConstantNode(LineNum, true));
}
break;
case 233:
{ /*boolean_constant*/
	yyval.ObjRef = AddNode(gcnew Ast::BooleanConstantNode(LineNum, false));
}
break;
case 234:
{ /*function_designator*/ 
	yyval.ObjRef = AddNode(gcnew Ast::FunctionDesignatorNode(LineNum, AstNode(yyvsp[-1].ObjRef), AstNode(yyvsp[0].ObjRef))); 
}
break;
case 235:
{ /*set_constructor*/ 
	yyval.ObjRef = AddNode(gcnew Ast::SetConstructorNode(LineNum, 
		gcnew MemberDesignatorListNode(LineNum, AstNodeList(yyvsp[-1].ObjRef))));
}
break;
case 236:
{ /*set_constructor*/ 
	yyval.ObjRef = AddNode(gcnew Ast::SetConstructorNode(LineNum, 
		nullptr));
}
break;
case 237:
{ /*member_designator_list*/ 
	AstNodeList(yyvsp[-2].ObjRef)->Add(AstNode(yyvsp[0].ObjRef)); 
    yyval.ObjRef = yyvsp[-2].ObjRef; 
}
break;
case 238:
{ /*member_designator_list*/ yyval.ObjRef = AddNodeList(yyvsp[0].ObjRef); }
break;
case 239:
{ /*member_designator*/ 
	yyval.ObjRef = AddNode(gcnew Ast::MemberDesignatorNode(LineNum, AstNode(yyvsp[-2].ObjRef), AstNode(yyvsp[0].ObjRef)));
}
break;
case 240:
{ /*member_designator*/ 
	yyval.ObjRef = AddNode(gcnew Ast::MemberDesignatorNode(LineNum, nullptr, AstNode(yyvsp[0].ObjRef)));
}
break;
case 241:
{ yyval.ObjRef = PLUS; }
break;
case 242:
{ yyval.ObjRef = MINUS; }
break;
case 243:
{ yyval.ObjRef = OR; }
break;
case 244:
{ yyval.ObjRef = STAR; }
break;
case 245:
{ yyval.ObjRef = SLASH; }
break;
case 246:
{ yyval.ObjRef = DIV; }
break;
case 247:
{ yyval.ObjRef = MOD; }
break;
case 248:
{ yyval.ObjRef = AND; }
break;
case 249:
{ yyval.ObjRef = EQUAL; }
break;
case 250:
{ yyval.ObjRef = NOTEQUAL; }
break;
case 251:
{ yyval.ObjRef = LT; }
break;
case 252:
{ yyval.ObjRef = GT; }
break;
case 253:
{ yyval.ObjRef = LE; }
break;
case 254:
{ yyval.ObjRef = GE; }
break;
case 255:
{ yyval.ObjRef = IN; }
break;
case 256:
{ /*identifier*/
	yyval.ObjRef = AddNode(gcnew Ast::IdentifierNode(LineNum, yyvsp[0].Text.tokenStart, yyvsp[0].Text.tokenLength));
}
break;
case 257:
{ yyval.ObjRef = SEMICOLON; }
break;
case 258:
{ yyval.ObjRef = COLON; }
break;
case 259:
{ yyval.ObjRef = COMMA; }
break;
case 260:
{ yyval.ObjRef = VAR; }
break;
case 261:
{ /*nil*/
	yyval.ObjRef = AddNode(gcnew Ast::NilNode(LineNum)); 
}
break;
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
