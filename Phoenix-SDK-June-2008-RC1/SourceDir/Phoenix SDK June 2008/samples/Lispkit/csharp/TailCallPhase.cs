using System.Diagnostics;

namespace Lispkit
{
   /// <summary>
   /// Extension object that is used to tag tail calls.
   /// </summary>
   public class TailCallExtensionObject : Phx.IR.InstructionExtensionObject
   {
      // Retrieves the TailCallExtensionObject object that is associated with
      // the given Instruction object.
      public static TailCallExtensionObject Get(Phx.IR.Instruction instruction)
      {
         return instruction.FindExtensionObject(
            typeof(TailCallExtensionObject)) as TailCallExtensionObject;
      }
   }

   /// <summary>
   /// Phase that is run before lowering to mark tail calls.
   /// </summary>
   public class MarkTailCallPhase : Phx.Phases.Phase
   {
      // Creates a new instance of the MarkTailCallPhase class.
      public static MarkTailCallPhase New(Phx.Phases.PhaseConfiguration config)
      {
         MarkTailCallPhase phase = new MarkTailCallPhase();         
         phase.Initialize(config, "Mark tail calls");
         return phase;
      }

      // Executes the phase on the provided Unit object.
      protected override void Execute(Phx.Unit unit)
      {
         // Only process function units.
         if (!unit.IsFunctionUnit)
         {
            return;
         }

         Phx.FunctionUnit functionUnit = unit.AsFunctionUnit;
         
         // Build the flow graph for the function unit if it was not
         // already built by the previous phase.
         bool builtFlowGraph = false;
         Phx.Graphs.FlowGraph flowGraph = functionUnit.FlowGraph;
         if (flowGraph == null)
         {
            functionUnit.BuildFlowGraph();
            flowGraph = functionUnit.FlowGraph;
            builtFlowGraph = true;            
         }

         // Find the basic block that ends with a return instruction.
         Phx.Graphs.BasicBlock returnBlock = 
            this.FindReturnBlock(flowGraph.LastBlock);
         if (returnBlock != null)
         {
            Phx.IR.Instruction returnInstruction = returnBlock.LastInstruction;
            Debug.Assert(returnInstruction.IsReturn);

            // Search for a tail call before the return instruction. The search
            // consists of two options:
            // 1. If the function does not end with a branch, the tail call 
            //    exists directly before the return instruction.
            // 2. If the function does end with a branch, the tail call 
            //    can be found from a sequence of branch statements from the 
            //    call site to the return instruction.
            // In either case, we also verify that the tail is a call
            // to the same function. In other words, we do not consider 
            // mutually-recursive functions.

            Phx.IR.Instruction instruction;

            // Case 1: the tail call exists directly before the return 
            // instruction.
            instruction = returnBlock.LastInstruction.Previous;
            if (this.IsTailCall(instruction))
            {
               // Mark the instruction as a tail call.
               this.MarkInstruction(instruction);
            }
            // Case 2: the tail call exists directly before a branch
            // to the return instruction.
            else
            {
               instruction = this.SearchForTailCall(returnBlock);
               if (instruction != null)
               {
                  // Mark the instruction as a tail call.
                  this.MarkInstruction(instruction);
               }
            }
         }

         // Delete the flow graph if we built it.
         if (builtFlowGraph)
         {
            functionUnit.DeleteFlowGraph();            
         }
      }

      private Phx.IR.Instruction SearchForTailCall(
         Phx.Graphs.BasicBlock basicBlock)
      {
         // A basic block that ends with a tail call has the following 
         // characteristics:
         // 1. If the function does not end with a branch, we 
         //    see the following instruction sequence:
         //     CALL    - call the tail function.
         //     ASSIGN  - assign the result to a temporary.
         //     GOTO    - jump to the return block.
         // 2. If the function ends with a branch, we check
         //    predecessor blocks for the CALL/ASSIGN/GOTO sequence.

         // Case 1: Look for CALL/ASSIGN/GOTO sequence.

         Phx.IR.Instruction instruction = basicBlock.LastInstruction;
         // GOTO
         if (instruction is Phx.IR.BranchInstruction) 
         {
            instruction = instruction.Previous;

            // ASSIGN
            if (instruction is Phx.IR.ValueInstruction &&
                instruction.Opcode == Phx.Common.Opcode.Assign) 
            {
               instruction = instruction.Previous;

               // CALL
               if (this.IsTailCall(instruction)) 
               {
                  return instruction;
               }
            }
         }

         // Case 2: Check predecessor blocks for the CALL/ASSIGN/GOTO sequence.

         for (uint i = 0; i < basicBlock.PredecessorCount; ++i)
         {
            instruction = this.SearchForTailCall(
               basicBlock.NthPredecessorNode(i) as Phx.Graphs.BasicBlock);
            if (instruction != null)
            {
               return instruction;
            }
         }

         // A tail call is not present in this block.

         return null;
      }

