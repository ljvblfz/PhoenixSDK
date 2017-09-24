//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Simple Phoenix based application to add a instrumentation for a simple
//       ProcTrace profiler.
//
//    Input file must be compiled with a phoenix based compiler and -Zi
//
// Usage:
//
//    Usage: ProcTraceInstr /out <filename> /pdbout <filename> [/funcname]
//              /in <image-name>
//
//-----------------------------------------------------------------------------

#include "..\..\common\samples.h"

// Not using any namespaces to show explicit class heirarchy
// using namespace Phx;

// Global declarations

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

public
ref class GlobalPlaceHolder
{
public:
   static Phx::Controls::StringControl ^  in;
   static Phx::Controls::StringControl ^  out;
   static Phx::Controls::StringControl ^  pdbout;
   static Phx::Controls::SetBooleanControl ^ funcname;
};

// Method declarations

Phx::Symbols::ImportSymbol ^
AddImport
(
   Phx::PEModuleUnit ^ module,
   System::String ^    dllname,
   System::String ^    methodname
);

void
DoAddInstrumentation
(
   Phx::PEModuleUnit ^ module,
   bool                doPassFuncName
);

Phx::Types::Type ^
GetExternalFuncType
(
   Phx::PEModuleUnit ^ module,
   bool                doPassFuncName
);

void                   InitCmdLineParser();
Phx::Symbols::GlobalVariableSymbol ^
InsertFuncNameString
(
   Phx::PEModuleUnit ^ module,
   Phx::FunctionUnit     ^ function
);

void
InstrumentMethod
(
   Phx::PEModuleUnit ^ module,
   Phx::FunctionUnit     ^ function,
   Phx::Symbols::Symbol    ^ symIAT,
   Phx::Types::Type  ^ functionType,
   bool                doPassFuncName
);

Phx::PEModuleUnit ^    OpenModule(System::String ^input);
int                    CheckCmdLine();
void                   Usage();

//-----------------------------------------------------------------------------
//
// Description:
//
//    Entry point for application
//
// Returns:
//
//    Nothing
//
//-----------------------------------------------------------------------------

