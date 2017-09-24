//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the IAstVisitor interface.
//
//-----------------------------------------------------------------------------

#pragma once

#include "AstClasses.h"

using namespace System::IO;
using namespace System::Collections::Generic;

namespace Ast 
{

//-----------------------------------------------------------------------------
//
// Description: Interface for visiting nodes in an AST.
//
// Remarks:
//              Rather than defining action in the AST itself, we use the
//              Visitor pattern so we can define AST actions externally.
//
//-----------------------------------------------------------------------------

interface class IAstVisitor
{
public:
   void Visit(Node^);
   void Visit(UnaryNode^);
   void Visit(BinaryNode^);
   void Visit(TernaryNode^);
   void Visit(PolyadicNode^);

   void Visit(ActualParameterListNode^);
   void Visit(ActualParameterNode^);
   void Visit(ArrayTypeNode^);
   void Visit(AssignmentStatementNode^);
   void Visit(BaseTypeNode^);
   void Visit(BlockNode^);
   void Visit(BooleanConstantNode^);
   void Visit(BooleanExpressionNode^);
   void Visit(CaseConstantListNode^);
   void Visit(CaseConstantNode^);
   void Visit(CaseIndexNode^);
   void Visit(CaseListElementListNode^);
   void Visit(CaseListElementNode^);
   void Visit(CaseStatementNode^);
   void Visit(CharacterStringNode^);
   void Visit(ClosedForStatementNode^);
   void Visit(ClosedIfStatementNode^);
   void Visit(ClosedStatementNode^);
   void Visit(ClosedWhileStatementNode^);
   void Visit(ClosedWithStatementNode^);
   void Visit(ComponentTypeNode^);
   void Visit(CompoundStatementNode^);
   void Visit(ConstantDefinitionNode^);
   void Visit(ConstantDefinitionPartNode^);
   void Visit(ConstantExponentiationNode^);
   void Visit(ConstantExpressionNode^);
   void Visit(ConstantFactorNode^);
   void Visit(ConstantListNode^);
   void Visit(ConstantNode^);
   void Visit(ConstantPrimaryNode^);
   void Visit(ConstantSimpleExpressionNode^);
   void Visit(ConstantTermNode^);
   void Visit(ControlVariableNode^);
   void Visit(DirectionNode^);
   void Visit(DirectiveNode^);
   void Visit(DomainTypeNode^);
   void Visit(EnumeratedTypeNode^);
   void Visit(ExponentiationBaseNode^);
   void Visit(ExponentiationNode^);
   void Visit(ExpressionBaseNode^);
   void Visit(ExpressionNode^);
   void Visit(FactorBaseNode^);
   void Visit(FactorNode^);
   void Visit(FieldDesignatorNode^);
   void Visit(FileNode^);
   void Visit(FileTypeNode^);
   void Visit(FinalValueNode^);
   void Visit(FormalParameterListNode^);
   void Visit(FormalParameterNode^);
   void Visit(FormalParameterSectionListNode^);
   void Visit(FormalParameterSectionNode^);
   void Visit(ForStatementNode^);
   void Visit(FunctionalParameterSpecificationNode^);
   void Visit(FunctionBlockNode^);
   void Visit(FunctionDeclarationNode^);
   void Visit(FunctionDesignatorNode^);
   void Visit(FunctionHeadingNode^);
   void Visit(FunctionIdentificationNode^);
   void Visit(GotoStatementNode^);
   void Visit(IdentifierListNode^);
   void Visit(IdentifierNode^);
   void Visit(IfStatementNode^);
   void Visit(IndexedVariableNode^);
   void Visit(IndexExpressionListNode^);
   void Visit(IndexExpressionNode^);
   void Visit(IndexListNode^);
   void Visit(IndexTypeNode^);
   void Visit(InitialValueNode^);
   void Visit(LabelDeclarationPartNode^);
   void Visit(LabelListNode^);
   void Visit(LabelNode^);
   void Visit(MemberDesignatorListNode^);
   void Visit(MemberDesignatorNode^);
   void Visit(ModuleNode^);
   void Visit(NewOrdinalTypeNode^);
   void Visit(NewPointerTypeNode^);
   void Visit(NewStructuredTypeNode^);
   void Visit(NewTypeNode^);
   void Visit(NilNode^);
   void Visit(NonLabeledClosedStatementNode^);
   void Visit(NonLabeledOpenStatementNode^);
   void Visit(NonStringNode^);
   void Visit(OpenForStatementNode^);
   void Visit(OpenIfStatementNode^);
   void Visit(OpenStatementNode^);
   void Visit(OpenWhileStatementNode^);
   void Visit(OpenWithStatementNode^);
   void Visit(OrdinalTypeNode^);
   void Visit(OtherwisePartNode^);
   void Visit(ParamsNode^);
   void Visit(PrimaryBaseNode^);
   void Visit(PrimaryNode^);
   void Visit(ProceduralParameterSpecificationNode^);
   void Visit(ProcedureAndFunctionDeclarationPartNode^);
   void Visit(ProcedureBlockNode^);
   void Visit(ProcedureDeclarationNode^);
   void Visit(ProcedureHeadingNode^);
   void Visit(ProcedureIdentificationNode^);
   void Visit(ProcedureOrFunctionDeclarationListNode^);
   void Visit(ProcedureOrFunctionDeclarationNode^);
   void Visit(ProcedureStatementNode^);
   void Visit(ProgramHeadingNode^);
   void Visit(ProgramNode^);
   void Visit(RecordSectionListNode^);
   void Visit(RecordSectionNode^);
   void Visit(RecordTypeNode^);
   void Visit(RecordVariableListNode^);
   void Visit(RepeatStatementNode^);
   void Visit(ResultTypeNode^);
   void Visit(SetConstructorNode^);
   void Visit(SetTypeNode^);
   void Visit(SimpleExpressionBaseNode^);
   void Visit(SimpleExpressionNode^);
   void Visit(StatementNode^);
   void Visit(StatementPartNode^);
   void Visit(StatementSequenceNode^);
   void Visit(StringNode^);
   void Visit(StructuredTypeNode^);
   void Visit(SubrangeTypeNode^);
   void Visit(TagFieldNode^);
   void Visit(TagTypeNode^);
   void Visit(TermBaseNode^);
   void Visit(TermNode^);
   void Visit(TypeDefinitionListNode^);
   void Visit(TypeDefinitionNode^);
   void Visit(TypeDefinitionPartNode^);
   void Visit(TypeDenoterNode^);
   void Visit(UnsignedConstantNode^);
   void Visit(UnsignedIntegerNode^);
   void Visit(UnsignedNumberNode^);
   void Visit(UnsignedRealNode^);
   void Visit(ValueParameterSpecificationNode^);
   void Visit(VariableAccessNode^);
   void Visit(VariableDeclarationListNode^);
   void Visit(VariableDeclarationNode^);
   void Visit(VariableDeclarationPartNode^);
   void Visit(VariableParameterSpecificationNode^);
   void Visit(VariantListNode^);
   void Visit(VariantNode^);
   void Visit(VariantPartNode^);
   void Visit(VariantSelectorNode^);
   void Visit(WhileStatementNode^);
   void Visit(WithStatementNode^);

public:
   static property IAstVisitor ^ Current;
};

} // namespace Ast 
