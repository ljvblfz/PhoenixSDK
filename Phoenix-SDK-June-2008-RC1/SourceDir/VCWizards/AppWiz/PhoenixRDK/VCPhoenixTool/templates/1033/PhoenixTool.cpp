//-----------------------------------------------------------------------------
//
// Description:
//
//    A standalone static analysis/instrumentation tool
//
// Usage:
//
[!if INSTRUMENTATION]
//    [!output PROJECT_NAME] /out <filename> /pdbout <filename>
//    /in <image-name>
[!else]
//    [!output PROJECT_NAME] /in <image-name>
[!endif]
//
//-----------------------------------------------------------------------------

#pragma region Using namespace declarations

using namespace System;

#pragma endregion
   
#include "[!output SAFE_PROJECT_NAME].h"

[!if INSTRUMENTATION]
//-----------------------------------------------------------------------------
//
// Description:
//
//    Initialize the instrumentation phase.
//
// Remarks:
//
//    Sets up a component control under debug builds, so that we
//    can dump IR before/after the phase and so on. In other
//    words, because we create this control, you can pass in
//    options like -predumpmtrace that will dump the IR for each
//    method before this phase runs.
//
//-----------------------------------------------------------------------------

void 
InstrumentPhase::Initialize()
{
#if defined(PHX_DEBUG_SUPPORT)
   InstrumentPhase::InstrumentPhaseControl = Phx::Controls::ComponentControl::New("root",
      "Inject instrumentation", "[!output PROJECT_NAME].cpp");
#endif
}

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

InstrumentPhase ^
InstrumentPhase::New
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   InstrumentPhase ^ instrumentPhase = gcnew InstrumentPhase();

   instrumentPhase->Initialize(config, 
       "[!output PROJECT_NAME] instrumentation phase");

#if defined(PHX_DEBUG_SUPPORT)
   instrumentPhase->PhaseControl = InstrumentPhase::InstrumentPhaseControl;
#endif

   return instrumentPhase;
}

//-----------------------------------------------------------------------------
// 
// Description:
//
//    Instrument a particular function.
//
// Arguments:
//
//    unit - The unit this phase is operating on.
//
// Remarks:
//
//    Insert your instrumentation code here.  For more information on
//    adding instrumentation code to a binary, please see the samples
//    readme file.
//
//-----------------------------------------------------------------------------

void
InstrumentPhase::Execute
(
   Phx::Unit ^ unit
)
{
   if (!unit->IsFunctionUnit)
   {
      return;
   }
   
   Phx::FunctionUnit ^ functionUnit = unit->AsFunctionUnit;
   
   // TODO: Do your instrumentation code of this unit here
}

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

EncodePhase ^
EncodePhase::New
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   EncodePhase ^ encodePhase = gcnew EncodePhase();

   encodePhase->Initialize(config, "Encode");

   return encodePhase;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Executes the EncodePhase phase.
//
// Arguments:
//
//    unit - The unit that this phase should operate on.
//
// Remarks:
//
//    This phase encodes the IR of the unit to ready it for writing.  Most
//    instrumentation tools will probably not need to modify this phase.
//
//-----------------------------------------------------------------------------

void
EncodePhase::Execute
(
   Phx::Unit ^ unit
)
{
   Phx::PE::Writer::EncodeUnit(unit);
}

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

EmitPhase ^
EmitPhase::New
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   EmitPhase ^ emitPhase = gcnew EmitPhase();

   emitPhase->Initialize(config, "Emit");

   return emitPhase;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Executes the EmitPhase phase.
//
// Arguments:
//
//    unit - The unit that this phase should operate on.
//
// Remarks:
//
//    Emits the final modified PE, complete with a new pdb.
//
//-----------------------------------------------------------------------------

void
EmitPhase::Execute
(
   Phx::Unit ^ unit
)
{
   if (!unit->IsPEModuleUnit)
   {
      return;
   }
   
   Phx::PEModuleUnit ^ peModule = unit->AsPEModuleUnit;

   // Retrieve the names of the input and output files for the tool.

   String ^ inputFileNameStr = Driver::in->GetValue(nullptr);   
   String ^ outputFileNameString = Driver::out->GetValue(nullptr);
   String ^ outputPdbFileNameStr = Driver::pdbout->GetValue(nullptr);

   Phx::Output::WriteLine("...writing to " + outputFileNameString);

   // Initialize a new PE writer.

   Phx::PE::Writer ^ writer = Phx::PE::Writer::New(
      Phx::GlobalData::GlobalLifetime, outputFileNameString, 
      outputPdbFileNameStr, peModule, peModule->SymbolTable,
      peModule->Architecture, peModule->Runtime);            

   writer->Write();
}
[!else]
//-----------------------------------------------------------------------------
//
// Description:
//
//    Initialize the static analysis phase.
//
// Remarks:
//
//    Sets up a component control under debug builds, so that we
//    can dump IR before/after the phase and so on. In other
//    words, because we create this control, you can pass in
//    options like -predumpmtrace that will dump the IR for each
//    method before this phase runs.
//
//-----------------------------------------------------------------------------

