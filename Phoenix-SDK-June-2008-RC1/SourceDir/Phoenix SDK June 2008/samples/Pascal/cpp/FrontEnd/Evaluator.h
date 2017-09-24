//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the Evaluator class.
//
//-----------------------------------------------------------------------------

#pragma once

#include "AstVisitorImpl.h"

using namespace System::IO;
using namespace System::Collections::Generic;
using namespace Ast;

namespace Pascal 
{

ref class VariableOrFieldAccess;

//-----------------------------------------------------------------------------
//
// Description: 
//
//    A concrete implementation of the IAstVisitor interface.
//    This class generates Phoenix Intermediate Representation (IR) for a given
//    abstract syntax tree.
//
// Remarks:
//    
//
//-----------------------------------------------------------------------------

ref class Evaluator : public AstVisitorImpl
{
public:

   Evaluator
   (
      String ^ sourceFileName
   )
      : AstVisitorImpl()
      , sourceFileName(sourceFileName)
      , withAccessNodes(gcnew List<VariableOrFieldAccess ^>())
      , pointerTypeFixups(gcnew Dictionary<String ^, String ^>())
      , fieldDesignatorCount(0)
      , unionBitOffset(nullptr)
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
   
public:    // properties

   // Obtains the ModuleUnit object created during evaluation
   // of the AST.

   property Phx::ModuleUnit ^ ModuleUnit
   {
      Phx::ModuleUnit ^ get()
      {
         return this->moduleUnit;
      }
   }

private: // properties

   // The current operand produced by many AST nodes.

   property Phx::IR::Operand ^ CurrentOperand;
 
   // The string name of the curren type being built.

   property String ^ NewTypeName;

   // Tracks the current With statement nesting count.

   property int WithNestingCount;
   
private: // methods

   // Creates a new Phx::Types::Type ^ object from the 
   // given TypeDenoterNode object.

   Phx::Types::Type ^ ResolveNewType
   (
      TypeDenoterNode ^ node
   );

   // Builds the index operand for an array variable access.

   Phx::IR::VariableOperand ^ 
   BuildArrayIndexOperand
   (
      Phx::Types::UnmanagedArrayType ^ arrayType,
      IndexedVariableNode ^ node
   );

   // Extracts the ordinal value from the given ConstantNode object.

   int
   GetOrdinalValue
   (
      Phx::FunctionUnit ^ functionUnit,
      ConstantNode ^ node,
      bool % error
   );

   // Builds a procedure call from the provided procedure name and parameters.

   void
   BuildProcedureCall
   (
      String ^ procedureName,
      ParamsNode ^ paramsNode,
      int sourceLineNumber
   );

   // Helper function for building For loops.
   
   void
   BuildForLoop
   (
      ForStatementNode ^ node
   );

   // Helper function for building While loops.

   void
   BuildWhileLoop
   (
      WhileStatementNode ^ node
   );

   // Helper function for building With statements.

   void
   BuildWithStatement
   (
      WithStatementNode ^ node,
      StatementNodeBase ^ statementNode
   );

   // Determines whether the given symbol is a field of the given 
   // variable/field access.

   bool 
   IsFieldOf
   (
      Phx::Symbols::FieldSymbol ^ fieldSymbol,
      VariableOrFieldAccess ^ variableOrFieldAccess      
   );

   // Walks the list of record variable access specifiers and finds the 
   // first symbol that matches the symbol's enclosing type.

   Phx::IR::Operand ^
   ExpandWithSymbol
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Symbols::FieldSymbol ^ fieldSymbol,
      int sourceLineNumber
   );

   // Calculates the bit size of the given RecordSectionListNode object.

   unsigned int
   CalculateBitSize
   (
      RecordSectionListNode ^ node
   );

   // Calculates the bit size of the given RecordSectionNode object.

   unsigned int
   CalculateBitSize
   (
      RecordSectionNode ^ node
   );

   // Calculates the bit size of the given VariantPartNode object.

   unsigned int
   CalculateBitSize
   (
      VariantPartNode ^ node
   );
   
   // Adds the elements of the given RecordSectionListNode object to the
   // given aggregate type.

   void
   AddFieldsToType
   (
      Phx::Types::AggregateType ^ type,
      RecordSectionListNode ^ node
   );

   // Adds the elements of the given RecordSectionNode object to the 
   // given aggregate type.

   void
   AddFieldsToType
   (
      Phx::Types::AggregateType ^ type,
      RecordSectionNode ^ node,
      int absoluteBitOffset
   );

   // Adds the elements of the given VariantPartNode object to the 
   // given aggregate type.
      
   void
   AddFieldsToType
   (
      Phx::Types::AggregateType ^ type,
      VariantPartNode ^ node
   );

   // Resolves forward declarations to undefined pointer types.

   void
   FixupPointerTypes
   (
      Phx::FunctionUnit ^ functionUnit, 
      int sourceLineNumber
   );

   // Adds the given identifiers to the given formal parameter list.

   static void
   AddFormalParameters
   (
      IdentifierListNode ^ identifierListNode,
      IdentifierNode ^ identifierNode,
      FormalParameterType parameterType,
      List<FormalParameter ^> ^ formalParameters,
      int sourceLineNumber
   );

   // Adds the given identifiers as procedure parameters to the given 
   // formal parameter list.

   static void
   AddFormalParameter
   (   
      IdentifierNode ^ identifierNode,
      FormalParameterListNode ^ formalParameterListNode,
      FormalParameterType parameterType,
      Phx::Types::Type ^ resultType,
      List<FormalParameter ^> ^ formalParameters,
      int sourceLineNumber
   );

   // Creates an array of FormalParameter objects from the given 
   // FormalParameterListNode object.

   static array<FormalParameter ^> ^ 
   BuildFormalParameterList
   (
      FormalParameterListNode ^ node
   );

   // Builds a procedure/function forward declaration.

   void
   BuildProcedureOrFunctionDirective
   (
      IdentifierNode ^ identifierNode,
      FormalParameterListNode ^ formalParameterListNode,
      Phx::Types::Type ^ resultType,
      DirectiveNode ^ directiveNode,
      int sourceLineNumber
   );

   // Builds a procedure/function block.

   void
   BuildProcedureOrFunctionBlock
   (
      IdentifierNode ^ identifierNode,
      FormalParameterListNode ^ formalParameterListNode,
      Phx::Types::Type ^ resultType,
      BlockNode ^ blockNode,
      int sourceLineNumber
   );

private: // members     
   
   // The module unit associated with the current source file.

   Phx::ModuleUnit ^ moduleUnit;
  
   // To support nested procedures, this tracks symbols defined
   // in another function unit. 
   // Because the resolved operand will likely be non-symbolic,
   // tracking the original symbol allows to perform
   // checking for issues such as 'assignment to const'.

   Phx::Symbols::Symbol ^ nonLocalSymbol;

   // Counts the depth of field accesses in the current expression.

   int fieldDesignatorCount;

   // Tracks the field identifiers within a with statement.

   List<VariableOrFieldAccess ^> ^ withAccessNodes;

   // The file name of the current source file.

   String ^ sourceFileName;

   // Bit offset for generating unions (variant records).
   // We use a pointer because we want the null value
   // to mean "don't use a union bit offset".

   int * unionBitOffset;

   // Map of forward-delcared pointer types. 

   Dictionary<String ^, String ^> ^ pointerTypeFixups;
};

} // namespace Pascal