//-----------------------------------------------------------------------------
//
// Description:
//
//   Loop Instrumentation
//
// Remarks:
//
//    Registers the "LoopInstrumentation" component control.
//
//-----------------------------------------------------------------------------

#include "LoopInstrumentation.h"

namespace LoopInstrumentation
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

   phase->Initialize(config, L"Loop Instrumentation");

#if defined(PHX_DEBUG_SUPPORT)

   phase->PhaseControl = Phase::LoopInstrumentationControl;

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

void Phase::Execute
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
               loop->FunctionUnit = functionUnit;
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

   // We need to do this for each module unit... 

   if (this->GlobalEnterStringSymbol == nullptr)
   {
      this->CreateLiteralStrings(functionUnit->ParentModuleUnit);
   }

   this->ImportLiteralStrings(functionUnit);

   for each (LoopExtensionObject ^ loop in loops)
   {
      if (loop->ParentLoop == nullptr)
      {
         loop->Instrument();
      }
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

void Phase::FindLoopBody(Phx::Graphs::BasicBlock ^ block)
{   
   // We should have attached an extension.

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

   Phase::LoopInstrumentationControl =
      Phx::Controls::ComponentControl::New(L"LoopInstrumentation",
      L"Loop Detection with Preheader and Postexit formation",
      L"LoopInstrumentation.cpp");

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
   return L"LoopInstrumentation";
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
   System::String ^ indent = gcnew System::String(' ', this->LoopDepth);

   System::Console::WriteLine("{0}Loop at {1} line {2}",
      indent,
      Phx::Utility::Undecorate(this->FunctionUnit->NameString, false),
      this->FunctionUnit->DebugInfo->GetLineNumber(this->LoopHeader->FirstInstruction->DebugTag));
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
      Phx::Graphs::BasicBlock ^ loopBlock = this->FunctionUnit->FlowGraph->Block(blockId);

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

//-----------------------------------------------------------------------------
//
// Description:
//
//    Instrument the loop described by the receiver.
//    
//-----------------------------------------------------------------------------

void LoopExtensionObject::Instrument()
{
   // Cache some useful stuff

   Phx::FunctionUnit ^ functionUnit = this->FunctionUnit;
   Phx::Types::Table ^ typeTable = functionUnit->TypeTable;
   Phx::Symbols::Table ^ symbolTable = functionUnit->SymbolTable;

   // Create the loop counter as a local variable.

   this->LoopName = System::String::Format("Loop{0}", 
      functionUnit->DebugInfo->GetLineNumber(this->LoopHeader->FirstInstruction->DebugTag));

   // Giving it a name will make IR dumps easier to understand.

   this->LoopCounterName = System::String::Format("{0}Counter", this->LoopName);

   Phx::Name loopCounterPhxName = Phx::Name::New(functionUnit->Lifetime, this->LoopCounterName);

   this->LoopCounterSymbol = Phx::Symbols::LocalVariableSymbol::NewAuto(symbolTable, 
      0, loopCounterPhxName, typeTable->Int32Type);

   // Create the symbol for printf
   //
   //    int __cdecl printf(const char *, ...);
   //
   // Here the __cdecl indicates the calling convention.
   //
   //  To begin with, we must create a
   // type object that represents the function signature.  In
   // Phoenix, function types are created with via the
   // FunctionTypeBuilder:

   Phx::Types::FunctionTypeBuilder ^ functionTypeBuilder =
      Phx::Types::FunctionTypeBuilder::New(typeTable);

   functionTypeBuilder->Begin();
   functionTypeBuilder->CallingConventionKind =
      Phx::Types::CallingConventionKind::CDecl;
   functionTypeBuilder->AppendReturnParameter(functionUnit->RegisterIntType);

   Phx::Types::Type ^ charType = typeTable->Character8Type;

   Phx::Types::Type ^ ptrToCharType = Phx::Types::PointerType::New(typeTable,
      Phx::Types::PointerTypeKind::UnmanagedPointer, typeTable->NativePointerBitSize,
      charType, nullptr);

   functionTypeBuilder->AppendParameter(ptrToCharType);

   functionTypeBuilder->AppendParameter(typeTable->UnknownType, Phx::Types::ParameterKind::Ellipsis);

   Phx::Types::FunctionType ^ printfType = functionTypeBuilder->GetFunctionType();

   // Now we're ready to create a symbol to represent printf itself. First
   // we must give it a linker name. To follow convention we prepend an
   // underscore. printf is external, so we indicate its visibility as a
   // reference.

   Phx::Name printfName = Phx::Name::New(functionUnit->Lifetime, "_printf");

   // The printf symbol is introduced at the ModuleUnit level.

   this->PrintfSymbol = Phx::Symbols::FunctionSymbol::New(
      functionUnit->ParentModuleUnit->SymbolTable, 0, printfName, printfType,
      Phx::Symbols::Visibility::GlobalReference);

   // Instrument preheaders, body, and postexits.

   for (Phx::Graphs::FlowEdge ^ edge = this->LoopHeader->PredecessorEdgeList; 
      edge != nullptr; 
      edge = edge->NextPredecessorEdge)
   {
      if (this->BackEdges->GetBit(edge->Id))
      {
         continue;
      }

      Phx::Graphs::BasicBlock ^ predBlock = edge->PredecessorNode;

      this->InstrumentPreheader(predBlock);
   }

   this->InstrumentBody(this->LoopHeader);

   for each (unsigned int blockId in this->ExitLoopBlocks)
   {
      Phx::Graphs::BasicBlock ^ loopBlock = this->FunctionUnit->FlowGraph->Block(blockId);

      for (Phx::Graphs::FlowEdge ^ edge = loopBlock->SuccessorEdgeList; 
         edge != nullptr; edge = edge->NextSuccessorEdge)
      {

         // Continue if this successor edge isn't the loop exit itself

         if (this->AllLoopBlocks->GetBit(edge->SuccessorNode->Id))
         {
            continue;
         }

         // If it's an exception edge, then we won't instrument this exit.

         if (edge->IsException)
         {
            continue;
         }

         this->InstrumentPostexit(edge->SuccessorNode);
      }
   }

   // Instrument any nested loops.

   for each(LoopExtensionObject ^ child in this->ChildLoops)
   {
      child->Instrument();
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Create printf format strings as global variables in the ModuleUnit.
//
// Remarks: 
//
//    The string symbols are recorded in properties GlobalEnterStringSymbol and
//    GlobalExitStringSymbol.
//    
//-----------------------------------------------------------------------------

void Phase::CreateLiteralStrings(Phx::ModuleUnit ^ moduleUnit)
{
   // Cache some useful stuff

   Phx::Types::Table ^ typeTable = moduleUnit->TypeTable;
   Phx::Symbols::Table ^ symbolTable = moduleUnit->SymbolTable;

   Phx::Symbols::SectionSymbol ^ stringSectSym = Phx::Symbols::SectionSymbol::New(symbolTable, 0,
      Phx::Name::New(moduleUnit->Lifetime, ".strings"));

   // We want this section to hold initialized, read only data. We
   // don't care about alignment, really, but will use 4 byte
   // minimum.

   stringSectSym->IsReadable = true;
   stringSectSym->IsInitialize = true;
   stringSectSym->Alignment = Phx::Alignment(Phx::Alignment::Kind::AlignTo4Bytes);

   // Along with the sectionSymbol we need a section. At this point we have
   // to divulge that we are creating output in COFF.

   Phx::Coff::Section ^ stringSection =
      Phx::Coff::Section::New(stringSectSym);

   stringSectSym->Section = stringSection;

   // Now to create the initial values. 

   System::String ^ enterString = "Entering loop on line %d\n";
   System::String ^ exitString = "Exiting loop on line %d (trip count %d)\n";

   // Now we need some symbols so we can refer to these strings in
   // our program IR. The names of these symbols are unimportant,
   // so we'll just make something up. The type of each symbol is
   // an array of characters with the appropriate length.

   this->GlobalEnterStringSymbol = this->CreateInitializedString(moduleUnit, enterString, "$SG1", stringSectSym);
   this->GlobalExitStringSymbol = this->CreateInitializedString(moduleUnit, exitString, "$SG2", stringSectSym);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Import module unit's symbols into the current function unit's symbol table.
//
// Remarks: 
//
//    In order to reference the printf strings in the code of a function unit,
//    the string symbols need to be imported into the function unit's table.
//    
//-----------------------------------------------------------------------------

void Phase::ImportLiteralStrings(Phx::FunctionUnit ^ functionUnit)
{
   Phx::Symbols::Table ^ symbolTable = functionUnit->SymbolTable;

   this->LocalEnterStringSymbol = 
      Phx::Symbols::NonLocalVariableSymbol::New(symbolTable, this->GlobalEnterStringSymbol);

   this->LocalExitStringSymbol = 
      Phx::Symbols::NonLocalVariableSymbol::New(symbolTable, this->GlobalExitStringSymbol);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Create an initialized string.
//
// Remarks: 
//
//    Initialized data is created as DataInstruction objects.
//    The Location property of a string symbol is set to the DataInstruction
//    that represents the initial value.
//    
//-----------------------------------------------------------------------------

Phx::Symbols::GlobalVariableSymbol ^
   Phase::CreateInitializedString
   (
   Phx::ModuleUnit ^ moduleUnit,
   System::String ^ value,
   System::String ^ symbolicName,
   Phx::Symbols::SectionSymbol ^ stringSectSym
   )
{
   // Cache some useful stuff

   Phx::Types::Table ^ typeTable = moduleUnit->TypeTable;
   Phx::Symbols::Table ^ symbolTable = moduleUnit->SymbolTable;

   unsigned int length = value->Length + 1;

   Phx::Name stringName =
      Phx::Name::New(moduleUnit->Lifetime, symbolicName);

   Phx::Types::Type ^ stringType =
      Phx::Types::UnmanagedArrayType::New(typeTable, Phx::Utility::BytesToBits(length),
      nullptr, typeTable->Character8Type);

   Phx::Symbols::GlobalVariableSymbol ^ stringSym =
      Phx::Symbols::GlobalVariableSymbol::New(symbolTable, 
      symbolTable->ExternIdMap->AllocateId(),
      stringName, stringType, Phx::Symbols::Visibility::File);

   stringSym->AllocationBaseSectionSymbol = stringSectSym;
   stringSym->Alignment = Phx::Alignment(Phx::Alignment::Kind::AlignTo1Byte);

   Phx::Section ^ stringSection = stringSectSym->Section;

   Phx::IR::DataInstruction ^ stringInstr =
      Phx::IR::DataInstruction::New(stringSection->DataUnit, length);

   stringInstr->WriteString(0, value);
   stringSection->AppendInstruction(stringInstr);
   stringSym->Location = Phx::Symbols::DataLocation::New(stringInstr);

   return stringSym;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate IR for printing the entry message and initializing the loop
//    counter to zero.
//
//-----------------------------------------------------------------------------

void LoopExtensionObject::InstrumentPreheader(Phx::Graphs::BasicBlock ^ block)
{
   // Print the entering message.... printf("Entering Loop%d", loopLineNumber);

   Phx::IR::Operand ^ enterStringOperand = Phx::IR::VariableOperand::New(this->FunctionUnit, 
      this->FunctionUnit->TypeTable->Character8Type, Phase::LocalEnterStringSymbol);
   enterStringOperand->ChangeToAddress();

   Phx::IR::Operand ^ loopLineNumberOperand = Phx::IR::ImmediateOperand::New(this->FunctionUnit, 
      this->FunctionUnit->TypeTable->Int32Type, 
      this->FunctionUnit->DebugInfo->GetLineNumber(this->LoopHeader->FirstInstruction->DebugTag));

   Phx::IR::Instruction ^ callInstruction = Phx::IR::CallInstruction::New(this->FunctionUnit, Phx::Common::Opcode::Call, this->PrintfSymbol);
   callInstruction->AppendSource(enterStringOperand);
   callInstruction->AppendSource(loopLineNumberOperand);

   block->LastInstruction->InsertBefore(callInstruction);
   callInstruction->DebugTag = block->LastInstruction->DebugTag;

   // Initialize the loop counter to zero.

   Phx::IR::Operand ^ loopCounter = Phx::IR::VariableOperand::New(this->FunctionUnit, this->LoopCounterSymbol->Type, this->LoopCounterSymbol);
   Phx::IR::Operand ^ zero = Phx::IR::ImmediateOperand::New(this->FunctionUnit, this->LoopCounterSymbol->Type, 0LL);
   Phx::IR::Instruction ^ assignInstruction = Phx::IR::ValueInstruction::NewUnary(this->FunctionUnit, Phx::Common::Opcode::Assign, loopCounter, zero);

   block->LastInstruction->InsertBefore(assignInstruction);
   assignInstruction->DebugTag = block->LastInstruction->DebugTag;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate IR to increment the counter variable.
//
// Remarks: 
//
//    This code is inserted in the loop header. 
//
//-----------------------------------------------------------------------------

void LoopExtensionObject::InstrumentBody(Phx::Graphs::BasicBlock ^ block)
{
   // Increment the counter variable.

   Phx::IR::Operand ^ loopCounter = Phx::IR::VariableOperand::New(this->FunctionUnit, this->LoopCounterSymbol->Type, this->LoopCounterSymbol);
   Phx::IR::Operand ^ one = Phx::IR::ImmediateOperand::New(this->FunctionUnit, this->LoopCounterSymbol->Type, 1LL);
   Phx::IR::Instruction ^ incrementInstruction = Phx::IR::ValueInstruction::NewBinary(this->FunctionUnit, Phx::Common::Opcode::Add, loopCounter, one, loopCounter);

   block->FirstInstruction->InsertAfter(incrementInstruction);
   incrementInstruction->DebugTag = block->FirstInstruction->DebugTag;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate IR for printing the exit message.
//
//-----------------------------------------------------------------------------

void LoopExtensionObject::InstrumentPostexit(Phx::Graphs::BasicBlock ^ block)
{
   // Print the exiting message.... printf("Exiting Loop%d (%d)", loopLineNumber, loopCount);

   Phx::IR::Operand ^ exitStringOperand = Phx::IR::VariableOperand::New(this->FunctionUnit, 
      this->FunctionUnit->TypeTable->Character8Type, Phase::LocalExitStringSymbol);
   exitStringOperand->ChangeToAddress();

   Phx::IR::Operand ^ loopLineNumberOperand = Phx::IR::ImmediateOperand::New(this->FunctionUnit, 
      this->FunctionUnit->TypeTable->Int32Type, 
      this->FunctionUnit->DebugInfo->GetLineNumber(this->LoopHeader->FirstInstruction->DebugTag));

   Phx::IR::Operand ^ loopCounterOperand = Phx::IR::VariableOperand::New(this->FunctionUnit, 
      this->LoopCounterSymbol->Type, this->LoopCounterSymbol);

   Phx::IR::Instruction ^ callInstruction = Phx::IR::CallInstruction::New(this->FunctionUnit, Phx::Common::Opcode::Call, this->PrintfSymbol);
   callInstruction->AppendSource(exitStringOperand);
   callInstruction->AppendSource(loopLineNumberOperand);
   callInstruction->AppendSource(loopCounterOperand);

   block->FirstInstruction->InsertAfter(callInstruction);
   callInstruction->DebugTag = block->FirstInstruction->DebugTag;
}

}
