//-----------------------------------------------------------------------------
//
// Description:
//
//   Depth First Search
//
// Remarks:
//
//    Registers the "DepthFirstSearch" component control.
//
//-----------------------------------------------------------------------------

#include "DepthFirstSearch.h"

namespace DepthFirstSearch
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

   phase->Initialize(config, L"Depth First Search");

#if defined(PHX_DEBUG_SUPPORT)

   phase->PhaseControl = Phase::DepthFirstSearchControl;

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

   functionUnit->BuildFlowGraph();

   Phx::Graphs::FlowGraph ^ flowGraph = functionUnit->FlowGraph;


   this->PrenumberDispenser = 1;
   this->PostnumberDispenser = 1;

   this->DepthFirstWalk(flowGraph->StartBlock);

   for each (Phx::Graphs::BasicBlock ^ block in flowGraph->BasicBlocks)
   {
      BlockExtension ^ blockExtension = BlockExtension::Get(block);

      if (blockExtension != nullptr)
      {
         System::Console::WriteLine("Block {0} Prenumber {1} Postnumber {2}", 
            block->Id, blockExtension->Prenumber, blockExtension->Postnumber);
      }
      else
      {
         System::Console::WriteLine("Block {0} is missing extension", block->Id);
      }
   }

   functionUnit->DeleteFlowGraph();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    The recursive function that implements the depth-first search.
//
//-----------------------------------------------------------------------------
void
Phase::DepthFirstWalk(Phx::Graphs::BasicBlock ^ block)
{
   BlockExtension ^ blockExtension = BlockExtension::Get(block);

   if (blockExtension == nullptr)
   {
      // First time we've encountered this block.

      blockExtension = gcnew BlockExtension();
      blockExtension->Prenumber = this->PrenumberDispenser++;
      block->AddExtensionObject(blockExtension);

      for each (Phx::Graphs::FlowEdge ^ edge in block->SuccessorEdges)
      {
         this->DepthFirstWalk(edge->SuccessorNode);
      }

      blockExtension->Postnumber = this->PostnumberDispenser++;
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

#if defined(PHX_DEBUG_SUPPORT)

   Phase::DepthFirstSearchControl =
      Phx::Controls::ComponentControl::New(L"DepthFirstSearch",
         L"Depth First Search",
         L"DepthFirstSearch.cpp");

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

   // InsertAfter CxxIL Reader

   basePhase = config->PhaseList->FindByName(L"CxxIL Reader");
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

//-----------------------------------------------------------------------------
//
// Description:
//
//    Returns the name of your plugin
//
//-----------------------------------------------------------------------------

System::String^ PlugIn::NameString::get()
{
	return L"DepthFirstSearch";
}

}
