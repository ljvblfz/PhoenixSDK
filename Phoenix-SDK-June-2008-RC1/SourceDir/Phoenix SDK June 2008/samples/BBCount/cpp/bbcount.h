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
//
// Remarks:
//
//    This sample provides some more general interfaces that intend to
//    simplify instrumenting Msil modules. BBCount is just one
//    example that shows how to use these interfaces.
//
//--------------------------------------------------------------------------

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

public
ref class Logger
{
public:
   Logger(String ^ mapFile, String ^ cntFile);

   void
   Close();

   void
   StartMap();

   void
   EndMap();

   void
   StartAssemblyMap(String ^ assemblyName);

   void
   EndAssemblyMap();

   void
   StartMethodMap
   (
      String ^     methodName,
      String ^     className,
      unsigned int nLocal,
      unsigned int nStack,
      unsigned int nInstr,
      String ^     file,
      unsigned int line
   );

   void
   EndMethodMap();

   void
   StartBlocksMap();

   void
   EndBlocksMap();

   void
   DumpBlockMap
   (
      unsigned int id,
      unsigned int offset,
      String ^     file,
      unsigned int line
   );

   void
   EndBlockMap();

   void
   StartBlockDisassemblyMap();

   void
   EndBlockDisassemblyMap();

   void
   StartMethodDisassemblyMap();

   void
   EndMethodDisassemblyMap();

   void
   DumpDisasmMap
   (
      unsigned int offset,
      String ^     disassembler,
      String ^     file,
      unsigned int line
   );

   // Report the total number of basic blocks we visited during
   // instrumentation and put it in a file so that the profiling
   // runtime could use that for initialization

   void
   ReportBBCnt(unsigned int totalBBNumber);

private:
   XmlTextWriter ^ mapWriter;
   StreamWriter ^  cntWriter;

   String ^     methodSrcFile;
   unsigned int methodSrcLine;
   String ^     srcFile;
   unsigned int srcLine;

   // Reset the [current] method's source file and line number.

   void
   SetMethodSrcFileAndLine(String ^ file, unsigned int line);

   // Write the source file and line number attributes.

   void
   WriteMethodSrcFileAndLine();

   void
   WriteSrcFileAndLine(String ^ file, unsigned int line);

   // Conditionally write the [current] source file and/or line number
   // attributes.

   void
   ConditionallyWriteSrcFileAndOrLine(String ^ file, unsigned int line);
};

