//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Implementation of the VisitTracker class.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

using namespace System;

#include "AstClasses.h"
#include "Ast.h"
#include "VisitTracker.h"
#include "YaccDeclarations.h"

namespace Pascal 
{
//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given Node object
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
VisitTracker::Visit
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
//    Tracks usage of the given UnaryNode object
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
VisitTracker::Visit
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
//    Tracks usage of the given BinaryNode object
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
VisitTracker::Visit
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
//    Tracks usage of the given TernaryNode object
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
VisitTracker::Visit
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
//    Tracks usage of the given PolyadicNode object
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
VisitTracker::Visit
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
//    Tracks usage of the given ActualParameterListNode object
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
VisitTracker::Visit
(
   ActualParameterListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ActualParameterNode object
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
VisitTracker::Visit
(
   ActualParameterNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ArrayTypeNode object
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
VisitTracker::Visit
(
   ArrayTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given AssignmentStatementNode object
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
VisitTracker::Visit
(
   AssignmentStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given BaseTypeNode object
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
VisitTracker::Visit
(
   BaseTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given BlockNode object
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
VisitTracker::Visit
(
   BlockNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given BooleanConstantNode object
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
VisitTracker::Visit
(
   BooleanConstantNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given BooleanExpressionNode object
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
VisitTracker::Visit
(
   BooleanExpressionNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given CaseConstantListNode object
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
VisitTracker::Visit
(
   CaseConstantListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given CaseConstantNode object
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
VisitTracker::Visit
(
   CaseConstantNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given CaseIndexNode object
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
VisitTracker::Visit
(
   CaseIndexNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given CaseListElementListNode object
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
VisitTracker::Visit
(
   CaseListElementListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given CaseListElementNode object
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
VisitTracker::Visit
(
   CaseListElementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given CaseStatementNode object
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
VisitTracker::Visit
(
   CaseStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given CharacterStringNode object
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
VisitTracker::Visit
(
   CharacterStringNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ClosedForStatementNode object
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
VisitTracker::Visit
(
   ClosedForStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ClosedIfStatementNode object
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
VisitTracker::Visit
(
   ClosedIfStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node); 
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ClosedStatementNode object
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
VisitTracker::Visit
(
   ClosedStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ClosedWhileStatementNode object
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
VisitTracker::Visit
(
   ClosedWhileStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ClosedWithStatementNode object
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
VisitTracker::Visit
(
   ClosedWithStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ComponentTypeNode object
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
VisitTracker::Visit
(
   ComponentTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given CompoundStatementNode object
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
VisitTracker::Visit
(
   CompoundStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ConstantDefinitionNode object
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
VisitTracker::Visit
(
   ConstantDefinitionNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ConstantDefinitionPartNode object
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
VisitTracker::Visit
(
   ConstantDefinitionPartNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ConstantExponentiationNode object
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
VisitTracker::Visit
(
   ConstantExponentiationNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ConstantExpressionNode object
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
VisitTracker::Visit
(
   ConstantExpressionNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ConstantFactorNode object
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
VisitTracker::Visit
(
   ConstantFactorNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ConstantListNode object
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
VisitTracker::Visit
(
   ConstantListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ConstantNode object
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
VisitTracker::Visit
(
   ConstantNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ConstantPrimaryNode object
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
VisitTracker::Visit
(
   ConstantPrimaryNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ConstantSimpleExpressionNode object
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
VisitTracker::Visit
(
   ConstantSimpleExpressionNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ConstantTermNode object
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
VisitTracker::Visit
(
   ConstantTermNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ControlVariableNode object
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
VisitTracker::Visit
(
   ControlVariableNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given DirectionNode object
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
VisitTracker::Visit
(
   DirectionNode^ node
)
{   
	ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given DirectiveNode object
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
VisitTracker::Visit
(
   DirectiveNode^ node
)
{   
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given DomainTypeNode object
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
VisitTracker::Visit
(
   DomainTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given EnumeratedTypeNode object
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
VisitTracker::Visit
(
   EnumeratedTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ExponentiationBaseNode object
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
VisitTracker::Visit
(
   ExponentiationBaseNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ExponentiationNode object
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
VisitTracker::Visit
(
   ExponentiationNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ExpressionBaseNode object
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
VisitTracker::Visit
(
   ExpressionBaseNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ExpressionNode object
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
VisitTracker::Visit
(
   ExpressionNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FactorBaseNode object
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
VisitTracker::Visit
(
   FactorBaseNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FactorNode object
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
VisitTracker::Visit
(
   FactorNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FieldDesignatorNode object
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
VisitTracker::Visit
(
   FieldDesignatorNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FileNode object
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
VisitTracker::Visit
(
   FileNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FileTypeNode object
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
VisitTracker::Visit
(
   FileTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FinalValueNode object
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
VisitTracker::Visit
(
   FinalValueNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FormalParameterListNode object
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
VisitTracker::Visit
(
   FormalParameterListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FormalParameterNode object
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
VisitTracker::Visit
(
   FormalParameterNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FormalParameterSectionListNode object
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
VisitTracker::Visit
(
   FormalParameterSectionListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FormalParameterSectionNode object
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
VisitTracker::Visit
(
   FormalParameterSectionNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ForStatementNode object
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
VisitTracker::Visit
(
   ForStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FunctionalParameterSpecificationNode object
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
VisitTracker::Visit
(
   FunctionalParameterSpecificationNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FunctionBlockNode object
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
VisitTracker::Visit
(
   FunctionBlockNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FunctionDeclarationNode object
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
VisitTracker::Visit
(
   FunctionDeclarationNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FunctionDesignatorNode object
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
VisitTracker::Visit
(
   FunctionDesignatorNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FunctionHeadingNode object
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
VisitTracker::Visit
(
   FunctionHeadingNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given FunctionIdentificationNode object
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
VisitTracker::Visit
(
   FunctionIdentificationNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given GotoStatementNode object
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
VisitTracker::Visit
(
   GotoStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);  
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given IdentifierListNode object
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
VisitTracker::Visit
(
   IdentifierListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given IdentifierNode object
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
VisitTracker::Visit
(
   IdentifierNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given IfStatementNode object
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
VisitTracker::Visit
(
   IfStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given IndexedVariableNode object
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
VisitTracker::Visit
(
   IndexedVariableNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given IndexExpressionListNode object
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
VisitTracker::Visit
(
   IndexExpressionListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given IndexExpressionNode object
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
VisitTracker::Visit
(
   IndexExpressionNode^ node
)
{      
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given IndexListNode object
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
VisitTracker::Visit
(
   IndexListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given IndexTypeNode object
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
VisitTracker::Visit
(
   IndexTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given InitialValueNode object
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
VisitTracker::Visit
(
   InitialValueNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given LabelDeclarationPartNode object
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
VisitTracker::Visit
(
   LabelDeclarationPartNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given LabelListNode object
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
VisitTracker::Visit
(
   LabelListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given LabelNode object
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
VisitTracker::Visit
(
   LabelNode^ node
)
{   
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given MemberDesignatorListNode object
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
VisitTracker::Visit
(
   MemberDesignatorListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given MemberDesignatorNode object
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
VisitTracker::Visit
(
   MemberDesignatorNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ModuleNode object
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
VisitTracker::Visit
(
   ModuleNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given NewOrdinalTypeNode object
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
VisitTracker::Visit
(
   NewOrdinalTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given NewPointerTypeNode object
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
VisitTracker::Visit
(
   NewPointerTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given NewStructuredTypeNode object
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
VisitTracker::Visit
(
   NewStructuredTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given NewTypeNode object
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
VisitTracker::Visit
(
   NewTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given NilNode object
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
VisitTracker::Visit
(
   NilNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given NonLabeledClosedStatementNode object
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
VisitTracker::Visit
(
   NonLabeledClosedStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given NonLabeledOpenStatementNode object
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
VisitTracker::Visit
(
   NonLabeledOpenStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given NonStringNode object
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
VisitTracker::Visit
(
   NonStringNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given OpenForStatementNode object
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
VisitTracker::Visit
(
   OpenForStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given OpenIfStatementNode object
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
VisitTracker::Visit
(
   OpenIfStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);  
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given OpenStatementNode object
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
VisitTracker::Visit
(
   OpenStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given OpenWhileStatementNode object
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
VisitTracker::Visit
(
   OpenWhileStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given OpenWithStatementNode object
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
VisitTracker::Visit
(
   OpenWithStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given OrdinalTypeNode object
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
VisitTracker::Visit
(
   OrdinalTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given OtherwisePartNode object
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
VisitTracker::Visit
(
   OtherwisePartNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ParamsNode object
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
VisitTracker::Visit
(
   ParamsNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given PrimaryBaseNode object
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
VisitTracker::Visit
(
   PrimaryBaseNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given PrimaryNode object
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
VisitTracker::Visit
(
   PrimaryNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ProceduralParameterSpecificationNode object
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
VisitTracker::Visit
(
   ProceduralParameterSpecificationNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ProcedureAndFunctionDeclarationPartNode object
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
VisitTracker::Visit
(
   ProcedureAndFunctionDeclarationPartNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ProcedureBlockNode object
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
VisitTracker::Visit
(
   ProcedureBlockNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);  
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ProcedureDeclarationNode object
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
VisitTracker::Visit
(
   ProcedureDeclarationNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ProcedureHeadingNode object
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
VisitTracker::Visit
(
   ProcedureHeadingNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ProcedureIdentificationNode object
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
VisitTracker::Visit
(
   ProcedureIdentificationNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ProcedureOrFunctionDeclarationListNode object
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
VisitTracker::Visit
(
   ProcedureOrFunctionDeclarationListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ProcedureOrFunctionDeclarationNode object
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
VisitTracker::Visit
(
   ProcedureOrFunctionDeclarationNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ProcedureStatementNode object
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
VisitTracker::Visit
(
   ProcedureStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ProgramHeadingNode object
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
VisitTracker::Visit
(
   ProgramHeadingNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ProgramNode object
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
VisitTracker::Visit
(
   ProgramNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given RecordSectionListNode object
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
VisitTracker::Visit
(
   RecordSectionListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given RecordSectionNode object
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
VisitTracker::Visit
(
   RecordSectionNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given RecordTypeNode object
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
VisitTracker::Visit
(
   RecordTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given RecordVariableListNode object
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
VisitTracker::Visit
(
   RecordVariableListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given RepeatStatementNode object
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
VisitTracker::Visit
(
   RepeatStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ResultTypeNode object
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
VisitTracker::Visit
(
   ResultTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given SetConstructorNode object
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
VisitTracker::Visit
(
   SetConstructorNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given SetTypeNode object
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
VisitTracker::Visit
(
   SetTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given SimpleExpressionBaseNode object
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
VisitTracker::Visit
(
   SimpleExpressionBaseNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given SimpleExpressionNode object
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
VisitTracker::Visit
(
   SimpleExpressionNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given StatementNode object
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
VisitTracker::Visit
(
   StatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given StatementPartNode object
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
VisitTracker::Visit
(
   StatementPartNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given StatementSequenceNode object
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
VisitTracker::Visit
(
   StatementSequenceNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node); 
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given StringNode object
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
VisitTracker::Visit
(
   StringNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given StructuredTypeNode object
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
VisitTracker::Visit
(
   StructuredTypeNode^ node
)
{	
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given SubrangeTypeNode object
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
VisitTracker::Visit
(
   SubrangeTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given TagFieldNode object
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
VisitTracker::Visit
(
   TagFieldNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given TagTypeNode object
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
VisitTracker::Visit
(
   TagTypeNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given TermBaseNode object
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
VisitTracker::Visit
(
   TermBaseNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given TermNode object
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
VisitTracker::Visit
(
   TermNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given TypeDefinitionListNode object
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
VisitTracker::Visit
(
   TypeDefinitionListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given TypeDefinitionNode object
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
VisitTracker::Visit
(
   TypeDefinitionNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given TypeDefinitionPartNode object
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
VisitTracker::Visit
(
   TypeDefinitionPartNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given TypeDenoterNode object
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
VisitTracker::Visit
(
   TypeDenoterNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given UnsignedConstantNode object
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
VisitTracker::Visit
(
   UnsignedConstantNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given UnsignedIntegerNode object
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
VisitTracker::Visit
(
   UnsignedIntegerNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given UnsignedNumberNode object
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
VisitTracker::Visit
(
   UnsignedNumberNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given UnsignedRealNode object
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
VisitTracker::Visit
(
   UnsignedRealNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given ValueParameterSpecificationNode object
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
VisitTracker::Visit
(
   ValueParameterSpecificationNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given VariableAccessNode object
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
VisitTracker::Visit
(
   VariableAccessNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given VariableDeclarationListNode object
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
VisitTracker::Visit
(
   VariableDeclarationListNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given VariableDeclarationNode object
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
VisitTracker::Visit
(
   VariableDeclarationNode^ node
)
{   
   ReportVisit(node->GetType());
   VisitChildren(node); 
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given VariableDeclarationPartNode object
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
VisitTracker::Visit
(
   VariableDeclarationPartNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given VariableParameterSpecificationNode object
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
VisitTracker::Visit
(
   VariableParameterSpecificationNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given VariantListNode object
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
VisitTracker::Visit
(
   VariantListNode^ node
)
{	
	ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given VariantNode object
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
VisitTracker::Visit
(
   VariantNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node); 
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given VariantPartNode object
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
VisitTracker::Visit
(
   VariantPartNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given VariantSelectorNode object
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
VisitTracker::Visit
(
   VariantSelectorNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given WhileStatementNode object
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
VisitTracker::Visit
(
   WhileStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Tracks usage of the given WithStatementNode object
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
VisitTracker::Visit
(
   WithStatementNode^ node
)
{
   ReportVisit(node->GetType());
   VisitChildren(node);
}

ref class TypeComparer: IComparer<Type^>
{
public:
   virtual int Compare(Type ^ x, Type ^ y)
   {
      return x->FullName->CompareTo(y->FullName);
   }
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    Prints statistics to the provided output stream.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void VisitTracker::PrintUsage(TextWriter^ out)
{
   // Perform a final fixup of the reporting data.

   FixupUsage();

   // Calculate the longest type name we visited.

   int longest = 0;
   for each (System::Type^ type in this->visitCount->Keys)
   {
      if (type->Name->Length > longest)
         longest = type->Name->Length;
   }
   
   // Create sorted array of keys.

   array<System::Type^>^ keys = 
      gcnew array<System::Type^>(this->visitCount->Keys->Count);
   int n = 0;
   for each (System::Type^ type in this->visitCount->Keys)
   {
      keys[n++] = type;
   }      
   Array::Sort(keys, gcnew TypeComparer());

   // Print the visit count of each type to the output stream.

   // count of keys whose value is greater than 0.
   int usedCount = 0;
   // count keys that were excluded from the report.
   int excludeCount = 0;
   array<String^>^ excludeTypeList = ReportExcludeList;
   for each (System::Type^ type in keys)
   {
      if (ExcludeTypeFromReport(excludeTypeList, type))
      {
         excludeCount++;
         continue;
      }

      // Format type name.
      System::Text::StringBuilder^ builder = 
         gcnew System::Text::StringBuilder(type->Name);
      
      // Append whitespace.
      while (builder->Length < longest)
         builder->Append(Char(' '));

      // Print the result to the output stream.
      int visitCount = this->visitCount[type];
      if (visitCount > 0)
         usedCount++;
      out->WriteLine("{0}: {1}", builder->ToString(), visitCount);
   }

   // Write total coverage percentage to the output stream.
   int totalCount = keys->Length-excludeCount;
   out->WriteLine("***Coverage = {0}/{1} ({2}%)", usedCount, totalCount,
      100 * usedCount / totalCount);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Prepares the usage report for printing.
//
// Remarks:
//
//    For each type we visited, ensure that any base classes are reported as
//    well. For example, if we visited an instance of OpenWhileStatementNode,
//    ensure we report an instance of WhileStatementNode as well.
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void 
VisitTracker::FixupUsage()
{
   Assembly^ assembly = Assembly::GetExecutingAssembly();

   System::Type^ nodeType = assembly->GetType("Ast.Node");

   Dictionary<System::Type^, int>^ temp = 
      gcnew Dictionary<System::Type^, int>();
   for each (System::Type^ type in this->visitCount->Keys)
   {
      temp->Add(type, this->visitCount[type]);
   }

   for each (System::Type^ type in temp->Keys)
   {
      Type^ t = type;
      while (!t->BaseType->Equals(nodeType))
      {
         ReportVisit(t->BaseType);
         t = t->BaseType;
      }
   }  
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given type is included in the provided
//    type name list.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool 
VisitTracker::ExcludeTypeFromReport
(
   array<String^>^ excludeList, 
   System::Type^ type
)
{      
   for each (String^ excludeName in excludeList)
   {
      if (type->Name->Equals(excludeName))
         return true;
   }
   return false;
}

} // namespace Pascal