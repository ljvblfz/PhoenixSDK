//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Simple Managed Phoenix based application to add nop's to an existing 
//    program
//
//    Input file must be compiled with a phoenix based compiler and -Zi
//
// Usage:
//    Addnop-tool [/ShowResults] <input-binary> <output-binary>
//    
//    Arguments in [] are optional.
//      /ShowResults   Dumps IR before and after instrumentation
//
//-----------------------------------------------------------------------------

// Not using Phx namespace to show explicit class heirarchy

#pragma region Using namespace declarations

using namespace System;

#pragma endregion

#include "addnop-tool.h"

//---------------------------------------------------------------------------
//
// Description:
//
//    Registers the targets availalable in the RDK.
//
// Returns:
//
//    Nothing
//
//---------------------------------------------------------------------------

void
InitializePhx()
{
   Phx::Targets::Architectures::Architecture ^ msilArch =
      Phx::Targets::Architectures::Msil::Architecture::New();
   Phx::GlobalData::RegisterTargetArchitecture(msilArch);

   Phx::Targets::Architectures::Architecture ^ x86Arch =
      Phx::Targets::Architectures::X86::Architecture::New();
   Phx::GlobalData::RegisterTargetArchitecture(x86Arch);

   Phx::Targets::Runtimes::Runtime ^ msilRuntime =
      Phx::Targets::Runtimes::Vccrt::Win::Msil::Runtime::New(msilArch);
   Phx::GlobalData::RegisterTargetRuntime(msilRuntime);

   Phx::Targets::Runtimes::Runtime ^ x86Runtime =
      Phx::Targets::Runtimes::Vccrt::Win32::X86::Runtime::New(x86Arch);
   Phx::GlobalData::RegisterTargetRuntime(x86Runtime);

   // Initialize the Phx infrastructure

   Phx::Initialize::BeginInitialization();
   
   // Initialize control
   
   DumpIRControl::StaticInitialize();
}

//---------------------------------------------------------------------------
//
// Description:
//
//    Initialize control. If enabled, it will dump IR for every functionUnit.
//
// Returns:
//
//    Nothing
//
//---------------------------------------------------------------------------

void
DumpIRControl::StaticInitialize()
{
   DumpIRControl::ShowResultsCtrl = Phx::Controls::SetBooleanControl::New(L"ShowResults",
      L"Dump IR before and after instrumentation",
      L"DumpIRControl::StaticInitialize");
}

//--------------------------------------------------------------------------
//
// Description:
//
//    This method does the work of adding Nop's before every real 
//    instruction.
//
// Arguments:
//
//    module - Input module.
//
// Returns:
//
//    Nothing
//
//--------------------------------------------------------------------------

void
DoAddNop
(
   Phx::PEModuleUnit ^ module
)
{
   // Iterate over each function actually has contribution in the module.

   for each (Phx::FunctionUnit ^ functionUnit in module->GetEnumerableContributionUnit(
         Phx::ContributionUnitEnumerationKind::WritableFunctionUnit))
   {
      functionUnit->DisassembleToBeforeLayout();

      // Do not modify the block for UnmanagedEntryPoint!
      // It should stay the same before and after instrumentation.
      // Modification of UnmanagedEntryPoint functionUnit
      // will make the instrumented App to fail to initialize properly
      // on execution.

      if (functionUnit->NameString->Equals("UnmanagedEntryPoint"))
      {
         continue;
      }
      
      // Dump IR for all funcUnits before instrumentation

      if (DumpIRControl::ShowResultsCtrl->IsEnabled(functionUnit))
      {
         Console::WriteLine("-----DUMP IR BEFORE INSERTING NOPs-----");
      
         for each (Phx::IR::Instruction ^ instruction in functionUnit->Instructions)
         {
            if (!instruction->IsDataInstruction)
            {
               instruction->Dump();
            }
         }
      }

      // Iterate the instructions

      for each (Phx::IR::Instruction ^ instruction in functionUnit->Instructions)
      {
         // HasFallThrough means that the previous instruction can transfer
         // control to this instruction directly.
         // This way we should add nop for the first RealInstruction in the list.
         // One special cases:
         //
         // 1. Don't want to insert nop before the ExitTypeFilter case.
         // A typefilter doesnt generate any real code, and the filter block
         // needs multiple instructions because a label cant have flow.
         // Inserting real code in this block is an error though since
         // it wont make it into the final image.
         //

         if (instruction->IsReal
            && instruction->Previous->HasFallThrough 
            && (instruction->Opcode != Phx::Common::Opcode::ExitTypeFilter))
         {
            // Create a new Nop

            Phx::IR::ValueInstruction ^ nop =
               Phx::IR::ValueInstruction::New(functionUnit, Phx::Common::Opcode::Nop);

            // Insert it before the instruction

            instruction->InsertBefore(nop);

            // Let lower find right instruction opcode.

            functionUnit->Lower->Instruction(nop);
         }
      }
      
      // Dump IR for all funcUnits after instrumentation

      if (DumpIRControl::ShowResultsCtrl->IsEnabled(functionUnit))
      {
         Console::WriteLine("-----DUMP IR AFTER INSERTING NOPs-----");
      
         for each (Phx::IR::Instruction ^ instruction in functionUnit->Instructions)
         {
            if (!instruction->IsDataInstruction)
            {
               instruction->Dump();
            }
         }
      }
   }
}

