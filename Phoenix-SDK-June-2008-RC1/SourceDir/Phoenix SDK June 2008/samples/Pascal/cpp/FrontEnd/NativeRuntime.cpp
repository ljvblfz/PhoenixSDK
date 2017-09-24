//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Implementation of the NativeRuntime class.
//
// Remarks:
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "YaccDeclarations.h"

using namespace System;
using namespace System::Diagnostics;

namespace Pascal 
{

//-----------------------------------------------------------------------------
//
// Description: Concrete implementation of the IRuntimeLibrary interface.
//              This implementation allows us to interact with the runtime 
//              library for native Pascal applications.
//
// Remarks:
//
//    We break the runtime functions into separate 'families' because
//    many different runtime functions require special processing.
//    For example, the built-in 'put' and 'get' functions must be translated
//    to specific runtime functions, depending on the argument list.
//    Another example is the 'pred' and 'succ' functions. The return type
//    of these functions are dependent on the argument type provided.
//
//-----------------------------------------------------------------------------

ref class NativeRuntime : public IRuntimeLibrary
{
public:

   // Constructs a new NativeRuntime object.

   NativeRuntime
   (
      Phx::ModuleUnit ^ module
   )  : module(module)
   {
      ModuleBuilder::Runtime = this;
      
      // Build the 'built-in' function table.

      BuildIntrinsicFunctionTable();

      // Build the function types that are part of the
      // native runtime library.

      BuildSetFunctionSymbols();
      BuildFileFunctionSymbols();
      BuildStringFunctionSymbols();
      BuildAllocationFunctionSymbols();
      BuildMathFunctionSymbols();
      BuildTransferFunctionSymbols();
      BuildUtilityFunctionSymbols();
      BuildDisplayFunctionSymbols();
   }
   
   // Determines whether the given string is the name of a runtime 
   // function.

   virtual bool
   IsRuntimeFunctionName
   (
      String ^ name
   );

   // Constructs a user call to a runtime library function.

   virtual Phx::IR::Instruction ^
   BuildRuntimeFunctionCall
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Builds the entry-point function into the program.

   virtual Phx::FunctionUnit ^
   BuildEntryFunction
   (
      Phx::ModuleUnit ^ moduleUnit,
      int sourceLineNumber
   );

   // Generates a call to the runtime range check function.

   virtual Phx::IR::Instruction ^
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

   virtual Phx::IR::Instruction ^
   CallSetFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Generates a call to the given 'string' function with the 
   // provided arguments.

   virtual Phx::IR::Instruction ^
   CallStringFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Generates a call to the given 'file' function with the 
   // provided arguments.

   virtual Phx::IR::Instruction ^
   CallFileFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Generates a call to the given 'allocation' function with the 
   // provided arguments.

   virtual Phx::IR::Instruction ^
   CallAllocationFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Generates a call to the given 'math' function with the    
   // provided arguments.

   virtual Phx::IR::Instruction ^
   CallMathFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Generates a call to the given 'transfer' function with the 
   // provided arguments.

   virtual Phx::IR::Instruction ^
   CallTransferFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^ > ^ arguments,
      int sourceLineNumber
   );

   // Generates a call to the given 'utility' function with the 
   // provided arguments.

   virtual Phx::IR::Instruction ^
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

   virtual Phx::IR::Instruction ^
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

   virtual Phx::Types::Type ^ 
   GetFunctionReturnType
   (
      String ^ functionName,
      List<Phx::IR::Operand ^> ^ arguments
   );

   // Determines whether the given symbol refers to the built-in 
   // 'input' or 'output' file symbols.

   virtual bool
   IsRuntimeFileSymbol
   (
      Phx::Symbols::Symbol ^ fileSymbol
   );
   
private:
  
   // Builds an internal list of all of the built-in Pascal functions
   // and procedures.

   void BuildIntrinsicFunctionTable();

   // Builds initialization data for the main function unit.

   void BuildInitializedData(
      Phx::FunctionUnit ^ mainFunctionUnit,
      int sourceLineNumber
   );

   // Builds the function symbols for the 'set' family of
   // built-in procedures.

   void BuildSetFunctionSymbols();

   // Builds the function symbols for the 'file' family of
   // built-in procedures.

   void BuildFileFunctionSymbols();

   // Builds the function symbols for the 'string' family of
   // built-in procedures.

   void BuildStringFunctionSymbols();

   // Builds the function symbols for the 'allocation' family of
   // built-in procedures.

   void BuildAllocationFunctionSymbols();

   // Builds the function symbols for the 'utility' family of
   // built-in procedures.

   void BuildUtilityFunctionSymbols();

   // Builds the function symbols for the 'display' family of
   // built-in procedures.

   void BuildDisplayFunctionSymbols();

   // Builds the function symbols for the 'math' family of
   // built-in procedures.

   void BuildMathFunctionSymbols();

   // Builds the function symbols for the 'transfer' family of
   // built-in procedures.

   void BuildTransferFunctionSymbols();

   // Initializes the built-in 'input' and 'output' file handles.

   void
   InitializeStandardHandles
   (
      Phx::FunctionUnit ^ functionUnit,
      int sourceLineNumber
   );

   // Generates a call to the standard read procedure.

   Phx::IR::Instruction ^ 
   CallFileRead
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^> ^ arguments,
      int sourceLineNumber
   );

   // Generates a call to the standard write procedure.

   Phx::IR::Instruction ^ 
   CallFileWrite
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ functionName,
      List<Phx::IR::Operand ^> ^ arguments,
      int sourceLineNumber
   );

   // Emits a call to the given function with the provided argument list.

   Phx::IR::Instruction ^
   CallFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Symbols::FunctionSymbol ^ functionSymbol,
      List<Phx::IR::Operand ^ > ^ arguments,
      bool checkArgumentCount,
      Phx::IR::Instruction ^ insertAfterInstruction,
      int sourceLineNumber
   );

   // Determines whether the given function name refers to a 'string' function.

   bool IsStringFunction(String ^ functionName);

   // Determines whether the given function name refers to a 'file' function.

   bool IsFileFunction(String ^ functionName);

   // Translates the given function name to the appropriate runtime library
   // function name.

   String ^ TranslateFileFunctionName(String ^ userName);
   
   // Determines whether the given function name refers to an
   // 'allocation' function.

   bool IsAllocationFunction(String ^ functionName);

   // Determines whether the given function name refers to a
   // 'math' function.

   bool IsMathFunction(String ^ functionName);

   // Translates the given function name to the appropriate runtime library
   // function name.

   String ^ TranslateMathFunctionName(String ^ userName, 
      List<Phx::IR::Operand ^ > ^ arguments);

   // Determines whether the given function name refers to a
   // 'transfer' function.

   bool IsTransferFunction(String ^ functionName);

private:

   // The module unit associated with this instance.

   Phx::ModuleUnit ^ module;

   // Global symbols for the standard 'input' and 'output' 
   // files.

   Phx::Symbols::GlobalVariableSymbol ^ standardInputSymbol;
   Phx::Symbols::GlobalVariableSymbol ^ standardOutputSymbol;
   
   // List of all of the built-in Pascal functions and procedures.

   List<String ^> ^ runtimeFunctionNames;

   // Maps 'set' function names to their function symbols.

   Dictionary<String ^, Phx::Symbols::FunctionSymbol ^> ^ 
      setFunctionSymbols;
   
   // Maps 'file' function names to their function symbols.

   Dictionary<String ^, Phx::Symbols::FunctionSymbol ^> ^ 
      fileFunctionSymbols;
   List<String ^> ^ fileFunctions;

   // Maps 'string' function names to their function symbols.

   Dictionary<String ^, Phx::Symbols::FunctionSymbol ^> ^ 
      stringFunctionSymbols;
   List<String ^> ^ stringFunctions;

   // Maps 'allocation' function names to their function symbols.

   Dictionary<String ^, Phx::Symbols::FunctionSymbol ^> ^ 
      allocFunctionSymbols; 
   List<String ^> ^ allocFunctions;

   // Maps 'utility' function names to their function symbols.

   Dictionary<String ^, Phx::Symbols::FunctionSymbol ^> ^ 
      utilityFunctionSymbols;

   // Maps 'display' function names to their function symbols.

   Dictionary<String ^, Phx::Symbols::FunctionSymbol ^> ^ 
      displayFunctionSymbols;

   // Maps 'math' function names to their function symbols.

   Dictionary<String ^, Phx::Symbols::FunctionSymbol ^> ^ 
      mathFunctionSymbols;
   List<String ^> ^ mathFunctions;

   // Maps 'transfer' function names to their function symbols.

   Dictionary<String ^, Phx::Symbols::FunctionSymbol ^> ^ 
      transferFunctionSymbols;
   List<String ^> ^ transferFunctions;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given string is the name of a runtime 
