//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    MTrace: A sample program that adds tracing into all
//    the Msil methods of a module
//
// Usage:
//
//    mtrace.exe [options] <input-module>
//
//    MTrace creates and writes out an instrumented version of the
//    input module, with a -mtrace suffix in the module name.  The
//    instrumented executable behaves identically to the original, but
//    if tracing is enabled, tracing output appears on standard
//    output.
//
// Remarks:
//
//    The MTrace sample program adds tracing capabilities into a
//    managed module.
//
//    MTrace adds tracing calls to each method in the module to note
//    when methods are entered and exiting. For example, if the original
//    method is:
//
//        public static int Main(string[] arguments)
//        {
//           Console.WriteLine("Hello, World!");
//           return 0;
//        }
//
//    MTrace modifies the Msil representation of Main to correspond to
//    the following source code:
//
//        public static int Main(string[] arguments)
//        {
//            Trace.WriteLine("Entering Main // foo.cs(23)");
//            Console.WriteLine("Hello World!");
//            Trace.WriteLine("Exiting Main");
//            return 0;
//        }
//
//    Mtrace will also write out an appropriately crafted
//    configuration file to enable the trace output. Thus, with
//    tracing enabled, the method will print its name and source
//    location on entry, and another message on exit. If the module
//    was built with debugging information, the method entry trace
//    will also include line number and file name.
//
//    MTrace makes use of the tracing support in
//    System.Diagnostics.Trace. Please refer MSDN documentation for
//    more information on tracing support in the .Net Framework.
//
//-----------------------------------------------------------------------------

using System;
using System.IO;
using System.Collections;

// Not using Phx namespace to show explicit class hierarchy

public class MTrace
{
   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Primary control of execution class.
   //
   //---------------------------------------------------------------------------
   class Driver
   {
      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Name of the executable file to process.
      //
      //------------------------------------------------------------------------

      string filename;

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Default Control
      //
      // Remarks:
      //
      //    Controls are used in Phoenix to pass options and arguments
      //    into the framework.
      //
      //    The default control is responsible for describing command
      //    line arguments that do not match any other control. Here
      //    we use it to capture the name of the module to process.
      //
      //------------------------------------------------------------------------

      class DefaultControl : Phx.Controls.DefaultControl
      {
         private Driver driver;

         //---------------------------------------------------------------------
         //
         // Description:
         //
         //    Default constructor for DefaultControl
         //
         //---------------------------------------------------------------------

         public DefaultControl
         (
            Driver driver
         )
         {
            this.driver = driver;
         }

         //---------------------------------------------------------------------
         //
         // Description:
         //
         //    Processes the default control from a string.
         //
         //---------------------------------------------------------------------

         public override void
         Process
         (
            string localString
         )
         {
            driver.filename = localString;
         }
      }

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Usage string for this sample.
      //
      // Remarks:
      //
      //    Options can include any of the standard phoenix controls,
      //    eg -dumptypes. There are no sample-specific options at
      //    this time.
      //
      //------------------------------------------------------------------------

      static void
      Usage()
      {
         Console.WriteLine("Usage: mtrace [options] <module-to-instrument>");
      }

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Process the module.
      //
      // Arguments:
      //
      //    argv - Command line arguments.
      //
      // Remarks:
      //
      //    Processes the command line arguments and then performs the
      //    instrumentation on the binary.
      //
      //------------------------------------------------------------------------

      public Phx.Term.Mode
      Process
      (
         string[] argv
      )
      {
         // Setup a default control to capture the name of
         // the module to instrument.

         DefaultControl defaultControl = new DefaultControl(this);
         Phx.Controls.Parser.RegisterDefaultControl(defaultControl);

         // Initialize controls set on the command line or by
         // environment variables, register plugins, etc.

         Phx.Initialize.EndInitialization("PHX|*|_PHX_|", argv);

         // Verify that we got a module name on the command line

         if (this.filename == null)
         {
            Usage();
            return Phx.Term.Mode.Fatal;
         }

         // Make the modifications.

         DoInstrumentation();

         // Write out a config file to enable tracing.

         DoConfigFile();

#if (PHX_DEBUG_CHECKS)
         // Make sure there were no asserts during the run.

         if (Phx.Asserts.AssertCount != 0)
         {
            return Phx.Term.Mode.Fatal;
         }
#endif
         return Phx.Term.Mode.Normal;
      }

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    This method sets up the various phase lists, and then creates
      //    the units and drives the instrumentation process.
      //
      // Remarks:
      //
      //    Phoenix uses Units as containers for IR or other
      //    units. Phase lists operate on units, invoking a succession
      //    of phases to transform units from one state to another.
      //
      //------------------------------------------------------------------------

