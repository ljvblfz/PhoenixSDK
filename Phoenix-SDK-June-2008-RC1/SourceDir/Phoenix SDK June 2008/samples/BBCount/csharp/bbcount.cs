#region Using directives

using System;
using System.IO;
using System.Collections;
using System.Xml;

#endregion

// Not using Phx namespace to show explicit class hierarchy

//--------------------------------------------------------------------------
//
// Description:
//
//    BBCount: A sample program that instruments an Msil assembly/module
//    with basic block counting routines which is in a DLL.
//
// Usage:
//
//    BBCount [/outdir <output-file-name>] [/pdb] [/map <map-name>]
//            [/cnt <count-name>] [/v] [/d] <src-assembly-list>
//
//      Perform basic-block instrumentation on an Msil image.
//      Arguments in [] are optional.
//
//      /v:   verbose
//      /d:   dump per-block disassembly
//      /outdir: directory to hold the instrumented Msil image
//            Default: current directory plus \instrumented
//      /pdb: output an updated pdb to the output directory
//      /map: name of map where you want to keep the mapping information.
//            Default: bbcount.map
//      /cnt: specify the file where you want to keep the total number
//            of basic blocks.
//            Default: bbcount.cnt
//
//      <src-assembly-list>
//            A semicolon-separated list of assemblies to instrument.
//            For each input assembly foo.exe an output assembly foo-new.exe is
//            created.
//
// Remarks:
//
//    This sample provides some more general interfaces that intend to
//    simplify instrumenting Msil modules. BBCount is just one
//    example that shows how to use these interfaces.
//
//
//--------------------------------------------------------------------------

public class Instrumentor
{

   //------------------------------------------------------------------------
   //
   //Description:
   //
   //    Initialization of the Phoenix infrastructure:
   //       (1) set the appropriate code generation targets and runtime
   //           for Phoenix. Here we assume targets is x86+msil
   //       (2) Initialize the Phx framework (Phx.Initialize.All())
   //
   // Returns:
   //
   //    Nothing
   //
   // Remarks:
   //
   //    A more general way is to read in the target from command line.
   //    Right now, we just assume it is x86+msil
   //
   //------------------------------------------------------------------------

   public static void
   InitializePhx()
   {
      // Setup X86/Msil Target and architecture

      Phx.Targets.Architectures.Architecture x86Arch = Phx.Targets.Architectures.X86.Architecture.New();
      Phx.Targets.Runtimes.Runtime win32x86Runtime =
               Phx.Targets.Runtimes.Vccrt.Win32.X86.Runtime.New(x86Arch);
      Phx.GlobalData.RegisterTargetArchitecture(x86Arch);
      Phx.GlobalData.RegisterTargetRuntime(win32x86Runtime);

      Phx.Targets.Architectures.Architecture msilArch = Phx.Targets.Architectures.Msil.Architecture.New();
      Phx.Targets.Runtimes.Runtime winMSILRuntime =
               Phx.Targets.Runtimes.Vccrt.Win.Msil.Runtime.New(msilArch);

      Phx.GlobalData.RegisterTargetArchitecture(msilArch);
      Phx.GlobalData.RegisterTargetRuntime(winMSILRuntime);

      // Initialize the Phx infrastructure
      Phx.Initialize.BeginInitialization();

      // And our phase
      InstrumentPhase.Initialize();
   }

   // A global logger that is used by different phases
   private static Logger logger;

   //------------------------------------------------------------------------
   //
   // Description:
   //
   //    The initialization of the Instrumentor.
   //    For now, we only do some initialization for the IntrumentPhase
   //
   //------------------------------------------------------------------------

   public static void
   Initialize(Logger l)
   {
      logger = l;
   }

   // The assembly is being processed
   private static string currentAssembly;

   // The main entry for processing one assembly
   public static void
   Process(string inFile)
   {
      currentAssembly = inFile;

      if(CmdLineParser.verbose)
         Console.WriteLine("Processing " + currentAssembly + " ...");

      // write out the map info for the assembly processing
      logger.StartAssemblyMap(currentAssembly);

      // Lookup the architecture and runtime.

      Phx.Targets.Architectures.Architecture architecture = Phx.GlobalData.GetTargetArchitecture("X86");
      Phx.Targets.Runtimes.Runtime runtime =
                  Phx.GlobalData.GetTargetRuntime("Vccrt-Win32-X86");

      // Create an empty program unit in which to place the module unit to be read.

      Phx.Lifetime lifetime = Phx.Lifetime.New(Phx.LifetimeKind.Global, null);

      Phx.ProgramUnit programUnit = Phx.ProgramUnit.New(lifetime, null,
         Phx.GlobalData.TypeTable, architecture, runtime);

      // Make an empty moduleUnit.
      //   The PEModuleUnit class is used as the top level Unit when a PE
      //   executable is read.  This extends Phx::ModuleUnit with properties
      //   for the PE specific attributes read from the source executable
      //   that have no other representation in the Phoenix IR.

      Phx.PEModuleUnit moduleUnit = Phx.PEModuleUnit.New(lifetime,
         Phx.Name.New(lifetime, currentAssembly), programUnit,
         Phx.GlobalData.TypeTable, architecture, runtime);

      // Prepare a phase list that we want to do with the input module
      Phx.Phases.PhaseConfiguration config = PreparePhaseList(lifetime);

      // Execute the phase list
      config.PhaseList.DoPhaseList(moduleUnit);

      // Make a closure for the assemlby in the map file
      logger.EndAssemblyMap();

   }

