//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Implementation of the AstVisitorImpl class.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "AstVisitorImpl.h"
#include "Ast.h"

using namespace System::Reflection;

namespace Pascal 
{

//-----------------------------------------------------------------------------
//
// Description:
//
//    Visits the child nodes of a given AST node.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
AstVisitorImpl::VisitChildren
(
   Node^ node
)
{
   // Lazy create the delegate map on first use.

   if (this->visitChildrenType == nullptr)
   {
      this->visitChildrenType = gcnew array<System::Type^>(4);
      this->visitChildrenCall = gcnew array<VisitChildrenDelegate^>(4);
      
      // Map each direct subclass of Node to its corresponding 
      // visit method.

      Assembly^ assembly = Assembly::GetExecutingAssembly();

      this->visitChildrenType[3] = assembly->GetType("Ast.UnaryNode");
      this->visitChildrenCall[3] = gcnew VisitChildrenDelegate(
         this, &AstVisitorImpl::VisitUnaryNode);
      this->visitChildrenType[2] = assembly->GetType("Ast.BinaryNode");
      this->visitChildrenCall[2] = gcnew VisitChildrenDelegate(
         this, &AstVisitorImpl::VisitBinaryNode);
      this->visitChildrenType[1] = assembly->GetType("Ast.TernaryNode");
      this->visitChildrenCall[1] = gcnew VisitChildrenDelegate(
         this, &AstVisitorImpl::VisitTernaryNode);
      this->visitChildrenType[0] = assembly->GetType("Ast.PolyadicNode");
      this->visitChildrenCall[0] = gcnew VisitChildrenDelegate(
         this, &AstVisitorImpl::VisitPolyadicNode);
   }
  
   System::Type ^ nodeType = node->GetType();

   // Call the approprite visit method.

   for (int i = 0; i < this->visitChildrenType->Length; ++i)
   {
      if (nodeType->IsSubclassOf(this->visitChildrenType[i]))
      {
         this->visitChildrenCall[i](node);
         break;
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Visits an AST node with one child.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void 
AstVisitorImpl::VisitUnaryNode
(
   Node^ node
)
{
   UnaryNode ^ unaryNode = safe_cast<UnaryNode ^>(node);
   if (unaryNode->First)
      unaryNode->First->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//     Visits an AST node with two children.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void 
AstVisitorImpl::VisitBinaryNode
(
   Node^ node
)
{
   BinaryNode ^ binaryNode = safe_cast<BinaryNode ^>(node);
   if (binaryNode->First)
      binaryNode->First->Accept(this);
   if (binaryNode->Second)
      binaryNode->Second->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Visits an AST node with three children.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void 
AstVisitorImpl::VisitTernaryNode
(
   Node^ node
)
{
   TernaryNode ^ ternaryNode = safe_cast<TernaryNode ^>(node);
   if (ternaryNode->First)
      ternaryNode->First->Accept(this);
   if (ternaryNode->Second)
      ternaryNode->Second->Accept(this);
   if (ternaryNode->Third)
      ternaryNode->Third->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Visits an AST node with many children.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void 
AstVisitorImpl::VisitPolyadicNode
(
   Node^ node
)
{
   PolyadicNode ^ polyadicNode = safe_cast<PolyadicNode ^>(node);
   for each (Node ^ node in polyadicNode->ChildList)
   {
      if (node) node->Accept(this);
   }
}

} // namespace Pascal 