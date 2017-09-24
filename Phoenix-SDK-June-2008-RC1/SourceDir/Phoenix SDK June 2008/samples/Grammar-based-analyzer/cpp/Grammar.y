
%{
#include <stdio.h>

// Scanner interface.
#include "Scanner.h"

// Parser interface.
#include "Parser.h"

// Include definition of abstract class Node.
#include "Ast.h"

extern "C" int yyparse();

using namespace System::Collections::Generic;

ref struct ParserGCRoots
{
public:

   // Root of the tree built by the parser.

   static ref class Node ^ _syntaxTree = nullptr;

   // Indirection to refer to managed Ast nodes using yacc attributes.

   static List<ref class Node ^>^ _astNodes
      = gcnew List<ref class Node ^>();

   // Indirection to refer to Ast node lists using yacc attributes.

   static List< List<ref class Node^>^ >^ _astNodeLists
      = gcnew List< List<ref class Node^>^ >();

   // Indirection to refer to Phx::Types::Type using yacc attributes.

   static array<Phx::Types::Type ^> ^ _phoenixType
      = gcnew array<Phx::Types::Type ^>((int)SymbolType::NumSymbolTypes); 

   // Pre-processing step to yyparse().

   static void Initialize()
   {
      // Populate _phoenixType.
      _phoenixType[(int)SymbolType::Int] = Phx::GlobalData::TypeTable->Int32Type;
      _phoenixType[(int)SymbolType::Boolean] = Phx::GlobalData::TypeTable->BooleanType;
      _phoenixType[(int)SymbolType::String] = Phx::GlobalData::TypeTable->SystemStringAggregateType;
      _phoenixType[(int)SymbolType::Error] = Phx::GlobalData::TypeTable->UnknownType;
   }

   // Post-processing step to yyparse().

   static void Cleanup()
   {
      _astNodes = nullptr;
      _astNodeLists = nullptr;
      for (int i = 0; i < (int)SymbolType::NumSymbolTypes; ++i)
      {
         _phoenixType[i] = nullptr;
      }
   }
};


// Short-hand for accessing the managed Ast nodes.

static inline Node^ AstNode(int nodeRef)
{
   return (nodeRef == -1) ? nullptr : ParserGCRoots::_astNodes[nodeRef];
}

// Short-hand for accessing Ast node lists.
static inline List<Node^>^ AstNodeList(int listRef)
{
   return (listRef == -1) ? nullptr : ParserGCRoots::_astNodeLists[listRef];
}

// Create a node reference for use in native datatypes, in particular in the union and array of unions
// that implement $$ and $i.
static inline int NewNode(Node^ newNode)
{
   ParserGCRoots::_astNodes->Add(newNode);
   return ParserGCRoots::_astNodes->Count - 1;
}

// Create a nodelist reference for use in yacc's native datatypes.
static inline int NewNodeList(int nodeRef =-1)
{
   // Create a new list and record it in the GC roots.
   List<Node^>^ newNodeList = gcnew List<Node^>();
   ParserGCRoots::_astNodeLists->Add(newNodeList);

   // If a nodeRef was passed, add the node to the new list.
   if (nodeRef != -1)
   {
      newNodeList->Add(ParserGCRoots::_astNodes[nodeRef]);
   }
   return ParserGCRoots::_astNodeLists->Count - 1;
}

// Some yacc implementations require yyerror to be declared.
void yyerror(char*);

%}

%union {
  int   IntVal;
  struct Token
  {
     char* tokenStart;
     int   tokenLength;
  } Text;
  int SymbolType;
  int ObjRef;
}

%start prog

%type <ObjRef> statement condition assignment 
%type <ObjRef> statement_list else_branch expression
%type <ObjRef> procedure_definition_list procedure_definition procedure_header
%type <ObjRef> formal_parameter formal_parameter_list 
%type <ObjRef> procedure_call actual_parameter_list return statement_block
%type <SymbolType> type


%token <Text> ID STRING_LITERAL 
%token <SymbolType> INT STRING BOOL
%token <IntVal> NUMBER
%token IF GETS TRUE FALSE RETURN

%nonassoc THEN
%nonassoc ELSE 

%nonassoc AND OR LT LE GT GE EQ NE
%right NOT

%left '+' '-'
%left '*' '/'
%right UMINUS

/* Dummy tokens to set a precedence order among nonterminals where needed. */
%left HIGHER
%left LOWER


%%

prog            :      procedure_definition_list
                          { ParserGCRoots::_syntaxTree = gcnew ModuleNode(LineNumber, AstNodeList($1)); }
                ;

procedure_definition_list : procedure_definition
                          { $$ = NewNodeList($1); }
                |       procedure_definition_list procedure_definition
                          { 
                             AstNodeList($1)->Add(AstNode($2)); 
                             $$ = $1; 
                          }
                ;

procedure_definition :   procedure_header '{' statement_list '}'
                          { 
                            AstNodeList($3)->Insert(0, AstNode($1)); // Add $1 as leftmost child.
                            $$ = NewNode(gcnew FunctionNode(LineNumber, AstNodeList($3))); 
                          }
                ;

procedure_header :       ID '(' formal_parameter_list ')'
                          { $$ = NewNode(gcnew HeaderNode(LineNumber, $1.tokenStart, $1.tokenLength, AstNodeList($3))); }
                ;

