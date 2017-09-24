//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Simple Managed Phoenix based application to add nop's to an existing 
//    program
//
//    Input file must be compiled with a phoenix based compiler and -Zi
//
// Usage:
//    Addnop-tool [/ShowResults] <input-binary> <output-binary>
//    
//    Arguments in [] are optional.
//      /ShowResults   Dumps IR before and after instrumentation
//
//-----------------------------------------------------------------------------

// Not using Phx namespace to show explicit class heirarchy
// using Phx;

#region Using directives

using System;

#endregion

//--------------------------------------------------------------------------
//
// Description:
//
//    A helper class does command-line parsing, also works as a holder of
//    some global variables
//
//    Right now, these options are parsed:
//
//       inFile:  The name of the input binary that we want to instrument.
//                This option is required.
//
//       outFile: The name of the instrumented binary that will be created.
//                This option is required.
//
//--------------------------------------------------------------------------

public class CmdLineParser
{
   // The binary to be instrumented

   public static string inFile;

   // The intrumented binary

   public static string outFile;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A control that captures the command-line arguments that do not
   //    match any other controls.
   //
   //--------------------------------------------------------------------------

   class DefaultControl : Phx.Controls.DefaultControl
   {
      public override void
      Process
      (
         string localString
      )
      {
         if (CmdLineParser.inFile == null)
         {
            CmdLineParser.inFile = localString;
         }
         else if (CmdLineParser.outFile == null)
         {
            CmdLineParser.outFile = localString;
         }
         else
         {
            Usage();
            throw new Exception();
         }
      }
   };
   
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Parse and check the command-line variables
   //
   // Returns:
   //
   //    Termination mode
   //
   //--------------------------------------------------------------------------

   public static Phx.Term.Mode
   ParseCmdLine
   (
       string[] argv
   )
   {  
      DefaultControl defaultControl = new DefaultControl();
      Phx.Controls.Parser.RegisterDefaultControl(defaultControl);

      // Check for Phoenix wide options first
      
      Phx.Initialize.EndInitialization("PHX|*|_PHX_|", argv);

      if (CmdLineParser.inFile == null || CmdLineParser.outFile == null)
      {
         Usage();
         return Phx.Term.Mode.Fatal;
      } 

      return Phx.Term.Mode.Normal;
   }
    
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Print the usage string to the console
   //
   // Returns:
   //
   //    Nothing
   //
   //--------------------------------------------------------------------------

   static void
   Usage()
   {
      string usage =
       "\nUsage: AddNop-tool [/ShowResults] <input-binary> <output-binary>\n";
      
      Console.WriteLine(usage);
   }
};

//--------------------------------------------------------------------------
//
// Description:
//
//    A helper class creates control.
//    If enabled, it will dump IR for every functionUnit.
//
//--------------------------------------------------------------------------

public class DumpIRControl
{
   public static void
   StaticInitialize()
   {
      // Create and initialize control

      DumpIRControl.ShowResultsCtrl = 
         Phx.Controls.SetBooleanControl.New ("ShowResults",
         "Dump IR before and after instrumentation",
         "DumpIRControl.StaticInitialize");
   }

   public static Phx.Controls.SetBooleanControl ShowResultsCtrl;
};

public class AddNopTool
{
   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Registers the targets availalable in the RDK.
   //
   // Returns:
   //
   //    Nothing
   //
   //---------------------------------------------------------------------------

   static void
   InitializePhx()
   {
      Phx.Targets.Architectures.Architecture msilArch =
         Phx.Targets.Architectures.Msil.Architecture.New();
      Phx.GlobalData.RegisterTargetArchitecture(msilArch);

      Phx.Targets.Architectures.Architecture x86Arch =
         Phx.Targets.Architectures.X86.Architecture.New();
      Phx.GlobalData.RegisterTargetArchitecture(x86Arch);

      Phx.Targets.Runtimes.Runtime msilRuntime =
         Phx.Targets.Runtimes.Vccrt.Win.Msil.Runtime.New(msilArch);
      Phx.GlobalData.RegisterTargetRuntime(msilRuntime);

      Phx.Targets.Runtimes.Runtime x86Runtime =
         Phx.Targets.Runtimes.Vccrt.Win32.X86.Runtime.New(x86Arch);
      Phx.GlobalData.RegisterTargetRuntime(x86Runtime);
         
      // Initialize the Phx infrastructure
      
      Phx.Initialize.BeginInitialization();
      
      // Initialize control
      
      DumpIRControl.StaticInitialize();
   }
   
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    This method does the work of adding Nop's before every real 
   //    instruction.
   //
   // Arguments:
   //
   //    module - Input module.
   //
   // Returns:
   //
   //    Nothing
   //
   //--------------------------------------------------------------------------

