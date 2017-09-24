//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    IR Longevity plug-in.
//
// Remarks:
//
//    This sample demonstrates the following concepts:
//
//    . Plug-Ins
//    . Phases
//    . Extension objects
//    . Event delegates
//
//    The longevity of IR objects is tracked during compilation. We take note
//    of the birth and death phase for each object. The count of birth-death
//    pairs is printed on the console when the process terminates.
//
//    We keep separate instruction and operand counts, but the distinct kinds
//    within those hierarchies are conglomerated.
//
// Usage:
//
// csc -r:phx.dll -t:library ir-longevity.cs
//
// cl ... -d2plugin:ir-longevity-plug-in.dll [-d2IRLongevityOutput:<file name>]
//    
//    Arguments in [] are optional.
//        IRLongevityOutput:<file name> -
//             Dumps the output from this sample into a file provided in
//             the command line. Otherwise, IR-Longevity.txt output file 
//             will be created by default.
//             
//             Note: if you invoke c2.exe via cl and specify multiple source
//             files on the command line, the IR-Longevity output will only
//             show the results for one of the files.
//
//-----------------------------------------------------------------------------

using System;
using System.IO;

namespace IRLongevity
{

//-----------------------------------------------------------------------------
//
// Description:
//
//    InstrBirthExtensionObject is attached to IR instructions to indicate the
//    phase in which they were constructed.
//
//-----------------------------------------------------------------------------

public class InstrBirthExtensionObject : Phx.IR.InstructionExtensionObject
{
   public Phx.Phases.Phase BirthPhase;

   // Boilerplate code