int
main
(
   array<System::String ^> ^ arguments
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

   int ret;

   if (0 != (ret = ::CheckCmdLine()))
   {
      return ret;
   }

   bool doPassFuncName =
      GlobalPlaceHolder::funcname->GetValue(nullptr);

   // Open the module and read it in.

   Phx::PEModuleUnit ^       moduleUnit;

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

   ::DoAddInstrumentation(moduleUnit, doPassFuncName);

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

int
CheckCmdLine()
{
   // Now parse the command-line

   if (System::String::Equals(GlobalPlaceHolder::in->GetValue(nullptr), L"")
      || System::String::Equals(GlobalPlaceHolder::out->GetValue(nullptr), L"")
      || System::String::Equals(GlobalPlaceHolder::pdbout->GetValue(nullptr),
         L""))
   {
      ::Usage();
      return 1;
   }

   return 0;
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
   GlobalPlaceHolder::in       = Phx::Controls::StringControl::New(L"in",
      L"input file",
      Phx::Controls::Control::MakeFileLineLocationString(L"ProcTraceInstr.cpp", __LINE__));
   GlobalPlaceHolder::out      = Phx::Controls::StringControl::New(L"out",
      L"output file",
      Phx::Controls::Control::MakeFileLineLocationString(L"ProcTraceInstr.cpp", __LINE__));
   GlobalPlaceHolder::pdbout   = Phx::Controls::StringControl::New(L"pdbout",
      L"output pdb file",
      Phx::Controls::Control::MakeFileLineLocationString(L"ProcTraceInstr.cpp", __LINE__));
   GlobalPlaceHolder::funcname = Phx::Controls::SetBooleanControl::New(L"funcname",
      L"Pass the function name as a parameter",
      Phx::Controls::Control::MakeFileLineLocationString(L"ProcTraceInstr.cpp", __LINE__));
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
   Phx::Output::WriteLine(L"Usage: ProcTrace /out <filename> "
      L"/pdbout <filename> [/funcname] /in <image-name>");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    This method does the work of adding the instrumentation to the function
//
// Remarks:
//
//    Iterates all IrUnits in the IR and Raises them
//    Calls InstrumentMethod for each FunctionUnit
//    Encodes the IrUnits ready for writing
//
// Returns:
//
//    Nothing
//
//-----------------------------------------------------------------------------

void
DoAddInstrumentation
(
   Phx::PEModuleUnit ^ module,        // Module being instrumented
   bool                doPassFuncName // Pass the name and address of function?
)
{
   System::String ^ dll    = L"ProcTraceRuntime.dll";
   System::String ^ method = nullptr;

   if (doPassFuncName == true)
   {
      method = L"_ProcTrace@8";
   }
   else
   {
      method = L"_ShoutAddr@4";
   }

   // Create the import

   Phx::Symbols::ImportSymbol ^ importsym = ::AddImport(module, dll, method);

   // Get the Type of the external Method

   Phx::Types::Type ^ functionType = ::GetExternalFuncType(module, doPassFuncName);

   // Get the symbol for the IAT slot to use as the call destination

   Phx::Symbols::Symbol ^ symIAT = importsym->ImportAddressTableSymbol;

   // Iterate over each function actually has contribution in the module.

   for (Phx::ContributionUnitIterator contribUnitIterator =
         module->GetEnumerableContributionUnit().GetEnumerator();
        contribUnitIterator.MoveNext();)
   {
      if (!contribUnitIterator.Current->IsFunctionUnit)
      {
         continue;
      }

      Phx::FunctionUnit ^ functionUnit = contribUnitIterator.Current->AsFunctionUnit;

      functionUnit->DisassembleToBeforeLayout();

      ::InstrumentMethod(module, functionUnit, symIAT, functionType,
         doPassFuncName);
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
   Phx::PEModuleUnit ^ module, // Module being instrumented
   System::String ^    dll,    // Name of the DLL
   System::String ^    method  // Name of the imported method
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
      importmodulesym = module->AddImportModule((Phx::ExternId)0, dllname,
         false);
   }

   // Check for the existance of this import

   Phx::Symbols::ImportSymbol ^ importsym =
      importmodulesym->FindImport(methodname);

   // If not then create it

   if (importsym == nullptr)
   {
      // 1st we need to create a symbol for the IAT slot that we will use
      // as the call target

      // Prepend "_imp_" to the import name

      System::String ^importString = System::String::Concat(L"_imp_", method);

      // Create the name for the symbol

      Phx::Name nameIAT = Phx::Name::New(module->Lifetime, importString);

      // As this is via an import we use an the intrinsic Int32Type

      Phx::Types::Type ^ pointerType = module->TypeTable->Int32Type;

      // Create the sym

      Phx::Symbols::GlobalVariableSymbol ^ symIAT  =
         Phx::Symbols::GlobalVariableSymbol::New(module->SymbolTable, 0, nameIAT, pointerType,
            Phx::Symbols::Visibility::DllImport);

      // Now we can create the import

      importsym = importmodulesym->AddImport((Phx::ExternId)0, methodname,
         (System::UInt16)0, symIAT, nullptr);
   }

   // Return the symbol.

   return importsym;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Get a type representing the prototype of the external method
//
//    Declaration depends on doPassFuncName flag :
//    _stdcall void Function(int currentAddress);
//    or
//    _stdcall void Function(int currentAddress, wchar_t* functionName);
//
// Remarks:
//
//    Creates a FunctionTypeBuilder
//    Sets the calling convention, return type and parameter list of the
//       external method
//
// Returns:
//
//    Types::Type representing the external method
//
//-----------------------------------------------------------------------------

Phx::Types::Type ^
GetExternalFuncType
(
   Phx::PEModuleUnit ^ module,        // Module being instrumented
   bool                doPassFuncName // Pass the name and address of function?
)
{
   // Get the type table

   Phx::Types::Table ^ typeTable = module->TypeTable;

   // We need to build up the prototype for the function...

   Phx::Types::FunctionTypeBuilder ^ functionTypeBuilder;

   functionTypeBuilder = Phx::Types::FunctionTypeBuilder::New(typeTable);

   functionTypeBuilder->Begin();

   // Set the calling convention

   // We have to use Stdcall here as this is the only calling convention that
   // will allow us to preserve the registers and flags across the call.
   // This is important as we are not raising the IR high enough to have the
   // lower process smooth any of the issues over.

   functionTypeBuilder->CallingConventionKind =
      Phx::Types::CallingConventionKind::Stdcall;

   // Set the return type - void

   functionTypeBuilder->AppendReturnParameter(typeTable->VoidType);

   // Set the param types..

   // Current Address - use UIntPointerPrimitiveType

   functionTypeBuilder->AppendParameter(typeTable->UIntPointerPrimitiveType);

   if (doPassFuncName == true)
   {
      // wchar_t *

      functionTypeBuilder->AppendParameter(
            typeTable->GetPointerType(Phx::Types::PointerTypeKind::UnmanagedPointer,
               8,
               typeTable->Character16Type));
   }

   // Return the Type representing the Function prototype

   return functionTypeBuilder->GetFunctionType();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Insert the name of the FunctionUnit into the IR as a piece of data
//
// Remarks:
//
//    Creates a Data instruction
//    Copies the contents of the Symbol name into the instruction
//    Creates a new GlobalVariableSymbol associated with the new data
//    Inserts the instruction into the IR
//
// Returns:
//
//    Phx::Symbols::GlobalVariableSymbol for the string
//
//-----------------------------------------------------------------------------

Phx::Symbols::GlobalVariableSymbol ^
InsertFuncNameString
(
   Phx::PEModuleUnit ^ module,  // Module being instrumented
   Phx::FunctionUnit     ^ function     // FunctionUnit from which to take the name
)
{
   // Create a GlobalVariableSymbol for the instruction.

   Phx::Symbols::GlobalVariableSymbol ^ stringSym = Phx::Symbols::GlobalVariableSymbol::New(
      module->SymbolTable, 0,
      Phx::Name::New(module->Lifetime, L"String"), module->TypeTable->UnknownType,
      Phx::Symbols::Visibility::File);

   // Set default alignment on the Symbol

   stringSym->Alignment = Phx::Alignment(Phx::Alignment::Kind::AlignTo1Byte);

   // Set the section symbol for the GlobalVariableSymbol
   // The strings are being inserted in the .text section, i.e. the same section
   // as the FunctionUnit

   Phx::Symbols::SectionSymbol ^ sectionSymbol = function->FunctionSymbol->AllocationBaseSectionSymbol;

   stringSym->AllocationBaseSectionSymbol = sectionSymbol;

   // Insert the Name of the FunctionUnit into the DataInstruction

   int              length = 0;
   System::String ^ string = function->FunctionSymbol->Name.NameString;

   if (string != nullptr)
   {
      length = string->Length;
   }

   // Create the DataInstruction

   Phx::IR::DataInstruction ^ stringInstr = Phx::IR::DataInstruction::New(
      sectionSymbol->Section->DataUnit, (length + 1) * sizeof(Phx::Character));

   // Copy the data into the DataInstruction

   Phx::ByteOffset charCount = 0;

   for (; charCount < length; charCount++)
   {
      // Use WriteInt16 to output Unicode strings to the buffer.
      
      stringInstr->WriteInt16(charCount * sizeof(Phx::Character), string->default[charCount]);
   }

   // Add a location to the symbol

   stringSym->Location = Phx::Symbols::DataLocation::New(stringInstr);

   // Insert the Instruction into the DataUnit

   sectionSymbol->Section->AppendInstruction(stringInstr);

   return stringSym;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Insert the instrumentation into the FunctionUnit
//
// Remarks:
//
//    Creates an Hir call instruction
//    Appends the appropriate parameters to it in the form of Operands
//    Inserts the instruction into the IR
//    Lowers the instruction to LIR
//
// Returns:
//
//    Nothing
//
//-----------------------------------------------------------------------------

void
InstrumentMethod
(
   Phx::PEModuleUnit ^ module,        // Module being instrumented
   Phx::FunctionUnit     ^ function,          // FunctionUnit to instrument
   Phx::Symbols::Symbol    ^ symIAT,        // Symbol of the import to call
   Phx::Types::Type  ^ functionType,      // FunctionType of the import
   bool                doPassFuncName // Pass the name and address of function?
)
{
   Phx::IR::Instruction ^ firstinstr = nullptr;

   for each (Phx::IR::Instruction ^ instruction in function->Instructions)
   {
      // Don't want a LabelInstruction, PragmaInstruction or DataInstruction

      if (instruction->IsReal)
      {
         firstinstr = instruction;
         break;
      }
   }

   // Now create the call to the import

   // Create an alignment for the MemoryOperand

   Phx::Alignment align = Phx::Alignment(Phx::Alignment::Kind::AlignTo1Byte);

   // This is NotAliased memory so we can use either NotAliasedMemoryTag
   // or IndirectAliasedMemoryTag

   Phx::Alias::Tag aliasTag = function->AliasInfo->NotAliasedMemoryTag;

   // We are accessing memory that we consider safe, i.e. will not
   // cause exceptions.

   Phx::Safety::Tag safetyTag = function->SafetyInfo->SafeTag;

   // Create the MemoryOperand for an indirect call through the IAT

   Phx::IR::MemoryOperand ^ opndCallTgt = Phx::IR::MemoryOperand::New(function,
      functionType, symIAT, nullptr, 0, align, aliasTag, safetyTag);

   // Create a new call instruction

   Phx::IR::CallInstruction ^call = Phx::IR::CallInstruction::New(
      function, Phx::Common::Opcode::Call, opndCallTgt);

   // Append the Operands for the parameters...

   // as this is via an import we use an the intrinsic UnmanagedPointerType

   Phx::Types::Type ^ pointerType = function->TypeTable->DefaultUnmanagedPointerType;

   Phx::IR::MemoryOperand ^ opndFuncAddress = Phx::IR::MemoryOperand::NewAddress(function,
      pointerType, function->FunctionSymbol, nullptr, 0,
      Phx::Alignment(Phx::Alignment::Kind::AlignTo4Bytes), aliasTag);

   call->AppendSource(opndFuncAddress);

   if (doPassFuncName == true)
   {
      // Insert the function name as a String into the IR for use later

      Phx::Symbols::GlobalVariableSymbol ^ stringSym = ::InsertFuncNameString(module,
         function);

      // now append the string pointer

      Phx::IR::MemoryOperand ^ opndStringAddress = Phx::IR::MemoryOperand::NewAddress(
         function, pointerType, stringSym, nullptr, 0,
         Phx::Alignment(Phx::Alignment::Kind::AlignTo1Byte), aliasTag);

      call->AppendSource(opndStringAddress);
   }

   // Insert it before the instruction

   firstinstr->InsertBefore(call);

   // Lower the instruction to the LIR of the Target platform

   function->Lower->Instruction(call);
}