//    function.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool
NativeRuntime::IsRuntimeFunctionName
(
   String ^ name
)
{
   // Function names are stored in all lower-case. Built-in Pascal identifiers
   // are not case-sensitive.

   return runtimeFunctionNames->Contains(name->ToLower());
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Constructs a user call to a runtime library function.
//
// Remarks:
//
//
// Returns:
//
//    The Instruction object for the function call.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^
NativeRuntime::BuildRuntimeFunctionCall
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ functionName,
   List<Phx::IR::Operand ^ > ^ arguments,
   int sourceLineNumber
)
{
   // Built-in Pascal identifiers are not case-sensitive.

   functionName = functionName->ToLower();

   Debug::Assert(IsRuntimeFunctionName(functionName));

   // Special case for built-in 'write(ln)' and 'read(ln)' procedures.

   if (functionName->StartsWith("write"))
   {
      return CallFileWrite(functionUnit, functionName, 
         arguments, sourceLineNumber);
   }
   else if (functionName->StartsWith("read"))
   {
      return CallFileRead(functionUnit, functionName,
         arguments, sourceLineNumber);
   }

   // Drill-down to specific function type.
   // Each function category may require special processing.

   else if (IsMathFunction(functionName))
   {
      return CallMathFunction(functionUnit, functionName,
         arguments, (int) sourceLineNumber);
   }
   else if (IsTransferFunction(functionName))
   {
      return CallTransferFunction(functionUnit, functionName,
         arguments, (int) sourceLineNumber);
   }
   else if (IsFileFunction(functionName))
   {
      return CallFileFunction(functionUnit, functionName,
         arguments, (int) sourceLineNumber);
   }
   else if (IsStringFunction(functionName))
   {
      return CallStringFunction(functionUnit, functionName,
         arguments, (int) sourceLineNumber);
   }
   else if (IsAllocationFunction(functionName))
   {
      return CallAllocationFunction(functionUnit, functionName,
         arguments, (int) sourceLineNumber);
   }

   Debug::Assert(false);
   return nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds the entry-point function into the program.
//
// Remarks:
//
//    For this implementation, this function builds the C 'main' function.
//
// Returns:
//
//    The FunctionUnit for the entry-point function.
//
//-----------------------------------------------------------------------------

Phx::FunctionUnit ^
NativeRuntime::BuildEntryFunction
(
   Phx::ModuleUnit ^ moduleUnit,
   int sourceLineNumber
)
{
   // Build the function type by using a FunctionTypeBuilder object.

   Phx::Types::FunctionTypeBuilder ^ functionTypeBuilder = 
      Phx::Types::FunctionTypeBuilder::New(moduleUnit->TypeTable);

   functionTypeBuilder->Begin();

   // Now indicate that this function requires the cdecl calling convention.

   functionTypeBuilder->CallingConventionKind = 
      Phx::Types::CallingConventionKind::CDecl;

   // Set the return value type. We choose 'int' to be the
   // register size of the target machine.

   functionTypeBuilder->AppendReturnParameter(
      moduleUnit->RegisterIntType);

   // Set the first argument type (int argc).

   functionTypeBuilder->AppendParameter(
      moduleUnit->RegisterIntType);

   // Set the second argument type (char** argv). 
   // We use 8-bit characters. Also, we choose the native pointer
   // size for the target machine.

   // Build 'char *'

   Phx::Types::PointerType ^ ptrToCharType = Phx::Types::PointerType::New(
      moduleUnit->TypeTable,
      Phx::Types::PointerTypeKind::UnmanagedPointer, 
      moduleUnit->TypeTable->NativePointerBitSize,
      moduleUnit->TypeTable->Character8Type, 
      nullptr
   );

   // Build 'char **'

   Phx::Types::PointerType ^ ptrToPtrToCharType = Phx::Types::PointerType::New(
      moduleUnit->TypeTable,
      Phx::Types::PointerTypeKind::UnmanagedPointer, 
      moduleUnit->TypeTable->NativePointerBitSize,
      ptrToCharType, 
      nullptr
   );

   functionTypeBuilder->AppendParameter(ptrToPtrToCharType);
   
   // Now we're ready to create a symbol to represent main. 
   // First we must give it a linker name. The Visual C++ linker 
   // expects identifiers to be prepended by an underscore character.

   Phx::Symbols::FunctionSymbol ^ mainFunctionSymbol = 
      Phx::Symbols::FunctionSymbol::New(
         moduleUnit->SymbolTable, 
         0, 
         Phx::Name::New(moduleUnit->Lifetime, "_main"), 
         functionTypeBuilder->GetFunctionType(),
         Phx::Symbols::Visibility::GlobalDefinition
      );

   // Create the function unit for the main function.

   Phx::Lifetime ^ lifetime = Phx::Lifetime::New(
      Phx::LifetimeKind::Function, 
      nullptr
   );
   
   Phx::FunctionUnit ^ mainFunctionUnit = Phx::FunctionUnit::New(
      lifetime,
      mainFunctionSymbol, 
      Phx::CodeGenerationMode::Native,
      Phx::GlobalData::TypeTable, 
      moduleUnit->Architecture, 
      moduleUnit->Runtime,
      moduleUnit, 
      ++ModuleBuilder::FunctionCounter
   );
   
   // Attach a symbol table to the function unit.

   Phx::Symbols::Table ^ mainFunctionSymbolTable = 
      Phx::Symbols::Table::New(mainFunctionUnit, 64, true);

   // Make it accessible by symbol name.

   mainFunctionSymbolTable->AddMap(
      Phx::Symbols::NameMap::New(mainFunctionSymbolTable, 64)
   );

   // Associate debug information so we can track source positions.

   Phx::Debug::Info::New(lifetime, mainFunctionUnit);

   mainFunctionUnit->CurrentDebugTag = 
      mainFunctionUnit->DebugInfo->CreateTag(
         Phx::Name::New(lifetime, "_main"), 
         0
      );

   mainFunctionUnit->FirstInstruction->DebugTag = 
      mainFunctionUnit->CurrentDebugTag;

   // Build ENTERFUNCTION.

   Phx::IR::LabelInstruction ^ enterInstruction = 
      Phx::IR::LabelInstruction::New(
         mainFunctionUnit,
         Phx::Common::Opcode::EnterFunction, 
         mainFunctionSymbol
      );

   mainFunctionUnit->FirstInstruction->InsertAfter(enterInstruction);

   // There needs to be a edge from start to enter.

   mainFunctionUnit->FirstInstruction->AppendLabelSource(
	   Phx::IR::LabelOperandKind::Technical, 
	   Phx::IR::LabelOperand::New(mainFunctionUnit, enterInstruction)
	);

   // Create a final exit point for the function. We do this first
   // so any return instructions will have a destination.

   Phx::IR::LabelInstruction ^ exitInstruction = 
      Phx::IR::LabelInstruction::New(
         mainFunctionUnit,
         Phx::Common::Opcode::ExitFunction
      );
   
   Phx::IR::Instruction ^ returnInstruction = 
      Phx::IR::BranchInstruction::NewReturn(
         mainFunctionUnit,
         Phx::Common::Opcode::Return, 
         exitInstruction
      );

   // Return 0 from main.

   returnInstruction->AppendSource(Phx::IR::ImmediateOperand::New(
         mainFunctionUnit,
         moduleUnit->RegisterIntType,
         (int) 0
      )
   );

   // Append the instruction to the IR stream.

   mainFunctionUnit->LastInstruction->InsertBefore(returnInstruction);
   mainFunctionUnit->LastInstruction->InsertBefore(exitInstruction);

   // Update source location on END.

   mainFunctionUnit->LastInstruction->DebugTag =
      mainFunctionUnit->CurrentDebugTag;

   mainFunctionUnit->FinishCreation();

   // Now that we're done with IR for this unit, pop it off the context
   // stack (creating the unit pushed it on).

   mainFunctionUnit->Context->PopUnit();

   // Build initialization data for the main function unit. This
   // includes the values of the built-in Boolean enumeration 
   // and the standard input and output files.

   BuildInitializedData(mainFunctionUnit, sourceLineNumber);

   return mainFunctionUnit;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a call to the runtime range check function.
//
// Remarks:
//
//    Call this method whenever an operand with a subrange type is generated.
//
// Returns:
//
//    The Instruction object for the function call.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^
NativeRuntime::RuntimeCheckRange
(
   Phx::FunctionUnit ^ functionUnit,   
   Phx::IR::Operand ^ lowerBoundOperand,
   Phx::IR::Operand ^ upperBoundOperand,
   Phx::IR::Operand ^ valueOperand,
   Phx::IR::Operand ^ sourceLineNumberOperand,
   int sourceLineNumber
)
{
   // Generate argument list to pass to the runtime function.

   List<Phx::IR::Operand ^ > ^ arguments = 
      gcnew List<Phx::IR::Operand ^ >();

   // The runtime function expect integer values.
   // If we're validating a smaller type (e.g. 'char'), widen the 
   // operand to integer size.

   Phx::Types::Resolve ^ resolve = nullptr;
   Phx::Types::Type ^ int32Type  = functionUnit->TypeTable->Int32Type;
   
   if (lowerBoundOperand->Type->ByteSize < int32Type->ByteSize)
   {
      if (resolve == nullptr)
         resolve = ModuleBuilder::Resolve;

      lowerBoundOperand = resolve->Convert(
         lowerBoundOperand, 
         functionUnit->LastInstruction->Previous, 
         int32Type
      );
   }
   if (upperBoundOperand->Type->ByteSize < int32Type->ByteSize)
   {
      if (resolve == nullptr)
         resolve = ModuleBuilder::Resolve;

      upperBoundOperand = resolve->Convert(
         upperBoundOperand,
         functionUnit->LastInstruction->Previous,
         int32Type
      );
   }
   if (valueOperand->Type->ByteSize < int32Type->ByteSize)
   {
      if (resolve == nullptr)
         resolve = ModuleBuilder::Resolve;

      valueOperand = resolve->Convert(
         valueOperand, 
         functionUnit->LastInstruction->Previous, 
         int32Type
      );
   }

   // Ensure source line number is also of integer type.

   if (! sourceLineNumberOperand->Type->Equals(int32Type))
   {
      if (resolve == nullptr)
         resolve = ModuleBuilder::Resolve;

      sourceLineNumberOperand = resolve->Convert(
         sourceLineNumberOperand, 
         functionUnit->LastInstruction->Previous, 
         int32Type
      );
   }

   // Append arguments.

   //lower
   arguments->Add(lowerBoundOperand);
   //upper
   arguments->Add(upperBoundOperand);
   //value
   arguments->Add(valueOperand);
   //file_index
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) ModuleBuilder::SourceFileIndex
      )
   );
   //source_line_number
   arguments->Add(sourceLineNumberOperand);

   // Generate the function call.

   return CallUtilityFunction(
      functionUnit,
      "runtime_check_int_range",
      arguments,
      functionUnit->LastInstruction->Previous,
      sourceLineNumber
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a call to the given 'set' function with the 
//    provided arguments.
//
// Remarks:
//
//    This method handles the following library calls:
//     * new_set
//     * empty_set
//     * free_set
//     * ref_set
//     * release_set
//     * set_set_range
//     * set_union
//     * set_intersection
//     * set_difference
//     * set_equality
//     * set_membership
//     * is_subset_of
//     * is_superset_of
//     * set_assign
//
// Returns:
//
//    The Instruction object for the function call.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^
NativeRuntime::CallSetFunction
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ functionName,
   List<Phx::IR::Operand ^ > ^ arguments,
   int sourceLineNumber
)
{
   // Ensure we have a function symbol that matches the provided
   // function name.

   Phx::Symbols::FunctionSymbol ^ functionSymbol = 
      setFunctionSymbols[functionName->ToLower()];

   Debug::Assert(functionSymbol != nullptr);

   // Generate an instruction for the call.

   Phx::IR::Instruction ^ callInstruction = Phx::IR::CallInstruction::New(
      functionUnit,
      Phx::Common::Opcode::Call,
      functionSymbol
   );
 
   // If the function takes multiple set arguments, each set argument
   // must be of the same underlying static type.

   Phx::Types::Type ^ setBaseType = nullptr;   
   bool foundError = false; // don't display errors more than once.
   
   Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

   Phx::Types::Type ^ voidPtrSetType = 
      TypeBuilder::GetTargetType(NativeType::Set);

   for each (Phx::IR::Operand ^ argument in arguments)
   {
      if (TypeBuilder::IsSetType(argument->Type))
      {         
         if (setBaseType == nullptr)
         {
            // Seed initial value.

            setBaseType = argument->Type;
         }
         else
         {
            // Compare against seeded value.

            if (! TypeBuilder::AreEquivalentTypes(setBaseType, argument->Type) 
             && ! foundError)
            {
               Output::ReportError(
                  sourceLineNumber,
                  Error::IncompatibleSetTypes
               );
               foundError = true;
            }
         }

         // Convert to void*.

         argument = resolve->Convert(
            argument,
            functionUnit->LastInstruction->Previous,
            voidPtrSetType
         );
      }

      // Append the set argument.

      callInstruction->AppendSource(argument);      
   }

   // Generate an append a result operand if the function is non-void.

   if (! functionSymbol->Type->AsFunctionType->ReturnType->IsVoid)
   {
      Phx::IR::Operand ^ resultOperand = 
         Phx::IR::VariableOperand::NewExpressionTemporary(
            functionUnit,
            functionSymbol->Type->AsFunctionType->ReturnType
         );
      
      callInstruction->AppendDestination(resultOperand);
   }
   
   // Append the instruction to the IR stream.

   functionUnit->LastInstruction->InsertBefore(callInstruction);

   return callInstruction;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a call to the given 'string' function with the 
//    provided arguments.
//
// Remarks:
//
//    This method handles the following library calls:
//     * _memcpy
//     * _strncmp
//
// Returns:
//
//    The Instruction object for the function call.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^
NativeRuntime::CallStringFunction
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ functionName,
   List<Phx::IR::Operand ^ > ^ arguments,
   int sourceLineNumber
)
{
   // Obtain the function symbol from the ModuleUnit symbol table.

   // Because the user doesn't call these, we don't have to normalize or 
   // store the function name internally.

   Phx::Symbols::Symbol ^ functionSymbol = 
      functionUnit->ParentModuleUnit->
         SymbolTable->NameMap->Lookup(
               Phx::Name::New(functionUnit->ParentModuleUnit->Lifetime, 
               functionName
            )
         );
   
   // Fixup operands of type "array of char" to either
   // "pointer to char" (strncmp) or "pointer to void" (memcmp).

   Phx::Types::Type ^ charType = 
     TypeBuilder::GetTargetType(NativeType::Char);
   Phx::Types::Type ^ voidType = functionUnit->TypeTable->VoidType;

   for (int n = 0; n < arguments->Count; ++n)
   {
      Phx::IR::Operand ^ argument = arguments[n];

      // If the argument is an array type, convert it to the appropriate
      // pointer type.

      if (argument->Type->IsArray)
      {
         Phx::Types::Type ^ ptrType;
         if (functionName->Equals("_memcpy"))
            ptrType = 
               functionUnit->TypeTable->GetUnmanagedPointerType(voidType);
         else
            ptrType = 
               functionUnit->TypeTable->GetUnmanagedPointerType(charType);
                 
         argument = IRBuilder::ConvertStringToPointer(functionUnit, argument);
         arguments[n] = argument;
      }
      
      // If the argument type was originally or was just converted to a pointer
      // type and we're calling memcpy, convert the argument to void*.

      if (argument->Type->IsPointer && functionName->Equals("_memcpy"))
      {
         Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

         argument = resolve->Convert(
            argument, 
            functionUnit->LastInstruction->Previous,
            functionUnit->TypeTable->GetUnmanagedPointerType(voidType)
         );

         arguments[n] = argument;
      }
   }

   // Validate the parameters.
   // ValidateCall will report any validation errors.

   if (! IRBuilder::ValidateCall(functionName, 
            functionSymbol->Type->AsFunctionType, arguments, sourceLineNumber)
      )
   {
      return nullptr;
   }
        
   // Call the function.

   return CallFunction(
      functionUnit,
      functionSymbol->AsFunctionSymbol,
      arguments,
      false,
      functionUnit->LastInstruction->Previous,
      sourceLineNumber
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a call to the given 'file' function with the 
//    provided arguments.
//
// Remarks:
//
//    This method handles the following library calls:
//     * file_reset
//     * file_rewrite
//     * file_eof
//     * file_eoln
//     * file_get_char
//     * file_get_int
//     * file_get_bool
//     * file_get_double
//     * file_get_string
//     * file_put_char
//     * file_put_int
//     * file_put_bool
//     * file_put_double
//     * file_put_string
//     * file_get_std_input
//     * file_get_std_output
//     * file_get_std_error
//     * file_open
//     * file_close
//     * file_page      
//     * file_eat_white
//
// Returns:
//
//    The Instruction object for the function call.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^
NativeRuntime::CallFileFunction
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ functionName,
   List<Phx::IR::Operand ^ > ^ arguments,
   int sourceLineNumber
)
{
   Phx::Types::Table ^ typeTable = functionUnit->TypeTable;

   // Remember the original file name. If we encounter an error, 
   // we can feed back the exact same character-casing that the 
   // uer provided.

   String ^ originalFunctionName = functionName;

   // Translate the function name to all lower-case.

   functionName = functionName->ToLower();

   // Translate user call (e.g. 'put(ch)')
   // into runtime call (e.g. 'file_put_char(ch)').

   // For 'put' and 'get', obtain the buffer variable for the file.

   Phx::Symbols::Symbol ^ valueSymbol = nullptr;
   Phx::IR::Operand ^ accessOperand   = nullptr;
   Phx::Types::Type ^ accessType      = nullptr;
   Phx::Symbols::Symbol ^ fileSymbol  = nullptr;

   if (functionName->Equals("put") ||
       functionName->Equals("get") ||
       functionName->Equals("reset"))
   {
      if (arguments[0]->Type->IsAggregateType)
      {
         // Load the .$current_value field from the operand.

         Phx::Symbols::FieldSymbol ^ fieldSymbol = 
            TypeBuilder::FindFieldSymbol(
               arguments[0]->Type->AsAggregateType,
               "$current_value"
            );

         valueSymbol = fieldSymbol;

         if (valueSymbol == nullptr)
            return nullptr;
     
         Phx::IR::Operand ^ argument = arguments[0];

         Phx::IR::Operand ^ baseOperand;
         if (argument->IsVariableOperand)
         {
            baseOperand = argument->CopyToAddressOperand();
         }
         else
         {
            baseOperand = argument->BaseOperand;
         }

         accessOperand = Phx::IR::MemoryOperand::New(
            functionUnit,
            fieldSymbol->Type,
            nullptr,
            baseOperand->AsVariableOperand,
            Phx::Utility::BitsToBytes(fieldSymbol->BitOffset),
            Phx::Alignment::NaturalAlignment(fieldSymbol->Type),
            functionUnit->AliasInfo->IndirectAliasedMemoryTag,
            functionUnit->SafetyInfo->SafeTag
         );

         accessType = fieldSymbol->Type;        
      }
      else
      {
         accessType = arguments[0]->Type;
      }
   }

   // Translate 'put' user function name to specific 
   // runtime function name.

   if (functionName->Equals("put"))
   {
      if (accessType->Equals(typeTable->Int8Type))
         functionName = "file_put_char";
      else if (accessType->Equals(typeTable->Int32Type))
         functionName = "file_put_int";
      else if (accessType->Equals(TypeBuilder::GetTargetType("boolean")))
      {
         functionName = "file_put_bool";         
         accessOperand->Type = functionUnit->TypeTable->Int32Type;
      }
      else if (accessType->Equals(typeTable->Float64Type))
         functionName = "file_put_double";
      else if (TypeBuilder::IsStringType(accessType))
         functionName = "file_put_string";
      else
      {
         Output::ReportError(
            sourceLineNumber,
            Error::GeneralTypeExpected,
            "Boolean, character, integer, real, or string"
         );
         return nullptr;
      }

      arguments->Add(accessOperand);
   }

   // Translate 'get' user function name to specific 
   // runtime function name.

   else if (functionName->Equals("get"))
   {     
      if (accessType->Equals(typeTable->Int8Type))
         functionName = "file_get_char";
      else if (accessType->Equals(typeTable->Int32Type))
         functionName = "file_get_int";
      else if (accessType->Equals(typeTable->Float64Type))
         functionName = "file_get_double";
      else if (TypeBuilder::IsStringType(accessType))
      {
         functionName = "file_get_string";
         arguments->Add(accessOperand);
      }
      else
      {
         Output::ReportError(
            sourceLineNumber,
            Error::GeneralTypeExpected,
            "character, integer, real, or string"
         );
         return nullptr;
      }
   }

   // Translate 'reset', 'rewrite', or 'page'.

   else if (functionName->Equals("reset"))
   {
      functionName = "file_reset";
   }
   else if (functionName->Equals("rewrite"))
   {
      functionName = "file_rewrite";
   }
   else if (functionName->Equals("page"))
   {
      functionName = "file_page";
   }

   else if (functionName->Equals("eof") || functionName->Equals("eoln"))
   {      
      // Insert the standard 'input' file if no other file was specified.

      if (arguments->Count == 0 || 
         !TypeBuilder::IsFileType(arguments[0]->Type))
      {
         // Ensure 'input' has been declared in the program identifier list.

         if (! ModuleBuilder::IsProgramParameter("input"))
         {
            Output::ReportError(
               sourceLineNumber,
               Error::UndeclaredProgramParameter,
               "input"
            );
         }

         Phx::Symbols::NonLocalVariableSymbol ^ localSymbol =
            ModuleBuilder::MakeProxyInFunctionSymbolTable(
               standardInputSymbol,
               functionUnit->SymbolTable
            );

         arguments->Insert(0, Phx::IR::VariableOperand::New(
            functionUnit,
            localSymbol->Type,
            localSymbol)
         );
      }

      if (functionName->Equals("eoln"))
      {
         Phx::IR::Operand ^ argument = arguments[0];

         // Load the .$current_value field from the operand.

         Phx::Symbols::FieldSymbol ^ fieldSymbol = TypeBuilder::FindFieldSymbol(
            argument->Type->AsAggregateType,
            "$current_value"
         );

         Phx::IR::Operand ^ baseOperand;
         if (argument->IsVariableOperand)
         {
            baseOperand = argument->CopyToAddressOperand();
         }
         else
         {
            baseOperand = argument->BaseOperand;
         }

         Phx::IR::Operand ^ operand = Phx::IR::MemoryOperand::New(
            functionUnit,
            fieldSymbol->Type,
            nullptr,
            baseOperand->AsVariableOperand,
            Phx::Utility::BitsToBytes(fieldSymbol->BitOffset),
            Phx::Alignment::NaturalAlignment(fieldSymbol->Type),
            functionUnit->AliasInfo->IndirectAliasedMemoryTag,
            functionUnit->SafetyInfo->SafeTag
         );       

         arguments->Add(operand);
      }

      functionName = "file_" + functionName;
   }
   else 
   {
      // This is an internal runtime call; maintain the same name.
   }

   // Replace arguments with file static type with the HANDLE runtime type.
   
   Phx::IR::Operand ^ fileOperand = nullptr;
   int stringLength = 0;

   for (int i = 0; i < arguments->Count; ++i)
   {
      Phx::Types::Type ^ argumentType = arguments[i]->Type;

      if (TypeBuilder::IsFileType(argumentType))
      {
         Phx::Symbols::FieldSymbol ^ handleSymbol = 
            TypeBuilder::FindFieldSymbol(
              arguments[i]->Type->AsAggregateType,
              "$runtime_handle"
            );
     
         Phx::IR::Operand ^ operand;
         if (arguments[i]->Symbol)
         {
            operand = Phx::IR::VariableOperand::New(
               functionUnit,
               handleSymbol->Field,
               arguments[i]->Symbol
            );
         }
         else
         {
            operand = Phx::IR::MemoryOperand::New(
               functionUnit,
               handleSymbol->Field,
               nullptr,
               arguments[i]->BaseOperand,
               0,
               Phx::Alignment::NaturalAlignment(handleSymbol->Type),
               functionUnit->AliasInfo->IndirectAliasedMemoryTag,
               functionUnit->SafetyInfo->SafeTag
            );
         }

         fileOperand = arguments[i];

         arguments->Insert(i, operand);
         arguments->RemoveAt(i+1);         
      }
      else if (TypeBuilder::IsStringType(argumentType))
      {
         Phx::IR::Operand ^ argument = arguments[i];

         stringLength = TypeBuilder::GetStringLength(
            argument->Symbol,
            argument->Type
         );

         if (argumentType->IsArray)
         {
            argument = 
               IRBuilder::ConvertStringToPointer(functionUnit, argument);
         
            if (functionName->Equals("file_put_string") ||
                functionName->Equals("file_get_string"))
            {
               Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

               Phx::Types::Type ^ charType = 
                  TypeBuilder::GetTargetType(NativeType::Char);

               Phx::Types::Type ^ ptrToCharType =
                  functionUnit->TypeTable->GetUnmanagedPointerType(charType);
               
               argument = resolve->Convert(
                  argument,
                  functionUnit->LastInstruction->Previous,
                  ptrToCharType
               );
            }
         }

         arguments[i] = argument;
      }
   }

   // Certain functions require the source file index and
   // line number as the final arguments.

   if (functionName->Contains("reset") ||
       functionName->Contains("rewrite") ||       
       functionName->Contains("open") ||
       functionName->Contains("get") ||
       functionName->Contains("put") ||
       functionName->Contains("eat"))

   {
      // String read/write also requires the string length.

      if (functionName->Equals("file_put_string") ||
          functionName->Equals("file_get_string"))
      {
         arguments->Add(Phx::IR::ImmediateOperand::New(
               functionUnit,
               functionUnit->TypeTable->Int32Type,
               (int) stringLength
            )
         );
      }

      arguments->Add(Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) ModuleBuilder::SourceFileIndex
         )
      );
      arguments->Add(Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) sourceLineNumber
         )
      );
   }

   // Get the function symbol for the function name.

   Phx::Symbols::FunctionSymbol ^ functionSymbol = 
      fileFunctionSymbols[functionName];

   Debug::Assert(functionSymbol != nullptr); 

   // Validate argument count.

   int expectedArgCount = 
      functionSymbol->Type->AsFunctionType->ParametersForInstruction.Count;
   if (expectedArgCount != arguments->Count)
   {
      Output::ReportError(
         sourceLineNumber,
         Error::InvalidArgumentCount,
         originalFunctionName, 
         expectedArgCount, 
         arguments->Count
      );
      return nullptr;
   }

   // Call the function.

   Phx::IR::Instruction ^ callInstruction = CallFunction(
      functionUnit,
      functionSymbol->AsFunctionSymbol,
      arguments,
      false,
      functionUnit->LastInstruction->Previous,
      sourceLineNumber
   );

   // For some functions, we have to post-process the function call. 
   // For example, for the 'get' function, copy the results into the 
   // buffer variable mapped to the file.

   originalFunctionName = originalFunctionName->ToLower();

   if (originalFunctionName->Equals("get") && 
      !functionName->Equals("file_get_string"))
   {
      IRBuilder::EmitAssign(
         functionUnit, 
         accessOperand, 
         callInstruction->DestinationOperand, 
         sourceLineNumber
      );
   }

   // For the 'reset' function, set the buffer variable
   // mapped to the file to the initial file value.

   else if (originalFunctionName->Equals("reset"))
   {
      // Call get(f) to get the value.
         
      arguments->Clear();
      arguments->Add(fileOperand);

      callInstruction = CallFileFunction(
         functionUnit,
         "get",
         arguments,
         sourceLineNumber
      );
   }

   // For the 'eof' and 'eoln' functions, copy the result into a Boolean value.

   else if (originalFunctionName->Equals("eof") ||
            originalFunctionName->Equals("eoln"))
   {
      Phx::IR::Operand ^ booleanResult = IRBuilder::PromoteToBoolean(
         functionUnit,
         callInstruction->DestinationOperand,
         sourceLineNumber
      );

      // Although this is not ideal, assign the new Boolean result to itself
      // so we can satisfy the requirement that this method return an 
      // instruction object.

      callInstruction = Phx::IR::ValueInstruction::NewUnary(
         functionUnit,
         Phx::Common::Opcode::Assign,
         booleanResult,
         booleanResult
      );

      // Append the instruction to the IR stream.

      functionUnit->LastInstruction->InsertBefore(callInstruction);
   }

   return callInstruction;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a call to the given 'allocation' function with the 
