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


namespace Phx
{

namespace Samples
{

namespace LocalOpt
{

// use more readable names for generics classes

using DagEdgeList = System.Collections.Generic.List<DagEdge>;
using DagNodeList = System.Collections.Generic.List<DagNode>;
using OpndKeyList = System.Collections.Generic.List<OpndKey>;
using OpndToOpndMap = Hashtable<OpndKey, Phx.IR.Operand>;


//-----------------------------------------------------------------------------
//
// Description:
//
//    The Phx PlugIn wrapper of the local optimization phase
//
//-----------------------------------------------------------------------------

public class PlugIn : Phx.PlugIn
{
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Register local optimization plug-in control.
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   public override void
   RegisterObjects()
   {
      // Register objects for the LocalOptPhase class

      LocalOptPhase.StaticInitialize();
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Create and insert local optimization phase after the MIR lower phase
   //
   // Arguments:
   //
   //    config - The PhaseConfiguration that this Plug-in will belong to
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   public override void
   BuildPhases
   (
      Phx.Phases.PhaseConfiguration config
   )
   {
      // First find the MIR Lower phase, after where we will insert out phase

      Phx.Phases.Phase mirlowerPhase;

      mirlowerPhase =
         config.PhaseList.FindByName("MIR Lower") as Phx.Phases.Phase;

      if (mirlowerPhase == null)
      {
         // Need a real error.

         Phx.Output.WriteLine("MIR Lower phase not found in phaselist:");
         Phx.Output.Write(config.ToString());

         return;
      }

      // Then, create a new instance of LocalOptPhase and insert it after
      // the MIR Lower phase

      Phx.Phases.Phase phase = LocalOptPhase.New(config);

      mirlowerPhase.InsertAfter(phase);
   }

   public override System.String NameString
   {
      get
      {
         return "Local Optimizer";
      }
   }
}

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

public class LocalOptPhase : Phx.Phases.Phase
{

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Static constructor for the Local Optimization phase.
   //
   // Arguments:
   //
   //    config - The PhaseConfiguration that this phase will belong to
   //
   // Returns:
   //
   //    LocalOptPhase object.
   //
   //--------------------------------------------------------------------------

