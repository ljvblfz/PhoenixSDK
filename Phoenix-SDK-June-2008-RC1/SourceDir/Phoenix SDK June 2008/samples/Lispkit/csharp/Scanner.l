/*
   
 */

%namespace LexScanner
%using LexParser;

%{

public override void yyerror(string format, params object[] args)
{
	format = string.Format("({0}, {1}): {2}", this.yyline, this.yycol+1, format);
    Console.WriteLine(format, args);
    this.errorCount++;
}

public int LineNumber
{
	get { return this.yyline; }
}

public int ColumnNumber
{
	get { return this.yycol + 1; }
}

public int ErrorCount
{
	get { return this.errorCount; }
}

private int errorCount = 0;

%}

A [aA]
B [bB]
C [cC]
D [dD]
E [eE]
F [fF]
G [gG]
H [hH]
I [iI]
J [jJ]
K [kK]
L [lL]
M [mM]
N [nN]
O [oO]
P [pP]
Q [qQ]
R [rR]
S [sS]
T [tT]
U [uU]
V [vV]
W [wW]
X [xX]
Y [yY]
Z [zZ]

digits [\-]?[0-9]+
identifier [a-zA-Z_][a-zA-Z0-9\-_]*

%%
%{

    // local variables
    
    this.errorCount = 0;
    
%}

"("					return (int)Tokens.LPAREN;
")"					return (int)Tokens.RPAREN;

{A}{D}{D}			return (int)Tokens.ADD;
{S}{U}{B}			return (int)Tokens.SUB;
{M}{U}{L}			return (int)Tokens.MUL;
{D}{I}{V}			return (int)Tokens.DIV;
{R}{E}{M}			return (int)Tokens.REM;

{N}{I}{L}			return (int)Tokens.NIL;
{Q}{U}{O}{T}{E}		return (int)Tokens.QUOTE;
{E}{Q}				return (int)Tokens.EQ;
{L}{E}{Q}			return (int)Tokens.LEQ;
{C}{A}{R} 			return (int)Tokens.CAR;
{C}{D}{R} 			return (int)Tokens.CDR;
{A}{T}{O}{M}		return (int)Tokens.ATOM;
{C}{O}{N}{S}		return (int)Tokens.CONS;
{I}{F}				return (int)Tokens.IF;
{L}{A}{M}{B}{D}{A}	return (int)Tokens.LAMBDA;
{L}{E}{T}			return (int)Tokens.LET;
{L}{E}{T}{R}{E}{C}	return (int)Tokens.LETREC;

{identifier}		yylval.StringValue = yytext; return (int)Tokens.IDENTIFIER;
{digits}			yylval.IntValue = Int32.Parse(yytext); return (int)Tokens.INTEGER;

%%

