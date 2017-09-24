using System;
using System.Collections.Generic;
using System.Text;


namespace SliceAnalysis
{
   // Generic slicing strategy class. The parameters to the specific
   // slicing algorithm and the method to find the definition of the
   // (operand, line) we are slicing on depends the slicing strategy
   // which may be one of either assembly mode or source code.
   internal abstract class SliceStrategy
   {
      protected Phx.BitVector.Fixed _unreacheableBlocks;
      /// <summary>
      /// Returns a BitVector of unreacheable blocks indexed by their 
      /// block IDs.
      /// </summary>
      internal Phx.BitVector.Fixed UnreacheableBlocks
      { get { return _unreacheableBlocks; } }

      protected SliceAnalysis.FunctionUnit _funcUnit;

      // Abstraction for the source code and assembly mode versions or the 
      // slicing algorithm.
      protected abstract void TraverseDefInstr(SliceInfo sliceInfo);

      // Creates a new SliceStrategy object for the given functionUnit.
      protected SliceStrategy(SliceAnalysis.FunctionUnit functionUnit)
      {
         _funcUnit = functionUnit;
      }

      // Set up the node walker/visitor to walk the program dependence 
      // graph in depth first order.
      protected void WalkGraph(SliceInfo sliceInfo, Phx.IR.Instruction instruction)
      {
         if (instruction != null)
         {
            InstrNodeVisitor visitor =
                new InstrNodeVisitor(_funcUnit.NodeToInstr, sliceInfo);

            Phx.Graphs.NodeWalker walker =
                Phx.Graphs.NodeWalker.New(_funcUnit.PhxFuncUnit.Lifetime);
            walker.Graph = _funcUnit.ProgramDepGraph;
            walker.WalkDisconnectedNodes = false;
            walker.WalkFrom(visitor, _funcUnit.InstrToNode[instruction]);
         }
      }

      // Create a new SliceInfo object. Call the correct version of 
      // TraverseDefInstr based on whether it is a source slice or
      // an assembly slice. Prune the slice as required.
      internal SliceInfo Slice()
      {
         SliceInfo sliceInfo = new SliceInfo();

         TraverseDefInstr(sliceInfo);

         sliceInfo.SSAInstrFilter();
         sliceInfo.NoLineInstrFilter();
         sliceInfo.UnreacheableFilter(_unreacheableBlocks);
         return sliceInfo;
      }
   }

   // Source code slicing specific SliceStrategy. 
   internal class SourceSliceStrategy : SliceStrategy
   {
      // Source code line number used to calculate the slice.
      uint _lineNum;

      // Variable name with respect to which the slice is being calculated.
      string _varName;

      // Set up the line number and variable name for this SliceStrategy 
      // object.
      internal SourceSliceStrategy(SliceAnalysis.FunctionUnit functionUnit,
          SourceSliceSpec specification)
         : base(functionUnit)
      {
         functionUnit.CreateLineMappings();
         _lineNum = specification.VarLine;
         _varName = specification.VarName;


         if(!functionUnit.PhxFuncUnit.ParentPEModuleUnit.IsPureMsil) 
            PropagateConstants(specification.ExecOffset);         
      }

      // Used by PropagateConstants to determine whether it should annotate
      // an operand.
      private bool CanAnnotate(
         Phx.IR.Operand definitionOperand, Phx.IR.Operand useOperand)
      {
         Phx.Loops.Loop defLoop = definitionOperand.Instruction.BasicBlock.Loop;
         Phx.Loops.Loop useLoop = useOperand.Instruction.BasicBlock.Loop;

         // if the definition is not in a loop we can annotate every use
         // if the definition is in a loop, and the use is not, we can annotate
         if (defLoop == null || useLoop == null)
            return true;

         while (defLoop.ParentLoop != null)
            defLoop = defLoop.ParentLoop;
         while (useLoop.ParentLoop != null)
            useLoop = useLoop.ParentLoop;

         // if the definition and use belong to the same outermost loop
         // then don't annotate, otherwise annotate
         return !(defLoop == useLoop);
      }

