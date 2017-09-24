//------------------------------------------------------------------------------
//
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
//           Console::WriteLine("Hello, World!");
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
//------------------------------------------------------------------------------

#pragma once

#pragma region Using namespace declarations

using namespace System;
using namespace System::IO;
using namespace System::Collections;

#pragma endregion

//------------------------------------------------------------------------------
//
// Description:
//
//    This class is responsible for the instrumentation of each method.
//
//------------------------------------------------------------------------------

public ref class
   InstrumentPhase : public Phx::Phases::Phase
{
public:

   static void
   Initialize();

   static InstrumentPhase ^
   New
   (
      Phx::Phases::PhaseConfiguration ^ config
   );

   Phx::Symbols::AssemblySymbol ^
   LookupAssembly
   (
      Phx::ModuleUnit ^ moduleUnit,
      String ^          assemblyNameString
   );

   void
   ImportTypes
   (
      Phx::ModuleUnit ^ moduleUnit
   );

protected:

   virtual void
   Execute
   (
      Phx::Unit ^ unit
   ) override;

   void
   InstrumentAfter
   (
      Phx::IR::Instruction ^ instruction,
      String ^         messageString
   );

private:

   //------------------------------------------------------------------------
   // 
   // Description:
   //
   //    Cached FunctionSymbol for 
   //    [System]System::Diagnostic::Trace::WriteLine(string).
   //
   //------------------------------------------------------------------------

   Phx::Symbols::FunctionSymbol ^ traceFuncSym;

#if (PHX_DEBUG_SUPPORT)
   static Phx::Controls::ComponentControl ^ InstrumentPhaseControl;
#endif
};

//--------------------------------------------------------------------------
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
//--------------------------------------------------------------------------

ref class EncodePhase : public Phx::Phases::Phase
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
// Remarks:
//
//    This phase writes out the instrumented binary.
//
//--------------------------------------------------------------------------

public ref class EmitPhase : public Phx::Phases::Phase
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

public
ref class Driver
{
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Name of the executable file to process.
   //
   //--------------------------------------------------------------------------

   String ^ filename;

public:

   //--------------------------------------------------------------------------
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

   ref class DefaultControl : public Phx::Controls::DefaultControl
   {
   public:

      DefaultControl
      (
         Driver ^ driver
      );

      virtual void
      Process
      (
         String ^ string
      ) override;

   private:

      Driver ^ driver;
   };

   static void
   Usage();

   Phx::Term::Mode
   Process
   (
      array<String ^> ^ arguments
   );

   void
   DoInstrumentation();

   void
   DoConfigFile();
};
