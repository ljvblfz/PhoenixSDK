//-----------------------------------------------------------------------------
//
// Description:
//
//   Loop Detection with Preheader and Postexit formation
//
// Remarks:
//
//    Registers the "LoopPreheaderAndPostExit" component control.
//
//-----------------------------------------------------------------------------

#include "LoopPreheaderAndPostExit.h"

namespace LoopPreheaderAndPostExit
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

   phase->Initialize(config, L"Loop Detection with Preheader and Postexit formation");

#if defined(PHX_DEBUG_SUPPORT)

   phase->PhaseControl = Phase::LoopPreheaderAndPostExitControl;

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

   // We'll keep track of all loops via their extension objects.

   System::Collections::Generic::List<LoopExtensionObject ^> ^ loops =
      gcnew System::Collections::Generic::List<LoopExtensionObject ^>();

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

            LoopExtensionObject ^ loop = LoopExtensionObject::Get(block);

            if (loop == nullptr)
            {
               // Evidently a new loop...
               // Create an extension object to describe it.

               loop = gcnew LoopExtensionObject();
               loop->LoopHeader = block;
               loop->ChildLoops = gcnew System::Collections::Generic::List<LoopExtensionObject ^>();
               block->AddExtensionObject(loop);
               loops->Add(loop);

               // Determine extent of the body and gather up nested loops.

               this->FindLoopBody(block);
            }
         }
      }
   }

   // Determine which loops are top level (they will not have parents)
   // and compute the loop nesting depth and loop exits.

   for each (LoopExtensionObject ^ loop in loops)
   {
      if (loop->ParentLoop == nullptr)
      {
         loop->ComputeDepth();
      }
   }

   System::Collections::Generic::List<Phx::Graphs::FlowEdge ^> ^ edgesToSplit =
      gcnew System::Collections::Generic::List<Phx::Graphs::FlowEdge ^>();

   for each (LoopExtensionObject ^ loop in loops)
   {
      if (loop->ParentLoop == nullptr)
      {
         // Find the edges we need to split to do instrumentation.

         edgesToSplit->AddRange(loop->DetermineEdgesToSplit());
      }
   }

   // split the edges

   for each (Phx::Graphs::FlowEdge ^ edge in edgesToSplit)
   {
      System::Console::WriteLine("Splitting {0}", edge);
      edge->FlowGraph->SplitEdge(edge);
   }

   if (Phx::Controls::DebugControls::TraceControl->IsEnabled(this->PhaseControl, functionUnit))
   {
      functionUnit->Dump();
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
//    In addition it instantiates the ParentLoop and ChildLoops members of the
//    LoopExtensionObject for the block.
//    
// Arguments:
//
//    block - [in] A loop header.
//
//-----------------------------------------------------------------------------

void 
Phase::FindLoopBody(Phx::Graphs::BasicBlock ^ block)
{   
   // We should have attached an extension prior to invoking FindLoopBody.

   LoopExtensionObject ^ loop = LoopExtensionObject::Get(block);

   // We'll use a bit vector to track the membership of blocks 
   // in the loop body.

   loop->AllLoopBlocks = 
      Phx::BitVector::Sparse::New(block->Graph->Lifetime);

   // The BackEdges bit vector will be used to identify predecessors to
   // loop headers from outside the loop.

   loop->BackEdges =
      Phx::BitVector::Sparse::New(block->Graph->Lifetime);

   // Add the header to the loop

   loop->AllLoopBlocks->SetBit(block->Id);

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
         // Add backedge to set of backedges

         loop->BackEdges->SetBit(edge->Id);

         // Add the backedge source to the loop body.

         loop->AllLoopBlocks->SetBit(predBlock->Id);

         // Add it as a place to start walking from.

         blocksToVisit->SetBit(predBlock->Id);
      }
   }

   // Now perform a reverse graph reachability transitive closure.

   while (!blocksToVisit->IsEmpty)
   {
      Phx::Graphs::BasicBlock ^ blockToVisit = 
         block->FlowGraph->Block(blocksToVisit->RemoveFirstBit());

      // If this block is itself a loop header, it represents a nested loop.
 
      LoopExtensionObject ^ nestedLoop = LoopExtensionObject::Get(blockToVisit);

      if (nestedLoop != nullptr)
      {
         // It may be indirectly nested -- that is contained in a loop
         // that the current loop contains. See if we've already determined a parent.
         
         if (nestedLoop->ParentLoop == nullptr)
         {
            // Nope, this is a directly nested loop, so lay claim to it.

            nestedLoop->ParentLoop = loop;
            loop->ChildLoops->Add(nestedLoop);
         }
      }

      for (Phx::Graphs::FlowEdge ^ edge = blockToVisit->PredecessorEdgeList; 
         edge != nullptr; edge = edge->NextPredecessorEdge)
      {
         Phx::Graphs::BasicBlock ^ predBlock = edge->PredecessorNode;

         if (!loop->AllLoopBlocks->GetBit(predBlock->Id))
         {
            // This block is also in the loop.

            loop->AllLoopBlocks->SetBit(predBlock->Id);

            // And we need to check its predecessors

            blocksToVisit->SetBit(predBlock->Id);
         }
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    FindLoopExits visits each body block found in its receiver and, if it has
//    a successor that is not in the loop, its last instruction is recorded as an exit.
//
//-----------------------------------------------------------------------------

void LoopExtensionObject::FindLoopExits()
{
   // We should have attached an extension.

   LoopExtensionObject ^ loop = this;
   Phx::Graphs::BasicBlock ^ block = loop->LoopHeader;

   // Find the set of exit blocks...

   loop->ExitLoopBlocks = 
      Phx::BitVector::Sparse::New(block->Graph->Lifetime);

   for each (unsigned int blockId in loop->AllLoopBlocks)
   {
      Phx::Graphs::BasicBlock ^ loopBlock = block->FlowGraph->Block(blockId);

      for (Phx::Graphs::FlowEdge ^ edge = loopBlock->SuccessorEdgeList; 
         edge != nullptr; edge = edge->NextSuccessorEdge)
      {
         Phx::Graphs::BasicBlock ^ successorBlock = edge->SuccessorNode;

         if (!loop->AllLoopBlocks->GetBit(successorBlock->Id))
         {
            loop->ExitLoopBlocks->SetBit(loopBlock->Id);
         }
      }
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

   Phase::LoopPreheaderAndPostExitControl =
      Phx::Controls::ComponentControl::New(L"LoopPreheaderAndPostExit",
         L"Loop Detection with Preheader and Postexit formation",
         L"LoopPreheaderAndPostExit.cpp");

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
	return L"LoopPreheaderAndPostExit";
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieve the LoopExtensionObject attached to a Block.
//
// Remarks:
//
//    This function assumes there is a LoopExtensionObject attached
//    to the argument. It will throw an InvalidCastException if there is none.
//
// Returns:
//
//    The LoopExtensionObject or nil.
//
//-----------------------------------------------------------------------------

LoopExtensionObject ^
LoopExtensionObject::Get
(
 Phx::Graphs::BasicBlock ^ block
 )
{
   return safe_cast<LoopExtensionObject ^>(block->FindExtensionObject(
      LoopExtensionObject::typeid));
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Display the data in a LoopExtensionObject on the console.
//    
//-----------------------------------------------------------------------------

void LoopExtensionObject::Describe()
{
   Phx::FunctionUnit ^ functionUnit = this->LoopHeader->FlowGraph->FunctionUnit;
   System::String ^ indent = gcnew System::String(' ', this->LoopDepth);

   System::Console::WriteLine("{0}Loop at {1} line {2}",
      indent,
      Phx::Utility::Undecorate(functionUnit->NameString, false),
      functionUnit->DebugInfo->GetLineNumber(this->LoopHeader->FirstInstruction->DebugTag));
   System::Console::WriteLine("{0}Header block: {1}", indent, this->LoopHeader->Id);
   System::Console::WriteLine("{0}Exclusive blocks: {1}", indent, this->ExclusiveLoopBlocks);
   System::Console::WriteLine("{0}Inclusive blocks: {1}", indent, this->AllLoopBlocks);
   System::Console::WriteLine("{0}Exit blocks: {1}", indent, this->ExitLoopBlocks);
   System::Console::WriteLine("{0}Backedges: {1}", indent, this->BackEdges);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Display LoopExtensionObject data in depth-first order.
//    
//-----------------------------------------------------------------------------

void LoopExtensionObject::DescribeAll()
{
   this->Describe();

   for each(LoopExtensionObject ^ child in this->ChildLoops)
   {
      child->DescribeAll();
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Visit loop descriptors in depth-first order, compute depths in pre-order,
//    and perform //    set operations to compute inclusive and exclusive sets
//    of body blocks.
//
//-----------------------------------------------------------------------------

void LoopExtensionObject::ComputeDepth()
{
   // Compute depth top-down.

   if (this->ParentLoop == nullptr)
   {
      this->LoopDepth = 0;
   }
   else
   {
      this->LoopDepth = this->ParentLoop->LoopDepth + 1;
   }

   for each(LoopExtensionObject ^ child in this->ChildLoops)
   {
      child->ComputeDepth();
   }

   // Compute exclusive blocks bottom up.

   this->ExclusiveLoopBlocks = this->AllLoopBlocks->Copy();

   for each(LoopExtensionObject ^ child in this->ChildLoops)
   {
      this->ExclusiveLoopBlocks->Minus(child->AllLoopBlocks);
   }

   this->FindLoopExits();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Split critical edges.
//
// Remarks:
// 
//    Walk predecessors of header, looking for non-backedges that come
//    from blocks that are control flow splits.
//    
//    Walk successors of exits, looking for blocks that are control flow merges.
//    
//-----------------------------------------------------------------------------

System::Collections::Generic::List<Phx::Graphs::FlowEdge ^> ^
LoopExtensionObject::DetermineEdgesToSplit()
{
   System::Collections::Generic::List<Phx::Graphs::FlowEdge ^> ^ edgesToSplit =
      gcnew System::Collections::Generic::List<Phx::Graphs::FlowEdge ^>();

   for (Phx::Graphs::FlowEdge ^ edge = this->LoopHeader->PredecessorEdgeList; 
      edge != nullptr; 
      edge = edge->NextPredecessorEdge)
   {
      // Don't instrument back edges to the header

      if (this->BackEdges->GetBit(edge->Id))
      {
         continue;
      }

      Phx::Graphs::BasicBlock ^ predBlock = edge->PredecessorNode;

      // Make sure the header is reachable through this edge

      if (!predBlock->HasPathFromStart)
      {
         continue;
      }

      // No need to split the edge if this is the only path out of
      // the predecessor block.

      if(predBlock->UniqueNonEHSuccessorBlock != nullptr)
      {
         continue;
      }

      // And of course the edge must be splittable.

      if (!edge->IsSplittable)
      {
         continue;
      }

      edgesToSplit->Add(edge);
   }

   for each (unsigned int blockId in this->ExitLoopBlocks)
   {
      Phx::Graphs::BasicBlock ^ loopBlock = this->LoopHeader->FunctionUnit->FlowGraph->Block(blockId);

      for (Phx::Graphs::FlowEdge ^ edge = loopBlock->SuccessorEdgeList; 
         edge != nullptr; edge = edge->NextSuccessorEdge)
      {

         // Continue if this successor edge isn't the loop exit itself

         if (this->AllLoopBlocks->GetBit(edge->SuccessorNode->Id))
         {
            continue;
         }

         // Continue if we don't need to split this edge (can only get to the successor node
         // through this node).

         if(edge->SuccessorNode->UniqueNonEHPredecessorBlock != nullptr)
         {
            continue;
         }

         if (!edge->IsSplittable)
         {
            continue;
         }

         edgesToSplit->Add(edge);
      }
   }

   if (edgesToSplit->Count > 0)
   {
      System::Console::WriteLine("Split {0} edges for instrumentation of loop on line {1} in {2}", 
         edgesToSplit->Count,
		 this->LoopHeader->FunctionUnit->DebugInfo->GetLineNumber(this->LoopHeader->FirstInstruction->DebugTag),
		 this->LoopHeader->FunctionUnit->NameString);
   }
   else
   {
      System::Console::WriteLine("No edges need to be split to instrument loop on line {0} in {1}",
		this->LoopHeader->FunctionUnit->DebugInfo->GetLineNumber(this->LoopHeader->FirstInstruction->DebugTag),
		this->LoopHeader->FunctionUnit->NameString);
   }

   return edgesToSplit;
}

}
