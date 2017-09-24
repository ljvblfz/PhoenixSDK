//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    DAG-based local optimizations plug-in.
//
// Remarks:
//
//    Registers "localOpt" component control if in the debug mode
//
//    The plug-in injects a phase after MIR Lower. The phase performs DAG-based 
//    local optimizations, including copy propagation, constant propagation, 
//    constant folding, CSE, simple scalar promotion and dead code elimination.
//    The main algorithms are from Section 9.8 of the red dragon compiler book.
//
// Usage:
//
//    csc /r:phx.dll /t:library /out:localOpt.dll 
//       localOpt-plug-in.cs dag.cs dagwalker.cs utility.cs
//
//    cl ... -d2plugin:localOpt.dll ...
//
//    Right now, optimizations available in the LocalOpt plug-in are:
//
//       FoldConstant         constant folding
//       DeadStore            dead store elimination
//       DeadExpr             dead expression elimination
//       ReduceStrength       simple strength reduction e.g. MUL X 4 -> SHL X 2
//    
//-----------------------------------------------------------------------------

#pragma once

#include "dag.h"

namespace Phx
{

namespace Samples
{

namespace LocalOpt
{

// use more readable names for generics classes

typedef System::Collections::Generic::List<DagEdge ^> DagEdgeList;
typedef System::Collections::Generic::List<DagNode ^> DagNodeList;
typedef System::Collections::Generic::List<OpndKey ^> OpndKeyList;
typedef Hashtable<OpndKey ^, Phx::IR::Operand ^> OpndToOpndMap;

//-----------------------------------------------------------------------------
//
// Description:
//
//    The Phx Plugin wrapper of the local optimization phase
//
//-----------------------------------------------------------------------------

public ref class Plugin : Phx::PlugIn
{
public:

   // public methods

   virtual void RegisterObjects() override;

   virtual void
   BuildPhases
   (
      Phx::Phases::PhaseConfiguration ^ config
   ) override;

   property System::String ^ NameString
   {
      virtual System::String ^ get() override { return L"Local Optimizer"; }
   }
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    Local optimization phase.
//    
// Remarks:
//
//    This optimization phase works in a per basic block style. But if a blcok
//    ends with an instruction with exception handlers, such as calls, we try
//    to merge it with its next block to form an extended basic block so that
//    more optimization opportunities can be exploited.
//
//-----------------------------------------------------------------------------

public ref class LocalOptPhase : Phx::Phases::Phase
{
public:

   // public methods

   static LocalOptPhase ^
   New
   (
      Phx::Phases::PhaseConfiguration ^ config
   ) ;

   static void StaticInitialize ();

protected:

   // protected methods

   virtual void Execute (Phx::Unit ^ unit) override ;

private:

#if defined(PHX_DEBUG_CHECKS)

   // Component control for debugging purpose only

   static Phx::Controls::ComponentControl ^ localOptCompCtrl;

#endif

};

//-----------------------------------------------------------------------------
//
// Description:
//
//    The dAG-based local optimizer.
//    
//-----------------------------------------------------------------------------

public ref class Optimization : Phx::Object
{
public:

   static Optimization ^
   New
   (
      Phx::FunctionUnit ^ functionUnit
   );

   void
   DoRange
   (
      Phx::IR::Instruction ^ start,
      Phx::IR::Instruction ^ end,
      int              instructionCount
   );

   Phx::IR::Instruction ^
   BuildDAG
   (
      DAG ^            theDag,
      Phx::IR::Instruction ^ startInstruction,
      Phx::IR::Instruction ^ endInstruction
   );

   static bool
   IsNotHandled
   (
      Phx::IR::Instruction ^ instruction
   ) ;

   void
   GenCodeFromDag
   (
      DAG ^            theDag,
      Phx::IR::Instruction ^ startInstruction,
      Phx::IR::Instruction ^ endInstruction
   );

   Phx::IR::Instruction ^
   Transfer
   (
      Phx::IR::Instruction ^ instruction
   );

private:

   Phx::IR::Operand ^
   ResolveMemOpnd
   (
      Phx::IR::MemoryOperand ^ memoryOperand
   );

   Phx::IR::Instruction ^
   SimpleBROpt
   (
      Phx::IR::Instruction ^ instruction
   );

private:

   Phx::FunctionUnit ^ functionUnit;

   Phx::Alias::Info ^ aliasInfo;

   // A rename map used for keeping mapping between original name of 
   // temporaries and their new names.

   OpndToOpndMap ^ renameMap;

};

} // namespace LocalOpt
} // namespace Samples
} // namespace Phx
