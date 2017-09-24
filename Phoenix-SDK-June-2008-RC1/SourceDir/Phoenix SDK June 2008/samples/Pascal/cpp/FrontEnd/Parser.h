//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the ParserGCRoots class.
//
//-----------------------------------------------------------------------------


#pragma once

namespace Ast
{
ref class Node;
}
ref class Ast::Node ^ Parse();

using namespace System;
using namespace System::Collections::Generic;
using namespace Ast;

//-----------------------------------------------------------------------------
//
// Description:
//
//    Declarations for interfacing the yacc parser with its caller.
//
// Remarks: 
//
//    
//-----------------------------------------------------------------------------

ref struct ParserGCRoots
{
public:

   // Root of the tree built by the parser.

   static ref class Node ^ syntaxTree = nullptr;

   // Indirection to reference managed AST nodes from yacc's native data types.

   static List<ref class Node ^>^ astNodes = nullptr;

   // Indirection to reference AST node lists from yacc's native data types.

   static List< List<ref class Node^>^ >^ astNodeLists = nullptr;

   // Pre-processing step to yyparse().

   static void Initialize()
   {
      astNodes = gcnew List<ref class Node ^>();
      astNodeLists = gcnew List< List<ref class Node^>^ >();
   }
   
   // Post-processing step to yyparse().
   
   static void Cleanup()
   {
      astNodes = nullptr;
      astNodeLists = nullptr;
   }
};