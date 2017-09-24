//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the AstVisitorImpl class.
//
//-----------------------------------------------------------------------------

#pragma once

#include "AstVisitor.h"

using namespace System::Reflection;
using namespace Ast;

namespace Pascal 
{

//-----------------------------------------------------------------------------
//
// Description: Base implementation of the IAstVisitor interface.
//
// Remarks:
//              This class provides helper methods to easily traverse an AST.
//
//-----------------------------------------------------------------------------

ref class AstVisitorImpl abstract : public IAstVisitor
{
public:

   AstVisitorImpl()
      : visitChildrenType(nullptr)
      , visitChildrenCall(nullptr)
   {
      IAstVisitor::Current = this;
   }
   
   virtual void Visit(Node^) = 0;
   virtual void Visit(UnaryNode^) = 0;
   virtual void Visit(BinaryNode^) = 0;
   virtual void Visit(TernaryNode^) = 0;
   virtual void Visit(PolyadicNode^) = 0;

   virtual void Visit(ActualParameterListNode^) = 0;
   virtual void Visit(ActualParameterNode^) = 0;
   virtual void Visit(ArrayTypeNode^) = 0;
   virtual void Visit(AssignmentStatementNode^) = 0;
   virtual void Visit(BaseTypeNode^) = 0;
   virtual void Visit(BlockNode^) = 0;
   virtual void Visit(BooleanConstantNode^) = 0;
   virtual void Visit(BooleanExpressionNode^) = 0;
   virtual void Visit(CaseConstantListNode^) = 0;
   virtual void Visit(CaseConstantNode^) = 0;
   virtual void Visit(CaseIndexNode^) = 0;
   virtual void Visit(CaseListElementListNode^) = 0;
   virtual void Visit(CaseListElementNode^) = 0;
   virtual void Visit(CaseStatementNode^) = 0;
   virtual void Visit(CharacterStringNode^) = 0;
   virtual void Visit(ClosedForStatementNode^) = 0;
   virtual void Visit(ClosedIfStatementNode^) = 0;
   virtual void Visit(ClosedStatementNode^) = 0;
   virtual void Visit(ClosedWhileStatementNode^) = 0;
   virtual void Visit(ClosedWithStatementNode^) = 0;
   virtual void Visit(ComponentTypeNode^) = 0;
   virtual void Visit(CompoundStatementNode^) = 0;
   virtual void Visit(ConstantDefinitionNode^) = 0;
   virtual void Visit(ConstantDefinitionPartNode^) = 0;
   virtual void Visit(ConstantExponentiationNode^) = 0;
   virtual void Visit(ConstantExpressionNode^) = 0;
   virtual void Visit(ConstantFactorNode^) = 0;
   virtual void Visit(ConstantListNode^) = 0;
   virtual void Visit(ConstantNode^) = 0;
   virtual void Visit(ConstantPrimaryNode^) = 0;
   virtual void Visit(ConstantSimpleExpressionNode^) = 0;
   virtual void Visit(ConstantTermNode^) = 0;
   virtual void Visit(ControlVariableNode^) = 0;
   virtual void Visit(DirectionNode^) =0;
   virtual void Visit(DirectiveNode^) = 0;
   virtual void Visit(DomainTypeNode^) = 0;
   virtual void Visit(EnumeratedTypeNode^) = 0;
   virtual void Visit(ExponentiationBaseNode^) = 0;
   virtual void Visit(ExponentiationNode^) = 0;
   virtual void Visit(ExpressionBaseNode^) = 0;
   virtual void Visit(ExpressionNode^) = 0;
   virtual void Visit(FactorBaseNode^) = 0;
   virtual void Visit(FactorNode^) = 0;
   virtual void Visit(FieldDesignatorNode^) = 0;
   virtual void Visit(FileNode^) = 0;
   virtual void Visit(FileTypeNode^) = 0;
   virtual void Visit(FinalValueNode^) = 0;
   virtual void Visit(FormalParameterListNode^) = 0;
   virtual void Visit(FormalParameterNode^) = 0;
   virtual void Visit(FormalParameterSectionListNode^) = 0;
   virtual void Visit(FormalParameterSectionNode^) = 0;
   virtual void Visit(ForStatementNode^) = 0;
   virtual void Visit(FunctionalParameterSpecificationNode^) = 0;
   virtual void Visit(FunctionBlockNode^) = 0;
   virtual void Visit(FunctionDeclarationNode^) = 0;
   virtual void Visit(FunctionDesignatorNode^) = 0;
   virtual void Visit(FunctionHeadingNode^) = 0;
   virtual void Visit(FunctionIdentificationNode^) = 0;
   virtual void Visit(GotoStatementNode^) = 0;
   virtual void Visit(IdentifierListNode^) = 0;
   virtual void Visit(IdentifierNode^) = 0;
   virtual void Visit(IfStatementNode^) = 0;
   virtual void Visit(IndexedVariableNode^) = 0;
   virtual void Visit(IndexExpressionListNode^) = 0;
   virtual void Visit(IndexExpressionNode^) = 0;
   virtual void Visit(IndexListNode^) = 0;
   virtual void Visit(IndexTypeNode^) = 0;
   virtual void Visit(InitialValueNode^) = 0;
   virtual void Visit(LabelDeclarationPartNode^) = 0;
   virtual void Visit(LabelListNode^) = 0;
   virtual void Visit(LabelNode^) = 0;
   virtual void Visit(MemberDesignatorListNode^) = 0;
   virtual void Visit(MemberDesignatorNode^) = 0;
   virtual void Visit(ModuleNode^) = 0;
   virtual void Visit(NewOrdinalTypeNode^) = 0;
   virtual void Visit(NewPointerTypeNode^) = 0;
   virtual void Visit(NewStructuredTypeNode^) = 0;
   virtual void Visit(NewTypeNode^) = 0;
   virtual void Visit(NilNode^) = 0;
   virtual void Visit(NonLabeledClosedStatementNode^) = 0;
   virtual void Visit(NonLabeledOpenStatementNode^) = 0;
   virtual void Visit(NonStringNode^) = 0;
   virtual void Visit(OpenForStatementNode^) = 0;
   virtual void Visit(OpenIfStatementNode^) = 0;
   virtual void Visit(OpenStatementNode^) = 0;
   virtual void Visit(OpenWhileStatementNode^) = 0;
   virtual void Visit(OpenWithStatementNode^) = 0;
   virtual void Visit(OrdinalTypeNode^) = 0;
   virtual void Visit(OtherwisePartNode^) = 0;
   virtual void Visit(ParamsNode^) = 0;
   virtual void Visit(PrimaryBaseNode^) = 0;
   virtual void Visit(PrimaryNode^) = 0;
   virtual void Visit(ProceduralParameterSpecificationNode^) = 0;
   virtual void Visit(ProcedureAndFunctionDeclarationPartNode^) = 0;
   virtual void Visit(ProcedureBlockNode^) = 0;
   virtual void Visit(ProcedureDeclarationNode^) = 0;
   virtual void Visit(ProcedureHeadingNode^) = 0;
   virtual void Visit(ProcedureIdentificationNode^) = 0;
   virtual void Visit(ProcedureOrFunctionDeclarationListNode^) = 0;
   virtual void Visit(ProcedureOrFunctionDeclarationNode^) = 0;
   virtual void Visit(ProcedureStatementNode^) = 0;
   virtual void Visit(ProgramHeadingNode^) = 0;
   virtual void Visit(ProgramNode^) = 0;
   virtual void Visit(RecordSectionListNode^) = 0;
   virtual void Visit(RecordSectionNode^) = 0;
   virtual void Visit(RecordTypeNode^) = 0;
   virtual void Visit(RecordVariableListNode^) = 0;
   virtual void Visit(RepeatStatementNode^) = 0;
   virtual void Visit(ResultTypeNode^) = 0;
   virtual void Visit(SetConstructorNode^) = 0;
   virtual void Visit(SetTypeNode^) = 0;
   virtual void Visit(SimpleExpressionBaseNode^) = 0;
   virtual void Visit(SimpleExpressionNode^) = 0;
   virtual void Visit(StatementNode^) = 0;
   virtual void Visit(StatementPartNode^) = 0;
   virtual void Visit(StatementSequenceNode^) = 0;
   virtual void Visit(StringNode^) = 0;
   virtual void Visit(StructuredTypeNode^) = 0;
   virtual void Visit(SubrangeTypeNode^) = 0;
   virtual void Visit(TagFieldNode^) = 0;
   virtual void Visit(TagTypeNode^) = 0;
   virtual void Visit(TermBaseNode^) = 0;
   virtual void Visit(TermNode^) = 0;
   virtual void Visit(TypeDefinitionListNode^) = 0;
   virtual void Visit(TypeDefinitionNode^) = 0;
   virtual void Visit(TypeDefinitionPartNode^) = 0;
   virtual void Visit(TypeDenoterNode^) = 0;
   virtual void Visit(UnsignedConstantNode^) = 0;
   virtual void Visit(UnsignedIntegerNode^) = 0;
   virtual void Visit(UnsignedNumberNode^) = 0;
   virtual void Visit(UnsignedRealNode^) = 0;
   virtual void Visit(ValueParameterSpecificationNode^) = 0;
   virtual void Visit(VariableAccessNode^) = 0;
   virtual void Visit(VariableDeclarationListNode^) = 0;
   virtual void Visit(VariableDeclarationNode^) = 0;
   virtual void Visit(VariableDeclarationPartNode^) = 0;
   virtual void Visit(VariableParameterSpecificationNode^) = 0;
   virtual void Visit(VariantListNode^) = 0;
   virtual void Visit(VariantNode^) = 0;
   virtual void Visit(VariantPartNode^) = 0;
   virtual void Visit(VariantSelectorNode^) = 0;
   virtual void Visit(WhileStatementNode^) = 0;
   virtual void Visit(WithStatementNode^) = 0;
   
protected: // methods

   // Visits the child nodes of a given AST node.

   void VisitChildren(Node^ node);

   // Delegate type to visit a specific type of AST node.

   delegate void VisitChildrenDelegate(Node^ node);

   // Visits an AST node with one child.

   void VisitUnaryNode(Node^ node);

   // Visits an AST node with two children.

   void VisitBinaryNode(Node^ node);

   // Visits an AST node with three children.

   void VisitTernaryNode(Node^ node);

   // Visits an AST node with many children.

   void VisitPolyadicNode(Node^ node);

private:   // members

   //
   // AST traversal helpers
   //

   // Array of abstract types that derive from Node.
   array<System::Type^>^ visitChildrenType;

   // Parallel array of delegate methods for the above type array.
   array<VisitChildrenDelegate^>^ visitChildrenCall;
};

} // namespace Pascal 