//    provided arguments.
//
// Remarks:
//
//    This method handles the following library calls:
//     * new
//     * dispose
//
// Returns:
//
//    The Instruction object for the function call.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^
NativeRuntime::CallAllocationFunction
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ functionName,
   List<Phx::IR::Operand ^ > ^ arguments,
   int sourceLineNumber
)
{
   // Translate the function name to all lower-case.

   functionName = functionName->ToLower();

   // Translate 'new' to '_new'.

   if (functionName->Equals("new"))
      functionName = "_new";

   // For the 'new' function, the user will have passed the object to be 
   // allocated. The runtime library function expects a size and will return
   // the allocated memory. 
   // Therefore, we need to replace the first argument and perform the 
   // assignment after the call.

   Phx::IR::Operand ^ allocationOperand = nullptr;
   
   if (functionName->Equals("_new"))
   {
      // User must pass a pointer as the first argument.

      if (arguments->Count == 0 || !arguments[0]->Type->IsPointerType)
      {
         Output::ReportError(
            sourceLineNumber,
            Error::GeneralTypeExpected,
            "pointer"
         );
         return nullptr;
      }

      allocationOperand = arguments[0];

      // Append size argument.

      arguments->Clear();
      arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) allocationOperand->Type->AsPointerType->ReferentType->ByteSize
         )
      );
   }
   else
   {
      Debug::Assert(functionName->Equals("dispose"));

      // The caller must pass a pointer as the first argument.
      // Report an error if the called provided no arguments or 
      // a non-pointer argument type.

      if (arguments->Count == 0 || 
         !arguments[0]->Type->IsPointerType)
      {
         Output::ReportError(
            sourceLineNumber,
            Error::GeneralTypeExpected,
            "pointer"
         );
         return nullptr;
      }

      // The dispose runtime function takes a void* as its 
      // first argument. Use a Resolve object to convert from
      // the pointer type to void*.

      Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

      Phx::Types::Type ^ pointerToVoidType = 
         functionUnit->TypeTable->GetUnmanagedPointerType(
            functionUnit->TypeTable->VoidType);

      Phx::IR::Operand ^ pointerOperand = 
         resolve->Convert(arguments[0], 
            functionUnit->LastInstruction->Previous,
            pointerToVoidType
         );

      // Replace the first argument with the converted
      // pointer operand.
      arguments->RemoveAt(0);
      arguments->Insert(0, pointerOperand);
   }

   // Get the function symbol for the function.

   Phx::Symbols::FunctionSymbol ^ functionSymbol = 
      allocFunctionSymbols[functionName];

   Debug::Assert(functionSymbol != nullptr);
   
   //file_index
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) ModuleBuilder::SourceFileIndex
      )
   );
   //source_line_number
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) sourceLineNumber
      )
   );

   // Call the function.

   Phx::IR::Instruction ^ callInstruction = CallFunction(
      functionUnit,
      functionSymbol->AsFunctionSymbol,
      arguments,
      true,
      functionUnit->LastInstruction->Previous,
      sourceLineNumber
   );

   // If we called the 'new' function, assign the returned memory
   // to the user variable.

   if (allocationOperand)
   {
      Debug::Assert(functionName->Equals("_new"));

      // Convert void* to the user type.

      Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

      Phx::IR::Operand ^ convertTypeOperand = resolve->Convert(
         callInstruction->DestinationOperand, 
         functionUnit->LastInstruction->Previous,
         allocationOperand->Type
      );

      Phx::IR::Instruction ^ assignInstruction = 
         Phx::IR::ValueInstruction::NewUnary(
            functionUnit,
            Phx::Common::Opcode::Assign,
            allocationOperand,
            convertTypeOperand
         );

      functionUnit->LastInstruction->InsertBefore(assignInstruction);
      callInstruction = assignInstruction;
   }
   
   return callInstruction;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a call to the given 'math' function with the    
