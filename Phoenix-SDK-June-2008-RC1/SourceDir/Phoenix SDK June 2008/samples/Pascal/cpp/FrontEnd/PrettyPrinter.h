//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the PrettyPrinter class.
//
//-----------------------------------------------------------------------------

#pragma once

#include "AstVisitorImpl.h"

using namespace System::IO;
using namespace System::Collections::Generic;

namespace Pascal 
{

//-----------------------------------------------------------------------------
//
// Description:
// 
//    A concrete implementation of the IAstVisitor interface.
//    This class pretty-prints the source program back to a provided 
//    output stream.
//    
// Remarks:
//    
//    Because this class acts on input data produced by lex/yacc, comments
//    are stripped from the resulting program listing.
//
//-----------------------------------------------------------------------------

ref class PrettyPrinter : public AstVisitorImpl
{
public:

   PrettyPrinter
   (
      TextWriter^ out
   )
      : AstVisitorImpl()
      , out(out)
      , indent(0)
      , indentString(String::Empty)
   {
   }

   virtual void Visit(Node^) override;
   virtual void Visit(UnaryNode^) override;
   virtual void Visit(BinaryNode^) override;
   virtual void Visit(TernaryNode^) override;
   virtual void Visit(PolyadicNode^) override;

   virtual void Visit(ActualParameterListNode^) override;
   virtual void Visit(ActualParameterNode^) override;
   virtual void Visit(ArrayTypeNode^) override;
   virtual void Visit(AssignmentStatementNode^) override;
   virtual void Visit(BaseTypeNode^) override;
   virtual void Visit(BlockNode^) override;
   virtual void Visit(BooleanConstantNode^) override;
   virtual void Visit(BooleanExpressionNode^) override;
   virtual void Visit(CaseConstantListNode^) override;
   virtual void Visit(CaseConstantNode^) override;
   virtual void Visit(CaseIndexNode^) override;
   virtual void Visit(CaseListElementListNode^) override;
   virtual void Visit(CaseListElementNode^) override;
   virtual void Visit(CaseStatementNode^) override;
   virtual void Visit(CharacterStringNode^) override;
   virtual void Visit(ClosedForStatementNode^) override;
   virtual void Visit(ClosedIfStatementNode^) override;
   virtual void Visit(ClosedStatementNode^) override;
   virtual void Visit(ClosedWhileStatementNode^) override;
   virtual void Visit(ClosedWithStatementNode^) override;
   virtual void Visit(ComponentTypeNode^) override;
   virtual void Visit(CompoundStatementNode^) override;
   virtual void Visit(ConstantDefinitionNode^) override;
   virtual void Visit(ConstantDefinitionPartNode^) override;
   virtual void Visit(ConstantExponentiationNode^) override;
   virtual void Visit(ConstantExpressionNode^) override;
   virtual void Visit(ConstantFactorNode^) override;
   virtual void Visit(ConstantListNode^) override;
   virtual void Visit(ConstantNode^) override;
   virtual void Visit(ConstantPrimaryNode^) override;
   virtual void Visit(ConstantSimpleExpressionNode^) override;
   virtual void Visit(ConstantTermNode^) override;
   virtual void Visit(ControlVariableNode^) override;
   virtual void Visit(DirectionNode^) override;
   virtual void Visit(DirectiveNode^) override;
   virtual void Visit(DomainTypeNode^) override;
   virtual void Visit(EnumeratedTypeNode^) override;
   virtual void Visit(ExponentiationBaseNode^) override;
   virtual void Visit(ExponentiationNode^) override;
   virtual void Visit(ExpressionBaseNode^) override;
   virtual void Visit(ExpressionNode^) override;
   virtual void Visit(FactorBaseNode^) override;
   virtual void Visit(FactorNode^) override;
   virtual void Visit(FieldDesignatorNode^) override;
   virtual void Visit(FileNode^) override;
   virtual void Visit(FileTypeNode^) override;
   virtual void Visit(FinalValueNode^) override;
   virtual void Visit(FormalParameterListNode^) override;
   virtual void Visit(FormalParameterNode^) override;
   virtual void Visit(FormalParameterSectionListNode^) override;
   virtual void Visit(FormalParameterSectionNode^) override;
   virtual void Visit(ForStatementNode^) override;
   virtual void Visit(FunctionalParameterSpecificationNode^) override;
   virtual void Visit(FunctionBlockNode^) override;
   virtual void Visit(FunctionDeclarationNode^) override;
   virtual void Visit(FunctionDesignatorNode^) override;
   virtual void Visit(FunctionHeadingNode^) override;
   virtual void Visit(FunctionIdentificationNode^) override;
   virtual void Visit(GotoStatementNode^) override;
   virtual void Visit(IdentifierListNode^) override;
   virtual void Visit(IdentifierNode^) override;
   virtual void Visit(IfStatementNode^) override;
   virtual void Visit(IndexedVariableNode^) override;
   virtual void Visit(IndexExpressionListNode^) override;
   virtual void Visit(IndexExpressionNode^) override;
   virtual void Visit(IndexListNode^) override;
   virtual void Visit(IndexTypeNode^) override;
   virtual void Visit(InitialValueNode^) override;
   virtual void Visit(LabelDeclarationPartNode^) override;
   virtual void Visit(LabelListNode^) override;
   virtual void Visit(LabelNode^) override;
   virtual void Visit(MemberDesignatorListNode^) override;
   virtual void Visit(MemberDesignatorNode^) override;
   virtual void Visit(ModuleNode^) override;
   virtual void Visit(NewOrdinalTypeNode^) override;
   virtual void Visit(NewPointerTypeNode^) override;
   virtual void Visit(NewStructuredTypeNode^) override;
   virtual void Visit(NewTypeNode^) override;
   virtual void Visit(NilNode^) override;
   virtual void Visit(NonLabeledClosedStatementNode^) override;
   virtual void Visit(NonLabeledOpenStatementNode^) override;
   virtual void Visit(NonStringNode^) override;
   virtual void Visit(OpenForStatementNode^) override;
   virtual void Visit(OpenIfStatementNode^) override;
   virtual void Visit(OpenStatementNode^) override;
   virtual void Visit(OpenWhileStatementNode^) override;
   virtual void Visit(OpenWithStatementNode^) override;
   virtual void Visit(OrdinalTypeNode^) override;
   virtual void Visit(OtherwisePartNode^) override;
   virtual void Visit(ParamsNode^) override;
   virtual void Visit(PrimaryBaseNode^) override;
   virtual void Visit(PrimaryNode^) override;
   virtual void Visit(ProceduralParameterSpecificationNode^) override;
   virtual void Visit(ProcedureAndFunctionDeclarationPartNode^) override;
   virtual void Visit(ProcedureBlockNode^) override;
   virtual void Visit(ProcedureDeclarationNode^) override;
   virtual void Visit(ProcedureHeadingNode^) override;
   virtual void Visit(ProcedureIdentificationNode^) override;
   virtual void Visit(ProcedureOrFunctionDeclarationListNode^) override;
   virtual void Visit(ProcedureOrFunctionDeclarationNode^) override;
   virtual void Visit(ProcedureStatementNode^) override;
   virtual void Visit(ProgramHeadingNode^) override;
   virtual void Visit(ProgramNode^) override;
   virtual void Visit(RecordSectionListNode^) override;
   virtual void Visit(RecordSectionNode^) override;
   virtual void Visit(RecordTypeNode^) override;
   virtual void Visit(RecordVariableListNode^) override;
   virtual void Visit(RepeatStatementNode^) override;
   virtual void Visit(ResultTypeNode^) override;
   virtual void Visit(SetConstructorNode^) override;
   virtual void Visit(SetTypeNode^) override;
   virtual void Visit(SimpleExpressionBaseNode^) override;
   virtual void Visit(SimpleExpressionNode^) override;
   virtual void Visit(StatementNode^) override;
   virtual void Visit(StatementPartNode^) override;
   virtual void Visit(StatementSequenceNode^) override;
   virtual void Visit(StringNode^) override;
   virtual void Visit(StructuredTypeNode^) override;
   virtual void Visit(SubrangeTypeNode^) override;
   virtual void Visit(TagFieldNode^) override;
   virtual void Visit(TagTypeNode^) override;
   virtual void Visit(TermBaseNode^) override;
   virtual void Visit(TermNode^) override;
   virtual void Visit(TypeDefinitionListNode^) override;
   virtual void Visit(TypeDefinitionNode^) override;
   virtual void Visit(TypeDefinitionPartNode^) override;
   virtual void Visit(TypeDenoterNode^) override;
   virtual void Visit(UnsignedConstantNode^) override;
   virtual void Visit(UnsignedIntegerNode^) override;
   virtual void Visit(UnsignedNumberNode^) override;
   virtual void Visit(UnsignedRealNode^) override;
   virtual void Visit(ValueParameterSpecificationNode^) override;
   virtual void Visit(VariableAccessNode^) override;
   virtual void Visit(VariableDeclarationListNode^) override;
   virtual void Visit(VariableDeclarationNode^) override;
   virtual void Visit(VariableDeclarationPartNode^) override;
   virtual void Visit(VariableParameterSpecificationNode^) override;
   virtual void Visit(VariantListNode^) override;
   virtual void Visit(VariantNode^) override;
   virtual void Visit(VariantPartNode^) override;
   virtual void Visit(VariantSelectorNode^) override;
   virtual void Visit(WhileStatementNode^) override;
   virtual void Visit(WithStatementNode^) override;
   
   // Common character symbols are abstracted to class properties so you
   // can easily replace syntax elements.

   static property String^ Colon
   {
      [DebuggerNonUserCode]
      String^ get() { return ":"; }
   }
   static property String^ Comma 
   {
      [DebuggerNonUserCode]
      String^ get() { return ","; }
   }
   static property String^ Dot 
   {
      [DebuggerNonUserCode]
      String^ get() { return "."; }
   }
   static property String^ Equals
   {
      [DebuggerNonUserCode]
      String^ get() { return "="; }
   }
   static property String^ FullQuote
   {
      [DebuggerNonUserCode]
      String^ get() { return "\""; }
   }   
   static property String^ GreaterThan
   {
      [DebuggerNonUserCode]
      String^ get() { return ">"; }
   }   
   static property String^ GreaterThanOrEqual
   {
      [DebuggerNonUserCode]
      String^ get() { return ">="; }
   }   
   static property String^ LeftBracket
   {
      [DebuggerNonUserCode]
      String^ get() { return "["; }
   }
   static property String^ LeftParenthesis
   {
      [DebuggerNonUserCode]
      String^ get() { return "("; }
   }
   static property String^ LessThan
   {
      [DebuggerNonUserCode]
      String^ get() { return "<"; }
   }   
   static property String^ LessThanOrEqual
   {
      [DebuggerNonUserCode]
      String^ get() { return "<="; }
   }   
   static property String^ Minus
   {
      [DebuggerNonUserCode]
      String^ get() { return "-"; }
   }   
   static property String^ Newline
   {
      [DebuggerNonUserCode]
      String^ get() { return Environment::NewLine; }
   }
   static property String^ NotEquals
   {
      [DebuggerNonUserCode] 
      String^ get() { return "<>"; }
   }
   static property String^ Plus
   {
      [DebuggerNonUserCode]
      String^ get() { return "+"; }
   }
   static property String^ RightBracket
   {
      [DebuggerNonUserCode]
      String^ get() { return "]"; }
   }
   static property String^ RightParenthesis
   {
      [DebuggerNonUserCode]
      String^ get() { return ")"; }
   }
   static property String^ Semicolon
   {
      [DebuggerNonUserCode]
      String^ get() { return ";"; }
   }
   static property String^ Slash
   {
      [DebuggerNonUserCode]
      String^ get() { return "/"; }
   }
   static property String^ Space
   {
      [DebuggerNonUserCode]
      String^ get() { return " "; }
   }
   static property String^ BackSpace
   {
      [DebuggerNonUserCode]
      String^ get() { return "\b"; }
   }
   static property String^ Star
   {
      [DebuggerNonUserCode]
      String^ get() { return "*"; }
   }

private: // properties

   // Establishes the number of indent characters to write after 
   // a newline is written.

   property int Indent
   {
      void set(int value);
      int get();
   }

   // Retrieves the current indent string.

   property String^ IndentString
   {
      String^ get();
   }

private: // methods

   // Prints the provided list of Node objects. The provided
   // separator string is printed between elements.

   void PrintNodes(List<Node^>^ list, String^ separator);

   // Writes the indent string to the output stream.
   inline void PrintIndent() { this->out->Write(IndentString); }

   // Prints the given number of 'unindent' characters to the output stream.

   void PrintUnindent(int n);

private: // members

   // The output stream writer.
   TextWriter^ out;

   // The current indent count.
   int indent;

   // The current indent string.
   String^ indentString;

   // Represents a single tab character string.   
   literal String^ tab = "   ";
};

} // namespace Pascal