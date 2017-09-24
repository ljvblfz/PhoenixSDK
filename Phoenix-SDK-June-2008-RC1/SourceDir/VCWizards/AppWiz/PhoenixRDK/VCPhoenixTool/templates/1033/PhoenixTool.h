//-----------------------------------------------------------------------------
//
// Description:
//
//    A standalone static analysis/instrumentation tool
//
// Usage:
//
[!if INSTRUMENTATION]
//    [!output PROJECT_NAME] /out <filename> [/pdbout <filename>] 
//    /in <image-name> [/pdbin <pdb-name>]
[!else]
//    [!output PROJECT_NAME] /in <image-name> [/pdbin <pdb-name>]
[!endif]
//
//-----------------------------------------------------------------------------

#pragma once

#pragma region Using namespace declarations

using namespace System;
using namespace System::IO; 
using namespace System::Collections; 

#pragma endregion

[!if INSTRUMENTATION]
//-----------------------------------------------------------------------------
//
// Description:
//
//    This class is responsible for the instrumentation of each method.
//
//-----------------------------------------------------------------------------

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

protected:

   virtual void
   Execute
   (
      Phx::Unit ^ unit
   ) override;

private:

#if defined(PHX_DEBUG_SUPPORT)
   static Phx::Controls::ComponentControl ^ InstrumentPhaseControl;
#endif
};

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

ref class 
EncodePhase : public Phx::Phases::Phase
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

//-----------------------------------------------------------------------------
//
// Description:
//
//    Emit the modified binary...
//
// Remarks:
//
//    This phase writes out the instrumented binary.
//
//-----------------------------------------------------------------------------

public ref class 
EmitPhase : public Phx::Phases::Phase
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

[!else]

//-----------------------------------------------------------------------------
//
// Description:
//
//    Perform static analysis on the units in the binary.
//
//-----------------------------------------------------------------------------

public ref class 
StaticAnalysisPhase : public Phx::Phases::Phase
{
public:

   static void 
   Initialize();

   static StaticAnalysisPhase ^
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

private:

#if (PHX_DEBUG_SUPPORT)
      static Phx::Controls::ComponentControl ^ StaticAnalysisPhaseCtrl;
#endif
};

[!endif]

public ref class 
Driver
{

public:

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Name of the executable file to process.
   //
   //--------------------------------------------------------------------------
   
   static Phx::Controls::StringControl ^ in;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Name of the output binary.
   //
   //--------------------------------------------------------------------------
   
   static Phx::Controls::StringControl ^ out;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Name of the output pdb.
   //
   //--------------------------------------------------------------------------
   
   static Phx::Controls::StringControl ^ pdbout;

   static void
   StaticInitialize(array<String ^> ^ arguments);
      
   static void 
   Usage();

private:

   static bool 
   CheckCommandLine();

   static void
   InitializeCommandLine();

   static void
   InitializeTargets();

public:
   
   Phx::Term::Mode
   Process();
};