//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the IRBuilder class.
//
//-----------------------------------------------------------------------------

#pragma once

#include "ArgumentModifier.h"

using namespace System::IO;
using namespace System::Collections::Generic;

namespace Pascal 
{

//-----------------------------------------------------------------------------
//
// Description: Helper class for generating Phoenix 
//              Intermediate Representation (IR).
//
// Remarks:
//              This class serves as a central location for generating many
//              common IR constructs, such as variable assignment. 
//              It is not a requirement for all IR generation to be performed
//              through this class. Custom IR generation is generally done
//              either when the operation is fairly self-constrained or 
//              when the operation requires specific attention.
//
//-----------------------------------------------------------------------------

ref class IRBuilder sealed
{
public:  // methods

   // Pushes a new argument list onto the argument list stack.
   // A stack of argument lists is required because an argument
   // itself may invoke a function or procedure call.

   static List<Phx::IR::Operand ^> ^
   PushArgumentList();

   // Pops the topmost argument list from the argument list stack.

   static void
   PopArgumentList();

   // Finds the argument modifier matching the given base operand.

   static ArgumentModifier ^ 
   FindArgumentModifier
   (
      Phx::IR::Operand ^ baseOperand
   );

   // Appends a unary operation to the end of the IR stream.
   // The op parameter may be one of PLUS, MINUS, or NOT, as
   // defined in YaccDeclarations.h.

   static Phx::IR::Operand ^ 
   EmitUnaryOp
   (
      Phx::FunctionUnit ^ functionUnit,
      unsigned op, 
      Phx::Types::Type ^ type, 
      Phx::IR::Operand ^ sourceOperand,
      int sourceLineNumber
   );

   // Appends a binary arithmetic operation to the end of the IR stream.
   // This method supports set operations (union (PLUS), 
   // difference (MINUS), and intersection (STAR)) as well 
   // as the following operators defined in YaccDeclarations.h:
   // PLUS, MINUS, STAR, SLASH, DIV, MOD, AND, OR

   static Phx::IR::Operand ^ 
   EmitBinaryArithmeticOp
   (
      Phx::FunctionUnit ^ functionUnit,
      unsigned op, 
      Phx::Types::Type ^ type, 
      Phx::IR::Operand ^ sourceOperand1, 
      Phx::IR::Operand ^ sourceOperand2,
      int sourceLineNumber
   );

   // Appends a binary relational operation to the end of the IR stream.
   // This method supports the set operations set membership (IN), 
   // set equlity (EQUAL/NOTEQUAL), subset of (LE), and superset of (GE).
   // This method also supports compare operations on strings, as well 
   // as the following operators defined in YaccDeclarations.h:
   // EQUAL, NOTEQUAL, LT, GT, LE, GE

   static Phx::IR::Operand ^ 
   EmitBinaryRelationalOp
   (
      Phx::FunctionUnit ^ functionUnit,
      unsigned op,    
      Phx::IR::Operand ^ sourceOperand1, 
      Phx::IR::Operand ^ sourceOperand2,      
      int sourceLineNumber
   );

   // Generates a conditional branch statement and appends it
   // to the end of the IR stream.

   static void
   EmitConditionalBranch
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::IR::Operand ^ conditionOperand,
      Phx::IR::LabelInstruction ^ trueLabelInstruction,
      Phx::IR::LabelInstruction ^ falseLabelInstruction
   );

   // Appends the given label instruction to the end of the IR stream.

   static void 
   EmitLabel
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::IR::LabelInstruction ^ labelInstr
   );

   // Creates an unconditional branch instruction to the given label target
   // and appends it to the end of the IR stream.

   static Phx::IR::BranchInstruction ^ 
   EmitGoto
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::IR::LabelInstruction ^ labelInstruction,
      int sourceLineNumber
   );

   // Creates an unconditional branch instruction to the given label target
   // and appends it after the given instruction.

   static Phx::IR::BranchInstruction ^ 
   EmitGoto
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::IR::Instruction ^ insertAfterInstruction,
      Phx::IR::LabelInstruction ^ labelInstruction,
      int sourceLineNumber
   );

   // Creates an assignment instruction between the given source and 
   // destination operands and appends it to the end of the IR stream.

   static Phx::IR::Operand ^
   EmitAssign
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::IR::Operand ^ destinationOperand,    
      Phx::IR::Operand ^ sourceOperand,
      int sourceLineNumber
   );

   // Appends the given switch instruction to the end of the IR stream.

   static Phx::IR::Operand ^
   EmitSwitch
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::IR::SwitchInstruction ^ switchInstructon,
      int sourceLineNumber
   );

   // Validates the argument number and argument types 
   // for the given function type.

   static bool 
   ValidateCall
   (
      String ^ functionName,
      Phx::Types::FunctionType ^ functionType, 
      List<Phx::IR::Operand ^> ^ argumentList,
      int sourceLineNumber
   );

   // Generates a call to the given procedure or function name
   // and appends it to the end of the IR stream.

   static Phx::IR::Instruction ^ 
   EmitCall
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ procedureName,
      List<Phx::IR::Operand ^> ^ argumentList,
      int sourceLineNumber
   );

   // Creates a new variable operand from the provided symbol.

   static Phx::IR::VariableOperand ^
   CreateVariableOperand
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Symbols::Symbol ^ symbol
   );
    
   // Promotes the provided operand to an operand of
   // the given enum type.

   static Phx::IR::Operand ^ 
   PromoteToEnum
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Types::Type ^ enumType,
      Phx::IR::Operand ^ operand,
      int sourceLineNumber
   );

   // Promotes the provided operand to an operand of
   // the Boolean enum type.

   static Phx::IR::Operand ^ 
   PromoteToBoolean
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::IR::Operand ^ operand,
      int sourceLineNumber
   );

   // Extracts the primary field from the given operand.

   static Phx::IR::Operand ^ 
   ExtractPrimaryEnumField
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::IR::Operand ^ enumOperand
   );

   // Converts an array of character type to a character pointer type.

   static Phx::IR::MemoryOperand ^
   ConvertStringToPointer
   (
      Phx::FunctionUnit ^ functionUnit, 
      Phx::IR::Operand ^ argument
   );

public:  // properties

   // Retrieves the topmost argument list on the argument list stack.

   static property List<Phx::IR::Operand ^> ^ ArgumentList
   {
      List<Phx::IR::Operand ^> ^ get();
   }

   // Retrieves the current argument modifier list.

   static property List<ArgumentModifier ^> ^ ArgumentModifierList
   {
      List<ArgumentModifier ^> ^ get();
   }

private: // methods

   // Converts the given operand to the provided new type.

   static Phx::IR::Operand ^
   ConvertOperandType
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::IR::Operand ^ operand,
      Phx::Types::Type ^ newType
   );

private: // members 

   static List<List<Phx::IR::Operand ^> ^> ^ argumentLists = 
      gcnew List<List<Phx::IR::Operand ^> ^>();

   static List<ArgumentModifier ^> ^ argumentModifierList = 
      gcnew List<ArgumentModifier ^>();

   static Phx::IR::Operand ^ currentOperand = nullptr;
};

} // namespace Pascal