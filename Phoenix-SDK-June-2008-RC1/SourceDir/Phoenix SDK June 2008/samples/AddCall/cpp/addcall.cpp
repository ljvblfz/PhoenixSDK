//------------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Simple Phoenix based application to add a call to an external method
//    to the start of a method within a binary
//
//    Input file must be compiled with a phoenix based compiler and -Zi
//
// Usage:
//
//    Usage: addcall /out <filename> /pdbout <filename> /importdll <filename>
//       /importmethod <exported method> /localmethod <local method>
//       /in <image-name>
//
//------------------------------------------------------------------------------

#pragma region Using namespace declarations

using namespace System;

// Not using Phx namespace to show explicit class heirarchy
// using namespace Phx;

#pragma endregion

// Include additional definitions from samples.h

#include "..\..\common\samples.h"

//------------------------------------------------------------------------------
//
// Description:
//
//    Place holder class for Global static data
//
// Remarks:
//
//    Create a placeholder class for the global StringControl objects as this wont
//    compile managed with global variables of this type.
//
//------------------------------------------------------------------------------

public
ref class GlobalPlaceHolder
{
public:
   static Phx::Controls::StringControl ^ in;
   static Phx::Controls::StringControl ^ out;
   static Phx::Controls::StringControl ^ pdbout;
   static Phx::Controls::StringControl ^ importdll;
   static Phx::Controls::StringControl ^ importmethod;
   static Phx::Controls::StringControl ^ localmethod;
};

// Method declarations

Phx::Symbols::ImportSymbol ^
AddImport
(
   Phx::PEModuleUnit ^ module,
   String            ^ dllname,
   String            ^ methodname
);
void                  DoAddInstrumentation(Phx::PEModuleUnit ^ module);
void                  InitCmdLineParser();
Phx::PEModuleUnit ^   OpenModule(String ^ input);
void                  CheckCmdLine();
void                  Usage();

//-----------------------------------------------------------------------------
//
// Description:
//
//    Entry point for application
// 
// Arguments:
// 
//    arguments - Command line arguments
//
// Returns:
//
//    Nothing
//
//-----------------------------------------------------------------------------

