//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//   Detect uninitialized local variable uses.
//
// Usage:
//
//   cl -c <compiland>
//      -d2 plugin:"<assemblyPath>\UninitializedLocal.dll"
//    [ -d2 {dump|predump}:UninitializedLocal ]
//    [ -d2 WarnMayUninit ]
//
//   UninitializedLocal.dll gets generated in the Debug or Release
//   subdirectory of UninitializedLocal\cpp.
//
// Remarks:
//
//   The UninitializedLocal plugin consists of a Plugin class and a Phase
//   class.
//   
//   The Plugin class creates the controls UninitializedLocal and WarnMayUninit
//   and is inserted after the CxxIL Reader phase as an instance of the Phase
//   class.
//
//   The workhorse of the Phase class is the method Execute, which traverses
//   all the uses of the fictitious "undefined memory" definition provided by
//   Phoenix's Single Static Assignment information.
//
//   Uses reached through an SSA Phi function represent variables that may be
//   uninitialized when used. The control WarnMayUninit, if present on the
//   command line, indicates that these potential errors are to be reported.
//
//------------------------------------------------------------------------------

#include "UninitializedLocal.h"

namespace UninitializedLocal
{

//-----------------------------------------------------------------------------
//
// Description:
//
//    New creates an instance of a phase.  Following Phoenix guidelines,
//    New is static.
//
// Arguments:
//
//    config - [in] A pointer to a Phases::PhaseConfiguration that provides
//          properties for retrieving the initial phase list.
//
// Returns:
//
//    A pointer to the new phase.
//
//-----------------------------------------------------------------------------

Phase ^
Phase::New
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   Phase ^ phase = gcnew Phase();

   phase->Initialize(config, L"Detect uninitialized local variable uses");
   phase->hasPrintedFileName = false;

#if defined(PHX_DEBUG_SUPPORT)

   phase->PhaseControl = Phase::UninitializedLocalCtrl;

#endif

