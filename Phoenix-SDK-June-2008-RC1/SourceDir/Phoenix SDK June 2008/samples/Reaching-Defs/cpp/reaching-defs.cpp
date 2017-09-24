//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//    Reaching definitions plug-in.
//    Sample illustrates Phoenix Data Flow package
//    implementing standard iterative algorithm for
//    reaching definitions.
//
//    The plug-in injects a phase after MIR Lower
//
// Usage:
//
//    -Plugin:reaching-defs.dll
//
//-----------------------------------------------------------------------------

using namespace System;
using namespace System::IO;

#include "..\..\common\samples.h"
#include "reaching-defs.h"

namespace Phx
{

namespace Samples
{

namespace ReachingDefs
{

//-----------------------------------------------------------------------------
//
// Description:
//
//    Register Reaching Definitions plug-in control.
//
// Returns:
//
//    Nothing.
//
//-----------------------------------------------------------------------------

void
PlugIn::RegisterObjects()
{
#if defined(PHX_DEBUG_SUPPORT)

   Phase::DebugControl =
      Phx::Controls::ComponentControl::New(L"ReachingDefs",
         L"Show reaching definitions",
         L"reaching-defs.cpp");

#endif
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Create and insert phase for Reaching Definitions plug-in
//
// Returns:
//
//    Nothing.
//
//-----------------------------------------------------------------------------

void
PlugIn::BuildPhases
(
   Phases::PhaseConfiguration ^ config
)
{
   Phases::Phase ^ mirLowerPhase = config->PhaseList->FindByName(L"MIR Lower");

   if (mirLowerPhase == nullptr)
   {
      // Need a real error.

      Phx::Output::WriteLine(L"MIR Lower phase not found in phaselist");

      return;
   }

   Phase ^ phase = Phase::New(config);

   mirLowerPhase->InsertAfter(phase);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Static constructor for the Reaching Definitions phase.
//
// Returns:
//
//    ReachingDefs phase object.
//
//-----------------------------------------------------------------------------

Phase ^
Phase::New
(
   Phases::PhaseConfiguration ^ config // containter for the phase 
)
{
   Phase ^ phase = gcnew Phase;
   phase->Initialize(config, L"Reaching Definitions Phase");

#if defined(PHX_DEBUG_SUPPORT)
   phase->PhaseControl = Phase::DebugControl;
#endif

   return phase;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Executes the Reaching Definitions phase.
//
//-----------------------------------------------------------------------------

void
Phase::Execute
(
   Phx::Unit ^ unit // unit  the phase should run for
)
{
   if (!unit->IsFunctionUnit)
   {
      return;
   }

   Phx::FunctionUnit ^ functionUnit = unit->AsFunctionUnit;

   Phx::Alias::Info ^ aliasInfo = functionUnit->AliasInfo;

   // check if alias information is ready

   if (!aliasInfo->IsComplete)
   {
      Output::WriteLine(
         L"Alias information has to be collected in order to "
         L"collect reaching definitions");

      return;
   }

   // prepare flow graph: we need up-to-date flow graph

   if (functionUnit->FlowGraph == nullptr)
   {
      functionUnit->BuildFlowGraph();
   }

   Walker ^ walker = Walker::New(functionUnit);

   // Collect definitions we will track.

   walker->Collect();

   // Compute In and Out sets for basic blocks.

   walker->Compute();

   // Collect reaching definitions for each operand we track.
   // Dump the information if requested so.

   walker->CollectOperandInfo();

   // Clean up what we created.

   walker->Delete();

   functionUnit->DeleteFlowGraph();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Static constructor for the Reaching Definitions Table.
//
// Returns:
//
//    ReachingDefs Table.
//
//-----------------------------------------------------------------------------

DefsTable ^
DefsTable::New
(
   Phx::FunctionUnit ^ functionUnit, // the functionUnit  to be analyzed
   Phx::Lifetime ^ lifetime     // lifetime to be used for allocations
)
{
   DefsTable ^ definitionTable = gcnew DefsTable;

   definitionTable->Lifetime = lifetime;

   definitionTable->CurrDefId = 0;

   definitionTable->FunctionUnit = functionUnit;

   Phx::UInt numLocTags = definitionTable->FunctionUnit->AliasInfo->NumberLocationTags;

   // Create DeId->Instruction mapping.
   // Use number of Locs as an initial estimation of number of
   // definitions we wwill track.

   definitionTable->DefToInstrMap =
      IdToInstructionMap::New(lifetime, nullptr, numLocTags);

   // Create LocTag->DefIds mapping

   Phx::UInt locationTag;
   definitionTable->LocTagToDefsArray = gcnew array<BitVector::Sparse ^>(numLocTags);
   for (locationTag = 0; locationTag < numLocTags; locationTag++)
   {
      definitionTable->LocTagToDefsArray[locationTag] = BitVector::Sparse::New(lifetime);
   }

   return definitionTable;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Delete Reaching Definitions Table.
//
//-----------------------------------------------------------------------------

void
DefsTable::Delete()
{
   Phx::FunctionUnit ^ functionUnit = this->FunctionUnit;

   // Delete DefId->Instruction mapping

   this->DefToInstrMap->Delete();

   // Delete all bitvectors in LocTag->DefIds mapping

   Phx::UInt numLocTags = functionUnit->AliasInfo->NumberLocationTags;
   Phx::UInt locationTag;
   for (locationTag = 0; locationTag < numLocTags; locationTag++)
   {
      this->LocTagToDefsArray[locationTag]->Delete();
   }

   // Delete all extending object we created for instructions and operands.

   for each (Phx::IR::Instruction ^ instruction in functionUnit->Instructions)
   {
      InstructionExtensionObject ^ extensionObject =
         InstructionExtensionObject::GetExtensionObject(instruction);

      if (extensionObject != nullptr)
      {
         instruction->UnlinkExtensionObject(extensionObject);
         extensionObject->Delete();
      }

      for each (Phx::IR::Operand ^ srcOperand in instruction->DataflowSourceOperands)
      {
         OperandExtensionObject ^ extensionObject =
            OperandExtensionObject::GetExtensionObject(srcOperand);
         if (extensionObject != nullptr)
         {
            srcOperand->UnlinkExtensionObject(extensionObject);
            extensionObject->Delete();
         }
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Record new definition for the given LocTag generated by
//    the given instruction.
//
//-----------------------------------------------------------------------------

void
DefsTable::AddLocDef
(
   Phx::Alias::Tag  locationTag,    // the LocTag we add defintion for
   Phx::IR::Instruction ^ definitionInstruction   // the instruction which generates the definiton
)
{
#if defined(PHX_DEBUG_CHECKS)
   Phx::Asserts::Assert(this->FunctionUnit->AliasInfo->IsLocationTag(locationTag));
#endif

   DefId currDefId = this->CurrDefId++;

   // Record new definition for the LocTag.

   this->LocTagToDefsArray[locationTag]->SetBit(currDefId);

   // record which instruction generates the defintion

   this->DefToInstrMap->Push(currDefId, definitionInstruction);

   // add the defintion to the set of defintion generated by the instruction.
   // The information is stored in the extending object. 
   // If it the first defintion generated by the instruction, 
   // then the extending object shoul dbe created as well.

   InstructionExtensionObject ^ extensionObject =
      InstructionExtensionObject::GetExtensionObject(definitionInstruction);
   if (extensionObject == nullptr)
   {
      extensionObject = InstructionExtensionObject::New(this->Lifetime);
      definitionInstruction->AddExtensionObject(extensionObject);
   }
   extensionObject->DefsBv->SetBit(currDefId);

}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Find the instruction which generates the given defintion.
//
// Returns:
//
//    The instruction.
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^
DefsTable::GetDefInstr
(
   DefId defId  // definition we need instruction for
)
{
   return this->DefToInstrMap->Lookup(defId);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Find the set of defintions for the given LocTag.
//
// Returns:
//
//    The set.
//-----------------------------------------------------------------------------

BitVector::Sparse ^
DefsTable::GetAliasTagDefs
(
   Phx::Alias::Tag aliasTag // the tag we need definitions for
)
{
#if defined(PHX_DEBUG_CHECKS)
   Phx::Asserts::Assert(this->FunctionUnit->AliasInfo->IsLocationTag(aliasTag));
#endif
   return this->LocTagToDefsArray[aliasTag];
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Static constructor for Reaching Definitions block data
//
// Returns:
//
//    The data.
//-----------------------------------------------------------------------------

Data ^
Data::New
(
   Phx::Lifetime ^ lifetime // Memory allocation lifetime
)
{
   // Allocate bit vectors the data contains

   Data ^ data = gcnew Data;

   data->InBitVector = BitVector::Sparse::New(lifetime);
   data->OutBitVector = BitVector::Sparse::New(lifetime);
   data->GenerateBitVector = BitVector::Sparse::New(lifetime);
   data->KillBitVector = BitVector::Sparse::New(lifetime);

   data->Lifetime = lifetime;

   return data;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Destructor for Reaching Definitions block data
//
//-----------------------------------------------------------------------------

void
Data::Delete()
{
   // Delete all bit vectors the data contains

   this->InBitVector->Delete();
   this->OutBitVector->Delete();
   this->GenerateBitVector->Delete();
   this->KillBitVector->Delete();
   if (this->HasExnInfo)
   {
      this->ExnOutBV->Delete();
      this->ExnGenBV->Delete();
      this->ExnKillBV->Delete();
   }
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Getter for the HasExnInfo property
//
// Returns:
//
//    True if and only if this block has been marked as having an outgoing
//    exception edge
//
//--------------------------------------------------------------------------

bool
Data::HasExnInfo::get()
{
   return (this->ExnOutBV != nullptr);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Ensure that storage has been allocated for exception edge info, and
//    indicate that this block has an exception successor.
//
//-----------------------------------------------------------------------------

void
Data::EnsureExnInfo()
{
   if (!this->HasExnInfo)
   {
      this->ExnOutBV = BitVector::Sparse::New(this->Lifetime);
      this->ExnGenBV = BitVector::Sparse::New(this->Lifetime);
      this->ExnKillBV = BitVector::Sparse::New(this->Lifetime);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Merge reach out from predeccessors into this block's reach in.
//
// Remarks:
//
//    If this is the first predeccessor, we can simply copy.
//
//-----------------------------------------------------------------------------

void
Data::Merge
(
   Phx::Dataflow::Data ^     baseDependencyData,   // Dependent data
   Phx::Dataflow::Data ^     ,/*baseBlockData       //Block data which is unused for this merge op*/
   Phx::Dataflow::MergeFlags flags                 // Merge flags
)
{
   Phx::BitVector::Sparse ^ outBV;          // definitions live out from dependencyData
   Data ^                   dependencyData = safe_cast<Data ^>(baseDependencyData);

#if defined(PHX_DEBUG_CHECKS)

   Phx::Dataflow::MergeFlags supportedFlags =
      static_cast<Phx::Dataflow::MergeFlags>(Phx::Dataflow::MergeFlags::None
         | Phx::Dataflow::MergeFlags::First
         | Phx::Dataflow::MergeFlags::EH);

   Phx::Asserts::Assert((flags & ~supportedFlags) == 
      static_cast<Phx::Dataflow::MergeFlags>(0),
      "ReachingDefs::Data::Merge only allows None, First, and EH flags");

#endif

   if ((flags & Phx::Dataflow::MergeFlags::EH) !=
      static_cast<Phx::Dataflow::MergeFlags>(0))
   {
      outBV = dependencyData->ExnOutBV;
   }
   else
   {
      outBV = dependencyData->OutBitVector;
   }

   if ((flags & Phx::Dataflow::MergeFlags::First) !=
      static_cast<Phx::Dataflow::MergeFlags>(0))
   {
      this->InBitVector->CopyBits(outBV);
   }
   else
   {
      this->InBitVector->Or(outBV);
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

Phx::Boolean
Data::SamePrecondition
(
   Phx::Dataflow::Data ^ baseBlockData // Block data
)
{
   Data ^ blockData = safe_cast<Data ^>(baseBlockData);

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

Phx::Boolean
Data::SamePostCondition
(
   Dataflow::Data ^ baseBlockData // Block data
)
{
   Data ^ blockData = safe_cast<Data ^>(baseBlockData);

   return (this->OutBitVector->Equals(blockData->OutBitVector) && (!blockData->HasExnInfo
         || blockData->ExnOutBV->Equals(this->ExnOutBV)));
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
Data::Update
(
   Dataflow::Data ^ baseTemporaryData // Temporary data
)
{
   Data ^      temporaryData = safe_cast<Data ^>(baseTemporaryData);

   // We simply swap the live in and live out bit vectors.

   BitVector::Sparse ^ bitVectorSwap;
   bitVectorSwap = this->InBitVector;
   this->InBitVector = temporaryData->InBitVector;
   temporaryData->InBitVector = bitVectorSwap;

   bitVectorSwap = this->OutBitVector;
   this->OutBitVector = temporaryData->OutBitVector;
   temporaryData->OutBitVector = bitVectorSwap;

   if (this->HasExnInfo)
   {
#if defined(PHX_DEBUG_CHECKS)
      Phx::Asserts::Assert(temporaryData->HasExnInfo);
#endif
      bitVectorSwap = this->ExnOutBV;
      this->ExnOutBV = temporaryData->ExnOutBV;
      temporaryData->ExnOutBV = bitVectorSwap;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Static operator New for the Walker class.
//
// Returns:
//
//    Walker object.
//
// Remarks:
//
//    What any such function must do is set the LifeTime of the new Walker.
//    The other initializations are specific to the particular ReachingDefs
//    walker.
//    
//-----------------------------------------------------------------------------

Walker ^
Walker::New
(
   Phx::FunctionUnit ^ functionUnit
)
{
   Walker     ^ walker = gcnew Walker;

   walker->FunctionUnit = functionUnit;

   walker->Lifetime = Phx::Lifetime::New(LifetimeKind::Temporary, functionUnit,
      L"reaching-defs.cpp", __LINE__);

   // To create a shorter lifespan than the whole walker's for tmp objects,
   // one could periodically delete and recreate TemporaryLifetime.

   walker->TemporaryLifetime = walker->Lifetime;

   walker->definitionTable =
      DefsTable::New(functionUnit, walker->TemporaryLifetime);

   return walker;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Delete an instance of the Walker class.
//
//-----------------------------------------------------------------------------

void
Walker::Delete()
{
   this->DefTable->Delete();
   Phx::Dataflow::Walker::Delete();
   this->TemporaryLifetime->Delete();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Collect all definitions  which have to be tracked
//
// Remarks:
// 
//    This preparatory step to calling Walker::Traverse is specific to
//    ReachingDefs.
//    
//-----------------------------------------------------------------------------

void
Walker::Collect()
{
   Phx::FunctionUnit    ^ functionUnit = this->FunctionUnit;
   Phx::Alias::Info ^ aliasInfo = functionUnit->AliasInfo;

   // collect definitions

   for each (Phx::IR::Instruction ^ instruction in functionUnit->Instructions)
   {
      // Ssa instructions are not going to add defintions for Location Tags

      if (instruction->IsSsa)
      {
         continue;
      }

      // We should treate instruction as it generates definition for each
      // location tag partially overlapped with any data flow operand
      // in the dests list of the instructon.

      for each (Phx::IR::Operand ^ dstOperand in instruction->DataflowDestinationOperands)
      {
         // We want to ignore definitons of temps until we done with
         // the iterative part of the algorithm because temps definitions are 
         // always in the same block where the temps are used

         if (dstOperand->IsExpressionTemporary)
         {
            continue;
         }

         foreach_may_partial_alias_of_tag(
            aliasTag, dstOperand->AliasTag, aliasInfo)
         {
            this->DefTable->AddLocDef(
               aliasTag, instruction);
         }
         next_may_partial_alias_of_tag;
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Compute reaching definitions for the specified function.
//
// Remarks:
//    This is the place where the benefits of using a subclass of Walker
//    are reaped: Walker::Init and Walker::Traverse are the functions
//    inherited from Walker to propagate DataFlow information through
//    the flow graph.
//    
//-----------------------------------------------------------------------------

void
Walker::Compute()
{
   // Reaching Definitions is iterative Dataflow framework traversal
   // in forward direction. The below code is a typical code to invoke
   // such form of DataFlow analysis.

   this->Initialize(Phx::Dataflow::Direction::Forward, this->FunctionUnit);

   this->Traverse(Phx::Dataflow::TraversalKind::Iterative, this->FunctionUnit);
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
//    to ReachingDefs is the allocation of ReachingDefs::Data objects in
//    BlockDataArray.
//    
//    Note that BlockDataArray is the main repository of data flow information.
//    The extension objects used by ReachingDefs are used to build GEN and KILL
//    sets, but not all problems require attaching information to instructions
//    and operands in order to compute a block's information.
//    
//-----------------------------------------------------------------------------

void
Walker::AllocateData
(
   Phx::UInt numberElements // Number of data elements
)
{
   array<Phx::Dataflow::Data ^> ^ dataArray;

   dataArray = gcnew array<Phx::Dataflow::Data ^>(numberElements);

   // Initialize the data.

   for (Phx::UInt i = 0; i < numberElements; i++)
   {
      dataArray[i] = Data::New(this->TemporaryLifetime);
   }

   this->BlockDataArray = dataArray;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Compute reaching definitions for the specified block by walking the
//    instructions in forward directions.
//
//    We compute OUT and, if required, the summary GEN and KILL sets.
//    ExprTmps are ignored since they are block lifetime.
//
// Remarks:
//
//   This function is invoked by Walker::Traverse. It applies the transition
//   function found in block to the IN data in baseTmpData so as to obtain
//   the OUT data in baseTmpData.
//   Note that it does not modify `block' (other than initializing its
//   transition function, i.e. the GEN and KILL sets in the case of
//   ReachingDefs); data flow information is only updated in `baseTmpData'.
//
//-----------------------------------------------------------------------------

void
Walker::EvaluateBlock
(
   Phx::Graphs::BasicBlock ^ block,      // Block to evaluate
   Phx::Dataflow::Data ^     baseTemporaryData // Temporary data
)
{
   Phx::FunctionUnit ^ functionUnit = this->FunctionUnit;
   Alias::Info   ^ aliasInfo = functionUnit->AliasInfo;
   DefsTable     ^ definitionTable = this->DefTable;

   Data ^  temporaryData;
   Data ^  blockData;

   temporaryData = safe_cast<Data ^>(baseTemporaryData);
   blockData = safe_cast<Data ^>(this->GetBlockData(block));

   // 
   // If block data is being evaluated for the first time, compute
   // GEN and KILL sets.
   //

   if (blockData->MustEvaluate)
   {
      for each (Phx::IR::Instruction ^ instruction in block->Instructions)
      {
#if defined(PHX_DEBUG_DUMPS)
         if (Controls::DebugControls::VerboseTraceControl->IsEnabled(Phase::DebugControl,
               functionUnit))
         {
            Output::WriteLine(instruction->ToString());
         }
#endif

         // Ssa instruction should contribute nothing to dataflow.

         if (instruction->IsSsa)
         {
            continue;
         }

         if (instruction->HasHandlerLabelOperand)
         {
#if defined(PHX_DEBUG_CHECKS)

            // Blocks are split on exception edges; any instruction that throws
            // should be the last in its block.

            Phx::Asserts::Assert(block->LastInstruction == instruction);

            // Only expecting to see one exception successor

            Phx::Asserts::Assert(!blockData->HasExnInfo);
#endif

            // Mark that this block has an outgoing exception edge.

            blockData->EnsureExnInfo();

            // Kill set from start to exception edge is the current kill set.

            blockData->ExnKillBV->CopyBits(blockData->KillBitVector);

            // Generate set from start to exception edge will be the current gen
            // set plus the faulting instruction's DefsBv (if the instruction is
            // e.g. a call with a parameter passed by reference, it may write
            // before the exception occurs).

            blockData->ExnGenBV->CopyBits(blockData->GenerateBitVector);

            // Note:  We are using extra bit vectors here to store the block
            // summary at exception exits as well as normal exits.  One could
            // reduce storage overhead by (A) detecting when exception and
            // normal exists have the same summary information, or (B) storing
            // summary information for the execution point just before the last
            // instruction, and applying the effects of the last instruction
            // each time Merge is called (either the exception case or the
            // normal case, depending on the MergeFlags)
         }

         // Get the defintions generated by the current instruction.

         InstructionExtensionObject ^ extensionObject =
            InstructionExtensionObject::GetExtensionObject(instruction);

         for each (Phx::IR::Operand ^ dstOperand in instruction->DataflowDestinationOperands)
         {
            if (dstOperand->IsExpressionTemporary)
            {
               continue;
            }

            // Update information based on "must" part of dataflow.
            // This part makes  "disambiguous" part of instruction definitions.
            // These definitions kill any other defintions of the location tags.

            foreach_must_total_alias_of_tag(
               aliasTag, dstOperand->AliasTag, aliasInfo)
            {
               BitVector::Sparse ^ defsBv =
                  definitionTable->GetAliasTagDefs(aliasTag);
               blockData->KillBitVector->Or(defsBv);
            }
            next_must_total_alias_of_tag;
         }

         // update gen information

         // remove everything currently killed from the block's Generate set

         blockData->GenerateBitVector->Minus(blockData->KillBitVector);

         if (extensionObject != nullptr)
         {
            // All definitions generated by the current instruction are 
            // not killed anymore. They are now part of Generate set.

            blockData->KillBitVector->Minus(extensionObject->DefsBv);
            blockData->GenerateBitVector->Or(extensionObject->DefsBv);
            if (blockData->HasExnInfo)
            {
               blockData->ExnGenBV->Or(extensionObject->DefsBv);
            }
         }

#if defined(PHX_DEBUG_DUMPS)
         if (Controls::DebugControls::VerboseTraceControl->IsEnabled(Phase::DebugControl,
               functionUnit))
         {
            Output::WriteLine(L"  Generate:  {0}", blockData->GenerateBitVector->ToString());
            Output::WriteLine(L"  Kill: {0}", blockData->KillBitVector->ToString());
         }
#endif
      }
   }

   // Compute Out = (In - Kill) + Generate. 
   // This is update which can trigger iterative walks if temporaryData->OutBitVector
   // changes compared to what is saved in the block data.

   temporaryData->OutBitVector->CalculateDataflow(temporaryData->InBitVector,
      blockData->KillBitVector, blockData->GenerateBitVector);

   if (blockData->HasExnInfo)
   {
      temporaryData->EnsureExnInfo();
      temporaryData->ExnOutBV->CalculateDataflow(temporaryData->InBitVector,
         blockData->ExnKillBV, blockData->ExnGenBV);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Compute reaching definitions for each operand with alias tag.
//    Stores the information in extending objects attached to operands.
//
// Remarks:
//
//    We start with In set for each block and moving in forward direction,
//    compute which part of currently reaching defintions defines the alias
//    tag corresponding to each operand and save this part.
//    Then we update the reaching definitons information in pretty much
//    the same way we did it in EvaluateBlock.
//
//-----------------------------------------------------------------------------

void Walker::CollectOperandInfo()
{
   Phx::FunctionUnit  ^ functionUnit = this->FunctionUnit;
   Alias::Info    ^ aliasInfo = functionUnit->AliasInfo;
   DefsTable      ^ definitionTable = this->DefTable;

   Output::WriteLine("** Reaching Definitions for {0}", functionUnit->NameString);

   for each (Phx::Graphs::BasicBlock ^ block in functionUnit->FlowGraph->BasicBlocks)
   {
      Data ^  blockData = safe_cast<Data ^>(this->GetBlockData(block));

      // Start with In set we collected in Traverse.

      BitVector::Sparse ^ inBV = blockData->InBitVector;

      for each (Phx::IR::Instruction ^ instruction in block->Instructions)
      {
         // Ssa instruction should contribute nothing to dataflow.

         if (instruction->IsSsa)
         {
            continue;
         }

#if defined(PHX_DEBUG_DUMPS)

         // Dump results of the phase if requested so.

         if (Controls::DebugControls::VerboseTraceControl->IsEnabled(
               this->DebugControl, functionUnit))
         {
            Output::WriteLine(L"{0}", instruction->ToString());
         }
#endif

         // for each operand which may overlap with a set of tags
         // compute combined set of defintions for entire set and 
         // intersect it with the current set of reaching definitions.

         for each (Phx::IR::Operand ^ srcOperand in instruction->DataflowSourceOperands)
         {
            // Create extending object for the operand.

            OperandExtensionObject ^ opndExtensionObject =
               OperandExtensionObject::New(this->TemporaryLifetime);
            srcOperand->AddExtensionObject(opndExtensionObject);

            // Compute combined set of defintions overlapped with the operand.

            foreach_must_total_alias_of_tag(
               tag, srcOperand->AliasTag, aliasInfo)
            {
               opndExtensionObject->ReachingDefsBv->Or(
                  DefTable->GetAliasTagDefs(tag));
            }
            next_must_total_alias_of_tag ;

            // Intersect it with the current set of reaching defintions.

            opndExtensionObject->ReachingDefsBv->And(inBV);

            // Display results

            if (!opndExtensionObject->ReachingDefsBv->IsEmpty)
            {
               Phx::Debug::Info ^ debugInfo = functionUnit->DebugInfo;

               if (debugInfo->IsValidTag(instruction->DebugTag))
               {
                  Output::WriteLine(
                     L"Use of {0} at {1} line {2}: Reaching Definitions: {3}",
                     srcOperand->ToString(),
                     Path::GetFileName(debugInfo->GetFileName(instruction->DebugTag)),
                     debugInfo->GetLineNumber(instruction->DebugTag),
                     opndExtensionObject->ReachingDefsBv->ToString());
               }
               else
               {
                  Output::WriteLine(
                     L"Use of {0} (no source location available): " +
                     L"Reaching Definitions: {1}",
                     srcOperand->ToString(),
                     opndExtensionObject->ReachingDefsBv->ToString());
               }

               for each (DefId id in opndExtensionObject->ReachingDefsBv)
               {
                  Output::WriteLine(L"({0}){1}",
                     id.ToString(), definitionTable->GetDefInstr(id)->ToString());
               }
            }
         }

         // We finished collecting part. Now we should update current
         // set of reaching defintions based on definitions generated
         // by the current  instruction.

         for each (Phx::IR::Operand ^ dstOperand in instruction->DataflowDestinationOperands)
         {
            if (dstOperand->IsExpressionTemporary)
            {
               // Now it is time to record defintions of temps, so
               // they will appear in the current Generate set and 
               // will be seen when we run accross their uses.

               this->DefTable->AddLocDef(dstOperand->AliasTag, instruction);
            }
            else
            {
               // update information based on must part of dataflow

               foreach_must_total_alias_of_tag(
                  tag, dstOperand->AliasTag, aliasInfo)
               {
                  BitVector::Sparse ^ defsBv =
                     definitionTable->GetAliasTagDefs(tag);
                  inBV->Minus(defsBv);
               }
               next_must_total_alias_of_tag;
            }
         }

         // update gen information

         InstructionExtensionObject ^ instrExtensionObject =
            InstructionExtensionObject::GetExtensionObject(instruction);
         if (instrExtensionObject != nullptr)
         {
            inBV->Or(instrExtensionObject->DefsBv);
         }
      }
   }
}

//-----------------------------------------------------------------------------
//
// The rest of methods in the file implement properties of main classes and
// other mandatory methods which are required for proper functioning of 
// base classes, or extending objects defined in the file, but beyond that do
// not provide any interesting technical insights.
//
//-----------------------------------------------------------------------------

InstructionExtensionObject ^
InstructionExtensionObject::New
(
   Phx::Lifetime ^ lifetime
)
{
   InstructionExtensionObject ^ extensionObject = gcnew InstructionExtensionObject;
   extensionObject->DefsBv = BitVector::Sparse::New(lifetime);
   return extensionObject;
}

void
InstructionExtensionObject::Delete()
{
   this->DefsBv->Delete();
}

InstructionExtensionObject ^
InstructionExtensionObject::GetExtensionObject
(
   Phx::IR::Instruction ^ instruction
)
{
   Phx::IR::InstructionExtensionObject ^ extensionObject =
      instruction->FindExtensionObject(InstructionExtensionObject::typeid);
   return safe_cast<InstructionExtensionObject ^>(extensionObject);
}

OperandExtensionObject ^
OperandExtensionObject::New
(
   Phx::Lifetime ^ lifetime
)
{
   OperandExtensionObject ^ extensionObject = gcnew OperandExtensionObject;
   extensionObject->ReachingDefsBv = BitVector::Sparse::New(lifetime);
   return extensionObject;
}

void
OperandExtensionObject::Delete()
{
   this->ReachingDefsBv->Delete();
}

OperandExtensionObject ^
OperandExtensionObject::GetExtensionObject
(
   Phx::IR::Operand ^ operand
)
{
   Phx::IR::OperandExtensionObject ^ extensionObject =
      operand->FindExtensionObject(OperandExtensionObject::typeid);

   return safe_cast<OperandExtensionObject ^>(extensionObject);
}

DefsTable ^
Walker::DefTable::get()
{
   return this->definitionTable;
}

} // ReachingDefs
} // Samples
} // Phx