int
main
(
   array<String ^> ^ arguments
)
{
   // Initialize the target architectures.
   
   Phx::Targets::Architectures::Architecture ^ architecture =
      Phx::Targets::Architectures::X86::Architecture::New();
   Phx::Targets::Runtimes::Runtime ^ runtime =
      Phx::Targets::Runtimes::Vccrt::Win32::X86::Runtime::New(architecture);
   Phx::GlobalData::RegisterTargetArchitecture(architecture);
   Phx::GlobalData::RegisterTargetRuntime(runtime);

   // Initialize the infrastructure.

   Phx::Initialize::BeginInitialization();

   // Initialize the cmd line stuff.

   ::InitCmdLineParser();

   // Check for Phoenix wide options like "-assertbreak".

   Phx::Initialize::EndInitialization(L"PHX|*|_PHX_", arguments);

   // Check the command line.

   ::CheckCmdLine();

   // Open the module and read it in.

   Phx::PEModuleUnit ^ moduleUnit;

   moduleUnit =
      Phx::PEModuleUnit::Open(GlobalPlaceHolder::in->GetValue(nullptr));

   // Setup output file name and PDB.

   moduleUnit->OutputImagePath = GlobalPlaceHolder::out->GetValue(nullptr);
   moduleUnit->OutputPdbPath = GlobalPlaceHolder::pdbout->GetValue(nullptr);

   // Iterator will load symbols implicitly.
   // However Load Global Symbols upfront and print total.

   moduleUnit->LoadGlobalSymbols();

   Phx::Output::WriteLine(L"Total Global Symbols  Count - {0} ",
      moduleUnit->SymbolTable->SymbolCount.ToString());

   // Do some useful work on the tool front here:

   ::DoAddInstrumentation(moduleUnit);

   // Close the ModuleUnit.

   moduleUnit->Close();

   // If this was not the end of the application it would be best to 
   // delete the ModuleUnit.

   // moduleUnit->Delete();

   return 0;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Check the commandline variables
//
//-----------------------------------------------------------------------------

void
CheckCmdLine()
{
   if (String::IsNullOrEmpty(
         GlobalPlaceHolder::in->GetValue(nullptr))
      || String::IsNullOrEmpty(
         GlobalPlaceHolder::out->GetValue(nullptr))
      || String::IsNullOrEmpty(
         GlobalPlaceHolder::pdbout->GetValue(nullptr))
      || String::IsNullOrEmpty(
         GlobalPlaceHolder::importdll->GetValue(nullptr))
      || String::IsNullOrEmpty(
         GlobalPlaceHolder::importmethod->GetValue(nullptr))
      || String::IsNullOrEmpty(
         GlobalPlaceHolder::localmethod->GetValue(nullptr)))
   {
      ::Usage();
      Environment::Exit(1);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Initialise the parser variables
//
// Returns:
//
//    Nothing
//
//-----------------------------------------------------------------------------

void
InitCmdLineParser()
{
   GlobalPlaceHolder::in           =
   Phx::Controls::StringControl::New(L"in", L"input file",
      Phx::Controls::Control::MakeFileLineLocationString(L"addcall.cpp", __LINE__));
   GlobalPlaceHolder::out          =
      Phx::Controls::StringControl::New(L"out", L"output file",
         Phx::Controls::Control::MakeFileLineLocationString(L"addcall.cpp", __LINE__));
   GlobalPlaceHolder::pdbout       =
      Phx::Controls::StringControl::New(L"pdbout", L"output pdb file",
         Phx::Controls::Control::MakeFileLineLocationString(L"addcall.cpp", __LINE__));
   GlobalPlaceHolder::importdll    =
      Phx::Controls::StringControl::New(L"importdll",
         L"Dll containing the method to call",
         Phx::Controls::Control::MakeFileLineLocationString(L"addcall.cpp", __LINE__));
   GlobalPlaceHolder::importmethod =
      Phx::Controls::StringControl::New(L"importmethod",
         L"Name of the method to call as it appears in the export list",
         Phx::Controls::Control::MakeFileLineLocationString(L"addcall.cpp", __LINE__));
   GlobalPlaceHolder::localmethod  =
      Phx::Controls::StringControl::New(L"localmethod",
         L"Method at the begining of which to insert the call",
         Phx::Controls::Control::MakeFileLineLocationString(L"addcall.cpp", __LINE__));
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Print the usage string to the console
//
// Returns:
//
//    Nothing
//
//-----------------------------------------------------------------------------

void
Usage()
{
   Phx::Output::WriteLine(L"Usage: AddCall /out <filename> "
      L"/pdbout <filename> /importdll <filename> "
      L"/importmethod <exported method> /localmethod <local method> "
      L"/in <image-name>");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    This method does the work of adding the instrumentation to the function
//
// Remarks:
//
//    Iterates all Units in the IR and Raises them
//    Finds the method specified on the command line and inserts a call
//    instruction to the new import at the start of that method
//    Encodes the Units ready for writing
//
// Returns:
//
//    Nothing
//
//-----------------------------------------------------------------------------

void
DoAddInstrumentation
(
   Phx::PEModuleUnit ^ moduleUnit     // Module being instrumented
)
{
   String ^ dll    = GlobalPlaceHolder::importdll->GetValue(nullptr);
   String ^ method = GlobalPlaceHolder::importmethod->GetValue(nullptr);

   // Create the import

   Phx::Symbols::ImportSymbol ^ importsym = ::AddImport(moduleUnit, dll, method);

   // Get the symbol for the IAT slot (i.e. the Import Address Table entry) to use as the call destination.

   Phx::Symbols::Symbol ^ symIAT = importsym->ImportAddressTableSymbol;

   // Method we want to instrument

   String ^ targetname = GlobalPlaceHolder::localmethod->GetValue(nullptr);

   // Iterate over each function actually has contribution in the module.

   for (Phx::ContributionUnitIterator contribUnitIterator =
         moduleUnit->GetEnumerableContributionUnit().GetEnumerator();
        contribUnitIterator.MoveNext();)
   {
      if (!contribUnitIterator.Current->IsFunctionUnit)
      {
         continue;
      }

      Phx::FunctionUnit ^ functionUnit = contribUnitIterator.Current->AsFunctionUnit;

      // Currently we must raise all FuncUnits when using 
      // Phx::ContributionUnitIterator otherwise resultant binary will not run.

      // Convert from Encoded IR to Low-Level IR

      functionUnit->DisassembleToBeforeLayout();

      // Only instrument the method we are interested in

      if (!String::Equals(targetname, functionUnit->FunctionSymbol->NameString))
      {
         continue;
      }

      // Get the first "real" instruction in the function.

      Phx::IR::Instruction ^ firstinstr = nullptr;

      for each (Phx::IR::Instruction ^ instruction in functionUnit->Instructions)
      {
         // Don't want a LabelInstruction, PragmaInstruction or DataInstruction

         if (instruction->IsReal)
         {
            firstinstr = instruction;
            break;
         }
      }

      // Now create the call to the import

      // As this is via an import we use an the intrinsic Int32Type

      Phx::Types::Type ^ pointerType = functionUnit->TypeTable->Int32Type;

      // Create an alignment for the MemoryOperand

      Phx::Alignment align = Phx::Alignment::NaturalAlignment(pointerType);

      // This is notAliased memory so we can use either NotAliasedMemoryTag
      // or IndirectAliasedMemoryTag

      Phx::Alias::Tag aliasTag = functionUnit->AliasInfo->NotAliasedMemoryTag;

      // We are accessing memory that we consider safe, i.e. will not
      // cause exceptions.

      Phx::Safety::Tag safetyTag = functionUnit->SafetyInfo->SafeTag;

      // Create the MemoryOperand for an indirect call through the IAT

      Phx::IR::MemoryOperand ^ operand = Phx::IR::MemoryOperand::New(functionUnit,
         pointerType, symIAT, nullptr, 0, align, aliasTag, safetyTag);

      // Create a new call instruction. This sample only supports x86, so 
      // we can use the platform specific instruction.

      Phx::IR::CallInstruction ^ call = Phx::IR::CallInstruction::New(
         functionUnit, Phx::Targets::Architectures::X86::Opcode::call,
         operand);

      // Insert it before the instruction

      firstinstr->InsertBefore(call);

      // Now legalize the instruction form

      functionUnit->Legalize->Instruction(call);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Find or Create the ImportSymbol for a method in a dll
//
// Remarks:
//
//    Searches the import list for a matching import
//    If this is not found create the IAT entry and symbol for the import
//
// Returns:
//
//    Phx::Symbols::Imports of the matching import
//
//-----------------------------------------------------------------------------

Phx::Symbols::ImportSymbol ^
AddImport
(
   Phx::PEModuleUnit ^ module,     // Module being instrumented
   String            ^ dll,        // Name of the DLL
   String            ^ method      // Name of the imported method
)
{
   Phx::Name dllname    = Phx::Name::New(module->Lifetime, dll);
   Phx::Name methodname = Phx::Name::New(module->Lifetime, method);

   // Do we have an import module for this DLL already?

   Phx::Symbols::ImportModuleSymbol ^ importmodulesym =
      module->FindImportModule(dllname);

   // If not create a new one

   if (importmodulesym == nullptr)
   {
      importmodulesym = module->AddImportModule(0, dllname,
         false);
   }

   // Check for the existance of this import

   Phx::Symbols::ImportSymbol ^ importsym =
      importmodulesym->FindImport(methodname);

   // If not then create it

   if (importsym == nullptr)
   {
      // First we need to create a symbol for the IAT slot (i.e. the Import
      // Address Table entry) that we will use as the call target.

      // Prepend "_imp_" to the import name

      String ^ importString = String::Concat(L"_imp_", method);

      // Create the name for the symbol

      Phx::Name nameIAT = Phx::Name::New(module->Lifetime, importString);

      // As this is via an import we use an the intrinsic Int32Type

      Phx::Types::Type ^ pointerType = module->TypeTable->Int32Type;

      // Now create the sym

      Phx::Symbols::GlobalVariableSymbol ^ symIAT  =
         Phx::Symbols::GlobalVariableSymbol::New(module->SymbolTable, 0, nameIAT, pointerType,
            Phx::Symbols::Visibility::DllImport);

      // Now we can create the import

      importsym = importmodulesym->AddImport(0, methodname,
         0, symIAT, nullptr);
   }

   // Return the symbol.

   return importsym;
}

