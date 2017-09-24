%namespace LexParser
%using Lispkit.Ast

%{

private int LineNumber
{
	get { return (this.scanner as LexScanner.Scanner).LineNumber; }
}

private int ColumnNumber
{
	get { return (this.scanner as LexScanner.Scanner).ColumnNumber; }
}

internal Node AstRootNode
{
	get { return astRootNode; }
}

private Node astRootNode = null;

%}

%start program

%token IDENTIFIER INTEGER LPAREN RPAREN
%token ADD SUB MUL DIV REM NIL
%token QUOTE EQ LEQ CAR CDR ATOM CONS IF LAMBDA LET LETREC

%token <StringValue> IDENTIFIER
%token <IntValue> INTEGER ADD SUB MUL DIV REM NIL QUOTE EQ LEQ 
%token <IntValue> CAR CDR ATOM CONS IF LAMBDA LET LETREC

%left '|'
%left '&'
%left ADD SUB
%left MUL DIV REM
%left UMINUS

%union {   
  internal string StringValue;
  internal int IntValue;  
  internal AritOp Op;  
  internal Node ObjectRef;
  internal Tokens TokenValue; 
}

%type <ObjectRef> callExp atom symbolic numeric 
%type <ObjectRef> SExpression SExpressionList symbolicList symbolicSequence
%type <ObjectRef> exp expNoParen quoteExp eqExp aritExp leqExp carExp cdrExp 
%type <ObjectRef> atomExp consExp ifExp lambdaExp letExp consPairList
%type <ObjectRef> consPair letrecExp consPairListRec consPairRec
%type <ObjectRef> callExp expList function program 

%type <IntValue>  integer

%type <Op>		  aritOp

%type <TokenValue> ReservedWord

%%

ReservedWord	: ADD  { $$ = Tokens.ADD; }
				| SUB  { $$ = Tokens.SUB; }
				| MUL  { $$ = Tokens.MUL; }
				| DIV  { $$ = Tokens.DIV; }
				| REM  { $$ = Tokens.REM; }
				| NIL { $$ = Tokens.NIL; }
				| QUOTE  { $$ = Tokens.QUOTE; }
				| EQ  { $$ = Tokens.EQ; }
				| LEQ  { $$ = Tokens.LEQ; }
				| CAR  { $$ = Tokens.CAR; }
				| CDR  { $$ = Tokens.CDR; }
				| ATOM  { $$ = Tokens.ATOM; }
				| CONS  { $$ = Tokens.CONS; }
				| IF { $$ = Tokens.IF; }
				| LAMBDA  { $$ = Tokens.LAMBDA; }
				| LET  { $$ = Tokens.LET; }
				| LETREC  { $$ = Tokens.LETREC; }
;

atom			: symbolic
{
	Lispkit.Output.TraceMessage("Matching 'atom'");
	$$ = new AtomNode(this.LineNumber, this.ColumnNumber, $1);
}
				| numeric
{
	Lispkit.Output.TraceMessage("Matching 'atom'");
	$$ = new AtomNode(this.LineNumber, this.ColumnNumber, $1);
}				
;

symbolic		: IDENTIFIER
{
	Lispkit.Output.TraceMessage("Matching 'symbolic'");
	$$ = new SymbolicNode(this.LineNumber, this.ColumnNumber, $1);
}
;

numeric			: integer
{
	Lispkit.Output.TraceMessage("Matching 'numeric'");
	$$ = new NumericNode(this.LineNumber, this.ColumnNumber, $1);
}
;

integer			: INTEGER
{
	Lispkit.Output.TraceMessage("Matching 'integer'");
	$$ = $1;
}
;

SExpression     : atom
{
	Lispkit.Output.TraceMessage("Matching 'SExpression'");	
	$$ = new SExpressionNode(this.LineNumber, this.ColumnNumber, $1);
}
				| LPAREN SExpressionList RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'SExpression'");	
	$$ = new SExpressionNode(this.LineNumber, this.ColumnNumber, $2);
}				
;				

SExpressionList : SExpression
{
	Lispkit.Output.TraceMessage("Matching 'SExpressionList'");	
	$$ = new SExpressionListNode(this.LineNumber, this.ColumnNumber, $1);
}
				| SExpression SExpressionList
{
	Lispkit.Output.TraceMessage("Matching 'SExpressionList'");	
	$$ = new SExpressionListNode(this.LineNumber, this.ColumnNumber, $1, $2);
}				
;

symbolicList	: LPAREN /*epsilon*/ RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'symbolicList'");	
	$$ = new SymbolicListNode(this.LineNumber, this.ColumnNumber);
}
				| LPAREN symbolicSequence RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'symbolicList'");	
	$$ = new SymbolicListNode(this.LineNumber, this.ColumnNumber, $2);
}				
;