      private void PropagateConstants(uint currentRVA)
      {
         Phx.FunctionUnit functionUnit = _funcUnit.PhxFuncUnit;
         Phx.Lifetime GLifetime = functionUnit.Lifetime;
         Phx.FunctionUnit GFuncunit = functionUnit;
         Phx.GlobalOptimizer.Info GInfo = Phx.GlobalOptimizer.Info.New(functionUnit, GLifetime);

         // Find the current instruction
         Phx.IR.Instruction currentInstruction = null;
         foreach (Phx.IR.Instruction instruction in
             Phx.IR.Instruction.Iterator(_funcUnit.PhxFuncUnit))
         {
            if (currentRVA - instruction.OriginalInstructionByteOffset >= 0)
            {
               currentInstruction = instruction;
            }
            else
            {
               break;
            }
         }

         // Rebuild a clean flow graph and discard any existing flow graph.
         // We need to do this because the flowgraph that we have already
         // constructed had been augmented with false edges for 
         // unconditional branches. This breaks the ordering between
         // predecessor edges and PHI nodes.
         functionUnit.BuildFlowGraphWithoutEH();
         functionUnit.BuildLoopInfo();

         System.Diagnostics.Debug.Assert(functionUnit.FlowGraph != null);
         System.Diagnostics.Debug.Assert(functionUnit.SsaInfo != null);

         BasicBlockVisitor visitor = new BasicBlockVisitor(currentInstruction);
         Phx.Graphs.NodeWalker walker =
             Phx.Graphs.NodeWalker.New(functionUnit.Lifetime);
         walker.Graph = functionUnit.FlowGraph;
         walker.WalkDisconnectedNodes = false;
         walker.ReverseWalkFrom(visitor, currentInstruction.BasicBlock);

         Phx.Lattices.ConstantLattice GConstLattice =
             Phx.Lattices.ConstantLattice.New(GFuncunit, GLifetime);

         foreach (Phx.IR.Operand operand in visitor.ReachingDefs.Values)
         {
            Phx.Constant.IntValue value = X86InsnLookupTable.EvaluateOpnd(operand);
            if (value != null)
            {
               
               // Annotate every use of this operand.
               for (Phx.IR.Operand useOp = operand.UseOperand; useOp != null;
                    useOp = useOp.UseOperand)
               {
                  if (CanAnnotate(operand, useOp))
                  {
                     Phx.Lattices.ConstantCell cell =
                        Phx.Lattices.ConstantCell.New(GConstLattice);
                     cell.Value = value;
                     GConstLattice.Attach(useOp, cell);
                  }
               }              
               
            }
         }

         // Set this property for the simulation framework to mark 
         // unreachable basic blocks.
         GConstLattice.DoUnreachableCode = true;

         // Set up a list of lattices which need to be applied during the 
         // simulation.
         Phx.Collections.LatticeList GLlist =
             Phx.Collections.LatticeList.New(GLifetime);
         GLlist.Append(GConstLattice);

         Phx.Simulators.RegionSimulator GRegSim =
             Phx.Simulators.RegionSimulator.New(GFuncunit, GLifetime);

         // Set up the set of basic blocks over which we want to run the 
         // GlobalOptimizer simulator. For now we're just setting up all the blocks
         // in the function to be run over the simulator. However, it
         // might be possible to run the simulator only over those blocks
         // in the static slice.

         GRegSim.SetupRegion(GFuncunit);

         GRegSim.Run(GLlist, Phx.Simulators.SimulatorDirection.Forward,
                     Phx.Simulators.SimulatorMode.Optimistic);

         // Build the bitvector to collect unreacheable block information.
         uint numBasicBlocks = 0;
         foreach (Phx.Graphs.BasicBlock B in GRegSim.BlockList)
         {
            numBasicBlocks++;
         }

         _unreacheableBlocks = Phx.BitVector.Fixed.New(functionUnit.Lifetime,
                                                       numBasicBlocks);
         foreach (Phx.Graphs.BasicBlock B in GRegSim.BlockList)
         {
            if (GRegSim.IsUnreachable(B))
            {
               _unreacheableBlocks.SetBit(B.Id);
               System.Diagnostics.Debug.WriteLine(
                   String.Format("Unreachable Block: {0}", B.ToString()));
            }
         }

         GLlist.Delete();
         GConstLattice.Delete();
         GRegSim.Delete();
      }