      void
      DoInstrumentation()
      {
         Console.WriteLine("Processing " + this.filename + " ...");

         // Lookup the architecture and runtime.

         Phx.Targets.Architectures.Architecture architecture =
            Phx.GlobalData.GetTargetArchitecture("X86");
         Phx.Targets.Runtimes.Runtime runtime =
            Phx.GlobalData.GetTargetRuntime("Vccrt-Win32-X86");

         // Create an empty program unit

         Phx.Lifetime lifetime = Phx.Lifetime.New(Phx.LifetimeKind.Global, null);

         Phx.ProgramUnit programUnit = Phx.ProgramUnit.New(lifetime, null,
            Phx.GlobalData.TypeTable, architecture, runtime);

         // Make an empty moduleUnit

         Phx.PEModuleUnit moduleUnit = Phx.PEModuleUnit.New(lifetime,
            Phx.Name.New(lifetime, this.filename), programUnit,
            Phx.GlobalData.TypeTable, architecture, runtime);

         // Create an overall phase list....

         Phx.Phases.PhaseConfiguration config =
            Phx.Phases.PhaseConfiguration.New(lifetime, "MTrace Phases");

         // Add phases...

         config.PhaseList.AppendPhase(Phx.PE.ReaderPhase.New(config));

         {
            // Create the per-function phase list....

            Phx.Phases.PhaseList unitList = Phx.PE.UnitListPhaseList.New(config,
               Phx.PE.UnitListWalkOrder.PrePass);

            unitList.AppendPhase(
               Phx.PE.RaiseIRPhase.New(config,
                  Phx.FunctionUnit.LowLevelIRBeforeLayoutFunctionUnitState));

            // HERE: change this phase if you want to instrument at function
            // or basic block level
            unitList.AppendPhase(InstrumentPhase.New(config));

            unitList.AppendPhase(EncodePhase.New(config));
            unitList.AppendPhase(Phx.PE.DiscardIRPhase.New(config));

            config.PhaseList.AppendPhase(unitList);
         }

         config.PhaseList.AppendPhase(EmitPhase.New(config));

         // Add any plugins or target-specific phases.

         Phx.GlobalData.BuildPlugInPhases(config);

         // Run the phases.

         config.PhaseList.DoPhaseList(moduleUnit);
      }

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    This method writes a config file to enable instrumentation
      //    for the instrumented module.
      //
      //------------------------------------------------------------------------

      void
      DoConfigFile()
      {
         string configFileName =
            Path.GetFileNameWithoutExtension(this.filename)
               + "-mtrace" + Path.GetExtension(this.filename)
               + ".config";

         Console.WriteLine("Writing " + configFileName + " ...");

         StreamWriter configFile = new StreamWriter(configFileName);

         configFile.WriteLine("<configuration>");
         configFile.WriteLine(" <system.diagnostics>");
         configFile.WriteLine("  <trace autoflush=\"true\" indentsize=\"3\" >");
         configFile.WriteLine("   <listeners>");
         configFile.WriteLine("    <remove name=\"Default\"/>");
         configFile.Write(    "    <add name=\"ConsoleTraceListener\"");
         configFile.WriteLine(" type=\"System.Diagnostics.ConsoleTraceListener\"/>");
         configFile.WriteLine("   </listeners>");
         configFile.WriteLine("  </trace>");
         configFile.WriteLine(" </system.diagnostics>");
         configFile.WriteLine("</configuration>");
         configFile.Close();
         Console.WriteLine("... done");
      }
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    This class is responsible for the instrumentation of each method.
   //
   //---------------------------------------------------------------------------