symbolicSequence : symbolic
{
	Lispkit.Output.TraceMessage("Matching 'symbolicSequence'");	
	$$ = new SymbolicSequenceNode(this.LineNumber, this.ColumnNumber, $1);
}
				| symbolic symbolicSequence				
{
	Lispkit.Output.TraceMessage("Matching 'symbolicSequence'");	
	$$ = new SymbolicSequenceNode(this.LineNumber, this.ColumnNumber, $1, $2);
}				
;

exp				: symbolic
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}
				| LPAREN quoteExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $2);
}
				| LPAREN eqExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $2);
}
				| LPAREN leqExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $2);
}
				| LPAREN carExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $2);
}
				| LPAREN cdrExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $2);
}
				| LPAREN atomExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $2);
}
				| LPAREN consExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $2);
}
				| LPAREN ifExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $2);
}
				| LPAREN lambdaExp RPAREN 
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $2);
}
				| LPAREN letExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $2);
}
				| LPAREN letrecExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $2);
}
				| LPAREN callExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $2);
}				
				| LPAREN aritExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $2);
}
;

expNoParen      : quoteExp				
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}				
				| eqExp
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}
				| leqExp
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}
				| carExp
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}
				| cdrExp
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}
				| atomExp
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}
				| consExp
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}
				| ifExp
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}
				| lambdaExp
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}
				| letExp
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}
				| letrecExp
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}
				| callExp
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}				
				| aritExp
{
	Lispkit.Output.TraceMessage("Matching 'exp'");	
	$$ = new ExpNode(this.LineNumber, this.ColumnNumber, $1);
}
;

quoteExp		: QUOTE SExpression
{
	Lispkit.Output.TraceMessage("Matching 'quoteExp'");	
	$$ = new QuoteExpNode(this.LineNumber, this.ColumnNumber, $2);
}
				| QUOTE ReservedWord
{
	Lispkit.Output.TraceMessage("Matching 'quoteExp'");	
	$$ = new QuoteExpNode(this.LineNumber, this.ColumnNumber, $2);
}
;				

eqExp			: EQ exp exp
{
	Lispkit.Output.TraceMessage("Matching 'eqExp'");
	$$ = new EqExpNode(this.LineNumber, this.ColumnNumber, $2, $3);
}
;

aritExp			: aritOp exp exp 
{
	Lispkit.Output.TraceMessage("Matching 'aritExp'");	
	$$ = new AritExpNode(this.LineNumber, this.ColumnNumber, $2, $3, $1);
}
;

aritOp			: ADD
{
	Lispkit.Output.TraceMessage("Matching 'aritOp'");	
	$$ = AritOp.Add;
} 
				| SUB
{
	Lispkit.Output.TraceMessage("Matching 'aritOp'");	
	$$ = AritOp.Sub;
}
				| MUL
{
	Lispkit.Output.TraceMessage("Matching 'aritOp'");	
	$$ = AritOp.Mul;
}
				| DIV
{
	Lispkit.Output.TraceMessage("Matching 'aritOp'");	
	$$ = AritOp.Div;
}
				| REM				
{
	Lispkit.Output.TraceMessage("Matching 'aritOp'");	
	$$ = AritOp.Rem;
}				
;

leqExp			: LEQ exp exp 
{
	Lispkit.Output.TraceMessage("Matching 'leqExp'");	
	$$ = new LeqExpNode(this.LineNumber, this.ColumnNumber, $2, $3);
}
;

carExp			: CAR exp
{
	Lispkit.Output.TraceMessage("Matching 'carExp'");
	$$ = new CarExpNode(this.LineNumber, this.ColumnNumber, $2);
}
;

cdrExp			: CDR exp
{
	Lispkit.Output.TraceMessage("Matching 'cdrExp'");
	$$ = new CdrExpNode(this.LineNumber, this.ColumnNumber, $2);
}
;

atomExp			: ATOM exp
{
	Lispkit.Output.TraceMessage("Matching 'atomExp'");
	$$ = new AtomExpNode(this.LineNumber, this.ColumnNumber, $2);
}
;

consExp			: CONS exp exp
{
	Lispkit.Output.TraceMessage("Matching 'consExp'");
	$$ = new ConsExpNode(this.LineNumber, this.ColumnNumber, $2, $3);
}
;

ifExp			: IF exp exp exp
{
	Lispkit.Output.TraceMessage("Matching 'ifExp'");
	$$ = new IfExpNode(this.LineNumber, this.ColumnNumber, $2, $3, $4);
}
;

lambdaExp		: LAMBDA symbolicList exp
{
	Lispkit.Output.TraceMessage("Matching 'lambdaExp'");
	$$ = new LambdaExpNode(this.LineNumber, this.ColumnNumber, $2, $3);
}
;