   return phase;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Execute is the phase's prime mover; all unit-centric processing occurs
//    here.  Note that Execute might be thought of as a "callback": as the
//    C2 host compiles each FunctionUnit, passing it from phase to phase, the
//    plug-in Execute method is called to do its work.
//
// Arguments:
//
//    unit - [in] The unit to process.
//
// Remarks:
//
//    Since IR exists only at the FunctionUnit level, we ignore ModuleUnits.
//
//    The order of units in a compiland passed to Execute is indeterminate.
//
//-----------------------------------------------------------------------------

void
Phase::Execute
(
   Phx::Unit ^ unit
)
{
   // Only FunctionUnit's have associated SSA info.

   if (!unit->IsFunctionUnit)
   {
      return;
   }

   Phx::FunctionUnit ^ functionUnit = unit->AsFunctionUnit;

   // Unless a previous phase has built SSA info, build it now.

   bool isSsaCleanUpRequired = false;
   if (!functionUnit->SsaInfo)
   {
      functionUnit->BuildSsaInfo(Phx::SSA::BuildOptions::DefaultNotAliased);

      // Record the fact that SSA info was built specifically for this method,
      // in which case it will need to be cleaned up before returning.

      isSsaCleanUpRequired = true;
   }

   // Gather uses of the fictitious "undefined memory" definition in the set
   // mustList. (The set mayList will be used for those uses that are
   // destination operands of a Phi function.)

   System::Collections::ArrayList ^ mustList =
      gcnew System::Collections::ArrayList();
   System::Collections::ArrayList ^ mayList =
      gcnew System::Collections::ArrayList();

   // Uses of UndefinedDefinitionOperand are gathered by following its definition-use chain.

   for (Phx::IR::Operand ^ useOperand = functionUnit->SsaInfo->UndefinedDefinitionOperand->UseOperand;
        useOperand != nullptr;
        useOperand = useOperand->UseOperand)
   {
      if (useOperand->IsVariableOperand)
      {
         Phx::Symbols::Symbol ^ symUse = useOperand->AsVariableOperand->Symbol;

         if (symUse
            && !symUse->Name.IsNull
            && !mustList->Contains(symUse)
			&& !mayList->Contains(symUse))
         {
            mustList->Add(symUse);
         }
      }

      // A variable could be an undefined source operand of
      // a Phi function (of class AliasOperand). If so, we will remove the reference to
	  // the "undefined memory" MustList if it already exists there, and store the reference
	  // as a "potentially" undefined referece as defined by the MayList.  We will report the
      // Phi destination operands as undefined provided the control WarnMayUninit
      // was specified on the command line.

      if (useOperand->Instruction->Opcode == Phx::Common::Opcode::Phi)
      {
         for each (Phx::IR::Operand ^ phiDst in useOperand->Instruction->DestinationOperands)
         {
            if (phiDst->IsVariableOperand)
            {
               Phx::Symbols::Symbol ^ dstSym = phiDst->AsVariableOperand->Symbol;

               if (dstSym
                  && !dstSym->Name.IsNull
                  && !mayList->Contains(dstSym))
               {
                  mayList->Add(dstSym);
				  if (mustList->Contains(dstSym))
				  {
					  mustList->Remove(dstSym);
				  }
               }
            }
         }
      }
   }

   // Issue first must warnings, then may warnings for variables
   // not yet reported.

   if ((mustList->Count > 0) || ((mayList->Count > 0) && WarnMayCtrl->IsEnabled(nullptr)))
   {
      if (!this->hasPrintedFileName)
      {
         Phx::Output::WriteLine(
            L"While compiling {0}", functionUnit->ParentModuleUnit->NameString);
         this->hasPrintedFileName = true;
      }
      Phx::Output::WriteLine(
         L"In function {0}", Phx::Utility::Undecorate(functionUnit->NameString, true));
   }

   for each (Phx::Symbols::Symbol ^ sym in mustList)
   {
      // Must overrides may.

      mayList->Remove(sym);

      Phx::Output::WriteLine(
         L"Warning C4700: local variable '{0}' "
         L"used without having been initialized.",
         sym->Name.NameString);
   }

   if (WarnMayCtrl->IsEnabled(nullptr))
   {
      for each (Phx::Symbols::Symbol ^ sym in mayList)
      {
         Phx::Output::WriteLine(
            L"Warning C4701: local variable '{0}' may "
            L"be used without having been initialized.",
            sym->Name.NameString);
      }
   }

   // Clean up SSA information if specifically built for this function.

   if (isSsaCleanUpRequired)
   {
      functionUnit->DeleteSsaInfo();
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    RegisterObjects initializes the plug-in's environment.  Normally, this
//    includes defining your command-line switches (controls) that should be
//    handled by Phoenix.  Phoenix calls this method early, upon loading the
//    plug-in's DLL.
//
// Remarks:
//
//    The RegisterObjects method is not the place to deal with phase-specific
//    issues, because the host has not yet built its phase list.
//    However, controls ARE phase-specific.  Because your phase object does
//    not exist yet, your phase's controls must be static fields,
//    accessable from here.
//
//-----------------------------------------------------------------------------

void
PlugIn::RegisterObjects()
{

   Phase::WarnMayCtrl =
      Phx::Controls::SetBooleanControl::New(L"WarnMayUninit",
         L"Warn on possibly uninitialized uses of local variables",
         L"UninitializedLocal.cpp");

#if defined(PHX_DEBUG_SUPPORT)

   Phase::UninitializedLocalCtrl =
      Phx::Controls::ComponentControl::New(L"UninitializedLocal",
         L"Detect uninitialized local variable uses",
         L"UninitializedLocal.cpp");

#endif

}

//-----------------------------------------------------------------------------
//
// Description:
//
//    BuildPhases is where the plug-in creates and initializes its phase
//    object(s), and inserts them into the phase list
//    already created by the c2 host.
//
// Arguments:
//
//    config - [in] A pointer to a Phases::PhaseConfiguration that provides
//          properties for retrieving the initial phase list
//
// Remarks:
//
//    Your plug-in determines a new phase's place in the list by locating
//    an existing phase by name and inserting the new phase before or after it.
//    A phase is created by its static New method.  One may place the insertion
//    code here in BuildPhases, or delegate it to the phase's New method.
//    Phoenix places few requirements on phase construction, so you are free
//    to choose the most reasonable approach for your needs.
//
//    Since we are inserting multiple instances of this phase, we'll build
//    their names from the name of the phases they follow.
//
//    The Phoenix framework has parsed the command line, so control values are
//    available at this point.
//
//-----------------------------------------------------------------------------

void
PlugIn::BuildPhases
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   Phx::Phases::Phase ^ basePhase;

   // Replace Uninitialized Local Use Detection Phase

   basePhase =
      config->PhaseList->FindByName(L"CxxIL Reader");
   if (basePhase == nullptr)
   {
      Phx::Output::WriteLine(L"CxxIL Reader phase "
         L"not found in phaselist:");
      Phx::Output::Write(config->ToString());

      return;
   }

   Phx::Phases::Phase ^ phase = Phase::New(config);
   basePhase->InsertAfter(phase);
}
}