      // Marks the given instruction as a tail call.
      private void MarkInstruction(Phx.IR.Instruction instruction)
      {
         instruction.AddExtensionObject(new TailCallExtensionObject());
      }

      // Determines whether the given instruction is a tail call.
      private bool IsTailCall(Phx.IR.Instruction instruction)
      {
         // Test whether the instruction is a CallInstruction object.
         Phx.IR.CallInstruction callInstruction = 
            instruction as Phx.IR.CallInstruction;
         if (callInstruction != null)
         {
            // Determine whether the source and target of the call refer
            // to the same function symbol.

            Phx.Symbols.FunctionSymbol callerFunctionSymbol =
               callInstruction.FunctionUnit.FunctionSymbol;
            Phx.Symbols.FunctionSymbol calleeFunctionSymbol = 
               callInstruction.FunctionSymbol;

            return (callerFunctionSymbol.Equals(calleeFunctionSymbol));
         }
         return false;
      }

      // Searches for the basic block that ends with a return instruction.
      private Phx.Graphs.BasicBlock FindReturnBlock(
         Phx.Graphs.BasicBlock basicBlock)
      {
         // Base case: the last instruction of the basic block 
         // is a return instruction.
         if (basicBlock != null && basicBlock.LastInstruction.IsReturn)
         {
            return basicBlock;
         }

         // Recursive case: search each predecessor block for 
         // the return instruction.
         for (uint i = 0; i < basicBlock.PredecessorCount; ++i)
         {
            Phx.Graphs.BasicBlock returnBlock = this.FindReturnBlock(
               basicBlock.NthPredecessorNode(i) as Phx.Graphs.BasicBlock);
            if (returnBlock != null)
            {
               return returnBlock;
            }
         }

         return null;
      }
   }

   /// <summary>
   /// Phase that is run after lowering to apply tail instructions.
   /// </summary>
   public class ApplyTailCallPhase : Phx.Phases.Phase
   {
      // Creates a new instance of the ApplyTailCallPhase class.
      public static ApplyTailCallPhase New(
         Phx.Phases.PhaseConfiguration config)
      {
         ApplyTailCallPhase phase = new ApplyTailCallPhase();
         phase.Initialize(config, "Apply tail instructions");
         return phase;
      }

      // Executes the phase on the provided Unit object.
      protected override void Execute(Phx.Unit unit)
      {
         // Only process function units.
         if (!unit.IsFunctionUnit)
         {
            return;
         }

         // Seach the function unit for call instructions that are marked
         // as tail calls.
         Phx.FunctionUnit functionUnit = unit.AsFunctionUnit;
         foreach (Phx.IR.Instruction instruction in functionUnit.Instructions)
         {            
            if (instruction is Phx.IR.CallInstruction)
            {
               TailCallExtensionObject extObj =
                  TailCallExtensionObject.Get(instruction);
               if (extObj != null)
               {
                  // We found a call instruction that was marked as a tail 
                  // call. Insert an MSIL tail. instruction directy before 
                  // the call instruction. This informs the runtime to discard 
                  // the stack frame for the current method, which allows us to
                  // perform recursive routines without overflowing the stack.

                  Phx.IR.Instruction tailInstruction =
                     Phx.IR.ValueInstruction.New(functionUnit,
                     Phx.Targets.Architectures.Msil.Opcode.TAILPREFIX);

                  instruction.InsertBefore(tailInstruction);
                  // The runtime requires tail. instructions to be followed 
                  // by a call instruction and then a ret instruction. Often,
                  // however, the result of the call instruction is assigned
                  // to another variable before the ret instruction.
                  // Because the recursive case of a function does not return 
                  // the result of the algorithm, we can safely create a copy 
                  // of the ret instruction directly after the call 
                  // instruction.

                  Phx.IR.Instruction returnInstruction = instruction.Next;
                  while (!returnInstruction.IsReturn)
                  {
                     returnInstruction = returnInstruction.Next;
                  }
                  instruction.InsertAfter(returnInstruction.Copy());

                  // Unmark the instruction as being a tail call.
                  instruction.RemoveExtensionObject(extObj);
               }
            }
         }
      }
   }
}
