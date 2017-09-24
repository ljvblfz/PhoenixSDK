//-----------------------------------------------------------------------------
//
// Description:
//
//   Find loop bodies and loop exits
//
// Remarks:
//
//    Registers the "NaturalLoopBodiesAndExits" component control.
//
//-----------------------------------------------------------------------------

#include "NaturalLoopBodiesAndExits.h"

namespace NaturalLoopBodiesAndExits
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

   phase->Initialize(config, L"Find loop bodies and loop exits");

#if defined(PHX_DEBUG_SUPPORT)

   phase->PhaseControl = Phase::NaturalLoopBodiesAndExitsControl;

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

   flowGraph->BuildDominators();

   if (Phx::Controls::DebugControls::TraceControl->IsEnabled(this->PhaseControl, functionUnit))
   {
      functionUnit->Dump();
   }

   // Remember loops that we've already seen by putting their IDs in this bit vector.

   Phx::BitVector::Sparse ^ loopHeaders = 
      Phx::BitVector::Sparse::New(flowGraph->Lifetime);

   // Walk flow graph in depth first post order

   Phx::Graphs::NodeFlowOrder ^ postOrder = 
      Phx::Graphs::NodeFlowOrder::New(flowGraph->Lifetime);

   postOrder->Build(flowGraph, Phx::Graphs::Order::PostOrder);

   for (unsigned int i = 1; i <= postOrder->NodeCount; ++i)
   {
      Phx::Graphs::BasicBlock ^ block = postOrder->Node(i)->AsBasicBlock;

      for (Phx::Graphs::FlowEdge ^ edge = block->PredecessorEdgeList; 
         edge != nullptr; edge = edge->NextPredecessorEdge)
      {
         Phx::Graphs::BasicBlock ^ predBlock = edge->PredecessorNode;

         if (block->Dominates(predBlock))
         {
            // Do we know about this header already?

            if (!loopHeaders->GetBit(block->Id))
            {
               // Evidently a new loop...
               // Determine extent of the body.

               this->FindLoopBody(block);
            }

            loopHeaders->SetBit(block->Id);
         }
      }
   }

   functionUnit->DeleteFlowGraph();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    FindLoopBody takes a loop header and, for each back-edge to this header,
//    it collects all nodes reachable backward from the back-edge's source.
//
//    In addition, each body block is visited and, if it has a successor that
//    is not in the loop, its last instruction is recorded as an exit.
//
// Arguments:
//
//    block - [in] A loop header.
//
//-----------------------------------------------------------------------------

