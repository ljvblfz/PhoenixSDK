//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Phoenix Plug-in sample.
//
// Remarks:
//
//    Registers "addnops" component control.
//
//    Injects a phase after Lower which injects a NOP prior to every
//    instruction.
//
// Usage:
//
//    cl ... -d2plugin:addnop-plug-in.dll
//
//------------------------------------------------------------------------------

#include "addnop-plug-in.h"

//------------------------------------------------------------------------------
//
// Description:
//
//    Registers any controls needed by the plug-in.
//
//------------------------------------------------------------------------------

void
AddNOPPlugIn::RegisterObjects()
{

#if defined(PHX_DEBUG_SUPPORT)

   AddNOPPhase::AddNOPCtrl = Phx::Controls::ComponentControl::New("addnops",
      "Inject a NOP before each instruction; called after Lower",
      "addnop-plug-in.cpp");

#endif

}

//------------------------------------------------------------------------------
//
// Description:
//
//    Inspects and modifies the phase listing of the host (i.e. a Phoenix
//    based compiler).
//
// Arguments:
//
//    config - Encapsulating object that simplifies handling of
//    the phase list and pre and post phase events.
//
//------------------------------------------------------------------------------

void
AddNOPPlugIn::BuildPhases
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   Phx::Phases::Phase ^ readerPhase;

   readerPhase = config->PhaseList->FindByName("Lower");
   if (readerPhase == nullptr)
   {

      Phx::Output::WriteLine("Lower phase not found in phaselist:");
      Phx::Output::Write(config->ToString());

      return;
   }

   Phx::Phases::Phase ^ phase = AddNOPPhase::New(config);
   readerPhase->InsertAfter(phase);
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Create a new AddNOPPhase object.
//
// Arguments:
//
//    config - The encapsulating object that simplifies handling of
//    the phase list and pre and post phase events.
//
// Returns:
//
//    A new AddNOPPhase object.
//
//------------------------------------------------------------------------------

AddNOPPhase ^
AddNOPPhase::New
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   AddNOPPhase ^ phase = gcnew AddNOPPhase();

   phase->Initialize(config, "Add NOPs");

#if defined(PHX_DEBUG_SUPPORT)

   phase->PhaseControl = AddNOPPhase::AddNOPCtrl;

#endif

   return phase;
}

//------------------------------------------------------------------------------
// 
// Description:
//
//    Execute the phase, inserting nop's before every instruction in
//    each function unit.
//
// Arguments:
//
//    unit - The unit this phase is operating on.
//
//------------------------------------------------------------------------------

void
AddNOPPhase::Execute
(
   Phx::Unit ^ unit
)
{
   int CountNops = 0;

   if (!unit->IsFunctionUnit)
   {
      return;
   }

   Phx::FunctionUnit ^ functionUnit = unit->AsFunctionUnit;

   Phx::IR::Instruction ^ nopInstruction;

   for each (Phx::IR::Instruction ^ instruction in functionUnit->Instructions)
   {
      if (instruction->Opcode == Phx::Common::Opcode::ExitBody)
      {
         // Quit when we get to ExitBody pseudo inst;
         // otherwise we'll mess up the Unwind code.

         break;
      }

      if (!instruction->IsReal)
      {
         continue;
      }

      // Create an architecture-neutral (Hir) nop, then lower to
      // make it architecture specific (LIR). Note that targets
      // that do not override the default lowering will actually
      // end up removing the Hir nop in lower instead of producing
      // the target-specific LIR equivalent.

      instruction->SetCurrentDebugTag();
      nopInstruction = Phx::IR::ValueInstruction::New(functionUnit, Phx::Common::Opcode::Nop);
      instruction->InsertBefore(nopInstruction);
      CountNops++;
      functionUnit->Lower->Instruction(nopInstruction);
   }
   Phx::Output::WriteLine("AddNop added {0} nops to {1}", 
      CountNops, functionUnit->NameString);
}
