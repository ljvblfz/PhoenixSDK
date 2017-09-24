//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Implementation of the FunctionUnitData and ModuleBuilder classes.
//
// Remarks:
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include <memory.h>

using namespace System;
using namespace System::IO;
using namespace System::Diagnostics;

namespace Pascal 
{

//-----------------------------------------------------------------------------
//
// Description: This class is provided for storing side data related to 
//              FunctionUnit construction, but specific to the Pascal language.
//
// Remarks:
//
//
//-----------------------------------------------------------------------------

ref class FunctionUnitData
{
public:

   FunctionUnitData
   (
      String ^ functionName,
      Phx::FunctionUnit ^ unit,
      FunctionUnitData ^ parent,
      array<FormalParameter ^> ^ formalParameters,
      DirectiveType directive
   )
      : labelDeclarationList(gcnew List<int>())
      , labelDefinitionMap(gcnew Dictionary<int, 
            Phx::IR::LabelInstruction ^>())
      , labelReferenceMap(gcnew Dictionary<int, int>())
      , labelFixupList(gcnew List<LabelFixup ^>())
   {
      Name = functionName;
      Unit = unit;
      Parent = parent;
      FormalParameters = formalParameters;
      Directive = directive;

      SourceFileName = ModuleBuilder::SourceFileName;
      SourceFileId = ModuleBuilder::SourceFileIndex;

      ChildList = gcnew List<FunctionUnitData ^>();
      ConstantSymbolList = gcnew List<Phx::Symbols::Symbol ^>();      
      ConstantSymbolValues = gcnew Dictionary<Phx::Symbols::Symbol ^, 
         ConstantValue ^>();
      SetSymbols = gcnew Dictionary<Phx::Symbols::Symbol ^, 
         Phx::Types::Type ^>();
      SetRelaseSymbols = gcnew List<Phx::Symbols::Symbol ^>();
      LocalSymbols = gcnew List<Phx::Symbols::Symbol ^>();
      NextTemporarySymbolId = 100;      
   }

public: // properties

   // The cached name of the function for fast lookup.
   property String ^ Name;
   
   // The FunctionUnit associated with this data node.
   property Phx::FunctionUnit ^ Unit;
   
   // The list of child procedures.
   property List<FunctionUnitData ^> ^ ChildList;
   
   // The parent of this data node.
   property FunctionUnitData ^ Parent;

   // The list of constants local to this function.
   property List<Phx::Symbols::Symbol ^> ^ ConstantSymbolList;
   
   // Maps symbols to their compile-time constant values.
   property Dictionary<Phx::Symbols::Symbol ^, 
      ConstantValue ^> ^ ConstantSymbolValues;

   // Maps set symbols to their compile-time type.
   property Dictionary<Phx::Symbols::Symbol ^, 
      Phx::Types::Type ^> ^ SetSymbols;

   // List of set symbols that will be released at the end of 
   // the function's execution.
   property List<Phx::Symbols::Symbol ^> ^ SetRelaseSymbols;

   // Cached list of symbols local to this function for fast lookup.
   property List<Phx::Symbols::Symbol ^> ^ LocalSymbols;

   // The file name the function is defined in.
   property String ^ SourceFileName;

   // A unique identifier associated with the source file the function
   // is defined in.
   property int SourceFileId;

   // Counter for generating unique symbol names.
   property int NextTemporarySymbolId;

   // The function directive.
   property DirectiveType Directive;

   // Caches formal parameters to this function for fast lookup.
   property array<FormalParameter ^> ^ FormalParameters;

   // The symbol for the return value from this function.
   property Phx::Symbols::Symbol ^ ReturnSymbol;

   // Numeric counter for generating temporary instructions.
   property static int CurrentDummyInstructionId;

   property array<int> ^ LabelDeclarations
   {
      // Retrieves the array of declared labels.

      array<int> ^ get();
   }

public: // methods

   // Adds the given label to the declaration list.

   bool AddLabelDeclaration(int label);

   // Determines whether the given label has been declared.

   bool IsLabelDeclared(int label);

   // Adds a reference to the given label.

   int AddLabelReference(int label);

   // Retrieves the reference count for the given label.

   int GetLabelReferenceCount(int label);

   // Adds an entry to the goto fixup list for the associated function unit.

   void
   AddGotoLabelFixup
   (
      int label, 
      Phx::IR::Instruction ^ replaceThisInstruction,
      int sourceLineNumber
   );

   // Retrieves the label instruction associated with the given label.

   Phx::IR::LabelInstruction ^ 
   GetLabelInstruction
   (
      int label,
      int sourceLineNumber
   );

   // Performs fixups on the goto fixup list.

   int
   FixupGotoLabels();

   // Appends a label instruction to the function unit.

   void BuildLabel
   (
      int label,
      int sourceLineNumber
   );

   // Retrives the numeric index of the given symbol.

   int GetLocalSymbolIndex
   (
      Phx::Symbols::Symbol ^ symbol
   );

private: // methods

   // Adds a label/instruction mapping to the label definition map.

   bool AddLabelDefinition
   (
      int label, 
      Phx::IR::LabelInstruction ^ instruction
   );
   
private: // data

   // List of all label declarations.
   List<int> ^ labelDeclarationList;

   // Maps label identifiers to label instructions.
   Dictionary<int, Phx::IR::LabelInstruction ^> ^ labelDefinitionMap;

   // Counts references made to labels.
   Dictionary<int, int> ^ labelReferenceMap;   
   
   // Describes a single label fixup.
   ref struct LabelFixup
   {
      property int Label;
      property Phx::IR::Instruction ^ ReplaceThisInstruction;
      property int SourceLineNumber;
   };

   // List of label fixups.
   List<LabelFixup ^> ^ labelFixupList;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    Resets state information between compilation units.
//
// Remarks:
//
//    Call this method prior to processing each source file to reset
//    state information.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void 
ModuleBuilder::NewContext()
{
   // Reinitialize all persistent data.

   externalIdCounter = 100;

   stringTable = 
      gcnew Dictionary<String ^, Phx::Symbols::GlobalVariableSymbol ^>();

   headFunctionUnitData = nullptr;

   uniqueSymbolId = 100;

   scopeList = gcnew List<Phx::Symbols::Table ^>();

   StringSymbolLengths = gcnew Dictionary<unsigned int, int>();

   MainFunctionUnit = nullptr;

   functionUnitStack = gcnew List<Phx::FunctionUnit ^>();

   FunctionCounter = 0;

   FunctionUnitData::CurrentDummyInstructionId = 0;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Creates a new ModuleUnit object for the current compiland.
//
// Remarks:
//
//
// Returns:
//
//    A new ModuleUnit object.
//
//-----------------------------------------------------------------------------

Phx::ModuleUnit ^ 
ModuleBuilder::CreateModuleUnit
(
   String ^ sourceFileName
)
{   
   // Generate a file name for the current source file.

   String ^ targetFileName = Path::GetFileName(
      Path::ChangeExtension(sourceFileName, ".obj")
   );

   // Get the target runtime.

   Phx::Targets::Runtimes::Runtime ^ runtime =
      Phx::GlobalData::GetFirstTargetRuntime();

   // Create a module and a symbol table for this module.

   Phx::Types::Table ^ typeTable = Phx::GlobalData::TypeTable;

   Phx::Lifetime ^ lifetime = Phx::Lifetime::New(
      Phx::LifetimeKind::Module, 
      nullptr
   );

   Phx::Name moduleName = Phx::Name::New(
      lifetime, 
      targetFileName
   );

   Phx::ModuleUnit ^ moduleUnit = Phx::ModuleUnit::New(
      lifetime, 
      moduleName, 
      nullptr, 
      typeTable, 
      runtime->Architecture,
      runtime
   );

   // Add a symbol table and name map to the module.

   Phx::Symbols::Table::New(moduleUnit, 64, true);

   if (moduleUnit->SymbolTable->NameMap == nullptr)
   {
      moduleUnit->SymbolTable->AddMap(
         Phx::Symbols::NameMap::New(moduleUnit->SymbolTable, 64)
      );
   }

   // Create a new Pascal NativeRuntimeLibrary object for invoking
   // runtime functions.

   Runtime = IRuntimeLibrary::NewNativeRuntime(moduleUnit);

   // Initialize the runtime for this module.

   runtime->ModuleInitialize(moduleUnit);
   
   // Create a .strings section to hold string constants.

   CreateStringSection(moduleUnit);

   // Register global constants such as true, false, and maxint.

   InitializeGlobalConstants(moduleUnit);

   // Set the global Resolve object.

   Resolve = Phx::Types::Resolve::New(lifetime, typeTable);

   // Return the new module unit.

   return moduleUnit;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds a parameter to the list of program parameters for 
//    the entire program.
//
// Remarks:
//
//    A program parameter is either a persistent file that will be declared
//    or a forward usage declaration for the standard 'input' and 'output' 
//    files.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::AddProgramParameter
(
   String ^ parameterName,
   int sourceLineNumber
)
{
   // Because the standard 'input' and 'output' files are not
   // case-sensitive, we must do extra processing to ensure
   // they are not multiply declared.

   String ^ stdFileName = nullptr;
   if (parameterName->ToLower()->Equals("input") ||
       parameterName->ToLower()->Equals("output"))
   {
      stdFileName = parameterName->ToLower();
   }
   if (stdFileName)
   {
      for each (String ^ s in programParameters)
      {
         if (s->ToLower()->Equals(stdFileName))
         {
            goto LError;
         }
      }
   }

   // Add the parameter name if it doesn't already exist.
   // Otherwise, generate an error message.

   if (programParameters->Contains(parameterName))
   {
      goto LError;
   }
   else
   {
      programParameters->Add(parameterName);
      return;
   }
   

LError:

   Output::ReportError(
      sourceLineNumber,
      Error::ProgramParameterRedeclaration,
      parameterName
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the provided name has been declared
//    as a program parameter.
//
// Remarks:
//
//
// Returns:
//
//    true if the provided name has been declared as a program parameter;
//    false otherwise.
//
//-----------------------------------------------------------------------------

bool
ModuleBuilder::IsProgramParameter
(
   String ^ parameterName     
)
{
   // Because the standard 'input' and 'output' files are not
   // case-sensitive, we must do extra processing to check
   // whether they were defined.

   String ^ stdFileName = nullptr;
   if (parameterName->ToLower()->Equals("input") ||
       parameterName->ToLower()->Equals("output"))
   {
      stdFileName = parameterName->ToLower();
   }
   if (stdFileName)
   {
      for each (String ^ s in programParameters)
      {
         if (s->ToLower()->Equals(stdFileName))
         {
            return true;
         }
      }
   }

   return programParameters->Contains(parameterName);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds the given file name to the overall list of 
//    processed source files.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void 
ModuleBuilder::AddSourceFilePath
(
   String ^ sourceFilePath
)
{
   fileNames->Insert(0, sourceFilePath);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the current FunctionUnit object.
//
// Remarks:
//
//
// Returns:
//
//   The current FunctionUnit object.
//
//-----------------------------------------------------------------------------

Phx::FunctionUnit ^ 
ModuleBuilder::CurrentFunctionUnit::get()
{
   if (functionUnitStack == nullptr ||
       functionUnitStack->Count == 0)
   {
      return nullptr;
   }
   return functionUnitStack[0];
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the current (most recently added) source file name.
//
// Remarks:
//
//
// Returns:
//
//   The current source file name.
//
//-----------------------------------------------------------------------------

String ^
ModuleBuilder::SourceFileName::get()
{
   return fileNames[0];
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the index of the current source file name.
//
// Remarks:
//
//
// Returns:
//
//   The index of the current source file name.
//
//-----------------------------------------------------------------------------

int 
ModuleBuilder::SourceFileIndex::get()
{
   return fileNames->Count - 1;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds a new global variable symbol to the symbol table
//    associated with the given ModuleUnit object.
//
// Remarks:
//
//
// Returns:
//
//   A new GlobalVariableSymbol for the global variable.
//
//-----------------------------------------------------------------------------

Phx::Symbols::GlobalVariableSymbol ^ 
ModuleBuilder::AddGlobalVariable
(
   Phx::ModuleUnit ^ module,
   String ^ name,
   Phx::Types::Type ^ type,
   unsigned char * data,
   unsigned byteSize
)
{
   // Generate a GlobalVariableSymbol object for the global variable.

   Phx::Symbols::GlobalVariableSymbol ^ symbol = 
      Phx::Symbols::GlobalVariableSymbol::New(
         module->SymbolTable,
         externalIdCounter++,
         Phx::Name::New(module->Lifetime, name),
         type,
         Phx::Symbols::Visibility::GlobalDefinition
      );

   // Global variables exist in the .data section.

   symbol->AllocationBaseSectionSymbol = 
      module->SymbolTable->ExternIdMap->Lookup(
         (unsigned) Phx::Symbols::PredefinedCxxILId::Data
      )->AsSectionSymbol;

   // Generate an alignment for the symbol.

   symbol->Alignment = 
      Phx::Alignment::NaturalAlignment(symbol->Type);

   // Obtain the global section associated with the 
   // allocation section symbol.

   Phx::Section ^ globalSection = 
      symbol->AllocationBaseSectionSymbol->Section;

   // Create an initializer instruction.

   Phx::IR::DataInstruction ^ initializerInstruction = 
      Phx::IR::DataInstruction::New(
         globalSection->DataUnit, byteSize);

   // Write the raw byte data to the instruction.

   initializerInstruction->WriteBytes(0, data, byteSize);

   // Append the instruction to the .data section.

   globalSection->AppendInstruction(initializerInstruction);

   // Generate a data location for the symbol.

   symbol->Location = 
      Phx::Symbols::DataLocation::New(initializerInstruction);

   return symbol;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves a global variable symbol for the given
//    global string constant.
//
// Remarks:
//
//    This method returns an existing symbol if the global constant
//    is already registered; otherwise, it creates a new symbol
//    and adds it to the string table mapping.
//
// Returns:
//
//   The GlobalVariableSymbol for the given string value.
//
//-----------------------------------------------------------------------------

Phx::Symbols::GlobalVariableSymbol ^
ModuleBuilder::GetStringSymbol
(      
   Phx::ModuleUnit ^ moduleUnit,
   String ^ value
)
{
   Phx::Symbols::GlobalVariableSymbol ^ stringSymbol = nullptr;

   // Check for an existing string mapping.

   if (! stringTable->TryGetValue(value, stringSymbol))
   {
      // The mapping does not exist; create a new global symbol
      // and add it to the string table.

      stringSymbol = CreateInitializedString(moduleUnit, value);
      stringTable->Add(value, stringSymbol);
   }

   // Return the string symbol.

   return stringSymbol;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves a non-local variable symbol for the given
//    global string constant.
//
// Remarks:
//
//    Call this method to proxy a global string symbol to a 
//    local function unit.
//
// Returns:
//
//   The NonLocalVariableSymbol for the given string value.
//
//-----------------------------------------------------------------------------

Phx::Symbols::NonLocalVariableSymbol ^
ModuleBuilder::GetNonLocalStringSymbol
(      
   Phx::FunctionUnit ^ functionUnit,
   String ^ value
)
{
   // Call the GetStringSymbol method to either retrieve the existing
   // string symbol or create a new one.

   Phx::Symbols::GlobalVariableSymbol ^ stringSymbol = 
      GetStringSymbol(functionUnit->ParentModuleUnit, value);
   
   // Generate a proxy into the given function unit's symbol
   // table and return.

   return MakeProxyInFunctionSymbolTable(
      stringSymbol, 
      functionUnit->SymbolTable
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves a non-local variable symbol for the given
//    global variable.
//
// Remarks:
//
//    Call this method to proxy a global variable to a 
//    local function unit.
//
// Returns:
//
//   The NonLocalVariableSymbol for the global variable.
//
//-----------------------------------------------------------------------------

Phx::Symbols::NonLocalVariableSymbol ^
ModuleBuilder::MakeProxyInFunctionSymbolTable
( 
   Phx::Symbols::GlobalVariableSymbol ^ global,
   Phx::Symbols::Table ^ functionSymbolTable
)
{
   // Check whether a proxy already exists in the function's symbol table.

   Phx::Symbols::Symbol ^ symbol = 
      functionSymbolTable->ExternIdMap->Lookup(global->ExternId);
   if (symbol != nullptr)
      return symbol->AsNonLocalVariableSymbol;

   // Otherwise, create a new NonLocalVariableSymbol.

   return Phx::Symbols::NonLocalVariableSymbol::New(
      functionSymbolTable, 
      global
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the formal parameters associated with the given
//    function name.
//
// Remarks:
//
//
// Returns:
//
//   The array of FormalParameter objects associated with the given function.
//
//-----------------------------------------------------------------------------

array<FormalParameter ^> ^
ModuleBuilder::GetFormalParameters
(
   String ^ functionName
)
{
   // Lookup the data node associated with the function name and return
   // its formal parameter array, or return nullptr if the function 
   // does not exist.

   FunctionUnitData ^ data = FindFunctionUnitData(functionName);
   if (data)
      return data->FormalParameters;
   return nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the directive type associated with the given
//    function unit.
//
// Remarks:
//
//
// Returns:
//
//   The directive type associated with the given function unit.
//
//-----------------------------------------------------------------------------

DirectiveType 
ModuleBuilder::GetDirectiveType
(
   Phx::FunctionUnit ^ functionUnit
)
{
   FunctionUnitData ^ data = FindFunctionUnitData(functionUnit);
   if (data)
      return data->Directive;
   return DirectiveType::None;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Sets the directive type associated with the given
//    function unit.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::SetDirectiveType
(
   Phx::FunctionUnit ^ functionUnit,
   DirectiveType directive
)
{
   FunctionUnitData ^ data = FindFunctionUnitData(functionUnit);
   if (data)
      data->Directive = directive;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Appends a label instruction to the given function unit.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::BuildLabel
(
   Phx::FunctionUnit ^ functionUnit, 
   int label,
   int sourceLineNumber
)
{
   // Defer the call to the data node associated with the
   // function unit.

   FunctionUnitData ^ data = FindFunctionUnitData(functionUnit);
   if (data)
      data->BuildLabel(label, sourceLineNumber);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the label instruction associated with the
//    given label identifier in the provided function unit.
//
// Remarks:
//
//
// Returns:
//
//   The label instruction associated with the
//   given label identifier in the provided function unit.
//
//-----------------------------------------------------------------------------

Phx::IR::LabelInstruction ^ 
ModuleBuilder::GetLabelInstruction
(
   Phx::FunctionUnit ^ functionUnit, 
   int label,
   int sourceLineNumber
)
{
   // Retrieve the label instruction from the data node 
   // associated with the function unit.

   FunctionUnitData ^ data = FindFunctionUnitData(functionUnit);
   if (data)
      return data->GetLabelInstruction(label, sourceLineNumber);
   return nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given label has been declared
//    within the given function unit.
//
// Remarks:
//
//
// Returns:
//
//   true if the given label has been declared; false otherwise.
//
//-----------------------------------------------------------------------------

bool 
ModuleBuilder::IsLabelDeclared
(
   Phx::FunctionUnit ^ functionUnit, 
   int label
)
{
   // Defer the call to the data node associated with the
   // function unit.

   FunctionUnitData ^ data = FindFunctionUnitData(functionUnit);
   if (data)
      return data->IsLabelDeclared(label);
   return false;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds a reference to the given label in the provided
//    function unit.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::AddLabelReference
(
   Phx::FunctionUnit ^ functionUnit, 
   int label
)
{
   // Defer the call to the data node associated with the
   // function unit.

   FunctionUnitData ^ data = FindFunctionUnitData(functionUnit);
   if (data)
      data->AddLabelReference(label);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds an entry to the goto fixup list for the given
//    function unit.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::AddGotoLabelFixup
(
   Phx::FunctionUnit ^ functionUnit, 
   int label, 
   Phx::IR::BranchInstruction ^ gotoInstruction,
   int sourceLineNumber
)
{
   // Defer the call to the data node associated with the
   // function unit.

   FunctionUnitData ^ data = FindFunctionUnitData(functionUnit);
   if (data)
      data->AddGotoLabelFixup(label, gotoInstruction, sourceLineNumber);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds the given label to the declaration list associated
//    with the given function unit.
//
// Remarks:
//
//
// Returns:
//
//   true if the label is unique to the function unit; false if 
//   it has already been declared.
//
//-----------------------------------------------------------------------------

bool
ModuleBuilder::AddLabelDeclaration
(
   Phx::FunctionUnit ^ functionUnit, 
   int label
)
{
   // Defer the call to the data node associated with the
   // function unit.

   FunctionUnitData ^ data = FindFunctionUnitData(functionUnit);
   if (data)
      return data->AddLabelDeclaration(label);
   return false;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the FunctionUnit object associated with the given 
//    function name.
//
// Remarks:
//
//
// Returns:
//
//   The FunctionUnit object associated with the given 
//   function name, or nullptr if the function is not defined.
//
//-----------------------------------------------------------------------------

Phx::FunctionUnit ^ 
ModuleBuilder::FindFunctionUnit
(
   String ^ functionUnitName
)
{
   // Return the unit associated with the data node associated 
   // with the function name, or nullptr if the data node could
   // not be found.

   FunctionUnitData ^ data = FindFunctionUnitData(functionUnitName);
   if (data)
      return data->Unit;
   return nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds a function symbol from the given parameters.
//
// Remarks:
//
//    Call this method to build a function symbol with the given
//    name, calling convention, argument list, and return value.
//
// Returns:
//
//   A new FunctionSymbol object for the function.
//
//-----------------------------------------------------------------------------

Phx::Symbols::FunctionSymbol ^
ModuleBuilder::BuildFunctionSymbol
(
   Phx::Lifetime ^ lifetime,
   Phx::Types::Table ^ typeTable,
   Phx::Symbols::Table ^ symbolTable,
   String ^ signature,
   Phx::Types::CallingConventionKind callingConvention,
   Phx::Symbols::Visibility visibility,
   array<Phx::Types::Type ^> ^ argumentList,
   bool isEllipsis,
   Phx::Types::Type ^ returnValue
)
{
   // Build the function type by using a FunctionTypeBuilder object.

   Phx::Types::FunctionTypeBuilder ^ typeBuilder = 
      Phx::Types::FunctionTypeBuilder::New(typeTable);

   typeBuilder->Begin();

   // Now indicate that this function requires the cdecl calling convention.

   typeBuilder->CallingConventionKind = callingConvention;

   // Set the return value type.

   typeBuilder->AppendReturnParameter(returnValue);

   // Append argument types.

   for each (Phx::Types::Type ^ argumentType in argumentList)
   {
      typeBuilder->AppendParameter(argumentType);
   }

   // If the function contains an ellipsis argument, append it to 
   // the end of the function argument list.

   if (isEllipsis)
   {
      Debug::Assert(callingConvention == 
         Phx::Types::CallingConventionKind::CDecl);

      typeBuilder->AppendParameter(typeTable->UnknownType, Phx::Types::ParameterKind::Ellipsis);
   }         

   // Now extract the function type.

   Phx::Types::FunctionType ^ functionType = 
      typeBuilder->GetFunctionType();

   // Now we're ready to create a symbol to represent the function itself. 
   // First we must give it a linker name. 

   Phx::Name name = Phx::Name::New(
      lifetime,
      signature
   );

   // Return a new FunctionSymbol object.

   return Phx::Symbols::FunctionSymbol::New(
      symbolTable,
      0, 
      name, 
      functionType,
      visibility
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds a FunctionUnit and its beginning part.
//
// Remarks:
//
//    Call this method to build the beginning part of a function. You can 
//    then append the instructions that make up the function body and
//    then call BuildEndFunction to finalize the function.
//
// Returns:
//
//   A new FunctionUnit object for the function.
//
//-----------------------------------------------------------------------------

Phx::FunctionUnit ^ 
ModuleBuilder::BuildBeginFunction
(
   Phx::ModuleUnit ^ moduleUnit,
   String ^ name,
   Phx::Types::CallingConventionKind callingConvKind,
   Phx::Symbols::Visibility visibility,
   array<FormalParameter ^> ^ formalParameters,
   Phx::Types::Type ^ returnType,
   DirectiveType directive,
   int sourceLineNumber
)
{   
   // Report an error and return if the function already exists.

   Phx::Name functionName = Phx::Name::New(moduleUnit->Lifetime, name);
   if (moduleUnit->SymbolTable->NameMap->Lookup(functionName) != nullptr)
   {
      String ^ functionOrProcedure = 
         returnType->Equals(
            moduleUnit->TypeTable->VoidType) ?
               "procedure" : "function";

      Output::ReportError(
         sourceLineNumber, 
         Error::FunctionRedeclaration,
         functionOrProcedure,
         name
      );

      return nullptr;
   }
   
   // Report an error if the return type is not a valid 
   // Pascal function return type.

   if (!returnType->IsVoid &&
       !returnType->IsScalar &&
       !TypeBuilder::IsSubrangeType(returnType))
   {
      Output::ReportError(
         sourceLineNumber,
         Error::InvalidReturnType
      );

      return nullptr;
   }

   // Remember the current function unit data node so we 
   // can insert the new one into the hierarchy.

   FunctionUnitData ^ parentFunctionUnitData = CurrentFunctionUnitData;

   // Get the global type table.

   Phx::Types::Table ^ typeTable = Phx::GlobalData::TypeTable;

   // Build the function type by using a FunctionTypeBuilder object.

   Phx::Types::FunctionTypeBuilder ^ typeBuilder =
      Phx::Types::FunctionTypeBuilder::New(typeTable);

   typeBuilder->Begin();

   // Set calling convention.

   typeBuilder->CallingConventionKind = callingConvKind;      

   // Establish return type.

   typeBuilder->AppendReturnParameter(returnType);
  
   // Append arguments.
   
   if (formalParameters)
   {
      for each (FormalParameter ^ parameter in formalParameters)
      {  
         // If the parameter type is not by value (e.g. a variable,
         // functional, or procedural parameter), set the parameter 
         // type to its pointer type. This is because we pass 
         // non-value parameters by address.

         Phx::Types::Type ^ parameterType = parameter->Type;
         if (parameter->ParameterType != FormalParameterType::Value)
         {
            parameterType = typeTable->GetUnmanagedPointerType(parameterType);
         }

         // Append the argument type.

         typeBuilder->AppendParameter(parameterType);
      }
   }

   // Now extract the function type.

   Phx::Types::FunctionType ^ functionType = typeBuilder->GetFunctionType();

   // Use the function type to create a function symbol. Then use the function
   // symbol to create a function unit. Finally, attach a symbol table 
   // to the function unit.

   // Create function symbol in the module's symbol table.

   Phx::Symbols::FunctionSymbol ^ functionSymbol = 
      Phx::Symbols::FunctionSymbol::New(
         moduleUnit->SymbolTable, 
         0 /* externId can be arbitrary here */,
         functionName, 
         functionType, 
         visibility
      );

   Phx::Lifetime ^ lifetime = Phx::Lifetime::New(
      Phx::LifetimeKind::Function, 
      nullptr
   );

   // Create function unit.

   Phx::FunctionUnit ^ functionUnit = Phx::FunctionUnit::New(
      lifetime,
      functionSymbol, 
      Phx::CodeGenerationMode::Native,
      Phx::GlobalData::TypeTable, 
      moduleUnit->Architecture, 
      moduleUnit->Runtime,
      moduleUnit, 
      ++FunctionCounter
   );

   // Set the current global function unit.

   functionUnitStack->Insert(0, functionUnit);

   // Create a FunctionUnitData to hold data specific to our
   // compiler and add it to the hierarchy.

   FunctionUnitData ^ node = gcnew FunctionUnitData(
      name,
      functionUnit,
      parentFunctionUnitData,
      formalParameters,
      directive
   );
      
   if (headFunctionUnitData == nullptr)
   {
      // Establish the head node of the hierarchy.

     headFunctionUnitData = node;      
   }
   else
   {
      // Insert as a child of the current item.

      parentFunctionUnitData->ChildList->Insert(0, node);
   }

   // Attach a local symbol table to the function unit.

   Phx::Symbols::Table ^ functionSymbolTable = Phx::Symbols::Table::New(
      functionUnit, 
      64, 
      true
   );

   // Make it accessible by symbol name.

   functionSymbolTable->AddMap(Phx::Symbols::NameMap::New(
      functionSymbolTable, 
      64
      )
   );

   // Push the symbol table onto the symbol stack.

   PushScope(functionSymbolTable);

   // Create debug information so we can track source positions.

   Phx::Debug::Info::New(lifetime, functionUnit);

   // Set the current debug tag of the function to refer to the
   // current file name and source line number.

   functionUnit->CurrentDebugTag = 
      functionUnit->DebugInfo->CreateTag(
         Phx::Name::New(
            functionUnit->Lifetime, 
            SourceFileName
         ),
         sourceLineNumber
      );

   // Update the debug tag of any instructions already associated
   // with the function unit.

   Phx::IR::Instruction ^ instruction = functionUnit->FirstInstruction;
   while (instruction)
   {
      instruction->DebugTag = functionUnit->CurrentDebugTag;
      instruction = instruction->Next;
   }

   // Build ENTERFUNCTION.

   Phx::IR::LabelInstruction ^ enterInstruction = 
      Phx::IR::LabelInstruction::New(
         functionUnit,
         Phx::Common::Opcode::EnterFunction, 
         functionSymbol
      );
    
   functionUnit->FirstInstruction->InsertAfter(enterInstruction);

   // There needs to be a edge from start to enter.

   Phx::IR::LabelOperand ^ labelOperand = 
      Phx::IR::LabelOperand::New(functionUnit, enterInstruction);

   functionUnit->FirstInstruction->AppendLabelSource(
      Phx::IR::LabelOperandKind::Technical, 
      labelOperand
   );
   
   // Add arguments to local symbol table.
   
   if (formalParameters)
   {
      for each (FormalParameter ^ parameter in formalParameters)
      {   
         // If the parameter type is not by value (e.g. a variable,
         // functional, or procedural parameter), set the parameter 
         // type to its pointer type. This is because we pass 
         // non-value parameters by address.

         Phx::Types::Type ^ parameterType = parameter->Type;
         if (parameter->ParameterType != FormalParameterType::Value)
         {
            parameterType = typeTable->GetUnmanagedPointerType(parameterType);
         }
         
         // Add the parameter declaration to the function unit 
         // symbol table. We use the Parameter storage class for
         // function parameters.

         Phx::Symbols::Symbol ^ localVariableSymbol = 
            AddVariableDeclarationSymbol(
               functionUnit,
               parameter->Name,
               parameterType,
               Phx::Symbols::StorageClass::Parameter,
               parameter->SourceLineNumber
            );

         // Append the operand to the enter instruction's destination list.

         enterInstruction->AppendDestination(
            Phx::IR::VariableOperand::New(
               functionUnit,
               localVariableSymbol->Type,
               localVariableSymbol
            )
         );
      } 
   }

   // Establish a symbol for the return value if we're building a function
   // and not a procedure.

   if (! returnType->IsVoid)
   {      
      node->ReturnSymbol = AddVariableDeclarationSymbol(
         functionUnit,
         name,
         returnType,
         Phx::Symbols::StorageClass::Auto,
         sourceLineNumber
      );
   }

   return functionUnit;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds the end part of a given FunctionUnit.
//
// Remarks:
//
//    Call this method to build the end part of a function after you called
//    BuildBeginFunction and appended the instructions that make up the 
//    function body. 
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::BuildEndFunction
(
    Phx::FunctionUnit ^ functionUnit,
    int sourceLineNumber
)
{
   // Fixup forward-declared label references and report usage warnings.

   PostProcessLabels(functionUnit, sourceLineNumber);

   // Build the enter and leave display calls. These methods will
   // know where to insert the call instructions into the IR stream.

   BuildEnterDisplay(functionUnit, sourceLineNumber);
   BuildLeaveDisplay(functionUnit, sourceLineNumber);

   // Release any set structures created during this function.

   FunctionUnitData ^ current = FindFunctionUnitData(functionUnit);
   
   for each (Phx::Symbols::Symbol ^ setSymbol in current->SetRelaseSymbols)
   {
      if (setSymbol == nullptr)
         continue;

      // Call the release_set function to clean up the set object.

      List<Phx::IR::Operand ^> ^ arguments = 
         gcnew List<Phx::IR::Operand ^>();

      //set
      arguments->Add(Phx::IR::VariableOperand::New(
            functionUnit,
            setSymbol->Type,
            setSymbol
         )
      );

      ModuleBuilder::Runtime->CallSetFunction(
         functionUnit,
         "release_set",
         arguments,
         sourceLineNumber
      );     
   }

   // Create exit and return instructions.

   Phx::IR::LabelInstruction ^ exitInstruction = 
      Phx::IR::LabelInstruction::New(
         functionUnit,
         Phx::Common::Opcode::ExitFunction
      );

   Phx::IR::Instruction ^ returnInstruction = 
      Phx::IR::BranchInstruction::NewReturn(
         functionUnit,
         Phx::Common::Opcode::Return, 
         exitInstruction
      );

   // Append a source operand to the return instruction 
   // for the return value if the function unit returns
   // a value.

   if (current->ReturnSymbol)
   {
      Phx::IR::VariableOperand ^ returnOperand = 
         Phx::IR::VariableOperand::New(
            functionUnit,
            current->ReturnSymbol->Type,
            current->ReturnSymbol
         );

      returnInstruction->AppendSource(returnOperand);
   }

   // Append the return and exit instructions to the IR stream.

   functionUnit->LastInstruction->InsertBefore(returnInstruction);
   functionUnit->LastInstruction->InsertBefore(exitInstruction);

   // Fixup all instructions after the return instruction
   // to reference the final source line number of the function
   // (when we created the function unit, there may
   // have been end instructions that were created with the 
   // initial line number).

   UInt32 debugTag = functionUnit->DebugInfo->CreateTag(
      Phx::Name::New(
         functionUnit->Lifetime, 
         SourceFileName
      ),
      sourceLineNumber
   );

   Phx::IR::Instruction ^ instruction = returnInstruction;
   while (instruction)
   {
      instruction->DebugTag = debugTag;
      instruction = instruction->Next;
   }

   // Finalize creation of the function unit.

   functionUnit->FinishCreation();

   // Now that we're done with IR for this unit, pop it off the context
   // stack (creating the unit pushed it on).

   functionUnit->Context->PopUnit();
   functionUnitStack->RemoveAt(0);
   
   // Pop the symbol table from the symbol stack.

   PopScope();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Activates the given FunctionUnit object for editing.
//
// Remarks:
//
//    Call this method to edit a completed function unit. Because additional
//    instructions are added when a function unit is completed, we temporarily
//    unlink from the return instruction to the end so normal "append
//    instruction" APIs work correctly.
//
// Returns:
//
//   The list of unlinked Instruction objects.
//
//-----------------------------------------------------------------------------

List<Phx::IR::Instruction ^> ^
ModuleBuilder::BeginFunctionEdit
(
   Phx::FunctionUnit ^ functionUnit,
   int sourceLineNumber
)
{
   // Set the current global function unit.

   functionUnitStack->Insert(0, functionUnit);

   // Push the symbol table onto the symbol stack.

   PushScope(functionUnit->SymbolTable);

   // Push the function unit onto the framework's context stack.

   Phx::Threading::Context ^ context = Phx::Threading::Context::GetCurrent();
   context->PushUnit(functionUnit);

   // Unlink all instructions from Return to End.

   List<Phx::IR::Instruction ^> ^ unlinkedInstructions = 
      gcnew List<Phx::IR::Instruction ^>();

   Phx::IR::Instruction ^ unlinkInstruction = functionUnit->LastInstruction;
   while (! unlinkInstruction->IsReturn)
   {      
      unlinkInstruction = unlinkInstruction->Previous;
      unlinkedInstructions->Insert(0, unlinkInstruction);
   }

   int n = unlinkedInstructions->Count-1;
   while (n >= 0)
   {
      unlinkedInstructions[n]->Unlink();
      --n;
   }
    
   return unlinkedInstructions;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Completes edit of the current FunctionUnit object.
//
// Remarks:
//
//    Call this method after you completed editing the function unit
//    specified to BeginFunctionEdit.
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void 
ModuleBuilder::EndFunctionEdit
(
   List<Phx::IR::Instruction ^> ^ unlinkedInstructions,
   int sourceLineNumber
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = CurrentFunctionUnit;

   // Append the unlinked instructions.

   for each (Phx::IR::Instruction ^ unlinkedInstruction in 
      unlinkedInstructions)
   {
      functionUnit->LastInstruction->InsertBefore(unlinkedInstruction);
   }

   // Fixup forward-declared label references and report usage warnings.

   PostProcessLabels(functionUnit, sourceLineNumber);

   // Pop the function unit from the context stack.

   functionUnit->Context->PopUnit();
   functionUnitStack->RemoveAt(0);

   // Pop the symbol table from the symbol stack.

   PopScope();   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pushes the given symbol table onto the symbol stack.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void 
ModuleBuilder::PushScope
(
   Phx::Symbols::Table ^ symTable
)
{
   scopeList->Insert(0, symTable);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the current symbol table from the symbol stack.
//
// Remarks:
//
//
// Returns:
//
//   The current symbol table from the symbol stack.
//
//-----------------------------------------------------------------------------

Phx::Symbols::Table ^ 
ModuleBuilder::TopScope::get()
{
   // If the local symbol stack is empty, return the symbol table that is
   // associated with the current function unit.
   
   if (scopeList->Count > 0)
      return scopeList[0];

   return CurrentFunctionUnit->SymbolTable;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pops the current symbol table from the symbol stack.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void 
ModuleBuilder::PopScope()
{
   scopeList->RemoveAt(0);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Performs a narrow to wide scope search for a symbol with the given name.
//
// Remarks:
//
//
// Returns:
//
//   The Symbol object with the given name, or nullptr if the symbol could
//   not be found.
//
//-----------------------------------------------------------------------------

Phx::Symbols::Symbol ^ 
ModuleBuilder::LookupSymbol
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ symbolName
)
{      
   // In the case a fully-qualified record field was provided (for example,
   // s.x), look up the symbol for the outermost enclosing aggregate.

   array<String ^> ^ tokens = symbolName->Split(fieldSeparator);

   // Create a Name object for name map lookup.

   Phx::Name name = Phx::Name::New(
      Phx::GlobalData::GlobalLifetime,
      tokens[0]
   );

   Phx::Symbols::Symbol ^ symbol = nullptr;

   // Lookup the symbol in the local symbol table stack.

   for (int i = 0; i < scopeList->Count; ++i)
   {
      symbol = scopeList[i]->NameMap->Lookup(name);
      if (symbol != nullptr)
      {        
         return symbol;
      }
   }

   // Walk the chain of nested procedures and search for the symbol.

   symbol = LookupSymbolAux(tokens[0], headFunctionUnitData);

   if (symbol == nullptr)
   {
      // Try global scope.

      symbol = functionUnit->ParentModuleUnit->
         SymbolTable->NameMap->Lookup(name);
   }
   
   // If the symbol still has not been located, append an underscore 
   // to the symbol name in case the symbol name refers to a CRT function.

   if (symbol == nullptr && symbolName[0] != '_')
   {
      symbol = LookupSymbol(functionUnit, "_" + symbolName);
   }

   return symbol;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Performs a narrow to wide scope search for the type symbol with the 
//    given name.
//
// Remarks:
//
//
// Returns:
//
//   The TypeSymbol object with the given name, or nullptr if the symbol 
//   could not be found.
//
//-----------------------------------------------------------------------------

Phx::Symbols::TypeSymbol ^ 
ModuleBuilder::LookupTypeSymbol
(
   Phx::FunctionUnit ^ functionUnit, 
   String ^ typeName
)
{
   // Walk the chain of nested procedures and search for the symbol.

   Phx::Symbols::TypeSymbol ^ symbol = LookupTypeSymbolAux(
      typeName, 
      FindFunctionUnitData(functionUnit)
   );

   if (symbol == nullptr)
   {
      // Try global scope.

      bool succeeded = false;
      Phx::Types::Type ^ targetType = 
         TypeBuilder::TryGetTargetType(typeName, succeeded);

      if (succeeded && targetType != nullptr)
      {         
         symbol = targetType->TypeSymbol;

         // Create a symbol for the type if it does not exist.

         if (symbol == nullptr)
         {
            symbol = Phx::Symbols::TypeSymbol::New(
               functionUnit->SymbolTable, 
               0,
               Phx::Name::New(functionUnit->Lifetime, typeName)
            );
            symbol->Type = targetType;
         }
      }

      // Search the global list of types for a match.

      if (symbol == nullptr)
      {
         Phx::ModuleUnit ^ moduleUnit = functionUnit->ParentModuleUnit;
         Phx::Types::Table ^ typeTable = moduleUnit->TypeTable;

         for each (Phx::Types::Type ^ type in typeTable->AllTypes)
         {
            if (type->TypeSymbol 
               && type->TypeSymbol->NameString->ToLower()->Equals(typeName))
            {
               symbol = type->TypeSymbol;
               break;
            }
         }
      }
   }
   
   return symbol;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Creates a new, non-user variable symbol with the given type and
//    storage class.
//
// Remarks:
//
//    Call this method to create a symbolic variable for internal use.
//
// Returns:
//
//   The new internal Symbol object of the provided type.
//
//-----------------------------------------------------------------------------

Phx::Symbols::Symbol ^
ModuleBuilder::AddInternalVariableDeclarationSymbol
(
   Phx::FunctionUnit ^ functionUnit, 
   Phx::Types::Type ^ type, 
   Phx::Symbols::StorageClass storageClass,
   int sourceLineNumber
)
{
   FunctionUnitData ^ current = FindFunctionUnitData(functionUnit);

   // Prepend invalid Pascal identifier characters to make the symbol
   // non-accessible from the source program.

   String ^ nameString = String::Format("$$temp{0,-3:G}", 
      current->NextTemporarySymbolId++);

   // Use the AddVariableDeclarationSymbol method to create the symbol.

   return AddVariableDeclarationSymbol(
      functionUnit,
      nameString,
      type,
      storageClass,
      sourceLineNumber
   );  
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Creates a new user variable symbol with the given type and
//    storage class.
//
// Remarks:
//
//
// Returns:
//
//   The new Symbol object of the provided type.
//
//-----------------------------------------------------------------------------

Phx::Symbols::Symbol ^ 
ModuleBuilder::AddVariableDeclarationSymbol
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ name,
   Phx::Types::Type ^ type,
   Phx::Symbols::StorageClass storageClass,
   int sourceLineNumber
)
{
   // If the identifier already exists, report an error.

   Phx::Symbols::Symbol ^ variableSymbol = 
      functionUnit->SymbolTable->NameMap->Lookup(
         Phx::Name::New(functionUnit->Lifetime, name)
      );
   
   if (variableSymbol != nullptr)
   {
      Output::ReportError(
         sourceLineNumber, 
         Error::SymbolRedeclaration,
         name, 
         TypeBuilder::GetSourceType(variableSymbol->Type)
      );
   }

   // Otherwise, the identifier is a new symbol. Create a new 
   // LocalVariableSymbol object for the identifier.

   else
   {
      Phx::Name variableName = 
         Phx::Name::New(functionUnit->Lifetime, name);

      Phx::Symbols::LocalVariableSymbol ^ symbol = 
         Phx::Symbols::LocalVariableSymbol::New(
            TopScope,
            0,
            variableName,
            type,
            storageClass
         );

      symbol->Alignment = 
         Phx::Alignment::NaturalAlignment(symbol->Type);

      variableSymbol = symbol;

      // Add the symbol to the cached list of local symbols for
      // the current function unit.

      CurrentFunctionUnitData->LocalSymbols->Add(variableSymbol);
   }

   return variableSymbol;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Marks the given symbol as a local constant by mapping the provided symbol
//    to the provided ConstantValue object.
//
// Remarks:
//
//    We don't use the framework's ConstantSymbol class because ConstantSymbol
//    creates a global symbol.
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::AddLocalConstant
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Symbols::Symbol ^ symbol,
   ConstantValue ^ value
)
{
   FunctionUnitData ^ node = FindFunctionUnitData(functionUnit);
   Debug::Assert(node != nullptr);

   // Add symbol to symbol list.
   node->ConstantSymbolList->Add(symbol);   
   // Map symbol to constant value.
   node->ConstantSymbolValues->Add(symbol, value);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given symbol is mapped to a constant value.
//
// Remarks:
//
//
// Returns:
//
//   true if the symbol is mapped to a constant value; false otherwise.
//
//-----------------------------------------------------------------------------

bool
ModuleBuilder::IsLocalConstant
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Symbols::Symbol ^ symbol
)
{
   FunctionUnitData ^ node = FindFunctionUnitData(functionUnit);
   Debug::Assert(node != nullptr);

   // Walk the chain of nested procedures to search for a constant 
   // value mapped to the provided symbol.

   while (node != nullptr)
   {
      if (node->ConstantSymbolList->Contains(symbol))
         return true;
      node = node->Parent;
   }
   return false;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the ConstantValue object mapped to the provided symbol.
//
// Remarks:
//
//    A precondition to calling this method is that there must be a valid
//    ConstantValue object mapped to the provided symbol. Call the 
//    IsLocalConstant method to first verify whether a valid mapping exists.
//
// Returns:
//
//   The ConstantValue mapped to the provided symbol, or nullptr if the 
//   mapping could not be found.
//
//-----------------------------------------------------------------------------

ConstantValue ^
ModuleBuilder::GetLocalConstantValue
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Symbols::Symbol ^ sym
)
{
   Debug::Assert(IsLocalConstant(functionUnit, sym));

   FunctionUnitData ^ node = FindFunctionUnitData(functionUnit);
   Debug::Assert(node != nullptr);

   // Walk the chain of nested procedures to search for a constant 
   // value mapped to the provided symbol.

   while (node != nullptr)
   {
      if (node->ConstantSymbolList->Contains(sym))
         return node->ConstantSymbolValues[sym];
      node = node->Parent;
   }

   Debug::Assert(false);
   return nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a unique identifier name or internal use.
//
// Remarks:
//
//
// Returns:
//
//   A unique string identifier.
//
//-----------------------------------------------------------------------------

String ^
ModuleBuilder::GenerateUniqueSymbolName()
{
   // Prepend invalid Pascal identifier characters to make the symbol
   // non-accessible from the source program.

   String ^ name = String::Format(
      "$$unique{0,-3:G}", 
      uniqueSymbolId
   );

   uniqueSymbolId++;

   return name;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves an operand that contains the runtime address of 
//    the given symbol.
//
// Remarks:
//
//    Call this method to build an operand for a non-local symbol that is 
//    part of a parent procedure.
//
// Returns:
//
//   A new Operand object that contains the runtime address of 
//   the given symbol.
//
//-----------------------------------------------------------------------------

Phx::IR::MemoryOperand ^
ModuleBuilder::GetNonLocalSymbolAddress
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Symbols::Symbol ^ symbol,
   int sourceLineNumber
)
{   
   // The symbol must come from a function unit.

   Debug::Assert(symbol->Table->Unit->IsFunctionUnit);

   FunctionUnitData ^ sourceFunctionUnitNode = FindFunctionUnitData(
      symbol->Table->Unit->AsFunctionUnit
   );
   Phx::FunctionUnit ^ sourceFunctionUnit = sourceFunctionUnitNode->Unit;

   // Build up the arguments needed to call the 'display_get_address'
   // runtime function.

   List<Phx::IR::Operand ^ > ^ arguments = 
      gcnew List<Phx::IR::Operand ^ >();

   //file_index
   arguments->Add(Phx::IR::ImmediateOperand::New(
      functionUnit,
      functionUnit->TypeTable->Int32Type,
      (int) sourceFunctionUnitNode->SourceFileId
      )
   );

   //function_index
   arguments->Add(Phx::IR::ImmediateOperand::New(
      functionUnit,
      functionUnit->TypeTable->Int32Type,
      (int) sourceFunctionUnit->Number
      )
   );

   //variable_index
   arguments->Add(Phx::IR::ImmediateOperand::New(
      functionUnit,
      functionUnit->TypeTable->Int32Type,
      (int) sourceFunctionUnitNode->GetLocalSymbolIndex(symbol)
      )
   );

   // Call the 'display_get_address' function.

   Phx::IR::Instruction ^ callInstruction =
      ModuleBuilder::Runtime->CallDisplayFunction(
         functionUnit,
         "display_get_address",
         arguments,
         functionUnit->LastInstruction->Previous,
         sourceLineNumber
      );

   // The result of the function call is the address of the top-most
   // activation of the provided symbol.

   Phx::IR::Operand ^ addressOperand = callInstruction->DestinationOperand;

   // Return a new MemoryOperand object to represent the variable.

   return Phx::IR::MemoryOperand::New(
      functionUnit,
      symbol->Type,
      nullptr,
      addressOperand->AsVariableOperand,
      0,
      Phx::Alignment::NaturalAlignment(symbol->Type),
      functionUnit->AliasInfo->IndirectAliasedMemoryTag,
      functionUnit->SafetyInfo->SafeTag
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Allocates the backing store for Pascal file objects.
//
// Remarks:
//
//    Because objects of 'file' type are built in to the Pascal language, we
//    call into the runtime to allocate and manage user files.
//    Although we enforce compile-time type checking of file objects
//    (e.g. ensure we don't read a real from a file of integer), the
//    run-time type of all file objects is HANDLE.
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::AllocateFile
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Symbols::Symbol ^ fileSymbol,
   Phx::Types::Type ^ staticType,
   int sourceLineNumber
)
{
   Debug::Assert(staticType->IsAggregateType);
   Phx::Types::AggregateType ^ fileType = staticType->AsAggregateType;

   // Use the name of the identifier to name the file.

   String ^ path = fileSymbol->NameString;

   // If the file is not declared as a program parameter, it is a 
   // temporary file (e.g. it does not persist after the program exits).

   int temporary = !ModuleBuilder::IsProgramParameter(path);

   // We use this list to make calls to the runtime.

   List<Phx::IR::Operand ^> ^ arguments = gcnew List<Phx::IR::Operand ^>();

   // Call file_open to construct a new file handle.

   //path
   Phx::Symbols::GlobalVariableSymbol ^ pathSym = 
      ModuleBuilder::GetStringSymbol(functionUnit->ParentModuleUnit, path);
      
   Phx::Symbols::NonLocalVariableSymbol ^ localStringSym =
      MakeProxyInFunctionSymbolTable(pathSym, functionUnit->SymbolTable);

   Phx::IR::VariableOperand ^ pathOperand = Phx::IR::VariableOperand::New(
      functionUnit,
      TypeBuilder::GetTargetType(NativeType::Char),
      localStringSym
   );
   pathOperand->ChangeToAddress();
   
   arguments->Add(pathOperand);

   //temporary
   arguments->Add(Phx::IR::ImmediateOperand::New(
      functionUnit,
      functionUnit->TypeTable->Int32Type,
      (int) temporary)
   );

   // CallFileFunction will append the remaining arguments.

   Phx::IR::Instruction ^ callInstruction = 
      ModuleBuilder::Runtime->CallFileFunction(
         functionUnit,
         "file_open",
         arguments,
         sourceLineNumber
      );

   // Create an operand for the file symbol.

   Phx::IR::VariableOperand ^ fileOperand = 
      Phx::IR::VariableOperand::New(
         functionUnit,
         fileSymbol->Type,
         fileSymbol
      );

   // Assign the allocated file handle to the .$runtime_handle field.
   
   Phx::Symbols::FieldSymbol ^ fieldSymbol = 
      TypeBuilder::FindFieldSymbol(
         fileType,
         "$runtime_handle"
      );
   Phx::IR::VariableOperand ^ fieldOperand = 
      Phx::IR::VariableOperand::New(
         functionUnit,
         fieldSymbol->Field,
         fileOperand->Symbol
      );

   Phx::IR::Instruction ^ assignInstruction = 
      Phx::IR::ValueInstruction::NewUnary(
         functionUnit,
         Phx::Common::Opcode::Assign,
         fieldOperand,
         callInstruction->DestinationOperand
      );
   functionUnit->LastInstruction->InsertBefore(assignInstruction);

   // Load the first element of the file into f^ (the .$current_value field) by
   // calling the 'get' runtime function.
   
   fileOperand->ChangeToMemory();

   arguments->Clear();   
   arguments->Add(fileOperand);
   ModuleBuilder::Runtime->CallFileFunction(
      functionUnit,
      "get",
      arguments,
      sourceLineNumber
   ); 
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Allocates the backing store for Pascal set objects.
//
// Remarks:
//
//    Because objects of 'set' type are built in to the Pascal language, we
//    call into the runtime to allocate and manage user sets.
//    Although we enforce compile-time type checking of set objects, the
//    run-time type of all file objects is void*.
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::AllocateSet
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Symbols::Symbol ^ setSymbol,
   Phx::Types::Type ^ baseType,
   int sourceLineNumber
)
{
   List<Phx::IR::Operand ^> ^ arguments = 
      gcnew List<Phx::IR::Operand ^>();

   // Call the runtime function 'new_set' to construct a new set.

   // Get the range of values for the set.

   ValueRange ^ typeRange = TypeBuilder::GetValueRange(baseType);

   //lower
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) typeRange->Lower
      )  
   );

   //upper
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) typeRange->Upper
      )
   );

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

   // Emit the call to the 'new_set' function. This 
   // function returns the memory location for the allocation.

   Phx::IR::Instruction ^ callInstruction = 
      ModuleBuilder::Runtime->CallSetFunction(
         functionUnit, "new_set", arguments, sourceLineNumber
      );

   // Convert from void* to the static type of the set.
   
   Phx::Types::Resolve ^ resolve = Resolve;

   Phx::IR::Operand ^ setOperand = resolve->Convert(
      callInstruction->DestinationOperand,
      functionUnit->LastInstruction->Previous,
      setSymbol->Type
   );

   Debug::Assert(!setOperand->Type->Equals(
      functionUnit->TypeTable->UnknownType
   ));
   
   Phx::IR::Operand ^ destinationOperand = 
      Phx::IR::VariableOperand::New(functionUnit, 
         setSymbol->Type, setSymbol);

   Phx::IR::Instruction ^ assignInstruction = 
      Phx::IR::ValueInstruction::NewUnary(
         functionUnit,
         Phx::Common::Opcode::Assign,
         destinationOperand,
         setOperand
      );
   functionUnit->LastInstruction->InsertBefore(assignInstruction);

   // Mark the created set for memory release after the function exits.

   Phx::Types::Type ^ setType = TypeBuilder::RegisterSetType(
      functionUnit,
      TypeBuilder::GetUniqueTypeName(),
      baseType
   );

   ModuleBuilder::AddSetDefinitionSymbol(
      functionUnit,
      setSymbol,
      setType
   );

   ModuleBuilder::AddSetSymbolToReleaseList(
      functionUnit,
      setSymbol
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Allocates the backing store for an array of Pascal set objects.
//
// Remarks:
//
//    An 'array of set' declaration might look like:
//       var sieve,primes : array[0..w] of set of 0..maxbit;
//    This method allocates a set object for each member of the array.
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::AllocateSetArray
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Symbols::Symbol ^ setArraySymbol,
   int sourceLineNumber
)
{
   // Calculate the element count from the size of the overall array 
   // divided by the size of a single element.

   Phx::Types::UnmanagedArrayType ^ arrayType = 
      setArraySymbol->Type->AsUnmanagedArrayType;

   Phx::Types::Type ^ baseType = 
      arrayType->ElementType->AsPointerType->ReferentType;

   int elementCount = arrayType->BitSize / arrayType->ElementType->BitSize;

   List<Phx::IR::Operand ^> ^ arguments = gcnew List<Phx::IR::Operand ^>();

   // We call 'new_set' to construct a new set array.
   // Set up the argument list to call the 'new_set' function once, and
   // we will use it in the loop below.

   ValueRange ^ typeRange = TypeBuilder::GetValueRange(baseType);

   //lower
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) typeRange->Lower
      )
   );

   //upper
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) typeRange->Upper
      )
   );

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

   // Create a variable operand for the base of the array.

   Phx::IR::VariableOperand ^ arrayBaseOperand = 
      Phx::IR::VariableOperand::New(
         functionUnit,
         setArraySymbol->Type,
         setArraySymbol
      );
   arrayBaseOperand->ChangeToAddress();

   // Allocate each array member.

   for (int i = 0; i < elementCount; ++i)
   {
      Phx::IR::Instruction ^ callInstruction = 
         ModuleBuilder::Runtime->CallSetFunction(
            functionUnit,
            "new_set",
            arguments,
            sourceLineNumber
         );      

      // Load the ith element.

      Phx::IR::MemoryOperand ^ destinationOperand = 
         Phx::IR::MemoryOperand::New(
            functionUnit,
            arrayType->ElementType,
            nullptr,
            arrayBaseOperand,
            i * arrayType->ElementType->ByteSize,
            Phx::Alignment::NaturalAlignment(arrayType->ElementType),
            functionUnit->AliasInfo->IndirectAliasedMemoryTag,
            functionUnit->SafetyInfo->SafeTag
         );

      // Convert from void* to the static type of the set.
   
      Phx::Types::Resolve ^ resolve = Resolve;

      Phx::IR::Operand ^ sourceOperand = resolve->Convert(
         callInstruction->DestinationOperand,
         functionUnit->LastInstruction->Previous,
         arrayType->ElementType
      );

      Debug::Assert(!sourceOperand->Type->Equals(
         functionUnit->TypeTable->UnknownType
      ));

      Phx::IR::Instruction ^ assignInstruction = 
         Phx::IR::ValueInstruction::NewUnary(
            functionUnit,
            Phx::Common::Opcode::Assign,
            destinationOperand,
            sourceOperand
         );

      functionUnit->LastInstruction->InsertBefore(assignInstruction);

      if (i==0)
      {
         Phx::Types::Type ^ setType = TypeBuilder::RegisterSetType(
            functionUnit,
            TypeBuilder::GetUniqueTypeName(),
            baseType
         );
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Add the provided set symbol to the list of set symbols mapped
//    to the given function unit.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::AddSetDefinitionSymbol
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Symbols::Symbol ^ symbol,
   Phx::Types::Type ^ staticType
)
{
   FindFunctionUnitData(functionUnit)->SetSymbols->Add(symbol, staticType);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the provided set symbol is mapped in the list 
//    of set symbols of the given function unit.
//
// Remarks:
//
//
// Returns:
//
//   true if the symbol is associated with the set symbols of the given
//   function unit; false otherwise.
//
//-----------------------------------------------------------------------------

bool
ModuleBuilder::IsSetDefinitionSymbol
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Symbols::Symbol ^ symbol
)
{
   return FindFunctionUnitData(functionUnit)->SetSymbols->ContainsKey(symbol);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Marks the given set symbol for memory release after the given
//    function exists.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::AddSetSymbolToReleaseList
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Symbols::Symbol ^ symbol
)
{
   FindFunctionUnitData(functionUnit)->SetRelaseSymbols->Add(symbol);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given symbol represents a pass-by-reference 
//    procedure parameter.
//
// Remarks:
//
//
// Returns:
//
//   true if the symbols is a pass-by-reference procedure parameter; 
//   false otherwise.
//
//-----------------------------------------------------------------------------

bool
ModuleBuilder::IsVariableParameterSymbol
(
   Phx::Symbols::Symbol ^ symbol
)
{
   // First check whether the symbol's storage class is Parameter.

   if (symbol &&
       symbol->IsVariableSymbol &&
       symbol->AsVariableSymbol->StorageClass == 
         Phx::Symbols::StorageClass::Parameter)
   {
      // Now check whether the parameter's ParameterType is
      // Variable.

      FormalParameter ^ formalParam = GetFormalParameter(
         symbol->NameString
      );

      if (formalParam && formalParam->ParameterType == 
         FormalParameterType::Variable)
      {
         return true;
      }
   }
   return false;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given function is callable from the current
//    location.
//
// Remarks:
//
//
// Returns:
//
//   true if the function is callable from the current location; 
//   false otherwise.
//
//-----------------------------------------------------------------------------

bool
ModuleBuilder::IsCallable
(
   Phx::Symbols::FunctionSymbol ^ functionSymbol
)
{
   // In order to be 'callable', the name of the function symbol must match
   // one of the functions in the chain of function units.
   
   FunctionUnitData ^ node = CurrentFunctionUnitData;
   
   // Check children.

   for each (FunctionUnitData ^ child in node->ChildList)
   {
      if (functionSymbol->Equals(child->Unit->FunctionSymbol))
      {
         return true;
      }
   }

   // Check siblings.

   if (node->Parent)
   {
      for each (FunctionUnitData ^ sibling in node->Parent->ChildList)
      {
         if (functionSymbol->Equals(sibling->Unit->FunctionSymbol))
         {
            return true;
         }
      }
   }

   // Walk the function hierarchy.

   node = node->Parent;
   while (node)
   {
      if (functionSymbol->Equals(node->Unit->FunctionSymbol))
      {
         return true;
      }
      for each (FunctionUnitData ^ child in node->ChildList)
      {
         if (functionSymbol->Equals(child->Unit->FunctionSymbol))
         {
            return true;
         }
      }

      node = node->Parent;
   }

   return false;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the list of all FunctionUnit objects for the
//    entire module.
//
// Remarks:
//
//
// Returns:
//
//   All FunctionUnit objects for the entire module.
//
//-----------------------------------------------------------------------------

IEnumerable<Phx::FunctionUnit ^> ^ 
ModuleBuilder::FunctionUnits::get()
{
   return GetFunctionUnitList();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the FunctionUnitData node associated with the given function
//    unit.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

FunctionUnitData ^ 
ModuleBuilder::FindFunctionUnitData
(
   Phx::FunctionUnit ^ functionUnit
)
{
   return FindFunctionUnitDataAux(functionUnit, headFunctionUnitData);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the FunctionUnitData node associated with the given function
//    name.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

FunctionUnitData ^ 
ModuleBuilder::FindFunctionUnitData
(
   String ^ functionName
)
{
   return FindFunctionUnitDataAux(functionName, headFunctionUnitData);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the FunctionUnitData node associated with the given function
//    unit.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

FunctionUnitData ^ 
ModuleBuilder::FindFunctionUnitDataAux
(
   Phx::FunctionUnit ^ functionUnit, 
   FunctionUnitData ^ node
)
{
   // Base case.

   if (node->Unit == functionUnit)
      return node;

   // Recursive case; check child list.

   for each(FunctionUnitData ^ child in node->ChildList)
   {
      FunctionUnitData ^ functionUnitData = 
         FindFunctionUnitDataAux(functionUnit, child);
      if (functionUnitData != nullptr)
         return functionUnitData;
   }
   return nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the FunctionUnitData node associated with the given function
//    name.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

FunctionUnitData ^ 
ModuleBuilder::FindFunctionUnitDataAux
(
   String ^ functionName,
   FunctionUnitData ^ node
)
{
   // Base cases.
   
   if (node == nullptr || node->Name == functionName)
      return node;

   // Recursive case; check child list.

   for each(FunctionUnitData ^ child in node->ChildList)
   {
      FunctionUnitData ^ functionUnitData = 
         FindFunctionUnitDataAux(functionName, child);
      if (functionUnitData != nullptr)
         return functionUnitData;
   }
   return nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the list of all FunctionUnit objects for the
//    entire module.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

List<Phx::FunctionUnit ^> ^ 
ModuleBuilder::GetFunctionUnitList()
{
   List<Phx::FunctionUnit ^> ^ list = gcnew List<Phx::FunctionUnit ^>();

   // Add the compiler-generated main().

   list->Add(MainFunctionUnit);
   
   // Add user functions.

   PopulateFunctionUnitList(list, headFunctionUnitData);
   
   return list;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Recursively populates the list of all FunctionUnit objects for the
//    entire module.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void 
ModuleBuilder::PopulateFunctionUnitList
(
   List<Phx::FunctionUnit ^> ^ list, 
   FunctionUnitData ^ node
)
{
   list->Add(node->Unit);
   for each (FunctionUnitData ^ child in node->ChildList)
      PopulateFunctionUnitList(list, child);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the FunctionUnitData object associated with the current
//    function unit.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

FunctionUnitData ^ 
ModuleBuilder::CurrentFunctionUnitData::get()
{
   if (headFunctionUnitData == nullptr)
      return nullptr;

   // Find the FunctionUnitData that matches the current FunctionUnit.

   return FindFunctionUnitData(
      Phx::Threading::Context::GetCurrent()->FunctionUnit
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Performs label post-processing for completed function units.
//
// Remarks:
//
//    Call this method to resolve label fixups (use before definition)
//    and report declaration/reference warnings.
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void 
ModuleBuilder::PostProcessLabels
(
   Phx::FunctionUnit ^ functionUnit,
   int sourceLineNumber
)
{
   FunctionUnitData ^ FunctionUnitData = CurrentFunctionUnitData;
   array<int> ^ labelDeclarations = FunctionUnitData->LabelDeclarations;

   // Fixup forward-referenced goto labels.

   FunctionUnitData->FixupGotoLabels();

   // Report labels that were declared but never defined.

   for each (int label in labelDeclarations)
   {
      Phx::IR::LabelInstruction ^ labelInstruction = 
         FunctionUnitData->GetLabelInstruction(
            label,
            sourceLineNumber
         );
      if (labelInstruction == nullptr)
      {
         Output::ReportWarning(
            sourceLineNumber,
            Error::DeclaredUndefinedLabel,
            label
         );
      }
   }

   // Report labels that were declared but never referenced.

   for each (int label in labelDeclarations)
   {
      int refCount = FunctionUnitData->GetLabelReferenceCount(label);
      if (refCount == 0)
      {
         Output::ReportWarning(
            sourceLineNumber,
            Error::UnreferencedLabel,
            label
         );
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pushes an activation context for the provided function unit.
//
// Remarks:
//
//    This method adds the address of all local variables to a side table
//    so that child (nested) procedures may access them.
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::BuildEnterDisplay
(
   Phx::FunctionUnit ^ functionUnit,
   int sourceLineNumber
)
{   
   FunctionUnitData ^ functionUnitNode = FindFunctionUnitData(functionUnit);

   // Call the 'display_enter' runtime function. The 'display_enter' function
   // takes the address of each local variable in the given function.
   // This function takes a variable number of arguments, so no special
   // data structures are required.

   List<Phx::IR::Operand ^ > ^ arguments = 
      gcnew List<Phx::IR::Operand ^ >();
   
   Phx::Symbols::Symbol ^ symbol = 
      ModuleBuilder::GetNonLocalStringSymbol(
         functionUnit,
         functionUnitNode->SourceFileName
      );

   //filename
   arguments->Add(Phx::IR::VariableOperand::New(
         functionUnit,
         TypeBuilder::GetTargetType(NativeType::Char),
         symbol
      )
   );
   arguments[0]->ChangeToAddress();

   //file_index
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) functionUnitNode->SourceFileId
      )
   );

   //function_index
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) functionUnit->Number
      )
   );

   //arg_count
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) functionUnitNode->LocalSymbols->Count
      )
   );

   for each (Phx::Symbols::Symbol ^ symbol in functionUnitNode->LocalSymbols)
   {
      //ellipsis argument - add address of each local symbol.

      Phx::IR::Operand ^ addressOperand = Phx::IR::VariableOperand::New(
         functionUnit,
         symbol->Type,
         symbol
      );
      addressOperand->ChangeToAddress(); 

      arguments->Add(addressOperand);      
   }

   // Call the runtime function.

   ModuleBuilder::Runtime->CallDisplayFunction(
      functionUnit,
      "display_enter",
      arguments,
      functionUnit->FirstEnterInstruction,
      sourceLineNumber
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pops the current activation context for the provided function unit.
//
// Remarks:
//
//    This method removes the top entry from the display table.
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::BuildLeaveDisplay
(
   Phx::FunctionUnit ^ functionUnit,
   int sourceLineNumber
)
{   
   FunctionUnitData ^ functionUnitNode = FindFunctionUnitData(functionUnit);

   // Call the 'display_leave' runtime function. 
   // This function will remove the top display record of the function.

   List<Phx::IR::Operand ^ > ^ arguments = 
      gcnew List<Phx::IR::Operand ^ >();
   
   //file_index
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) functionUnitNode->SourceFileId
      )
   );

   //function_index
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) functionUnit->Number
      )
   );

   ModuleBuilder::Runtime->CallDisplayFunction(
      functionUnit,
      "display_leave",
      arguments,
      functionUnit->LastInstruction->Previous,
      sourceLineNumber
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Initializes the global built-in constants for the given module unit.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
ModuleBuilder::InitializeGlobalConstants
(
   Phx::ModuleUnit ^ moduleUnit
)
{
   Phx::Symbols::Table ^ symbolTable = moduleUnit->SymbolTable;
   Phx::Types::Table ^ typeTable = moduleUnit->TypeTable;

   // Register the 'Boolean' enumeration, as well as the
   // 'true' and 'false' constants.

   String ^ typeName = "Boolean";

   // Pascal enums are always backed by integer type.

   Phx::Types::PrimitiveType ^ underlyingPrimitiveType = 
      moduleUnit->TypeTable->Int32Type;

   Phx::Name name = Phx::Name::New(
      symbolTable->Lifetime, 
      typeName
   );

   Phx::Symbols::TypeSymbol ^ typeSymbol = 
      Phx::Symbols::TypeSymbol::New(
         symbolTable,
         0,
         name
      );

   Phx::Types::EnumType ^ enumType = 
      Phx::Types::EnumType::New(
         typeTable,
         typeSymbol,
         underlyingPrimitiveType
      );

   // Append a single integer field to the enum type.

   // Prepend invalid Pascal identifier characters to make the symbol
   // non-accessible from the source program.

   String ^ fieldName = "$$data";

   Phx::Symbols::FieldSymbol ^ fieldSymbol = 
      Phx::Symbols::FieldSymbol::New(
         symbolTable,
         0,
         Phx::Name::New(moduleUnit->Lifetime, fieldName),
         underlyingPrimitiveType
      );
   enumType->AppendFieldSymbol(fieldSymbol);

   // Register the valid range for the field symbol.

   ValueRange ^ enumRange = gcnew ValueRange(
      0,    // false
      1     // true
   );

   TypeBuilder::RegisterTypeRange(
      enumType,
      enumRange,
      0
   );

   // Set metaproperties for enum type.

   enumType->IsOnlyData = true;
   enumType->IsSealed = true;
   enumType->HasCompleteInheritanceInfo = true;
   enumType->HasCompleteOverrideInfo = true;
   enumType->IsSelfDescribing = true;
      
   List<Phx::Symbols::Symbol ^> ^ symbols = 
      gcnew List<Phx::Symbols::Symbol ^>();

   unsigned int booleanSize = enumType->ByteSize;
   unsigned char * initialValue = new unsigned char[booleanSize];

   for (int i = 0; i < 2; ++i)
   {
      Phx::Symbols::GlobalVariableSymbol ^ globalSymbol = 
         AddGlobalVariable(
            moduleUnit,
            i == 0 ? "false" : "true",
            enumType,
            initialValue,
            booleanSize
         );

      symbols->Add(globalSymbol);
   }

   TypeBuilder::RegisterEnumType(enumType, symbols);
   delete[] initialValue;

   // Register the 'maxint' value.

   const Int32 maxInt = Int32::MaxValue; // 2^31 - 1

   initialValue = 
      new unsigned char[typeTable->Int32Type->ByteSize];   
   ::memcpy(initialValue, &maxInt, sizeof maxInt);

   AddGlobalVariable(
      moduleUnit, "maxint", typeTable->Int32Type,
      initialValue, typeTable->Int32Type->ByteSize);

   delete[] initialValue;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Creates a global symbol with the given initial string value.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

Phx::Symbols::GlobalVariableSymbol ^
ModuleBuilder::CreateInitializedString
(
   Phx::ModuleUnit ^ moduleUnit,
   String ^ value
)
{
   // Generate a unique symbol name for the string.

   String ^ stringName = 
      String::Format("$SG{0,-3:G}", externalIdCounter);

   // Create an array type to for the string.

   unsigned len = (unsigned) value->Length + 1;

   Phx::Types::Type ^ stringType = Phx::Types::UnmanagedArrayType::New(
      moduleUnit->TypeTable,
      Phx::Utility::BytesToBits(len),
      nullptr, 
      moduleUnit->TypeTable->Character8Type
   );

   // Create the global symbol for the string.

   Phx::Symbols::GlobalVariableSymbol ^ stringSymbol = 
      Phx::Symbols::GlobalVariableSymbol::New(
         moduleUnit->SymbolTable, 
         externalIdCounter,
         Phx::Name::New(moduleUnit->Lifetime, stringName), 
         stringType, 
         Phx::Symbols::Visibility::File
      );

   // Record the length of the string.

   StringSymbolLengths[externalIdCounter] = value->Length;
   
   // Set allocation and alignment information.

   stringSymbol->AllocationBaseSectionSymbol = stringSectionSymbol;
   stringSymbol->Alignment = 
      Phx::Alignment(Phx::Alignment::Kind::AlignTo1Byte);

   // Write the string to the string data section.

   Phx::IR::DataInstruction ^ stringInstruction = 
      Phx::IR::DataInstruction::New(
         stringSectionSymbol->Section->DataUnit, 
         len
      );

   stringInstruction->WriteString(0, value);
   stringSectionSymbol->Section->AppendInstruction(stringInstruction);
   stringSymbol->Location = Phx::Symbols::DataLocation::New(stringInstruction);

   externalIdCounter++;

   return stringSymbol;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Creates a section for storing global strings.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void 
ModuleBuilder::CreateStringSection
(
   Phx::ModuleUnit ^ moduleUnit
)
{
   // Create a section symbol named '.strings' to hold 
   // global string data.

   stringSectionSymbol = 
      Phx::Symbols::SectionSymbol::New(
         moduleUnit->SymbolTable, 0,
         Phx::Name::New(moduleUnit->Lifetime, ".strings"));

   // We want this section to hold initialized, read only data. We
   // don't care about alignment, but will use 4 byte
   // minimum.

   stringSectionSymbol->IsReadable   = true;
   stringSectionSymbol->IsWritable   = false;
   stringSectionSymbol->IsInitialize = true;
   stringSectionSymbol->Alignment = 
      Phx::Alignment(Phx::Alignment::Kind::AlignTo4Bytes);

   // Along with the section symbol we need a section. 
   // At this point we have to divulge that we are creating output in COFF.
   
   stringSectionSymbol->Section = Phx::Coff::Section::New(stringSectionSymbol);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Performs a narrow to wide scope search for a symbol with the given name.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

Phx::Symbols::Symbol ^ 
ModuleBuilder::LookupSymbolAux
(
   String ^ name, 
   FunctionUnitData ^ node
)
{
   // Base case; node is invalid.

   if (node == nullptr)
   {
      return nullptr;
   }

   // Base case; check whether the symbol exists within the 
   // symbol table of the current function unit.

   Phx::FunctionUnit ^ functionUnit = node->Unit;
   
   Phx::Symbols::Symbol ^ symbol = 
      functionUnit->SymbolTable->NameMap->Lookup(
         Phx::Name::New(functionUnit->Lifetime, name)
      );

   if (symbol != nullptr)
   {
      return symbol;
   }

   // Recursive case; lookup symbol in parent node.

   return LookupSymbolAux(name, node->Parent);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Performs a narrow to wide scope search for the type symbol with the 
//    given name.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

Phx::Symbols::TypeSymbol ^ 
ModuleBuilder::LookupTypeSymbolAux
(
   String ^ name, 
   FunctionUnitData ^ node
)
{
   // Base case; node is invalid.

   if (node == nullptr)
   {
      return nullptr;
   }

   // Base case; check whether the symbol exists within the 
   // symbol table of the current function unit.

   Phx::FunctionUnit ^ functionUnit = node->Unit;
   
   Phx::Symbols::Symbol ^ symbol = 
      functionUnit->SymbolTable->LookupByName(
         Phx::Name::New(functionUnit->Lifetime, name)
      );

   if (symbol != nullptr)
   {
      return symbol->AsTypeSymbol;
   }

   // Recursive case; lookup symbol in parent node.
     
   return LookupTypeSymbolAux(name, node->Parent);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the formal parameter with the given name from the current
//    function unit.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

FormalParameter ^
ModuleBuilder::GetFormalParameter
(
   String ^ name
)
{
   FunctionUnitData ^ current = CurrentFunctionUnitData;
   if (current && current->FormalParameters)
   {
      for each (FormalParameter ^ param in current->FormalParameters)
      {
         if (param->Name->Equals(name))
         {
            return param;
         }
      }
   }
   return nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the array of declared labels.
//
// Remarks:
//
//
// Returns:
//
//   The array of declared labels.
//
//-----------------------------------------------------------------------------

array<int> ^ 
FunctionUnitData::LabelDeclarations::get()
{
   return labelDeclarationList->ToArray();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds the given label to the declaration list.
//
// Remarks:
//
//
// Returns:
//
//   true if the label is unique to the function unit; false if 
//   it has already been declared.
//
//-----------------------------------------------------------------------------

bool 
FunctionUnitData::AddLabelDeclaration(int label)
{
   if (labelDeclarationList->Contains(label))
      return false;

   labelDeclarationList->Add(label);
   labelReferenceMap->Add(label, 0);
   return true;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given label has been declared.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool 
FunctionUnitData::IsLabelDeclared(int label)
{
   return labelDeclarationList->Contains(label);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds a reference to the given label.
//
// Remarks:
//
//    We track references to user-provided labels so we can issue warnings
//    for unreferenced label definitions.
//
// Returns:
//
//   The reference count for the given label.
//
//-----------------------------------------------------------------------------

int 
FunctionUnitData::AddLabelReference(int label)
{
  labelReferenceMap[label]++;
  return labelReferenceMap[label];
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the reference count for the given label.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

int 
FunctionUnitData::GetLabelReferenceCount(int label)
{
  return labelReferenceMap[label];
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds an entry to the goto fixup list for the associated function unit.
//
// Remarks:
//
//    Because labels can be referenced before they are defined, we maintain
//    a fixup list for cases where the actual label target has not yet been
//    defined.
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
FunctionUnitData::AddGotoLabelFixup
(
   int label, 
   Phx::IR::Instruction ^ replaceThisInstruction,
   int sourceLineNumber
)
{
   LabelFixup ^ fixup = gcnew LabelFixup;
   fixup->Label = label;
   fixup->ReplaceThisInstruction = replaceThisInstruction;
   fixup->SourceLineNumber = sourceLineNumber;

   labelFixupList->Add(fixup);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the label instruction associated with the given label.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

Phx::IR::LabelInstruction ^ 
FunctionUnitData::GetLabelInstruction
(
   int label,
   int sourceLineNumber
)
{
   // Check for the special case of a "dummy" instruction
   // that will be inserted as a temporary instruction.

   if (label == -1)
   {
      // Since all Pascal labels are numeric,
      // we can safely use a non-numeric string constant to 
      // name 'dummy' instructions.

      String ^ nameString = String::Format(
         "__dummy{0,-3:G}", CurrentDummyInstructionId
      );
      CurrentDummyInstructionId++;

      Phx::Name name = Phx::Name::New(
         this->Unit->Lifetime, 
         nameString
      );   

      Phx::Symbols::LabelSymbol ^ labelSymbol = 
         Phx::Symbols::LabelSymbol::New(
            this->Unit->SymbolTable, 
            name
         );

      return Phx::IR::LabelInstruction::New(
         this->Unit, 
         labelSymbol
      );
   }

   if (labelDefinitionMap->ContainsKey(label))
      return labelDefinitionMap[label];
   return nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Performs fixups on the goto fixup list.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

int
FunctionUnitData::FixupGotoLabels()
{
   // Process each fixup in the list.

   int count = 0;
   for each (LabelFixup ^ fixup in labelFixupList)
   {
      // Get the label instruction associated with the label number.

      Phx::IR::LabelInstruction ^ labelInstruction = 
         GetLabelInstruction(
            fixup->Label, 
            fixup->SourceLineNumber
         );

      // If GetLabelInstruction returned nullptr, then the label was
      // never actually defined. In this case, report an error.

      if (labelInstruction == nullptr)
      {
         Output::ReportError(
            fixup->SourceLineNumber,
            Error::UndefinedLabel,
            fixup->Label
         );
      }

      // Otherwise, emit a new Goto instruction directly after
      // the replace instruction and then unlink the 
      // replace instruction.

      else
      {
         IRBuilder::EmitGoto(
            Unit, 
            fixup->ReplaceThisInstruction,
            labelInstruction, 
            fixup->SourceLineNumber
        );

         // Unlink the previously added 'dummy' label.

         fixup->ReplaceThisInstruction->Unlink();

         // Mark that we've referenced this label.

         AddLabelReference(fixup->Label);

         ++count;
      }
   }
   
   labelFixupList->Clear();
   return count;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Appends a label instruction to the function unit.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void 
FunctionUnitData::BuildLabel
(
   int label,
   int sourceLineNumber
)
{
   Phx::FunctionUnit ^ functionUnit = this->Unit;

   // First ensure the label has been forward declared.

   if (! IsLabelDeclared(label))
   {
      Output::ReportError(
         sourceLineNumber,
         Error::UndeclaredLabel,
         label
      );

      // Continue with code generation so we can find 
      // other potential problems.
      // Because we reported the error, no code will be
      // generated.
   }

   // Although flow control in Phoenix is explicit, it is not 
   // explicit in Pascal. Record the logical 'last' instruction 
   // in the current function unit and later append explicit flow control 
   // to the label instruction we are about to make.
   // Note we only need to do this if the last logical instruction is not 
   // already a goto statement.

   Phx::IR::Instruction ^ lastInstruction = 
      functionUnit->LastInstruction->Previous;

   if (lastInstruction->IsBranchInstruction)
   {
      // Null-out the instruction handle so we don't append anything to 
      // it later.

      lastInstruction = nullptr;
   }

   // Check for a redefinition of this label.

   if (GetLabelInstruction(label, sourceLineNumber))
   {
      Output::ReportError(
         sourceLineNumber,
         Error::LabelRedefinition,
         label
      );
   }

   // Otherwise, add a definition for this label.

   else
   {
      // Generate a new LabelSymbol for this label.

      Phx::Name labelName = Phx::Name::New(
         functionUnit->Lifetime, 
         label.ToString()
      );

      Phx::Symbols::LabelSymbol ^ labelSymbol = 
         Phx::Symbols::LabelSymbol::New(
            functionUnit->SymbolTable, 
            labelName
         );

      // Create a new LabelInstruction and append it to the 
      // IR instruction stream.

      Phx::IR::LabelInstruction ^ labelInstruction = 
         Phx::IR::LabelInstruction::New(
            functionUnit, 
            labelSymbol
         );

      IRBuilder::EmitLabel(functionUnit, labelInstruction);

      AddLabelDefinition(label, labelInstruction);

      // If the previous last instruction was a non-branching instruction,
      // create explicit flow into this label region.

      if (lastInstruction)
      {
         IRBuilder::EmitGoto(
            functionUnit, 
            lastInstruction, 
            labelInstruction, 
            sourceLineNumber
         );
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrives the numeric index of the given symbol.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

int 
FunctionUnitData::GetLocalSymbolIndex
(
   Phx::Symbols::Symbol ^ symbol
)
{
   return LocalSymbols->IndexOf(symbol);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds a label/instruction mapping to the label definition map.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool 
FunctionUnitData::AddLabelDefinition
(
   int label, 
   Phx::IR::LabelInstruction ^ instruction
)
{
   if (labelDefinitionMap->ContainsKey(label))
      return false;

   labelDefinitionMap->Add(label, instruction);
   return true;
}

} // namespace Pascal
