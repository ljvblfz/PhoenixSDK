%{
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
 
// Scanner interface.
#include "Scanner.h"

// Parser interface.
#include "Parser.h"

// Include definition of abstract class Node.
#include "Ast.h"

extern int   LineNum; // current line in the scanner
extern char* yytext;  // current token in the scanner
extern "C" int yyparse();

// Short-hand for accessing the managed AST nodes.
static inline Node^ AstNode(int nodeRef)
{
   return (nodeRef == -1) ? nullptr : ParserGCRoots::astNodes[nodeRef];
}

// Short-hand for accessing AST node lists.
static inline List<Node^>^ AstNodeList(int listRef)
{
   return (listRef == -1) ? nullptr : ParserGCRoots::astNodeLists[listRef];
}

// Create a node reference for use in native datatypes, in particular in the union and array of unions
// that implement $$ and $i.
static inline int AddNode(Node^ newNode)
{
   ParserGCRoots::astNodes->Add(newNode);
   return ParserGCRoots::astNodes->Count - 1;
}

// Create a nodelist reference for use in yacc's native datatypes.
static inline int AddNodeList(int nodeRef =-1)
{
   // Create a new list and record it in the GC roots.
   List<Node^>^ newNodeList = gcnew List<Node^>();
   ParserGCRoots::astNodeLists->Add(newNodeList);

   // If a nodeRef was passed, add the node to the new list.
   if (nodeRef != -1)
   {
      newNodeList->Add(ParserGCRoots::astNodes[nodeRef]);
   }
   return ParserGCRoots::astNodeLists->Count - 1;
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
  int ObjRef;
  int RelOp;
  int ArithmeticOp;
  double RealVal;
}

//%start file

%type <ObjRef> program module program_heading block identifier identifier_list  
%type <ObjRef> label_declaration_part constant_definition_part
%type <ObjRef> formal_parameter variable_declaration variable_declaration_part variable_declaration_list
%type <ObjRef> type_definition_part constant_list constant_definition 
%type <ObjRef> procedure_and_function_declaration_part statement_part 
%type <ObjRef> cexpression csimple_expression cterm cfactor cexponentiation cprimary
%type <ObjRef> expression simple_expression term factor exponentiation primary boolean_expression
%type <ObjRef> addop mulop relop sign
%type <ObjRef> label label_list boolean_constant
%type <ObjRef> unsigned_constant unsigned_number unsigned_real unsigned_integer constant non_string
%type <ObjRef> compound_statement statement_sequence statement 
%type <ObjRef> open_statement closed_statement non_labeled_open_statement non_labeled_closed_statement
%type <ObjRef> closed_with_statement closed_if_statement closed_while_statement closed_for_statement
%type <ObjRef> open_with_statement open_if_statement open_while_statement open_for_statement
%type <ObjRef> assignment_statement procedure_statement goto_statement case_statement repeat_statement
%type <ObjRef> variable_access function_designator set_constructor 
%type <ObjRef> control_variable initial_value final_value direction
%type <ObjRef> record_variable_list
%type <ObjRef> proc_or_func_declaration_list proc_or_func_declaration 
%type <ObjRef> formal_parameter_list formal_parameter_section formal_parameter_section_list
%type <ObjRef> value_parameter_specification variable_parameter_specification 
%type <ObjRef> procedural_parameter_specification functional_parameter_specification
%type <ObjRef> procedure_declaration procedure_heading procedure_block
%type <ObjRef> function_declaration function_heading function_block function_identification
%type <ObjRef> colon semicolon directive result_type 
%type <ObjRef> new_type ordinal_type new_ordinal_type enumerated_type subrange_type 
%type <ObjRef> structured_type new_structured_type array_type record_type set_type file_type
%type <ObjRef> new_pointer_type component_type index_type index_list
%type <ObjRef> record_section_list record_section variant_part
%type <ObjRef> type_definition_list type_definition variant_selector variant_list variant
%type <ObjRef> tag_field tag_type
%type <ObjRef> case_constant_list case_constant case_index case_list_element
%type <ObjRef> base_type domain_type case_list_element_list
%type <ObjRef> procedure_identification indexed_variable field_designator 
%type <ObjRef> index_expression_list index_expression params actual_parameter_list actual_parameter
%type <ObjRef> otherwisepart nil member_designator_list member_designator comma var type_denoter

%token ARRAY ASSIGNMENT CASE COLON COMMA CONST 
%token DO DOT DOTDOT DOWNTO ELSE END EXTERNAL BFALSE FOR FORWARD FUNCTION
%token GOTO IF LBRAC LPAREN MINUS NIL NOT
%token OF OR OTHERWISE PACKED PBEGIN PFILE PLUS PROCEDURE PROGRAM RBRAC
%token RECORD REPEAT RPAREN SEMICOLON SET SLASH STAR STARSTAR THEN
%token TO BTRUE TYPE UNTIL VAR WHILE WITH IDENTIFIER

%token <Text> IDENTIFIER CHARACTER_STRING FORWARD EXTERNAL UPARROW TO DOWNTO COLON SEMICOLON COMMA VAR
%token <IntVal> DIGSEQ LABEL
%token <RealVal> REALNUMBER
%token <RelOp> EQUAL NOTEQUAL LT GT LE GE IN
%token <ArithmeticOp> PLUS MINUS OR STAR STARSTAR SLASH DIV MOD AND

%nonassoc THEN
%nonassoc ELSE 

%nonassoc AND OR LT LE GT GE EQ NE
%right NOT

%left '+' '-'
%left '*' '/'
%right UMINUS
%right UPLUS

/* Dummy tokens to set a precedence order among nonterminals where needed. */
%left HIGHER
%left LOWER

%%

file : program
{ /*file*/ ParserGCRoots::syntaxTree = gcnew Ast::FileNode(LineNum, AstNode($1)); }
 | module
{ /*file*/ ParserGCRoots::syntaxTree = gcnew Ast::FileNode(LineNum, AstNode($1)); }
 ;

program : program_heading semicolon block DOT
{ /*program*/ $$ = AddNode(gcnew Ast::ProgramNode(LineNum, AstNode($1), AstNode($3))); }
 ;

program_heading : PROGRAM identifier 
{ /*program_heading*/ $$ = AddNode(gcnew Ast::ProgramHeadingNode(LineNum, AstNode($2), 
		nullptr)); }
 | PROGRAM identifier LPAREN identifier_list RPAREN
{ /*program_heading*/ $$ = AddNode(gcnew Ast::ProgramHeadingNode(LineNum, AstNode($2), 
		gcnew IdentifierListNode(LineNum, AstNodeList($4)))); }
 ;

identifier_list : identifier_list comma formal_parameter 
{ /*identifier_list*/
	AstNodeList($1)->Add(AstNode($3)); 
	$$ = $1; 
}
 | formal_parameter { /*identifier_list*/ $$ = AddNodeList($1); }
 ;
 
formal_parameter : identifier
{ /*formal_parameter*/ $$ = AddNode(gcnew Ast::FormalParameterNode(LineNum, AstNode($1))); }
 ;
 
block : label_declaration_part
 constant_definition_part
 type_definition_part
 variable_declaration_part
 procedure_and_function_declaration_part
 statement_part 
 { /*block*/ 
	List<Node^>^ list = gcnew List<Node^>();
	list->Add(AstNode($1));
	list->Add(AstNode($2));
	list->Add(AstNode($3));
	list->Add(AstNode($4));
	list->Add(AstNode($5));
	list->Add(AstNode($6));
	$$ = AddNode(gcnew Ast::BlockNode(LineNum, list)); 
 }
 ;

module : constant_definition_part
 type_definition_part
 variable_declaration_part
 procedure_and_function_declaration_part 
{ /*module*/ 
	List<Node^>^ list = gcnew List<Node^>();
	list->Add(AstNode($1));
	list->Add(AstNode($2));
	list->Add(AstNode($3));
	list->Add(AstNode($4));	
	$$ = AddNode(gcnew Ast::ModuleNode(LineNum, list)); }
 ;

label_declaration_part : LABEL label_list semicolon
{ /*label_declaration_part*/ 
	$$ = AddNode(gcnew Ast::LabelDeclarationPartNode(LineNum, gcnew LabelListNode(LineNum, AstNodeList($2)))); 
}
 | /*epsilon*/ 
{ /*label_declaration_part*/ 
	$$ = AddNode(gcnew Ast::LabelDeclarationPartNode(LineNum, nullptr));
}
 ;

label_list : label_list comma label
{ /*label_list*/ 
	AstNodeList($1)->Add(AstNode($3)); 
    $$ = $1; 
}
 | label { /*label_list*/ $$ = AddNodeList($1); }
 ;

label : DIGSEQ
{ /*label*/ $$ = AddNode(gcnew Ast::LabelNode(LineNum, $1)); }
 ;

constant_definition_part : CONST constant_list
{ /*constant_definition_part*/ 
	$$ = AddNode(gcnew Ast::ConstantDefinitionPartNode(LineNum, gcnew ConstantListNode(LineNum, AstNodeList($2)))); 
}
 | /*epsilon*/ 
{ /*constant_definition_part*/ 
	$$ = AddNode(gcnew Ast::ConstantDefinitionPartNode(LineNum, nullptr));
}
 ;

constant_list : constant_list constant_definition
{ /*constant_list*/ 
	AstNodeList($1)->Add(AstNode($2)); 
    $$ = $1; 
}
 | constant_definition { /*constant_list*/ $$ = AddNodeList($1); }
 ;

constant_definition : identifier EQUAL cexpression semicolon
{  /*constant_definition*/
	$$ = AddNode(gcnew Ast::ConstantDefinitionNode(LineNum, AstNode($1), AstNode($3))); 
}
 ;

cexpression : csimple_expression 
{ /*cexpression*/ $$ = AddNode(gcnew Ast::ConstantExpressionNode(LineNum, AstNode($1))); }
 | csimple_expression relop csimple_expression
{ /*cexpression*/ $$ = AddNode(gcnew Ast::ConstantExpressionNode(LineNum, $2, AstNode($1), AstNode($3))); }
 ;

csimple_expression : cterm 
{ /*csimple_expression*/ $$ = AddNode(gcnew Ast::ConstantSimpleExpressionNode(LineNum, AstNode($1))); }
 | csimple_expression addop cterm
{ /*csimple_expression*/ $$ = AddNode(gcnew Ast::ConstantSimpleExpressionNode(LineNum, $2, AstNode($1), AstNode($3))); }
 ;

cterm : cfactor 
{ /*cterm*/ $$ = AddNode(gcnew Ast::ConstantTermNode(LineNum, AstNode($1))); }
 | cterm mulop cfactor
{ /*cterm*/ $$ = AddNode(gcnew Ast::ConstantTermNode(LineNum, $2, AstNode($1), AstNode($3))); }
 ;

cfactor : sign cfactor
{ /*cfactor*/ $$ = AddNode(gcnew Ast::ConstantFactorNode(LineNum, $1, AstNode($2))); }
 | cexponentiation
{ /*cfactor*/ $$ = AddNode(gcnew Ast::ConstantFactorNode(LineNum, AstNode($1))); }
 ;

cexponentiation : cprimary 
{ /*cexponentiation*/ $$ = AddNode(gcnew Ast::ConstantExponentiationNode(LineNum, AstNode($1))); }
 | cprimary STARSTAR cexponentiation
{ /*cexponentiation*/ $$ = AddNode(gcnew Ast::ConstantExponentiationNode(LineNum, AstNode($1), AstNode($3))); }
 ;

cprimary : identifier
{ /*cprimary*/ $$ = AddNode(gcnew Ast::ConstantPrimaryNode(LineNum, AstNode($1))); }
 | LPAREN cexpression RPAREN
{ /*cprimary*/ $$ = AddNode(gcnew Ast::ConstantPrimaryNode(LineNum, AstNode($2))); }
 | unsigned_constant
{ /*cprimary*/ $$ = AddNode(gcnew Ast::ConstantPrimaryNode(LineNum, AstNode($1))); }
 | NOT cprimary
{ /*cprimary*/ $$ = AddNode(gcnew Ast::ConstantPrimaryNode(LineNum, AstNode($2))); }
 ;

constant : non_string
{ /*constant*/ $$ = AddNode(gcnew Ast::ConstantNode(LineNum, AstNode($1))); }
 | sign non_string
{ /*constant*/ $$ = AddNode(gcnew Ast::ConstantNode(LineNum, $1, AstNode($2))); }
 | CHARACTER_STRING 
{ /*constant*/ $$ = AddNode(gcnew Ast::ConstantNode(LineNum, gcnew CharacterStringNode(LineNum, $1.tokenStart, $1.tokenLength))); }
 ;

sign : PLUS
{ $$ = PLUS; }
 | MINUS
{ $$ = MINUS; }
 ;

non_string : DIGSEQ { /*non_string*/ $$ = AddNode(gcnew Ast::NonStringNode(LineNum, (int)$1)); }
 | identifier		{ /*non_string*/ $$ = AddNode(gcnew Ast::NonStringNode(LineNum, AstNode($1))); }
 | REALNUMBER		{ /*non_string*/ $$ = AddNode(gcnew Ast::NonStringNode(LineNum, (double)$1)); }
 ;
 
type_definition_part : TYPE type_definition_list
{ /*type_definition_part*/ $$ = AddNode(gcnew Ast::TypeDefinitionPartNode(LineNum, 
		gcnew TypeDefinitionListNode(LineNum, AstNodeList($2)))); }
 | /*epsilon*/ 
{ /*type_definition_part*/ $$ = AddNode(gcnew Ast::TypeDefinitionPartNode(LineNum, 
		nullptr)); }
 ;

type_definition_list : type_definition_list type_definition
{ /*type_definition_list*/ 
	AstNodeList($1)->Add(AstNode($2)); 
    $$ = $1; 
}
 | type_definition { /*type_definition_list*/ $$ = AddNodeList($1); }
 ;

type_definition : identifier EQUAL type_denoter semicolon
{ /*type_definition*/ $$ = AddNode(gcnew Ast::TypeDefinitionNode(LineNum, AstNode($1), AstNode($3))); }
 ;

type_denoter : identifier
{ /*type_denoter*/
	$$ = AddNode(gcnew Ast::TypeDenoterNode(LineNum, AstNode($1)));
}
 | new_type 
{ /*type_denoter*/
	$$ = AddNode(gcnew Ast::TypeDenoterNode(LineNum, AstNode($1)));
} 
 ;

new_type : new_ordinal_type
{ /*new_type*/
	$$ = AddNode(gcnew Ast::NewTypeNode(LineNum, AstNode($1)));
}
 | new_structured_type
{ /*new_type*/
	$$ = AddNode(gcnew Ast::NewTypeNode(LineNum, AstNode($1)));
} 
 | new_pointer_type
{ /*new_type*/
	$$ = AddNode(gcnew Ast::NewTypeNode(LineNum, AstNode($1)));
} 
 ;

new_ordinal_type : enumerated_type
{ /*new_type*/
	$$ = AddNode(gcnew Ast::NewOrdinalTypeNode(LineNum, AstNode($1)));
}
 | subrange_type
{ /*new_type*/
	$$ = AddNode(gcnew Ast::NewOrdinalTypeNode(LineNum, AstNode($1)));
} 
 ;

enumerated_type : LPAREN identifier_list RPAREN
{ /*enumerated_type*/
	$$ = AddNode(gcnew Ast::EnumeratedTypeNode(LineNum, gcnew IdentifierListNode(LineNum, AstNodeList($2))));
}
 ;

subrange_type : constant DOTDOT constant
{ /*subrange_type*/
	$$ = AddNode(gcnew Ast::SubrangeTypeNode(LineNum, AstNode($1), AstNode($3)));
}
 ;

new_structured_type : structured_type
{ /*new_structured_type*/
	$$ = AddNode(gcnew Ast::NewStructuredTypeNode(LineNum, AstNode($1), false));
}
 | PACKED structured_type
{ /*new_structured_type*/
	$$ = AddNode(gcnew Ast::NewStructuredTypeNode(LineNum, AstNode($2), true));
}
 ;

structured_type : array_type
{ /*structured_type*/
	$$ = AddNode(gcnew Ast::StructuredTypeNode(LineNum, AstNode($1)));
}
 | record_type
{ /*structured_type*/
	$$ = AddNode(gcnew Ast::StructuredTypeNode(LineNum, AstNode($1)));
}
 | set_type
{ /*structured_type*/
	$$ = AddNode(gcnew Ast::StructuredTypeNode(LineNum, AstNode($1)));
} 
 | file_type
{ /*structured_type*/
	$$ = AddNode(gcnew Ast::StructuredTypeNode(LineNum, AstNode($1)));
} 
 ;

array_type : ARRAY LBRAC index_list RBRAC OF component_type
{ /*array_type*/
	$$ = AddNode(gcnew Ast::ArrayTypeNode(LineNum, gcnew IndexListNode(LineNum, AstNodeList($3)), AstNode($6)));
}
 ;

index_list : index_list comma index_type
{ /*index_list*/
	AstNodeList($1)->Add(AstNode($3)); 
	$$ = $1; 
}
 | index_type { /*index_list*/ $$ = AddNodeList($1); }
 ;

index_type : ordinal_type 
{ /*index_type*/
	$$ = AddNode(gcnew Ast::IndexTypeNode(LineNum, AstNode($1)));
} 
 ;

ordinal_type : new_ordinal_type
{ /*ordinal_type*/
	$$ = AddNode(gcnew Ast::OrdinalTypeNode(LineNum, AstNode($1)));
} 
 | identifier
{ /*ordinal_type*/
	$$ = AddNode(gcnew Ast::OrdinalTypeNode(LineNum, AstNode($1)));
}  
 ;

component_type : type_denoter 
{ /*component_type*/
	$$ = AddNode(gcnew Ast::ComponentTypeNode(LineNum, AstNode($1)));
} 
 ;

record_type : RECORD record_section_list END
{ /*record_type*/
	$$ = AddNode(gcnew Ast::RecordTypeNode(LineNum, gcnew RecordSectionListNode(LineNum, AstNodeList($2)), nullptr));
} 
 | RECORD record_section_list semicolon variant_part END
{ /*record_type*/
	$$ = AddNode(gcnew Ast::RecordTypeNode(LineNum, gcnew RecordSectionListNode(LineNum, AstNodeList($2)), AstNode($4)));
}  
 | RECORD variant_part END
{ /*record_type*/
	$$ = AddNode(gcnew Ast::RecordTypeNode(LineNum, nullptr, AstNode($2)));
}  
 ;

record_section_list : record_section_list semicolon record_section
{ /*record_section_list*/
	AstNodeList($1)->Add(AstNode($3)); 
	$$ = $1; 
}
 | record_section { /*record_section_list*/ $$ = AddNodeList($1); }
 ;

record_section : identifier_list colon type_denoter
{ /*record_section*/
	$$ = AddNode(gcnew Ast::RecordSectionNode(LineNum, gcnew IdentifierListNode(LineNum, AstNodeList($1)), AstNode($3)));
}
 ;

variant_part : CASE variant_selector OF variant_list semicolon
{ /*variant_part*/
	$$ = AddNode(gcnew Ast::VariantPartNode(LineNum, AstNode($2), 
		gcnew VariantListNode(LineNum, AstNodeList($4)), true));
}
 | CASE variant_selector OF variant_list
{ /*variant_part*/
	$$ = AddNode(gcnew Ast::VariantPartNode(LineNum, AstNode($2), 
		gcnew VariantListNode(LineNum, AstNodeList($4)), false));
} 
 | /*epsilon*/
{ /*variant_part*/
	$$ = AddNode(gcnew Ast::VariantPartNode(LineNum, nullptr, 
		nullptr, false));
} 
 ;

variant_selector : tag_field COLON tag_type
{ /*variant_selector*/
	$$ = AddNode(gcnew Ast::VariantSelectorNode(LineNum, AstNode($1), AstNode($3))); 
}
 | tag_type
{ /*variant_selector*/
	$$ = AddNode(gcnew Ast::VariantSelectorNode(LineNum, nullptr, AstNode($1))); 
}
 ;

variant_list : variant_list semicolon variant
{ /*variant_list*/
	AstNodeList($1)->Add(AstNode($3)); 
	$$ = $1; 
}
 | variant { /*variant_list*/ $$ = AddNodeList($1); }
 ;

variant : case_constant_list COLON LPAREN record_section_list RPAREN
{ /*variant*/
	$$ = AddNode(gcnew Ast::VariantNode(LineNum, gcnew CaseConstantListNode(LineNum, AstNodeList($1)), 
		gcnew RecordSectionListNode(LineNum, AstNodeList($4)), nullptr)); 
}
 | case_constant_list COLON LPAREN record_section_list semicolon variant_part RPAREN
{ /*variant*/
	$$ = AddNode(gcnew Ast::VariantNode(LineNum, gcnew CaseConstantListNode(LineNum, AstNodeList($1)), 
		gcnew RecordSectionListNode(LineNum, AstNodeList($4)), AstNode($6)));
}
 | case_constant_list COLON LPAREN variant_part RPAREN
{ /*variant*/
	$$ = AddNode(gcnew Ast::VariantNode(LineNum, gcnew CaseConstantListNode(LineNum, AstNodeList($1)), 
		nullptr, AstNode($4)));
} 
 ;

case_constant_list : case_constant_list comma case_constant
{ /*case_constant_list*/
	AstNodeList($1)->Add(AstNode($3)); 
	$$ = $1; 
}
 | case_constant { /*case_constant_list*/ $$ = AddNodeList($1); }
 ;

case_constant : constant
{ /*case_constant*/
	$$ = AddNode(gcnew Ast::CaseConstantNode(LineNum, AstNode($1), nullptr)); 
}
 | constant DOTDOT constant
{ /*case_constant*/
	$$ = AddNode(gcnew Ast::CaseConstantNode(LineNum, AstNode($1), AstNode($3))); 
} 
 ;

tag_field : identifier 
{ /*tag_field*/
	$$ = AddNode(gcnew Ast::TagFieldNode(LineNum, AstNode($1))); 
}
 ;

tag_type : identifier 
{ /*tag_type*/
	$$ = AddNode(gcnew Ast::TagTypeNode(LineNum, AstNode($1))); 
}
 ;

set_type : SET OF base_type
{ /*set_type*/
	$$ = AddNode(gcnew Ast::SetTypeNode(LineNum, AstNode($3))); 
}
 ;

base_type : ordinal_type
{ /*base_type*/
	$$ = AddNode(gcnew Ast::BaseTypeNode(LineNum, AstNode($1))); 
}
 ;

file_type : PFILE OF component_type
{ /*file_type*/
	$$ = AddNode(gcnew Ast::FileTypeNode(LineNum, AstNode($3))); 
}
 ;

new_pointer_type : UPARROW domain_type
{ /*new_pointer_type*/
	$$ = AddNode(gcnew Ast::NewPointerTypeNode(LineNum, $1.tokenStart[0], AstNode($2))); 
}
 ;

domain_type : identifier 
{ /*domain_type*/
	$$ = AddNode(gcnew Ast::DomainTypeNode(LineNum, AstNode($1))); 
}
 ;

variable_declaration_part : var variable_declaration_list semicolon 
{ /*variable_declaration_part*/ 
	$$ = AddNode(gcnew Ast::VariableDeclarationPartNode(LineNum, 
		gcnew VariableDeclarationListNode(LineNum, AstNodeList($2))));
}
 | /*epsilon*/ 
{ /*variable_declaration_part*/ 
	$$ = AddNode(gcnew Ast::VariableDeclarationPartNode(LineNum, 
		nullptr)); 
}
 ;

variable_declaration_list :
	variable_declaration_list semicolon variable_declaration
{ /*variable_declaration_list*/ 
	AstNodeList($1)->Add(AstNode($3)); 
	$$ = $1; 
}
 | variable_declaration 
{ /*variable_declaration_list*/ $$ = AddNodeList($1); }
 ;

variable_declaration : identifier COLON type_denoter
{ /*variable_declaration*/
	$$ = AddNode(gcnew Ast::VariableDeclarationNode(LineNum, AstNode($1), AstNode($3))); 
}
 | identifier_list COLON type_denoter 
{ /*variable_declaration*/
	$$ = AddNode(gcnew Ast::VariableDeclarationNode(LineNum, gcnew IdentifierListNode(LineNum, AstNodeList($1)), AstNode($3))); 
} 
 ;
 
procedure_and_function_declaration_part : proc_or_func_declaration_list semicolon
{ /*procedure_and_function_declaration_part*/ 
	$$ = AddNode(gcnew Ast::ProcedureAndFunctionDeclarationPartNode(LineNum, 
		gcnew ProcedureOrFunctionDeclarationListNode(LineNum, AstNodeList($1))));
}
 | /*epsilon*/ 
{ /*procedure_and_function_declaration_part*/ 
	$$ = AddNode(gcnew Ast::ProcedureAndFunctionDeclarationPartNode(LineNum, 
		nullptr));
}
 ;

proc_or_func_declaration_list :
   proc_or_func_declaration_list semicolon proc_or_func_declaration
{ /*proc_or_func_declaration_list*/ 
	AstNodeList($1)->Add(AstNode($3)); 
	$$ = $1; 
}
 | proc_or_func_declaration
{ /*proc_or_func_declaration_list*/ 
	$$ = AddNodeList($1); 
}
 ;

proc_or_func_declaration : procedure_declaration
{ /*proc_or_func_declaration*/ 
	$$ = AddNode(gcnew Ast::ProcedureOrFunctionDeclarationNode(LineNum, AstNode($1)));
}
 | function_declaration
{ /*proc_or_func_declaration*/ 
	$$ = AddNode(gcnew Ast::ProcedureOrFunctionDeclarationNode(LineNum, AstNode($1)));
}
 ;

procedure_declaration : procedure_heading semicolon directive
{ /*procedure_declaration*/ 
	$$ = AddNode(gcnew Ast::ProcedureDeclarationNode(LineNum, AstNode($1), AstNode($3))); 
}
 | procedure_heading semicolon procedure_block
{ /*procedure_declaration*/ 
	$$ = AddNode(gcnew Ast::ProcedureDeclarationNode(LineNum, AstNode($1), AstNode($3))); 
}
 ;

procedure_heading : procedure_identification
{ /*procedure_heading*/ 
	$$ = AddNode(gcnew Ast::ProcedureHeadingNode(LineNum, AstNode($1), nullptr));
}
 | procedure_identification formal_parameter_list
{ /*procedure_heading*/ 
	$$ = AddNode(gcnew Ast::ProcedureHeadingNode(LineNum, AstNode($1), AstNode($2)));
}
 ;

directive : FORWARD
{ /*directive*/
	$$ = AddNode(gcnew Ast::DirectiveNode(LineNum, $1.tokenStart, $1.tokenLength));
}
 | EXTERNAL
{ /*directive*/
	$$ = AddNode(gcnew Ast::DirectiveNode(LineNum, $1.tokenStart, $1.tokenLength));
}
 ;

formal_parameter_list : LPAREN formal_parameter_section_list RPAREN 
{ /*formal_parameter_list*/	
	$$ = AddNode(gcnew Ast::FormalParameterListNode(LineNum, 
		gcnew FormalParameterSectionListNode(LineNum, AstNodeList($2)))); 
}
 ;
 
formal_parameter_section_list : formal_parameter_section_list semicolon formal_parameter_section
{ /*formal_parameter_section_list*/
	AstNodeList($1)->Add(AstNode($3)); 
	$$ = $1; 
}
 | formal_parameter_section { /*formal_parameter_section_list*/ $$ = AddNodeList($1); }
 ;

formal_parameter_section : 
   value_parameter_specification
{ /*formal_parameter_section*/	
	$$ = AddNode(gcnew Ast::FormalParameterSectionNode(LineNum, AstNode($1))); 
}
 | variable_parameter_specification
{ /*formal_parameter_section*/	
	$$ = AddNode(gcnew Ast::FormalParameterSectionNode(LineNum, AstNode($1))); 
} 
 | procedural_parameter_specification
{ /*formal_parameter_section*/	
	$$ = AddNode(gcnew Ast::FormalParameterSectionNode(LineNum, AstNode($1))); 
}
 | functional_parameter_specification
{ /*formal_parameter_section*/	
	$$ = AddNode(gcnew Ast::FormalParameterSectionNode(LineNum, AstNode($1))); 
} 
 ;

value_parameter_specification : identifier_list COLON identifier
{ /*value_parameter_specification*/	
	$$ = AddNode(gcnew Ast::ValueParameterSpecificationNode(LineNum, 
		gcnew IdentifierListNode(LineNum, AstNodeList($1)), AstNode($3)));
}
 ;

variable_parameter_specification : VAR identifier_list COLON identifier
{ /*variable_parameter_specification*/	
	$$ = AddNode(gcnew Ast::VariableParameterSpecificationNode(LineNum, 
		gcnew IdentifierListNode(LineNum, AstNodeList($2)), AstNode($4)));
}
 ;

procedural_parameter_specification : procedure_heading 
{ /*procedural_parameter_specification*/	
	$$ = AddNode(gcnew Ast::ProceduralParameterSpecificationNode(LineNum, AstNode($1)));
}
 ;

functional_parameter_specification : function_heading 
{ /*functional_parameter_specification*/	
	$$ = AddNode(gcnew Ast::FunctionalParameterSpecificationNode(LineNum, AstNode($1)));
}
 ;

procedure_identification : PROCEDURE identifier 
{ /*procedure_identification*/
	$$ = AddNode(gcnew Ast::ProcedureIdentificationNode(LineNum, AstNode($2))); 
}
;

procedure_block : block 
{ /*procedure_block*/ 
	$$ = AddNode(gcnew Ast::ProcedureBlockNode(LineNum, AstNode($1))); 
}
 ;

function_declaration : function_heading semicolon directive
{ /*function_declaration*/ 
	$$ = AddNode(gcnew Ast::FunctionDeclarationNode(LineNum, AstNode($1), AstNode($3))); 
}
 | function_identification semicolon function_block
{ /*function_declaration*/ 
	$$ = AddNode(gcnew Ast::FunctionDeclarationNode(LineNum, AstNode($1), AstNode($3))); 
}
 | function_heading semicolon function_block
{ /*function_declaration*/ 
	$$ = AddNode(gcnew Ast::FunctionDeclarationNode(LineNum, AstNode($1), AstNode($3))); 
}
 ;

function_heading : FUNCTION identifier COLON result_type
{ /*function_heading*/ 
	$$ = AddNode(gcnew Ast::FunctionHeadingNode(LineNum, AstNode($2), 
		nullptr, AstNode($4)));
}
 | FUNCTION identifier formal_parameter_list COLON result_type
{ /*function_heading*/ 
	$$ = AddNode(gcnew Ast::FunctionHeadingNode(LineNum, AstNode($2), 
		AstNode($3), AstNode($5)));
}
 ;

result_type : identifier 
{ /*result_type*/
	$$ = AddNode(gcnew Ast::ResultTypeNode(LineNum, AstNode($1)));
}
 ;

function_identification : FUNCTION identifier 
{ /*function_identification*/ 
	$$ = AddNode(gcnew Ast::FunctionIdentificationNode(LineNum, AstNode($2)));
}
 ;

function_block : block 
{ /*function_block*/ 
	$$ = AddNode(gcnew Ast::FunctionBlockNode(LineNum, AstNode($1)));
}
 ;

statement_part : compound_statement 
{ /*statement_part*/ 
	$$ = AddNode(gcnew Ast::StatementPartNode(LineNum, AstNode($1))); 
}
 ;

compound_statement : PBEGIN statement_sequence END
{ /*compound_statement*/ 
	$$ = AddNode(gcnew Ast::CompoundStatementNode(LineNum, gcnew StatementSequenceNode(LineNum, AstNodeList($2)))); 
}
 ;
 
statement_sequence : statement_sequence semicolon statement
{ /*statement_sequence*/ 
	AstNodeList($1)->Add(AstNode($3)); 
    $$ = $1; 
}
 | statement { /*statement_sequence*/ $$ = AddNodeList($1); }
 ;
 
statement : open_statement
{ /*statement*/ $$ = AddNode(gcnew Ast::StatementNode(LineNum, AstNode($1))); }
 | closed_statement
{ /*statement*/ $$ = AddNode(gcnew Ast::StatementNode(LineNum, AstNode($1))); }
 ;

open_statement : label COLON non_labeled_open_statement
{ /*open_statement*/
	$$ = AddNode(gcnew Ast::OpenStatementNode(LineNum, AstNode($1), AstNode($3)));
}
 | non_labeled_open_statement 
{ /*open_statement*/ 
	$$ = AddNode(gcnew Ast::OpenStatementNode(LineNum, nullptr, AstNode($1)));
}
 ;

closed_statement : label COLON non_labeled_closed_statement
{ /*closed_statement*/
	$$ = AddNode(gcnew Ast::ClosedStatementNode(LineNum, AstNode($1), AstNode($3)));
}
 | non_labeled_closed_statement 
{ /*closed_statement*/ 
	$$ = AddNode(gcnew Ast::ClosedStatementNode(LineNum, nullptr, AstNode($1)));
}
 ;

non_labeled_closed_statement : assignment_statement		
{ /*non_labeled_closed_statement*/ $$ = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode($1))); }
 | procedure_statement
{ /*non_labeled_closed_statement*/ $$ = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode($1))); }
 | goto_statement
{ /*non_labeled_closed_statement*/ $$ = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode($1))); }
 | compound_statement
{ /*non_labeled_closed_statement*/ $$ = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode($1))); }
 | case_statement
{ /*non_labeled_closed_statement*/ $$ = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode($1))); }
 | repeat_statement
{ /*non_labeled_closed_statement*/ $$ = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode($1))); }
 | closed_with_statement
{ /*non_labeled_closed_statement*/ $$ = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode($1))); }
 | closed_if_statement
{ /*non_labeled_closed_statement*/ $$ = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode($1))); }
 | closed_while_statement
{ /*non_labeled_closed_statement*/ $$ = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode($1))); }
 | closed_for_statement
{ /*non_labeled_closed_statement*/ $$ = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, AstNode($1))); }
 | /*epsilon*/
{ /*non_labeled_closed_statement*/ $$ = AddNode(gcnew Ast::NonLabeledClosedStatementNode(LineNum, nullptr)); }
 ;