   class InstrumentPhase : Phx.Phases.Phase
   {

#if (PHX_DEBUG_SUPPORT)
      private static Phx.Controls.ComponentControl InstrumentPhaseControl;
#endif

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Initialize the instrumentation phase.
      //
      // Remarks:
      //
      //    Sets up a component control under debug builds, so that we
      //    can dump ir before/after the phase and so on. In other
      //    words, because we create this control, you can pass in
      //    options like -predumpmtrace that will dump the IR for each
      //    method before this phase runs.
      //
      //------------------------------------------------------------------------

      public static void
      Initialize()
      {
#if (PHX_DEBUG_SUPPORT)
         InstrumentPhaseControl = Phx.Controls.ComponentControl.New("mtrace",
            "Inject tracing into each method", "nop-plug-in.cs");
#endif
      }

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Create a new InstrumentationPhase object.
      //
      // Arguments:
      //
      //    config - The encapsulating object that simplifies handling of
      //    the phase list and pre and post phase events.
      //
      // Returns:
      //
      //    A new InstrumentationPhase object.
      //
      //------------------------------------------------------------------------

      public static InstrumentPhase
      New
      (
         Phx.Phases.PhaseConfiguration config
      )
      {
         InstrumentPhase instrumentPhase = new InstrumentPhase();

         instrumentPhase.Initialize(config, "MTrace Instrumentation");

#if (PHX_DEBUG_SUPPORT)
         instrumentPhase.PhaseControl = InstrumentPhaseControl;
#endif

         return instrumentPhase;
      }

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Locate a reference to a named assembly.
      //
      // Arguments:
      //
      //    moduleUnit - Module that the assembly reference is located in.
      //    assemblyNameString - Name of the assembly.
      //
      // Returns:
      //
      //    Assembly reference symbol for the named assembly specified.
      //
      //------------------------------------------------------------------------

      Phx.Symbols.AssemblySymbol
      LookupAssembly
      (
         Phx.ModuleUnit moduleUnit,
         string assemblyNameString
      )
      {
         Phx.Name assemblyName =
            Phx.Name.New(Phx.GlobalData.GlobalLifetime, assemblyNameString);
         Phx.Symbols.Table symbolTable = moduleUnit.SymbolTable;
         Phx.Symbols.NameMap nameMap = symbolTable.NameMap;

         // If the module was not compiled with debug, the module sym
         // table may not have a name map.

         if (nameMap == null)
         {
            nameMap = Phx.Symbols.NameMap.New(symbolTable, 64);
            symbolTable.AddMap(nameMap);
         }

         Phx.Symbols.Symbol sym = nameMap.Lookup(assemblyName);

         // Note that there might be a number of symbols with
         // identical names, so search through until we have an
         // assembly reference.

         while (sym != null)
         {
            if (sym.IsAssemblySymbol)
            {
               return sym.AsAssemblySymbol;
            }
            sym = nameMap.LookupNext(sym);
         }
         return null;
      }

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //   Cached FunctionSymbol for [System]System.Diagnostic.Trace.WriteLine(string).
      //
      //------------------------------------------------------------------------

      private Phx.Symbols.FunctionSymbol traceFuncSym;

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Create a symbol for the tracing function
      //       [System]System.Diagnostic.Trace.WriteLine(string).
      //
      // Remarks:
      //
      //    We need to call
      //    [System]System.Diagnostic.Trace.WriteLine(string) in our
      //    instrumentation, so we need a way to refer to this
      //    method. In the IR we just need a functionSymbol, which we'll
      //    create here and cache.
      //
      //    Getting the proper functionSymbol  requires a few steps:
      //
      //    (1) Locate or create a reference to the System assembly.
      //    (2) Locate or create the System.Diagnostics.Trace class
      //    (3) Locate or create the WriteLine(string) method.
      //
      //------------------------------------------------------------------------