void
StaticAnalysisPhase::Initialize()
{
#if defined(PHX_DEBUG_SUPPORT)
   StaticAnalysisPhase::StaticAnalysisPhaseCtrl = 
      Phx::Controls::ComponentControl::New("root",
      "Perform Static Analysis", "[!output PROJECT_NAME].cpp");
#endif
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Creates a new StaticAnalysisPhase object
//
// Arguments:
//
//    config - The encapsulating object that simplifies handling of
//    the phase list and pre and post phase events.
//
//-----------------------------------------------------------------------------

StaticAnalysisPhase ^
StaticAnalysisPhase::New
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   StaticAnalysisPhase ^ staticAnalysisPhase = gcnew StaticAnalysisPhase();

   staticAnalysisPhase->Initialize(config, "Static Analysis");

#if defined(PHX_DEBUG_SUPPORT)
   staticAnalysisPhase->PhaseControl = 
      StaticAnalysisPhase::StaticAnalysisPhaseCtrl;
#endif

   return staticAnalysisPhase;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Executes the StaticAnalysisPhase phase.
//
// Arguments:
//
//    unit - The unit that this phase should operate on.
//
// Remarks:
//
//    You should add your static analysis code here.  Each unit of the
//    program will pass through this function.
//
//-----------------------------------------------------------------------------

void
StaticAnalysisPhase::Execute
(
   Phx::Unit ^ unit
)
{
   if(!unit->IsFunctionUnit)
   {
      return;
   }

   Phx::FunctionUnit ^ functionUnit = unit->AsFunctionUnit;

   // TODO: Do your static analysis of this unit here
}
[!endif]

//-----------------------------------------------------------------------------
//
// Description:
//
//    Does the static initialization of the Phoenix framework and the program.
//
// Arguments:
//
//    arguments - Array of command line argument strings.
//
// Remarks:
//
//    StaticInitialize registers the available targets and processes in the 
//    command-line options.
//
//-----------------------------------------------------------------------------

void
Driver::StaticInitialize
(
   array<String ^> ^ arguments
)
{
   // Initialize the available targets.

   Driver::InitializeTargets();
   
   // Start initialization of the Phoenix framework.

   Phx::Initialize::BeginInitialization();

   // Initialize the command line string controls

   Driver::InitializeCommandLine();

[!if INSTRUMENTATION]
   // Initialize the component control for the instrumentation phase.
   // This is included so that standard Phoenix controls can be used on
   // this too.

   InstrumentPhase::Initialize();
[!else]
   // Initialize the component control for the static analysis phase.
   // This is included so that standard Phoenix controls can be used on
   // this too.

   StaticAnalysisPhase::Initialize();
[!endif]

   Phx::Initialize::EndInitialization(L"PHX|*|_PHX_", arguments);

   // Check the processed command line options against those required
   // by the tool for execution.  If they are not present, exit the app.

   if(!Driver::CheckCommandLine())
   {
      Driver::Usage();
      Phx::Term::All(Phx::Term::Mode::Fatal);
      Environment::Exit(1);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Usage string for this sample.
//
// Remarks:
//
//    Options can include any of the standard phoenix controls,
//    eg -dumptypes.
//
//-----------------------------------------------------------------------------
      
void 
Driver::Usage()
{
[!if INSTRUMENTATION]
   Phx::Output::WriteLine(L"[!output PROJECT_NAME] /out <filename> "
      L"/pdbout <filename> /in <image-name>");  
[!else]
   Phx::Output::WriteLine(L"[!output PROJECT_NAME] /in <image-name>");
[!endif]
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Checks the command line against the required options.
//
// Returns:
//
//    True if the command line has all the required options, false otherwise.
//
//-----------------------------------------------------------------------------

bool
Driver::CheckCommandLine()
{
[!if INSTRUMENTATION]
   return !(String::IsNullOrEmpty(Driver::in->GetValue(nullptr))
      || String::IsNullOrEmpty(Driver::out->GetValue(nullptr))
      || String::IsNullOrEmpty(Driver::pdbout->GetValue(nullptr)));
[!else]
   return !String::IsNullOrEmpty(Driver::in->GetValue(nullptr));
[!endif]
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Registers string controls with Phoenix for command line option
//    processing.
//
//-----------------------------------------------------------------------------

void
Driver::InitializeCommandLine()
{
   // Initialize each command line option (string controls), so that
   // the framework knows about them.

   Driver::in           = Phx::Controls::StringControl::New(L"in", 
      L"input file", 
      Phx::Controls::Control::MakeFileLineLocationString(L"[!output PROJECT_NAME].cpp", 
      __LINE__));
[!if INSTRUMENTATION]
   Driver::out          = Phx::Controls::StringControl::New(L"out", 
      L"output file", 
      Phx::Controls::Control::MakeFileLineLocationString(L"[!output PROJECT_NAME].cpp", 
      __LINE__));
   Driver::pdbout       = Phx::Controls::StringControl::New(L"pdbout", 
      L"output pdb file", 
      Phx::Controls::Control::MakeFileLineLocationString(L"[!output PROJECT_NAME].cpp", 
      __LINE__));
[!endif]
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Initialize the dependent architectures and runtimes.
//
// Remarks:
//
//    All runtimes and architectures available in the RDK are registered.
//
//-----------------------------------------------------------------------------

void 
Driver::InitializeTargets()
{
   // Setup targets available in the RDK.

   Phx::Targets::Architectures::Architecture ^ x86Arch = 
      Phx::Targets::Architectures::X86::Architecture::New();
   Phx::Targets::Runtimes::Runtime ^ win32x86Runtime = 
      Phx::Targets::Runtimes::Vccrt::Win32::X86::Runtime::New(x86Arch);
   Phx::GlobalData::RegisterTargetArchitecture(x86Arch);
   Phx::GlobalData::RegisterTargetRuntime(win32x86Runtime);

   Phx::Targets::Architectures::Architecture ^ msilArch = 
      Phx::Targets::Architectures::Msil::Architecture::New();
   Phx::Targets::Runtimes::Runtime ^ win32MSILRuntime = 
      Phx::Targets::Runtimes::Vccrt::Win::Msil::Runtime::New(msilArch);
   Phx::GlobalData::RegisterTargetArchitecture(msilArch);
   Phx::GlobalData::RegisterTargetRuntime(win32MSILRuntime);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Process the module.
//
// Remarks:
//
//    The launch point for the instrumentation/static analysis functionality
//    of the tool.  Process first opens the existing binary and determines
//    its target architecture and runtime.  Then, it creates a PE module unit
//    with the input binary and a phase listing.  The phase listing is then
//    executed on the PE module.
//
//-----------------------------------------------------------------------------

Phx::Term::Mode
Driver::Process()
{
   Phx::Output::WriteLine("Processing " + 
      Driver::in->GetValue(nullptr) + " ...");

   // Lookup the architecture and runtime from the existing assembly.

   Phx::PEModuleUnit ^ oldModuleUnit = 
      Phx::PEModuleUnit::Open(Driver::in->GetValue(nullptr));

   Phx::Targets::Architectures::Architecture ^ architecture = oldModuleUnit->Architecture;
   Phx::Targets::Runtimes::Runtime ^ runtime = oldModuleUnit->Runtime;

   oldModuleUnit->Close();
   oldModuleUnit->Delete();

   // Create an empty program unit

   Phx::Lifetime ^ lifetime = 
      Phx::Lifetime::New(Phx::LifetimeKind::Global, nullptr);

   Phx::ProgramUnit ^ programUnit = Phx::ProgramUnit::New(lifetime, nullptr,
      Phx::GlobalData::TypeTable, architecture, runtime);

   // Make an empty moduleUnit

   Phx::PEModuleUnit ^ moduleUnit = Phx::PEModuleUnit::New(lifetime, 
      Phx::Name::New(lifetime, Driver::in->GetValue(nullptr)), programUnit,
      Phx::GlobalData::TypeTable, architecture, runtime);

   // Create an overall phase list....

   Phx::Phases::PhaseConfiguration ^ config =
      Phx::Phases::PhaseConfiguration::New(lifetime, "[!output PROJECT_NAME] Phases");

   // Add phases...
      
   config->PhaseList->AppendPhase(Phx::PE::ReaderPhase::New(config));

   // Create the per-function phase list....

   Phx::Phases::PhaseList ^ unitList = Phx::PE::UnitListPhaseList::New(config,
      Phx::PE::UnitListWalkOrder::PrePass);
   
   unitList->AppendPhase(
      Phx::PE::RaiseIRPhase::New(config,
         Phx::FunctionUnit::LowLevelIRBeforeLayoutFunctionUnitState));

[!if INSTRUMENTATION]
   unitList->AppendPhase(InstrumentPhase::New(config));
   unitList->AppendPhase(EncodePhase::New(config));
[!else]
   unitList->AppendPhase(StaticAnalysisPhase::New(config));
[!endif]
   unitList->AppendPhase(Phx::PE::DiscardIRPhase::New(config));
   
   config->PhaseList->AppendPhase(unitList);

[!if INSTRUMENTATION]
   config->PhaseList->AppendPhase(EmitPhase::New(config));
[!endif]

   // Add any plugins or target-specific phases.
      
   Phx::GlobalData::BuildPlugInPhases(config);

   // Run the phases.

   config->PhaseList->DoPhaseList(moduleUnit);

   return Phx::Term::Mode::Normal;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Entry point for application
//
// Arguments:
//
//    argument - Command line argument strings.
//
//-----------------------------------------------------------------------------

int
main
(
   array<String ^> ^ arguments
)
{
   // Do static initialization of the Phoenix framework.

   Driver::StaticInitialize(arguments);
   
   // Process the binary.

   Driver ^ driver = gcnew Driver();
   Phx::Term::Mode termMode = driver->Process();

   Phx::Term::All(termMode);
   return (termMode == Phx::Term::Mode::Normal ? 0 : 1);
}