non_labeled_open_statement : open_with_statement		
{ /*non_labeled_open_statement*/ $$ = AddNode(gcnew Ast::NonLabeledOpenStatementNode(LineNum, AstNode($1))); }
 | open_if_statement
{ /*non_labeled_open_statement*/ $$ = AddNode(gcnew Ast::NonLabeledOpenStatementNode(LineNum, AstNode($1))); }
 | open_while_statement
{ /*non_labeled_open_statement*/ $$ = AddNode(gcnew Ast::NonLabeledOpenStatementNode(LineNum, AstNode($1))); }
 | open_for_statement
{ /*non_labeled_open_statement*/ $$ = AddNode(gcnew Ast::NonLabeledOpenStatementNode(LineNum, AstNode($1))); }
 ;

repeat_statement : REPEAT statement_sequence UNTIL boolean_expression
{ /*repeat_statement*/
	$$ = AddNode(gcnew Ast::RepeatStatementNode(LineNum, 
		gcnew StatementSequenceNode(LineNum, AstNodeList($2)), AstNode($4)));
}
 ;

open_while_statement : WHILE boolean_expression DO open_statement
{ /*open_while_statement*/
	$$ = AddNode(gcnew Ast::OpenWhileStatementNode(LineNum, AstNode($2), AstNode($4)));
}
 ;

