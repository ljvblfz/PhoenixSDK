using System;
using System.Collections.Generic;
using System.Text;

// AliasTag is typedef'd to be an int in Phoenix
// but this must be added to use it from C#
using AliasTag = System.Int32;

namespace SliceAnalysis
{
   /// <summary>
   /// To be plugged into a Phx.Graphs.NodeWalker to find the reaching
   /// definitions of a given instruction.
   /// </summary>
   class BasicBlockVisitor : Phx.Graphs.NodeVisitor
   {
      private Dictionary<string, Phx.IR.Operand> _reachingDefs =
          new Dictionary<string, Phx.IR.Operand>();

      public Dictionary<string, Phx.IR.Operand> ReachingDefs
      { get { return _reachingDefs; } }

      private Phx.IR.Instruction _startInstr;

      private SliceAnalysis.Utilities.Set<AliasTag> killSet =
          new SliceAnalysis.Utilities.Set<AliasTag>();

      public BasicBlockVisitor(Phx.IR.Instruction startInstruction)
      {
         _startInstr = startInstruction;
      }

      public override void PreVisit(Phx.Graphs.Node node)
      {
         Phx.Graphs.BasicBlock block = node.AsBasicBlock;
         bool foundStartInstr = false;

         // For each basic block we visit, we always start from its last
         // instruction to look for definitions.  The only exception is the first
         // block we encounter.  Our starting instruction might not 
         // be the last instruction of its basic block, and we don't want
         // to include definitions after the starting instruction.
         // TODO: RaviR-08/04/06 Check for correctness.
         Phx.IR.Instruction instruction = block.LastInstruction;
         while (instruction != null)
         {
            if (block == _startInstr.BasicBlock)
            {
               if (!foundStartInstr)
               {
                  if (instruction == _startInstr)
                  {
                     foundStartInstr = true;
                  }
                  else
                  {
                     instruction = instruction.Previous;
                     continue;
                  }
               }
            }

            // Kill all operands that alias with the modref operand on this
            // call instruction because the call could potentially redefine
            // any such operand.
            if (instruction.IsCallInstruction)
            {
               foreach (Phx.IR.Operand dstOperand in instruction.DestinationOperands)
               {
                  if (dstOperand.IsAliasOperand)
                  {
                     killSet.Add(dstOperand.AliasTag);
                  }
               }
            }

            foreach (Phx.IR.Operand operand in instruction.ExplicitDestinationOperands)
            {
               string opndName;

               // When we encounter a candidate for a local variable, we
               // add it to the dictionary (but only once).
               if ((opndName = X86InsnLookupTable.OpndToName(operand)) != null)
               {
                  if (!_reachingDefs.ContainsKey(opndName))
                  {
                     System.Diagnostics.Debug.WriteLine(
                         String.Format("Found reaching definition of {0}<{1}>",
                        opndName, operand.DefinitionNumber));

                     Phx.Alias.Info aliasInfo = instruction.FunctionUnit.AliasInfo;
                     bool aliasOverlap = false;

                     foreach (AliasTag tag in killSet)
                     {
                        // We use the aliasing information to check whether 
                        // this definition kills all the operands.
                        if (aliasInfo.MayPartiallyOverlap(tag, operand.AliasTag))
                        {
                           aliasOverlap = true;
                           break;
                        }
                     }

                     if (!aliasOverlap)
                        _reachingDefs.Add(opndName, operand);
                  }
               }
            }
            if (instruction == block.FirstInstruction)
            {
               break;
            }
            instruction = instruction.Previous;
         }
      }
   }
}
