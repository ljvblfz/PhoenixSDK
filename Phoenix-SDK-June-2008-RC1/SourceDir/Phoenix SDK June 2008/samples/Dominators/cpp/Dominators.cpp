//-----------------------------------------------------------------------------
//
// Description:
//
//   Dominance Computation
//
// Remarks:
//
//    Registers the "Dominators" component control.
//
//-----------------------------------------------------------------------------

#include "Dominators.h"

namespace Dominators
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

   phase->Initialize(config, L"Dominance Computation");

#if defined(PHX_DEBUG_SUPPORT)

   phase->PhaseControl = Phase::DominatorsControl;

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

   // Data flow solutions require the flow graph.

   functionUnit->BuildFlowGraph();
   
   Phx::Graphs::FlowGraph ^ flowGraph = functionUnit->FlowGraph;

   // ISSUES:
   //
   // Impact of unreachable blocks...
   // Awkwardness of special initialization...

   if (Phx::Controls::DebugControls::VerboseTraceControl->
      IsEnabled(this->PhaseControl, functionUnit))
   {
      functionUnit->Dump();
   }

   // Solve the dominance equations.

   DominanceWalker ^ walker = DominanceWalker::New(functionUnit);
   walker->Initialize(Phx::Dataflow::Direction::Forward, functionUnit);
   walker->SetBoundaryConditions(flowGraph);
   walker->Traverse(Phx::Dataflow::TraversalKind::Iterative, functionUnit);

   // Display the results

   System::Console::WriteLine("** Dominance computation for {0}", 
      Phx::Utility::Undecorate(functionUnit->NameString, false));

   for each (Phx::Graphs::BasicBlock ^ block in flowGraph->BasicBlocks)
   {
      DominanceData ^ dominanceData = 
         safe_cast<DominanceData ^>(walker->GetBlockData(block->Id));

      if (block->PredecessorCount == 0 && ! block->IsStart)
      {
         System::Console::WriteLine("{0} (unreachable) is dominated by {1}", 
            block->Id, block->Id);
      }
      else
      {
         System::Console::WriteLine("{0} is dominated by {1}", 
            block->Id, dominanceData->OutBitVector);
      }
   }

   // Clean up.

   walker->Delete();
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

   Phase::DominatorsControl =
      Phx::Controls::ComponentControl::New(L"Dominators",
         L"Dominance Computation",
         L"Dominators.cpp");

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
	return L"Dominators";
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Static constructor for the block data of the Dominators problem.
//    
//-----------------------------------------------------------------------------