   static void
   DoAddNop
   (
      Phx.PEModuleUnit module
   )
   {  
      // Iterate over each function actually has contribution in the module.

      foreach (Phx.FunctionUnit functionUnit in 
         module.GetEnumerableContributionUnit(Phx.ContributionUnitEnumerationKind.WritableFunctionUnit))
      {
         // Do not modify the block for UnmanagedEntryPoint!
         // It should stay the same before and after instrumentation.
         // Modification of UnmanagedEntryPoint functionUnit
         // will make the instrumented App to fail to initialize properly on execution.

         functionUnit.DisassembleToBeforeLayout();

         if (functionUnit.NameString.Equals("UnmanagedEntryPoint"))
         {
            continue;
         }

         // Dump IR for all funcUnits before instrumentation

         if (DumpIRControl.ShowResultsCtrl.IsEnabled(functionUnit))
         {
            Console.WriteLine("-----DUMP IR BEFORE INSERTING NOPs-----");
         
            foreach (Phx.IR.Instruction instruction in functionUnit.Instructions)
            {
               if (!instruction.IsDataInstruction)
               {
                  instruction.Dump();
               }
            }
         }

         // Iterate the instructions

         foreach (Phx.IR.Instruction instruction in functionUnit.Instructions)
         {
            // HasFallThrough means that the previous instruction can transfer
            // control to this instruction directly.
            // This way we should add nop for the first RealInstruction in the list.
            // One special cases:
            //
            // 1. Don't want to insert nop before the ExitTypeFilter case.
            // A typefilter doesnt generate any real code, and the filter block
            // needs multiple instructions because a label cant have flow.
            // Inserting real code in this block is an error though since
            // it wont make it into the final image.
            
            if (instruction.IsReal && instruction.Previous.HasFallThrough 
               && instruction.Opcode != Phx.Common.Opcode.ExitTypeFilter
               )
            {
               // Create a new Nop

               Phx.IR.ValueInstruction nop = 
                  Phx.IR.ValueInstruction.New(functionUnit, Phx.Common.Opcode.Nop);
               
               // Insert it before the instruction
               
               instruction.InsertBefore(nop);
               
               // Let lower find right instruction opcode.

               functionUnit.Lower.Instruction(nop);
            }
         }

         // Dump IR for all funcUnits after instrumentation

         if (DumpIRControl.ShowResultsCtrl.IsEnabled(functionUnit))
         {
            Console.WriteLine("-----DUMP IR AFTER INSERTING NOPs-----");

            foreach (Phx.IR.Instruction instruction in functionUnit.Instructions)
            {
               if (!instruction.IsDataInstruction)
               {
                  instruction.Dump();
               }
            }
         }
      }
   }
   
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Entry point for application
   //
   // Arguments:
   //
   //    argv - Command-line arguments.
   //
   //--------------------------------------------------------------------------

   public static int
   Main
   (
      String[] argv
   )
   {
      Phx.Term.Mode termMode = Phx.Term.Mode.Normal;
      try
      {
         AddNopTool.InitializePhx();
         
         // Parsing command-line options

         termMode = CmdLineParser.ParseCmdLine(argv);

         if (termMode == Phx.Term.Mode.Fatal)
         {
            Phx.Term.All(termMode);
            return ((termMode == Phx.Term.Mode.Normal) ? 0 : 1);
         }

         Phx.PEModuleUnit module = null;
         
         // Open the module

         module = Phx.PEModuleUnit.Open(CmdLineParser.inFile);

         // Set up the writer

         module.OutputImagePath = CmdLineParser.outFile;

         // Do some useful work on the tool front here

         AddNopTool.DoAddNop(module);

         // Write out the new pe

         module.Close();
      }
      catch (Exception e)
      {
         Console.WriteLine("An Error was encountered while processing '{1}'.\nException details: {0}", 
		    e, CmdLineParser.inFile);
         termMode = Phx.Term.Mode.Fatal;
      }
      
      // clean up Phx and terminate
      
      Phx.Term.All(termMode);
      return ((termMode == Phx.Term.Mode.Normal) ? 0 : 1);
   }
}