//    provided arguments.
//
// Remarks:
//
//    This method handles the following library calls:
//     * pow
//     * ab
//     * fabs
//     * sin
//     * cos
//     * odd
//     * mod
//     * sqr
//     * sqrf
//     * trunc
//     * round
//
// Returns:
//
//    The Instruction object for the function call.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^
NativeRuntime::CallMathFunction
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ functionName,
   List<Phx::IR::Operand ^ > ^ arguments,
   int sourceLineNumber
)
{
   // The built-in functions 'sin', 'cos', 'arctan', 'ln', 'exp', 
   // 'pow', and 'sqrt' may accept integer type. However, the runtime
   // functions take real numbers.
   // Flag the need for conversion if we are calling one of these functions.

   array<String ^> ^ promoteFunctions = 
      { "sin", "cos", "arctan", "ln", "exp", "pow", "sqrt" };
   bool promoteToReal = false;
   for each (String ^ promoteFunction in promoteFunctions)
   {
      if (functionName->ToLower()->Equals(promoteFunction))
      {
         promoteToReal = true;
         break;
      }
   }

   // Translate the Pascal function name to the equivalent
   // runtime library name (such as 'arctan' to 'atan').

   functionName = TranslateMathFunctionName(functionName, arguments);

   // Obtain the function symbol for the function.

   Phx::Symbols::FunctionSymbol ^ functionSymbol = 
      mathFunctionSymbols[functionName];

   Debug::Assert(functionSymbol != nullptr);
   
   Debug::Assert(functionSymbol->Type->AsFunctionType->
      ParametersForInstruction.Count == arguments->Count);

   // First, create a new Call instruction.

   Phx::IR::Instruction ^ callInstruction = 
      Phx::IR::CallInstruction::New(functionUnit,
         Phx::Common::Opcode::Call,
         functionSymbol
      );
   
   Phx::Types::Type ^ integerType = 
      TypeBuilder::GetTargetType(NativeType::Integer);
   Phx::Types::Type ^ realType = 
      TypeBuilder::GetTargetType(NativeType::Real);
   
   // Append source arguments.

   for each (Phx::IR::Operand ^ argument in arguments)
   {
      // Test for need to promote argument to floating point.

      if (promoteToReal && argument->Type->Equals(integerType))
      {
         Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

         argument = resolve->Convert(
            argument,
            functionUnit->LastInstruction->Previous,
            realType
         );
      }

      // All math routies must be either Integer or Real type.

      if (! argument->Type->Equals(integerType) &&
          ! argument->Type->Equals(realType))
      {
         Output::ReportError(
            sourceLineNumber,
            Error::GeneralTypeExpected,
            "integer or real"
         );
      }

      callInstruction->AppendSource(argument);
   }

   // Generate an append a result operand if the function is non-void.

   if (! functionSymbol->Type->AsFunctionType->ReturnType->IsVoid)
   {
      Phx::IR::Operand ^ result = Phx::IR::VariableOperand::NewExpressionTemporary(
         functionUnit,
         functionSymbol->Type->AsFunctionType->ReturnType
      );

      callInstruction->AppendDestination(result);
   }

   // Append the instruction to the IR stream.

   functionUnit->LastInstruction->InsertBefore(callInstruction);

   // The runtime 'odd' function returns int, so we must promote it to Boolean.

   if (functionName->Equals("odd"))
   {
      Phx::IR::Operand ^ booleanResult = IRBuilder::PromoteToBoolean(
         functionUnit,
         callInstruction->DestinationOperand,
         sourceLineNumber
         );

      // Although this is not ideal, assign the new Boolean result to itself
      // so we can satisfy the requirement that this method return an 
      // instruction object.

      callInstruction = Phx::IR::ValueInstruction::NewUnary(
         functionUnit,
         Phx::Common::Opcode::Assign,
         booleanResult,
         booleanResult
      );

      functionUnit->LastInstruction->InsertBefore(callInstruction);      
   }   

   return callInstruction;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a call to the given 'transfer' function with the 
//    provided arguments.
//
// Remarks:
//
//    This method handles the following library calls:
//     * ord
//     * chr
//     * pred
//     * succ
//     * pack
//     * unpack
//
// Returns:
//
//    The Instruction object for the function call.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^
NativeRuntime::CallTransferFunction
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ functionName,
   List<Phx::IR::Operand ^ > ^ arguments,
   int sourceLineNumber
)
{
   // Remember the original file name. If we encounter an error, 
   // we can feed back the exact same character-casing that the 
   // uer provided.
   
   String ^ originalFunctionName = functionName;

   // Get the function symbol for the function.

   functionName = functionName->ToLower();

   Phx::Symbols::FunctionSymbol ^ functionSymbol = 
      transferFunctionSymbols[functionName];

   Debug::Assert(functionSymbol != nullptr);
   
   // First, create a new Call instruction.

   Phx::IR::Instruction ^ callInstruction = 
      Phx::IR::CallInstruction::New(
         functionUnit,
         Phx::Common::Opcode::Call,
         functionSymbol
      );

   Phx::Types::Type ^ voidType = functionUnit->TypeTable->VoidType;

   // For the 'pack' and 'unpack' routines, we need to alter the arguments.
   
   bool isPack = false;   
   if (functionName->Equals("pack") || functionName->Equals("unpack"))
   {
      // Validate argument count.

      if (3 != arguments->Count)
      {
         Output::ReportError(
            sourceLineNumber,
            Error::InvalidArgumentCount,
            originalFunctionName, 
            3, 
            arguments->Count
         );
         return nullptr;
      }

      isPack = true;
            
      Phx::Types::Type ^ ptrToVoidType = Phx::Types::PointerType::New(
         functionUnit->TypeTable,
         Phx::Types::PointerTypeKind::UnmanagedPointer, 
         functionUnit->TypeTable->NativePointerBitSize,
         voidType, 
         nullptr
      );

      Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

      Phx::IR::Operand ^ a; int aArg;
      Phx::IR::Operand ^ i; int iArg;
      Phx::IR::Operand ^ z; int zArg;

      // pack:   (a,i,z) --> (a,z,offset,size)
      // unpack: (z,a,i) --> (a,z,offset,size)
      
      if (functionName[0] == 'u') //unpack
      {
         z = arguments[0]; zArg = 1;
         a = arguments[1]; aArg = 2;
         i = arguments[2]; iArg = 3;
      }
      else //pack
      {
         a = arguments[0]; aArg = 1;
         i = arguments[1]; iArg = 2;
         z = arguments[2]; zArg = 3;
      }

      // Validate argument types.

      // a must be array type.

      if (! a->Type->IsArray)
      {
         Output::ReportError(
            sourceLineNumber,
            Error::GeneralTypeExpected,
            "array"
         );
         return nullptr;
      }

      // z must be array type.

      if (! z->Type->IsArray)
      {
         Output::ReportError(
            sourceLineNumber,
            Error::GeneralTypeExpected,
            "array"
         );
         return nullptr;
      }

      // argument types must be equal.

      if (! a->Type->AsUnmanagedArrayType->ElementType->Equals(
            z->Type->AsUnmanagedArrayType->ElementType))
      {
         Output::ReportError(
            sourceLineNumber,
            Error::MismatchedArrayElementTypes,
            originalFunctionName
         );
         return nullptr;
      }

      // z must be >= than a.

      int aSize = a->Type->AsUnmanagedArrayType->ByteSize;
      int zSize = z->Type->AsUnmanagedArrayType->ByteSize;

      if (aSize < zSize)
      {
         Output::ReportError(
            sourceLineNumber,
            Error::MismatchedArraySizes,
            originalFunctionName
         );
         return nullptr;
      }

      // indexer must be integer type.

      if (! i->Type->Equals(TypeBuilder::GetTargetType(NativeType::Integer)))
      {
         Output::ReportError(
            sourceLineNumber,
            Error::GeneralTypeExpected,
            "integer"
         );
         return nullptr;
      }

      // Convert and append arguments.

      int elementSize = a->Type->AsUnmanagedArrayType->ElementType->ByteSize;
      int arraySize = zSize;

      //a
      if (! a->IsAddress)
         a->ChangeToAddress();

      // Convert to void*.

      a = resolve->Convert(
         a,
         functionUnit->LastInstruction->Previous,
         ptrToVoidType
      );

      callInstruction->AppendSource(a);

      //z
      if (! z->IsAddress)
         z->ChangeToAddress();

      // Convert to void*.

      z = resolve->Convert(
         z,
         functionUnit->LastInstruction->Previous,
         ptrToVoidType
      );

      callInstruction->AppendSource(z);

      //offset
      callInstruction->AppendSource(
         IRBuilder::EmitBinaryArithmeticOp(
            functionUnit,
            STAR,
            functionUnit->TypeTable->Int32Type,
            Phx::IR::ImmediateOperand::New(
                  functionUnit,
                  functionUnit->TypeTable->Int32Type,
                  (int) elementSize
               ),
            i,
            sourceLineNumber
            )
         );
      //size
      callInstruction->AppendSource(
         Phx::IR::ImmediateOperand::New(
               functionUnit,
               functionUnit->TypeTable->Int32Type,
               (int) arraySize
            )
         );
   }
         
   Phx::Types::Type ^ resultType = voidType;
   
   if (! isPack)
   {
      Debug::Assert(functionSymbol->Type->AsFunctionType->
         ParametersForInstruction.Count == arguments->Count);

      Phx::Types::Type ^ intType = 
         TypeBuilder::GetTargetType(NativeType::Integer);
      
      for each (Phx::IR::Operand ^ argument in arguments)
      {
         // The parameters to these routines must be of ordinal type.

         if (! TypeBuilder::IsOrdinalType(argument->Type))
         {
            Output::ReportError(
               sourceLineNumber,
               Error::GeneralTypeExpected,
               "ordinal"
            );
         }

         resultType = argument->Type;

         // Widen to size of integer if needed.

         if (argument->Type->ByteSize < intType->ByteSize)
         {
            Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

            argument = resolve->Convert(
               argument,
               functionUnit->LastInstruction->Previous,
               intType
            );
         }

         // Convert enum values to integer.
      
         if (argument->Type->IsEnumType)
         {
            argument = IRBuilder::ExtractPrimaryEnumField(
               functionUnit,
               argument
            );
         }

         callInstruction->AppendSource(argument);
      }
   }

   // Generate an append a result operand if the function is non-void.

   if (! functionSymbol->Type->AsFunctionType->ReturnType->IsVoid)
   {
      Phx::IR::Operand ^ result = Phx::IR::VariableOperand::NewExpressionTemporary(
         functionUnit,
         functionSymbol->Type->AsFunctionType->ReturnType
      );

      callInstruction->AppendDestination(result);
   }

   // Append the instruction to the IR stream.

   functionUnit->LastInstruction->InsertBefore(callInstruction);

   // For the 'succ' and 'pred' functions, return the same type that was
   // passed to the function.

   if (functionName->Equals("succ") || functionName->Equals("pred"))
   {  
      if (! resultType->Equals(callInstruction->DestinationOperand->Type))
      {
         Phx::IR::Operand ^ resultOperand;
         if (resultType->IsEnumType)
         {
            resultOperand = IRBuilder::PromoteToEnum(
               functionUnit,
               resultType,
               callInstruction->DestinationOperand,
               sourceLineNumber
            );

            // Although this is not ideal, assign the new enum result to itself
            // so we can satisfy the requirement that this method return an 
            // instruction object.

            callInstruction = Phx::IR::ValueInstruction::NewUnary(
               functionUnit,
               Phx::Common::Opcode::Assign,
               resultOperand,
               resultOperand
               );
          
            functionUnit->LastInstruction->InsertBefore(callInstruction);
            return callInstruction;
         }
         else
         {
            resultOperand = Phx::IR::VariableOperand::NewExpressionTemporary(
               functionUnit,
               resultType
            );
            Phx::IR::Operand ^ sourceOperand = 
               callInstruction->DestinationOperand;
            if (! sourceOperand->Type->Equals(resultOperand->Type))
            {
               Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

               sourceOperand = resolve->Convert(
                  sourceOperand,
                  functionUnit->LastInstruction->Previous,
                  resultType
               );
            }
            Phx::IR::Instruction ^ assignInstruction = 
               Phx::IR::ValueInstruction::NewUnary(
                  functionUnit,
                  Phx::Common::Opcode::Assign,
                  resultOperand,
                  sourceOperand
               );

            functionUnit->LastInstruction->InsertBefore(assignInstruction);
            return assignInstruction;
         }
      }
   }

   return callInstruction;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a call to the given 'utility' function with the 
//    provided arguments.
//
// Remarks:
//
//    This method handles the following library calls:
//     * get_lower_bound
//     * get_upper_bound
//     * runtime_check_int_range
//
// Returns:
//
//    The Instruction object for the function call.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^
NativeRuntime::CallUtilityFunction
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ functionName,
   List<Phx::IR::Operand ^ > ^ arguments,
   Phx::IR::Instruction ^ insertAfterInstruction,
   int sourceLineNumber
)
{
   // Obtain the function symbol for the function.

   Phx::Symbols::FunctionSymbol ^ functionSymbol = 
      utilityFunctionSymbols[functionName->ToLower()];

   Debug::Assert(functionSymbol != nullptr);

   // Call the function.

   return CallFunction(
      functionUnit,
      functionSymbol->AsFunctionSymbol,
      arguments,
      false,
      insertAfterInstruction,
      sourceLineNumber
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a call to the given 'display' function with the 
//    provided arguments.
//
// Remarks:
//
//    This method handles the following library calls:
//     * display_enter
//     * display_leave
//     * display_get_address
//
// Returns:
//
//    The Instruction object for the function call.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^
NativeRuntime::CallDisplayFunction
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ functionName,
   List<Phx::IR::Operand ^ > ^ arguments, 
   Phx::IR::Instruction ^ insertAfterInstruction,
   int sourceLineNumber
)
{
   return CallFunction(
      functionUnit,
      displayFunctionSymbols[functionName->ToLower()],
      arguments,
      false, // function might contain varargs argument
      insertAfterInstruction,
      sourceLineNumber
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the return type for the runtime function that matches
//    the given signature (name and argument lits).
//
// Remarks:
//
//    Because the return type of some functions is dependent on their arguments,
//    we must know the argument list to determine the return type.
//
// Returns:
//
//    The return type of the function.
//
//-----------------------------------------------------------------------------

Phx::Types::Type ^ 
NativeRuntime::GetFunctionReturnType
(
   String ^ functionName,
   List<Phx::IR::Operand ^> ^ arguments
)
{
   // Translate the function name to all lower-case.

   functionName = functionName->ToLower();

   // Verify that the function is a runtime function.

   Debug::Assert(IsRuntimeFunctionName(functionName));

   // Special case for built-in 'write(ln)' and 'read(ln)' procedures.

   if (functionName->StartsWith("write"))
   {
      return GetFunctionReturnType("put", arguments);
   }
   else if (functionName->StartsWith("read"))
   {
      return GetFunctionReturnType("get", arguments);
   }

   // Special case for 'eof' and 'eoln' functions.

   else if (functionName->Equals("eof") ||
            functionName->Equals("eoln"))
   {
      return TypeBuilder::GetTargetType("Boolean");
   }

   else if (IsFileFunction(functionName))
   {
      return 
         fileFunctionSymbols
         [TranslateFileFunctionName(functionName)]->Type->
            AsFunctionType->ReturnType;
   }

   else if (IsStringFunction(functionName))
   {
      return
         stringFunctionSymbols
         [functionName->ToLower()]->Type->AsFunctionType->ReturnType;
   }

   else if (IsMathFunction(functionName))
   {
      return 
         mathFunctionSymbols
         [TranslateMathFunctionName(functionName, arguments)]->Type->
            AsFunctionType->ReturnType;
   }
   else if (IsTransferFunction(functionName))
   {
      // For the 'succ' and 'pred' functions, the return type is
      // the same as the argument type.

      if (functionName->Equals("succ") || functionName->Equals("pred"))
      {
         if (arguments->Count > 0)
            return arguments[0]->Type;
      }

      return transferFunctionSymbols[functionName]->Type->
         AsFunctionType->ReturnType;
   }

   Debug::Assert(false);
   return nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given symbol refers to the built-in 
//    'input' or 'output' file symbols.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool
NativeRuntime::IsRuntimeFileSymbol
(
   Phx::Symbols::Symbol ^ fileSymbol
)
{
   if (fileSymbol == nullptr)
      return false;

   return fileSymbol->NameString->Equals(
      this->standardInputSymbol->NameString) ||
          fileSymbol->NameString->Equals(
      this->standardOutputSymbol->NameString);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds an internal list of all of the built-in Pascal functions
//    and procedures.
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
NativeRuntime::BuildIntrinsicFunctionTable()
{   
   runtimeFunctionNames = gcnew List<String ^>();

   for each(String ^ functionName in gcnew array<String ^> {
      // Functions.
      "abs", "arctan", "chr", "cos", "eof", "eoln", "exp",
      "ln", "odd", "ord", "pred", "round", "sin", "sqr", 
      "sqrt", "succ", "trunc",
      // Procedures.
      "dispose", "get", "new", "pack", "page", "put",
      "read", "readln", "reset", "rewrite", "unpack",
      "write", "writeln",
   })
   {
      runtimeFunctionNames->Add(functionName);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds initialization data for the main function unit.
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
NativeRuntime::BuildInitializedData
(
   Phx::FunctionUnit ^ functionUnit,
   int sourceLineNumber
)
{
   // Initialize values for global Boolean enumeration.

   if (functionUnit)
   {
      array<Phx::Symbols::Symbol ^> ^ booleanSymbols = {
         ModuleBuilder::LookupSymbol(functionUnit, "false"),
         ModuleBuilder::LookupSymbol(functionUnit, "true"),
      };

      Phx::Types::Type ^ booleanType = TypeBuilder::GetTargetType("Boolean");
      Phx::Types::Type ^ underlyingPrimType = 
         booleanType->AsAggregateType->UnderlyingPrimitiveType;

      for (int i = 0; i < 2; ++i)
      {
         // Get global symbol.

         Phx::Symbols::Symbol ^ symbol = booleanSymbols[i];

         // Create local proxy.

         symbol = ModuleBuilder::MakeProxyInFunctionSymbolTable(
            symbol->AsGlobalVariableSymbol, 
            functionUnit->SymbolTable
         );

         // Load first field from memory.

         Phx::IR::VariableOperand ^ baseOperand = 
            Phx::IR::VariableOperand::New(
               functionUnit,
               booleanType,
               symbol
            );

         baseOperand->ChangeToAddress();

         Phx::IR::MemoryOperand ^ destinationOperand = 
            Phx::IR::MemoryOperand::New(
               functionUnit,
               booleanType->GetField(
                  underlyingPrimType,
                  0),
               nullptr,
               baseOperand,
               0,
               Phx::Alignment::NaturalAlignment(underlyingPrimType),
               functionUnit->AliasInfo->IndirectAliasedMemoryTag,
               functionUnit->SafetyInfo->SafeTag
            );

         // Generate initial value (0 for false; 1 for true).

         Phx::IR::ImmediateOperand ^ sourceOperand = 
            Phx::IR::ImmediateOperand::New(
               functionUnit,
               underlyingPrimType,
               (int) i
            );

         // Generate assignment statement.

         Phx::IR::Instruction ^ assignInstruction = 
            Phx::IR::ValueInstruction::NewUnary(
               functionUnit,
               Phx::Common::Opcode::Assign,
               destinationOperand, 
               sourceOperand
            );
        
         functionUnit->FirstEnterInstruction->InsertAfter(assignInstruction);
      }
      
      // Initialize the built-in 'input' and 'output' file handles.

      InitializeStandardHandles(functionUnit, sourceLineNumber);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds the function symbols for the 'set' family of
//    built-in procedures.
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
NativeRuntime::BuildSetFunctionSymbols()
{
   setFunctionSymbols =
      gcnew Dictionary<String ^, Phx::Symbols::FunctionSymbol ^>();

   Phx::Types::Table ^ typeTable = this->module->TypeTable;
   
   // Build the function types.

   Phx::Types::Type ^ setType = TypeBuilder::GetTargetType(NativeType::Set);
   Phx::Types::Type ^ int32Type = typeTable->Int32Type;
   Phx::Types::Type ^ voidType = typeTable->VoidType;

   array<String ^> ^ signatures = {
      "_new_set",
      "_empty_set",
      "_free_set",
      "_ref_set",
      "_release_set",
      "_set_set_values",
      "_set_union",
      "_set_intersection",
      "_set_difference",
      "_set_equality",
      "_set_membership",
      "_is_subset_of",
      "_is_superset_of",
      "_set_assign",
   };
   
   array<array<Phx::Types::Type ^> ^> ^ argumentLists = {
      {int32Type, int32Type, int32Type, int32Type},
      {int32Type, int32Type},
      {setType},
      {setType},
      {setType},
      {int32Type, int32Type, setType, int32Type, int32Type},
      {setType, setType, int32Type, int32Type},
      {setType, setType, int32Type, int32Type},
      {setType, setType, int32Type, int32Type},
      {setType, setType},
      {setType, int32Type},
      {setType, setType},
      {setType, setType},
      {setType, setType, int32Type, int32Type, int32Type},
   };

   array<bool> ^ isEllipsis = {
      false, false, false, false, false,
      true,
      false, false, false, false, false,
      false, false, false,
   };
   
   array<Phx::Types::Type ^> ^ returnValues = {
      setType, setType, voidType, voidType, voidType, voidType,
      setType, setType, setType,
      int32Type, int32Type, int32Type, int32Type,
      voidType,
   };

   for (int i = 0; i < signatures->Length; ++i)
   {      
      // Create and add the function symbol to the 
      // set function symbol map.
      // The string name is adjusted to remove the 
      // initial underscore character.

      setFunctionSymbols[signatures[i]->Substring(1)] = 
            ModuleBuilder::BuildFunctionSymbol(
            typeTable->Lifetime,
            typeTable,
            this->module->SymbolTable,
            signatures[i],
            Phx::Types::CallingConventionKind::CDecl,
            Phx::Symbols::Visibility::GlobalReference,
            argumentLists[i],
            isEllipsis[i],
            returnValues[i]
         );
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds the function symbols for the 'file' family of
//    built-in procedures.
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
NativeRuntime::BuildFileFunctionSymbols()
{
   fileFunctionSymbols =
      gcnew Dictionary<String ^, Phx::Symbols::FunctionSymbol ^>();
  
   Phx::Types::Table ^ typeTable = this->module->TypeTable;

   // Build the function types.

   Phx::Types::Type ^ handleType = 
      TypeBuilder::GetTargetType("HANDLE");
   Phx::Types::Type ^ int8Type = 
      typeTable->Int8Type;
   Phx::Types::Type ^ int32Type = 
      typeTable->Int32Type;
   Phx::Types::Type ^ float64Type = 
      typeTable->Float64Type;
   Phx::Types::Type ^ voidType = 
      typeTable->VoidType;
   Phx::Types::Type ^ ptrToCharType = 
      Phx::Types::PointerType::New(
         typeTable,
         Phx::Types::PointerTypeKind::UnmanagedPointer, 
         typeTable->NativePointerBitSize,
         typeTable->Int8Type, 
         nullptr
      );
  
   array<String ^> ^ signatures = {
      "_file_reset",
      "_file_rewrite",
      "_file_eof",
      "_file_eoln",
      "_file_get_char",
      "_file_get_int",
      "_file_get_bool",
      "_file_get_double",
      "_file_get_string",
      "_file_put_char",
      "_file_put_int",
      "_file_put_bool",
      "_file_put_double",
      "_file_put_string",
      "_file_get_std_input",
      "_file_get_std_output",
      "_file_get_std_error",
      "_file_open",
      "_file_close",
      "_file_page",
      "_file_eat_white",
      "_file_set_modifier",
   };
      
   array<array<Phx::Types::Type ^> ^> ^ argumentLists = {
      {handleType, int32Type, int32Type},
      {handleType, int32Type, int32Type},
      {handleType},
      {handleType, int8Type},
      {handleType, int32Type, int32Type},
      {handleType, int32Type, int32Type},
      {handleType, int32Type, int32Type},
      {handleType, int32Type, int32Type},
      {handleType, ptrToCharType, int32Type, int32Type, int32Type},
      {handleType, int8Type, int32Type, int32Type},
      {handleType, int32Type, int32Type, int32Type},
      {handleType, int32Type, int32Type, int32Type},
      {handleType, float64Type, int32Type, int32Type},
      {handleType, ptrToCharType, int32Type, int32Type, int32Type},
      {int32Type, int32Type},
      {int32Type, int32Type},
      {int32Type, int32Type},
      {ptrToCharType, int32Type, int32Type, int32Type},
      {handleType},
      {handleType},
      {handleType, int32Type, int32Type},
      {handleType, int32Type, int32Type},
   };

   array<bool> ^ isEllipsis = {
      false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, 
      false, false, false, false, false, false,
   };
   
   array<Phx::Types::Type ^> ^ returnValues = {
      voidType, voidType, int32Type, int32Type, int8Type,
      int32Type, int32Type, float64Type, ptrToCharType,
      voidType, voidType, voidType, voidType, voidType,
      handleType, handleType, handleType, handleType,
      voidType, voidType, voidType, voidType,
   };

   for (int i = 0; i < signatures->Length; ++i)
   {
      // Add the function symbol to the set function symbol map.
      // The string name is adjusted to remove the 
      // initial underscore character.

      fileFunctionSymbols[signatures[i]->Substring(1)] =
            ModuleBuilder::BuildFunctionSymbol(
            typeTable->Lifetime,
            typeTable,
            this->module->SymbolTable,
            signatures[i],
            Phx::Types::CallingConventionKind::CDecl,
            Phx::Symbols::Visibility::GlobalReference,
            argumentLists[i],
            isEllipsis[i],
            returnValues[i]
         );
   }
  
   fileFunctions = gcnew List<String ^>();
   fileFunctions->Add("put");
   fileFunctions->Add("get");
   fileFunctions->Add("reset");
   fileFunctions->Add("rewrite");
   fileFunctions->Add("page");
   fileFunctions->Add("eof");
   fileFunctions->Add("eoln");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds the function symbols for the 'string' family of
//    built-in procedures.
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
NativeRuntime::BuildStringFunctionSymbols()
{
   stringFunctionSymbols =
      gcnew Dictionary<String ^, Phx::Symbols::FunctionSymbol ^>();
      
   Phx::Types::Table ^ typeTable = this->module->TypeTable;

   // Build the function types.

   Phx::Types::Type ^ int32Type = typeTable->Int32Type;
   Phx::Types::Type ^ sizetType = typeTable->UInt32Type;
   Phx::Types::Type ^ ptrToVoidType = Phx::Types::PointerType::New(
      typeTable,
      Phx::Types::PointerTypeKind::UnmanagedPointer, 
      typeTable->NativePointerBitSize,
      typeTable->VoidType, 
      nullptr
   );
   Phx::Types::Type ^ ptrToCharType = Phx::Types::PointerType::New(
      typeTable,
      Phx::Types::PointerTypeKind::UnmanagedPointer, 
      typeTable->NativePointerBitSize,
      typeTable->Int8Type, 
      nullptr
   );
 
   array<String ^> ^ signatures = {
      "_memcpy",
      "_strncmp",    
   };
      
   array<array<Phx::Types::Type ^> ^> ^ argumentLists = {
      {ptrToVoidType, ptrToVoidType, sizetType},
      {ptrToCharType, ptrToCharType, sizetType},
   };
   
   array<Phx::Types::Type ^> ^ returnValues = {
      int32Type, int32Type,
   };

   for (int i = 0; i < signatures->Length; ++i)
   {
      // Add the function symbol to the set function symbol map.
      // The string name is adjusted to remove the initial 
      // underscore character.

      stringFunctionSymbols[signatures[i]] =
            ModuleBuilder::BuildFunctionSymbol(
            typeTable->Lifetime,
            typeTable,
            this->module->SymbolTable,
            signatures[i],
            Phx::Types::CallingConventionKind::CDecl,
            Phx::Symbols::Visibility::GlobalReference,
            argumentLists[i],
            false,
            returnValues[i]
         );
   }

   stringFunctions = gcnew List<String ^>();
   stringFunctions->Add("_memcpy");
   stringFunctions->Add("_strncmp");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds the function symbols for the 'allocation' family of
//    built-in procedures.
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
NativeRuntime::BuildAllocationFunctionSymbols()
{
   allocFunctionSymbols =
      gcnew Dictionary<String ^, Phx::Symbols::FunctionSymbol ^>();

   Phx::Types::Table ^ typeTable = this->module->TypeTable;

   // Build the function type.

   Phx::Types::Type ^ int32Type = typeTable->Int32Type;   
   Phx::Types::Type ^ voidType = typeTable->VoidType;
   Phx::Types::Type ^ ptrToVoidType = Phx::Types::PointerType::New(
      typeTable,
      Phx::Types::PointerTypeKind::UnmanagedPointer, 
      typeTable->NativePointerBitSize,
      voidType, 
      nullptr
   );

   array<String ^> ^ signatures = {
      "__new", "_dispose",
   };   
   
   array<array<Phx::Types::Type ^> ^> ^ argumentLists = {
      {int32Type, int32Type, int32Type},
      {ptrToVoidType, int32Type, int32Type},
   };

   array<bool> ^ isEllipsis = {
      false, false,
   };
   
   array<Phx::Types::Type ^> ^ returnValues = {
      ptrToVoidType, voidType,      
   };

   for (int i = 0; i < signatures->Length; ++i)
   {
      // Add the function symbol to the set function symbol map.
      // The string name is adjusted to remove the initial 
      // underscore character.

      allocFunctionSymbols[signatures[i]->Substring(1)] = 
         ModuleBuilder::BuildFunctionSymbol(
            typeTable->Lifetime,
            typeTable,
            this->module->SymbolTable,
            signatures[i],
            Phx::Types::CallingConventionKind::CDecl,
            Phx::Symbols::Visibility::GlobalReference,
            argumentLists[i],
            isEllipsis[i],
            returnValues[i]
         );
   }

   allocFunctions = gcnew List<String ^>();
   allocFunctions->Add("new");
   allocFunctions->Add("dispose");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds the function symbols for the 'utility' family of
//    built-in procedures.
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
NativeRuntime::BuildUtilityFunctionSymbols()
{
   utilityFunctionSymbols =
      gcnew Dictionary<String ^, Phx::Symbols::FunctionSymbol ^>();
   
   Phx::Types::Table ^ typeTable = this->module->TypeTable;

   // Build the function type.
   
   Phx::Types::Type ^ voidType = typeTable->VoidType;
   Phx::Types::Type ^ int32Type = typeTable->Int32Type;
   Phx::Types::Type ^ float64Type = typeTable->Float64Type;
   Phx::Types::Type ^ ptrToCharType = Phx::Types::PointerType::New(
      typeTable,
      Phx::Types::PointerTypeKind::UnmanagedPointer, 
      typeTable->NativePointerBitSize,
      typeTable->Int8Type, 
      nullptr
   );

   array<String ^> ^ signatures = {
      "_set_filename",
      "_get_lower_bound",
      "_get_upper_bound",
      "_runtime_check_int_range",
      "_halt",
      "_runtime_init",
      "_runtime_exit",
   };
   
   array<array<Phx::Types::Type ^> ^> ^ argumentLists = {
      {ptrToCharType, int32Type},
      {int32Type},
      {int32Type},
      {int32Type, int32Type, int32Type, int32Type, int32Type},
      {},
      {},
      {},
   };

   array<bool> ^ isEllipsis = {
      false, true, true, false, false, false, false,
   };
   
   array<Phx::Types::Type ^> ^ returnValues = {
      voidType, int32Type, int32Type,
      voidType, voidType, voidType, voidType,
   };

   for (int i = 0; i < signatures->Length; ++i)
   {
      // Add the function symbol to the utility function symbol map.
      // The string name is adjusted to remove the initial 
      // underscore character.

      utilityFunctionSymbols[signatures[i]->Substring(1)] = 
         ModuleBuilder::BuildFunctionSymbol(
            typeTable->Lifetime,
            typeTable,
            this->module->SymbolTable,
            signatures[i],
            Phx::Types::CallingConventionKind::CDecl,
            Phx::Symbols::Visibility::GlobalReference,
            argumentLists[i],
            isEllipsis[i],
            returnValues[i]
         );      
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds the function symbols for the 'display' family of
//    built-in procedures.
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
NativeRuntime::BuildDisplayFunctionSymbols()
{
   displayFunctionSymbols =
      gcnew Dictionary<String ^, Phx::Symbols::FunctionSymbol ^>();
  
   Phx::Types::Table ^ typeTable = this->module->TypeTable;

   // Build the function type.
   
   Phx::Types::Type ^ voidType = typeTable->VoidType;
   Phx::Types::Type ^ ptrToVoidType = Phx::Types::PointerType::New(
      typeTable,
      Phx::Types::PointerTypeKind::UnmanagedPointer, 
      typeTable->NativePointerBitSize,
      voidType, 
      nullptr
   );
   Phx::Types::Type ^ int32Type = typeTable->Int32Type;
   Phx::Types::Type ^ ptrToCharType = Phx::Types::PointerType::New(
      typeTable,
      Phx::Types::PointerTypeKind::UnmanagedPointer, 
      typeTable->NativePointerBitSize,
      typeTable->Int8Type, 
      nullptr
   );

   array<String ^> ^ signatures = {
      "_display_enter",
      "_display_leave",
      "_display_get_address",
   };
   
   array<array<Phx::Types::Type ^> ^> ^ argumentLists = {
      {ptrToCharType, int32Type, int32Type, int32Type},
      {int32Type, int32Type},
      {int32Type, int32Type, int32Type},
   };

   array<bool> ^ isEllipsis = {
      true, false, false,
   };
   
   array<Phx::Types::Type ^> ^ returnValues = {
      voidType, voidType,
      ptrToVoidType,
   };

   for (int i = 0; i < signatures->Length; ++i)
   {
      // Add the function symbol to the utility function symbol map.
      // The string name is adjusted to remove the initial underscore character.

      displayFunctionSymbols[signatures[i]->Substring(1)] = 
         ModuleBuilder::BuildFunctionSymbol(
            typeTable->Lifetime,
            typeTable,
            this->module->SymbolTable,
            signatures[i],
            Phx::Types::CallingConventionKind::CDecl,
            Phx::Symbols::Visibility::GlobalReference,
            argumentLists[i],
            isEllipsis[i],
            returnValues[i]
         );
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds the function symbols for the 'math' family of
//    built-in procedures.
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
NativeRuntime::BuildMathFunctionSymbols()
{
   mathFunctionSymbols =
      gcnew Dictionary<String ^, Phx::Symbols::FunctionSymbol ^>();
   
   Phx::Types::Table ^ typeTable = this->module->TypeTable;

   // Build the function type.

   Phx::Types::Type ^ int32Type = typeTable->Int32Type;
   Phx::Types::Type ^ doubleType = typeTable->Float64Type;
   Phx::Types::Type ^ voidType = typeTable->VoidType;

   array<String ^> ^ signatures = {
      "_pow",
      "_abs",
      "_fabs",
      "_sin",
      "_cos",
      "_atan",
      "_odd",
      "_mod",
      "_sqr",
      "_sqrf",
      "_trunc",
      "_round",
      "_exp",
      "_log",
      "_sqrt",
   };   
   
   array<array<Phx::Types::Type ^> ^> ^ argumentLists = {
      {doubleType, doubleType},
      {int32Type},
      {doubleType},
      {doubleType},
      {doubleType},
      {doubleType},
      {int32Type},
      {int32Type, int32Type},
      {int32Type},
      {doubleType},
      {doubleType},
      {doubleType},
      {doubleType},
      {doubleType},
      {doubleType},
   };

   array<bool> ^ isEllipsis = {
      false, false, false, false, false, false,
      false, false, false, false, false, false,
      false, false, false,
   };
   
   array<Phx::Types::Type ^> ^ returnValues = {
      doubleType, int32Type,
      doubleType, doubleType, doubleType, doubleType,
      int32Type, int32Type, int32Type, doubleType,
      int32Type, int32Type, 
      doubleType, doubleType, doubleType,
   };

   for (int i = 0; i < signatures->Length; ++i)
   {
      // Add the function symbol to the set function symbol map.
      // The string name is adjusted to remove the initial underscore character.

      mathFunctionSymbols[signatures[i]->Substring(1)] = 
         ModuleBuilder::BuildFunctionSymbol(
            typeTable->Lifetime,
            typeTable,
            this->module->SymbolTable,
            signatures[i],
            Phx::Types::CallingConventionKind::CDecl,
            Phx::Symbols::Visibility::GlobalReference,
            argumentLists[i],
            isEllipsis[i],
            returnValues[i]
         );      
   }

   mathFunctions = gcnew List<String ^>();
   mathFunctions->Add("abs");
   mathFunctions->Add("sin");
   mathFunctions->Add("cos");
   mathFunctions->Add("arctan");
   mathFunctions->Add("odd");
   mathFunctions->Add("mod");
   mathFunctions->Add("sqr");
   mathFunctions->Add("trunc");
   mathFunctions->Add("round");
   mathFunctions->Add("exp");
   mathFunctions->Add("ln");
   mathFunctions->Add("sqrt");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds the function symbols for the 'transfer' family of
//    built-in procedures.
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
NativeRuntime::BuildTransferFunctionSymbols()
{
   transferFunctionSymbols =
      gcnew Dictionary<String ^, Phx::Symbols::FunctionSymbol ^>();
  
   Phx::Types::Table ^ typeTable = this->module->TypeTable;

   // Build the function type.

   Phx::Types::Type ^ int32Type = typeTable->Int32Type;
   Phx::Types::Type ^ charType = typeTable->Int8Type;
   Phx::Types::Type ^ voidType = typeTable->VoidType;
   Phx::Types::Type ^ ptrToVoidType = Phx::Types::PointerType::New(
      typeTable,
      Phx::Types::PointerTypeKind::UnmanagedPointer, 
      typeTable->NativePointerBitSize,
      voidType, 
      nullptr
   );

   array<String ^> ^ signatures = {
      "_ord",
      "_chr",
      "_pred",
      "_succ",
      "_pack",
      "_unpack",
   };   
   
   array<array<Phx::Types::Type ^> ^> ^ argumentLists = {
      {int32Type},
      {int32Type},
      {int32Type},
      {int32Type},
      {ptrToVoidType, ptrToVoidType, int32Type, int32Type},
      {ptrToVoidType, ptrToVoidType, int32Type, int32Type},
   };

   array<bool> ^ isEllipsis = {
      false, false, false, false, false, false,
   };
   
   array<Phx::Types::Type ^> ^ returnValues = {
      int32Type, charType, int32Type, int32Type,
      voidType, voidType,
   };

   for (int i = 0; i < signatures->Length; ++i)
   {
      // Add the function symbol to the set function symbol map.
      // The string name is adjusted to remove the initial underscore character.

      transferFunctionSymbols[signatures[i]->Substring(1)] = 
         ModuleBuilder::BuildFunctionSymbol(
            typeTable->Lifetime,
            typeTable,
            this->module->SymbolTable,
            signatures[i],
            Phx::Types::CallingConventionKind::CDecl,
            Phx::Symbols::Visibility::GlobalReference,
            argumentLists[i],
            isEllipsis[i],
            returnValues[i]
         );      
   }

   transferFunctions = gcnew List<String ^>();
   transferFunctions->Add("ord");
   transferFunctions->Add("chr");
   transferFunctions->Add("pred");
   transferFunctions->Add("succ");
   transferFunctions->Add("pack");
   transferFunctions->Add("unpack");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Initializes the built-in 'input' and 'output' file handles.
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
NativeRuntime::InitializeStandardHandles
(
   Phx::FunctionUnit ^ functionUnit,
   int sourceLineNumber
)
{
   // Create a 256-byte primitive type to back the file type.
   // This will be large enough to hold the maximum-length string.
   // (120 characters)

   Phx::Types::Type ^ componentType = 
   functionUnit->TypeTable->GetPrimitiveType(
      (int) Phx::Types::PrimitiveTypeKind::Unknown,
      256 * functionUnit->TypeTable->Character8Type->BitSize
   );
   
   // Register the file type '$stdfile'.

   Phx::Types::Type ^ fileType = TypeBuilder::RegisterFileType(
      functionUnit->TypeTable,
      functionUnit->SymbolTable,
      "$stdfile",
      componentType,
      sourceLineNumber
   );
   
   unsigned char * initialValue = new unsigned char[fileType->ByteSize];

   // Create standard input handle.

   standardInputSymbol = ModuleBuilder::AddGlobalVariable(
      this->module,
      "input",
      fileType,
      initialValue,
      fileType->ByteSize
   );

   // Create a local proxy.

   Phx::Symbols::NonLocalVariableSymbol ^ localSymbol =
      ModuleBuilder::MakeProxyInFunctionSymbolTable(
         standardInputSymbol, 
         functionUnit->SymbolTable
      );

   // Load the $runtime_handle field from memory.

   Phx::Symbols::FieldSymbol ^ handleFieldSymbol = 
      TypeBuilder::FindFieldSymbol(
         fileType->AsAggregateType,
         "$runtime_handle"
      );

   Phx::IR::MemoryOperand ^ destinationOperand = 
      Phx::IR::MemoryOperand::New(
         functionUnit,
         handleFieldSymbol->Type,
         localSymbol,
         nullptr,
         0, 
         Phx::Alignment::NaturalAlignment(handleFieldSymbol->Type),
         functionUnit->AliasInfo->IndirectAliasedMemoryTag,
         functionUnit->SafetyInfo->SafeTag
      );

   // Call into the runtime to generate the standard input file handle.

   Phx::IR::Instruction ^ callInstruction = CallFileFunction(
      functionUnit,
      "file_get_std_input",
      gcnew List<Phx::IR::Operand ^>(),
      sourceLineNumber
   );

   // The CallFileFunction method will have appended the Call instruction
   // to the end of the function unit. We actually want this to happen first,
   // so we unlink the instruction and insert it after the enter
   // instruction.

   callInstruction->Unlink();
   functionUnit->FirstEnterInstruction->InsertAfter(callInstruction);

   // Assign the standard input file to the input.$runtime_handle field.

   Phx::IR::Instruction ^ assignInstruction = 
      Phx::IR::ValueInstruction::NewUnary(
         functionUnit,
         Phx::Common::Opcode::Assign,
         destinationOperand,
         callInstruction->DestinationOperand
      );

   // Append the instruction to the IR stream.

   callInstruction->InsertAfter(assignInstruction);

   // Create standard output handle.

   standardOutputSymbol = ModuleBuilder::AddGlobalVariable(
      this->module,
      "output",
      fileType,
      initialValue,
      fileType->ByteSize
   );

   // Create a local proxy.

   localSymbol =
      ModuleBuilder::MakeProxyInFunctionSymbolTable(
         standardOutputSymbol, 
         functionUnit->SymbolTable
      );

   // Load the $runtime_handle field from memory.

   destinationOperand = Phx::IR::MemoryOperand::New(
      functionUnit,
      handleFieldSymbol->Type,
      localSymbol,
      nullptr,
      0, 
      Phx::Alignment::NaturalAlignment(handleFieldSymbol->Type),
      functionUnit->AliasInfo->IndirectAliasedMemoryTag,
      functionUnit->SafetyInfo->SafeTag
   );

   // Call into the runtime to generate the standard output file handle.

   callInstruction = CallFileFunction(
      functionUnit,
      "file_get_std_output",
      gcnew List<Phx::IR::Operand ^>(),
      sourceLineNumber
   );

   // Like we did for the standard input handle, unlink the instruction 
   // and insert it after the enter instruction.

   callInstruction->Unlink();
   assignInstruction->InsertAfter(callInstruction);

   assignInstruction = Phx::IR::ValueInstruction::NewUnary(
      functionUnit,
      Phx::Common::Opcode::Assign,
      destinationOperand,
      callInstruction->DestinationOperand
   );
 
   callInstruction->InsertAfter(assignInstruction);

   delete[] initialValue;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a call to the standard read procedure.
//
// Remarks:
//
//    In Pascal, read(f, x) is equivalent to:
//    x := f^; get(f)
//
//    For read(f, x0, x1, ..., xn) we repeat the process n times:
//    x0 := f^; get(f)
//    x1 := f^; get(f)
//    ...
//    xn := f^; get(f)
//
// Returns:
//
//    The last instruction emitted for the read call.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^ 
NativeRuntime::CallFileRead
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ functionName,
   List<Phx::IR::Operand ^> ^ arguments,
   int sourceLineNumber
)
{   
   // read(x) is equivalent to read(input, x). 
   // We'll need to determine whether the first argument is a file
   // specifier or a value.

   int firstArgument;

   Phx::IR::Instruction ^ lastInstruction = nullptr;
   Phx::IR::Operand ^ fileHandleOperand;
   Phx::Types::Table ^ typeTable = functionUnit->TypeTable;
   
   // Set the file handle to the 'input' file if no other file was specified.

   if (arguments->Count == 0 || !TypeBuilder::IsFileType(arguments[0]->Type))
   {
      // Ensure 'input' has been declared in the program identifier list.

      if (! ModuleBuilder::IsProgramParameter("input"))
      {
         Output::ReportError(
            sourceLineNumber,
            Error::UndeclaredProgramParameter,
            "input"
         );
      }

      // Create a local proxy.

      Phx::Symbols::NonLocalVariableSymbol ^ localSymbol =
         ModuleBuilder::MakeProxyInFunctionSymbolTable(
            standardInputSymbol,
            functionUnit->SymbolTable
         );

      fileHandleOperand = Phx::IR::VariableOperand::New(
         functionUnit,
         localSymbol->Type,
         localSymbol
      );

      firstArgument = 0;
   }
   else
   {
      fileHandleOperand = arguments[0];
      firstArgument = 1;
   }

   Phx::Types::Type ^ fileType = fileHandleOperand->Type;
   Phx::Symbols::Symbol ^ fileSymbol = fileHandleOperand->Symbol;

   bool isUserFile = ! IsRuntimeFileSymbol(fileHandleOperand->Symbol);
   
   List<Phx::IR::Operand ^> ^ argumentList = 
      gcnew List<Phx::IR::Operand ^>();

   // Obtain the value operand mapped to the provided file.

   Phx::Symbols::Symbol ^ valueSymbol;
   Phx::IR::Operand ^ valueOperand;

   if (isUserFile)
   {
      // Load the allocated file handle into the file.$current_value field.
   
      Phx::Symbols::FieldSymbol ^ fieldSymbol = TypeBuilder::FindFieldSymbol(
         fileType->AsAggregateType,
         "$current_value"
      );

      valueSymbol = fieldSymbol;

      if (fileHandleOperand->IsTemporary)
      {
         valueOperand = Phx::IR::VariableOperand::NewExpressionTemporary(
            functionUnit,
            fieldSymbol->Field,
            fileHandleOperand->Id
         );
      }
      else if (fileHandleOperand->Symbol)
      {
         valueOperand = Phx::IR::VariableOperand::New(
            functionUnit,
            fieldSymbol->Field,
            fileHandleOperand->Symbol
         );
      }      
      else
      {
         Debug::Assert(fileHandleOperand->IsMemoryOperand);

         valueOperand = Phx::IR::MemoryOperand::New(
            functionUnit,
            fieldSymbol->Field,
            nullptr,
            fileHandleOperand->BaseOperand,
            0,
            Phx::Alignment::NaturalAlignment(fieldSymbol->Field->Type),
            functionUnit->AliasInfo->IndirectAliasedMemoryTag,
            functionUnit->SafetyInfo->SafeTag
         );
      }
   }
      
   for (int i = firstArgument; i < arguments->Count; i++)
   {
      Phx::IR::Operand ^ argument = arguments[i];
      Phx::Types::Type ^ argumentType = argument->Type;

      Phx::Types::Type ^ componentType = 
         TypeBuilder::GetFileComponentType(
            fileHandleOperand->Type
         );

      // For the standard 'input' and 'output' files, we accept any allowable
      // file type. For other files, each argument must match the user 
      // file type.

      if (isUserFile)
      {
         if (!argumentType->Equals(componentType))
         {
            Output::ReportError(
               sourceLineNumber,
               Error::InvalidArgument,
               functionName, 
               TypeBuilder::GetSourceType(componentType), 
               i+1
            );
            continue;
         }
      }
      else
      {
         Phx::Symbols::FieldSymbol ^ fieldSymbol = TypeBuilder::FindFieldSymbol(
            fileType->AsAggregateType,
            "$current_value"
         );
         
         valueSymbol = fieldSymbol;

         int byteOffset = Phx::Utility::BitsToBytes(fieldSymbol->BitOffset);

         if (! valueSymbol->Type->Equals(argumentType))
         {
            TypeBuilder::ChangeFieldType(
               fileType->AsAggregateType,
               "$current_value",
               argumentType
            );
         }

         Phx::IR::Operand ^ baseOperand = 
            fileHandleOperand->CopyToAddressOperand();

         valueOperand = Phx::IR::MemoryOperand::New(
            functionUnit,
            argumentType,
            nullptr,
            baseOperand->AsVariableOperand,
            byteOffset,
            Phx::Alignment::NaturalAlignment(argumentType),
            functionUnit->AliasInfo->IndirectAliasedMemoryTag,
            functionUnit->SafetyInfo->SafeTag
         );        
      }

      int opOrder[2];
      if (isUserFile)
      {
         opOrder[0] = 0;
         opOrder[1] = 1;
      }
      else
      {
         opOrder[0] = 1;
         opOrder[1] = 0;
      }

      for (int j = 0; j < 2; j++)
      {
         if (opOrder[j] == 0)
         {
            // Perform the x := f^ part first.
            
            IRBuilder::EmitAssign(
               functionUnit,
               argument,
               valueOperand,
               sourceLineNumber
               );        
         }
         else
         {
            // Now call get(f).

            argumentList->Clear();
            argumentList->Add(fileHandleOperand);

            lastInstruction = CallFileFunction(
               functionUnit,
               "get",
               argumentList,
               sourceLineNumber
            );
         }
      }

      // If the user is calling readln, eat all available whitespace
      // until we reach eof or the next non-whitespace character.

      if (functionName->ToLower()->EndsWith("ln"))
      {
         argumentList->Clear();
         argumentList->Add(fileHandleOperand);

         CallFileFunction(
            functionUnit,
            "file_eat_white",
            argumentList,
            sourceLineNumber
         );
      }
   }

   return lastInstruction;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a call to the standard write procedure.
//
// Remarks:
//
//    In Pascal, write(f, x) is equivalent to:
//    f^ := x; put(f)
//
//    For write(f, x0, x1, ..., xn) we repeat the process n times:
//    f^ := x0; put(f)
//    f^ := x1; put(f)
//    ...
//    f^ := xn; put(f)
//
// Returns:
//
//    The last instruction emitted for the write call.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^ 
NativeRuntime::CallFileWrite
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ functionName,
   List<Phx::IR::Operand ^> ^ arguments,
   int sourceLineNumber
)
{   
   // write(x) is equivalent to write(output, x). 
   // We'll need to determine whether the first argument is a file
   // specifier or a value.

   int firstArgument;

   Phx::IR::Instruction ^ lastInstruction = nullptr;
   Phx::IR::Operand ^ fileHandleOperand;
   Phx::Types::Table ^ typeTable = functionUnit->TypeTable;
   
   // Set the file handle to the 'output' file if no other file was specified.

   if (arguments->Count > 0 && TypeBuilder::IsFileType(arguments[0]->Type))
   {
      fileHandleOperand = arguments[0];
      firstArgument = 1;
   }
   else
   {
      // Ensure 'output' has been declared in the program identifier list.

      if (! ModuleBuilder::IsProgramParameter("output"))
      {
         Output::ReportError(
            sourceLineNumber,
            Error::UndeclaredProgramParameter,
            "output"
         );
      }

      // Create local proxy.

      Phx::Symbols::NonLocalVariableSymbol ^ localSymbol =
         ModuleBuilder::MakeProxyInFunctionSymbolTable(
            standardOutputSymbol, 
            functionUnit->SymbolTable
         );

      fileHandleOperand = Phx::IR::VariableOperand::New(
         functionUnit,
         localSymbol->Type,
         localSymbol
      );

      firstArgument = 0;
   }

   Phx::Types::Type ^ fileType = fileHandleOperand->Type;
   Phx::Symbols::Symbol ^ fileSymbol = fileHandleOperand->Symbol;

   bool isUserFile = ! IsRuntimeFileSymbol(fileHandleOperand->Symbol);

   List<Phx::IR::Operand ^> ^ argumentList = 
      gcnew List<Phx::IR::Operand ^>();

   // Obtain the value operand mapped to the provided file.

   Phx::Symbols::Symbol ^ valueSymbol;
   Phx::IR::Operand ^ valueOperand;
   
   if (isUserFile)
   {
      // Load the allocated file handle into the file.$current_value field.
   
      Phx::Symbols::FieldSymbol ^ fieldSymbol = TypeBuilder::FindFieldSymbol(
         fileType->AsAggregateType,
         "$current_value"
      );

      valueSymbol = fieldSymbol;

      valueOperand = Phx::IR::VariableOperand::New(
         functionUnit,
         fieldSymbol->Field,
         fileSymbol
      );
   }

   // If the user is calling writeln, append control and 
   // line feed characters.

   if (functionName->ToLower()->EndsWith("ln"))
   {
      arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int8Type,
         (__int8) '\r')
      );
      arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int8Type,
         (__int8) '\n')
      );
   }
      
   // Process the argument list.

   for (int i = firstArgument; i < arguments->Count; i++)
   {
      Phx::IR::Operand ^ argument = arguments[i];

      // If the argument is a function call, first emit the call
      // and replace the current argument with the result of 
      // the call.

      if (argument->Type->IsFunctionType)
      {
         Phx::IR::Instruction ^ callInstruction = 
            IRBuilder::EmitCall(
               functionUnit,      
               argument->Symbol->NameString,
               gcnew List<Phx::IR::Operand ^>(),
               sourceLineNumber
            );
         argument = callInstruction->DestinationOperand;
      }

      // Check whether a width specifier was provided for the current operand.

      ArgumentModifier ^ modifier = 
         IRBuilder::FindArgumentModifier(argument);

      // If a modifier was provided, apply it to the current procedure call.

      if (modifier)
      {
         List<Phx::IR::Operand ^> ^ argumentList = 
            gcnew List<Phx::IR::Operand ^>();

         Debug::Assert(modifier->MinimumFieldWidth != nullptr);

         //hFile
         argumentList->Add(fileHandleOperand);
         //which
         argumentList->Add(Phx::IR::ImmediateOperand::New(
               functionUnit,
               functionUnit->TypeTable->Int32Type,
               (int) 0
            )
         );
         //modifier
         argumentList->Add(modifier->MinimumFieldWidth);

         CallFileFunction(
            functionUnit,
            "file_set_modifier",
            argumentList,
            sourceLineNumber
         );

         if (modifier->FractionLength)
         {
            // The second width specifier is only valid for type Real.

            if (! argument->Type->Equals(
               TypeBuilder::GetTargetType(NativeType::Real)))
            {
               Output::ReportError(
                  sourceLineNumber,
                  Error::InvalidWidthSpecifier
               );
            }

            // Remove second two arguments.

            argumentList->RemoveRange(argumentList->Count-2, 2);

            //which
            argumentList->Add(Phx::IR::ImmediateOperand::New(
                  functionUnit,
                  functionUnit->TypeTable->Int32Type,
                  (int) 1
               )
            );
            //modifier
            argumentList->Add(modifier->FractionLength);

            CallFileFunction(
               functionUnit,
               "file_set_modifier",
               argumentList,
               sourceLineNumber
            );
         }

         // Remove the argument modifier from the list.

         IRBuilder::ArgumentModifierList->Remove(modifier);
      }

      Phx::Types::Type ^ argumentType = argument->Type;

      Phx::Types::Type ^ componentType = isUserFile ? 
         TypeBuilder::GetFileComponentType(fileHandleOperand->Type) :
         argumentType;

      // For the standard 'input' and 'output' files, we accept any allowable
      // file type. For other files, each argument must match the user 
      // file type.

      if (isUserFile)
      {
         if (! argumentType->Equals(componentType))
         {
            Output::ReportError(
               sourceLineNumber,
               Error::InvalidArgument,
               functionName, 
               TypeBuilder::GetSourceType(componentType), i+1
            );
            continue;
         }
      }
      else
      {
         Phx::Symbols::FieldSymbol ^ fieldSymbol = TypeBuilder::FindFieldSymbol(
            fileType->AsAggregateType,
            "$current_value"
         );

         valueSymbol = fieldSymbol;
         int byteOffset = Phx::Utility::BitsToBytes(fieldSymbol->BitOffset);

         if (! valueSymbol->Type->Equals(argumentType))
         {
            TypeBuilder::ChangeFieldType(
               fileType->AsAggregateType,
               "$current_value",
               argumentType
            );            
         }
        
         Phx::IR::Operand ^ baseOperand = 
            fileHandleOperand->CopyToAddressOperand();
        
         valueOperand = Phx::IR::MemoryOperand::New(
            functionUnit,
            argumentType,
            nullptr,
            baseOperand->AsVariableOperand,
            byteOffset,
            Phx::Alignment::NaturalAlignment(argumentType),
            functionUnit->AliasInfo->IndirectAliasedMemoryTag,
            functionUnit->SafetyInfo->SafeTag
         );         
      }

      // Perform the f^ := x part first.
      
      IRBuilder::EmitAssign(
         functionUnit,
         valueOperand,
         argument,
         sourceLineNumber
      );

      // Now call put(f).

      argumentList->Clear();
      argumentList->Add(fileHandleOperand);

      lastInstruction = CallFileFunction(
         functionUnit,
         "put",
         argumentList,
         sourceLineNumber
      );
   }

   return lastInstruction;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Emits a call to the given function with the provided argument list.
//
// Remarks:
//
//
// Returns:
//
//    The generated Call instruction.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^
NativeRuntime::CallFunction
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Symbols::FunctionSymbol ^ functionSymbol,
   List<Phx::IR::Operand ^ > ^ arguments,
   bool checkArgumentCount,
   Phx::IR::Instruction ^ insertAfterInstruction,
   int sourceLineNumber
)
{
   Debug::Assert(functionSymbol != nullptr);

   if (checkArgumentCount)
   {
      // Validate argument count.

      Debug::Assert(functionSymbol->Type->AsFunctionType->
         ParametersForInstruction.Count == arguments->Count);
   }

   // First, create a new Call instruction.

   Phx::IR::Instruction ^ callInstruction = 
      Phx::IR::CallInstruction::New(
         functionUnit,
         Phx::Common::Opcode::Call,
         functionSymbol
      );
  
   // Append source arguments.

   for each (Phx::IR::Operand ^ argument in arguments)
      callInstruction->AppendSource(argument);

   // Generate an append a result operand if the function is non-void.

   if (! functionSymbol->Type->AsFunctionType->ReturnType->IsVoid)
   {
      Phx::IR::Operand ^ result = Phx::IR::VariableOperand::NewExpressionTemporary(
         functionUnit,
         functionSymbol->Type->AsFunctionType->ReturnType
      );

      callInstruction->AppendDestination(result);
   }
   
   insertAfterInstruction->InsertAfter(callInstruction);

   return callInstruction;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given function name refers to a 'string' function.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool 
NativeRuntime::IsStringFunction
(
   String ^ functionName
)
{
   return stringFunctions &&
          stringFunctions->Contains(functionName->ToLower());
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given function name refers to a 'file' function.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool 
NativeRuntime::IsFileFunction
(
   String ^ functionName
)
{
   return fileFunctions && 
          fileFunctions->Contains(functionName->ToLower());
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Translates the given function name to the appropriate runtime library
//    function name.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

String ^
NativeRuntime::TranslateFileFunctionName
(
   String ^ userName
)
{
   // For 'put' and 'get' functions, we don't know which function is 
   // being referenced because we don't know the parameter type.
   // For the purposes of users of this call, they are only interested
   // in the return value, which is the same across all put/get functions.

   if (userName->Equals("put"))
      return "file_put_char";
   else if (userName->Equals("get"))
      return "file_get_char";

   else if (userName->Equals("reset"))
      return "file_reset";
   else if (userName->Equals("rewrite"))
      return "file_rewrite";
   else if (userName->Equals("page"))
      return "file_page";
   else if (userName->Equals("eof"))
      return "file_eof";
   else if (userName->Equals("eoln"))
      return "file_eoln";
   else 
      // This is an internal runtime call; maintain the same name.
      return userName;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given function name refers to an
//    'allocation' function.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool 
NativeRuntime::IsAllocationFunction
(
   String ^ functionName
)
{
   return allocFunctions && 
          allocFunctions->Contains(functionName->ToLower());
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given function name refers to a
//    'math' function.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool 
NativeRuntime::IsMathFunction
(
   String ^ functionName
)
{
   return mathFunctions && 
          mathFunctions->Contains(functionName->ToLower());
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Translates the given function name to the appropriate runtime library
//    function name.
//
// Remarks:
//
//    We need the argument list so that we can resolve the proper function 
//    name. For example, the 'abs' function resolves to 'fabs' if the 
//    argument is floating point; otherwise, it resolves to 'abs' for 
//    integers.
//
// Returns:
//
//
//-----------------------------------------------------------------------------

String ^ 
NativeRuntime::TranslateMathFunctionName
(
   String ^ userName, 
   List<Phx::IR::Operand ^ > ^ arguments
)
{
   userName = userName->ToLower();

   if (userName->Equals("abs"))
   {
      if (arguments[0]->Type->IsFloat)
      {
         return "fabs";
      }
      Debug::Assert(arguments[0]->Type->IsInt);
   }
   else if (userName->Equals("sqr"))
   {
      if (arguments[0]->Type->IsFloat)
      {
         return "sqrf";
      }
      Debug::Assert(arguments[0]->Type->IsInt);
   }
   else if (userName->Equals("arctan"))
   {
      return "atan";
   }
   else if (userName->Equals("ln"))
   {
      return "log";
   }

   return userName;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given function name refers to a
//    'transfer' function.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool 
NativeRuntime::IsTransferFunction
(
   String ^ functionName
)
{
   return transferFunctions && 
          transferFunctions->Contains(functionName->ToLower());
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Creates a new IRuntimeLibrary object that can communicate with
//    the native runtime library.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

IRuntimeLibrary ^
IRuntimeLibrary::NewNativeRuntime
(
   Phx::ModuleUnit ^ moduleUnit
)
{
   return gcnew NativeRuntime(moduleUnit);
}

} // namespace Pascal