DominanceData ^
DominanceData::New
(
   Phx::Lifetime ^ lifetime
)
{
   DominanceData ^ data = gcnew DominanceData;

   data->InBitVector = Phx::BitVector::Sparse::New(lifetime);
   data->OutBitVector = Phx::BitVector::Sparse::New(lifetime);

   return data;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Destructor for the block data of the Dominators problem.
//    
//-----------------------------------------------------------------------------

void
DominanceData::Delete()
{
   this->InBitVector->Delete();
   this->OutBitVector->Delete();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Merge implements the confluence operator, which is intersection (bit-wise
//    And) for dominators.
//
//-----------------------------------------------------------------------------

void
DominanceData::Merge
(
   Phx::Dataflow::Data ^     baseDependencyData,
   Phx::Dataflow::Data ^     baseBlockData,
   Phx::Dataflow::MergeFlags flags
)
{
   DominanceData ^ dependencyData = safe_cast<DominanceData ^>(baseDependencyData);

   if ((flags & Phx::Dataflow::MergeFlags::First) != Phx::Dataflow::MergeFlags::None)
   {
      this->InBitVector->CopyBits(dependencyData->OutBitVector);
   }
   else
   {
      this->InBitVector->And(dependencyData->OutBitVector);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determine if this and baseBlockData have the same IN set.
//
// Returns:
//
//    True if and only if this and baseBlockData have the same IN value.
//
// Remarks:
// 
//    This function is called by Traverse after merging values from
//    predecessors and prior to deciding whether to update the block's
//    OUT value.
//
//-----------------------------------------------------------------------------

bool
DominanceData::SamePrecondition
(
   Phx::Dataflow::Data ^ baseBlockData
)
{
   DominanceData ^ blockData = safe_cast<DominanceData ^>(baseBlockData);

   return this->InBitVector->Equals(blockData->InBitVector);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determine if this and blockData have the same out set(s).
//
// Returns:
//
//    True if and only if this and blockData have the same OUT value.
//
// Remarks:
//
//    This function is called by Traverse after re-evaluating a block's OUT
//    value into a temporary object and prior to deciding whether to update
//    the block's data itself.
//
//-----------------------------------------------------------------------------

bool
DominanceData::SamePostCondition
(
   Phx::Dataflow::Data ^ baseBlockData // Block data
)
{
   DominanceData ^ blockData = safe_cast<DominanceData ^>(baseBlockData);

   return this->OutBitVector->Equals(blockData->OutBitVector);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Update this block's data from temporaryData.
//
// Remarks:
//
//    This function is invoked by traverse only if (1) the block is being
//    evaluated for the first time or (2) SamePostCondition returned false.
//
//    The distinction between EvalBlock and Update avoids a useless copy
//    when updating the block into temporary data yields an unchanging
//    postcondition.
//    
//-----------------------------------------------------------------------------

void
DominanceData::Update
(
   Phx::Dataflow::Data ^ baseTemporaryData // Temporary data
)
{
   DominanceData ^ temporaryData = safe_cast<DominanceData ^>(baseTemporaryData);

   // We simply swap the in and out bit vectors.

   Phx::BitVector::Sparse ^ bitVectorSwap = this->InBitVector;
   this->InBitVector = temporaryData->InBitVector;
   temporaryData->InBitVector = bitVectorSwap;

   bitVectorSwap = this->OutBitVector;
   this->OutBitVector = temporaryData->OutBitVector;
   temporaryData->OutBitVector = bitVectorSwap;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Static constructor for the graph walker of the Dominators framework.
//
//-----------------------------------------------------------------------------

DominanceWalker ^
DominanceWalker::New
(
   Phx::FunctionUnit ^ functionUnit
)
{
   DominanceWalker ^ walker = gcnew DominanceWalker;

   walker->Lifetime = Phx::Lifetime::New(Phx::LifetimeKind::Temporary, 
      functionUnit, L"dominators.cpp", __LINE__);

   return walker;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Delete an instance of the DominanceWalker class.
//
//-----------------------------------------------------------------------------

void
DominanceWalker::Delete()
{
   __super::Delete();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Allocate the data flow data for given number of elements.
//
// Remarks:
//
//    This function is used by Walker::Init to allocate data flow
//    information in every block of the flow graph. As such, it has to be
//    defined by any subclass of Walker.
//    
//    It follows a standard pattern. The only thing in it that is specific
//    to Dominators is the allocation of DominanceData objects in
//    BlockDataArray.
//    
//-----------------------------------------------------------------------------

void
DominanceWalker::AllocateData
(
   unsigned int numberElements
)
{
   array<Phx::Dataflow::Data ^> ^ dataArray;

   dataArray = gcnew array<Phx::Dataflow::Data ^>(numberElements);

   // Initialize the data.

   for (unsigned int i = 0; i < numberElements; i++)
   {
      dataArray[i] = DominanceData::New(this->Lifetime);
   }

   this->BlockDataArray = dataArray;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Set initial OUT values.
//
// Remarks: 
//
//    Start has an initial OUT value of just { Start }.
//    All other blocks start with the universal set.
//    
//-----------------------------------------------------------------------------

void
DominanceWalker::SetBoundaryConditions
(
   Phx::Graphs::FlowGraph ^ flowGraph
)
{
   for each (Phx::Graphs::BasicBlock ^ block in flowGraph->BasicBlocks)
   {
      DominanceData ^ dominanceData = 
         safe_cast<DominanceData ^>(this->GetBlockData(block->Id));

      if (block->IsStart)
      {
         dominanceData->OutBitVector->SetBit(block->Id);
      }
      else
      {
         dominanceData->OutBitVector->SetBitRange(1, flowGraph->NodeCount);
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Implement the transition function that maps IN values to OUT values.
//
// Remarks:
//
//    In the Dominators problem, the block's instructions play no role in this
//    function.
//
//-----------------------------------------------------------------------------

void
DominanceWalker::EvaluateBlock
(
   Phx::Graphs::BasicBlock ^ block,
   Phx::Dataflow::Data ^     baseTemporaryData
)
{
   DominanceData ^  temporaryData = safe_cast<DominanceData ^>(baseTemporaryData);
   DominanceData ^  blockData = safe_cast<DominanceData ^>(this->GetBlockData(block));

   // If this block has no predecessors and is not start we want to
   // ignore its impact on dominance.

   if (block->PredecessorCount == 0 && ! block->IsStart)
   {
      temporaryData->OutBitVector->SetBitRange(1, block->FlowGraph->NodeCount);
   }
   else
   {
      temporaryData->OutBitVector->CopyBits(temporaryData->InBitVector);
      temporaryData->OutBitVector->SetBit(block->Id);
   }
}
}
