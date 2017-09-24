//-----------------------------------------------------------------------------
//
// Description:
//
//   [!output PHASE_DESCRIPTION]
//
// Remarks:
//
//    Registers the "[!output SAFE_PROJECT_NAME]" component control.
//
//-----------------------------------------------------------------------------

#include "[!output SAFE_PROJECT_NAME].h"

namespace [!output SAFE_PROJECT_NAME]
{

//-----------------------------------------------------------------------------
//
// Description:
//
//    New creates an instance of a phase.  Following Phoenix guidelines,
//    New is static.
//
// Arguments:
//
//    config - [in] A pointer to a Phases::PhaseConfiguration that provides
//          properties for retrieving the initial phase list.
//
// Returns:
//
//    A pointer to the new phase.
//
//-----------------------------------------------------------------------------

Phase ^
Phase::New
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   Phase ^ phase = gcnew Phase();

   phase->Initialize(config, L"[!output PHASE_DESCRIPTION]");

#if defined(PHX_DEBUG_SUPPORT)

   phase->PhaseControl = Phase::[!output SAFE_PROJECT_NAME]Control;

#endif

   return phase;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Execute is the phase's prime mover; all unit-centric processing occurs
//    here.  Note that Execute might be thought of as a "callback": as the
//    C2 host compiles each FunctionUnit, passing it from phase to phase, the
//    plug-in Execute method is called to do its work.
//
// Arguments:
//
//    unit - [in] The unit to process.
//
// Remarks:
//
//    Since IR exists only at the FunctionUnit level, we ignore ModuleUnits.
//
//    The order of units in a compiland passed to Execute is indeterminate.
//
//-----------------------------------------------------------------------------

void
Phase::Execute
(
   Phx::Unit ^ unit
)
{
   if (!unit->IsFunctionUnit)
   {
      return;
   }

   Phx::FunctionUnit ^ functionUnit = unit->AsFunctionUnit;

   //TODO: Insert your phase logic here
      
   // This example loop iterates through all the instructions in the
   // intermediate representation of the FunctionUnit.

   for each (Phx::IR::Instruction ^ instruction in Phx::IR::Instruction::Iterator(functionUnit))
   {
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    RegisterObjects initializes the plug-in's environment.  Normally, this
//    includes defining your command-line switches (controls) that should be
//    handled by Phoenix.  Phoenix calls this method early, upon loading the
//    plug-in's DLL.
//
// Remarks:
//
//    The RegisterObjects method is not the place to deal with phase-specific
//    issues, because the host has not yet built its phase list.
//    However, controls ARE phase-specific.  Because your phase object does
//    not exist yet, your phase's controls must be static fields,
//    accessable from here.
//
//-----------------------------------------------------------------------------

void
PlugIn::RegisterObjects()
{

#if defined(PHX_DEBUG_SUPPORT)

   Phase::[!output SAFE_PROJECT_NAME]Control =
      Phx::Controls::ComponentControl::New(L"[!output SAFE_PROJECT_NAME]",
         L"[!output PHASE_DESCRIPTION]",
         L"[!output SAFE_PROJECT_NAME].cpp");

#endif

}

//-----------------------------------------------------------------------------
//
// Description:
//
//    BuildPhases is where the plug-in creates and initializes its phase
//    object(s), and inserts them into the phase list
//    already created by the c2 host.
//
// Arguments:
//
//    config - [in] A pointer to a Phases::PhaseConfiguration that provides
//          properties for retrieving the initial phase list.
//
// Remarks:
//
//    Your plug-in determines a new phase's place in the list by locating
//    an existing phase by name and inserting the new phase before or after it.
//    A phase is created by its static New method.  One may place the insertion
//    code here in BuildPhases, or delegate it to the phase's New method.
//    Phoenix places few requirements on phase construction, so you are free
//    to choose the most reasonable approach for your needs.
//
//    Since we are inserting multiple instances of this phase, we'll build
//    their names from the name of the phases they follow.
//
//    The Phoenix framework has parsed the command line, so control values are
//    available at this point.
//
//-----------------------------------------------------------------------------

void
PlugIn::BuildPhases
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   Phx::Phases::Phase ^ basePhase;

   // [!output METHOD] [!output PHASE]

   basePhase = config->PhaseList->FindByName(L"[!output PHASE]");
   if (basePhase == nullptr)
   {
      Phx::Output::WriteLine(L"[!output PHASE] phase "
         L"not found in phaselist:");
      Phx::Output::Write(config->ToString());

      return;
   }

   Phx::Phases::Phase ^ phase = Phase::New(config);
   basePhase->[!output METHOD](phase);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Returns the name of your plugin
//
//-----------------------------------------------------------------------------

System::String^ PlugIn::NameString::get()
{
	return L"[!output SAFE_PROJECT_NAME]";
}

}