closed_while_statement : WHILE boolean_expression DO closed_statement
{ /*closed_while_statement*/
	$$ = AddNode(gcnew Ast::ClosedWhileStatementNode(LineNum, AstNode($2), AstNode($4)));
}
 ;

open_for_statement : FOR control_variable ASSIGNMENT initial_value direction
   final_value DO open_statement
{ /*open_for_statement*/
	List<Node^>^ list = gcnew List<Node^>();
	list->Add(AstNode($2));
	list->Add(AstNode($4));
	list->Add(AstNode($5));
	list->Add(AstNode($6));
	list->Add(AstNode($8));
	$$ = AddNode(gcnew Ast::OpenForStatementNode(LineNum, list)); 
}
 ;

closed_for_statement : FOR control_variable ASSIGNMENT initial_value direction
   final_value DO closed_statement
{ /*closed_for_statement*/
	List<Node^>^ list = gcnew List<Node^>();
	list->Add(AstNode($2));
	list->Add(AstNode($4));
	list->Add(AstNode($5));
	list->Add(AstNode($6));
	list->Add(AstNode($8));
	$$ = AddNode(gcnew Ast::ClosedForStatementNode(LineNum, list)); 
}
 ;

open_with_statement : WITH record_variable_list DO open_statement
{ /*open_with_statement*/
	$$ = AddNode(gcnew Ast::OpenWithStatementNode(LineNum, 
		gcnew Ast::RecordVariableListNode(LineNum, AstNodeList($2)), AstNode($4))); 
}
 ;