   static public InstrBirthExtensionObject
   Get
   (
      Phx.IR.Instruction instruction
   )
   {
      return instruction.FindExtensionObject(typeof(InstrBirthExtensionObject))
         as InstrBirthExtensionObject;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    OpndBirthExtensionObject is attached to IR operands to indicate the
//    phase in which they were constructed.
//
//-----------------------------------------------------------------------------

public class OpndBirthExtensionObject : Phx.IR.OperandExtensionObject
{
   public Phx.Phases.Phase BirthPhase;

   // Boilerplate code

   static public OpndBirthExtensionObject
   Get
   (
      Phx.IR.Operand operand
   )
   {
      return operand.FindExtensionObject(typeof(OpndBirthExtensionObject))
         as OpndBirthExtensionObject;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    SweepPhase walks the IR to "kill" all instructions and operands.
//
// Remarks:
//
//    Because of the way Phoenix does memory management, the IR existing at
//    the end of function phase processing is not deleted. This phase is
//    placed after encoding as the final resting place for the IR.
//
//    This code is packaged as a phase for demonstration purposes. To be more
//    accurate and flexible, we'd use the DeleteUnitEvent. Doing that would
//    require a minor change to the accounting/output code since we'd no
//    longer have a phase to hang our hat on.
//
//-----------------------------------------------------------------------------

public class SweepPhase : Phx.Phases.Phase
{
#if (PHX_DEBUG_SUPPORT)
   static public Phx.Controls.ComponentControl DebugControl;
#endif

   private PlugIn plugIn;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Static constructor for SweepPhase.
   //
   // Arguments:
   //
   //    config - A pointer to a Phx.Phases.PhaseConfiguration that provides
   //       properties for retrieving the initial phase list.
   //    plugIn - A pointer to an IRLongevity.PlugIn.
   //
   // Returns:
   //
   //    A pointer to the new phase.
   //
   //--------------------------------------------------------------------------

   public static SweepPhase
   New
   (
      Phx.Phases.PhaseConfiguration config,
      IRLongevity.PlugIn     plugIn
   )
   {
      SweepPhase phase = new SweepPhase();

      phase.plugIn = plugIn;

      phase.Initialize(config, "IR Longevity Sweep");

#if (PHX_DEBUG_SUPPORT)
      phase.PhaseControl = SweepPhase.DebugControl;
#endif

      return phase;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Execute the SweepPhase.
   //
   // Arguments:
   //
   //    unit - The Phx.Unit to work on.
   //
   // Remarks:
   //
   //    We walk the IR instructions and operands invoking the plug-in's
   //    DeleteOperandEventHandler. This simulates the IR being deleted at the
   //    end of compilation.
   //
   //    Having "killed" all the IR, we tell the plug-in to remove its event
   //    handlers. This is to protect against some other phase creating or
   //    deleting IR after us.
   //
   //--------------------------------------------------------------------------

   protected override void
   Execute
   (
      Phx.Unit unit
   )
   {
      if (!unit.IsFunctionUnit)
      {
         return;
      }

      Phx.FunctionUnit functionUnit = unit.AsFunctionUnit;

      Phx.IR.Instruction instruction;
      Phx.IR.Operand  operand;

      for (instruction = functionUnit.FirstInstruction; instruction != null; instruction = instruction.Next)
      {
         for (operand = instruction.DestinationOperand; operand != null; operand = operand.Next)
         {
            this.plugIn.DeleteOperandEventHandler(operand);
         }

         for (operand = instruction.SourceOperand; operand != null; operand = operand.Next)
         {
            this.plugIn.DeleteOperandEventHandler(operand);
         }

         this.plugIn.DeleteInstructionEventHandler(instruction);
      }

      this.plugIn.RemoveIREventHandlers(functionUnit);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Phase visitor to collect all the phases in an array list.
//
// Remarks:
//
//    We ignore phase lists.
//
//-----------------------------------------------------------------------------

public class PhaseVisitor : Phx.Phases.PhaseVisitor
{
   public System.Collections.ArrayList arrayList;

   public override void
   Visit
   (
      Phx.Phases.Phase phase
   )
   {
      arrayList.Add(phase);
   }
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    The IR longevity plug-in.
//
// Remarks:
//
//    The longevity of IR objects is tracked during compilation. We take note
//    of the birth and death phase for each object. The count of birth-death
//    pairs is printed on the console when the process terminates.
//
//    We keep separate instruction and operand counts, but the distinct kinds
//    within those hierarchies are conglomerated.
//
//    The output will be redirected into a file provided in the command line.
//    Otherwise, IR-Longevity.txt will be created by default.
//    
//-----------------------------------------------------------------------------

public class PlugIn : Phx.PlugIn
{
   private Phx.DependencyObject TermEventDependencyObject;
   private Phx.DependencyObject NewUnitDependencyObject;
   private Phx.DependencyObject NewInstrDependencyObject;
   private Phx.DependencyObject DeleteInstrDependencyObject;
   private Phx.DependencyObject NewOpndDependencyObject;
   private Phx.DependencyObject DeleteOperandDependencyObject;

   private System.Collections.ArrayList PhaseArrayList;
   private uint [,] InstrLongevityArray;
   private uint [,] OpndLongevityArray;
    
    //  This control should allow the user to provide the output file.
    
    private Phx.Controls.StringControl LongevityOuputCtrl;
    private System.String        OutputFile;
    private bool FileOpened = false;


   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Register objects used by the IR longevity plug-in.
   //
   //    A string control, called IRLongevityOutput, is created here, and used
   //    in PlugIn.BuildPhases, to specify the output file.
   //    The output will be dumped into a file provided in the command line.
   //    Otherwise, IR-Longevity.txt will be created by default.
   //
   //--------------------------------------------------------------------------

   public override void
   RegisterObjects()
   {
       System.String defaultFile = "IR-Longevity.txt";
       System.String descriptor;
       
       descriptor = String.Format(
           "IR-Longevity plug-in will write its output into specified file." + 
           "Default file is {0}", defaultFile);

      Phx.Initialize.EndInitializationEvent.Insert(this.EndInitialization);

      TermEventDependencyObject =
         Phx.DependencyObject.New(ref Phx.Term.TermEventDependencyList,
            "IRLongevity", "*");

      NewUnitDependencyObject =
         Phx.DependencyObject.New(ref Phx.Unit.NewUnitEventDependencyList,
            "IRLongevity", "*");

      NewInstrDependencyObject =
         Phx.DependencyObject.New(ref Phx.Unit.NewInstructionEventDependencyList,
            "IRLongevity", "*");

      DeleteInstrDependencyObject =
         Phx.DependencyObject.New(ref Phx.Unit.DeleteInstructionEventDependencyList,
            "IRLongevity", "*");

      NewOpndDependencyObject =
         Phx.DependencyObject.New(ref Phx.Unit.NewOperandEventDependencyList,
            "IRLongevity", "*");

      DeleteOperandDependencyObject =
         Phx.DependencyObject.New(ref Phx.Unit.DeleteOperandEventDependencyList,
            "IRLongevity", "*");

#if (PHX_DEBUG_SUPPORT)

      SweepPhase.DebugControl = Phx.Controls.ComponentControl.New("irlongevitysweep",
         "Debug the IR longevity sweep phase",
         "ir-longevity.cs");

#endif
       
       LongevityOuputCtrl = Phx.Controls.StringControl.New("IRLongevityOutput",
           defaultFile,
           descriptor,
           "ir-longevity.cs");
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Add handlers for dependent static events.
   //
   //--------------------------------------------------------------------------

   private void EndInitialization()
   {
      Phx.Term.TermEvent.Insert(this.TermEventHandler,
         this.TermEventDependencyObject);
      Phx.Unit.NewUnitEvent.Insert(this.NewUnitEventHandler,
         this.NewUnitDependencyObject);
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Add the SweepPhase after the Encoding phase.
   //
   // Arguments:
   //
   //    config - A pointer to a Phx::Phases::PhaseConfiguration that will own the
   //       phases.
   //
   // Remarks:
   //
   //    The Phoenix framework has parsed the command line, so control values
   //    are available at this point.
   //
   //--------------------------------------------------------------------------

   public override void
   BuildPhases
   (
      Phx.Phases.PhaseConfiguration config
   )
   {
      // Retrieve the user's desired file to dump the output.

      this.OutputFile = this.LongevityOuputCtrl.GetValue(null);

      Phx.Phases.Phase encodingPhase;
      SweepPhase       sweepPhase = SweepPhase.New(config, this);

      encodingPhase = config.PhaseList.FindByName("Encoding");
      if (encodingPhase != null)
      {
         encodingPhase.InsertAfter(sweepPhase);
      }
      else
      {
         Phx.Output.WriteLine("Encoding phase not found in phaselist:");
         Phx.Output.Write(config.ToString());
         Phx.Output.WriteLine("Appending.");

         // Didn't find the encoding phase; simply append to the phaselist.

         config.PhaseList.AppendPhase(sweepPhase);
      }

      // Assume the phase list is now complete and walk it to compute the
      // number of phases.

      PhaseVisitor visitor = new PhaseVisitor();
      visitor.arrayList = new System.Collections.ArrayList(30);
      visitor.WalkPhases(config.PhaseList);
      this.PhaseArrayList =
         System.Collections.ArrayList.ReadOnly(visitor.arrayList);

      int numPhases = this.PhaseArrayList.Count;
      this.InstrLongevityArray = new uint[numPhases,numPhases];
      this.OpndLongevityArray = new uint[numPhases,numPhases];
   }

   public override System.String NameString
   {
      get
      {
         return "IR Longevity";
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Handler for the NewUnitEvent.
   //
   // Arguments:
   //
   //    unit - The unit that was just constructed.
   //
   // Remarks:
   //
   //    We only care about FuncUnits. When we see one, we add our IR event
   //    handlers to the FunctionUnit's event lists.
   //
   //--------------------------------------------------------------------------

   private void
   NewUnitEventHandler
   (
      Phx.Unit unit
   )
   {
      if (!unit.IsFunctionUnit)
      {
         return;
      }

      Phx.FunctionUnit functionUnit = unit.AsFunctionUnit;

      // Add our IR event handlers.

      functionUnit.NewInstructionEvent.Insert(this.NewInstructionEventHandler,
         this.NewInstrDependencyObject);
      functionUnit.DeleteInstructionEvent.Insert(this.DeleteInstructionEventHandler,
         this.DeleteInstrDependencyObject);

      functionUnit.NewOperandEvent.Insert(this.NewOperandEventHandler,
         this.NewOpndDependencyObject);
      functionUnit.DeleteOperandEvent.Insert(this.DeleteOperandEventHandler,
         this.DeleteOperandDependencyObject);
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Remove our IR event handlers from a FunctionUnit's event lists.
   //
   // Arguments:
   //
   //    unit - The unit from which to remove our events handlers.
   //
   //--------------------------------------------------------------------------

   public void
   RemoveIREventHandlers
   (
      Phx.FunctionUnit functionUnit
   )
   {
      // Remove our IR event handlers.

      functionUnit.NewInstructionEvent.Remove(this.NewInstructionEventHandler);
      functionUnit.DeleteInstructionEvent.Remove(this.DeleteInstructionEventHandler);

      functionUnit.NewOperandEvent.Remove(this.NewOperandEventHandler);
      functionUnit.DeleteOperandEvent.Remove(this.DeleteOperandEventHandler);
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Handler for the NewInstructionEvent.
   //
   // Arguments:
   //
   //    instruction - The instruction being constructed.
   //
   // Remarks:
   //
   //    We create an InstrBirthExtensionObject to annotate the
   //    instruction. It will contain the phase currently executing, which
   //    indicates when the instruction was born.
   //
   //--------------------------------------------------------------------------

   private void
   NewInstructionEventHandler
   (
      Phx.IR.Instruction instruction
   )
   {
      InstrBirthExtensionObject extObj = new InstrBirthExtensionObject();
      extObj.BirthPhase = instruction.FunctionUnit.Phase;
      instruction.AddExtensionObject(extObj);
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Handler for the DeleteInstructionEvent.
   //
   // Arguments:
   //
   //    instruction - The instruction being deleted.
   //
   // Remarks:
   //
   //    We retrieve the instruction's birth phase from the attached
   //    InstrBirthExtensionObject. The current phase is noted as the death
   //    phase of the instruction. The InstrBirthExtensionObject is destroyed
   //    in the process, having served its purpose.
   //
   //--------------------------------------------------------------------------

   public void
   DeleteInstructionEventHandler
   (
      Phx.IR.Instruction instruction
   )
   {
      InstrBirthExtensionObject extObj = InstrBirthExtensionObject.Get(instruction);
      this.NoteLifetime(this.InstrLongevityArray, extObj.BirthPhase,
         instruction.FunctionUnit.Phase);
      instruction.RemoveExtensionObject(extObj);
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Handler for the NewOperandEvent.
   //
   // Arguments:
   //
   //    operand - The operand being constructed.
   //
   // Remarks:
   //
   //    We create a OpndBirthExtensionObject to annotate the operand. It will
   //    contain the phase currently executing, which indicates when the
   //    operand was born.
   //
   //--------------------------------------------------------------------------

   private void
   NewOperandEventHandler
   (
      Phx.IR.Operand operand
   )
   {
      OpndBirthExtensionObject extObj = new OpndBirthExtensionObject();
      extObj.BirthPhase = operand.FunctionUnit.Phase;
      operand.AddExtensionObject(extObj);
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Handler for the DeleteOperandEvent.
   //
   // Arguments:
   //
   //    operand - The operand being deleted.
   //
   // Remarks:
   //
   //    We retrieve the operand's birth phase from the attached
   //    OpndBirthExtensionObject. The current phase is noted as the death
   //    phase of the operand. The InstrBirthExtensionObject is destroyed in
   //    the process, having served its purpose.
   //
   //--------------------------------------------------------------------------

   public void
   DeleteOperandEventHandler
   (
      Phx.IR.Operand operand
   )
   {
      OpndBirthExtensionObject extObj = OpndBirthExtensionObject.Get(operand);
      this.NoteLifetime(this.OpndLongevityArray, extObj.BirthPhase,
         operand.FunctionUnit.Phase);
      operand.RemoveExtensionObject(extObj);
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Make note of the lifetime of an object.
   //
   // Arguments:
   //
   //    longevityArray - The longevity array to update.
   //    birthPhase - The phase the object was born in.
   //    deathPhase - The phase the object died in.
   //
   // Remarks:
   //
   //    We are keeping a simple count of birth-death pairs.
   //
   //--------------------------------------------------------------------------

   private void
   NoteLifetime
   (
      uint [,]         longevityArray,
      Phx.Phases.Phase birthPhase,
      Phx.Phases.Phase deathPhase
   )
   {
      if (birthPhase == null)
      {
         // START/END bracketing IR don't have a birth phase.

         return;
      }

      int birthIndex = this.PhaseArrayList.IndexOf(birthPhase);
      int deathIndex = this.PhaseArrayList.IndexOf(deathPhase);

      longevityArray[birthIndex,deathIndex]++;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Termination event handler to display longevity information.
   //
   // Arguments:
   //
   //    mode - unused.
   //
   // Remarks:
   //
   //    We display the birth-death information collected during compilation.
   //
   //--------------------------------------------------------------------------

   private void
   TermEventHandler
   (
      Phx.Term.Mode mode
   )
   {
      this.DisplayLongevityArray("Instruction", this.InstrLongevityArray);
      System.Console.WriteLine();
      this.DisplayLongevityArray("Operand", this.OpndLongevityArray);
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Display longevity information in tabular format.
   //
   // Arguments:
   //
   //    typeName - Name for the type of longevity info being displayed.
   //    longevityArray - Array of longevity counts.
   //
   // Remarks:
   //
   //    Death phases are displayed across the top with birth phases down the
   //    left. Immediately after the birth phase name is total count of
   //    objects created by that phase. At the intersection of a birth-death
   //    phase pair is the number of objects created in the birth phase and
   //    deleted in the death phase.
   //
   //    Sample dump follows (gathered from compilation of 8queens.c)
   //
   //    Instruction longevity:
   //                                   CxxIL Reader
   //                                  /   MIR Lower
   //                                 /   /   Ssa Info Destruction
   //                                /   /   /   Address Mode Builder
   //                               /   /   /   /   Flow Optimization
   //                              /   /   /   /   /   Encoding
   //                             /   /   /   /   /   /   IR Longevity Sweep
   //                            /   /   /   /   /   /   /
   //    CxxIL Reader     ( 188)   4  28   2  13  34     107
   //    Loop recogniti (   4)                   4
   //    Ssa Info Constant (   8)           8
   //    Canonicalize          (   3)                           3
   //    Lower          (  44)                   2   7  35
   //    Frame Generati (  10)                          10
   //    Block Layout   (  15)                  15
   //
   //--------------------------------------------------------------------------

   private void
   DisplayLongevityArray
   (
      string   typeName,
      uint [,] longevityArray
   )
   {
      Phx.Phases.Phase birthPhase;
      Phx.Phases.Phase deathPhase;
      int              numPhases = this.PhaseArrayList.Count;
      int              birthIndex;
      int              deathIndex;
      bool []          phaseHasDeathInfo = new bool[numPhases];
      int              numPhasesWithDeathInfo = 0;
      
      // Create an instance of StreamWriter to write text to a file.
      
      FileStream fs;

      // We are dumping lifetimes for operands or instructions respectively
      // and we call this method several times. As a result, we have to
      // _create_ the output file for the first time this function is called
      // and _append_ an information to the same file after that.
       
      if (!this.FileOpened)
      {
         // Open an output file for the first time.
         
         fs = new FileStream(this.OutputFile, FileMode.Create,
            FileAccess.Write, FileShare.None);
           
         this.FileOpened = true;
      }
      else
      {
         // Open an output file to append the information to the same file.
           
         fs = new FileStream(this.OutputFile, FileMode.Append,
            FileAccess.Write, FileShare.None);
      }

      using (StreamWriter sw = new StreamWriter(fs))
      {
         // Determine which phases have object deaths in them.

         for (deathIndex = 0; deathIndex < numPhases; deathIndex++)
         {
            for (birthIndex = 0; birthIndex < numPhases; birthIndex++)
            {
               if (longevityArray[birthIndex, deathIndex] != 0)
               {
                  numPhasesWithDeathInfo++;
                  phaseHasDeathInfo[deathIndex] = true;
                  break;
               }
            }
         }

         sw.WriteLine();
         sw.WriteLine("{0} longevity:", typeName);

         // Display death phases across the top with angle lines.

         int deathInfoIndex = 0;
         for (deathIndex = 0; deathIndex < numPhases; deathIndex++)
         {
            if (!phaseHasDeathInfo[deathIndex])
            {
               continue;
            }

            deathPhase = this.PhaseArrayList[deathIndex] as Phx.Phases.Phase;

            sw.Write("{0,32}", " ");

            for (int i = 0; i < numPhasesWithDeathInfo - deathInfoIndex; i++)
            {
               sw.Write(" ");
            }

            for (int i = 0; i < deathInfoIndex; i++)
            {
               sw.Write("/{0, 12}", " ");
            }

            sw.WriteLine(deathPhase.NameString);

            deathInfoIndex++;
         }

         sw.Write("{0,32}", " ");

         for (int i = 0; i < deathInfoIndex; i++)
         {
            sw.Write("/{0, 12}", " ");
         }

         sw.WriteLine();

         // Display birth phases down the left side with death counts across.

         deathInfoIndex = 0;
         for (birthIndex = 0; birthIndex < numPhases; birthIndex++)
         {
            uint birthCount = 0;

            // Prescan to count all births.

            for (deathIndex = birthIndex; deathIndex < numPhases; deathIndex++)
            {
               birthCount += longevityArray[birthIndex, deathIndex];
            }

            // Ignore 0 birth phases.

            if (birthCount != 0)
            {
               birthPhase = this.PhaseArrayList[birthIndex] as Phx.Phases.Phase;

               string truncString = birthPhase.NameString;

               if (truncString.Length > 14)
               {
                  truncString = truncString.Substring(0, 14);
               }

               sw.Write("{0,-14} ( {1,-10} ) |", truncString, birthCount);

               // Space over for each active death phase that is longer in play
               // (because birthIndex is beyond it).

               for (int i = 0; i < deathInfoIndex; i++)
               {
                  sw.Write(" {0,-10} |", " ");
               }

               for (deathIndex = birthIndex; deathIndex < numPhases; deathIndex++)
               {
                  uint count = longevityArray[birthIndex, deathIndex];

                  if (count == 0)
                  {
                     // There is no longevity info for this pair of phases, we
                     // still need to space over if the death phase has any
                     // activity.

                     if (phaseHasDeathInfo[deathIndex])
                     {
                        sw.Write(" {0,-10} |", " ");
                     }

                     continue;
                  }

                  sw.Write(" {0,-10} |", count);
               }

               sw.WriteLine();
            }

            if (phaseHasDeathInfo[birthIndex])
            {
               deathInfoIndex++;
            }
         }

         // Close the output file.

         sw.Close();
      }
   }
}

} // namespace IRLongevity
