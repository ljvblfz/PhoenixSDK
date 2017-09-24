//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Phoenix Plug-in sample.
//
// Remarks:
//
//    Registers "addnops" component control.
//
//    Injects a phase after Lower which injects a NOP prior to every
//    instruction.
//
// Usage:
//
//    cl ... -d2plugin:addnop-plug-in.dll
//
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//
// Description:
//
//    Class designed to interface our phase with a Phoenix
//    based compiler.
//
// Remarks:
//
//    To interface with a Phoenix based compiler, a DLL contains a class
//    derived from Phx.PlugIn. This class must implement two methods:
//    RegisterObjects and BuildPhases. RegisterObjects is called when the
//    plug-in is loaded and is responsible for registering any component
//    controls the plug-in might need. BuildPhases is responsible for
//    modifying the phase list of the compiler, perhaps including a new phase
//    or replacing an existing one.
//
//------------------------------------------------------------------------------

public class 
AddNOPPlugIn : Phx.PlugIn
{
   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Registers any controls needed by the plug-in.
   //
   //---------------------------------------------------------------------------

   public override void
   RegisterObjects()
   {

// ISSUE-WORKAROUND-MattMoor-2008/06/10
// See bug 447129 for details.
//#if (PHX_DEBUG_SUPPORT)

      AddNOPPhase.AddNOPCtrl = Phx.Controls.ComponentControl.New("addnops",
         "Inject a NOP before each instruction; called after Lower",
         "addnop-plug-in.cs");

// ISSUE-WORKAROUND-MattMoor-2008/06/10
// See bug 447129 for details.
//#endif

   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Inspects and modifies the phase listing of the host (i.e. a Phoenix
   //    based compiler).
   //
   // Arguments:
   //
   //    config - Encapsulating object that simplifies handling of
   //    the phase list and pre and post phase events.
   //
   //---------------------------------------------------------------------------

   public override void
   BuildPhases
   (
      Phx.Phases.PhaseConfiguration config
   )
   {
      Phx.Phases.Phase readerPhase;

      readerPhase = config.PhaseList.FindByName("Lower");
      if (readerPhase == null)
      {

         Phx.Output.WriteLine("Lower phase not found in phaselist:");
         Phx.Output.Write(config.ToString());

         return;
      }

      Phx.Phases.Phase phase = AddNOPPhase.New(config);
      readerPhase.InsertAfter(phase);
   }
   
   public override System.String NameString
   {
       get
       {
           return "Add NOP";
       }
   }
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Phase that injects nop's before every instruction.
//
// Remarks:
//
//    Each phase must implement two methods: New and Execute.  New, a
//    static method, should initialize any phase controls and return a new
//    instance of the phase.  Execute is responsible for performing the
//    actual work of the phase.
//
//------------------------------------------------------------------------------

public class 
AddNOPPhase : Phx.Phases.Phase
{
   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Create a new AddNOPPhase object.
   //
   // Arguments:
   //
   //    config - The encapsulating object that simplifies handling of
   //    the phase list and pre and post phase events.
   //
   // Returns:
   //
   //    A new AddNOPPhase object.
   //
   //---------------------------------------------------------------------------

   public static AddNOPPhase
   New
   (
      Phx.Phases.PhaseConfiguration config
   )
   {
      AddNOPPhase phase = new AddNOPPhase();

      phase.Initialize(config, "Add NOPs");

// ISSUE-WORKAROUND-MattMoor-2008/06/10
// See bug 447129 for details.
//#if (PHX_DEBUG_SUPPORT)

      phase.PhaseControl = AddNOPPhase.AddNOPCtrl;

// ISSUE-WORKAROUND-MattMoor-2008/06/10
// See bug 447129 for details.
//#endif

      return phase;
   }

   //---------------------------------------------------------------------------
   // 
   // Description:
   //
   //    Execute the phase, inserting nop's before every instruction in
   //    each function unit.
   //
   // Arguments:
   //
   //    unit - The unit this phase is operating on.
   //
   //---------------------------------------------------------------------------

   protected override void
   Execute
   (
      Phx.Unit unit
   )
   {
      int CountNops = 0;

      if (!unit.IsFunctionUnit)
      {
         return;
      }

      Phx.FunctionUnit functionUnit = unit.AsFunctionUnit;

      Phx.IR.Instruction nopInstruction;

      foreach (Phx.IR.Instruction instruction in functionUnit.Instructions)
      {
         if (instruction.Opcode == Phx.Common.Opcode.ExitBody)
         {
            // Quit when we get to ExitBody pseudo inst;
            // otherwise we'll mess up the Unwind code.

            break;
         }

         if (!instruction.IsReal)
         {
            continue;
         }

         // Create an architecture-neutral (Hir) nop, then lower to
         // make it architecture specific (LIR). Note that targets
         // that do not override the default lowering will actually
         // end up removing the Hir nop in lower instead of producing
         // the target-specific LIR equivalent.

         instruction.SetCurrentDebugTag();
         nopInstruction = Phx.IR.ValueInstruction.New(functionUnit, Phx.Common.Opcode.Nop);
         instruction.InsertBefore(nopInstruction);
         CountNops++;
         functionUnit.Lower.Instruction(nopInstruction);
      }
      Phx.Output.WriteLine("AddNop added {0} nops to {1}",
         CountNops, functionUnit.NameString);
   }

   //---------------------------------------------------------------------------
   // 
   // Description:
   //
   //    Component control providing standard Phoenix controls for this phase.
   //
   //---------------------------------------------------------------------------

// ISSUE-WORKAROUND-MattMoor-2008/06/10
// See bug 447129 for details.
//#if (PHX_DEBUG_SUPPORT)

   public static Phx.Controls.ComponentControl AddNOPCtrl;

// ISSUE-WORKAROUND-MattMoor-2008/06/10
// See bug 447129 for details.
//#endif

}