      void
      ImportTypes
      (
         Phx.ModuleUnit moduleUnit
      )
      {
         // Only do this once.

         if (traceFuncSym != null)
         {
            return;
         }

         // (1) -- get a reference to the System Assembly.
         //
         // We first look through all the existing assembly
         // references to see if System happens to be there.  If not,
         // we create a new reference and pattern it after the
         // expected reference to mscorlib.

         Phx.Symbols.AssemblySymbol referenceSymbol = LookupAssembly(moduleUnit, "System");

         if (referenceSymbol == null)
         {
            // Pattern an assembly ref to system after mscorlib.

            Phx.Symbols.AssemblySymbol mscorlibSym =
               LookupAssembly(moduleUnit, "mscorlib");
            Phx.Manifest sysManifest =
               Phx.Manifest.New(Phx.GlobalData.GlobalLifetime);
            Phx.Manifest mscorlibManifest = mscorlibSym.Manifest;

            Phx.Name sysName =
               Phx.Name.New(Phx.GlobalData.GlobalLifetime, "System");

            sysManifest.Name = sysName;
            sysManifest.HashAlgorithm = mscorlibManifest.HashAlgorithm;
            sysManifest.PublicKey = mscorlibManifest.PublicKey;
            sysManifest.Version = mscorlibManifest.Version;
            sysManifest.CultureString = mscorlibManifest.CultureString;

            referenceSymbol = Phx.Symbols.AssemblySymbol.New(null,
               sysManifest, sysName, moduleUnit.SymbolTable);
         }

         // (2) Create a class type for System.Diagnostics.Trace.
         //
         // This class will just be a simple aggregate type.
         // (handle case where it already exists)

         Phx.Name traceTypeName = Phx.Name.New(Phx.GlobalData.GlobalLifetime,
            "System.Diagnostics.Trace");

         Phx.Symbols.MsilTypeSymbol traceTypeSym =
            Phx.Symbols.MsilTypeSymbol.New(moduleUnit.SymbolTable, traceTypeName, 0);

         Phx.Types.AggregateType traceType =
            Phx.Types.AggregateType.NewDynamicSize(moduleUnit.TypeTable,
            traceTypeSym);

         traceType.IsDefinition = false;

         // Now, attach the class type to the mscorlib assembly reference.

         referenceSymbol.InsertInLexicalScope(traceTypeSym, traceTypeName);

         // (3) Create the symbol for the WriteLine function.
         // (handle case where it already exists)

         // Build up a phx type for the method.

         Phx.Types.Type stringType = moduleUnit.TypeTable.GetObjectPointerType(
            moduleUnit.TypeTable.SystemStringAggregateType);

         Phx.Types.FunctionType functionType = moduleUnit.TypeTable.GetFunctionType(
            Phx.Types.CallingConventionKind.ClrCall, 0,
            moduleUnit.TypeTable.VoidType,
            stringType, null, null, null);

         Phx.Name functionName = Phx.Name.New(Phx.GlobalData.GlobalLifetime,
            "WriteLine");
         traceFuncSym = Phx.Symbols.FunctionSymbol.New(moduleUnit.SymbolTable, 0, functionName,
            functionType, Phx.Symbols.Visibility.GlobalReference);

         // Add it as a method of the traceType.

         traceType.AppendMethodSymbol(traceFuncSym);
         traceTypeSym.InsertInLexicalScope(traceFuncSym, functionName);
      }

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Instrument a particular function.
      //
      // Arguments:
      //
      //    unit - The unit this phase is operating on.
      //
      //------------------------------------------------------------------------

      override protected void
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

         // Only handle Msil functions.

         if (!functionUnit.Architecture.NameString.Equals("Msil"))
         {
            return;
         }

         Phx.ModuleUnit moduleUnit = functionUnit.ParentUnit.AsModuleUnit;

         // Make sure we have the necessary function sym.

         ImportTypes(moduleUnit);

         // Find the ENTERFUNC, and insert a ldstr/call just afterwards.

         Phx.IR.Instruction enterFunction =
            functionUnit.FirstInstruction.FindNextInstruction(Phx.Common.Opcode.EnterFunction);

         // Build up the string for function entry. Get the source
         // location.  Note that methods may be lacking debug
         // information either because they were not compiled with
         // debug, or because they represent compiler-injected
         // methods like .ctors.

         uint debugTag = enterFunction.DebugTag;
         string messageString = "Entering " + functionUnit.NameString;

         if (debugTag != 0)
         {
            Phx.Debug.Info debugInfo = functionUnit.DebugInfo;
            string sourceFileName = debugInfo.GetFileName(debugTag);
            uint sourceLine = debugInfo.GetLineNumber(debugTag);
            string sourceFile = Path.GetFileName(sourceFileName);
            messageString +=  " [" + sourceFile + "(" + sourceLine + ")]";
         }

