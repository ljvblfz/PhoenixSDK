//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//     EasyPlugin.cs: A C2 plugin performing a collection of basic actions, namely
//     display function names, count instructions, count operands, dump opcodes,
//     dump whole instructions.
//
// Usage: 
//
//    Launch RDK Console
//    cd samples\EasyPlugin\csharp\bin\debug
//    copy ..\..\..\..\..\Applications\Tests\src\cpp\Identity.cpp .
//    cl -d2 plugin:EasyPlugin.dll Identity.cpp
//    
//-----------------------------------------------------------------------------

using System;


//-----------------------------------------------------------------------------
//
// Description:
//
//    The plugin class.
//
// Remarks: 
//
//    Creates the phase that executes the plugin code.
// 
//-----------------------------------------------------------------------------

public class EasyPlugin : Phx.PlugIn {

   public override void RegisterObjects() { }

   public override void BuildPhases(Phx.Phases.PhaseConfiguration config) 
   {
      Phx.Phases.Phase after = config.PhaseList.FindByName("Canonicalize");
      Phx.Phases.Phase ph = MyPhase.New(config);
      after.InsertBefore(ph);
   }

   public override System.String NameString 
   { 
      get 
      { 
         return "EasyPlugin"; 
      } 
   }
}


//-----------------------------------------------------------------------------
//
// Description:
//
//    MyPhase gets inserted after Canonicalize and dumps information about
//    each function unit.
//
//-----------------------------------------------------------------------------

public class MyPhase : Phx.Phases.Phase {

   //-----------------------------------------------------------------------------
   //
   // Description:
   //
   //    Create a new phase.
   //    
   //-----------------------------------------------------------------------------


   public static Phx.Phases.Phase New(Phx.Phases.PhaseConfiguration config) 
   {
      MyPhase ph = new MyPhase();
      ph.Initialize(config, "EasyPlugin");
      return ph;
   }


   //-----------------------------------------------------------------------------
   //
   // Description:
   //
   //    Execute contains the code that is executed on each unit encountered
   //    by the compiler.
   //    
   //-----------------------------------------------------------------------------

   protected override void Execute(Phx.Unit unit) 
   {
      if (!unit.IsFunctionUnit) return;
      Phx.FunctionUnit f = unit.AsFunctionUnit;
      string fname = f.NameString;

      System.Console.WriteLine(">> Current function: {0}", fname);

      // Count instructions.

      int nInstructions = CountInstructions(f);
      System.Console.WriteLine(">> Function {0} has {1} instructions", fname, nInstructions);
    
      // Count operands.

      int nOperands = CountOperands(f);
      System.Console.WriteLine(">> Function {0} has {1} operands", fname, nOperands);
    
      // Dump all opcodes that occur in the current function unit.

      DumpOpcodes(f);
    
      // Dump the IR for the current function unit.

      DumpFunc(f);
   }

   private int CountInstructions(Phx.FunctionUnit f) 
   {
      Phx.IR.Instruction i = f.FirstInstruction;
      int n = 0;
      for ( ; i != null; i = i.Next) ++n;
      return n;
   }

   private int CountOperands(Phx.FunctionUnit f) 
   {
      Phx.IR.Instruction i = f.FirstInstruction;
      int n = 0;
      for ( ; i != null; i = i.Next) {
         Phx.IR.Operand o;
         for (o = i.SourceOperandList; o != null; o = o.Next) ++n;
         for (o = i.DestinationOperandList; o != null; o = o.Next) ++n;
      }
      return n;
   }

   private void DumpOpcodes(Phx.FunctionUnit f) 
   {
      Phx.IR.Instruction i = f.FirstInstruction;
      System.Console.WriteLine();
      System.Console.WriteLine("<<<<< {0} >>>>>", f.NameString);
      for (; i != null; i = i.Next) 
      {
         System.Console.Write("{0} ", i.OpcodeToString());
      }
      System.Console.WriteLine();
   }

   private void DumpFunc(Phx.FunctionUnit f) 
   {
      Phx.IR.Instruction i = f.FirstInstruction;
      System.Console.WriteLine();
      System.Console.WriteLine("<<<<< {0} >>>>>", f.NameString);
      for ( ; i != null; i = i.Next) 
      {
         System.Console.WriteLine("{0} ", i.ToString());
      }
      System.Console.WriteLine();
   }
}