      // Find the operand corresponding to the variable and line number
      // in the IR and walk the perform a depth first traversal of the
      // program dependence graph.
      protected override void TraverseDefInstr(SliceInfo sliceInfo)
      {
         // Find the list of instructions associated with the given line number.
         List<Phx.IR.Instruction> instructions;
         if (_funcUnit.LineMap.ContainsKey(_lineNum))
         {
            instructions = _funcUnit.LineMap[_lineNum];
         }
         else
         {
            throw new NoMatchingLineException(_lineNum);
         }

         for (int i = instructions.Count - 1; i >= 0; i--)
         {
            Phx.IR.Instruction instruction = instructions[i];
            foreach (Phx.IR.Operand operand in instruction.DestinationAndSourceOperands)
            {
               if ((operand.Symbol != null && operand.Symbol.NameString.Equals(_varName)) ||
                   (operand.Field != null && operand.Field.FieldSymbol != null &&
                    operand.Field.FieldSymbol.NameString.Equals(_varName)))
               {
                  WalkGraph(sliceInfo, operand.DefinitionInstruction);
                  return;
               }
            }
         }
         throw new DefInstrNotFoundException(_varName);
      }
   }

   // Assembly mode slicing specific SliceStrategy.
   internal class AsmSliceStrategy : SliceStrategy
   {
      // The index of the operand in the assembly instruction in Visual 
      // Studio's dissasembly window, on which we are slicing.
      private uint _opndIndex;

      // The offset of the instruction from the start of the function on 
      // which we are slicing.
      private uint _instrOffset;

      // Set up the operand index and instruction offset for this SliceStrategy 
      // object.
      internal AsmSliceStrategy(SliceAnalysis.FunctionUnit functionUnit,
          AsmSliceSpec specification)
         : base(functionUnit)
      {
         _opndIndex = specification.OpndIndex;
         _instrOffset = specification.InstrOffset;
      }

      // Find the operand corresponding to the operand index and 
      // instruction offset in the IR and walk the perform a depth 
      // first traversal of the program dependence graph.
      protected override void TraverseDefInstr(SliceInfo sliceInfo)
      {
         foreach (Phx.IR.Instruction instruction in
                  _funcUnit.PhxFuncUnit.Instructions)
         {
            if (instruction.OriginalInstructionByteOffset == _instrOffset && instruction.IsReal)
            {
               // Map the operand index in Visual Studio's representation 
               // of the X86 instruction to an operand in Phoenix's IR.
               Phx.IR.Operand operand = X86InsnLookupTable.Lookup(_opndIndex, instruction);

               if (operand != null)
               {
                  // We need to handle MemOpnds specially because these 
                  // operands do not have a single definition but have
                  // one definition for each of it's constituent 
                  // operands. So we need to traverse the PDG from each
                  // of the operands.
                  if (operand.IsMemoryOperand)
                  {
                     for (Phx.IR.Operand memoryOperand = operand;
                          memoryOperand != null;
                          memoryOperand = memoryOperand.Next)
                     {
                        WalkGraph(sliceInfo, memoryOperand.DefinitionInstruction);
                     }
                  }
                  else
                  {
                     WalkGraph(sliceInfo, operand.DefinitionInstruction);
                  }
                  return;
               }
            }
         }
         throw new DefInstrNotFoundException(string.Format("OpndIndex {0}",
            _opndIndex));
      }
   }
}
