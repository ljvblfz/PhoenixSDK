//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//
// Remarks:
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Phases.h"

using namespace System;

OptimizationPhase ^
OptimizationPhase::New
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   OptimizationPhase ^ phase = gcnew OptimizationPhase();

   phase->Initialize(config, "Expression Optimizations");

   return phase;
}

void
OptimizationPhase::Execute
(
   Phx::Unit ^ unit
)
{
   Phx::FunctionUnit ^ functionUnit = unit->AsFunctionUnit;

   Phx::IR::Instruction ^ instruction = functionUnit->FirstInstruction;
   while (instruction)
   {
      Phx::Expression::Optimization::Optimize(
         instruction, 
         Phx::Expression::CompareControl::Expressions, 
         Phx::Expression::OptimizationControl::Full
      );
      instruction = instruction->Next;
   }
}

Encode ^
Encode::New
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   Encode ^ encode = gcnew Encode();

   encode->Initialize(config, "Encoding");

#if (PHX_DEBUG_SUPPORT)
   encode->PhaseControl = Phx::Targets::Runtimes::Encode::DebugControl;
#endif

   return encode;
} 

void
Encode::Execute
(
   Phx::Unit ^ unit
)
{
   Phx::FunctionUnit ^ functionUnit = unit->AsFunctionUnit;
   Phx::Targets::Runtimes::Encode ^ encode = functionUnit->Encode;
   Phx::Unit ^ module = functionUnit->ParentUnit;
   Phx::Symbols::FunctionSymbol ^ functionSymbol = 
      functionUnit->FunctionSymbol;

   functionUnit->EncodedIRLifetime = module->Lifetime;
   unsigned int binarySize = encode->FinalizeIR();

   Phx::IR::DataInstruction ^ encodedInstruction =
      Phx::IR::DataInstruction::New(module, binarySize);
  
   // The function is this big.

   functionSymbol->ByteSize = binarySize;

   encode->Function(encodedInstruction->GetDataPointer(0));

   // Copy fixups and debug.

   encodedInstruction->FixupList = encode->FixupList;
   encodedInstruction->DebugOffsets = encode->DebugOffsets;

   // Set the func sym's location to the encoded IR.

   functionSymbol->Location = 
      Phx::Symbols::DataLocation::New(encodedInstruction);

   // Add the encoded IR to the appropriate section.

   Phx::Symbols::SectionSymbol ^ textSectionSymbol =
      module->SymbolTable->ExternIdMap->Lookup(
         (unsigned int)Phx::Symbols::PredefinedCxxILId::Text)->AsSectionSymbol;

   textSectionSymbol->Section->AppendInstruction(encodedInstruction);

   // And note that the function ended up here.

   functionSymbol->AllocationBaseSectionSymbol = textSectionSymbol;
}

Lister ^
Lister::New
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   Lister ^ lister = gcnew Lister();

   lister->Initialize(config, "Lister");

#if (PHX_DEBUG_SUPPORT)
   lister->PhaseControl = Phx::Targets::Runtimes::Lister::DebugControl;
#endif
   
   return lister;
}

void
Lister::Execute
(
   Phx::Unit ^ unit
)
{
   Phx::FunctionUnit ^ functionUnit = unit->AsFunctionUnit;

   functionUnit->Lister->Function();
}