         InstrumentAfter(enterFunction, messageString);

         // Now find the return points. These will all reference the
         // EXITFUNC label which in turn should reference END.

         Phx.IR.Instruction exitFunc =
            functionUnit.LastInstruction.FindPreviousInstruction(Phx.Common.Opcode.ExitFunction);
         Phx.IR.LabelOperand exitLabelOperand = exitFunc.DestinationOperand.AsLabelOperand;

         // Keep track of which ones we have instrumented to avoid doubling up.

         ArrayList iRet = new ArrayList();

         for (Phx.IR.LabelOperand refOpnd = exitLabelOperand.ReferenceLabelOperandList;
              refOpnd != null; refOpnd = refOpnd.NextReferenceLabelOperand)
         {
            // See if ref comes from a return instruction.

            Phx.IR.Instruction refInstr = refOpnd.Instruction;

            if (refInstr.IsReturn)
            {
               // We have a ret, and we'd like to instrument just
               // before it.  Note that refInstr.Previous is guaranteed
               // to be in the same basic block, so we don't have do
               // do anything complicated here.

               if (!iRet.Contains(refInstr))
               {
                  InstrumentAfter(refInstr.Previous, "Leaving " +
                     functionUnit.NameString);
                  iRet.Add(refInstr);
               }
            }
         }
      }

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Add instrumentation just after the indicated instruction.
      //
      // Arguments:
      //
      //    instruction - Instruction to insert instrumentation after.
      //    messageString - Message to insert.
      //
      //------------------------------------------------------------------------

      void
      InstrumentAfter
      (
         Phx.IR.Instruction instruction,
         string messageString
      )
      {
         Phx.FunctionUnit functionUnit = instruction.FunctionUnit;
         Phx.ModuleUnit moduleUnit = functionUnit.ParentUnit.AsModuleUnit;
         functionUnit.CurrentDebugTag = instruction.DebugTag;

         Phx.Types.Type stringType = moduleUnit.TypeTable.GetObjectPointerType(
            moduleUnit.TypeTable.SystemStringAggregateType);

         Phx.Name stringName = Phx.Name.New(Phx.GlobalData.GlobalLifetime, messageString);
         Phx.Symbols.ConstantSymbol stringSym = Phx.Symbols.ConstantSymbol.New(moduleUnit.SymbolTable, 0,
            stringName, moduleUnit.TypeTable.ObjectPointerSystemStringType, messageString);

         Phx.IR.Operand strOpnd = Phx.IR.MemoryOperand.NewAddress(functionUnit,
            stringType, stringSym, null, 0,
            Phx.Alignment.NaturalAlignment(stringType),
            functionUnit.AliasInfo.NotAliasedMemoryTag);

         Phx.IR.Instruction ldStr = Phx.IR.ValueInstruction.NewUnaryExpression(functionUnit,
            Phx.Targets.Architectures.Msil.Opcode.ldstr, stringType, strOpnd);

         Phx.IR.CallInstruction call = Phx.IR.CallInstruction.New(functionUnit,
             Phx.Targets.Architectures.Msil.Opcode.call,
            this.traceFuncSym);

         call.AppendSource(ldStr.DestinationOperand);
         instruction.InsertAfter(call);
         instruction.InsertAfter(ldStr);

         // Assign registers.

         ldStr.DestinationOperand.Register = Phx.Targets.Architectures.Msil.Register.SR0;
         call.GetFirstArgumentOperand().Register = Phx.Targets.Architectures.Msil.Register.SR0;

         // Explicitly break the expression temporary link here since we
         // assigned registers.

         ldStr.DestinationOperand.BreakExpressionTemporary();
      }
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Encode the modified function.
   //
   // Remarks:
   //
   //    This phase produces encoded binary instructions from the
   //    Phoenix IR.
   //
   //---------------------------------------------------------------------------

   class EncodePhase : Phx.Phases.Phase
   {
      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Creates a new EncodePhase object
      //
      // Arguments:
      //
      //    config - The encapsulating object that simplifies handling of
      //    the phase list and pre and post phase events.
      //
      //------------------------------------------------------------------------