   private static Phx.Phases.PhaseConfiguration
   PreparePhaseList(Phx.Lifetime lifetime)
   {
      Phx.Phases.PhaseConfiguration config = Phx.
         Phases.PhaseConfiguration.New(lifetime, "BBCount Phases");

      // Add phases...

      config.PhaseList.AppendPhase(
         Phx.PE.ReaderPhase.New(config));

      {
         // Create the per-function phase list....

         Phx.Phases.PhaseList unitList = Phx.PE.UnitListPhaseList.New(config,
            Phx.PE.UnitListWalkOrder.PrePass);

         unitList.AppendPhase(
            Phx.PE.RaiseIRPhase.New(config,
               Phx.FunctionUnit.LowLevelIRBeforeLayoutFunctionUnitState));

         // HERE: change this phase if you want to instrument at function
         // or basic block level
         unitList.AppendPhase(InstrumentPhase.New(config, logger));

         unitList.AppendPhase(EncodePhase.New(config));
         unitList.AppendPhase(Phx.PE.DiscardIRPhase.New(config));

         config.PhaseList.AppendPhase(unitList);
      }

      // HERE: change this phase if you want to instrument program start point
      // Right now there is no instrumentation for program start point
      //lastPhase = InstrumentPhase.New(config, lastPhase);

      config.PhaseList.AppendPhase(EmitPhase.New(config));

      // Add any plugins or target-specific phases.

      Phx.GlobalData.BuildPlugInPhases(config);

      return config;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    This class is responsible for the instrumentation
   //
   //--------------------------------------------------------------------------

   public class InstrumentPhase : Phx.Phases.Phase
   {

#if (PHX_DEBUG_SUPPORT)

       private static Phx.Controls.ComponentControl InstrumentPhaseControl;

#endif
      // HERE: put static data structures that are common for instrumenting
      // both ModuleUnit and FunctionUnit

      // unique ID assigned to each basic block
      public static uint currentId;

      // A hashtable of call prototypes that are already created
      private static Hashtable funcProtoFactory;

      // A queue of instructions from the current function
      public static Queue funcDisassembly;

      // Initialization of the static members of InstrumentPhase class
      public static void
      Initialize()
      {

#if (PHX_DEBUG_SUPPORT)

         InstrumentPhaseControl = Phx.Controls.ComponentControl.New("bbcount",
             "Inject bb counting into each method", "bbcount.cs");

#endif
         currentId = 0;
         funcProtoFactory = new Hashtable();
         funcDisassembly = new Queue();
      }


      // The logger used to dump out map information
      private Logger logger;

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    The static constructor of InstrumentPhase
      //
      //-----------------------------------------------------------------------
      public static InstrumentPhase
      New
      (
         Phx.Phases.PhaseConfiguration config,
         Logger logger
      )
      {
         InstrumentPhase instrumentPhase = new InstrumentPhase();

         instrumentPhase.Initialize(config, "BBCount Instrumentation");

#if (PHX_DEBUG_SUPPORT)

         instrumentPhase.PhaseControl = InstrumentPhaseControl;

#endif

         instrumentPhase.logger = logger;

         return instrumentPhase;
      }


      //-----------------------------------------------------------------------
      // The allowed types for parameters of the instrumented call
      //-----------------------------------------------------------------------
      enum ParamType
      {
         zCHAR,
         zINT,
         zUINT,
         zLONG,
         zULONG,
         zCHARPTR,
         //TODO: more?
      }

      //----------------------------------------------------------------------
      //
      // Description:
      //
      //    Provide interfaces to import a method reference to a Msil module
      //
      //----------------------------------------------------------------------

      class FuncPrototype
      {
         public Phx.Symbols.FunctionSymbol functionSymbol;

         // The type of the argument list, from left to right
         public ParamType[] argumentTypes;

         //-------------------------------------------------------------------
         //
         // Description:
         //
         //    Construct a new instance of FuncPrototype based on the given
         //    method signature, create a functionSymbol for the imported method
         //
         //
         // Remarks:
         //
         //    Method signature is in format:
         //
         //       [AssemblyName]ClassName.MethodName(argumentType1,[argumentType2,..])
         //
         //       where argumentType is one of ParamType defined above
         //
         //    For example, the signature used in this sample:
         //
         //       [RtWrapper]RtWrapper.BBCount(zUINT)
         //
         //-------------------------------------------------------------------

         public FuncPrototype
         (
            Phx.PEModuleUnit module,
            string funcSig
         )
         {

            // parsing the function signature
            // Format: [assembly]class.method(param1type,paramt2ype,...)

            int idx1 = funcSig.IndexOf('[');
            int idx2 = funcSig.IndexOf(']');

            string assemblyName = funcSig.Substring(idx1+1, idx2-idx1-1);

            int idx3 = funcSig.LastIndexOf('.');
            string className = funcSig.Substring(idx2+1, idx3-idx2-1);

            int idx4 = funcSig.IndexOf('(');
            string methodName = funcSig.Substring(idx3+1, idx4-idx3-1);

            // parse the parameter type list

            int idx5 = funcSig.LastIndexOf(')');
            char[] dem = {','};
            string[] splits = (funcSig.Substring(idx4+1, idx5-idx4-1)).Split(dem);

            int argc = splits.Length;

            argumentTypes = new ParamType[argc];

            for(int i = 0; i < argc; i++)
            {
               argumentTypes[i] = (ParamType)ParamType.Parse(
                                 typeof(ParamType), splits[i].Trim());
            }

            // translate the ParamType to internal phx Type
            Phx.Types.Type[] internalArgTypes = new Phx.Types.Type[argc];

            for(int i = 0; i < argc; i++)
            {
               switch(argumentTypes[i])
               {
               case ParamType.zCHAR:
                  internalArgTypes[i] = module.TypeTable.Character16Type;
                  break;

               case ParamType.zINT:
                  internalArgTypes[i] = module.TypeTable.Int32Type;
                  break;

               case ParamType.zUINT:
                  internalArgTypes[i] = module.TypeTable.UInt32Type;
                  break;

               case ParamType.zLONG:
                  internalArgTypes[i] = module.TypeTable.Int64Type;
                  break;

               case ParamType.zULONG:
                  internalArgTypes[i] = module.TypeTable.UInt64Type;
                  break;

               case ParamType.zCHARPTR:
                  internalArgTypes[i] = module.TypeTable.GetObjectPointerType(
                            module.TypeTable.SystemStringAggregateType);
                  break;

               default:
                  Console.WriteLine("Unsupported type for parameters of callback function");
                  throw(new Exception());
               }
            }

            // Import the related Symbols of this function prototype
            functionSymbol = CreateFunctionSymbol(module, assemblyName, className, methodName,
                        internalArgTypes);
         }


         //------------------------------------------------------------------
         //
         // Description:
         //
         //    A helper function that creates a functionSymbol for the inserted call
         //
         //------------------------------------------------------------------

         private Phx.Symbols.FunctionSymbol
         CreateFunctionSymbol
         (
            Phx.PEModuleUnit moduleUnit,
            string assemblyName,
            string className,
            string methodName,
            Phx.Types.Type[] argumentTypeList
         )
         {
            // (1) -- get a Symbol reference to the Assembly.

            Phx.Symbols.AssemblySymbol assemblySymbol = FindOrCreateAssemblySym(
                     moduleUnit, assemblyName);

            // (2) -- get a Symbol reference to the class

            Phx.Symbols.MsilTypeSymbol classTypeSym = FindOrCreateClassSym(
                     moduleUnit, className, assemblySymbol);

            // (3) -- create a type for the class

            Phx.Types.AggregateType classType = Phx.Types.AggregateType.NewDynamicSize(
                     moduleUnit.TypeTable, classTypeSym);

            classType.IsDefinition = false;


            // (4) -- Create the symbol reference for the inserted method.

            Phx.Types.FunctionTypeBuilder functionTypeBuilder =
                     Phx.Types.FunctionTypeBuilder.New(moduleUnit.TypeTable);

            functionTypeBuilder.Begin();

            // Set the calling convention
            functionTypeBuilder.CallingConventionKind =
                              Phx.Types.CallingConventionKind.ClrCall;

            // Set the return type - instrumented call always return void
            functionTypeBuilder.AppendReturnParameter(moduleUnit.TypeTable.VoidType);

            // Set the param types..
            for(int i = 0; i < argumentTypeList.Length; i++)
            {
               functionTypeBuilder.AppendParameter(argumentTypeList[i]);
            }

            Phx.Types.FunctionType functionType = functionTypeBuilder.GetFunctionType();

            Phx.Name functionName = Phx.Name.New(moduleUnit.Lifetime, methodName);

            functionSymbol = Phx.Symbols.FunctionSymbol.New(moduleUnit.SymbolTable, 0, functionName,
               functionType, Phx.Symbols.Visibility.GlobalReference);
            classTypeSym.InsertInLexicalScope(functionSymbol, functionName);

            // Add it as a method of classType.
            classType.AppendMethodSymbol(functionSymbol);

            return functionSymbol;
         }

         //--------------------------------------------------------------------------
         //
         // Description:
         //
         //    Find or create a sym reference to a named assembly.
         //
         //--------------------------------------------------------------------------

         private Phx.Symbols.AssemblySymbol
         FindOrCreateAssemblySym
         (
            Phx.ModuleUnit moduleUnit,
            string assemblyName
         )
         {
            // A more general API that creates a Symbol given the path name
            // of an assembly or module will be available soon.

            // For now, we assume the assembly is created in default way
            // by csc without publictoken in it. BCL assemblies are not
            // supported yet.

            // We first look through all the existing assembly
            // references to see if this assembly is already there.


            Phx.Symbols.AssemblySymbol referenceSymbol = null;

            foreach (Phx.Symbols.Symbol symbol in moduleUnit.SymbolTable.AllSymbols)
            {
               if (symbol is Phx.Symbols.AssemblySymbol)
               {
                  Phx.Symbols.AssemblySymbol assemblySymbol = symbol as Phx.Symbols.AssemblySymbol;

                  if (assemblySymbol.NameString.Equals(assemblyName))
                  {
                     referenceSymbol = assemblySymbol;
                     break;
                  }
               }
            }

            // The assemblySymbol is not created yet

            if (referenceSymbol == null)
            {
							 // TODO: Hard coded creation of a manifest. Should be other way
							 // Add more general way once API for manifest is available

               Phx.Name dllName = Phx.Name.New(moduleUnit.Lifetime, assemblyName);
               Phx.Manifest dllManifest = Phx.Manifest.New(moduleUnit.Lifetime);

               dllManifest.Name = dllName;
               dllManifest.HashAlgorithm = 0x00008004;

               Phx.Version version = Phx.Version.New(moduleUnit.Lifetime);
               version.BuildNumber = 0;
               version.MajorVersion = 0;
               version.MinorVersion = 0;
               version.RevisionNumber = 0;
               dllManifest.Version = version;

               referenceSymbol = Phx.Symbols.AssemblySymbol.New(null,
                  dllManifest, dllName, moduleUnit.SymbolTable);
            }

            return referenceSymbol;
         }

         //--------------------------------------------------------------------------
         //
         // Description:
         //
         //    Find or create a sym reference to a named class.
         //
         //--------------------------------------------------------------------------

         private Phx.Symbols.MsilTypeSymbol
         FindOrCreateClassSym
         (
            Phx.ModuleUnit moduleUnit,
            string className,
            Phx.Symbols.AssemblySymbol assemblySymbol
         )
         {
            Phx.Symbols.MsilTypeSymbol classTypeSym = null;

            foreach (Phx.Symbols.Symbol symbol in moduleUnit.SymbolTable.AllSymbols)
            {
               if (symbol is Phx.Symbols.MsilTypeSymbol)
               {
                  Phx.Symbols.MsilTypeSymbol tmpSym = symbol as Phx.Symbols.MsilTypeSymbol;

                  if (tmpSym.NameString.Equals(className))
                  {
                     classTypeSym = tmpSym;
                     break;
                  }
               }
            }

            // the msil type symbol has not been created yet
            if (classTypeSym == null)
            {

               Phx.Name classTypeName = Phx.Name.New(
                     moduleUnit.Lifetime, className);

               classTypeSym = Phx.Symbols.MsilTypeSymbol.New(
                     moduleUnit.SymbolTable, classTypeName, 0);

               // Now, insert the class type to the scope of system assembly.

               assemblySymbol.InsertInLexicalScope(classTypeSym, classTypeName);

            }

            return classTypeSym;

         }
      }


      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    If the FuncPrototype is already created in the hash table,
      //    the hashed FuncPrototype is returned. Otherwise, a new instance
      //    of FuncPrototype is created according to the given function
      //    signature.
      //
      // Returns:
      //
      //    The FuncPrototype object that is according to the given function
      //    signature.
      //
      //---------------------------------------------------------------------

      private static FuncPrototype
      FindOrCreateFuncProto
      (
         Phx.PEModuleUnit moduleUnit,
         string funcSig
      )
      {
         string key = moduleUnit.NameString + funcSig;

         FuncPrototype fproto = (FuncPrototype) funcProtoFactory[key];

         if (fproto == null)
         {
            fproto = new FuncPrototype(moduleUnit, key);
            funcProtoFactory[key] = fproto;
         }

         return fproto;
      }

      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    A helper function to insert instructions that implement a call
      //    to the method represented by the given FuncPrototype
      //
      //---------------------------------------------------------------------

      private static void
      InsertCallInstr
      (
         Phx.FunctionUnit functionUnit,
         FuncPrototype fproto, // FuncPrototype for the inserted call
         Object[] arguments,      // the real argument, might be boxed
         Phx.IR.Instruction startInstruction   // Insert new instruction before this one
      )
      {
         Phx.Types.Table typeTable =
                  functionUnit.ParentUnit.TypeTable;

         if (functionUnit.FlowGraph == null)
            functionUnit.BuildFlowGraph();

         // create the call instruction
         Phx.IR.Instruction call = Phx.IR.CallInstruction.New(functionUnit,
                  Phx.Targets.Architectures.Msil.Opcode.call, fproto.functionSymbol);

         Phx.IR.Instruction ldInstr;

         // create instructions that will push arguments onto stack
         ParamType[] paramTypeList = fproto.argumentTypes;

         for (int i = 0; i < arguments.Length; i++)
         {
            Phx.IR.Operand argumentOperand;

            switch (paramTypeList[i])
            {
               case ParamType.zCHAR:
               {

                  argumentOperand = Phx.IR.ImmediateOperand.New(functionUnit, typeTable.Int32Type,
                           (char)arguments[i]);

                  ldInstr = Phx.IR.ValueInstruction.NewUnaryExpression(functionUnit,
                     Phx.Targets.Architectures.Msil.Opcode.ldc, typeTable.Int32Type,
                     argumentOperand);

                  break;
               }

               case ParamType.zINT:
               {
                  argumentOperand = Phx.IR.ImmediateOperand.New(functionUnit, typeTable.Int32Type,
                           (int)arguments[i]);

                  ldInstr = Phx.IR.ValueInstruction.NewUnaryExpression(functionUnit,
                     Phx.Targets.Architectures.Msil.Opcode.ldc, typeTable.Int32Type,
                     argumentOperand);

                  break;

               }

               case ParamType.zUINT:
               {
                  argumentOperand = Phx.IR.ImmediateOperand.New(functionUnit, typeTable.UInt32Type,
                            (uint)arguments[i]);

                  ldInstr = Phx.IR.ValueInstruction.NewUnaryExpression(functionUnit,
                     Phx.Targets.Architectures.Msil.Opcode.ldc, typeTable.UInt32Type,
                     argumentOperand);

                  break;

               }

               case ParamType.zLONG:
               {
                  argumentOperand = Phx.IR.ImmediateOperand.New(functionUnit, typeTable.Int64Type,
                           (long)arguments[i]);

                  ldInstr = Phx.IR.ValueInstruction.NewUnaryExpression(functionUnit,
                     Phx.Targets.Architectures.Msil.Opcode.ldc, typeTable.Int64Type,
                     argumentOperand);

                  break;
               }

                case ParamType.zULONG:
               {
                  argumentOperand = Phx.IR.ImmediateOperand.New(functionUnit, typeTable.UInt64Type,
                           (ulong)arguments[i]);

                  ldInstr = Phx.IR.ValueInstruction.NewUnaryExpression(functionUnit,
                     Phx.Targets.Architectures.Msil.Opcode.ldc, typeTable.UInt64Type,
                     argumentOperand);

                  break;
               }

               case ParamType.zCHARPTR:
               {
                  string newString = (string) arguments[i];

                  Phx.Types.Type stringType = typeTable.GetObjectPointerType(
                                    typeTable.SystemStringAggregateType);

                  Phx.Name newStringName = Phx.Name.New(
                        Phx.GlobalData.GlobalLifetime, newString);

                  Phx.Symbols.ConstantSymbol stringSym = Phx.Symbols.ConstantSymbol.New(
                        functionUnit.ParentUnit.SymbolTable,
                        0,
                        newStringName,
                        typeTable.SystemStringAggregateType,
                        newString);

                  argumentOperand = Phx.IR.MemoryOperand.NewAddress(functionUnit,
                        stringType, stringSym, null, 0,
                        Phx.Alignment.NaturalAlignment(stringType),
                        functionUnit.AliasInfo.NotAliasedMemoryTag);

                  ldInstr = Phx.IR.ValueInstruction.NewUnaryExpression(functionUnit,
                     Phx.Targets.Architectures.Msil.Opcode.ldstr, stringType, argumentOperand);

                  break;
               }

               default:
                  Console.WriteLine("Unsupported primitive type for parameters of callback function");
                  throw new Exception();
            }

            ldInstr.DestinationOperand.Register = Phx.Targets.Architectures.Msil.Register.SR0;

            startInstruction.InsertBefore(ldInstr);
            call.AppendSource(ldInstr.DestinationOperand);

            ldInstr.DestinationOperand.BreakExpressionTemporary();
            ldInstr.DebugTag = startInstruction.DebugTag;
         }

         startInstruction.InsertBefore(call);
      }


      override protected void
      Execute
      (
         Phx.Unit unit
      )
      {
         // unit.Dump();

         if (!unit.IsFunctionUnit && !unit.IsPEModuleUnit)
         {
            return;
         }

         if (unit.IsPEModuleUnit)
         {
            // HERE: Insert your instrumentation at the PEModule level

            // For example: instrument some initialization of the profiling
            // runtime. For this sample, no instrumentation at this point

            return;
         }

         Phx.FunctionUnit functionUnit = unit.AsFunctionUnit;

         // Only handle Msil functions.

         if (!functionUnit.Architecture.NameString.Equals("Msil"))
         {
            return;
         }

         Phx.PEModuleUnit moduleUnit = functionUnit.ParentUnit.AsPEModuleUnit;

         // HERE: insert your instrumentation at the function level
         // For this sample, nothing is instrumented at this point

         // Build a less ambiguous signature to use instead of the
         // bare method name.

         Phx.Symbols.FunctionSymbol functionSymbol = functionUnit.FunctionSymbol;
         Phx.Types.FunctionType functionType = functionSymbol.FunctionType;

         string methodName = functionSymbol.NameString + "(";
         bool first = true;
         foreach (Phx.Types.Parameter parameter in functionType.UserDefinedParameters)
         {
            if (first)
            {
               first = false;
            }
            else
            {
               methodName += ", ";
            }
            methodName += parameter.Type.ToString();
         }
         methodName += ")";

         // Build the control flow graph for the current function
         functionUnit.BuildFlowGraph();
         Phx.Graphs.FlowGraph fg = functionUnit.FlowGraph;

         // dump method info to the map file

         string className = "<Module>";
         
         if (functionSymbol.EnclosingAggregateType != null)
         {
            className = functionSymbol.EnclosingAggregateType.TypeSymbol.NameString;
         }

         logger.StartMethodMap
            (methodName, className,
             0,                 // we can't get the size of locals, yet.
             0,                 // we can't get the size of stack, yet.
             functionUnit.InstructionId,  // the size of the method (in instructions)
             fg.StartBlock.FirstInstruction.GetFileName(),
             fg.StartBlock.FirstInstruction.GetLineNumber());

         // Instrument and dump the blocks for the current function.

         logger.StartBlocksMap();

         // design_template.doc
         // Begin Snippet MainLoop

         foreach (Phx.Graphs.BasicBlock block in fg.BasicBlocks)
         {
            // Find the first real instruction in the block
            Phx.IR.Instruction firstInstruction = null;

            foreach (Phx.IR.Instruction instruction in block.Instructions)
            {
               // Set asside each instruction for later disassembly.

               funcDisassembly.Enqueue(instruction);

               // Remember the first real instruction in the block.

               if (firstInstruction == null && instruction.IsReal)
               {
                  Phx.Common.Opcode opcode = instruction.Opcode as Phx.Common.Opcode;

                  // Some real instructions aren't really real.

                  if (opcode == Phx.Common.Opcode.ReturnFinally ||
                      opcode == Phx.Common.Opcode.Leave ||
                      opcode == Phx.Common.Opcode.Unreached ||
                      opcode == Phx.Common.Opcode.ExitTypeFilter)
                  {
                     continue;
                  }

                  firstInstruction = instruction;
               }
            }

            if (firstInstruction != null)
            {
               // HERE: insert your instrumentation at BB level
               // For this sample, we insert basic block counting routines

               // dump mapping information of current basic block

               logger.DumpBlockMap(currentId, firstInstruction.GetMsilOffset(),
                  firstInstruction.GetFileName(), firstInstruction.GetLineNumber());

               // creae the FuncPrototype of the inserted call
               FuncPrototype fproto = FindOrCreateFuncProto(moduleUnit,
                  "[RtWrapper]RtWrapper.BBCount(zUINT)");

               // create the call instruction

               // set the parameter list
               Object[] arguments = { currentId };

               InsertCallInstr(functionUnit, fproto, arguments, firstInstruction);

               // Dump the disassembly for the current block.
               if (CmdLineParser.dumpPerBlockDisassembly)
               {
                  logger.StartBlockDisassemblyMap();

                  foreach (Phx.IR.Instruction instruction in block.Instructions)
                  {
                     logger.DumpDisasmMap(instruction.GetMsilOffset(),
                        instruction.ToString(), instruction.GetFileName(),
                        instruction.GetLineNumber());
                  }

                  logger.EndBlockDisassemblyMap();
               }

               logger.EndBlockMap();

               // Increment the currentId of basic blocks
               currentId++;
            }
         }

         // End Snippet MainLoop

         functionUnit.DeleteFlowGraph();

         logger.EndBlocksMap();

         // Dump the disassembly for the current function.

         logger.StartMethodDisassemblyMap();

         while (0 < funcDisassembly.Count)
         {
            Phx.IR.Instruction instruction = funcDisassembly.Dequeue() as Phx.IR.Instruction;
            logger.DumpDisasmMap(instruction.GetMsilOffset(), instruction.ToString(),
               instruction.GetFileName(), instruction.GetLineNumber());
         }

         logger.EndMethodDisassemblyMap();
         logger.EndMethodMap();
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Encode the modified function.
   //
   //--------------------------------------------------------------------------

   class EncodePhase : Phx.Phases.Phase
   {
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

      override protected void
      Execute
      (
         Phx.Unit unit
      )
      {
         Phx.PE.Writer.EncodeUnit(unit);
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Emit the modified binary...
   //
   //--------------------------------------------------------------------------

   class EmitPhase : Phx.Phases.Phase
   {
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

         string inputFileNameStr = peModule.NameString;

         string outputFileNameString = CmdLineParser.outDir +
            Path.GetFileName(inputFileNameStr);

         string outputPDBNameStr = null;

         if (CmdLineParser.pdbOut)
         {
             outputPDBNameStr = CmdLineParser.outDir +
                 Path.GetFileNameWithoutExtension(inputFileNameStr) + ".pdb";
         }

         Phx.PE.Writer writer = Phx.PE.Writer.New(
            Phx.GlobalData.GlobalLifetime,
            outputFileNameString,
            outputPDBNameStr,
            peModule,
            peModule.SymbolTable,
            peModule.Architecture,
            peModule.Runtime);

         writer.Write();
      }
   }

}

//------------------------------------------------------------------------
//
// Description:
//
//    A help class that used to factor out code for dumping out the
//    mapping information.
//
//    For now, it's a thin wrapper of a XMLTextWriter
//
//------------------------------------------------------------------------
public class Logger
{
   private XmlTextWriter mapWriter;
   private StreamWriter cntWriter;

   private string methodSrcFile;
   private uint methodSrcLine;
   private string srcFile;
   private uint srcLine;

   public Logger(string mapFile, string cntFile)
   {
      mapWriter = new XmlTextWriter(mapFile, null);
      cntWriter = new StreamWriter(File.OpenWrite(cntFile));
      methodSrcFile = "";
      methodSrcLine = 0;
      srcFile = "";
      srcLine = 0;
   }

   public void
   Close()
   {
      mapWriter.Close();
      cntWriter.Close();
   }

   public void
   StartMap()
   {
      // write out the xml header
      mapWriter.WriteStartDocument(true);
      mapWriter.Formatting = Formatting.Indented;
      mapWriter.Indentation = 3;

      mapWriter.WriteStartElement("assemblies");
   }

   public void
   EndMap()
   {
      mapWriter.WriteEndElement();  // for the <assemblies> tag
      mapWriter.WriteEndDocument();
   }

   public void
   StartAssemblyMap(string assemblyName)
   {
      mapWriter.WriteStartElement("assembly");
      mapWriter.WriteAttributeString("name", assemblyName);
   }

   public void
   EndAssemblyMap()
   {
      mapWriter.WriteEndElement(); // for the <assembly> tag
   }

   public void
   StartMethodMap
   (
      string methodName,
      string className,
      uint nLocal,
      uint nStack,
      uint nInstr,
      string file,
      uint line
   )
   {
      SetMethodSrcFileAndLine(file, line);

      mapWriter.WriteStartElement("method");

      mapWriter.WriteAttributeString("name", methodName);

      mapWriter.WriteAttributeString("class", className);

      mapWriter.WriteAttributeString("locals",
               string.Format("{0}", nLocal));

      mapWriter.WriteAttributeString("stack",
               string.Format("{0}", nStack));

      mapWriter.WriteAttributeString("size",
               string.Format("{0}", nInstr));

      WriteMethodSrcFileAndLine();
   }

   public void
   EndMethodMap()
   {
      mapWriter.WriteEndElement(); //for the <method> tag
   }

   public void
   StartBlocksMap()
   {
      mapWriter.WriteStartElement("blocks");
      WriteMethodSrcFileAndLine();
   }

   public void
   EndBlocksMap()
   {
      mapWriter.WriteEndElement(); //for the <blocks> tag
   }

   public void
   DumpBlockMap
   (
      uint id,
      uint offset,
      string file,
      uint line
   )
   {
      mapWriter.WriteStartElement("block");
      mapWriter.WriteAttributeString("id", string.Format("{0}", id));
      mapWriter.WriteAttributeString("count", string.Format("{0}", 0));
      mapWriter.WriteAttributeString("offset", string.Format("{0}", offset));
      WriteSrcFileAndLine(file, line);
   }

   public void
   EndBlockMap()
   {
      mapWriter.WriteEndElement(); //for the <block> tag
   }

   public void
   StartBlockDisassemblyMap()
   {
      mapWriter.WriteStartElement("blk-disassembler");
   }

   public void
   EndBlockDisassemblyMap()
   {
      mapWriter.WriteEndElement(); //for the <blk-disassembler> tag
   }

   public void
   StartMethodDisassemblyMap()
   {
      mapWriter.WriteStartElement("disassembly");
      WriteMethodSrcFileAndLine();
   }

   public void
   EndMethodDisassemblyMap()
   {
      mapWriter.WriteEndElement(); //for the <disassembly> tag
   }

   public void
   DumpDisasmMap
   (
      uint offset,
      string disassembler,
      string file,
      uint line
   )
   {
      char[] nl = {'\n'};
      string[] disasmLines = disassembler.Split(nl);
      for (int i = 0; i < disasmLines.Length; i += 1)
      {
         mapWriter.WriteStartElement("disassembler");
         mapWriter.WriteAttributeString("offset", string.Format("{0,5}",
            offset));
         ConditionallyWriteSrcFileAndOrLine(file, line);
         mapWriter.WriteString(disasmLines[i].Trim());
         mapWriter.WriteEndElement();
      }
   }

   // Reset the [current] method's source file and line number.

   private void
   SetMethodSrcFileAndLine(string file, uint line)
   {
      methodSrcFile = file;
      methodSrcLine = line;

      srcFile = "";
      srcLine = 0;
   }

   // Write the source file and line number attributes.

   private void
   WriteMethodSrcFileAndLine()
   {
      WriteSrcFileAndLine(methodSrcFile, methodSrcLine);
   }

   private void
   WriteSrcFileAndLine(string file, uint line)
   {
      srcFile = file;
      srcLine = line;

      mapWriter.WriteAttributeString("srcFile", string.Format("{0}", srcFile));
      mapWriter.WriteAttributeString("srcLine", string.Format("{0}", srcLine));
   }

   // Conditionally write the [current] source file and/or line number
   // attributes.

   private void
   ConditionallyWriteSrcFileAndOrLine(string file, uint line)
   {
      if (srcFile != file)
      {
         mapWriter.WriteAttributeString("srcFile", string.Format("{0}", file));
         mapWriter.WriteAttributeString("srcLine", string.Format("{0}", line));
         srcFile = file;
         srcLine = line;
      }
      else if (srcLine != line)
      {
         mapWriter.WriteAttributeString("srcLine", string.Format("{0}", line));
         srcLine = line;
      }
   }

   // Report the total number of basic blocks we visited during
   // instrumentation and put it in a file so that the profiling
   // runtime could use that for initialization
   public void
   ReportBBCnt(uint totalBBNumber)
   {
      cntWriter.Write(totalBBNumber);
   }

}

//--------------------------------------------------------------------------
//
// Description:
//
//    A helper class does command line parsing, also works as a holder of
//    some global variables
//
//    Right now, four options are parsed:
//
//       inFiles: The list of the Msil images that we want to instrument.
//                This option is required.
//
//                The intrumented Msil assemblies will be named using
//                the name of the original assembly with "-new" suffix
//                before file extension.
//
//                For example, hello.exe -> hello-new.exe
//
//       mapFile: The name of the output file that we put mapping
//                information in. By default, "bbcount.map" will be used
//
//       cntFile: The name of the output file where we keep the total
//                number of basic blocks in the input image. By default,
//                "bbcount.cnt" will be used
//
//       verbose: whether dumping information during processing
//--------------------------------------------------------------------------
public class CmdLineParser
{
   // The assembly list to be processed

   public static string inFiles;

   // The directory where we put intrumented version in

   public static string outDir;

   // The name of map file where we keep mapping information

   public static string mapFile;

   // The name of count file where we keep the total number of basic blocks

   public static string cntFile;

   // A flag used to decide whether dumping information during processing

   public static bool verbose = false;

   // A flag used to cause dumping per-block disassembly.

   public static bool dumpPerBlockDisassembly = false;

   // A flag used to decide whether to output an updated pdb

   public static bool pdbOut = false;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A control that capture the command line arguments that do not
   //    match any other controls. Here we use it to capture the list
   //    of assemblies to be instrumented.
   //
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
         if(inFiles == null)
            inFiles = localString;
         else
            inFiles = inFiles + ";" + localString;
      }
   }

   //-----------------------------------------------------------------------------
   //
   // Description:
   //
   //    Parse and check the commandline variables
   //
   // Returns:
   //
   //    Nothing
   //
   //-----------------------------------------------------------------------------

   public static Phx.Term.Mode
   ParseCmdLine
   (
       string[] argv
   )
   {
      DefaultControl defaultControl = new DefaultControl();
      Phx.Controls.Parser.RegisterDefaultControl(defaultControl);

      Phx.Controls.StringControl outdirCtrl = Phx.Controls.StringControl.New(
         "outdir", "instrumented\\",
         "the directory to put the instrumented assemblies", "bbcount.cs");

      Phx.Controls.StringControl mapCtrl = Phx.Controls.StringControl.New(
         "map", "bbcount.map", "intrument map file", "bbcount.cs");

      Phx.Controls.StringControl cntCtrl = Phx.Controls.StringControl.New(
         "cnt", "bbcount.cnt", "basic block count file", "bbcount.cs");

      Phx.Controls.SetBooleanControl verboseCtrl = Phx.Controls.SetBooleanControl.New(
         "v", "verbose flag", "bbcount.cs");

      Phx.Controls.SetBooleanControl dumpPerBlockDisassemblyCtrl =
         Phx.Controls.SetBooleanControl.New ("d", "dump per-block disassembly flag",
         "bbcount.cs");

      Phx.Controls.SetBooleanControl pdbOutCtrl = Phx.Controls.SetBooleanControl.New(
         "pdb", "output updated pdb", "bbcount.cs");

      // Check for Phoenix wide options first
      Phx.Initialize.EndInitialization("PHX|*|_PHX_|", argv);
       
      if (inFiles == null)
      {
         Usage();
         return Phx.Term.Mode.Fatal;
      } 
      
      outDir = Path.GetFullPath(outdirCtrl.GetValue(null));
      if (!outDir.EndsWith("\\"))
      {
         outDir = outDir + "\\";
      }

      DirectoryInfo outDirInfo = new DirectoryInfo(outDir);
      if (!outDirInfo.Exists)
      {
         outDirInfo.Create();
      }

      mapFile = mapCtrl.GetValue(null);
      cntFile = cntCtrl.GetValue(null);
      verbose = verboseCtrl.GetValue(null);
      pdbOut  = pdbOutCtrl.GetValue(null);
      dumpPerBlockDisassembly = dumpPerBlockDisassemblyCtrl.IsEnabled(null);

      return Phx.Term.Mode.Normal;
   }

   //-----------------------------------------------------------------------------
   //
   // Description:
   //
   //    Print the usage string to the console
   //
   // Returns:
   //
   //    Nothing
   //
   //-----------------------------------------------------------------------------

   static void
   Usage()
   {
      string usage =
       @"Usage: BBCount [/outdir <output-file-name>] [/pdb] [/map <map-name>]
               [/cnt <count-name>] [/v] [/d] <src-assembly-list>

       Perform basic-block instrumentation on an Msil image.
       Arguments in [] are optional.

       /v:   verbose
       /d:   dump per-block disassembly
       /outdir: directory to hold the instrumented Msil image
             Default: current directory plus \instrumented
       /pdb: output an updated pdb to the output directory
       /map: name of map where you want to keep the mapping information.
             Default: bbcount.map
       /cnt: specify the file where you want to keep the total number
             of basic blocks.
             Default: bbcount.cnt";

      Console.WriteLine(usage);
   }

}

public class BBCount
{

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Entry point for application
   //
   // Remarks:
   //
   //    The order of initialization matters. Please initializing
   //    Phoenix first, then parsing command line, then initializing
   //    the Instrumentor, and finally do the real work
   //
   //--------------------------------------------------------------------------

   public static int
   Main
   (
      String[] argv      // command line argument strings
   )
   {
      Phx.Term.Mode termMode = Phx.Term.Mode.Normal;

#if F
      try
#endif
      {
         // (1) -- Initializing the Phoenix framework.
         // Always do this before anything else !!

         Instrumentor.InitializePhx();

         // (2) -- Parsing command line options

         termMode = CmdLineParser.ParseCmdLine(argv);

         if (termMode == Phx.Term.Mode.Fatal)
         {
            Phx.Term.All(termMode);
            return (termMode == Phx.Term.Mode.Normal ? 0 : 1);
         }

         // (3) -- Create an instance of Logger class that will be used
         //        for dumping out mapping information

         Logger logger = new Logger(CmdLineParser.mapFile,
                  CmdLineParser.cntFile);

         // (4) -- Initialize the Intrumentor

         Instrumentor.Initialize(logger);


         // (5) -- Do the real intrumentation work

         // process the input assemblies one by one

         logger.StartMap();

         char[] delim = { ';' };
         string[] images = CmdLineParser.inFiles.Split(delim);
         for (int i = 0; i < images.Length; i++)
         {
            Instrumentor.Process(images[i]);
         }

         logger.EndMap();

         // (6) -- Report the total number of basic blocks processed
         uint totalBBNumber = Instrumentor.InstrumentPhase.currentId;
         if(CmdLineParser.verbose)
            Console.WriteLine("There are {0} basic blocks total.",
                  totalBBNumber);

         logger.ReportBBCnt(totalBBNumber);

         logger.Close();

      }
#if F
      catch (Exception e)
      {
         Console.WriteLine("Unhandled Exception: {0}", e);
         termMode = Phx.Term.Mode.Fatal;
      }
#endif
      // clean up Phx and terminate

      Phx.Term.All(termMode);
      return (termMode == Phx.Term.Mode.Normal ? 0 : 1);

   }
}
