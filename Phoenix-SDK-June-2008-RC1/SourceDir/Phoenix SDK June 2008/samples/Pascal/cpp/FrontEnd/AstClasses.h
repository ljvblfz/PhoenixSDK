//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//     Forward declarations for all AST classes.
//
//-----------------------------------------------------------------------------

#pragma once

namespace Ast 
{

ref class Node;
ref class UnaryNode;
ref class BinaryNode;
ref class TernaryNode;
ref class PolyadicNode;

ref class ActualParameterListNode;
ref class ActualParameterNode;
ref class ArrayTypeNode;
ref class AssignmentStatementNode;
ref class BaseTypeNode;
ref class BlockNode;
ref class BooleanConstantNode;
ref class BooleanExpressionNode;
ref class CaseConstantListNode;
ref class CaseConstantNode;
ref class CaseIndexNode;
ref class CaseListElementListNode;
ref class CaseListElementNode;
ref class CaseStatementNode;
ref class ClosedForStatementNode;
ref class ClosedIfStatementNode;
ref class ClosedStatementNode;
ref class ClosedWhileStatementNode;
ref class ClosedWithStatementNode;
ref class ComponentTypeNode;
ref class CompoundStatementNode;
ref class ConstantDefinitionNode;
ref class ConstantDefinitionPartNode;
ref class ConstantExponentiationNode;
ref class ConstantExpressionNode;
ref class ConstantFactorNode;
ref class ConstantListNode;
ref class ConstantNode;
ref class ConstantPrimaryNode;
ref class ConstantSimpleExpressionNode;
ref class ConstantTermNode;
ref class ControlVariableNode;
ref class DirectionNode;
ref class CharacterStringNode;
ref class DirectiveNode;
ref class DomainTypeNode;
ref class EnumeratedTypeNode;
ref class ExponentiationBaseNode;
ref class ExponentiationNode;
ref class ExpressionBaseNode;
ref class ExpressionNode;
ref class FactorBaseNode;
ref class FactorNode;
ref class FieldDesignatorNode;
ref class FileNode;
ref class FileTypeNode;
ref class FinalValueNode;
ref class FormalParameterListNode;
ref class FormalParameterNode;
ref class FormalParameterSectionListNode;
ref class FormalParameterSectionNode;
ref class ForStatementNode;
ref class FunctionalParameterSpecificationNode;
ref class FunctionBlockNode;
ref class FunctionDeclarationNode;
ref class FunctionDesignatorNode;
ref class FunctionHeadingNode;
ref class FunctionIdentificationNode;
ref class GotoStatementNode;
ref class IdentifierListNode;
ref class IdentifierNode;
ref class IfStatementNode;
ref class IndexedVariableNode;
ref class IndexExpressionListNode;
ref class IndexExpressionNode;
ref class IndexListNode;
ref class IndexTypeNode;
ref class InitialValueNode;
ref class LabelDeclarationPartNode;
ref class LabelListNode;
ref class LabelNode;
ref class MemberDesignatorListNode;
ref class MemberDesignatorNode;
ref class ModuleNode;
ref class NewOrdinalTypeNode;
ref class NewPointerTypeNode;
ref class NewStructuredTypeNode;
ref class NewTypeNode;
ref class NilNode;
ref class NonLabeledClosedStatementNode;
ref class NonLabeledOpenStatementNode;
ref class NonStringNode;
ref class OpenForStatementNode;
ref class OpenIfStatementNode;
ref class OpenStatementNode;
ref class OpenWhileStatementNode;
ref class OpenWithStatementNode;
ref class OrdinalTypeNode;
ref class OtherwisePartNode;
ref class ParamsNode;
ref class PrimaryBaseNode;
ref class PrimaryNode;
ref class ProceduralParameterSpecificationNode;
ref class ProcedureAndFunctionDeclarationPartNode;
ref class ProcedureBlockNode;
ref class ProcedureDeclarationNode;
ref class ProcedureHeadingNode;
ref class ProcedureIdentificationNode;
ref class ProcedureOrFunctionDeclarationListNode;
ref class ProcedureOrFunctionDeclarationNode;
ref class ProcedureStatementNode;
ref class ProgramHeadingNode;
ref class ProgramNode;
ref class RecordSectionListNode;
ref class RecordSectionNode;
ref class RecordTypeNode;
ref class RecordVariableListNode;
ref class RepeatStatementNode;
ref class ResultTypeNode;
ref class SetConstructorNode;
ref class SetTypeNode;
ref class SimpleExpressionBaseNode;
ref class SimpleExpressionNode;
ref class StatementNode;
ref class StatementPartNode;
ref class StatementSequenceNode;
ref class StringNode;
ref class StringNode;
ref class StructuredTypeNode;
ref class SubrangeTypeNode;
ref class TagFieldNode;
ref class TagTypeNode;
ref class TermBaseNode;
ref class TermNode;
ref class TypeDefinitionListNode;
ref class TypeDefinitionNode;
ref class TypeDefinitionPartNode;
ref class TypeDenoterNode;
ref class UnsignedConstantNode;
ref class UnsignedIntegerNode;
ref class UnsignedNumberNode;
ref class UnsignedRealNode;
ref class ValueParameterSpecificationNode;
ref class ValueParameterSpecificationNode;
ref class VariableAccessNode;
ref class VariableDeclarationListNode;
ref class VariableDeclarationNode;
ref class VariableDeclarationPartNode;
ref class VariableParameterSpecificationNode;
ref class VariantListNode;
ref class VariantNode;
ref class VariantPartNode;
ref class VariantSelectorNode;
ref class WhileStatementNode;
ref class WithStatementNode;

interface class IAstVisitor;

}  // namespace Ast