closed_with_statement : WITH record_variable_list DO closed_statement
{ /*closed_with_statement*/
	$$ = AddNode(gcnew Ast::ClosedWithStatementNode(LineNum, 
		gcnew Ast::RecordVariableListNode(LineNum, AstNodeList($2)), AstNode($4))); 
}
 ;

open_if_statement : IF boolean_expression THEN statement
{ /*open_if_statement*/
	$$ = AddNode(gcnew Ast::OpenIfStatementNode(LineNum, AstNode($2), AstNode($4), nullptr)); 
}
 | IF boolean_expression THEN closed_statement ELSE open_statement
{ /*open_if_statement*/
	$$ = AddNode(gcnew Ast::OpenIfStatementNode(LineNum, AstNode($2), AstNode($4), AstNode($6))); 
}
 ;

closed_if_statement : IF boolean_expression THEN closed_statement
   ELSE closed_statement
{ /*closed_if_statement*/
	$$ = AddNode(gcnew Ast::ClosedIfStatementNode(LineNum, AstNode($2), AstNode($4), AstNode($6))); 
}   
 ;

assignment_statement : variable_access ASSIGNMENT expression 
{ /*assignment_statement*/
	$$ = AddNode(gcnew Ast::AssignmentStatementNode(LineNum, AstNode($1), AstNode($3))); 
}  	
 ;

