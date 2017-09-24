// FrontEnd.cpp : main project file.

#include "stdafx.h"
#include <string.h>
#include <stdlib.h>

using namespace System;

#include "Scanner.h"
#include "Parser.h"
#include "Ast.h"
#include "VisitTracker.h"
#include "PrettyPrinter.h"
#include "Evaluator.h"
#include "Phases.h"

using namespace Pascal;

extern bool InitScanner(char* file_name);

// Delegate for StringControl broadcast messages.

delegate void ProcessStringControl(String ^ command);

// Handler class for StringControl objects.
// We'll use this to handle the "default" string control.
// Strings passed on the command-line that are otherwise
// unrecognized will be treated as input file names.

ref class StringControl : public Phx::Controls::DefaultControl
{
public:
   StringControl
   (
      ProcessStringControl ^ callback
   )  : callback(callback)
   {
   }

   virtual void Process(String ^ s) override
   {
      this->callback(s);
   }

private:
   ProcessStringControl ^ callback;
};

// The main program class.

ref class FrontEnd
{
public:

   static int Main(int argc, char** argv)
   {
      // Parse command-line arguments, skipping program name.

      array<String^>^ args = gcnew array<String^>(argc-1); 
      for (int i = 1; i < argc; i++)
      {
         args[i-1] = gcnew String(argv[i]);
      }

      // We have to check for the /clr flag before initializing Phoenix,
      // since the initialization of Phoenix depends on this value.

      Pascal::Configuration::IsManaged = false;
      for each (String ^ arg in args)
      {
         arg = arg->ToLower();
         if (arg->Equals("/clr") || arg->Equals("-clr"))
            // Treat this flag like a standard Phoenix control; each
            // instance of the control negates the current value.
            Pascal::Configuration::IsManaged = 
               !Pascal::Configuration::IsManaged;
      }            

      // Initialize the Phoenix library for code generation.
           
      InitializePhoenix(args);

      // Ensure we received at least one source file to process.
      
      if (fileNames == nullptr || fileNames->Count == 0)
      {
         try
         {
            Output::ReportFatalError(Error::EmptyFileList);
         }
         catch (FatalErrorException ^)
         {
            // ReportFatalError throws this exception by default;
            // just catch it and do nothing.
         }

         // Report usage to the user and exit.

         Usage();
         return Exit(-1);
      }

      // Process each source file.

      int totalErrors = 0;
      int totalWarnings = 0;

      for each (String ^ fileName in fileNames)
      {
         ModuleBuilder::AddSourceFilePath(fileName);
         Output::CurrentSourceFileName = fileName;

         // Create a new context for the current source file.

         ModuleBuilder::NewContext();

         // Process the file.

         Phx::ModuleUnit ^ moduleUnit = ProcessFile(fileName);

         // Write object file if compilation succeeded.

         if (! noCompile->GetValue(nullptr) && Output::ErrorCount == 0)
         {            
            WriteObjectFile(
               moduleUnit,
               fileName, 
               Path::ChangeExtension(fileName, ".obj")
            );
         }

         // Output error/warning counts.

         Output::ReportMessage(
            String::Format("\r\n{0} - {1} error(s), {2} warning(s).", 
               Path::GetFileName(fileName), 
               Output::ErrorCount, 
               Output::WarningCount
            )
         );

         // Increment total counts.

         totalErrors += Output::ErrorCount;
         totalWarnings += Output::WarningCount;

         // Reset error, warning counts for next file.

         Output::ErrorCount = 0;
         Output::WarningCount = 0;
      }

      // Build the final executable if no errors were 
      // encountered and the compilation unit is a program 
      // (e.g. not just a collection of modules).

      if (totalErrors == 0 && 
         ! noCompile->GetValue(nullptr) && 
         ! compileOnly->GetValue(nullptr) && 
          Pascal::Configuration::IsProgram)
      {     
         String ^ outputFileName = 
            Path::ChangeExtension(Pascal::Configuration::ProgramName, ".exe");

         totalErrors +=
            !! WriteExecutableFile(
               outputFileName,
               debugMode->GetValue(nullptr)
            );
        
         // Output total error/warning counts.

         Output::ReportMessage(
            String::Format("\r\n{0} - {1} error(s), {2} warning(s).", 
            outputFileName,
            totalErrors, 
            totalWarnings            
            )
         );
      }      

      return Exit(-totalErrors);
   }

private:
   
   static void 
   WriteObjectFile
   (
      Phx::ModuleUnit ^ moduleUnit,
      String ^ sourceFileName,
      String ^ targetFileName
   )
   {
      Phx::Targets::Runtimes::Runtime ^ runtime =
         Phx::GlobalData::GetFirstTargetRuntime();

      // Prepare to write the object file.

      Phx::Coff::ObjectWriter ^ objectWriter = Phx::Coff::ObjectWriter::New(
         Phx::GlobalData::GlobalLifetime, 
         moduleUnit, 
         targetFileName, 
         sourceFileName, 
         runtime->Architecture, 
         0, 
         0, 
         false
      );
   
      // Write the object.
   
      Output::ReportMessage(String::Format(
         "Writing {0}...", 
         Path::GetFileName(targetFileName)
         )
      );
      
      objectWriter->Write();

      // Add the target filename to the list of object files.

      objectFiles->Add(targetFileName);
   }

   static int 
   WriteExecutableFile
   (
      String ^ outputFileName,
      bool debug
   )
   {
      // Collect the names of each object file in the final image.

      Text::StringBuilder ^ builder = gcnew Text::StringBuilder();
      for each (String ^ objectFile in objectFiles)
      {
         builder->AppendFormat("\"{0}\" ", objectFile);
      }

      // Set the linker output path.

      String ^ outFile = Path::Combine(
         outpath->GetValue(nullptr),
         outputFileName
         ); 

      // Append dependent runtime libraries.

      builder->Append("mspvcrt.lib Kernel32.lib User32.lib ");

      // Suppress linker startup banner.

      builder->Append("/NOLOGO ");
      
      // Specifiy /OUT path.
      // Also specify /DEBUG and /PDB options for Debug builds.

      builder->AppendFormat("/OUT:\"{0}\"", outFile);

      if (debug)
      {
         builder->AppendFormat(" /PDB:\"{0}\"", 
            Path::ChangeExtension(outFile, ".pdb")
         );
      }
      
      // Execute link.exe.

      Output::ReportMessage(Environment::NewLine + "Linking...");

      int exitCode = Utility::ExecuteProcess(
         "link.exe", 
         builder->ToString(),
         true
      );

      return exitCode;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Builds up a phase list for compilation.
   //
   // Returns:
   //
   //    Phase list to use in processing each function.
   //
   //--------------------------------------------------------------------------

   static Phx::Phases::PhaseConfiguration ^
   BuildPhaseList()
   {
      Phx::Targets::Runtimes::Runtime ^ runtime =
         Phx::GlobalData::GetFirstTargetRuntime();

      // When we get around to generating IR, we'll need to gradually
      // transform it into x86 machine code. Phx provides a number of
      // phases to help in this task. The phases are strung together
      // in a list, which we'll now build.

      Phx::Phases::PhaseConfiguration ^ configuration = 
         Phx::Phases::PhaseConfiguration::New(
            Phx::GlobalData::GlobalLifetime,
            "Phase Configuration"
         );
      Phx::Phases::PhaseList ^ phaseList = configuration->PhaseList;

      // Optimize each instruction in the function unit if the /Oe flag
      // was provided on the command-line.

      OptimizationPhase ^ optimizationPhase = nullptr;
      if (optimizeExpressions->GetValue(nullptr))
      {
         optimizationPhase = OptimizationPhase::New(configuration); 
      }

      array<Phx::Phases::Phase ^> ^ phases = {
         optimizationPhase,
         Phx::Types::TypeCheckPhase::New(configuration),
         Phx::MirLowerPhase::New(configuration),         
         Phx::Targets::Runtimes::CanonicalizePhase::New(configuration),
         Phx::Targets::Runtimes::LowerPhase::New(configuration),
         Phx::RegisterAllocator::PriorityOrder::
            GlobalRegisterAllocatorPhase::New(configuration),
         Phx::StackAllocatePhase::New(configuration),
         Phx::Targets::Runtimes::FrameGenerationPhase::New(configuration),
         Phx::Targets::Runtimes::SwitchLowerPhase::New(configuration),
         Phx::Graphs::BlockLayoutPhase::New(configuration),
         Phx::FlowOptimizer::Phase::New(configuration),
      };

      for each (Phx::Phases::Phase ^ phase in phases)
      {
         if (phase != nullptr)
            phaseList->AppendPhase(phase);
      }

      // Add-in custom phases.

      phaseList->AppendPhase(Encode::New(configuration));
      phaseList->AppendPhase(Lister::New(configuration));

      // Add in any target-specific phases (eg x87 register allocation).

      runtime->AddPhases(configuration);

      // Add in any plugin phases.

      Phx::GlobalData::BuildPlugInPhases(configuration);

      return configuration;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Run the phases on a function unit.
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   static void
   ExecutePhases
   (
      Phx::Unit ^ functionUnit
   )
   {
      if (phaseConfig == nullptr)
         phaseConfig = BuildPhaseList();

      Phx::Threading::Context ^ context = Phx::Threading::Context::GetCurrent();
      context->PushUnit(functionUnit);

      phaseConfig->PhaseList->DoPhaseList(functionUnit);

      context->PopUnit();
   }

   static Phx::ModuleUnit ^
   ProcessFile
   (
      String ^ fileName
   )
   {
      // lex and yacc require native C-strings.

      int bufferSize = 0;
      char * path = NULL;

      // First call Utf8Encode with a 0-sized buffer to get
      // the required buffer length.

      if (! Utility::Utf8Encode(fileName, path, bufferSize))
      {         
         Output::ReportError(0, Error::InvalidFile, fileName);
         return nullptr;
      }

      // Allocate memory for the buffer and call Utf8Encode
      // again.

      path = new char[bufferSize];

      if (! Utility::Utf8Encode(fileName, path, bufferSize))
      {  
         Output::ReportError(0, Error::InvalidFile, fileName);
         delete[] path;
         return nullptr;
      }      

      // Point yyin to the input stream.
      
      if (! InitScanner(path))
      {
         Output::ReportError(0, Error::InvalidFile, fileName);
         delete[] path;
         return nullptr;
      }
      
      delete[] path;

      // Parse the input.

      Ast::Node ^ astRoot = Parse();
      
      if (astRoot != nullptr)
      {        
         TextWriter ^ writer = Output::MessageWriter;
         if (writer == nullptr)
           writer = Output::DefaultWriter;

         try
         {            
            // If /v was supplied on the command-line, visit the AST and
            // track visits.

            if (reportVisitation->GetValue(nullptr))
            {
               VisitTracker^ tracker = gcnew VisitTracker();
               astRoot->Accept(tracker);
               tracker->PrintUsage(writer);
            }

            // If /p was supplied on the command-line, visit the AST and
            // pretty-print the source listing.

            if (printListing->GetValue(nullptr))
            {  
               PrettyPrinter^ printer = gcnew PrettyPrinter(writer);
               astRoot->Accept(printer);               
            }

            // Generate the intermediate representation (IR) stream for 
            // the current source file.

            if( ! noCompile->GetValue(nullptr))
            {
               Output::ReportMessage(
                  String::Format("Compiling...{0}{1}", 
                     Environment::NewLine, Path::GetFileName(fileName)
                  )
               );

               Evaluator^ evaluator = gcnew Evaluator(fileName);            
               astRoot->Accept(evaluator);
            
               // If no errors were reported, execute the phase list for 
               // each FunctionUnit in the current module.

               if (Output::ErrorCount == 0)
               {               
                  for each (Phx::FunctionUnit ^ functionUnit 
                     in ModuleBuilder::FunctionUnits)
                  {
                     if (functionUnit == nullptr)
                        continue;

                     // Skip this function unit if it was marked as "external".

                     DirectiveType directive = 
                        ModuleBuilder::GetDirectiveType(functionUnit);
                     if (directive == DirectiveType::External)
                        continue;

                     ExecutePhases(functionUnit);
                  }
               
                  // Return the ModuleUnit created by the evaluator.

                  return evaluator->ModuleUnit;
               }
            }
         }
         catch (FatalErrorException ^)
         {
            return nullptr;
         }
      }      
      return nullptr;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    
   //
   // Remarks:
   //    
   //
   //--------------------------------------------------------------------------

   ref class ControlComparer: IComparer<Phx::Controls::Control^>
   {
   public:
      virtual int Compare
      (
         Phx::Controls::Control ^ x, 
         Phx::Controls::Control ^ y
      )
      {
         return x->NameString->CompareTo(y->NameString);
      }
   };

   static void Usage()
   {
      array<Phx::Controls::Control ^> ^ controls = { 
         compileOnly,
         noCompile,
         reportVisitation,
         printListing,
         optimizeExpressions,
         debugMode,
         clr,
      };

      array<Phx::Controls::Control ^>::Sort(
         controls, 
         gcnew ControlComparer()
      );
      
      Text::StringBuilder ^ builder = gcnew Text::StringBuilder();
      
      builder->Append(Environment::NewLine);
      builder->Append("Usage: msp.exe ");
      
      for each (Phx::Controls::Control ^ control in controls)
      {
         builder->AppendFormat("[/{0}] ", control->NameString);
      }
      builder->AppendFormat("<source-file(s)>{0}", Environment::NewLine);

      builder->AppendFormat("{0}"
         "Phoenix Pascal Compiler.{0}"
         "Arguments in [] are optional.{0}"    
         "{0}",
         Environment::NewLine
      );

      for each (Phx::Controls::Control ^ control in controls)
      {
         builder->AppendFormat("/{0}\t- {1}{2}",
            control->NameString, control->DescriptionString, 
            Environment::NewLine
         );
      }

      Console::WriteLine(builder->ToString());
   }

   static int Exit(int exitCode)
   {
      if (System::Diagnostics::Debugger::IsAttached)
      {
         System::Console::Write(
            "{0}Press any key to exit: ", Environment::NewLine);
         System::Console::ReadKey(true);
      }
      return exitCode;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Perform framework initialization prior to parsing.
   //
   // Remarks:
   // 
   //--------------------------------------------------------------------------

   static void InitializePhoenix(array<String ^> ^argv)
   {
      Phx::Targets::Architectures::Architecture ^ architecture;
      Phx::Targets::Runtimes::Runtime ^ runtime;

      if (Pascal::Configuration::IsManaged)
      {
         architecture = 
         Phx::Targets::Architectures::Msil::Architecture::New();
         runtime = 
         Phx::Targets::Runtimes::Vccrt::Win::Msil::Runtime::New(architecture);
      }
      else
      {
         architecture = 
         Phx::Targets::Architectures::X86::Architecture::New();
         runtime = 
         Phx::Targets::Runtimes::Vccrt::Win32::X86::Runtime::New(architecture);
      }

      Phx::GlobalData::RegisterTargetArchitecture(architecture);
      Phx::GlobalData::RegisterTargetRuntime(runtime);

      Phx::Initialize::BeginInitialization();
      
      // Register custom controls for the compiler.

      RegisterControls();

      // EndInitialization completes control initialization and plugin
      // registration. It is necessary to complete control initialization 
      // to be able to create module units.

      Phx::Initialize::EndInitialization("PHX|*|_PHX_", argv);

      // Enable some controls. We want to see the names of types, types
      // in our IR dumps, linenumbers in our IR dumps, and linenumbers
      // in our object file.

      Phx::Controls::Parser::ParseArgumentString(nullptr,
           "-dumptypesym -dump:types -dump:linenumbers -lineleveldebug");
   }

   static void RegisterControls()
   {
      // We want to supply a plain filename on the command line and not
      // have to prepend Phoenix control syntax (e.g. "-input:source.p");
      // to do this we create a default control that processes any
      // command-line arguments that Phoenix doesn't recognize as a control.
      
      Phx::Controls::Parser::RegisterDefaultControl(
         gcnew StringControl(
            gcnew ProcessStringControl(&AddFilename)
         )
      );

      // Boolean control to compile without linking.

      compileOnly = Phx::Controls::SetBooleanControl::New(
         "c",
         "Compile without linking",
         "Pascal compiler"
         );

      // Boolean control to disable compilation.

      noCompile = Phx::Controls::SetBooleanControl::New(
         "nc",
         "Skip compilation",
         "Pascal compiler"
         );

      // Boolean control to report visitation tracking.

      reportVisitation = Phx::Controls::SetBooleanControl::New(
         "v",
         "Report visitation tracking",
         "Pascal compiler"
      );

      // Boolean control to print source listing.

      printListing = Phx::Controls::SetBooleanControl::New(
         "p",
         "Print source listing",
         "Pascal compiler"
      );
     
      // Boolean control to enable optimizations.

      optimizeExpressions = Phx::Controls::SetBooleanControl::New(
         "Oe",
         "Enable expression optimization",
         "Pascal compiler"
      );

      // Boolean control to emit debug information.

      debugMode = Phx::Controls::SetBooleanControl::New(
         "d",
         "Generate debug information",
         "Pascal compiler"
      );

      // Boolean control to enable the common language runtime (CLR)

      clr = Phx::Controls::SetBooleanControl::New(
         "clr", 
         "Enable common language runtime support (not yet implemented)",
         "Pascal compiler"
      );

      // String control to override the default output path.

      outpath = Phx::Controls::StringControl::New(
         "outpath:",
         "Overrides the output path",
         "Pascal compiler"
      );
   }

   static void AddFilename(String ^ fileName)
   {
      fileNames->Add(fileName);
   }

private:

   static List<String ^> ^ fileNames = gcnew List<String ^>();

   static Phx::Controls::SetBooleanControl ^ compileOnly;
   static Phx::Controls::SetBooleanControl ^ noCompile;
   static Phx::Controls::SetBooleanControl ^ reportVisitation;
   static Phx::Controls::SetBooleanControl ^ printListing;
   static Phx::Controls::SetBooleanControl ^ debugMode;
   static Phx::Controls::SetBooleanControl ^ optimizeExpressions;
   static Phx::Controls::SetBooleanControl ^ clr;
   static Phx::Controls::StringControl     ^ outpath;    

   static Phx::Phases::PhaseConfiguration ^ phaseConfig;

   static List<String ^> ^ objectFiles = gcnew List<String ^>();
};

int
main
(
  int   argc,
  char* argv[]
)
{
   int retVal = -1;
   Phx::Term::Mode termMode = Phx::Term::Mode::Normal;

   // In Release mode, catch any unhandled exceptions and process them 
   // as a "fatal error". Is prevents the application from appearing to "crash".

#ifndef _DEBUG

   try
   {
      retVal = FrontEnd::Main(argc, argv);
   }

   // Catch block for unhandled managed exceptions.

   catch (Exception ^ e)
   {
      try
      {
         Output::ReportFatalError(
            -1,
            Error::InternalCompilerError,
            e->Message, 
            e->StackTrace, 
            Environment::NewLine
         );
      }
      catch (FatalErrorException ^)
      {
      }
      termMode = Phx::Term::Mode::Fatal;
   }

   // Catch block for unhandled native exceptions.

   catch (...)
   {
      try
      {
         Output::ReportFatalError(Error::UnknownInternalCompilerError);
      }
      catch (FatalErrorException ^)
      {
      }      
      termMode = Phx::Term::Mode::Fatal;
   }
  
#else

   // In Debug mode, don't catch unhandled exceptions; this allows us 
   // to attach a debugger at the call site.

   retVal = FrontEnd::Main(argc, argv);

#endif


   Phx::Term::All(termMode);
   return retVal;
}