//--------------------------------------------------------------------------
//
// Description:
//
//    A control that captures the command-line arguments that do not
//    match any other controls.
//
// Returns:
//
//    Nothing
//
//--------------------------------------------------------------------------

void
CmdLineParser::DefaultControl::Process
(
   String ^ string
)
{
   if (CmdLineParser::inFile == nullptr)
   {
      CmdLineParser::inFile = string;
   }
   else if (CmdLineParser::outFile == nullptr)
   {
      CmdLineParser::outFile = string;
   }
   else
   {
      Usage();
      throw(gcnew Exception());
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Parse and check the command-line variables
//
// Returns:
//
//    Termination mode
//
//-----------------------------------------------------------------------------

Phx::Term::Mode
CmdLineParser::ParseCmdLine
(
   array<String ^> ^ argv
)
{
   DefaultControl ^ defaultControl = gcnew DefaultControl();
   Phx::Controls::Parser::RegisterDefaultControl(defaultControl);

   // Check for Phoenix wide options first

   Phx::Initialize::EndInitialization("PHX|*|_PHX_|", argv);

   if ((inFile == nullptr) || (outFile == nullptr))
   {
      Usage();
      return Phx::Term::Mode::Fatal;
   }

   return Phx::Term::Mode::Normal;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Print the usage String ^ to the console
//
// Returns:
//
//    Nothing
//
//-----------------------------------------------------------------------------

void
CmdLineParser::Usage()
{
   String ^ usage =
      L"\nUsage: AddNop-tool [/ShowResults] <input-binary> <output-binary>\n";

   Phx::Output::WriteLine(usage);
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Entry point for application
//
// Arguments:
//
//    argv - Command-line arguments
//
//--------------------------------------------------------------------------

int
main
(
   array<String ^> ^ argv
)
{
   Phx::Term::Mode termMode = Phx::Term::Mode::Normal;

   try
   {
      ::InitializePhx();

      // Parsing command-line options

      termMode = CmdLineParser::ParseCmdLine(argv);

      if (termMode == Phx::Term::Mode::Fatal)
      {
         Phx::Term::All(termMode);
         return ((termMode == Phx::Term::Mode::Normal) ? 0 : 1);
      }
      
      Phx::PEModuleUnit ^ module = nullptr;

      // Open the module

      module = Phx::PEModuleUnit::Open(CmdLineParser::inFile);

      // Set up the writer

      module->OutputImagePath = CmdLineParser::outFile;

      // Do some useful work on the tool front here

      ::DoAddNop(module);

      // Write out the new binary

      module->Close();
   }
   catch (Exception ^ e)
   {
	  Console::WriteLine("An Error was encountered while processing '{1}'.\nException details: {0}", 
		 e, CmdLineParser::inFile);
      termMode = Phx::Term::Mode::Fatal;
   }
   
   // clean up Phx and terminate

   Phx::Term::All(termMode);
   return ((termMode == Phx::Term::Mode::Normal) ? 0 : 1);
}