variable_access : identifier
{ /*variable_access*/
	$$ = AddNode(gcnew Ast::VariableAccessNode(LineNum, 0, AstNode($1)));
}
 | indexed_variable
{ /*variable_access*/
	$$ = AddNode(gcnew Ast::VariableAccessNode(LineNum, 0, AstNode($1)));
}
 | field_designator
{ /*variable_access*/
	$$ = AddNode(gcnew Ast::VariableAccessNode(LineNum, 0, AstNode($1)));
}
 | variable_access UPARROW
{ /*variable_access*/
	$$ = AddNode(gcnew Ast::VariableAccessNode(LineNum, $2.tokenStart[0], AstNode($1)));
}
 ;

indexed_variable : variable_access LBRAC index_expression_list RBRAC
{ /*indexed_variable*/ 
	$$ = AddNode(gcnew Ast::IndexedVariableNode(LineNum, AstNode($1), 
		gcnew IndexExpressionListNode(LineNum, AstNodeList($3))));
}
 ;

index_expression_list : index_expression_list comma index_expression
{ /*index_expression_list*/ 
	AstNodeList($1)->Add(AstNode($3)); 
    $$ = $1; 
}
 | index_expression { /*index_expression_list*/ $$ = AddNodeList($1); }
 ;
 
index_expression : expression
{ /*index_expression*/ 
	$$ = AddNode(gcnew Ast::IndexExpressionNode(LineNum, AstNode($1)));
} 
 ;

