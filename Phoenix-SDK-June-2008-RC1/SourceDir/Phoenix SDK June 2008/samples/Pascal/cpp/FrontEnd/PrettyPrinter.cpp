//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Implementation of the PrettyPrinter class.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

using namespace System;

#include "AstClasses.h"
#include "Ast.h"
#include "PrettyPrinter.h"
#include "YaccDeclarations.h"

namespace Pascal 
{

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given Node object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   Node^ node
)
{
   throw gcnew Exception("Visit() not implemented for Node^");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given UnaryNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   UnaryNode^ node
)
{
   throw gcnew Exception("Visit() not implemented for UnaryNode^");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given BinaryNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   BinaryNode^ node
)
{
   throw gcnew Exception("Visit() not implemented for BinaryNode^");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given TernaryNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   TernaryNode^ node
)
{
   throw gcnew Exception("Visit() not implemented for TernaryNode^");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given PolyadicNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   PolyadicNode^ node
)
{
   throw gcnew Exception("Visit() not implemented for PolyadicNode^");

   for each (Node^ child in node->ChildList)
   {
      child->Accept(this);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ActualParameterListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ActualParameterListNode^ node
)
{
   PrintNodes(node->ChildList, Comma+Space);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ActualParameterNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ActualParameterNode^ node
)
{
   node->First->Accept(this);
   if (node->Second)
   {
      this->out->Write(Space+Colon+Space);
      node->Second->Accept(this);

      if (node->Third)
      {
         this->out->Write(Space+Colon+Space);
         node->Third->Accept(this);
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ArrayTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ArrayTypeNode^ node
)
{
   this->out->Write("array"+Space+LeftBracket);
   node->IndexList->Accept(this);
   this->out->Write(RightBracket+Space+"of"+Space);
   node->ComponentType->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given AssignmentStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   AssignmentStatementNode^ node
)
{
   PrintIndent();
   node->VariableAccess->Accept(this);
   this->out->Write(Space+Colon+Equals+Space);
   node->Expression->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given BaseTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   BaseTypeNode^ node
)
{
   node->OrdinalType->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given BlockNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   BlockNode^ node
)
{
   // Only forward non-nullptr nodes to PrintNodes.
   // This will prevent extra indentation when a nullptr
   // child is encountered.

   List<Node^>^ list = gcnew List<Node^>();
   for each (Node^ node in node->ChildList)
   {
      if (node && node->HasChildren)
         list->Add(node);
   }
   if (list->Count > 0)
      PrintNodes(list, IndentString);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given BooleanConstantNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   BooleanConstantNode^ node
)
{
   this->out->Write("{0}", node->Value ? "true" : "false");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given BooleanExpressionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   BooleanExpressionNode^ node
)
{
   node->Expression->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given CaseConstantListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   CaseConstantListNode^ node
)
{
   PrintNodes(node->ChildList, Comma+Space);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given CaseConstantNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   CaseConstantNode^ node
)
{
   node->First->Accept(this);

   if (node->Second)
   {
      this->out->Write(Dot+Dot);
      node->Second->Accept(this);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given CaseIndexNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   CaseIndexNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given CaseListElementListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   CaseListElementListNode^ node
)
{
   PrintNodes(node->ChildList, Semicolon+Newline+IndentString);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given CaseListElementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   CaseListElementNode^ node
)
{
   node->First->Accept(this);
   this->out->Write(Space+Colon+Space);
   node->Second->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given CaseStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   CaseStatementNode^ node
)
{
   this->out->Write("case"+Space);
   node->CaseIndex->Accept(this);
   this->out->Write(Space+"of"+Space+Newline);
   Indent++;
   PrintIndent();
   node->CaseListElementList->Accept(this);

   if (node->HasFirstSemicolon)
      this->out->Write(Semicolon+Newline);
   else
      this->out->Write(Newline);

   if (node->OtherwisePart)
   {
      PrintIndent();
      node->OtherwisePart->Accept(this);

      this->out->Write(Space);
      node->Statement->Accept(this);

      if (node->HasSecondSemicolon)
        this->out->Write(Semicolon);
      this->out->Write(Newline);
   }   

   Indent--;
   PrintIndent();
   this->out->Write("end");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given CharacterStringNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   CharacterStringNode^ node
)
{
   this->out->Write("'{0}'", node->StringValue);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ClosedForStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ClosedForStatementNode^ node
)
{
   ForStatementNode ^ baseNode = node;
   Visit(baseNode); 
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ClosedIfStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ClosedIfStatementNode^ node
)
{
   IfStatementNode ^ baseNode = node;
   Visit(baseNode);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ClosedStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ClosedStatementNode^ node
)
{
   if (node->Label)
   { 
      // Unindent for label. 
      PrintUnindent(1);    

      node->Label->Accept(this);
      this->out->Write(Colon+Newline);
      PrintIndent();
   }
   node->NonLabeledClosedStatement->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ClosedWhileStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ClosedWhileStatementNode^ node
)
{
   WhileStatementNode ^ baseNode = node;
   Visit(baseNode); 
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ClosedWithStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ClosedWithStatementNode^ node
)
{
   WithStatementNode ^ baseNode = node;
   Visit(baseNode); 
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ComponentTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ComponentTypeNode^ node
)
{
   node->TypeDenoter->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given CompoundStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   CompoundStatementNode^ node
)
{
   this->out->WriteLine("begin");
   Indent++;
   PrintIndent();
   node->StatementSequence->Accept(this);
   Indent--;
   this->out->Write(Newline);
   PrintIndent();
   this->out->Write("end");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ConstantDefinitionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ConstantDefinitionNode^ node
)
{
   node->Identifier->Accept(this);
   this->out->Write(Space+Equals+Space);
   node->ConstantExpression->Accept(this);
   this->out->WriteLine(Semicolon);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ConstantDefinitionPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ConstantDefinitionPartNode^ node
)
{
   if (node->First)
   {
      this->out->WriteLine("const");
      Indent++;
	   PrintIndent();
      node->ConstantList->Accept(this);
      Indent--;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ConstantExponentiationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ConstantExponentiationNode^ node
)
{
   ExponentiationBaseNode ^ baseNode = node;
   Visit(baseNode);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ConstantExpressionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ConstantExpressionNode^ node
)
{
   ExpressionBaseNode ^ baseNode = node;
   Visit(baseNode);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ConstantFactorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ConstantFactorNode^ node
)
{
   FactorBaseNode ^ baseNode = node;
   Visit(baseNode);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ConstantListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ConstantListNode^ node
)
{
   PrintNodes(node->ChildList, IndentString);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ConstantNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ConstantNode^ node
)
{
   // ConstantNode can be 
   //    sign non_string | 
   //    CHARACTER_STRING

   NonStringNode^ nonString = node->NonString;
   if (nonString)
   {
      String^ sign = String::Empty;
      switch (node->Sign)
      {
      case MINUS:
         sign = Minus;
         break;
      case PLUS:
      default:
      // for default and PLUS, print the empty string.
         break;
      }

      this->out->Write(sign);
   }
   
   node->First->Accept(this);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ConstantPrimaryNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ConstantPrimaryNode^ node
)
{
   ConstantExpressionNode^ constExpr = node->ConstantExpression;
   if (constExpr)
   {
      node->First->Accept(this);
   }
   else
   {
      ConstantPrimaryNode^ constPrim = node->ConstantPrimary;
      if (constPrim)
         this->out->Write("not"+Space);
      
      node->First->Accept(this);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ConstantSimpleExpressionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ConstantSimpleExpressionNode^ node
)
{
   SimpleExpressionBaseNode ^ baseNode = node;
   Visit(baseNode);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ConstantTermNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ConstantTermNode^ node
)
{
   TermBaseNode ^ baseNode = node;
   Visit(baseNode);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ControlVariableNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ControlVariableNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given DirectionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   DirectionNode^ node
)
{   
	this->out->Write(node->Direction);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given DirectiveNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   DirectiveNode^ node
)
{   
   this->out->Write(node->StringValue);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given DomainTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   DomainTypeNode^ node
)
{
   node->Identifier->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given EnumeratedTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   EnumeratedTypeNode^ node
)
{
   this->out->Write(LeftParenthesis);
   node->First->Accept(this);
   this->out->Write(RightParenthesis);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ExponentiationBaseNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ExponentiationBaseNode^ node
)
{
   node->First->Accept(this);
   
   if (node->Second)
   {
      this->out->Write(Space+Star+Star+Space);
      node->Second->Accept(this);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ExponentiationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ExponentiationNode^ node
)
{
   ExponentiationBaseNode ^ baseNode = node;
   Visit(baseNode);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ExpressionBaseNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ExpressionBaseNode^ node
)
{
   String^ operatorString = nullptr;

   switch (node->RelationalOperator)
   {
   case EQUAL:
      operatorString = Equals;
      break;
   case NOTEQUAL:
      operatorString = NotEquals;
      break;
   case GT:
      operatorString = GreaterThan;
      break;
   case GE:
      operatorString = GreaterThanOrEqual;
      break;
   case LT:
      operatorString = LessThan;
      break;
   case LE:
      operatorString = LessThanOrEqual;
      break;
   case IN:
      operatorString = "in";
      break;
   case (unsigned)-1:
	   break;
   default:
      Debug::Assert(false);
      break;
   }

   node->First->Accept(this);

   if (operatorString != nullptr)
   {
      this->out->Write(Space+operatorString+Space);
   }
   if (node->Second)
   {
      node->Second->Accept(this);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ExpressionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ExpressionNode^ node
)
{
   ExpressionBaseNode ^ baseNode = node;
   Visit(baseNode);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FactorBaseNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FactorBaseNode^ node
)
{
   String^ sign = String::Empty;
   switch (node->Sign)
   {
   case MINUS:
      sign = Minus;
      break;
   case PLUS:
   default:
   // for default and PLUS, print the empty string.
      break;
   }

   this->out->Write(sign);
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FactorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FactorNode^ node
)
{
   FactorBaseNode ^ baseNode = node;
   Visit(baseNode);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FieldDesignatorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FieldDesignatorNode^ node
)
{
   node->VariableAccess->Accept(this);
   this->out->Write(Dot);
   node->Identifier->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FileNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FileNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FileTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FileTypeNode^ node
)
{
   this->out->Write("file of"+Space);
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FinalValueNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FinalValueNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FormalParameterListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FormalParameterListNode^ node
)
{
   this->out->Write(LeftParenthesis);
   node->First->Accept(this);
   this->out->Write(RightParenthesis);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FormalParameterNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FormalParameterNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FormalParameterSectionListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FormalParameterSectionListNode^ node
)
{
   PrintNodes(node->ChildList, Semicolon+Space);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FormalParameterSectionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FormalParameterSectionNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ForStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ForStatementNode^ node
)
{
   this->out->Write("for"+Space);
   node->ControlVariable->Accept(this);
   this->out->Write(Space+Colon+Equals+Space);      
   node->InitialValue->Accept(this);
   this->out->Write(Space);
   node->Direction->Accept(this); 
   this->out->Write(Space);           
   node->FinalValue->Accept(this);
   this->out->WriteLine(Space+"do");     

   Indent++;
   PrintIndent();
   node->ChildList[4]->Accept(this); 
   Indent--;

}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FunctionalParameterSpecificationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FunctionalParameterSpecificationNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FunctionBlockNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FunctionBlockNode^ node
)
{
   node->Block->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FunctionDeclarationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FunctionDeclarationNode^ node
)
{
   node->First->Accept(this);
   this->out->Write(Semicolon+Space);
   node->Second->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FunctionDesignatorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FunctionDesignatorNode^ node
)
{
   node->First->Accept(this);
   node->Second->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FunctionHeadingNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FunctionHeadingNode^ node
)
{
   PrintIndent();
   this->out->Write("function"+Space);
   node->First->Accept(this);

   if (node->Second)
   {
      this->out->Write(Space);
      node->Second->Accept(this);
   }

   this->out->Write(Space+Colon+Space);
   node->Third->Accept(this);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given FunctionIdentificationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   FunctionIdentificationNode^ node
)
{
   this->out->Write("function"+Space);
   node->First->Accept(this);    
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given GotoStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   GotoStatementNode^ node
)
{
   this->out->Write("goto"+Space);
   node->Label->Accept(this);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given IdentifierListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   IdentifierListNode^ node
)
{
   PrintNodes(node->ChildList, Comma+Space);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given IdentifierNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   IdentifierNode^ node
)
{
   this->out->Write(node->Name);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given IfStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   IfStatementNode^ node
)
{
   this->out->Write("if"+Space);
   node->BooleanExpression->Accept(this);
   this->out->Write(Space+"then"+Space+Newline);

   Indent++;      
   PrintIndent();
   node->Second->Accept(this);
   Indent--;

   if (node->Third)
   {
      this->out->Write(Newline);
      PrintIndent();
      this->out->Write("else"+Newline);
      Indent++;
      PrintIndent();
      node->Third->Accept(this);
      Indent--;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given IndexedVariableNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   IndexedVariableNode^ node
)
{
   node->First->Accept(this);
   this->out->Write(LeftBracket);
   node->Second->Accept(this);
   this->out->Write(RightBracket);    
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given IndexExpressionListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   IndexExpressionListNode^ node
)
{
   PrintNodes(node->ChildList, Comma+Space);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given IndexExpressionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   IndexExpressionNode^ node
)
{      
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given IndexListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   IndexListNode^ node
)
{
   PrintNodes(node->ChildList, Comma+Space);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given IndexTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   IndexTypeNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given InitialValueNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   InitialValueNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given LabelDeclarationPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   LabelDeclarationPartNode^ node
)
{
   if (node->First)
   {
      this->out->WriteLine("label");
      Indent++;
      node->LabelList->Accept(this);
      Indent--;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given LabelListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   LabelListNode^ node
)
{
   PrintIndent();
   PrintNodes(node->ChildList, Comma+Space);
   this->out->WriteLine(Semicolon);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given LabelNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   LabelNode^ node
)
{   
   this->out->Write("{0}", node->Label);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given MemberDesignatorListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   MemberDesignatorListNode^ node
)
{
   PrintNodes(node->ChildList, Comma+Space);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given MemberDesignatorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   MemberDesignatorNode^ node
)
{
   if (node->First)
   {
      node->First->Accept(this);
      this->out->Write(Dot+Dot);
   }
   node->Second->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ModuleNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ModuleNode^ node
)
{
   PrintNodes(node->ChildList, String::Empty);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given NewOrdinalTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   NewOrdinalTypeNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given NewPointerTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   NewPointerTypeNode^ node
)
{
   this->out->Write(node->UpArrow);
   node->DomainType->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given NewStructuredTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   NewStructuredTypeNode^ node
)
{
   if (node->IsPacked)
      this->out->Write("packed"+Space);
   
   node->StructuredType->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given NewTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   NewTypeNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given NilNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   NilNode^ node
)
{
   this->out->Write(node->Nil);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given NonLabeledClosedStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   NonLabeledClosedStatementNode^ node
)
{
   if (node->First)
      node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given NonLabeledOpenStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   NonLabeledOpenStatementNode^ node
)
{
   if (node->First)
      node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given NonStringNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   NonStringNode^ node
)
{
   switch(node->Kind)
   {
   case NonStringNode::ValueKind::Identifier:
      node->Identifier->Accept(this);
      break;
   case NonStringNode::ValueKind::IntValue:
      this->out->Write("{0}", node->IntegerValue);
      break;
   case NonStringNode::ValueKind::RealValue:
      this->out->Write("{0}", node->RealValue);
      break;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given OpenForStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   OpenForStatementNode^ node
)
{
   ForStatementNode ^ baseNode = node;
   Visit(baseNode); 
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given OpenIfStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   OpenIfStatementNode^ node
)
{
   IfStatementNode ^ baseNode = node;
   Visit(baseNode);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given OpenStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   OpenStatementNode^ node
)
{
   if (node->Label)
   { 
      // Unindent for label.
      PrintUnindent(1); 

      node->Label->Accept(this);
      this->out->Write(Colon+Newline);
      PrintIndent();
   }
   node->Second->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given OpenWhileStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   OpenWhileStatementNode^ node
)
{
   WhileStatementNode ^ baseNode = node;
   Visit(baseNode);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given OpenWithStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   OpenWithStatementNode^ node
)
{
   WithStatementNode ^ baseNode = node;
   Visit(baseNode);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given OrdinalTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   OrdinalTypeNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given OtherwisePartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   OtherwisePartNode^ node
)
{
   this->out->Write("otherwise" + (node->HasColon ? Colon : Space));
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ParamsNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ParamsNode^ node
)
{
   this->out->Write(LeftParenthesis);
   node->First->Accept(this);
   this->out->Write(RightParenthesis);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given PrimaryBaseNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   PrimaryBaseNode^ node
)
{
   PrimaryBaseNode ^ baseNode = node;
   Visit(baseNode);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given PrimaryNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   PrimaryNode^ node
)
{
   bool done = false;
      
   ExpressionNode^ expr = node->Expression;
   if (expr)
   {
      this->out->Write(LeftParenthesis);
      expr->Accept(this);
      this->out->Write(RightParenthesis);
   }
   else
   {
      PrimaryNode^ prim = node->Primary;
      if (prim)
          this->out->Write("not"+Space);
    
      node->First->Accept(this);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ProceduralParameterSpecificationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ProceduralParameterSpecificationNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ProcedureAndFunctionDeclarationPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ProcedureAndFunctionDeclarationPartNode^ node
)
{
   if (node->ProcedureOrFunctionDeclarationList)
   {
      node->ProcedureOrFunctionDeclarationList->Accept(this);
   }
   this->out->Write(Semicolon+Newline);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ProcedureBlockNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ProcedureBlockNode^ node
)
{
   node->Block->Accept(this);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ProcedureDeclarationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ProcedureDeclarationNode^ node
)
{
   node->ProcedureHeading->Accept(this);
   if (node->Directive)
   {
      this->out->Write(Semicolon+Space);
      node->Directive->Accept(this);
   }
   else
   {
      Debug::Assert(node->ProcedureBlock != nullptr);
      this->out->Write(Semicolon+Newline);
      Indent++;
      PrintIndent();
      node->ProcedureBlock->Accept(this);
      Indent--;
   }   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ProcedureHeadingNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ProcedureHeadingNode^ node
)
{
   PrintIndent();
   node->ProcedureIdentification->Accept(this);
   if (node->FormalParameterList)
   {
      this->out->Write(Space);
      node->FormalParameterList->Accept(this);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ProcedureIdentificationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ProcedureIdentificationNode^ node
)
{
   this->out->Write("procedure"+Space);
   node->Identifier->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ProcedureOrFunctionDeclarationListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ProcedureOrFunctionDeclarationListNode^ node
)
{
   PrintNodes(node->ChildList, Semicolon+Newline+IndentString);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ProcedureOrFunctionDeclarationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ProcedureOrFunctionDeclarationNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ProcedureStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ProcedureStatementNode^ node
)
{
   node->First->Accept(this);
   if (node->Second)
   {
      this->out->Write(Space);
      node->Second->Accept(this);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ProgramHeadingNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ProgramHeadingNode^ node
)
{
   this->out->Write("program"+Space);
   node->First->Accept(this);

   if (node->Second)
   {
      this->out->Write(Space+LeftParenthesis);
      node->Second->Accept(this);
      this->out->Write(RightParenthesis);
   }   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ProgramNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ProgramNode^ node
)
{
   node->First->Accept(this);
   this->out->WriteLine(Semicolon);
   node->Second->Accept(this);
   this->out->WriteLine(Dot);
   this->out->WriteLine();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given RecordSectionListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   RecordSectionListNode^ node
)
{
   PrintNodes(node->ChildList, Semicolon+Newline+IndentString);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given RecordSectionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   RecordSectionNode^ node
)
{
   node->IdentifierList->Accept(this);
   this->out->Write(Space+Colon+Space);
   node->TypeDenoter->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given RecordTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   RecordTypeNode^ node
)
{
   this->out->Write("record"+Newline);
   
   Indent++;
   PrintIndent();

   if (node->RecordSectionList && node->VariantPart)
   {	   
      node->RecordSectionList->Accept(this);
      this->out->WriteLine(Semicolon);
      node->VariantPart->Accept(this);
   }
   else if (node->RecordSectionList)
   {	
      node->RecordSectionList->Accept(this);
   }
   else
   {
      node->VariantPart->Accept(this);
   }

   this->out->Write(Newline);
   Indent--;
   PrintIndent();
   this->out->Write("end");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given RecordVariableListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   RecordVariableListNode^ node
)
{
   PrintNodes(node->ChildList, Comma+Space);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given RepeatStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   RepeatStatementNode^ node
)
{
   int indent0 = Indent;
   this->out->Write("repeat"+Newline);
   Indent++;
   PrintIndent();
   node->StatementSequence->Accept(this);
   PrintUnindent(Indent-indent0);
   Indent--;   
   this->out->Write("until"+Space);
   node->BooleanExpression->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ResultTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ResultTypeNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given SetConstructorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   SetConstructorNode^ node
)
{
   this->out->Write(LeftBracket);
   if (node->First)
      node->First->Accept(this);
   this->out->Write(RightBracket);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given SetTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   SetTypeNode^ node
)
{
   this->out->Write("set"+Space+"of"+Space);
   node->BaseType->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given SimpleExpressionBaseNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   SimpleExpressionBaseNode^ node
)
{
   String^ operatorString = nullptr;

   switch(node->AddOperator)
   {
   case PLUS:
      operatorString = Plus;
      break;
   case MINUS:
      operatorString = Minus;
      break;
   case OR:
      operatorString = "or";
      break;      
   default:         
      break;
   }

   node->First->Accept(this);
   if (operatorString != nullptr)
   {
      this->out->Write(Space+operatorString+Space);
   }
   if (node->Second)
   {
      node->Second->Accept(this);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given SimpleExpressionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   SimpleExpressionNode^ node
)
{
   SimpleExpressionBaseNode ^ baseNode = node;
   Visit(baseNode);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given StatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   StatementNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given StatementPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   StatementPartNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given StatementSequenceNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   StatementSequenceNode^ node
)
{
   PrintNodes(node->ChildList, Semicolon+Newline+IndentString);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given StringNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   StringNode^ node
)
{
   this->out->Write(FullQuote + node->StringValue + FullQuote);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given StructuredTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   StructuredTypeNode^ node
)
{	
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given SubrangeTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   SubrangeTypeNode^ node
)
{
   node->FirstConstant->Accept(this);
   this->out->Write(Dot+Dot);
   node->SecondConstant->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given TagFieldNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   TagFieldNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given TagTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   TagTypeNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given TermBaseNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   TermBaseNode^ node
)
{
   String^ operatorString = nullptr;

   switch(node->MulOperator)
   {
   case STAR:
      operatorString = Star;
      break;
   case SLASH:
      operatorString = Slash;
      break;
   case DIV:
      operatorString = "div";
      break;   
   case MOD:
      operatorString = "mod";
      break; 
   case AND:
      operatorString = "and";
      break; 
   default:         
      break;
   }

   node->First->Accept(this);   
   if (operatorString != nullptr)
   {
      this->out->Write(Space+operatorString+Space);
   }
   if (node->Second)
   {
      node->Second->Accept(this);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given TermNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   TermNode^ node
)
{
   TermBaseNode ^ baseNode = node;
   Visit(baseNode);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given TypeDefinitionListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   TypeDefinitionListNode^ node
)
{
   PrintNodes(node->ChildList, IndentString);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given TypeDefinitionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   TypeDefinitionNode^ node
)
{
   node->First->Accept(this);
   this->out->Write(Space+Equals+Space);
   node->Second->Accept(this);
   this->out->WriteLine(Semicolon);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given TypeDefinitionPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   TypeDefinitionPartNode^ node
)
{
   if (node->TypeDefinitionList)
   {
      this->out->WriteLine("type");
      Indent++;
	  PrintIndent();
      node->TypeDefinitionList->Accept(this);
      Indent--;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given TypeDenoterNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   TypeDenoterNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given UnsignedConstantNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   UnsignedConstantNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given UnsignedIntegerNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   UnsignedIntegerNode^ node
)
{
   this->out->Write("{0}", node->Value);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given UnsignedNumberNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   UnsignedNumberNode^ node
)
{
   node->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given UnsignedRealNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   UnsignedRealNode^ node
)
{
   this->out->Write("{0}", node->Value);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given ValueParameterSpecificationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   ValueParameterSpecificationNode^ node
)
{
   node->First->Accept(this);
   this->out->Write(Space+Colon+Space);
   node->Second->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given VariableAccessNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   VariableAccessNode^ node
)
{
   node->First->Accept(this);
   this->out->Write(node->UpArrow);      
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given VariableDeclarationListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   VariableDeclarationListNode^ node
)
{
   PrintNodes(node->ChildList, Semicolon+Newline+IndentString);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given VariableDeclarationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   VariableDeclarationNode^ node
)
{   
   node->First->Accept(this);
   this->out->Write(Space+Colon+Space);
   node->TypeDenoter->Accept(this);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given VariableDeclarationPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   VariableDeclarationPartNode^ node
)
{
   if (node->VariableDeclarationList)
   {
      this->out->Write("var"+Newline);
      Indent++;
	   PrintIndent();
      node->VariableDeclarationList->Accept(this);
      Indent--;
      this->out->Write(Semicolon+Newline);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given VariableParameterSpecificationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   VariableParameterSpecificationNode^ node
)
{
   this->out->Write("var"+Space);
   node->IdentifierList->Accept(this);
   this->out->Write(Space+Colon+Space);
   node->Identifier->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given VariantListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   VariantListNode^ node
)
{	
	PrintNodes(node->ChildList, Semicolon+Newline+IndentString);	
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given VariantNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   VariantNode^ node
)
{
   node->CaseConstantList->Accept(this);

   this->out->Write(Space+Colon+Space+LeftParenthesis);
	   
   if (node->RecordSectionList && node->VariantPart)
   {
      node->RecordSectionList->Accept(this);
      this->out->Write(Semicolon+Space);
      node->VariantPart->Accept(this);
   }
   else
   {            
	  Indent++;
      if (node->RecordSectionList)
         node->RecordSectionList->Accept(this);
      else if (node->VariantPart)
         node->VariantPart->Accept(this);
	  Indent--;
   }

   this->out->Write(RightParenthesis);  
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given VariantPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   VariantPartNode^ node
)
{
   if (node->VariantSelector && node->VariantList)
   {
      PrintIndent();
      this->out->Write("case"+Space);
      node->VariantSelector->Accept(this);
      this->out->Write(Space+"of"+Newline);
      Indent++;	  
	   PrintIndent();
      node->VariantList->Accept(this);
      Indent--;

      if (node->HasSemicolon)
		  this->out->Write(Semicolon);
   }
   else
   {
      Debug::Assert(node->VariantSelector == nullptr);
      Debug::Assert(node->VariantList == nullptr);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given VariantSelectorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   VariantSelectorNode^ node
)
{
   if (node->First)
   {
      node->First->Accept(this);
      this->out->Write(Space+Colon+Space);
   }
   node->Second->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given WhileStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   WhileStatementNode^ node
)
{
   this->out->Write("while"+Space);
   node->First->Accept(this);
   this->out->Write(Space+"do"+Space+Newline);
   Indent++;
   PrintIndent();
   node->Second->Accept(this);
   Indent--;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pretty prints the given WithStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
PrettyPrinter::Visit
(
   WithStatementNode^ node
)
{
   this->out->Write("with"+Space);
   node->RecordVariableList->Accept(this);
   this->out->Write(Space+"do"+Newline);

   Indent++;
   PrintIndent();
   node->Second->Accept(this);
   Indent--;
}

void PrettyPrinter::Indent::set(int value)
{
   Debug::Assert(value >= 0);

   if (this->indent != value)
   {
      this->indent = value;
      this->indentString = String::Empty;
      for (int i = 0; i < this->indent; ++i)
         this->indentString += this->tab;
   }
}

int PrettyPrinter::Indent::get()
{
   return this->indent;
}


String^ PrettyPrinter::IndentString::get()
{
   return this->indentString;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Prints the provided list of Node objects. The provided
//    separator string is printed between elements.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void 
PrettyPrinter::PrintNodes
(
   List<Node^>^ list, 
   String^ separator
)
{
   String^ sep = nullptr;
   for each (Node^ node in list)
   {
      if (sep == nullptr)
         sep = separator;
      else
         this->out->Write(sep);     
      node->Accept(this);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Prints the given number of 'unindent' characters to the output stream.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void PrettyPrinter::PrintUnindent
(
   int n
)
{
   // For each tab, write an 'unindent' string.
   for (int j = 0; j < n; ++j)
      // For each character in a standard tab, write a backspace character.
      for (int i = 0; i < const_cast<String^>(this->tab)->Length; ++i)
      {
         this->out->Write(BackSpace);
      }
}

} // namespace Pascal