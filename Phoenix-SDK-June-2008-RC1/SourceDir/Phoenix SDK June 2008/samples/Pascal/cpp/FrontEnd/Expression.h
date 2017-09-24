//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//     Definition of the IExpression interface.
//
//-----------------------------------------------------------------------------

using namespace System;

namespace Ast
{

//-----------------------------------------------------------------------------
//
// Description:
// 
//    Interface implemented by AST node classes that have type properties.
//
// Remarks:
//
//    IExpression is implemented by subclasses of Ast.Node.
//    This interface is used to provide expressions with a data type.
//
//-----------------------------------------------------------------------------

interface class IExpression
{
   // Retrieves the Type associated with this expression.

   property Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   // Performs type checking on this expression.

   bool TypeCheck();
};

}  // namespace Ast