field_designator : variable_access DOT identifier
{ /*field_designator*/ 
	$$ = AddNode(gcnew Ast::FieldDesignatorNode(LineNum, AstNode($1), AstNode($3)));
} 
 ;

procedure_statement : identifier params
{ /*procedure_statement*/ 
	$$ = AddNode(gcnew Ast::ProcedureStatementNode(LineNum, AstNode($1), AstNode($2)));
} 
 | identifier
{ /*procedure_statement*/ 
	$$ = AddNode(gcnew Ast::ProcedureStatementNode(LineNum, AstNode($1), nullptr));
}
 ;

params : LPAREN actual_parameter_list RPAREN 
{ /*params*/ 
	$$ = AddNode(gcnew Ast::ParamsNode(LineNum, gcnew ActualParameterListNode(LineNum, AstNodeList($2))));
}
 ;

actual_parameter_list : actual_parameter_list comma actual_parameter
{ /*actual_parameter_list*/ 
	AstNodeList($1)->Add(AstNode($3)); 
    $$ = $1; 
}
 | actual_parameter { /*actual_parameter_list*/ $$ = AddNodeList($1); }
 ;

/*
 * this forces you to check all this to be sure that only write and
 * writeln use the 2nd and 3rd forms, you really can't do it easily in
 * the grammar, especially since write and writeln aren't reserved
 */