void 
Phase::FindLoopBody(Phx::Graphs::BasicBlock ^ block)
{   
   Phx::FunctionUnit ^ functionUnit = block->FlowGraph->FunctionUnit;

   // We'll use a bit vector to track the membership of blocks 
   // in the loop body.

   Phx::BitVector::Sparse ^ loopBlocks = 
      Phx::BitVector::Sparse::New(block->Graph->Lifetime);

   // Add the header to the loop

   loopBlocks->SetBit(block->Id);

   // We'll use a second bit vector to track the set of blocks whose
   // predecessors still need to be visited.

   Phx::BitVector::Sparse ^ blocksToVisit = 
      Phx::BitVector::Sparse::New(block->Graph->Lifetime);

   for (Phx::Graphs::FlowEdge ^ edge = block->PredecessorEdgeList; 
      edge != nullptr; edge = edge->NextPredecessorEdge)
   {
      Phx::Graphs::BasicBlock ^ predBlock = edge->PredecessorNode;

      if (block->Dominates(predBlock))
      {
         // Add the backedge source to the loop body.

         loopBlocks->SetBit(predBlock->Id);

         // Add it as a place to start walking from.

         blocksToVisit->SetBit(predBlock->Id);
      }
   }

   // Now perform a reverse graph reachability transitive closure.

   while (!blocksToVisit->IsEmpty)
   {
      Phx::Graphs::BasicBlock ^ blockToVisit = 
         block->FlowGraph->Block(blocksToVisit->RemoveFirstBit());

      for (Phx::Graphs::FlowEdge ^ edge = blockToVisit->PredecessorEdgeList; 
         edge != nullptr; edge = edge->NextPredecessorEdge)
      {
         Phx::Graphs::BasicBlock ^ predBlock = edge->PredecessorNode;

         if (!loopBlocks->GetBit(predBlock->Id))
         {
            // This block is also in the loop.

            loopBlocks->SetBit(predBlock->Id);

            // And we need to check its predecessors

            blocksToVisit->SetBit(predBlock->Id);
         }
      }
   }

   // Determine what lines these blocks represent. First form the set of debug tags.

   Phx::BitVector::Sparse ^ loopTags = Phx::BitVector::Sparse::New(block->Graph->Lifetime);

   for each (unsigned int blockId in loopBlocks)
   {
      Phx::Graphs::BasicBlock ^ loopBlock = block->FlowGraph->Block(blockId);

      for each (Phx::IR::Instruction ^ instruction in loopBlock->Instructions)
      {
         loopTags->SetBit(instruction->DebugTag);
      }
   }

   // Map the debug tag set into a description of the loop body.

   System::String ^ loopDescription = "lines: ";
   bool isFirst = true;

   for each (unsigned int debugTag in loopTags)
   {
      if (!isFirst)
      {
         loopDescription += ", ";
      }
      isFirst = false;

      loopDescription += functionUnit->DebugInfo->GetLineNumber(debugTag);
   }

   
   // Find the set of exit blocks...

   System::String ^ exitDescription = "";
   isFirst = true;

   Phx::BitVector::Sparse ^ exitBlocks = 
      Phx::BitVector::Sparse::New(block->Graph->Lifetime);

   for each (unsigned int blockId in loopBlocks)
   {
      Phx::Graphs::BasicBlock ^ loopBlock = block->FlowGraph->Block(blockId);

      for (Phx::Graphs::FlowEdge ^ edge = loopBlock->SuccessorEdgeList; 
         edge != nullptr; edge = edge->NextSuccessorEdge)
      {
         Phx::Graphs::BasicBlock ^ successorBlock = edge->SuccessorNode;

         if (!loopBlocks->GetBit(successorBlock->Id))
         {
            exitBlocks->SetBit(loopBlock->Id);

            if (!isFirst)
            {
               exitDescription += ", ";
            }
            isFirst = false;

            Phx::IR::Instruction ^ exitInstruction = loopBlock->LastInstruction;
            Phx::IR::Instruction ^ targetInstruction = successorBlock->FirstInstruction;

            exitDescription += "from line " 
               + functionUnit->DebugInfo->GetLineNumber(exitInstruction->DebugTag)
               + " to "
               + functionUnit->DebugInfo->GetLineNumber(targetInstruction->DebugTag);
         }
      }
   }

   if (isFirst)
   {
      exitDescription += "(none)";
   }


   // Now emit the full set of blocks in the loop and the loop exits.
   
   Phx::IR::Instruction ^ loopHeadLabel = block->FirstInstruction;

   System::Console::WriteLine("Found loop: Function {0} file {1} line {2}",
      Phx::Utility::Undecorate(functionUnit->NameString, false),
      functionUnit->DebugInfo->GetFileName(loopHeadLabel->DebugTag),
      functionUnit->DebugInfo->GetLineNumber(loopHeadLabel->DebugTag));
   System::Console::WriteLine("Body is {0}", loopDescription);
   System::Console::WriteLine("Exits are {0}", exitDescription);
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

   Phase::NaturalLoopBodiesAndExitsControl =
      Phx::Controls::ComponentControl::New(L"NaturalLoopBodiesAndExits",
         L"Find loop bodies and loop exits",
         L"NaturalLoopBodiesAndExits.cpp");

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
	return L"NaturalLoopBodiesAndExits";
}

}