formal_parameter_list:                   %prec LOWER    { $$ = NewNodeList(); }
                |       formal_parameter %prec HIGHER   { $$ = NewNodeList($1); }
                |       formal_parameter_list ',' formal_parameter
                           {
                             AstNodeList($1)->Add(AstNode($3)); 
                             $$ = $1; 
                           }
                ;

formal_parameter :      type ID 
                           { $$ = NewNode(gcnew FormalParameterNode(LineNumber, ParserGCRoots::_phoenixType[$1], $2.tokenStart, $2.tokenLength)); }
                ;

type            :       INT    
                |       STRING 
                |       BOOL   
                ;

statement_list  :       /* epsilon */            { $$ = NewNodeList(); }
                |       statement_list statement {
                                                   AstNodeList($1)->Add(AstNode($2)); 
                                                   $$ = $1; 
                                                 }
                ;

statement       :       condition
                |       assignment
                |       procedure_call
                |       return
                |       statement_block
                ;

statement_block :       '{' statement_list '}'  
                           { $$ = NewNode(gcnew StatementBlockNode(LineNumber, AstNodeList($2))); }
                ;

condition       :       IF '(' expression ')' statement else_branch
                           { $$ = NewNode(gcnew ConditionNode(LineNumber, AstNode($3), AstNode($5), AstNode($6))); }
                ;

else_branch     :       /* empty production has lower precedence than nonempty one */ %prec THEN 
                           { $$ = -1; }
                |       ELSE statement
                           { $$ = $2; }
                ;

assignment      :       ID GETS expression   ';'
                           { $$ = NewNode(gcnew AssignmentNode(LineNumber, $1.tokenStart, $1.tokenLength, AstNode($3))); }
                ;

return          :       RETURN ';'              
                           { $$ = NewNode(gcnew ReturnNode(LineNumber)); }
                ;

procedure_call   :       ID '(' actual_parameter_list ')' ';'
                           { $$ = NewNode(gcnew FunctionCallNode(LineNumber, $1.tokenStart, $1.tokenLength, AstNodeList($3))); }
                ;

actual_parameter_list :                 %prec LOWER     { $$ = NewNodeList(); }
                |       expression      %prec HIGHER    { $$ = NewNodeList($1); }
                |       actual_parameter_list ',' expression
                           {
                             AstNodeList($1)->Add(AstNode($3)); 
                             $$ = $1; 
                           }
                ;

expression      :       NUMBER
                           { $$ = NewNode(gcnew NumberNode(LineNumber, $1)); }
                |       ID
                           { $$ = NewNode(gcnew VariableNode(LineNumber, $1.tokenStart, $1.tokenLength)); }
                |       FALSE
                           { $$ = NewNode(gcnew LogicalNode(LineNumber, false)); }
                |       TRUE
                           { $$ = NewNode(gcnew LogicalNode(LineNumber, true)); }
                |       STRING_LITERAL
                           { $$ = NewNode(gcnew StringNode(LineNumber, $1.tokenStart, $1.tokenLength)); }

                |       expression '*' expression
                           { $$ = NewNode(gcnew BinaryArithmeticNode(LineNumber, '*', AstNode($1), AstNode($3))); }
                |       expression '/' expression
                           { $$ = NewNode(gcnew BinaryArithmeticNode(LineNumber, '/', AstNode($1), AstNode($3))); }
                |       expression '+' expression
                           { $$ = NewNode(gcnew BinaryArithmeticNode(LineNumber, '+', AstNode($1), AstNode($3))); }
                |       expression '-' expression
                           { $$ = NewNode(gcnew BinaryArithmeticNode(LineNumber, '-', AstNode($1), AstNode($3))); }
                |       '-' expression                         %prec UMINUS
                           { $$ = NewNode(gcnew UnaryMinusNode(LineNumber, AstNode($2))); }

                |       expression EQ expression
                           { $$ = NewNode(gcnew ComparisonNode(LineNumber, EQ, AstNode($1), AstNode($3))); }
                |       expression NE expression
                           { $$ = NewNode(gcnew ComparisonNode(LineNumber, NE, AstNode($1), AstNode($3))); }

                |       expression GT expression
                           { $$ = NewNode(gcnew ComparisonNode(LineNumber, GT, AstNode($1), AstNode($3))); }
                |       expression GE expression
                           { $$ = NewNode(gcnew ComparisonNode(LineNumber, GE, AstNode($1), AstNode($3))); }
                |       expression LT expression
                           { $$ = NewNode(gcnew ComparisonNode(LineNumber, LT, AstNode($1), AstNode($3))); }
                |       expression LE expression
                           { $$ = NewNode(gcnew ComparisonNode(LineNumber, LE, AstNode($1), AstNode($3))); }

                |       expression AND expression
                           { $$ = NewNode(gcnew BinaryLogicalNode(LineNumber, AND, AstNode($1), AstNode($3))); }
                |       expression OR expression
                           { $$ = NewNode(gcnew BinaryLogicalNode(LineNumber, OR, AstNode($1), AstNode($3))); }

                |       NOT expression
                           { $$ = NewNode(gcnew NegationNode(LineNumber, AstNode($2))); }

                |       '(' expression ')'
                           { $$ = $2; }
                ;


%%

void yyerror(char *s) {
   fprintf(stderr, "%s\n", s);
   fprintf(stderr, "   Line %d. Current token: %s\n", LineNumber, yytext);
}


Node ^ Parse()
{
   ParserGCRoots::Initialize();
   int status = yyparse();
   Node ^ astRoot = ParserGCRoots::_syntaxTree;
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