   public static LocalOptPhase
   New
   (
      Phx.Phases.PhaseConfiguration config
   )
   {
      LocalOptPhase phase = new LocalOptPhase();

      phase.Initialize(config, "DAG-based Local Optimizations");

#if (PHX_DEBUG_CHECKS)

      phase.PhaseControl = localOptCompCtrl;

#endif // PHX_DEBUG_CHECKS


      return phase;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Static initialization for the LocalOptPhase class. Right now, only
   //    a component control is registered if it's debug build
   //
   // Returns:
   //
   //    Nothing
   //
   //--------------------------------------------------------------------------

   public static void
   StaticInitialize()
   {
#if (PHX_DEBUG_CHECKS)

      // This control is for debug purpose only. If it is registered,
      // it can be used with some Phoenix diagnotic controls. e.g.
      // -dump:localopt, -predump:localopt, etc.

      localOptCompCtrl = Phx.Controls.ComponentControl.New("localOpt",
         "perform DAG-based local optimizations; called after MIR Lower",
         "localopt-plug-in.cs");

#endif // PHX_DEBUG_CHECKS


   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Executes the Local Optimization phase.
   //
   // Arguments:
   //
   //    unit - The function unit to be processed
   //
   // Returns:
   //
   //    Nothing
   //
   //--------------------------------------------------------------------------

   protected override void
   Execute
   (
      Phx.Unit unit
   )
   {
      // check if the input unit is a FunctionUnit

      if (!unit.IsFunctionUnit)
      {
         return;
      }

      Phx.FunctionUnit functionUnit = unit as Phx.FunctionUnit;

      // check if alias information is ready

      if (!functionUnit.AliasInfo.IsComplete)
      {
         Phx.Output.WriteLine("Alias information has to be collected in"
            + "order to collect reaching definitions");
         return;
      }

      // prepare flow graph for the current phase

      Phx.Graphs.FlowGraph fg = functionUnit.FlowGraph;

      // If a basic block has a handler edge, its temporaries are not killed
      // across the basic block boundary. In this case, we need to construct
      // the extended basic block first

      functionUnit.BuildFlowGraph(Phx.Graphs.BlockStyle.IgnoreHandlerEdges);

      // Some ExprTmps are live cross basic blocks, which breaks the assumption
      // of the localOpt phase. See Phoenix IR of \benchi\bench.c, simple(), t183
      // ExpressionBuilder will force to break ExprTmps across basic blocks

      functionUnit.ExpressionBuilder.Function(functionUnit, false, false, true);

      // create a new instance of the local optimizer

      Optimization optimizer = Optimization.New(functionUnit);

      // perform local optimization for each basic block

      foreach (Phx.Graphs.BasicBlock block in
         Phx.Graphs.BasicBlock.Iterator(functionUnit.FlowGraph))
      {
         // Skip some special flow basic blocks, such as start/end/exit

         if (block.IsStart || block.IsEnd || block.IsExit)
         {
            continue;
         }

         // Perform dag-based local optimization on the extended basic block
         // The instruction count is used as a hint to the size of hash tables
         // in the DAG

         optimizer.DoRange(block.FirstInstruction, block.LastInstruction,
            (int)block.InstructionCount);
      }

      // delete the flow graph if it does not exist when entering this phase

      if (fg == null)
      {
         functionUnit.DeleteFlowGraph();
      }

      // build expression temporary again to improve quality of register allocation

      functionUnit.ExpressionBuilder.Function(functionUnit, false, false, true);

   }




#if (PHX_DEBUG_CHECKS)

   // Component control for debugging purpose only

   private static Phx.Controls.ComponentControl localOptCompCtrl;

#endif // PHX_DEBUG_CHECKS


}

//-----------------------------------------------------------------------------
//
// Description:
//
//    The dAG-based local optimizer.
//
//-----------------------------------------------------------------------------

public class Optimization : Phx.Object
{
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Static constructor for the Local Optimizer.
   //
   // Arguments:
   //
   //    functionUnit - The function unit to be processed
   //
   // Returns:
   //
   //    LocalOpt.Optimization object.
   //
   //--------------------------------------------------------------------------

   public static Optimization
   New
   (
      Phx.FunctionUnit functionUnit
   )
   {
      Optimization newOptimizer = new Optimization();

      // cache the current functionUnit and its aliasInfo

      newOptimizer.functionUnit = functionUnit;
      newOptimizer.aliasInfo = functionUnit.AliasInfo;

      return newOptimizer;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Perform local optimization on the given instruction range
   //
   // Arguments:
   //
   //    start - The first instruction in the range
   //    end   - The last instruction in the range
   //    instructionCount - Number of instructions in the range
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   public void
   DoRange
   (
      Phx.IR.Instruction start,
      Phx.IR.Instruction end,
      int instructionCount
   )
   {
      Phx.IR.Instruction startInstruction = start;
      Phx.IR.Instruction endInstruction = end;

      // Skip instructions that are not real at the beginning of block

      while (startInstruction!=null && !startInstruction.IsReal && startInstruction != endInstruction)
      {
         startInstruction = startInstruction.Next;
      }

      if (startInstruction == null)
      {
         // no real instructions, simply return
         return;
      }

      // Skip instructions that are not real or are unconditional branch at
      // the end of block

      while ((endInstruction != null && endInstruction!= startInstruction)
         && (!endInstruction.IsReal 
            || (endInstruction.IsBranchInstruction && endInstruction.AsBranchInstruction.IsUnconditional)))
      {
         endInstruction = endInstruction.Previous;
      }

      if (startInstruction == endInstruction)
      {
         // only one instruction, no need to do optimization
         return;
      }

      // DAG cannot handle some types of instructions, such as CMP, MULTTIBYTE,
      // either because that they have special evaluation order constraints
      // which are not caught by data dependency, or they have more than one
      // explicit dst operand. For such instructions, we first process all
      // instructions before them, delete the DAG, handle these special
      // instructions one by one, and restart a new DAG once we find another
      // sequence of candidate instructions. It's possible that some source
      // operands used by these special instructions have been renamed by the
      // DAG process, or can be simplify based on the result of the previous
      // DAG analysis. Therefore, we keep a renameMap for this purpose.

      // Create a renameMap to be used by DAG construction

      this.renameMap = new OpndToOpndMap(instructionCount);

      Phx.IR.Instruction guardInstr = endInstruction.Next;

      while (startInstruction != guardInstr)
      {
         // Create a new DAG use the instruction count of the basic block as
         // its size hint

         DAG theDag = DAG.New(this.functionUnit, instructionCount);

         // Build DAG structures for the current instruction range

         Phx.IR.Instruction stopAt = this.BuildDAG(theDag, startInstruction, endInstruction);

         // Find some special instructions, handle them separately

         if (stopAt != startInstruction && stopAt != startInstruction.Next)
         {
            // More than one instructions are processed by BuildDAG()
            // generate code for instructions processed so far
            this.GenCodeFromDag(theDag, startInstruction, stopAt.Previous);
         }

         while (stopAt != guardInstr && IsNotHandled(stopAt))
         {
            // Process all special instructions until reach an instruction
            // can be processed by DAG

            stopAt = this.Transfer(stopAt);
            stopAt = stopAt.Next;
         }

         this.renameMap.Clear();

         startInstruction = stopAt;
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Build the DAG structures for the given instruction range.
   //
   // Arguments:
   //
   //    theDag - The DAG table to be constructed
   //    startInstruction - The first instruction to process
   //    endInstruction - The last instruction to process
   //
   // Returns:
   //
   //    The next instruction of the last instruction handled or the special
   //    instruction that stops the building process
   //
   //--------------------------------------------------------------------------

   public Phx.IR.Instruction
   BuildDAG
   (
      DAG theDag,
      Phx.IR.Instruction startInstruction,
      Phx.IR.Instruction endInstruction
   )
   {

      Phx.IR.Instruction instruction = null;

      for (instruction = startInstruction; instruction != endInstruction.Next; instruction = instruction.Next)
      {
         if (Optimization.IsNotHandled(instruction))
         {
            // the instruction needs special care
            return instruction;
         }

         DagNode newTargetNode = null;

         // Find or create a dag node for its right hand side

         if (instruction.IsCopy)
         {
            // A simple assignment instruction, find or create a use node

            newTargetNode =
               theDag.FindOrCreateNode(instruction.SourceOperand, DagNodeKind.Use);

            if(this.aliasInfo.MustExactlyOverlap(instruction.DestinationOperand,
                                                newTargetNode.Label))
            {
               // Resolved to something like y = y, ignore this instruction
               continue;
            }

         }
         else
         {
            // Other kind of instructions, create an expression node

            newTargetNode = theDag.FindOrCreateNode(
               instruction.DestinationOperand, DagNodeKind.Expression);

         }

         // Kill all nodes whose values might be affected by the dst operands
         // of this instruction. And Create new Definition nodes for the explicit dst
         // operand of this instruction and attach it to the newTargetNode

         theDag.HandleDstOpnd(instruction, newTargetNode);

      }

      return instruction;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Check whether the current instruction is a special instruction that
   //    could not be handled by the DAG process
   //
   // Arguments:
   //
   //    instruction - The instruction that asks the "IsNotHandled" question
   //
   // Returns
   //
   //    True if it is not handled, otherwise false
   //
   //--------------------------------------------------------------------------

   public static bool
   IsNotHandled
   (
      Phx.IR.Instruction instruction
   )
   {
      // We don't handle a non-value instruction unless it's a call

      if (!instruction.IsValueInstruction && !instruction.IsCallInstruction)
      {
         return true;
      }

      // We don't handle the following instructions in DAG



      Phx.Opcode opcode = (Phx.Opcode)instruction.Opcode;

      switch (Phx.Common.Opcode.GetIndex(opcode))
      {
         case Phx.Common.Opcode.Index.Phi:
         case Phx.Common.Opcode.Index.Chi:
         case Phx.Common.Opcode.Index.MemoryCopy:
         case Phx.Common.Opcode.Index.CpBlk:
         case Phx.Common.Opcode.Index.MemorySet:
         case Phx.Common.Opcode.Index.InitObj:
         case Phx.Common.Opcode.Index.InitBlk:
         case Phx.Common.Opcode.Index.Question:
         case Phx.Common.Opcode.Index.Switch:
         case Phx.Common.Opcode.Index.MultipleByteAllocate:
         case Phx.Common.Opcode.Index.MultipleByteArgument:
         case Phx.Common.Opcode.Index.Throw:
         case Phx.Common.Opcode.Index.Rethrow:
         case Phx.Common.Opcode.Index.Nop:
            return true;
         // What else?
      }

      return false;

   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Regenerate Phoenix IR from the DAG structures of the given instruction
   //    range. Generated instruction list will be linked before the startInstruction,
   //    and then the old instruction range will be unlinked. Also, the rename-
   //    Map will be updated based on the result of current DAG process
   //
   // Arguments:
   //
   //    theDag - The DAG that has been constructed so far
   //    startInstruction - The first instruction in the instruction range
   //    endInstruction - The last instruction in the instruction range
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   public void
   GenCodeFromDag
   (
      DAG theDag,
      Phx.IR.Instruction startInstruction,
      Phx.IR.Instruction endInstruction
   )
   {

#if (PHX_DEBUG_CHECKS)

      Phx.Asserts.Assert(this.renameMap != null,
         "LocalOpt.Optimization.GenCodeFromDag, this.renameMap != null");

#endif

      // Create a new GenCodeWalker

      GenCodeWalker walker = GenCodeWalker.New(this.functionUnit, theDag, startInstruction);

      // DAG is a forest. All root nodes are held by the rootlist of the DAG.
      // Iterate over the rootlist and generate code for each substree.

      foreach (DagNode rootNode in theDag.RootList)
      {
         // a root node in the DAG, gen code for it
         walker.Walk(rootNode);
      }

      // Update the renameMap using the result of the current DAG process

      theDag.UpdateRenameMap(this.renameMap);

      // Unlink the old instruction range

      IR.InstructionRange range = new IR.InstructionRange(startInstruction, endInstruction);
      Phx.IR.Instruction.Unlink(range);

   }


   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Handle special instructions that could not be processed by the DAG.
   //    If an entry exists in the renameMap for any src operand of the given
   //    instruction, change it to the corresponding opreand cached in the map.
   //
   // Arguments:
   //
   //    instruction - The special instruction to transfer
   //
   // Returns:
   //
   //    The transfered instruction
   //
   //--------------------------------------------------------------------------

   public Phx.IR.Instruction
   Transfer
   (
      Phx.IR.Instruction instruction
   )
   {
      Phx.IR.Instruction newInstruction = instruction;

      // Iterate through the explicit src operand list and replace them
      // by its current value if it has an entry in the renameMap

      if (instruction.LastExplicitSourceOperand != null)
      {

         Phx.IR.Operand srcOperand = instruction.SourceOperand;
         Phx.IR.Operand prevSrcOpnd = srcOperand;
         bool firstSrc = true;

         while (srcOperand != null)
         {
            if (srcOperand.IsDataflow)
            {
               // We only care about dagflow operands


               if (srcOperand.IsMemoryOperand)
               {
                  // It's possible that the base/index/segment operands of a
                  // memoryOperand are renamed by the DAG process, resolve them
                  // first before lookup the renameMap

                  srcOperand = ResolveMemOpnd(srcOperand.AsMemoryOperand);

               }

               // Lookup the renameMap to see if any information is cached
               // for the current src operand

               Phx.IR.Operand newSourceOperand = this.renameMap.Lookup(
                  OpndKey.New(srcOperand, DAG.GetHashCode(srcOperand)));

               if(newSourceOperand != null)
               {
                  // An entry exists for the given src operand
                  // Replace the old src operand with the new one

                  if (newSourceOperand != srcOperand
                     && !this.aliasInfo.MustExactlyOverlap(srcOperand, newSourceOperand))
                  {
                     instruction.ReplaceSource(srcOperand, newSourceOperand);
                  }
               }
            }

            // It is possible that srcOperand.Next is unlinked due to resolving
            // of the current srcOperand if it's a memoryOperand, i.e., it's an editing
            // while iterating process. We need the previous srcOperand to handle
            // the loop correctly.

            if (firstSrc)
            {
               //first srcOperand
               srcOperand = instruction.SourceOperand.Next;
               prevSrcOpnd = instruction.SourceOperand;
               firstSrc = false;
            }
            else
            {
               if (prevSrcOpnd.Next == null)
               {
                  break;
               }
               srcOperand = prevSrcOpnd.Next.Next;
               prevSrcOpnd = prevSrcOpnd.Next;
            }
         }
      }

      // Remove all opnds that become invalid due to dstOpnds from the
      // renameMap

      foreach(Phx.IR.Operand dstOperand in instruction.DestinationOperands)
      {
         if (dstOperand.IsTemporary)
         {
            // temporaries never kill any operand assuming it has only one definition
            continue;
         }

         if (dstOperand.IsMemoryOperand)
         {
            // Again, we need to resolve a memory operand's base/index/segment
            // operands first

            ResolveMemOpnd(dstOperand.AsMemoryOperand);

         }

         // Put what we want to delete in a list and delete them after
         // iteration to avoid the editing-while-iterating situation.

         OpndKeyList toDelete = new OpndKeyList();

         foreach (OpndKey key in this.renameMap.Keys)
         {
            Phx.IR.Operand operand = key.Operand;

            if (operand.IsTemporary)
            {
               // Temporaries will not never be killed assuming it has one definition
               continue;
            }

            if(this.functionUnit.AliasInfo.MayPartiallyOverlap(dstOperand, operand))
            {
               // The two operands might have effects to each other. Put operand
               // to the toDelete list

               toDelete.Add(key);
            }
         }

         // Remove all operands in the toDelete list from the renameMap

         foreach (OpndKey key in toDelete)
         {
            this.renameMap.Remove(key);
         }
      }

      // Try to perform a simple branch folding optimization

      newInstruction = this.SimpleBROpt(instruction);

      return newInstruction;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Resolve the baseOperand/indexOperand/SegmentOperand of the given memory operand node
   //
   // Arguments:
   //
   //    memoryOperand - The memory operand to be resolved
   //
   // Returns:
   //
   //    The resolved memory operand
   //
   //--------------------------------------------------------------------------

   private Phx.IR.Operand
   ResolveMemOpnd
   (
      Phx.IR.MemoryOperand memoryOperand
   )
   {

      Phx.IR.Operand baseOperand = memoryOperand.BaseOperand;

      if (baseOperand != null)
      {
         Phx.IR.Operand newBaseOperand = this.renameMap.Lookup(
            OpndKey.New(baseOperand, DAG.GetHashCode(baseOperand)));

         if(newBaseOperand != null)
         {

            // Update the BaseOperand if it is resolved to something else

            if(!this.aliasInfo.MustExactlyOverlap(baseOperand, newBaseOperand))
            {
               memoryOperand.BaseOperand = newBaseOperand.AsVariableOperand;
            }
         }
      }

      Phx.IR.Operand indexOperand = memoryOperand.IndexOperand;
      if (indexOperand != null)
      {
         // Update the IndexOperand if it is resolved to something else

         Phx.IR.Operand newIndexOperand = this.renameMap.Lookup(
            OpndKey.New(indexOperand, DAG.GetHashCode(indexOperand)));

         if(newIndexOperand != null &&
            !this.aliasInfo.MustExactlyOverlap(indexOperand, newIndexOperand))
         {
            memoryOperand.IndexOperand = newIndexOperand.AsVariableOperand;
         }
      }

      Phx.IR.Operand segmentOperand = memoryOperand.SegmentOperand;

      if (segmentOperand != null)
      {
         // Update the SegmentOperand if it is resolved to something else

         Phx.IR.Operand newSegmentOperand = this.renameMap.Lookup(
            OpndKey.New(segmentOperand, DAG.GetHashCode(segmentOperand)));

         if(newSegmentOperand != null &&
            !this.aliasInfo.MustExactlyOverlap(segmentOperand, newSegmentOperand))
         {
            memoryOperand.SegmentOperand = newSegmentOperand.AsVariableOperand;
         }
      }

      return memoryOperand;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Perform a simple optimization that translate simply sequences like
   //
   //       t113 = CMP(NE) 1, 0
   //       CBRANCH(NE) t113, $L7, $L6
   //
   //    To
   //       GOTO  $L7
   //
   // Arguments:
   //
   //    instruction - The current instruction that is being tranferred
   //
   // Returns:
   //
   //    The input instruction if no optimization is performed. Otherwise,
   //    the new GOTO instruction is returned.
   //
   //--------------------------------------------------------------------------

   private Phx.IR.Instruction
   SimpleBROpt
   (
      Phx.IR.Instruction instruction
   )
   {
      Phx.IR.Instruction newInstruction = instruction;

      // Determine whether the input instruction is a candidate of our
      // simple branch optimization

      if (instruction.IsCompareInstruction
         && instruction.SourceOperand1.IsIntImmediate && instruction.SourceOperand2.IsIntImmediate
         && instruction.Next.IsBranchInstruction && instruction.Next.AsBranchInstruction.IsConditional)
      {
         Phx.IR.CompareInstruction compareInstruction = instruction.AsCompareInstruction;
         Phx.IR.BranchInstruction  branchInstruction = instruction.Next.AsBranchInstruction;

         // Try to fold the CMP instruction

         Phx.Constant.Table constantTable = compareInstruction.FunctionUnit.ConstantTable;

         Phx.Constant.ArithmeticValue srcValue1 = constantTable.LookupOrInsert(
            compareInstruction.SourceOperand1.AsImmediateOperand.IntValue);

         Phx.Constant.ArithmeticValue srcValue2 = constantTable.LookupOrInsert(
            compareInstruction.SourceOperand2.AsImmediateOperand.IntValue);

         Phx.Constant.ArithmeticValue foldedValue;

         Phx.Constant.Result result = new Phx.Constant.Result();

         foldedValue = Phx.Constant.Info.EvaluateBinary(ref result, instruction.Opcode,
            compareInstruction.ConditionCode, compareInstruction.SourceOperand1.Type, 
            srcValue1, srcValue2);

         if (foldedValue != null && !result.HasRestrictions)
         {
            // Folding succeeds

            Phx.IR.LabelOperandKind targetKind;

            if (foldedValue.IsZero)
            {
               // The condition is evaluated to false, goto the not-taken edge
               targetKind = Phx.IR.LabelOperandKind.False;
            }
            else
            {
               // The condition is evaluated to true, goto the taken edge
               targetKind = Phx.IR.LabelOperandKind.True;
            }

            // Clear the profile tags of the original branch edge that will be
            // removed

            Phx.Profile.Info profileInfo = branchInstruction.FunctionUnit.ProfileInfo;

            foreach(Phx.IR.Operand srcOperand1 in branchInstruction.SourceOperands)
            {
               if (srcOperand1.IsLabelOperand
                  && srcOperand1.AsLabelOperand.LabelOperandKind != targetKind)
               {
                  profileInfo.ClearData(srcOperand1);
               }
            }

            // Change the conditional branch to a unconditional branch

            branchInstruction.ChangeToUnconditionalBranch(targetKind);

            newInstruction = branchInstruction;

            // Unlink the original CMP instruction

            compareInstruction.Unlink();
         }
      }

      return newInstruction;
   }

   // private members

   private Phx.FunctionUnit functionUnit;

   private Phx.Alias.Info aliasInfo;

   // A rename map used for keeping mapping between original name of
   // temporaries and their new names.

   private OpndToOpndMap renameMap;
}


} // namespace LocalOpt
} // namespace Samples
} // namespace Phx
