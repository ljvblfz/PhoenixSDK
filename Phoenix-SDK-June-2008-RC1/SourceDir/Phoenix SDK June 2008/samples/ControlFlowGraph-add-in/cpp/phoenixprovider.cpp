//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Phoenix subsystem of the add-in.  Provides Phoenix services to the
//    rest of the add-in.
//
//------------------------------------------------------------------------------

#include "phoenixprovider.h"

namespace ControlFlowGraph
{

//------------------------------------------------------------------------------
//
// Description:
//
//    Default constructor.
//
//------------------------------------------------------------------------------

PhoenixProvider::PhoenixProvider()
{
   if (!Phx::GlobalData::PhoenixIsInitialized)
   {
      InitializePhoenix();
   }
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Returns the flow graph of the function specified by the function
//    name, type name, and parameter types.
//
// Arguments:
//
//    binaryFile - The full path of the binary file that contains the
//    function
//    pdbFile - The pdb that contains information about binaryFile.
//    targetFile - The path of the file that contains the definition of
//    of the function that we are trying to find.
//    targetLine - The line number of the function that we are trying to
//    find.
//
// Returns:
//
//    The flow graph of the function in targetFile closest to targetLine.
//
// Remarks:
//
//    GetFlowGraph loads the binary and pdb specified by the file name and
//    pdb, then calls FindClosestFuncSym to retrieve the functional unit
//    corresponding to
//
//------------------------------------------------------------------------------

Phx::Graphs::FlowGraph^
PhoenixProvider::GetFlowGraph
(
   String ^      binaryFile,
   String ^      pdbFile,
   String ^      targetFile,
   unsigned int  targetLine
)
{

   // The module representation of the binary

   Phx::PEModuleUnit ^  peModuleUnit;

   try
   {
      // Open the pe using the pe module unit reader.  Give it the pdb and
      // binary output file of this solution.  Note that the pdb must
      // be a full profile PDB for an unmanaged binary.

      peModuleUnit = Phx::PEModuleUnit::Open(binaryFile, pdbFile,
         Phx::Symbols::SymbolicInfoLevel::IsForOptimisticAnalysisOnly,
         nullptr);
      peModuleUnit->LoadEncodedIRUnitList();

      // Find the symbol in the available scopes of the assembly
      // that is closest to the target line and in the target file.

      Phx::FunctionUnit ^ functionUnit = nullptr;

      functionUnit =
         this->FindClosestFuncUnit(targetFile, targetLine, peModuleUnit);

      // If we couldn't find a function with the specified contraints, just
      // return a null flow graph.

      if (functionUnit == nullptr)
      {
         return nullptr;
      }

      // Next, retrieve the flow graph.  Retrieve the without
      // exception handling edges to avoid a messy graph.

      functionUnit->BuildFlowGraphWithoutEH();

      peModuleUnit->Close();

      return functionUnit->FlowGraph;
   }
   catch(Exception^)
   {
      // Something went wrong when retrieving the flow graph, just
      // return null, indicating no flow graph could be obtained

      return nullptr;
   }
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Helper method that locates the function in the assembly that is closest 
//    to the line indicated by targetLine in targetFile.
//
// Arguments:
//
//    targetFile - The file that contains the function that we are locating.
//    targetLine - The line that the function is on.  This need not be exact.
//    For instance, Intellisense will provide a line number that corresponds
//    to the header line of the function definition, while the pdb line
//    numbers correspond to the first executable line of the function.
//    pe - The PEModuleUnit that contains the function.
//
// Returns:
//
//    The Phx::FunctionUnit of the function closest to the target line in the target
//    file.
//
// Remarks:
//
//    FindClosestFuncUnit walks the functional units in the pe module and
//    finds the set closest to the target line in the target file.
//
//------------------------------------------------------------------------------

Phx::FunctionUnit^
PhoenixProvider::FindClosestFuncUnit
(
   String ^            targetFile,
   unsigned int        targetLine,
   Phx::PEModuleUnit ^ pe
)
{
   unsigned int      closestLine = (unsigned int)-1;
   Phx::FunctionUnit ^   closestFuncUnit = nullptr;

   // Iterate through each of the contributing functional units in the PE
   // and enumerate its file name and line number.

   for each (Phx::Unit ^ unit in pe->ChildUnits)
   {
      if (!unit->IsFunctionUnit)
      {
         continue;
      }

      Phx::FunctionUnit ^ functionUnit = unit->AsFunctionUnit;

      pe->Context->PushUnit(functionUnit);

      // The method must be "raised" to obtain its debug information.

      functionUnit =
         pe->Raise(functionUnit->FunctionSymbol, Phx::FunctionUnit::SymbolicFunctionUnitState);

      pe->Context->PopUnit();

      String ^ file =
         functionUnit->DebugInfo->GetFileName(functionUnit->FunctionSymbol->DebugTag);

      // Check it against our target file name.

      if (!String::Compare(file, targetFile,
            StringComparison::CurrentCultureIgnoreCase))
      {
         // Now get the line, and check to see whether it's even a
         // candidate.  It must be on or after the target line.  If it
         // is, set the return parameters and return;

         int line =
            functionUnit->DebugInfo->GetLineNumber(functionUnit->FunctionSymbol->DebugTag);

         long long symLineDiff = line - targetLine;
         long long closestSymLineDiff = closestLine - targetLine;

         if ((symLineDiff < closestSymLineDiff) && (symLineDiff >= 0))
         {
            closestLine = line;
            closestFuncUnit = functionUnit;
         }
      }
   }

   return closestFuncUnit;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Initializes Phoenix.
//
//------------------------------------------------------------------------------

void
PhoenixProvider::InitializePhoenix()
{
   // Initialize the targets first

   InitializeTargets();

   // Start initializing Phoenix before doing anything else

   Phx::Initialize::BeginInitialization();

   // Initialize the controls

   Phx::Initialize::EndInitialization(L"PHX|*|_PHX_|", 0, nullptr);

   // Enable raise option to stay compatible with dynamic slice tool.

   Phx::Controls::Parser::ParseArgumentString(nullptr, "/raise");
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Registers the targets available in the RDK.
//
//------------------------------------------------------------------------------

void
PhoenixProvider::InitializeTargets()
{
   // Register all the targets and architectures that
   // are possible with the RDK

   Phx::Targets::Architectures::Architecture ^ x86Arch =
      Phx::Targets::Architectures::X86::Architecture::New();
   Phx::Targets::Runtimes::Runtime ^ win32x86Runtime =
      Phx::Targets::Runtimes::Vccrt::Win32::X86::Runtime::New(x86Arch);

   Phx::GlobalData::RegisterTargetArchitecture(x86Arch);
   Phx::GlobalData::RegisterTargetRuntime(win32x86Runtime);

   Phx::Targets::Architectures::Architecture ^ msilArch =
      Phx::Targets::Architectures::Msil::Architecture::New();
   Phx::Targets::Runtimes::Runtime ^ winMSILRuntime =
      Phx::Targets::Runtimes::Vccrt::Win::Msil::Runtime::New(msilArch);
   Phx::GlobalData::RegisterTargetArchitecture(msilArch);
   Phx::GlobalData::RegisterTargetRuntime(winMSILRuntime);
}

} // ControlFlowGraph