public
ref class Instrumentor
{
public:

   static void
   InitializePhx();

   static void
   Initialize(Logger ^ l);

   // The main entry for processing one assembly

   static void
   Process(String ^ inFile);

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    This class is responsible for the instrumentation
   //
   //--------------------------------------------------------------------------

   ref class InstrumentPhase : Phx::Phases::Phase
   {
   public:

      // Initialization of the static members of InstrumentPhase class

      static void
      Initialize();

      //------------------------------------------------------------------------
      //
      // Description:
      //
      //    The static constructor of InstrumentPhase
      //
      //-----------------------------------------------------------------------

      static InstrumentPhase ^
      New
      (
         Phx::Phases::PhaseConfiguration ^ config,
         Logger ^                   logger
      );

      // unique ID assigned to each basic block

      static unsigned int currentId;

      // A queue of instructions from the current function

      static Queue ^ funcDisassembly;

      //-----------------------------------------------------------------------
      // The allowed types for parameters of the instrumented call
      //-----------------------------------------------------------------------

      enum class ParamType
      {
         zCHAR,
         zINT,
         zUINT,
         zLONG,
         zULONG,
         zCHARPTR,

         //TODO: more?

      };

   private:

#if (PHX_DEBUG_SUPPORT)

      static Phx::Controls::ComponentControl ^ InstrumentPhaseControl;

#endif

      // HERE: put static data structures that are common for instrumenting
      // both ModuleUnit and FunctionUnit


      // A hashtable of call prototypes that are already created

      static Hashtable ^ funcProtoFactory;

      // The logger used to dump out map information

      Logger ^ logger;

      //----------------------------------------------------------------------
      //
      // Description:
      //
      //    Provide interfaces to import a method reference to a Msil module
      //
      //----------------------------------------------------------------------

      ref class FuncPrototype
      {
      public:
         Phx::Symbols::FunctionSymbol ^ functionSymbol;

         // The type of the argument list, from left to right

         array<ParamType ^> ^ argumentTypes;

         FuncPrototype
         (
            Phx::PEModuleUnit ^ module,
            String ^            funcSig
         );

      private:
         Phx::Symbols::FunctionSymbol ^
         CreateFunctionSymbol
         (
            Phx::PEModuleUnit ^         moduleUnit,
            String ^                    assemblyName,
            String ^                    className,
            String ^                    methodName,
            array<Phx::Types::Type ^> ^ argumentTypeList
         );

         Phx::Symbols::AssemblySymbol ^
         FindOrCreateAssemblySym
         (
            Phx::ModuleUnit ^ moduleUnit,
            String ^          assemblyName
         );

         Phx::Symbols::MsilTypeSymbol ^
         FindOrCreateClassSym
         (
            Phx::ModuleUnit ^        moduleUnit,
            String ^                 className,
            Phx::Symbols::AssemblySymbol ^ assemblySymbol
         );
      };

      static FuncPrototype ^
      FindOrCreateFuncProto
      (
         Phx::PEModuleUnit ^ moduleUnit,
         String ^            funcSig
      );

      static void
      InsertCallInstr
      (
         Phx::FunctionUnit ^           functionUnit,
         FuncPrototype ^           fproto,

         // FuncPrototype for the inserted call

         array<System::Object ^> ^ arguments,

         // the real argument, might be boxed

         Phx::IR::Instruction ^          startInstruction

         // Insert new instruction before this one

      );

   protected:

      virtual void
      Execute
      (
         Phx::Unit ^ unit
      ) override;
   };

private:

   // A global logger that is used by different phases

   static Logger ^ logger;

   // The assembly is being processed

   static String ^ currentAssembly;

   static Phx::Phases::PhaseConfiguration ^
   PreparePhaseList(Phx::Lifetime ^ lifetime);

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Encode the modified function.
   //
   //--------------------------------------------------------------------------

   ref class EncodePhase : Phx::Phases::Phase
   {
   public:
      static EncodePhase ^
      New
      (
         Phx::Phases::PhaseConfiguration ^ config
      );

   protected:
      virtual void
      Execute
      (
         Phx::Unit ^ unit
      ) override;
   };

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Emit the modified binary...
   //
   //--------------------------------------------------------------------------

   ref class EmitPhase : Phx::Phases::Phase
   {
   public:
      static EmitPhase ^
      New
      (
         Phx::Phases::PhaseConfiguration ^ config
      );

   protected:
      virtual void
      Execute
      (
         Phx::Unit ^ unit
      ) override;
   };
};

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

public
ref class CmdLineParser
{
public:

   // The assembly list to be processed

   static String ^ inFiles;

   // The directory where we put intrumented version in

   static String ^ outDir;

   // The name of map file where we keep mapping information

   static String ^ mapFile;

   // The name of count file where we keep the total number of basic blocks

   static String ^ cntFile;

   // A flag used to decide whether dumping information during processing

   static bool verbose = false;

   // A flag used to cause dumping per-block disassembly.

   static bool dumpPerBlockDisassembly = false;

   // A flag used to decide whether to output an updated pdb

   static bool pdbOut = false;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A control that capture the command line arguments that do not
   //    match any other controls. Here we use it to capture the list
   //    of assemblies to be instrumented.
   //
   //--------------------------------------------------------------------------

   ref class DefaultControl : Phx::Controls::DefaultControl
   {
   public:
      virtual void
      Process
      (
         String ^ string
      ) override;
   };

   static Phx::Term::Mode
   ParseCmdLine
   (
      array<String ^> ^ argv
   );

   static void
   Usage();
};
