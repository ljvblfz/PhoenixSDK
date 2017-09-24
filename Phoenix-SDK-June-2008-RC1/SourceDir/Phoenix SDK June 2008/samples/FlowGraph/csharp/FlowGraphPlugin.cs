//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    The PlugIn and Phase classes that describe how FlowChart is to be invoked
//    and behave.
//
//-----------------------------------------------------------------------------


namespace FlowGraph 
{
   //-----------------------------------------------------------------------------
   //
   // Description:
   //
   //    The FlowGraph PlugIn class
   //
   // Remarks:
   //
   //    A Phoenix plugin to build the Flowgraph of a FunctionUnit.
   //
   //-----------------------------------------------------------------------------

   public class PlugIn : Phx.PlugIn 
   {

      //--------------------------------------------------------------------------
      //
      // Description:
      //
      //    The RegisterObjects method is typically used to register any 
      //    controls for the current plug-in (such controls are used to parse
      //    the command-line supplied to the host, extracting information on
      //    behalf of the plug-in).  However, the current plug-in has no
      //    controls, so we supply an empty body for this method.
      //
      //--------------------------------------------------------------------------

      public override void RegisterObjects() { }

      //--------------------------------------------------------------------------
      //
      // Description:
      //
      //    The BuildPhases method locates the final IR-transformation phase in the host's
      //    phase list.  For the case of C2, this is called "Canonicalize".  It then calls
      //    Phase.New to create a new FlowGraph.Phase object and insert before
      //    the canonicalization phase.
      //
      //--------------------------------------------------------------------------

      public override void BuildPhases(Phx.Phases.PhaseConfiguration config) 
      {
         Phx.Phases.Phase after = config.PhaseList.FindByName("Canonicalize");
         Phx.Phases.Phase ph = Phase.New(config);
         after.InsertBefore(ph);
      }

      public override System.String NameString 
      { 
         get 
         { 
            return "FlowGraphPlugin"; 
         } 
      }
   }

   //-----------------------------------------------------------------------------
   //
   // Description:
   //
   //    The FlowGraph Phase class
   //
   // Remarks:
   //
   //    This phase supplies the implementation for two methods (defined by
   //    its Phase base class): New and Execute
   //
   //-----------------------------------------------------------------------------

   public class Phase : Phx.Phases.Phase 
   {

      //--------------------------------------------------------------------------
      //
      // Description:
      //
      //    The New method creates a new FuncNamesPhase object and inserts 
      //    it into the host's phase list, immediately before the existing
      //    phase 'Canonicalize'.
      //
      //--------------------------------------------------------------------------

      public static Phx.Phases.Phase New(Phx.Phases.PhaseConfiguration config) 
      {
         Phase phase = new Phase();
         phase.Initialize(config, "FlowGraphPlugin");
         return phase;
      }

      //--------------------------------------------------------------------------
      //
      // Description:
      //
      //    The Execute method is the phase's workhorse. It gets called by
      //    the host to process each successive unit.
      //
      //--------------------------------------------------------------------------

      protected override void Execute(Phx.Unit unit) 
      {
         // System.Diagnostics.Debugger.Break();

         if (!unit.IsFunctionUnit) return;

         Phx.FunctionUnit f = unit.AsFunctionUnit;
         System.Console.WriteLine("<<<<< {0} >>>>>", f.NameString);

         //DumpFunc(f);       // comment-out to see more diagnostic info

         // Construct FlowGraph and display it.

         FlowGraph.Graph graph = new FlowGraph.Graph(f);
         graph.DumpBlocks();
         graph.DumpEdges();
      }

      //-----------------------------------------------------------------------------
      //
      // Description:
      //
      //    Dump IR for the function unit.
      //    
      //-----------------------------------------------------------------------------

      private void DumpFunc(Phx.FunctionUnit f)
      {
         Phx.IR.Instruction instruction = f.FirstInstruction;
         uint n = 0;
         while (instruction != null) 
         {
            System.Console.Write(n++);
            DumpInstruction(instruction);
            instruction = instruction.Next; 
         }
      }

      //-----------------------------------------------------------------------------
      //
      // Description:
      //
      //    Dump an IR instruction.
      //    
      //-----------------------------------------------------------------------------

      public void DumpInstruction(Phx.IR.Instruction instruction) 
      {
         System.Console.Write(" {0}", instruction.Opcode.ToString());

         Phx.IR.Operand operand = instruction.DestinationOperand; int n = 1;    
         while (operand != null) 
         {
            System.Console.Write(" {0}<<{1}", n, operand.ToString());
            operand = operand.Next;
            n++; 
         }

         operand = instruction.SourceOperand; n = 1;
         while (operand != null) 
         {
            System.Console.Write(" {0}>>{1}", n, operand.ToString());
            operand = operand.Next;
            n++;
         }

         System.Console.WriteLine();
      }

   }
}