actual_parameter : expression
{ /*actual_parameter*/ 	
    $$ = AddNode(gcnew Ast::ActualParameterNode(LineNum, AstNode($1), nullptr, nullptr));
}
 | expression COLON expression
{ /*actual_parameter*/ 	
    $$ = AddNode(gcnew Ast::ActualParameterNode(LineNum, AstNode($1), AstNode($3), nullptr));
} 
 | expression COLON expression COLON expression
{ /*actual_parameter*/ 	
    $$ = AddNode(gcnew Ast::ActualParameterNode(LineNum, AstNode($1), AstNode($3), AstNode($5)));
} 
 ;

goto_statement : GOTO label
{ /*goto_statement*/
	$$ = AddNode(gcnew Ast::GotoStatementNode(LineNum, AstNode($2)));
}
 ;

case_statement : CASE case_index OF case_list_element_list END
{ /*case_statement*/
	$$ = AddNode(gcnew Ast::CaseStatementNode(LineNum, AstNode($2), 
		gcnew CaseListElementListNode(LineNum, AstNodeList($4)), false, nullptr, nullptr, false));
}
 | CASE case_index OF case_list_element_list SEMICOLON END
{ /*case_statement*/
	$$ = AddNode(gcnew Ast::CaseStatementNode(LineNum, AstNode($2), 
		gcnew CaseListElementListNode(LineNum, AstNodeList($4)), true, nullptr, nullptr, false));
} 
 | CASE case_index OF case_list_element_list SEMICOLON
   otherwisepart statement END
{ /*case_statement*/
	$$ = AddNode(gcnew Ast::CaseStatementNode(LineNum, AstNode($2), 
		gcnew CaseListElementListNode(LineNum, AstNodeList($4)), true, AstNode($6), AstNode($7), false));
}    
 | CASE case_index OF case_list_element_list SEMICOLON
   otherwisepart statement SEMICOLON END
{ /*case_statement*/
	$$ = AddNode(gcnew Ast::CaseStatementNode(LineNum, AstNode($2), 
		gcnew CaseListElementListNode(LineNum, AstNodeList($4)), true, AstNode($6), AstNode($7), true));
}    
 ;

case_index : expression 
{ /*case_index*/
	$$ = AddNode(gcnew Ast::CaseIndexNode(LineNum, AstNode($1)));
}  
 ;

case_list_element_list : case_list_element_list semicolon case_list_element
{ /*case_list_element_list*/ 
	AstNodeList($1)->Add(AstNode($3)); 
    $$ = $1; 
}
 | case_list_element { /*case_list_element_list*/ $$ = AddNodeList($1); }
 ;
 
case_list_element : case_constant_list COLON statement
{ /*case_index*/
	$$ = AddNode(gcnew Ast::CaseListElementNode(LineNum, 
		gcnew CaseConstantListNode(LineNum, AstNodeList($1)), AstNode($3)));
} 
 ;

otherwisepart : OTHERWISE
{ /*otherwisepart*/
	$$ = AddNode(gcnew Ast::OtherwisePartNode(LineNum, false));
}
 | OTHERWISE COLON
{ /*otherwisepart*/
	$$ = AddNode(gcnew Ast::OtherwisePartNode(LineNum, true));
}
 ;

control_variable : identifier 
{ /*control_variable*/
	$$ = AddNode(gcnew Ast::ControlVariableNode(LineNum, AstNode($1)));
} 
 ;

initial_value : expression 
{ /*initial_value*/
	$$ = AddNode(gcnew Ast::InitialValueNode(LineNum, AstNode($1)));
} 
 ;

direction : TO
{ /*direction*/
	$$ = AddNode(gcnew Ast::DirectionNode(LineNum, $1.tokenStart, $1.tokenLength)); 
}
 | DOWNTO
{ /*direction*/
	$$ = AddNode(gcnew Ast::DirectionNode(LineNum, $1.tokenStart, $1.tokenLength)); 
}
 ;

final_value : expression 
{ /*final_value*/
	$$ = AddNode(gcnew Ast::FinalValueNode(LineNum, AstNode($1))); 
}
 ;

record_variable_list : record_variable_list comma variable_access
{ /*record_variable_list*/ 
	AstNodeList($1)->Add(AstNode($3)); 
    $$ = $1; 
}
 | variable_access { /*record_variable_list*/ $$ = AddNodeList($1); }
 ;
  
boolean_expression : expression 
{ /*boolean_expression*/ 
	$$ = AddNode(gcnew Ast::BooleanExpressionNode(LineNum, AstNode($1))); 
} 
 ;

expression : simple_expression 
{ /*expression*/ 
	$$ = AddNode(gcnew Ast::ExpressionNode(LineNum, AstNode($1))); 
}
 | simple_expression relop simple_expression
{ /*expression*/
	$$ = AddNode(gcnew Ast::ExpressionNode(LineNum, $2, AstNode($1), AstNode($3)));
}
 ;

