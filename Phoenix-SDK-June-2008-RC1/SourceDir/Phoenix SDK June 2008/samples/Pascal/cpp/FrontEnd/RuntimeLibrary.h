//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the IRuntimeLibrary interface.
//
//-----------------------------------------------------------------------------

#pragma once

using namespace System;
using namespace System::Collections::Generic;

namespace Pascal
{

//-----------------------------------------------------------------------------
//
// Description: Provides an interface to initializing and calling into the
//              runtime library that is associated with a Pascal program.
//
// Remarks:
//
//
//-----------------------------------------------------------------------------

interface class IRuntimeLibrary
{
public:

   // Creates a new IRuntimeLibrary object that can communicate with
   // the native runtime library.

   static IRuntimeLibrary ^
   NewNativeRuntime
   (
      Phx::ModuleUnit ^ module
   );
   
   // Determines whether the given string is the name of a runtime 
   // function.

   bool
   IsRuntimeFunctionName
   (
      String ^ name
   );

   // Constructs a user call to a runtime library function.

   Phx::IR::Instruction ^
   BuildRuntimeFunctionCall
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Builds the entry-point function into the program.

   Phx::FunctionUnit ^
   BuildEntryFunction
   (
      Phx::ModuleUnit ^ moduleUnit,
      int sourceLineNumber
   );

   // Generates a call to the runtime range check function.

   Phx::IR::Instruction ^
   RuntimeCheckRange
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::IR::Operand ^ lowerBoundOperand,
      Phx::IR::Operand ^ upperBoundOperand,
      Phx::IR::Operand ^ valueOperand,
      Phx::IR::Operand ^ sourceLineNumberOperand,
      int sourceLineNumber
   );

   // Generates a call to the given 'set' function with the 
   // provided arguments.

   Phx::IR::Instruction ^
   CallSetFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Generates a call to the given 'string' function with the 
   // provided arguments.

   Phx::IR::Instruction ^
   CallStringFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Generates a call to the given 'file' function with the 
   // provided arguments.

   Phx::IR::Instruction ^
   CallFileFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Generates a call to the given 'allocation' function with the 
   // provided arguments.

   Phx::IR::Instruction ^
   CallAllocationFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Generates a call to the given 'math' function with the    
   // provided arguments.

   Phx::IR::Instruction ^
   CallMathFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Generates a call to the given 'transfer' function with the 
   // provided arguments.

   Phx::IR::Instruction ^
   CallTransferFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Generates a call to the given 'utility' function with the 
   // provided arguments.

   Phx::IR::Instruction ^
   CallUtilityFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      Phx::IR::Instruction ^ insertAfterInstruction,
      int sourceLineNumber
   );

   // Generates a call to the given 'display' function with the 
   // provided arguments.

   Phx::IR::Instruction ^
   CallDisplayFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      Phx::IR::Instruction ^ insertAfterInstruction,
      int sourceLineNumber
   );
   
   // Retrieves the return type for the runtime function that matches
   // the given signature (name and argument lits).

   Phx::Types::Type ^ 
   GetFunctionReturnType
   (
      String ^ functionName,
      List<Phx::IR::Operand ^> ^ arguments
   );

   // Determines whether the given symbol refers to the built-in 
   // 'input' or 'output' file symbols.

   bool
   IsRuntimeFileSymbol
   (
      Phx::Symbols::Symbol ^ fileSymbol
   );
};

}  // namespace Pascal