letExp			: LET exp consPairList                  
{
	Lispkit.Output.TraceMessage("Matching 'letExp'");
	$$ = new LetExpNode(this.LineNumber, this.ColumnNumber, $2, $3);
}
				| LET exp
{
	Lispkit.Output.TraceMessage("Matching 'letExp'");
	$$ = new LetExpNode(this.LineNumber, this.ColumnNumber, $2);
}
;

consPairList	: consPair consPairList
{
	Lispkit.Output.TraceMessage("Matching 'consPairList'");
	$$ = new ConsPairListNode(this.LineNumber, this.ColumnNumber, $1, $2);
}
				| consPair
{
	Lispkit.Output.TraceMessage("Matching 'consPairList'");
	$$ = new ConsPairListNode(this.LineNumber, this.ColumnNumber, $1);
}				
;			

consPair		: LPAREN symbolic expNoParen RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'consPair'");
	$$ = new ConsPairNode(this.LineNumber, this.ColumnNumber, $2, $3);
}
;

letrecExp		: LETREC exp consPairListRec
{
	Lispkit.Output.TraceMessage("Matching 'letrecExp'");
	$$ = new LetrecExpNode(this.LineNumber, this.ColumnNumber, $2, $3);
}
				| LETREC exp
{
	Lispkit.Output.TraceMessage("Matching 'letrecExp'");
	$$ = new LetrecExpNode(this.LineNumber, this.ColumnNumber, $2);
}
;

consPairListRec	: consPairRec consPairListRec
{
	Lispkit.Output.TraceMessage("Matching 'consPairListRec'");
	$$ = new ConsPairRecListNode(this.LineNumber, this.ColumnNumber, $1, $2);
}
				| consPairRec
{
	Lispkit.Output.TraceMessage("Matching 'consPairListRec'");
	$$ = new ConsPairRecListNode(this.LineNumber, this.ColumnNumber, $1);
}
;			

consPairRec		: LPAREN symbolic '.' lambdaExp RPAREN 
{
	Lispkit.Output.TraceMessage("Matching 'consPairRec'");
	$$ = new ConsPairRecNode(this.LineNumber, this.ColumnNumber, $2, $4);
}
				| LPAREN symbolic lambdaExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'consPairRec'");
	$$ = new ConsPairRecNode(this.LineNumber, this.ColumnNumber, $2, $3);
}
;

callExp			: function expList
{
	Lispkit.Output.TraceMessage("Matching 'callExp'");
	$$ = new CallExpNode(this.LineNumber, this.ColumnNumber, $1, $2);
}
				| function
{
	Lispkit.Output.TraceMessage("Matching 'callExp'");
	$$ = new CallExpNode(this.LineNumber, this.ColumnNumber, $1);
}
;

expList			: exp expList
{
	Lispkit.Output.TraceMessage("Matching 'expList'");
	$$ = new ExpListNode(this.LineNumber, this.ColumnNumber, $1, $2);
}
				| exp
{
	Lispkit.Output.TraceMessage("Matching 'expList'");	
	$$ = new ExpListNode(this.LineNumber, this.ColumnNumber, $1);
}			
;				

function		: symbolic 
{
	Lispkit.Output.TraceMessage("Matching 'function'");
	$$ = new FunctionNode(this.LineNumber, this.ColumnNumber, $1);
}
				| LPAREN lambdaExp RPAREN 
{
	Lispkit.Output.TraceMessage("Matching 'function'");
	$$ = new FunctionNode(this.LineNumber, this.ColumnNumber, $2);
}				
				| LPAREN letExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'function'");
	$$ = new FunctionNode(this.LineNumber, this.ColumnNumber, $2);
}				
				| LPAREN letrecExp RPAREN 
{
	Lispkit.Output.TraceMessage("Matching 'function'");
	$$ = new FunctionNode(this.LineNumber, this.ColumnNumber, $2);
}				
;

program			: LPAREN letrecExp RPAREN 
{
	Lispkit.Output.TraceMessage("Matching 'program'");
	$$ = new ProgramNode(this.LineNumber, this.ColumnNumber, $2);
	this.astRootNode = $$;
}
				| LPAREN letExp RPAREN 
{
	Lispkit.Output.TraceMessage("Matching 'program'");
	$$ = new ProgramNode(this.LineNumber, this.ColumnNumber, $2);
	this.astRootNode = $$;
}				
				| LPAREN lambdaExp RPAREN
{
	Lispkit.Output.TraceMessage("Matching 'program'");
	$$ = new ProgramNode(this.LineNumber, this.ColumnNumber, $2);
	this.astRootNode = $$;
}
;

%%