simple_expression : term 
{ /*simple_expression*/ 
	$$ = AddNode(gcnew Ast::SimpleExpressionNode(LineNum, AstNode($1))); 
}
 | simple_expression addop term
{ /*simple_expression*/
	$$ = AddNode(gcnew Ast::SimpleExpressionNode(LineNum, $2, AstNode($1), AstNode($3))); 
}
 ;

term : factor
{ /*term*/ 
	$$ = AddNode(gcnew Ast::TermNode(LineNum, AstNode($1))); 
}
 | term mulop factor
{ /*term*/
	$$ = AddNode(gcnew Ast::TermNode(LineNum, $2, AstNode($1), AstNode($3))); 
} 
 ;

factor : sign factor
{ /*factor*/ $$ = AddNode(gcnew Ast::FactorNode(LineNum, $1, AstNode($2))); 
}
 | exponentiation
{ /*factor*/ $$ = AddNode(gcnew Ast::FactorNode(LineNum, AstNode($1))); 
}
 ;

exponentiation : primary 
{ /*exponentiation*/
	$$ = AddNode(gcnew Ast::ExponentiationNode(LineNum, AstNode($1))); 
}
 | primary STARSTAR exponentiation 
{ /*exponentiation*/
	$$ = AddNode(gcnew Ast::ExponentiationNode(LineNum, AstNode($1), AstNode($3))); 
}
 ;

primary : variable_access		
{ /*primary*/ $$ = AddNode(gcnew Ast::PrimaryNode(LineNum, AstNode($1))); }
 | unsigned_constant
{ /*primary*/ $$ = AddNode(gcnew Ast::PrimaryNode(LineNum, AstNode($1))); }
 | function_designator
{ /*primary*/ $$ = AddNode(gcnew Ast::PrimaryNode(LineNum, AstNode($1))); }
 | set_constructor
{ /*primary*/ $$ = AddNode(gcnew Ast::PrimaryNode(LineNum, AstNode($1))); }
 | LPAREN expression RPAREN
{ /*primary*/ $$ = AddNode(gcnew Ast::PrimaryNode(LineNum, AstNode($2))); }
 | NOT primary
{ /*primary*/ $$ = AddNode(gcnew Ast::PrimaryNode(LineNum, AstNode($2))); } 
 ;
 
unsigned_constant : unsigned_number
{ /*unsigned_constant*/ $$ = AddNode(gcnew Ast::UnsignedConstantNode(LineNum, AstNode($1))); }
 | CHARACTER_STRING
{ /*unsigned_constant*/ $$ = AddNode(gcnew Ast::UnsignedConstantNode(LineNum, gcnew CharacterStringNode(LineNum, $1.tokenStart, $1.tokenLength))); }
 | nil
{ /*unsigned_constant*/ $$ = AddNode(gcnew Ast::UnsignedConstantNode(LineNum, AstNode($1))); }
 | boolean_constant
{ /*unsigned_constant*/ $$ = AddNode(gcnew Ast::UnsignedConstantNode(LineNum, AstNode($1))); } 
 ;

unsigned_number : unsigned_integer
{ /*unsigned_number*/ $$ = AddNode(gcnew Ast::UnsignedNumberNode(LineNum, AstNode($1))); }
 | unsigned_real 
{ /*unsigned_number*/ 
	$$ = AddNode(gcnew Ast::UnsignedNumberNode(LineNum, AstNode($1))); 
}
 ;
 
unsigned_integer : DIGSEQ
{ /*unsigned_integer*/ 
	$$ = AddNode(gcnew Ast::UnsignedIntegerNode(LineNum, $1)); 
}
 ;

unsigned_real : REALNUMBER
{ /*unsigned_real*/ 
	$$ = AddNode(gcnew Ast::UnsignedRealNode(LineNum, $1)); 
}
 ;
 
boolean_constant : BTRUE
{ /*boolean_constant*/
	$$ = AddNode(gcnew Ast::BooleanConstantNode(LineNum, true));
} 
 | BFALSE
{ /*boolean_constant*/
	$$ = AddNode(gcnew Ast::BooleanConstantNode(LineNum, false));
}  
 ;

/* functions with no params will be handled by plain identifier */
function_designator : identifier params
{ /*function_designator*/ 
	$$ = AddNode(gcnew Ast::FunctionDesignatorNode(LineNum, AstNode($1), AstNode($2))); 
}
 ;

set_constructor : LBRAC member_designator_list RBRAC
{ /*set_constructor*/ 
	$$ = AddNode(gcnew Ast::SetConstructorNode(LineNum, 
		gcnew MemberDesignatorListNode(LineNum, AstNodeList($2))));
}
 | LBRAC RBRAC
{ /*set_constructor*/ 
	$$ = AddNode(gcnew Ast::SetConstructorNode(LineNum, 
		nullptr));
} 
 ;

member_designator_list : member_designator_list comma member_designator
{ /*member_designator_list*/ 
	AstNodeList($1)->Add(AstNode($3)); 
    $$ = $1; 
}
 | member_designator { /*member_designator_list*/ $$ = AddNodeList($1); }
 ;

member_designator : expression DOTDOT expression
{ /*member_designator*/ 
	$$ = AddNode(gcnew Ast::MemberDesignatorNode(LineNum, AstNode($1), AstNode($3)));
} 
 | expression
{ /*member_designator*/ 
	$$ = AddNode(gcnew Ast::MemberDesignatorNode(LineNum, nullptr, AstNode($1)));
}  
 ;

addop : PLUS    { $$ = PLUS; }
 | MINUS		{ $$ = MINUS; }
 | OR			{ $$ = OR; }
 ;

mulop : STAR	{ $$ = STAR; }
 | SLASH		{ $$ = SLASH; }
 | DIV			{ $$ = DIV; }
 | MOD			{ $$ = MOD; }
 | AND			{ $$ = AND; }
 ;

relop : EQUAL	{ $$ = EQUAL; }
 | NOTEQUAL		{ $$ = NOTEQUAL; }
 | LT			{ $$ = LT; }
 | GT			{ $$ = GT; }
 | LE			{ $$ = LE; }
 | GE			{ $$ = GE; }
 | IN			{ $$ = IN; }
 ;

identifier : IDENTIFIER	
{ /*identifier*/
	$$ = AddNode(gcnew Ast::IdentifierNode(LineNum, $1.tokenStart, $1.tokenLength));
}
 ;

semicolon : SEMICOLON 
{ $$ = SEMICOLON; }
 ;

colon : COLON 
{ $$ = COLON; }
 ;
 
comma : COMMA
{ $$ = COMMA; }
 ;
 
var : VAR	
{ $$ = VAR; }
 ;
 
nil : NIL
{ /*nil*/
	$$ = AddNode(gcnew Ast::NilNode(LineNum)); 
}
 ;
 
 %%
 
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