      public static EncodePhase
      New
      (
         Phx.Phases.PhaseConfiguration config
      )
      {
         EncodePhase encodePhase = new EncodePhase();

         encodePhase.Initialize(config, "Encode");

         return encodePhase;
      }

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Executes the EncodePhase phase.
      //
      // Arguments:
      //
      //    unit - The unit that this phase should operate on.
      //
      //------------------------------------------------------------------------

      override protected void
      Execute
      (
         Phx.Unit unit
      )
      {
         Phx.PE.Writer.EncodeUnit(unit);
      }
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Emit the modified binary...
   //
   // Remarks:
   //
   //    This phase writes out the instrumented binary.
   //
   //---------------------------------------------------------------------------

   class EmitPhase : Phx.Phases.Phase
   {
      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Creates a new EmitPhase object
      //
      // Arguments:
      //
      //    config - The encapsulating object that simplifies handling of
      //    the phase list and pre and post phase events.
      //
      //------------------------------------------------------------------------

      public static EmitPhase
      New
      (
         Phx.Phases.PhaseConfiguration config
      )
      {
         EmitPhase emitPhase = new EmitPhase();

         emitPhase.Initialize(config, "Emit");

         return emitPhase;
      }

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    Executes the EmitPhase phase.
      //
      // Arguments:
      //
      //    unit - The unit that this phase should operate on.
      //
      //------------------------------------------------------------------------

      override protected void
      Execute
      (
         Phx.Unit unit
      )
      {
         if (!unit.IsPEModuleUnit)
         {
            return;
         }

         Phx.PEModuleUnit peModule = unit.AsPEModuleUnit;

         string inputFileNameStr =
            peModule.NameString;
         string inputFileNameBaseStr =
            Path.GetFileNameWithoutExtension(inputFileNameStr);
         string outputFileNameString =
            inputFileNameBaseStr + "-mtrace"
            + Path.GetExtension(inputFileNameStr);
         string outputPdbFileNameStr =
            inputFileNameBaseStr + "-mtrace.pdb";

         Console.WriteLine("...writing to " + outputFileNameString);

         // PDB may not exist; if so, pass empty string to the
         // writer for the pdb name.

         if (peModule.PdbReader == null)
         {
            outputPdbFileNameStr = "";
         }
         else
         {
            Console.WriteLine("...writing pdb to " + outputPdbFileNameStr);
         }

         Phx.PE.Writer writer = Phx.PE.Writer.New(
            Phx.GlobalData.GlobalLifetime, outputFileNameString,
            outputPdbFileNameStr, peModule, peModule.SymbolTable,
            peModule.Architecture, peModule.Runtime);

         writer.Write();
      }
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Initialize the dependent architectures and runtimes. We
   //    include x86 here in the off chance that we see a mixed (x86
   //    and msil) binary.
   //
   //---------------------------------------------------------------------------

   static void
   InitializeTargets()
   {
      // Setup X86/Msil Target and architecture

      Phx.Targets.Architectures.Architecture x86Arch =
         Phx.Targets.Architectures.X86.Architecture.New();
      Phx.Targets.Runtimes.Runtime win32x86Runtime =
         Phx.Targets.Runtimes.Vccrt.Win32.X86.Runtime.New(x86Arch);
      Phx.GlobalData.RegisterTargetArchitecture(x86Arch);
      Phx.GlobalData.RegisterTargetRuntime(win32x86Runtime);

      Phx.Targets.Architectures.Architecture msilArch =
         Phx.Targets.Architectures.Msil.Architecture.New();
      Phx.Targets.Runtimes.Runtime winMSILRuntime =
         Phx.Targets.Runtimes.Vccrt.Win.Msil.Runtime.New(msilArch);
      Phx.GlobalData.RegisterTargetArchitecture(msilArch);
      Phx.GlobalData.RegisterTargetRuntime(winMSILRuntime);
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Entry point for application
   //
   // Arguments:
   //
   //    argv - Command line argument strings.
   //
   //---------------------------------------------------------------------------

   public static int
   Main
   (
      String[] argv
   )
   {
      InitializeTargets();
      Phx.Initialize.BeginInitialization();
      InstrumentPhase.Initialize();

      Driver driver = new Driver();
      Phx.Term.Mode termMode = driver.Process(argv);

      Phx.Term.All(termMode);
      return (termMode == Phx.Term.Mode.Normal ? 0 : 1);
   }
}
