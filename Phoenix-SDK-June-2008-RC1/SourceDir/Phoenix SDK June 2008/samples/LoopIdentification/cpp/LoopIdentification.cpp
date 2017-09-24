//-----------------------------------------------------------------------------
//
// Description:
//
//   Identify loops in the code
//
// Remarks:
//
//    Registers the "LoopIdentification" component control.
//
//-----------------------------------------------------------------------------

#include "LoopIdentification.h"

namespace LoopIdentification
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

   phase->Initialize(config, L"Identify loops in the code");

#if defined(PHX_DEBUG_SUPPORT)

   phase->PhaseControl = Phase::LoopIdentificationControl;

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
   if (!unit->IsFunctionUnit)
   {
      return;
   }

   Phx::FunctionUnit ^ functionUnit = unit->AsFunctionUnit;

   functionUnit->BuildFlowGraphWithoutEH();

   Phx::Graphs::FlowGraph ^ flowGraph = functionUnit->FlowGraph;

   flowGraph->BuildDepthFirstNumbers();

   if (Phx::Controls::DebugControls::TraceControl->IsEnabled(this->PhaseControl, functionUnit))
   {
      functionUnit->Dump();
   }

   // Now walk all blocks looking for back edges.

   for (Phx::Graphs::BasicBlock ^ block = flowGraph->StartBlock; 
      block != nullptr; block = block->Next)
   {
      for (Phx::Graphs::FlowEdge ^ edge = block->SuccessorEdgeList; 
         edge != nullptr; edge = edge->NextSuccessorEdge)
      {
         if (edge->IsBack)
         {
            Phx::Graphs::BasicBlock ^ loopHeadBlock = edge->PredecessorNode;
            Phx::IR::Instruction ^ loopHeadLabel = loopHeadBlock->FirstInstruction;

            System::Console::WriteLine("Found loop: Function {0} file {1} line {2}",
               Phx::Utility::Undecorate(functionUnit->NameString, false),
               functionUnit->DebugInfo->GetFileName(loopHeadLabel->DebugTag),
               functionUnit->DebugInfo->GetLineNumber(loopHeadLabel->DebugTag));
         }
      }
   }

   functionUnit->DeleteFlowGraph();
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

#if defined(PHX_DEBUG_SUPPORT)

   Phase::LoopIdentificationControl =
      Phx::Controls::ComponentControl::New(L"LoopIdentification",
         L"Identify loops in the code",
         L"LoopIdentification.cpp");

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
//          properties for retrieving the initial phase list.
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

   // InsertAfter CIL Reader

   basePhase = config->PhaseList->FindByName(L"CxxIL Reader");
   if (basePhase == nullptr)
   {
      Phx::Output::WriteLine(L"CIL Reader phase "
         L"not found in phaselist:");
      Phx::Output::Write(config->ToString());

      return;
   }

   Phx::Phases::Phase ^ phase = Phase::New(config);
   basePhase->InsertAfter(phase);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Returns the name of your plugin
//
//-----------------------------------------------------------------------------

System::String^ PlugIn::NameString::get()
{
	return L"LoopIdentification";
}

}
