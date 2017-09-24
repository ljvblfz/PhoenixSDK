//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Implementation of the Node class and derived classes.
//
// Remarks:
//
//    The Node class represents a node of the abstract syntax tree that
//    is built at parse time (pascal.y).
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "Ast.h"
#include "YaccDeclarations.h"

using namespace System::Diagnostics;
using namespace Pascal;

namespace Ast
{

// Sets the current debug tag for the current function unit.
void 
Node::SetCurrentDebugTag()
{
   // If the current function unit is valid, then
   // set the current debug tag to the current
   // source file name and line number.

   Phx::FunctionUnit ^ functionUnit = ModuleBuilder::CurrentFunctionUnit;

   if (functionUnit != nullptr)
   {
      UInt32 debugTag = functionUnit->DebugInfo->CreateTag(
         Phx::Name::New(
            functionUnit->Lifetime, 
            Pascal::ModuleBuilder::SourceFileName
         ),
         this->sourceLineNumber
      );
      functionUnit->CurrentDebugTag = debugTag;
   }
}   

[DebuggerNonUserCode]
List<ActualParameterNode ^> ^ ActualParameterListNode::ActualParameters::get()
{
   List<ActualParameterNode ^> ^ list = 
      gcnew List<ActualParameterNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<ActualParameterNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
ExpressionNode ^ ActualParameterNode::FirstExpression::get()
{
   return safe_cast<ExpressionNode ^>(this->first);
}

[DebuggerNonUserCode]
ExpressionNode ^ ActualParameterNode::SecondExpression::get()
{
   try
   {
      ExpressionNode ^ n = safe_cast<ExpressionNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ExpressionNode ^ ActualParameterNode::ThirdExpression::get()
{
   try
   {
      ExpressionNode ^ n = safe_cast<ExpressionNode ^>(this->third);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
IndexListNode ^ ArrayTypeNode::IndexList::get()
{
   try
   {
      IndexListNode ^ n = safe_cast<IndexListNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ComponentTypeNode ^ ArrayTypeNode::ComponentType::get()
{
   try
   {
      ComponentTypeNode ^ n = safe_cast<ComponentTypeNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
VariableAccessNode ^ AssignmentStatementNode::VariableAccess::get()
{
   return safe_cast<VariableAccessNode ^>(this->first);
}

[DebuggerNonUserCode]
ExpressionNode ^ AssignmentStatementNode::Expression::get()
{
   return safe_cast<ExpressionNode ^>(this->second);
}

String^ AssignmentStatementNode::Name::get()
{
   return VariableAccess->Name;
}
   

[DebuggerNonUserCode]
OrdinalTypeNode ^ BaseTypeNode::OrdinalType::get()
{
   return safe_cast<OrdinalTypeNode ^>(this->first);
}

[DebuggerNonUserCode]
LabelDeclarationPartNode ^ BlockNode::LabelDeclarationPart::get()
{
   return safe_cast<LabelDeclarationPartNode ^>(this->childList[0]);
}

[DebuggerNonUserCode]
ConstantDefinitionPartNode ^ BlockNode::ConstantDefinitionPart::get()
{
   return safe_cast<ConstantDefinitionPartNode ^>(this->childList[1]);
}

[DebuggerNonUserCode]
TypeDefinitionPartNode ^ BlockNode::TypeDefinitionPart::get()
{
   return safe_cast<TypeDefinitionPartNode ^>(this->childList[2]);
}

[DebuggerNonUserCode]
VariableDeclarationPartNode ^ BlockNode::VariableDeclarationPart::get()
{
   return safe_cast<VariableDeclarationPartNode ^>(this->childList[3]);
}

[DebuggerNonUserCode]
ProcedureAndFunctionDeclarationPartNode ^ 
BlockNode::ProcedureAndFunctionDeclarationPart::get()
{
   return 
      safe_cast<ProcedureAndFunctionDeclarationPartNode ^>(this->childList[4]);
}

[DebuggerNonUserCode]
StatementPartNode ^ BlockNode::StatementPart::get()
{
   return safe_cast<StatementPartNode ^>(this->childList[5]);
}

[DebuggerNonUserCode]
ExpressionNode ^ BooleanExpressionNode::Expression::get()
{
   return safe_cast<ExpressionNode ^>(this->first);
}

[DebuggerNonUserCode]
List<CaseConstantNode ^> ^ CaseConstantListNode::CaseConstants::get()
{
   List<CaseConstantNode ^> ^ list = 
      gcnew List<CaseConstantNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<CaseConstantNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
ConstantNode ^ CaseConstantNode::FirstConstant::get()
{
   return safe_cast<ConstantNode ^>(this->first);
}

[DebuggerNonUserCode]
ConstantNode ^ CaseConstantNode::SecondConstant::get()
{
   try
   {
      ConstantNode ^ n = safe_cast<ConstantNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ExpressionNode ^ CaseIndexNode::Expression::get()
{
   return safe_cast<ExpressionNode ^>(this->first);
}

[DebuggerNonUserCode]
List<CaseListElementNode ^> ^ CaseListElementListNode::CaseListElements::get()
{
   List<CaseListElementNode ^> ^ list = 
      gcnew List<CaseListElementNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<CaseListElementNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
CaseConstantListNode ^ CaseListElementNode::CaseConstantList::get()
{
   return safe_cast<CaseConstantListNode ^>(this->first);
}

[DebuggerNonUserCode]
StatementNode ^ CaseListElementNode::Statement::get()
{
   return safe_cast<StatementNode ^>(this->second);
}

[DebuggerNonUserCode]
CaseIndexNode ^ CaseStatementNode::CaseIndex::get()
{
   return safe_cast<CaseIndexNode ^>(this->childList[0]);
}

[DebuggerNonUserCode]
CaseListElementListNode ^ CaseStatementNode::CaseListElementList::get()
{
   return safe_cast<CaseListElementListNode ^>(this->childList[1]);
}

[DebuggerNonUserCode]
OtherwisePartNode ^ CaseStatementNode::OtherwisePart::get()
{
   try
   {
      OtherwisePartNode ^ n = 
         safe_cast<OtherwisePartNode ^>(this->childList[2]);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
StatementNode ^ CaseStatementNode::Statement::get()
{
   try
   {
      StatementNode ^ n = safe_cast<StatementNode ^>(this->childList[3]);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

// Translates the given control character sequence to 
// its runtime representation.
wchar_t
CharacterStringNode::TranslateControlCharacter
(
   wchar_t c
)
{
   // Lazy create the control map on first use.
   if (controlCharacterMap == nullptr)
   {
      controlCharacterMap = gcnew Dictionary<wchar_t, wchar_t>();
      controlCharacterMap['a'] = '\a';
      controlCharacterMap['b'] = '\b';
      controlCharacterMap['f'] = '\f';
      controlCharacterMap['n'] = '\n';
      controlCharacterMap['r'] = '\r';
      controlCharacterMap['t'] = '\t';
      controlCharacterMap['v'] = '\v';
      controlCharacterMap['?'] = '\?';
   }

   if (controlCharacterMap->ContainsKey(c))
      return controlCharacterMap[c];
   return c;
}

// Checks for special character sequences.
void
CharacterStringNode::Fixup()
{
   // Special case for single quote.

   if (this->string->Contains("''"))
   {
      this->string = this->string->Replace("''", "'");
   }

   // Check for character escape sequences.

   int index = this->string->IndexOf('\\', 0);
   while (index != -1)
   {
      if (index == this->string->Length - 1)
      {
         Pascal::Output::ReportError(
            SourceLineNumber,
            Pascal::Error::InvalidEscapeSequence
         );
         return;
      }
      else
      {
         wchar_t c = TranslateControlCharacter(this->string[index+1]);
         this->string = this->string->Remove(index, 2)->Insert(
            index, c.ToString());
         index = this->string->IndexOf('\\', index);
      }
   }
}

[DebuggerNonUserCode]
ClosedStatementNode ^ ClosedForStatementNode::ClosedStatement::get()
{
   return safe_cast<ClosedStatementNode ^>(this->childList[4]);
}

[DebuggerNonUserCode]
ClosedStatementNode ^ ClosedIfStatementNode::FirstClosedStatement::get()
{
   return safe_cast<ClosedStatementNode ^>(this->second);
}

[DebuggerNonUserCode]
ClosedStatementNode ^ ClosedIfStatementNode::SecondClosedStatement::get()
{
   return safe_cast<ClosedStatementNode ^>(this->third);
}

[DebuggerNonUserCode]
LabelNode ^ ClosedStatementNode::Label::get()
{
   try
   {
      LabelNode ^ n = safe_cast<LabelNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
NonLabeledClosedStatementNode ^ 
   ClosedStatementNode::NonLabeledClosedStatement::get()
{
   return safe_cast<NonLabeledClosedStatementNode ^>(this->second);
}

[DebuggerNonUserCode]
ClosedStatementNode ^ ClosedWhileStatementNode::ClosedStatement::get()
{
   return safe_cast<ClosedStatementNode ^>(this->second);
}

[DebuggerNonUserCode]
ClosedStatementNode ^ ClosedWithStatementNode::ClosedStatement::get()
{
   return safe_cast<ClosedStatementNode ^>(this->second);
}

[DebuggerNonUserCode]
TypeDenoterNode ^ ComponentTypeNode::TypeDenoter::get()
{
   return safe_cast<TypeDenoterNode ^>(this->first);
}

[DebuggerNonUserCode]
StatementSequenceNode ^ CompoundStatementNode::StatementSequence::get()
{
   return safe_cast<StatementSequenceNode ^>(this->first);
}

[DebuggerNonUserCode]
IdentifierNode ^ ConstantDefinitionNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

[DebuggerNonUserCode]
ConstantExpressionNode ^ ConstantDefinitionNode::ConstantExpression::get()
{
   return safe_cast<ConstantExpressionNode ^>(this->second);
}

[DebuggerNonUserCode]
ConstantListNode ^ ConstantDefinitionPartNode::ConstantList::get()
{
   return safe_cast<ConstantListNode ^>(this->first);
}

[DebuggerNonUserCode]
ConstantPrimaryNode ^ ConstantExponentiationNode::ConstantPrimary::get()
{
   return safe_cast<ConstantPrimaryNode ^>(this->first);
}

[DebuggerNonUserCode]
ConstantExponentiationNode ^ 
   ConstantExponentiationNode::ConstantExponentiation::get()
{
   try
   {
      ConstantExponentiationNode ^ n = 
         safe_cast<ConstantExponentiationNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ConstantSimpleExpressionNode ^ 
   ConstantExpressionNode::FirstConstantSimpleExpression::get()
{
   return safe_cast<ConstantSimpleExpressionNode ^>(this->first);
}

[DebuggerNonUserCode]
ConstantSimpleExpressionNode ^ 
   ConstantExpressionNode::SecondConstantSimpleExpression::get()
{
   return safe_cast<ConstantSimpleExpressionNode ^>(this->second);
}

[DebuggerNonUserCode]
ConstantFactorNode ^ ConstantFactorNode::ConstantFactor::get()
{
   try
   {
      ConstantFactorNode ^ n = safe_cast<ConstantFactorNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ConstantExponentiationNode ^ 
   ConstantFactorNode::ConstantExponentiation::get()
{
   try
   {
      ConstantExponentiationNode ^ n = 
         safe_cast<ConstantExponentiationNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
List<ConstantDefinitionNode ^> ^ ConstantListNode::ConstantDefinitions::get()
{
   List<ConstantDefinitionNode ^> ^ list = 
      gcnew List<ConstantDefinitionNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<ConstantDefinitionNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
NonStringNode ^ ConstantNode::NonString::get()
{
   try
   {
      NonStringNode ^ n = safe_cast<NonStringNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
CharacterStringNode ^ ConstantNode::CharacterString::get()
{
   try
   {
      CharacterStringNode ^ n = safe_cast<CharacterStringNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
IdentifierNode ^ ConstantPrimaryNode::Identifier::get()
{
   try
   {
      IdentifierNode ^ n = safe_cast<IdentifierNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ConstantExpressionNode ^ ConstantPrimaryNode::ConstantExpression::get()
{
   try
   {
      ConstantExpressionNode ^ n = 
         safe_cast<ConstantExpressionNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
UnsignedConstantNode ^ ConstantPrimaryNode::UnsignedConstant::get()
{
   try
   {
      UnsignedConstantNode ^ n = safe_cast<UnsignedConstantNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ConstantPrimaryNode ^ ConstantPrimaryNode::ConstantPrimary::get()
{
   try
   {
      ConstantPrimaryNode ^ n = safe_cast<ConstantPrimaryNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ConstantTermNode ^ ConstantSimpleExpressionNode::FirstConstantTerm::get()
{
   try
   {
      ConstantTermNode ^ n = safe_cast<ConstantTermNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ConstantSimpleExpressionNode ^ 
   ConstantSimpleExpressionNode::ConstantSimpleExpression::get()
{
   try
   {
      ConstantSimpleExpressionNode ^ n = 
         safe_cast<ConstantSimpleExpressionNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ConstantTermNode ^ ConstantSimpleExpressionNode::SecondConstantTerm::get()
{
   return safe_cast<ConstantTermNode ^>(this->second);
}

[DebuggerNonUserCode]
ConstantFactorNode ^ ConstantTermNode::FirstConstantFactor::get()
{
   try
   {
      ConstantFactorNode ^ n = safe_cast<ConstantFactorNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ConstantTermNode ^ ConstantTermNode::ConstantTerm::get()
{
   try
   {
      ConstantTermNode ^ n = safe_cast<ConstantTermNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ConstantFactorNode ^ ConstantTermNode::SecondConstantFactor::get()
{
   return safe_cast<ConstantFactorNode ^>(this->second);
}

[DebuggerNonUserCode]
IdentifierNode ^ ControlVariableNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

[DebuggerNonUserCode]
IdentifierNode ^ DomainTypeNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

[DebuggerNonUserCode]
IdentifierListNode ^ EnumeratedTypeNode::IdentifierList::get()
{
   return safe_cast<IdentifierListNode ^>(this->first);
}

[DebuggerNonUserCode]
bool ExponentiationBaseNode::IsExponentiationNode::get()
{
   try
   {
      ExponentiationNode^ test = safe_cast<ExponentiationNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
ExponentiationNode^ ExponentiationBaseNode::AsExponentiationNode::get()
{
   return safe_cast<ExponentiationNode^>(this);
}

[DebuggerNonUserCode]
bool ExponentiationBaseNode::IsConstantExponentiationNode::get()
{
   try
   {
      ConstantExponentiationNode^ test = 
         safe_cast<ConstantExponentiationNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
ConstantExponentiationNode^ 
   ExponentiationBaseNode::AsConstantExponentiationNode::get()
{
   return safe_cast<ConstantExponentiationNode^>(this);
}

[DebuggerNonUserCode]
PrimaryNode ^ ExponentiationNode::Primary::get()
{
   return safe_cast<PrimaryNode ^>(this->first);
}

[DebuggerNonUserCode]
ExponentiationNode ^ ExponentiationNode::Exponentiation::get()
{
   try
   {
      ExponentiationNode ^ n = safe_cast<ExponentiationNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
bool ExpressionBaseNode::IsExpressionNode::get()
{
   try
   {
      ExpressionNode^ test = safe_cast<ExpressionNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
ExpressionNode^ ExpressionBaseNode::AsExpressionNode::get()
{
   return safe_cast<ExpressionNode^>(this);
}

[DebuggerNonUserCode]
bool ExpressionBaseNode::IsConstantExpressionNode::get()
{
   try
   {
      ConstantExpressionNode^ test = safe_cast<ConstantExpressionNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
ConstantExpressionNode^ ExpressionBaseNode::AsConstantExpressionNode::get()
{
   return safe_cast<ConstantExpressionNode^>(this);
}

[DebuggerNonUserCode]
SimpleExpressionNode ^ ExpressionNode::FirstSimpleExpression::get()
{
   return safe_cast<SimpleExpressionNode ^>(this->first);
}

[DebuggerNonUserCode]
SimpleExpressionNode ^ ExpressionNode::SecondSimpleExpression::get()
{
   return safe_cast<SimpleExpressionNode ^>(this->second);
}

[DebuggerNonUserCode]
bool FactorBaseNode::IsFactorNode::get()
{
   try
   {
      FactorNode^ test = safe_cast<FactorNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
FactorNode^ FactorBaseNode::AsFactorNode::get()
{
   return safe_cast<FactorNode^>(this);
}

[DebuggerNonUserCode]
bool FactorBaseNode::IsConstantFactorNode::get()
{
   try
   {
      ConstantFactorNode^ test = safe_cast<ConstantFactorNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
ConstantFactorNode^ FactorBaseNode::AsConstantFactorNode::get()
{
   return safe_cast<ConstantFactorNode^>(this);
}

[DebuggerNonUserCode]
FactorNode ^ FactorNode::Factor::get()
{
   try
   {
      FactorNode ^ n = safe_cast<FactorNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ExponentiationNode ^ FactorNode::Exponentiation::get()
{
   try
   {
      ExponentiationNode ^ n = safe_cast<ExponentiationNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
VariableAccessNode ^ FieldDesignatorNode::VariableAccess::get()
{
   return safe_cast<VariableAccessNode ^>(this->first);
}

[DebuggerNonUserCode]
IdentifierNode ^ FieldDesignatorNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->second);
}

String^ FieldDesignatorNode::Name::get()
{   
   return VariableAccess->Name + "." + Identifier->Name;
}

[DebuggerNonUserCode]
ProgramNode ^ FileNode::Program::get()
{
   try
   {
      ProgramNode ^ n = safe_cast<ProgramNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ModuleNode ^ FileNode::Module::get()
{
   try
   {
      ModuleNode ^ n = safe_cast<ModuleNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ComponentTypeNode ^ FileTypeNode::ComponentType::get()
{
   return safe_cast<ComponentTypeNode ^>(this->first);
}

[DebuggerNonUserCode]
ExpressionNode ^ FinalValueNode::Expression::get()
{
   return safe_cast<ExpressionNode ^>(this->first);
}

[DebuggerNonUserCode]
FormalParameterSectionListNode ^ 
   FormalParameterListNode::FormalParameterSectionList::get()
{
   return safe_cast<FormalParameterSectionListNode ^>(this->first);
}

[DebuggerNonUserCode]
IdentifierNode ^ FormalParameterNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

String^ FormalParameterNode::Name::get()
{
   return Identifier->Name;
}

[DebuggerNonUserCode]
List<FormalParameterSectionNode ^> ^ 
   FormalParameterSectionListNode::FormalParameterSections::get()
{
   List<FormalParameterSectionNode ^> ^ list = 
      gcnew List<FormalParameterSectionNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<FormalParameterSectionNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
ValueParameterSpecificationNode ^ 
   FormalParameterSectionNode::ValueParameterSpecification::get()
{
   try
   {
      ValueParameterSpecificationNode ^ n = 
         safe_cast<ValueParameterSpecificationNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
VariableParameterSpecificationNode ^ 
   FormalParameterSectionNode::VariableParameterSpecification::get()
{
   try
   {
      VariableParameterSpecificationNode ^ n = 
         safe_cast<VariableParameterSpecificationNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ProceduralParameterSpecificationNode ^ 
   FormalParameterSectionNode::ProceduralParameterSpecification::get()
{
   try
   {
      ProceduralParameterSpecificationNode ^ n = 
         safe_cast<ProceduralParameterSpecificationNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
FunctionalParameterSpecificationNode ^ 
   FormalParameterSectionNode::FunctionalParameterSpecification::get()
{
   try
   {
      FunctionalParameterSpecificationNode ^ n = 
         safe_cast<FunctionalParameterSpecificationNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ControlVariableNode ^ ForStatementNode::ControlVariable::get()
{
   return safe_cast<ControlVariableNode ^>(this->childList[0]);
}

[DebuggerNonUserCode]
InitialValueNode ^ ForStatementNode::InitialValue::get()
{
   return safe_cast<InitialValueNode ^>(this->childList[1]);
}

[DebuggerNonUserCode]
DirectionNode ^ ForStatementNode::Direction::get()
{
   return safe_cast<DirectionNode ^>(this->childList[2]);
}

[DebuggerNonUserCode]
FinalValueNode ^ ForStatementNode::FinalValue::get()
{
   return safe_cast<FinalValueNode ^>(this->childList[3]);
}

[DebuggerNonUserCode]
FunctionHeadingNode ^ 
   FunctionalParameterSpecificationNode::FunctionHeading::get()
{
   return safe_cast<FunctionHeadingNode ^>(this->first);
}

[DebuggerNonUserCode]
BlockNode ^ FunctionBlockNode::Block::get()
{
   return safe_cast<BlockNode ^>(this->first);
}

[DebuggerNonUserCode]
FunctionHeadingNode ^ FunctionDeclarationNode::FunctionHeading::get()
{
   try
   {
      FunctionHeadingNode ^ n = safe_cast<FunctionHeadingNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
FunctionIdentificationNode ^ 
   FunctionDeclarationNode::FunctionIdentification::get()
{
   try
   {
      FunctionIdentificationNode ^ n = 
         safe_cast<FunctionIdentificationNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
DirectiveNode ^ FunctionDeclarationNode::Directive::get()
{
   try
   {
      DirectiveNode ^ n = safe_cast<DirectiveNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
FunctionBlockNode ^ FunctionDeclarationNode::FunctionBlock::get()
{
   try
   {
      FunctionBlockNode ^ n = safe_cast<FunctionBlockNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
IdentifierNode ^ FunctionDesignatorNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

[DebuggerNonUserCode]
ParamsNode ^ FunctionDesignatorNode::Params::get()
{
   return safe_cast<ParamsNode ^>(this->second);
}

[DebuggerNonUserCode]
IdentifierNode ^ FunctionHeadingNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

[DebuggerNonUserCode]
FormalParameterListNode ^ FunctionHeadingNode::FormalParameterList::get()
{
   try
   {
      FormalParameterListNode ^ n = 
         safe_cast<FormalParameterListNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ResultTypeNode ^ FunctionHeadingNode::ResultType::get()
{
   return safe_cast<ResultTypeNode ^>(this->third);
}

[DebuggerNonUserCode]
IdentifierNode ^ FunctionIdentificationNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

[DebuggerNonUserCode]
LabelNode ^ GotoStatementNode::Label::get()
{
   return safe_cast<LabelNode ^>(this->first);
}

void
IdentifierNode::Fixup()
{
   // Because built-in keywords are not case sensitive, allow
   // mixed casing of the built-in Input and Output variables.
   // Also allow mixed casing of the built-in constant 'maxint'.
   
   // Note that mixed casing of the 'nil', 'true', and 'false'
   // constants are already handled by the lex grammar.

   if (this->identifier->ToLower()->Equals("input"))
      this->identifier = "input";
   else if (this->identifier->ToLower()->Equals("output"))
      this->identifier = "output";
   else if (this->identifier->ToLower()->Equals("maxint"))
      this->identifier = "maxint";
}

[DebuggerNonUserCode]
List<FormalParameterNode ^> ^ 
   IdentifierListNode::FormalParameters::get()
{
   List<FormalParameterNode ^> ^ list = 
      gcnew List<FormalParameterNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<FormalParameterNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
BooleanExpressionNode ^ IfStatementNode::BooleanExpression::get()
{
   return safe_cast<BooleanExpressionNode ^>(this->first);
}

[DebuggerNonUserCode]
bool IfStatementNode::IsOpenIfStatementNode::get()
{
   try
   {
      OpenIfStatementNode^ test = safe_cast<OpenIfStatementNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
OpenIfStatementNode^ IfStatementNode::AsOpenIfStatementNode::get()
{
   return safe_cast<OpenIfStatementNode^>(this);
}

[DebuggerNonUserCode]
bool IfStatementNode::IsClosedIfStatementNode::get()
{
   try
   {
      ClosedIfStatementNode^ test = safe_cast<ClosedIfStatementNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
ClosedIfStatementNode^ IfStatementNode::AsClosedIfStatementNode::get()
{
   return safe_cast<ClosedIfStatementNode^>(this);
}

[DebuggerNonUserCode]
VariableAccessNode ^ IndexedVariableNode::VariableAccess::get()
{
   return safe_cast<VariableAccessNode ^>(this->first);
}

[DebuggerNonUserCode]
IndexExpressionListNode ^ IndexedVariableNode::IndexExpressionList::get()
{
   return safe_cast<IndexExpressionListNode ^>(this->second);
}

String^ IndexedVariableNode::Name::get()
{
   return VariableAccess->Name;
}

[DebuggerNonUserCode]
List<IndexExpressionNode ^> ^ IndexExpressionListNode::IndexExpressions::get()
{
   List<IndexExpressionNode ^> ^ list = 
      gcnew List<IndexExpressionNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<IndexExpressionNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
ExpressionNode ^ IndexExpressionNode::Expression::get()
{
   return safe_cast<ExpressionNode ^>(this->first);
}

[DebuggerNonUserCode]
List<IndexTypeNode ^> ^ IndexListNode::IndexTypes::get()
{
   List<IndexTypeNode ^> ^ list = 
      gcnew List<IndexTypeNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<IndexTypeNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
OrdinalTypeNode ^ IndexTypeNode::OrdinalType::get()
{
   return safe_cast<OrdinalTypeNode ^>(this->first);
}

[DebuggerNonUserCode]
ExpressionNode ^ InitialValueNode::Expression::get()
{
   return safe_cast<ExpressionNode ^>(this->first);
}

[DebuggerNonUserCode]
LabelListNode ^ LabelDeclarationPartNode::LabelList::get()
{
   return safe_cast<LabelListNode ^>(this->first);
}

[DebuggerNonUserCode]
List<LabelNode ^> ^ LabelListNode::Labels::get()
{
   List<LabelNode ^> ^ list = 
      gcnew List<LabelNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<LabelNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
List<MemberDesignatorNode ^> ^ 
   MemberDesignatorListNode::MemberDesignators::get()
{
   List<MemberDesignatorNode ^> ^ list = 
      gcnew List<MemberDesignatorNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<MemberDesignatorNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
ExpressionNode ^ MemberDesignatorNode::FirstExpression::get()
{
   try
   {
      ExpressionNode ^ n = safe_cast<ExpressionNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ExpressionNode ^ MemberDesignatorNode::SecondExpression::get()
{
   return safe_cast<ExpressionNode ^>(this->second);
}

[DebuggerNonUserCode]
ConstantDefinitionPartNode ^ ModuleNode::ConstantDefinitionPart::get()
{
   return safe_cast<ConstantDefinitionPartNode ^>(this->childList[0]);
}

[DebuggerNonUserCode]
TypeDefinitionPartNode ^ ModuleNode::TypeDefinitionPart::get()
{
   return safe_cast<TypeDefinitionPartNode ^>(this->childList[1]);
}

[DebuggerNonUserCode]
VariableDeclarationPartNode ^ ModuleNode::VariableDeclarationPart::get()
{
   return safe_cast<VariableDeclarationPartNode ^>(this->childList[2]);
}

[DebuggerNonUserCode]
ProcedureAndFunctionDeclarationPartNode ^ 
   ModuleNode::ProcedureAndFunctionDeclarationPart::get()
{
   return 
      safe_cast<ProcedureAndFunctionDeclarationPartNode ^>(this->childList[3]);
}

[DebuggerNonUserCode]
EnumeratedTypeNode ^ NewOrdinalTypeNode::EnumeratedType::get()
{
   try
   {
      EnumeratedTypeNode ^ n = safe_cast<EnumeratedTypeNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
SubrangeTypeNode ^ NewOrdinalTypeNode::SubrangeType::get()
{
   try
   {
      SubrangeTypeNode ^ n = safe_cast<SubrangeTypeNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
DomainTypeNode ^ NewPointerTypeNode::DomainType::get()
{
   return safe_cast<DomainTypeNode ^>(this->first);
}

[DebuggerNonUserCode]
StructuredTypeNode ^ NewStructuredTypeNode::StructuredType::get()
{
   return safe_cast<StructuredTypeNode ^>(this->first);
}

[DebuggerNonUserCode]
NewOrdinalTypeNode ^ NewTypeNode::NewOrdinalType::get()
{
   try
   {
      NewOrdinalTypeNode ^ n = safe_cast<NewOrdinalTypeNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
NewStructuredTypeNode ^ NewTypeNode::NewStructuredType::get()
{
   try
   {
      NewStructuredTypeNode ^ n = 
         safe_cast<NewStructuredTypeNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
NewPointerTypeNode ^ NewTypeNode::NewPointerType::get()
{
   try
   {
      NewPointerTypeNode ^ n = safe_cast<NewPointerTypeNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
AssignmentStatementNode ^ 
   NonLabeledClosedStatementNode::AssignmentStatement::get()
{
   try
   {
      AssignmentStatementNode ^ n = 
         safe_cast<AssignmentStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ProcedureStatementNode ^ 
   NonLabeledClosedStatementNode::ProcedureStatement::get()
{
   try
   {
      ProcedureStatementNode ^ n = 
         safe_cast<ProcedureStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
GotoStatementNode ^ NonLabeledClosedStatementNode::GotoStatement::get()
{
   try
   {
      GotoStatementNode ^ n = safe_cast<GotoStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
CompoundStatementNode ^ 
   NonLabeledClosedStatementNode::CompoundStatement::get()
{
   try
   {
      CompoundStatementNode ^ n = 
         safe_cast<CompoundStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
CaseStatementNode ^ NonLabeledClosedStatementNode::CaseStatement::get()
{
   try
   {
      CaseStatementNode ^ n = safe_cast<CaseStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
RepeatStatementNode ^ NonLabeledClosedStatementNode::RepeatStatement::get()
{
   try
   {
      RepeatStatementNode ^ n = safe_cast<RepeatStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ClosedIfStatementNode ^ 
   NonLabeledClosedStatementNode::ClosedIfStatement::get()
{
   try
   {
      ClosedIfStatementNode ^ n = 
         safe_cast<ClosedIfStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ClosedWhileStatementNode ^ 
   NonLabeledClosedStatementNode::ClosedWhileStatement::get()
{
   try
   {
      ClosedWhileStatementNode ^ n = 
         safe_cast<ClosedWhileStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ClosedForStatementNode ^ 
   NonLabeledClosedStatementNode::ClosedForStatement::get()
{
   try
   {
      ClosedForStatementNode ^ n = 
         safe_cast<ClosedForStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ClosedWithStatementNode ^ 
   NonLabeledClosedStatementNode::ClosedWithStatement::get()
{
   try
   {
      ClosedWithStatementNode ^ n = 
         safe_cast<ClosedWithStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
OpenWithStatementNode ^ 
   NonLabeledOpenStatementNode::OpenWithStatement::get()
{
   try
   {
      OpenWithStatementNode ^ n = 
         safe_cast<OpenWithStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
OpenIfStatementNode ^ NonLabeledOpenStatementNode::OpenIfStatement::get()
{
   try
   {
      OpenIfStatementNode ^ n = safe_cast<OpenIfStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
OpenWhileStatementNode ^ NonLabeledOpenStatementNode::OpenWhileStatement::get()
{
   try
   {
      OpenWhileStatementNode ^ n = 
         safe_cast<OpenWhileStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
OpenForStatementNode ^ NonLabeledOpenStatementNode::OpenForStatement::get()
{
   try
   {
      OpenForStatementNode ^ n = safe_cast<OpenForStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
IdentifierNode ^ NonStringNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->identifier);
}

[DebuggerNonUserCode]
OpenStatementNode ^ OpenForStatementNode::OpenStatement::get()
{
   return safe_cast<OpenStatementNode ^>(this->childList[4]);
}

[DebuggerNonUserCode]
StatementNode ^ OpenIfStatementNode::Statement::get()
{
   try
   {
      StatementNode ^ n = safe_cast<StatementNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ClosedStatementNode ^ OpenIfStatementNode::ClosedStatement::get()
{
   try
   {
      ClosedStatementNode ^ n = safe_cast<ClosedStatementNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
OpenStatementNode ^ OpenIfStatementNode::OpenStatement::get()
{
   try
   {
      OpenStatementNode ^ n = safe_cast<OpenStatementNode ^>(this->third);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
LabelNode ^ OpenStatementNode::Label::get()
{
   try
   {
      LabelNode ^ n = safe_cast<LabelNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
NonLabeledOpenStatementNode ^ OpenStatementNode::NonLabeledOpenStatement::get()
{
   return safe_cast<NonLabeledOpenStatementNode ^>(this->second);
}

[DebuggerNonUserCode]
OpenStatementNode ^ OpenWithStatementNode::OpenStatement::get()
{
   return safe_cast<OpenStatementNode ^>(this->second);
}

[DebuggerNonUserCode]
OpenStatementNode ^ OpenWhileStatementNode::OpenStatement::get()
{
   return safe_cast<OpenStatementNode ^>(this->second);
}

[DebuggerNonUserCode]
NewOrdinalTypeNode ^ OrdinalTypeNode::NewOrdinalType::get()
{
   try
   {
      NewOrdinalTypeNode ^ n = safe_cast<NewOrdinalTypeNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
IdentifierNode ^ OrdinalTypeNode::Identifier::get()
{
   try
   {
      IdentifierNode ^ n = safe_cast<IdentifierNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ActualParameterListNode ^ ParamsNode::ActualParameterList::get()
{
   return safe_cast<ActualParameterListNode ^>(this->first);
}

[DebuggerNonUserCode]
bool PrimaryBaseNode::IsPrimaryNode::get()
{
   try
   {
      PrimaryNode^ test = safe_cast<PrimaryNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
PrimaryNode^ PrimaryBaseNode::AsPrimaryNode::get()
{
   return safe_cast<PrimaryNode^>(this);
}

[DebuggerNonUserCode]
bool PrimaryBaseNode::IsConstantPrimaryNode::get()
{
   try
   {
      ConstantPrimaryNode^ test = safe_cast<ConstantPrimaryNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
ConstantPrimaryNode^ PrimaryBaseNode::AsConstantPrimaryNode::get()
{
   return safe_cast<ConstantPrimaryNode^>(this);
}

[DebuggerNonUserCode]
VariableAccessNode ^ PrimaryNode::VariableAccess::get()
{
   try
   {
      VariableAccessNode ^ n = safe_cast<VariableAccessNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ExpressionNode ^ PrimaryNode::Expression::get()
{
   try
   {
      ExpressionNode ^ n = safe_cast<ExpressionNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
UnsignedConstantNode ^ PrimaryNode::UnsignedConstant::get()
{
   try
   {
      UnsignedConstantNode ^ n = safe_cast<UnsignedConstantNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
PrimaryNode ^ PrimaryNode::Primary::get()
{
   try
   {
      PrimaryNode ^ n = safe_cast<PrimaryNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
FunctionDesignatorNode ^ PrimaryNode::FunctionDesignator::get()
{
   try
   {
      FunctionDesignatorNode ^ n = 
         safe_cast<FunctionDesignatorNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
SetConstructorNode ^ PrimaryNode::SetConstructor::get()
{
   try
   {
      SetConstructorNode ^ n = safe_cast<SetConstructorNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ProcedureHeadingNode ^ 
   ProceduralParameterSpecificationNode::ProcedureHeading::get()
{
   return safe_cast<ProcedureHeadingNode ^>(this->first);
}

[DebuggerNonUserCode]
ProcedureOrFunctionDeclarationListNode ^ 
   ProcedureAndFunctionDeclarationPartNode::
      ProcedureOrFunctionDeclarationList::get()
{
   return safe_cast<ProcedureOrFunctionDeclarationListNode ^>(this->first);
}

[DebuggerNonUserCode]
BlockNode ^ ProcedureBlockNode::Block::get()
{
   return safe_cast<BlockNode ^>(this->first);
}

[DebuggerNonUserCode]
ProcedureHeadingNode ^ ProcedureDeclarationNode::ProcedureHeading::get()
{
   return safe_cast<ProcedureHeadingNode ^>(this->first);
}

[DebuggerNonUserCode]
DirectiveNode ^ ProcedureDeclarationNode::Directive::get()
{
   try
   {
      DirectiveNode ^ n = safe_cast<DirectiveNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ProcedureBlockNode ^ ProcedureDeclarationNode::ProcedureBlock::get()
{
   try
   {
      ProcedureBlockNode ^ n = safe_cast<ProcedureBlockNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ProcedureIdentificationNode ^ 
   ProcedureHeadingNode::ProcedureIdentification::get()
{
   return safe_cast<ProcedureIdentificationNode ^>(this->first);
}

[DebuggerNonUserCode]
FormalParameterListNode ^ ProcedureHeadingNode::FormalParameterList::get()
{
   try
   {
      FormalParameterListNode ^ n = 
         safe_cast<FormalParameterListNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
IdentifierNode ^ ProcedureIdentificationNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

[DebuggerNonUserCode]
List<ProcedureOrFunctionDeclarationNode ^> ^ 
 ProcedureOrFunctionDeclarationListNode::ProcedureOrFunctionDeclarations::get()
{
   List<ProcedureOrFunctionDeclarationNode ^> ^ list = 
      gcnew List<ProcedureOrFunctionDeclarationNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<ProcedureOrFunctionDeclarationNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
ProcedureDeclarationNode ^ 
   ProcedureOrFunctionDeclarationNode::ProcedureDeclaration::get()
{
   try
   {
      ProcedureDeclarationNode ^ n = 
         safe_cast<ProcedureDeclarationNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
FunctionDeclarationNode ^ 
   ProcedureOrFunctionDeclarationNode::FunctionDeclaration::get()
{
   try
   {
      FunctionDeclarationNode ^ n = 
         safe_cast<FunctionDeclarationNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
IdentifierNode ^ ProcedureStatementNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

[DebuggerNonUserCode]
ParamsNode ^ ProcedureStatementNode::Params::get()
{
   try
   {
      ParamsNode ^ n = safe_cast<ParamsNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
IdentifierNode ^ ProgramHeadingNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

[DebuggerNonUserCode]
IdentifierListNode ^ ProgramHeadingNode::IdentifierList::get()
{
   try
   {
      IdentifierListNode ^ n = safe_cast<IdentifierListNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ProgramHeadingNode ^ ProgramNode::ProgramHeading::get()
{
   return safe_cast<ProgramHeadingNode ^>(this->first);
}

[DebuggerNonUserCode]
BlockNode ^ ProgramNode::Block::get()
{
   return safe_cast<BlockNode ^>(this->second);
}

[DebuggerNonUserCode]
List<RecordSectionNode ^> ^ RecordSectionListNode::RecordSections::get()
{
   List<RecordSectionNode ^> ^ list = 
      gcnew List<RecordSectionNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<RecordSectionNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
IdentifierListNode ^ RecordSectionNode::IdentifierList::get()
{
   return safe_cast<IdentifierListNode ^>(this->first);
}

[DebuggerNonUserCode]
TypeDenoterNode ^ RecordSectionNode::TypeDenoter::get()
{
   return safe_cast<TypeDenoterNode ^>(this->second);
}

[DebuggerNonUserCode]
RecordSectionListNode ^ RecordTypeNode::RecordSectionList::get()
{
   try
   {
      RecordSectionListNode ^ n = 
         safe_cast<RecordSectionListNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
VariantPartNode ^ RecordTypeNode::VariantPart::get()
{
   try
   {
      VariantPartNode ^ n = safe_cast<VariantPartNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
List<VariableAccessNode ^> ^ RecordVariableListNode::VariableAccesses::get()
{   
   List<VariableAccessNode ^> ^ list = 
      gcnew List<VariableAccessNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<VariableAccessNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
StatementSequenceNode ^ RepeatStatementNode::StatementSequence::get()
{
   return safe_cast<StatementSequenceNode ^>(this->first);
}

[DebuggerNonUserCode]
BooleanExpressionNode ^ RepeatStatementNode::BooleanExpression::get()
{
   return safe_cast<BooleanExpressionNode ^>(this->second);
}

[DebuggerNonUserCode]
IdentifierNode ^ ResultTypeNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

[DebuggerNonUserCode]
MemberDesignatorListNode ^ SetConstructorNode::MemberDesignatorList::get()
{
   try
   {
      MemberDesignatorListNode ^ n = 
         safe_cast<MemberDesignatorListNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
BaseTypeNode ^ SetTypeNode::BaseType::get()
{
   return safe_cast<BaseTypeNode ^>(this->first);
}

[DebuggerNonUserCode]
bool SimpleExpressionBaseNode::IsSimpleExpressionNode::get()
{
   try
   {
      SimpleExpressionNode^ test = safe_cast<SimpleExpressionNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
SimpleExpressionNode^ SimpleExpressionBaseNode::AsSimpleExpressionNode::get()
{
   return safe_cast<SimpleExpressionNode^>(this);
}

[DebuggerNonUserCode]
bool SimpleExpressionBaseNode::IsConstantSimpleExpressionNode::get()
{
   try
   {
      ConstantSimpleExpressionNode^ test = 
         safe_cast<ConstantSimpleExpressionNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
ConstantSimpleExpressionNode^ 
   SimpleExpressionBaseNode::AsConstantSimpleExpressionNode::get()
{
   return safe_cast<ConstantSimpleExpressionNode^>(this);
}

[DebuggerNonUserCode]
TermNode ^ SimpleExpressionNode::FirstTerm::get()
{
   try
   {
      TermNode ^ n = safe_cast<TermNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
SimpleExpressionNode ^ SimpleExpressionNode::SimpleExpression::get()
{
   try
   {
      SimpleExpressionNode ^ n = safe_cast<SimpleExpressionNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
TermNode ^ SimpleExpressionNode::SecondTerm::get()
{
   return safe_cast<TermNode ^>(this->second);
}

[DebuggerNonUserCode]
OpenStatementNode ^ StatementNode::OpenStatement::get()
{
   try
   {
      OpenStatementNode ^ n = safe_cast<OpenStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ClosedStatementNode ^ StatementNode::ClosedStatement::get()
{
   try
   {
      ClosedStatementNode ^ n = safe_cast<ClosedStatementNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
CompoundStatementNode ^ StatementPartNode::CompoundStatement::get()
{
   return safe_cast<CompoundStatementNode ^>(this->first);
}

[DebuggerNonUserCode]
List<StatementNode ^> ^ StatementSequenceNode::Statements::get()
{
   List<StatementNode ^> ^ list = 
      gcnew List<StatementNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<StatementNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
ArrayTypeNode ^ StructuredTypeNode::ArrayType::get()
{
   try
   {
      ArrayTypeNode ^ n = safe_cast<ArrayTypeNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
RecordTypeNode ^ StructuredTypeNode::RecordType::get()
{
   try
   {
      RecordTypeNode ^ n = safe_cast<RecordTypeNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
SetTypeNode ^ StructuredTypeNode::SetType::get()
{
   try
   {
      SetTypeNode ^ n = safe_cast<SetTypeNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
FileTypeNode ^ StructuredTypeNode::FileType::get()
{
   try
   {
      FileTypeNode ^ n = safe_cast<FileTypeNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
ConstantNode ^ SubrangeTypeNode::FirstConstant::get()
{
   return safe_cast<ConstantNode ^>(this->first);
}

[DebuggerNonUserCode]
ConstantNode ^ SubrangeTypeNode::SecondConstant::get()
{
   return safe_cast<ConstantNode ^>(this->second);
}

[DebuggerNonUserCode]
IdentifierNode ^ TagFieldNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

[DebuggerNonUserCode]
IdentifierNode ^ TagTypeNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

[DebuggerNonUserCode]
bool TermBaseNode::IsTermNode::get()
{
   try
   {
      TermNode^ test = safe_cast<TermNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
TermNode^ TermBaseNode::AsTermNode::get()
{
   return safe_cast<TermNode^>(this);
}

[DebuggerNonUserCode]
bool TermBaseNode::IsConstantTermNode::get()
{
   try
   {
      ConstantTermNode^ test = safe_cast<ConstantTermNode^>(this);
      return true;
   }
   catch(InvalidCastException^)
   {
      return false;
   }
}

[DebuggerNonUserCode]
ConstantTermNode^ TermBaseNode::AsConstantTermNode::get()
{
   return safe_cast<ConstantTermNode^>(this);
}

[DebuggerNonUserCode]
FactorNode ^ TermNode::FirstFactor::get()
{
   try
   {
      FactorNode ^ n = safe_cast<FactorNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
TermNode ^ TermNode::Term::get()
{
   try
   {
      TermNode ^ n = safe_cast<TermNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
FactorNode ^ TermNode::SecondFactor::get()
{
   return safe_cast<FactorNode ^>(this->second);
}

[DebuggerNonUserCode]
List<TypeDefinitionNode ^> ^ TypeDefinitionListNode::TypeDefinitions::get()
{
   List<TypeDefinitionNode ^> ^ list = 
      gcnew List<TypeDefinitionNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<TypeDefinitionNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
IdentifierNode ^ TypeDefinitionNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->first);
}

[DebuggerNonUserCode]
TypeDenoterNode ^ TypeDefinitionNode::TypeDenoter::get()
{
   return safe_cast<TypeDenoterNode ^>(this->second);
}

[DebuggerNonUserCode]
TypeDefinitionListNode ^ TypeDefinitionPartNode::TypeDefinitionList::get()
{
   return safe_cast<TypeDefinitionListNode ^>(this->first);
}

[DebuggerNonUserCode]
IdentifierNode ^ TypeDenoterNode::Identifier::get()
{
   try
   {
      IdentifierNode ^ n = safe_cast<IdentifierNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
NewTypeNode ^ TypeDenoterNode::NewType::get()
{
   try
   {
      NewTypeNode ^ n = safe_cast<NewTypeNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
UnsignedNumberNode ^ UnsignedConstantNode::UnsignedNumber::get()
{
   try
   {
      UnsignedNumberNode ^ n = safe_cast<UnsignedNumberNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
CharacterStringNode ^ UnsignedConstantNode::CharacterString::get()
{
   try
   {
      CharacterStringNode ^ n = safe_cast<CharacterStringNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
NilNode ^ UnsignedConstantNode::Nil::get()
{
   try
   {
      NilNode ^ n = safe_cast<NilNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
BooleanConstantNode ^ UnsignedConstantNode::BooleanConstant::get()
{
   try
   {
      BooleanConstantNode ^ n = safe_cast<BooleanConstantNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
UnsignedIntegerNode ^ UnsignedNumberNode::UnsignedInteger::get()
{
   try
   {
      UnsignedIntegerNode ^ n = safe_cast<UnsignedIntegerNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
UnsignedRealNode ^ UnsignedNumberNode::UnsignedReal::get()
{
   try
   {
      UnsignedRealNode ^ n = safe_cast<UnsignedRealNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
IdentifierListNode ^ ValueParameterSpecificationNode::IdentifierList::get()
{
   return safe_cast<IdentifierListNode ^>(this->first);
}

[DebuggerNonUserCode]
IdentifierNode ^ ValueParameterSpecificationNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->second);
}

[DebuggerNonUserCode]
IdentifierNode ^ VariableAccessNode::Identifier::get()
{
   try
   {
      IdentifierNode ^ n = safe_cast<IdentifierNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
IndexedVariableNode ^ VariableAccessNode::IndexedVariable::get()
{
   try
   {
      IndexedVariableNode ^ n = safe_cast<IndexedVariableNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
FieldDesignatorNode ^ VariableAccessNode::FieldDesignator::get()
{
   try
   {
      FieldDesignatorNode ^ n = safe_cast<FieldDesignatorNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
VariableAccessNode ^ VariableAccessNode::VariableAccess::get()
{
   try
   {
      VariableAccessNode ^ n = safe_cast<VariableAccessNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

String^ 
VariableAccessNode::Name::get()
{
   if (this->cachedName) 
      return this->cachedName;

   if (Identifier)           this->cachedName = Identifier->Name;
   else if (IndexedVariable) this->cachedName = IndexedVariable->Name;
   else if (FieldDesignator) this->cachedName = FieldDesignator->Name;
   else if (VariableAccess)  this->cachedName = VariableAccess->Name;
   
   Debug::Assert(this->cachedName != nullptr);
   return this->cachedName;
}

[DebuggerNonUserCode]
List<VariableDeclarationNode ^> ^ 
   VariableDeclarationListNode::VariableDeclarations::get()
{
   List<VariableDeclarationNode ^> ^ list = 
      gcnew List<VariableDeclarationNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<VariableDeclarationNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
IdentifierNode ^ VariableDeclarationNode::Identifier::get()
{
   try
   {
      IdentifierNode ^ n = safe_cast<IdentifierNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
IdentifierListNode ^ VariableDeclarationNode::IdentifierList::get()
{
   try
   {
      IdentifierListNode ^ n = safe_cast<IdentifierListNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
TypeDenoterNode ^ VariableDeclarationNode::TypeDenoter::get()
{
   return safe_cast<TypeDenoterNode ^>(this->second);
}

[DebuggerNonUserCode]
VariableDeclarationListNode ^ 
   VariableDeclarationPartNode::VariableDeclarationList::get()
{
   try
   {
      VariableDeclarationListNode ^ n = 
         safe_cast<VariableDeclarationListNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
IdentifierListNode ^ VariableParameterSpecificationNode::IdentifierList::get()
{
   return safe_cast<IdentifierListNode ^>(this->first);
}

[DebuggerNonUserCode]
IdentifierNode ^ VariableParameterSpecificationNode::Identifier::get()
{
   return safe_cast<IdentifierNode ^>(this->second);
}

[DebuggerNonUserCode]
List<VariantNode ^> ^ VariantListNode::Variants::get()
{
   List<VariantNode ^> ^ list = 
      gcnew List<VariantNode ^>(this->childList->Count);
   for each (Node ^ node in this->childList)
      list->Add(safe_cast<VariantNode ^>(node));
   return list;
}

[DebuggerNonUserCode]
CaseConstantListNode ^ VariantNode::CaseConstantList::get()
{
   return safe_cast<CaseConstantListNode ^>(this->first);
}

[DebuggerNonUserCode]
RecordSectionListNode ^ VariantNode::RecordSectionList::get()
{
   try
   {
      RecordSectionListNode ^ n = 
         safe_cast<RecordSectionListNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
VariantPartNode ^ VariantNode::VariantPart::get()
{
   try
   {
      VariantPartNode ^ n = safe_cast<VariantPartNode ^>(this->third);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
VariantSelectorNode ^ VariantPartNode::VariantSelector::get()
{
   try
   {
      VariantSelectorNode ^ n = safe_cast<VariantSelectorNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
VariantListNode ^ VariantPartNode::VariantList::get()
{
   try
   {
      VariantListNode ^ n = safe_cast<VariantListNode ^>(this->second);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
TagFieldNode ^ VariantSelectorNode::TagField::get()
{
   try
   {
      TagFieldNode ^ n = safe_cast<TagFieldNode ^>(this->first);
      return n;
   }
   catch(InvalidCastException ^)
   {
      return nullptr;
   }
}

[DebuggerNonUserCode]
TagTypeNode ^ VariantSelectorNode::TagType::get()
{
   return safe_cast<TagTypeNode ^>(this->second);
}

[DebuggerNonUserCode]
RecordVariableListNode ^ WithStatementNode::RecordVariableList::get()
{
   return safe_cast<RecordVariableListNode ^>(this->first);
}

[DebuggerNonUserCode]
BooleanExpressionNode ^ WhileStatementNode::BooleanExpression::get()
{
   return safe_cast<BooleanExpressionNode ^>(this->first);
}

// Sets the current debug tag for the current function unit.
void 
WhileStatementNode::SetCurrentDebugTag()
{
   // If the current function unit is valid, then
   // set the current debug tag to the current
   // source file name and line number.

   Phx::FunctionUnit ^ functionUnit = ModuleBuilder::CurrentFunctionUnit;

   if (functionUnit != nullptr)
   {
      // Use the line number associated with the BooleanExpression part.

      UInt32 debugTag = functionUnit->DebugInfo->CreateTag(
         Phx::Name::New(
            functionUnit->Lifetime, 
            Pascal::ModuleBuilder::SourceFileName
         ),
         BooleanExpression->SourceLineNumber
      );
      functionUnit->CurrentDebugTag = debugTag;
   }
}

//-----------------------------------------------------------------------------
// IExpression interface implementations.
//-----------------------------------------------------------------------------

Phx::Types::Type ^ 
BooleanConstantNode::Type::get()
{
   return TypeBuilder::GetTargetType(NativeType::Boolean);
}

Phx::Types::Type ^ 
BooleanExpressionNode::Type::get()
{
   return this->Expression->Type;
}

Phx::Types::Type ^
GetExponentiationType
(
   PrimaryBaseNode ^ firstExpression, 
   ExponentiationBaseNode ^ secondExpression  
) 
{
   Phx::Types::Type ^ integerType = 
      TypeBuilder::GetTargetType(NativeType::Integer);
   Phx::Types::Type ^ realType = 
      TypeBuilder::GetTargetType(NativeType::Real);
   Phx::Types::Type ^ unknownType = 
      TypeBuilder::GetTargetType(NativeType::Unknown);

   if (firstExpression && secondExpression)
   {
      Phx::Types::Type ^ first = firstExpression->Type;
      Phx::Types::Type ^ second = secondExpression->Type;

      // Test for function types,    
      // such as: x**f or f**x
      // where f designates a function call, and x designates
      // a type matching the return value of f.

      if (first->IsFunctionType)
         first = first->AsFunctionType->ReturnType;
      if (second->IsFunctionType)
         second = second->AsFunctionType->ReturnType;

      // For exponentiation (**), the both operators may be real or integer.

      if (! first->Equals(integerType) &&
          ! first->Equals(realType))
      {
         Output::ReportError(
            firstExpression->SourceLineNumber,
            Error::GeneralTypeExpected,
            "integer or real"
         );
         return unknownType;      
      }
      
      if (! second->Equals(integerType) &&
          ! second->Equals(realType))
      {
         Output::ReportError(
            secondExpression->SourceLineNumber,
            Error::GeneralTypeExpected,
            "integer or real"
         );
         return unknownType;      
      }

      return realType;
   }
   else
   {
      return firstExpression->Type;
   }
}

Phx::Types::Type ^ 
ConstantExponentiationNode::Type::get()
{
   return GetExponentiationType(
      this->ConstantPrimary,
      this->ConstantExponentiation);
}

Phx::Types::Type ^ 
ExponentiationNode::Type::get()
{
   return GetExponentiationType(
      this->Primary,
      this->Exponentiation);
}

Phx::Types::Type ^
GetExpressionType
(
   SimpleExpressionBaseNode ^ firstExpression, 
   SimpleExpressionBaseNode ^ secondExpression,
   unsigned relationalOperator
) 
{
   if (firstExpression && secondExpression)
   {
      Phx::Types::Type ^ first = firstExpression->Type;
      Phx::Types::Type ^ second = secondExpression->Type;

      // Check for function types.

      if (first->IsFunctionType)
         first = first->AsFunctionType->ReturnType;
      if (second->IsFunctionType)
         second = second->AsFunctionType->ReturnType;

      switch (relationalOperator)
      {      
      case EQUAL:
      case NOTEQUAL:
         
         if (TypeBuilder::IsSetType(first) && TypeBuilder::IsSetType(second))
         {
            if(TypeBuilder::AreEquivalentTypes(first, second))
            {
               return TypeBuilder::GetTargetType(NativeType::Boolean);
            }
            return TypeBuilder::GetTargetType(NativeType::Unknown);      
         }

         if (! TypeBuilder::IsSimpleType(first))
         {
            Output::ReportError(
               firstExpression->SourceLineNumber,
               Error::GeneralTypeExpected,
               "simple"
            );
            return TypeBuilder::GetTargetType(NativeType::Unknown);            
         }
         if (! TypeBuilder::IsSimpleType(second))
         {
            Output::ReportError(
               secondExpression->SourceLineNumber,
               Error::GeneralTypeExpected,
               "simple"
            );
            return TypeBuilder::GetTargetType(NativeType::Unknown);            
         }
     
         return TypeBuilder::GetTargetType(NativeType::Boolean);

      case LT:
      case GT:

         if (! TypeBuilder::IsSimpleType(first))
         {
            Output::ReportError(
               firstExpression->SourceLineNumber,
               Error::GeneralTypeExpected,
               "simple"
            );
            return TypeBuilder::GetTargetType(NativeType::Unknown);            
         }
         if (! TypeBuilder::IsSimpleType(second))
         {
            Output::ReportError(
               secondExpression->SourceLineNumber,
               Error::GeneralTypeExpected,
               "simple"
            );
            return TypeBuilder::GetTargetType(NativeType::Unknown);            
         }
      
         return TypeBuilder::GetTargetType(NativeType::Boolean);

      case LE:
      case GE:

         if (TypeBuilder::IsSetType(first) && TypeBuilder::IsSetType(second))
         {
            if(TypeBuilder::AreEquivalentTypes(first, second))
            {
               return first;
            }
            return TypeBuilder::GetTargetType(NativeType::Unknown);      
         }

         if (! TypeBuilder::IsSimpleType(first))
         {
            Output::ReportError(
               firstExpression->SourceLineNumber,
               Error::GeneralTypeExpected,
               "simple"
            );
            return TypeBuilder::GetTargetType(NativeType::Unknown);            
         }
         if (! TypeBuilder::IsSimpleType(second))
         {
            Output::ReportError(
               secondExpression->SourceLineNumber,
               Error::GeneralTypeExpected,
               "simple"
            );
            return TypeBuilder::GetTargetType(NativeType::Unknown);            
         }

         return TypeBuilder::GetTargetType(NativeType::Boolean);

      case IN:
         
         // First operand must be Ordinal.

         if (! TypeBuilder::IsOrdinalType(first))
         {
            Output::ReportError(
               firstExpression->SourceLineNumber,
               Error::GeneralTypeExpected,
               "ordinal"
            );
            return TypeBuilder::GetTargetType(NativeType::Unknown);            
         }

         // Second operand denotes host Set type.

         return second;
     
      default:
         Debug::Assert(false);
         return TypeBuilder::GetTargetType(NativeType::Unknown);      
      }
   }
   else
   {
      return firstExpression->Type;
   }
}

Phx::Types::Type ^ 
ExpressionNode::Type::get()
{
   return GetExpressionType(
      this->FirstSimpleExpression,
      this->SecondSimpleExpression,
      this->RelationalOperator);  
}

Phx::Types::Type ^ 
ConstantExpressionNode::Type::get()
{
   return GetExpressionType(
      this->FirstConstantSimpleExpression,
      this->SecondConstantSimpleExpression,
      this->RelationalOperator);
}

Phx::Types::Type ^ 
ConstantNode::Type::get()
{
   if (this->CharacterString)
   {
      return this->CharacterString->Type;
   }
   return this->NonString->Type;
}

Phx::Types::Type ^
GetFactorType
(
   FactorBaseNode ^ firstExpression, 
   ExponentiationBaseNode ^ secondExpression,
   unsigned sign
)
{
   Phx::Types::Type ^ type;
   int sourceLineNumber;

   if (firstExpression)
   {
      type = firstExpression->Type;
      sourceLineNumber = firstExpression->SourceLineNumber;
   }
   else if (secondExpression)
   {
      type = secondExpression->Type;
      sourceLineNumber = secondExpression->SourceLineNumber;
   }
   
   Debug::Assert(type != nullptr);

   // Test for function types.

   if (type->IsFunctionType)
      type = type->AsFunctionType->ReturnType;

   
   switch (sign)
   {
   case PLUS:
   case MINUS:

      // Type operand must be integer or real.

      if (! type->Equals(TypeBuilder::GetTargetType(NativeType::Integer)) &&
          ! type->Equals(TypeBuilder::GetTargetType(NativeType::Real)) )
      {
         Output::ReportError(
            sourceLineNumber,
            Error::GeneralTypeExpected,
            "integer or real"
         );
         return TypeBuilder::GetTargetType(NativeType::Unknown);            
      }
      break;

   default:
      break;
   }
      
   return type;
}

Phx::Types::Type ^ 
ConstantFactorNode::Type::get()
{
   return GetFactorType(
      this->ConstantFactor,
      this->ConstantExponentiation,
      this->Sign);
}

Phx::Types::Type ^ 
ComponentTypeNode::Type::get()
{
   return this->TypeDenoter->Type;
}

Phx::Types::Type ^ 
FactorNode::Type::get()
{
   return GetFactorType(
      this->Factor,
      this->Exponentiation,
      this->Sign);
}

Phx::Types::Type ^ 
FunctionDesignatorNode::Type::get()
{
   Phx::FunctionUnit ^ functionUnit = ModuleBuilder::CurrentFunctionUnit;
   String ^ functionName = this->Identifier->Name;

   if (ModuleBuilder::Runtime->IsRuntimeFunctionName(functionName))
   {
      // We need to supply an actual argument list to the runtime so it can 
      // distinguish between overloaded functions.

      List<Phx::IR::Operand ^> ^ arguments = IRBuilder::PushArgumentList();

      if (this->Params)
         this->Params->Accept(IAstVisitor::Current);

      IRBuilder::PopArgumentList();

      return ModuleBuilder::Runtime->GetFunctionReturnType(
         functionName,
         arguments
      );
   }
   else
   {
      Phx::Symbols::Symbol ^ symbol = 
         ModuleBuilder::LookupSymbol(functionUnit, functionName);

      // Functions also contain a local symbol by the same name, so to resolve
      // the symbol for a function call, go directly to the module's 
      // symbol table.

      if (symbol == nullptr || !symbol->Type->IsFunctionType)
      {         
         Phx::ModuleUnit ^ moduleUnit = functionUnit->ParentModuleUnit;

         Phx::Symbols::Symbol ^ moduleSymbol = 
            moduleUnit->SymbolTable->NameMap->Lookup(
               Phx::Name::New(moduleUnit->Lifetime, functionName)
            );

         if (moduleSymbol != nullptr)
            symbol = moduleSymbol;
      }

      if (symbol != nullptr)
      {
         Phx::Types::FunctionType ^ functionType = nullptr;
         if (symbol->Type->IsFunctionType)
         {
            functionType = symbol->Type->AsFunctionType;
         }
         else if (symbol->Type->IsPointerType && 
                  symbol->Type->AsPointerType->ReferentType->IsFunctionType)
         {
            functionType = 
               symbol->Type->AsPointerType->ReferentType->AsFunctionType;
         }

         if (functionType)
         {           
            return functionType;
         }
      }
      return functionUnit->TypeTable->UnknownType;
   }
}

Phx::Types::Type ^
FieldDesignatorNode::Type::get()
{
   return this->Identifier->Type;
}

Phx::Types::Type ^ 
IdentifierNode::Type::get()
{
   Phx::FunctionUnit ^ functionUnit = ModuleBuilder::CurrentFunctionUnit;
   
   Phx::Symbols::Symbol ^ symbol = ModuleBuilder::LookupSymbol(
      functionUnit,
      this->Name
   );
   
   if (symbol)
      return symbol->Type;
   return functionUnit->TypeTable->UnknownType;
}

Phx::Types::Type ^ 
NilNode::Type::get()
{
   return TypeBuilder::GetTargetType(NativeType::NilPointer);
}

Phx::Types::Type ^ 
ConstantPrimaryNode::Type::get()
{
   if (this->Identifier)
      return this->Identifier->Type;
   else if (this->ConstantExpression)
      return this->ConstantExpression->Type;
   else if (this->UnsignedConstant)
      return this->UnsignedConstant->Type;
   else if (this->ConstantPrimary)
      return this->ConstantPrimary->Type;

   __assume(0);
   Debug::Assert(false);
}

Phx::Types::Type ^ 
PrimaryNode::Type::get()
{
   if (this->VariableAccess)
      return this->VariableAccess->Type;
   else if (this->UnsignedConstant)
      return this->UnsignedConstant->Type;
   else if (this->FunctionDesignator)
      return this->FunctionDesignator->Type;
   else if (this->SetConstructor)
      return this->SetConstructor->Type;
   else if (this->Expression)
      return this->Expression->Type;
   else if (this->Primary)
      return this->Primary->Type;
   
   __assume(0);
   Debug::Assert(false);
}

Phx::Types::Type ^
GetSimpleExpressionType
(
   SimpleExpressionBaseNode ^ firstExpression, 
   TermBaseNode ^ secondExpression,
   unsigned addOp
) 
{
   Phx::Types::Type ^ first = firstExpression->Type;
   Phx::Types::Type ^ second = secondExpression->Type;
   
   Phx::Types::Type ^ booleanType = 
      TypeBuilder::GetTargetType(NativeType::Boolean);
   Phx::Types::Type ^ integerType = 
      TypeBuilder::GetTargetType(NativeType::Integer);
   Phx::Types::Type ^ realType    = 
      TypeBuilder::GetTargetType(NativeType::Real);
   Phx::Types::Type ^ unknownType = 
      TypeBuilder::GetTargetType(NativeType::Unknown);

   if (first->IsFunctionType)
   {
      first = first->AsFunctionType->ReturnType;
   }
   if (second->IsFunctionType)
   {
      second = second->AsFunctionType->ReturnType;
   }

   switch (addOp)
   {
   case PLUS:
   case MINUS:

      // Check for set union/difference operators.

      if (TypeBuilder::IsSetType(first) && TypeBuilder::IsSetType(second))
      {
         if (! TypeBuilder::AreEquivalentTypes(first, second))
         {
            Output::ReportError(
               firstExpression->SourceLineNumber,
               Error::IncompatibleSetTypes
            );               
            return unknownType;      
         }
         return first;
      }

      // At this point, both operators must be integer or real.
      // If either operator is real, result is real.

      if (! first->Equals(integerType) && ! first->Equals(realType))
      {
         Output::ReportError(
            firstExpression->SourceLineNumber,
            Error::GeneralTypeExpected,
            "set, integer or real"
         );
         return unknownType;            
      }

       if (! second->Equals(integerType) && ! second->Equals(realType))
      {
         Output::ReportError(
            secondExpression->SourceLineNumber,
            Error::GeneralTypeExpected,
            "set, integer or real"
         );
         return unknownType;            
      }

      if (first->Equals(realType) || second->Equals(realType))
         return realType;
      return integerType;

   case OR:

      // Both operators must be Boolean.

      if (! first->Equals(booleanType))
      {
         Output::ReportError(
            firstExpression->SourceLineNumber,
            Error::GeneralTypeExpected,
            "Boolean"
         );
      }

      if (! second->Equals(booleanType))
      {
         Output::ReportError(
            secondExpression->SourceLineNumber,
            Error::GeneralTypeExpected,
            "Boolean"
         );
         return unknownType;            
      }

      return booleanType;
   }
   return unknownType;
}

Phx::Types::Type ^ 
ConstantSimpleExpressionNode::Type::get()
{
   if (this->ConstantSimpleExpression && this->SecondConstantTerm)
   {
      return GetSimpleExpressionType(
         this->ConstantSimpleExpression,
         this->SecondConstantTerm,
         this->AddOperator);
   }
   else
   {
      return this->FirstConstantTerm->Type;
   }  
}

Phx::Types::Type ^ 
SimpleExpressionNode::Type::get()
{
   if (this->SimpleExpression && this->SecondTerm)
   {
      return GetSimpleExpressionType(
         this->SimpleExpression,
         this->SecondTerm,
         this->AddOperator);
   }
   else
   {
      return this->FirstTerm->Type;
   }  
}

Phx::Types::Type ^ 
SetConstructorNode::Type::get()
{
   Phx::Types::Type ^ baseType = nullptr;
   if (this->MemberDesignatorList)
   {
      baseType = this->MemberDesignatorList->Type;
   }
   return TypeBuilder::GetSetType(baseType);
}

Phx::Types::Type ^ 
MemberDesignatorListNode::Type::get()
{
   Phx::FunctionUnit ^ functionUnit = ModuleBuilder::CurrentFunctionUnit;
   Phx::Types::Type ^ type = nullptr;

   for each (MemberDesignatorNode ^ memberDesignatorNode in 
      this->MemberDesignators)
   {
      Phx::Types::Type ^ memberType = 
         memberDesignatorNode->Type;

      if (type == nullptr)
      {
         type = memberType;
      }
      else
      {
         if (! type->Equals(memberType))
         {
            Output::ReportError(
               this->SourceLineNumber,
               Error::MismatchedMemberDesignators
            );
            return functionUnit->TypeTable->UnknownType;
         }
      }
   }

   return type;
}

Phx::Types::Type ^ 
CharacterStringNode::Type::get()
{
   Phx::FunctionUnit ^ functionUnit = ModuleBuilder::CurrentFunctionUnit;

   Phx::Types::Type ^ charType = 
      TypeBuilder::GetTargetType(NativeType::Char);
   
   if (this->IsCharacterType)
   {
      return charType;  
   }

   // Create and register the new string type (based on the element count).
   // Register this like we would 'packed array[0..n] of char'.

   unsigned int elementCount = this->StringValue->Length;
   unsigned int bitSize = (charType->BitSize * elementCount);

   Phx::Name name = Phx::Name::New(
      functionUnit->Lifetime, 
      TypeBuilder::GetUniqueTypeName()
   );

   Phx::Symbols::TypeSymbol ^ typeSymbol = Phx::Symbols::TypeSymbol::New(
      functionUnit->SymbolTable,
      0,
      name
   );

   Phx::Types::UnmanagedArrayType ^ arrayType = 
      Phx::Types::UnmanagedArrayType::New(
         functionUnit->TypeTable,
         bitSize, 
         typeSymbol, 
   	   charType
      );

  array<TypedValueRange ^> ^ indexRanges = {
      gcnew TypedValueRange(
         functionUnit->TypeTable->Int32Type,
         0,
         elementCount-1
      )
   };

   TypeBuilder::RegisterArrayType(
      arrayType, 
      indexRanges, 
      this->SourceLineNumber
   );

   return arrayType;
}

Phx::Types::Type ^ 
NonStringNode::Type::get()
{
   switch (this->Kind)
   {
   case ValueKind::Identifier:
      return this->Identifier->Type;
   case ValueKind::IntValue:
      return TypeBuilder::GetTargetType(NativeType::Integer);
   case ValueKind::RealValue:
      return TypeBuilder::GetTargetType(NativeType::Real);
   default:
      Debug::Assert(false);
      return TypeBuilder::GetTargetType(NativeType::Unknown);
   }
}

Phx::Types::Type ^
GetTermType
(
   TermBaseNode ^ firstExpression, 
   FactorBaseNode ^ secondExpression,
   unsigned mulOp
)
{
   Phx::Types::Type ^ first = firstExpression->Type;
   Phx::Types::Type ^ second = secondExpression->Type;

   Phx::Types::Type ^ booleanType = 
      TypeBuilder::GetTargetType(NativeType::Boolean);
   Phx::Types::Type ^ integerType = 
      TypeBuilder::GetTargetType(NativeType::Integer);
   Phx::Types::Type ^ realType    = 
      TypeBuilder::GetTargetType(NativeType::Real);
   Phx::Types::Type ^ unknownType = 
      TypeBuilder::GetTargetType(NativeType::Unknown);

   // Test for function types,
   // such as: x/f or f*x
   // where f designates a function call, and x designates
   // a type matching the return value of f.

   if (first->IsFunctionType)
      first = first->AsFunctionType->ReturnType;
   if (second->IsFunctionType)
      second = second->AsFunctionType->ReturnType;

   switch (mulOp)
   {   
   case STAR:

      // Check for set intersection operator.

      if (TypeBuilder::IsSetType(first) && TypeBuilder::IsSetType(second))
      {
         if (! TypeBuilder::AreEquivalentTypes(first, second))
         {
            Output::ReportError(
               firstExpression->SourceLineNumber,
               Error::IncompatibleSetTypes
            );
            return unknownType;      
         }
         return first;
      }


   case SLASH:

      // Both operators must be integer or real.
      // If either operator is real, result is real.

      if (! first->Equals(integerType) &&
          ! first->Equals(realType))
      {
         Output::ReportError(
            firstExpression->SourceLineNumber,
            Error::GeneralTypeExpected,
            "integer or real"
         );
         return unknownType;            
      }
      if (! second->Equals(integerType) &&
          ! second->Equals(realType))
      {
         Output::ReportError(
            secondExpression->SourceLineNumber,
            Error::GeneralTypeExpected,
            "integer or real"
         );
         return unknownType;            
      }

      if (mulOp == SLASH || 
          (first->Equals(realType) || second->Equals(realType)))
         return realType;
      return integerType;

   case DIV:
   case MOD:

      // Modulus or integer division; both operators 
      // must be Integer.

      if (! first->Equals(integerType))
      {
         Output::ReportError(
            firstExpression->SourceLineNumber,
            Error::GeneralTypeExpected,
            "integer"
         );
         return unknownType;      
      }
      if (! second->Equals(integerType))
      {
         Output::ReportError(
            secondExpression->SourceLineNumber,
            Error::GeneralTypeExpected,
            "integer"
         );
         return unknownType;
      }
      return integerType;

   case AND:
      
      // Both operators must be Boolean.

      if (! first->Equals(booleanType))
      {
         Output::ReportError(
            firstExpression->SourceLineNumber,
            Error::GeneralTypeExpected,
            "Boolean"
         );
         return unknownType;
      }

      if (! second->Equals(booleanType))
      {
         Output::ReportError(
            secondExpression->SourceLineNumber,
            Error::GeneralTypeExpected,
            "Boolean"
         );
         return unknownType;            
      }

      return booleanType;

   default:
      Debug::Assert(false);
      return unknownType;      
   }
}

Phx::Types::Type ^ 
ConstantTermNode::Type::get()
{
   if (this->ConstantTerm && this->SecondConstantFactor)
   {
      return GetTermType(
         this->ConstantTerm,
         this->SecondConstantFactor,
         this->MulOperator);
   }
   else
   {
      return this->FirstConstantFactor->Type;
   }
}

Phx::Types::Type ^ 
TermNode::Type::get()
{
   if (this->Term && this->SecondFactor)
   {
      return GetTermType(
         this->Term,
         this->SecondFactor,
         this->MulOperator);
   }
   else
   {
      return this->FirstFactor->Type;
   }
}

Phx::Types::Type ^ 
TypeDenoterNode::Type::get()
{
   // If an identifier was provided, return the type associated 
   // with the identifier.

   if (this->Identifier)
   {
      return TypeBuilder::GetTargetType(this->Identifier->Name);
   }

   // Otherwise, this is an anonymous type.

   return TypeBuilder::GetTargetType(NativeType::Unknown);
}

Phx::Types::Type ^ 
UnsignedConstantNode::Type::get()
{
   if (this->UnsignedNumber)
      return this->UnsignedNumber->Type;
   else if (this->CharacterString)
      return this->CharacterString->Type;
   else if (this->Nil)
      return this->Nil->Type;
   else if (this->BooleanConstant)
      return this->BooleanConstant->Type;

   Debug::Assert(false);
   return TypeBuilder::GetTargetType(NativeType::Unknown); 
}

Phx::Types::Type ^ 
UnsignedIntegerNode::Type::get()
{
   return TypeBuilder::GetTargetType(NativeType::Integer);
}

Phx::Types::Type ^ 
UnsignedNumberNode::Type::get()
{
   if (this->UnsignedInteger)
      return this->UnsignedInteger->Type;
   else if(this->UnsignedReal)
      return this->UnsignedReal->Type;

   Debug::Assert(false);
   return TypeBuilder::GetTargetType(NativeType::Unknown); 
}

Phx::Types::Type ^ 
UnsignedRealNode::Type::get()
{
   return TypeBuilder::GetTargetType(NativeType::Real);
}

Phx::Types::Type ^ 
VariableAccessNode::Type::get()
{
   Phx::FunctionUnit ^ functionUnit = ModuleBuilder::CurrentFunctionUnit;

   if (this->Identifier)
   {
      Phx::Symbols::Symbol ^ symbol = 
         ModuleBuilder::LookupSymbol(functionUnit, this->Name);
      if (symbol)
      {
         if (ModuleBuilder::IsVariableParameterSymbol(symbol))
            return symbol->Type->AsPointerType->ReferentType;
         return symbol->Type;
      }
      return TypeBuilder::GetTargetType(NativeType::Unknown);
   }
   else if (this->FieldDesignator)
   {      
      Phx::Types::Type ^ accessType = 
         this->FieldDesignator->VariableAccess->Type;
      ModuleBuilder::PushScope(
         TypeBuilder::GetSymbolTable(accessType->AsAggregateType)
      );
      Phx::Symbols::Symbol ^ symbol = 
         ModuleBuilder::LookupSymbol(
            functionUnit, 
            this->FieldDesignator->Identifier->Name
         );
      ModuleBuilder::PopScope();
      if (symbol)
         return symbol->Type;   
      return TypeBuilder::GetTargetType(NativeType::Unknown);
   }
   else if (this->IndexedVariable)
   {      
      Phx::Types::Type ^ indexedVariableType = this->IndexedVariable->Type;
      
      if (indexedVariableType->IsArray)
      {    
         return indexedVariableType->AsUnmanagedArrayType->ElementType;   
      }
      return indexedVariableType;         
   }
   else
   {
      // This is variable_access^
      // Attempt to dereference the variable access.

      Phx::Types::Type ^ accessType = this->VariableAccess->Type;
      if (TypeBuilder::IsFileType(accessType))
      {
         return TypeBuilder::GetFileComponentType(accessType);
      }
      else if (TypeBuilder::IsPointerType(accessType))
      {
         return accessType->AsPointerType->ReferentType;
      }
      return TypeBuilder::GetTargetType(NativeType::Unknown);
   }
}

Phx::Types::Type ^ 
IndexedVariableNode::Type::get()
{
   if (this->VariableAccess)
   {
      return this->VariableAccess->Type;
   }
   else
   {
      return 
         this->IndexExpressionList->IndexExpressions[
            this->IndexExpressionList->IndexExpressions->Count - 1]->
               Expression->Type;
   }
}

Phx::Types::Type ^ 
MemberDesignatorNode::Type::get()
{
   Phx::FunctionUnit ^ functionUnit = ModuleBuilder::CurrentFunctionUnit;
   Phx::Types::Type ^ secondExpressionType = this->SecondExpression->Type;

   if (this->FirstExpression)
   {
      Phx::Types::Type ^ firstExpressionType = this->FirstExpression->Type;
      if (firstExpressionType->Equals(secondExpressionType))
      {
         return secondExpressionType;
      }
      else
      {
         return functionUnit->TypeTable->UnknownType;
      }
   }
   else
   {
      return secondExpressionType;
   }
}

bool
BooleanConstantNode::TypeCheck()
{
   return true;
}

bool
BooleanExpressionNode::TypeCheck()
{
   return true;
}

bool
CharacterStringNode::TypeCheck()
{
   return true;
}

bool
ConstantNode::TypeCheck()
{
   return true;
}

bool
ComponentTypeNode::TypeCheck()
{
   return true;
}

bool 
ExpressionNode::TypeCheck()
{
   bool first = this->FirstSimpleExpression->TypeCheck();
   if (this->SecondSimpleExpression)
   {
      return first && this->SecondSimpleExpression->TypeCheck();
   }
   else
   {
      return first;
   }
}

bool 
ConstantExpressionNode::TypeCheck()
{
   bool first = this->FirstConstantSimpleExpression->TypeCheck();
   if (this->SecondConstantSimpleExpression)
   {
      return first && this->SecondConstantSimpleExpression->TypeCheck();
   }
   else
   {
      return first;
   }
}

bool
FieldDesignatorNode::TypeCheck()
{
   return true;
}

bool
FunctionDesignatorNode::TypeCheck()
{
   return true;
}

bool
IdentifierNode::TypeCheck()
{
   return true;
}

bool
IndexedVariableNode::TypeCheck()
{
   return true;
}

bool
MemberDesignatorNode::TypeCheck()
{
   return true;
}

bool
NilNode::TypeCheck()
{
   return true;
}

bool
NonStringNode::TypeCheck()
{
   return true;
}

bool 
SimpleExpressionNode::TypeCheck()
{
   if (this->FirstTerm)
   {
      return this->FirstTerm->TypeCheck();
   }
   else
   {
      return this->SimpleExpression->TypeCheck() &&
         this->SecondTerm->TypeCheck();
   }
}

bool 
ConstantSimpleExpressionNode::TypeCheck()
{
   if (this->FirstConstantTerm)
   {
      return this->FirstConstantTerm->TypeCheck();
   }
   else
   {
      return this->ConstantSimpleExpression->TypeCheck() &&
         this->SecondConstantTerm->TypeCheck();
   }
}

bool 
TermNode::TypeCheck()
{
   if (this->FirstFactor)
   {
      return this->FirstFactor->TypeCheck();
   }
   else
   {
      return this->Term->TypeCheck() &&
         this->SecondFactor->TypeCheck();
   }
}

bool 
ConstantTermNode::TypeCheck()
{
   if (this->FirstConstantFactor)
   {
      return this->FirstConstantFactor->TypeCheck();
   }
   else
   {
      return this->ConstantTerm->TypeCheck() &&
         this->SecondConstantFactor->TypeCheck();
   }
}

bool 
FactorNode::TypeCheck()
{
   if (this->Factor)
   {
      return this->Factor->TypeCheck();
   }
   else
   {
      return this->Exponentiation->TypeCheck();
   }
}

bool 
ConstantFactorNode::TypeCheck()
{
   if (this->ConstantFactor)
   {
      return this->ConstantFactor->TypeCheck();
   }
   else
   {
      return this->ConstantExponentiation->TypeCheck();
   }
}

bool 
ExponentiationNode::TypeCheck()
{
   bool first = this->Primary->TypeCheck();
   if (this->Exponentiation)
   {
      return first && this->Exponentiation->TypeCheck();
   }
   else
   {
      return first;
   }
}

bool 
ConstantExponentiationNode::TypeCheck()
{
   bool first = this->ConstantPrimary->TypeCheck();
   if (this->ConstantExponentiation)
   {
      return first && this->ConstantExponentiation->TypeCheck();
   }
   else
   {
      return first;
   }
}

bool 
PrimaryNode::TypeCheck()
{
   Phx::FunctionUnit ^ functionUnit = ModuleBuilder::CurrentFunctionUnit;
   Phx::Symbols::Symbol ^ variableSymbol = nullptr;
   
   if (this->VariableAccess)
   {
      variableSymbol = 
         ModuleBuilder::LookupSymbol(functionUnit, this->VariableAccess->Name);
      if (variableSymbol == nullptr)
      {
         Output::ReportError(
            this->SourceLineNumber, 
            Error::UndeclaredIdentifier,
            this->VariableAccess->Name
         );
         return false;
      }
      return true;
   }

   else if (this->UnsignedConstant)
      return this->UnsignedConstant->TypeCheck();

   else if (this->FunctionDesignator)
      return this->FunctionDesignator->TypeCheck();

   else if (this->SetConstructor)
      return this->SetConstructor->TypeCheck();

   else if (this->Expression)
      return this->Expression->TypeCheck();

   else if (this->Primary)
      return this->Primary->TypeCheck();

   __assume(0);
   Debug::Assert(false);
}

bool 
ConstantPrimaryNode::TypeCheck()
{
   Phx::FunctionUnit ^ functionUnit = ModuleBuilder::CurrentFunctionUnit;
   Phx::Symbols::Symbol ^ variableSymbol = nullptr;

   if (this->Identifier)
   {
      variableSymbol = 
         ModuleBuilder::LookupSymbol(functionUnit, this->Identifier->Name);
      if (variableSymbol == nullptr)
      {
         Output::ReportError(
            this->SourceLineNumber, 
            Error::UndeclaredIdentifier,
            this->Identifier->Name
         );
         return false;
      }
      return true;
   }

   else if (this->ConstantExpression)
      return this->ConstantExpression->TypeCheck();

   else if (this->UnsignedConstant)
      return this->UnsignedConstant->TypeCheck();

   else if (this->ConstantPrimary)
      return this->ConstantPrimary->TypeCheck();

   __assume(0);
   Debug::Assert(false);
}

bool 
SetConstructorNode::TypeCheck()
{
   if (this->MemberDesignatorList)
   {
      return this->MemberDesignatorList->TypeCheck();      
   }
   else
   {
      // The empty set is OK.

      return true;
   }
}

bool 
MemberDesignatorListNode::TypeCheck()
{
   Phx::Types::Type ^ type = this->Type;
   if (type && 
      ! type->Equals(TypeBuilder::GetTargetType(NativeType::Unknown)))
      return true;
   return false;   
}

bool
TypeDenoterNode::TypeCheck()
{
   return true;
}

bool
UnsignedConstantNode::TypeCheck()
{
   return true;
}

bool
UnsignedIntegerNode::TypeCheck()
{
   return true;
}

bool
UnsignedNumberNode::TypeCheck()
{
   return true;
}

bool
UnsignedRealNode::TypeCheck()
{
   return true;
}

bool
VariableAccessNode::TypeCheck()
{
   return true;
}

}  // namespace Ast
