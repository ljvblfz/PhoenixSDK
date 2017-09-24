//-----------------------------------------------------------------------------
//
// Description:
//
//   IPO plugin
//
// Remarks:
//
//    Registers the "ipoPlugin" component control.
//
//-----------------------------------------------------------------------------

#include "ipo-plug-in.h"

namespace ipoPlugin
{

//-----------------------------------------------------------------------------
//
// Description:
//
//    New creates an instance of a C2 pass with 2 phases: 
//       C2::Phases::CxxILReaderPhase and Phase
//    For a C2 pass, all 6 phaseList must be set properly
//
// Arguments:
//
//    config - [in] A pointer to a Passes::PassConfiguration
//
// Returns:
//
//    A pointer to the new pass.
//
//-----------------------------------------------------------------------------

C2Pass ^
C2Pass::New
(
   Phx::Passes::PassConfiguration ^ config
)
{
   C2Pass ^ pass = gcnew C2Pass();
   Phx::Graphs::CallGraphProcessOrder order = Phx::Graphs::CallGraphProcessOrder::BottomUp;

   pass->Initialize(config, order, L"New Pass-1");

   // Build the PhaseList with only 2 phases: C2::Phases::CxxILReaderPhase and Phase

   Phx::Phases::PhaseConfiguration ^ phaseConfiguration = 
      Phx::Phases::PhaseConfiguration::New(pass->Configuration->Lifetime, L"New Pass-1 Phases");   

   Phx::Phases::PhaseList ^ preCompilePhaseList = Phx::Phases::PhaseList::New(phaseConfiguration, L"New Pass-1 Pre-compilation phases");
   preCompilePhaseList->AppendPhase(C2::Phases::CxxILReaderPhase::New(phaseConfiguration));

   Phx::Phases::PhaseList ^ compilePhaseList = Phx::Phases::PhaseList::New(phaseConfiguration, L"New Pass-1 Compilation phases");
   compilePhaseList->AppendPhase(Phase::New(phaseConfiguration));

   Phx::Phases::PhaseList ^ postCompilePhaseList = Phx::Phases::PhaseList::New(phaseConfiguration, L"New Pass-1 Post-compilation phases");

   phaseConfiguration->PhaseList->AppendPhase(preCompilePhaseList);
   phaseConfiguration->PhaseList->AppendPhase(compilePhaseList);
   phaseConfiguration->PhaseList->AppendPhase(postCompilePhaseList);

   pass->PhaseConfigurationNative = phaseConfiguration;
   pass->PhaseConfiguration = phaseConfiguration;

   pass->PreCompilePhaseListNative = preCompilePhaseList;
   pass->CompilePhaseListNative = compilePhaseList;
   pass->PostCompilePhaseListNative = postCompilePhaseList;

   pass->PreCompilePhaseList = preCompilePhaseList;
   pass->CompilePhaseList = compilePhaseList;
   pass->PostCompilePhaseList = postCompilePhaseList;

   return pass;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Execute is the C2 Pass. 
//    Since this is a C2Pass, we can simply use C2 CallGraphCompilation() which 
//    is multi-threaded.  The compilation order is based on pass->CallGraphOrder.
//
// Arguments:
//
//    moduleUnit - [in] The moduleUnit to process.
//
//-----------------------------------------------------------------------------

bool
C2Pass::Execute
(
   Phx::ModuleUnit ^ moduleUnit
)
{
   Phx::Graphs::CallGraph ^ callGraph = moduleUnit->CallGraph;

   if (callGraph == nullptr) {
      Phx::Output::WriteLine(L"null call graph");
      return false;
   }

   C2::Driver::CallGraphCompilation(moduleUnit, this->CallGraphOrder);
   return true;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    New creates an instance of a Phx Pass with 2 phases: 
//       C2::Phases::CxxILReaderPhase and Phase
//
// Arguments:
//
//    config - [in] A pointer to a Passes::PassConfiguration
//
// Returns:
//
//    A pointer to the new pass.
//
//-----------------------------------------------------------------------------

Pass ^
Pass::New
(
   Phx::Passes::PassConfiguration ^ config
)
{
   Pass ^ pass = gcnew Pass();
   Phx::Graphs::CallGraphProcessOrder order = Phx::Graphs::CallGraphProcessOrder::TopDown;

   pass->Initialize(config, order, L"New Pass-2");

   // Build the PhaseList with only 2 phases: C2::Phases::CxxILReaderPhase and Phase

   Phx::Phases::PhaseConfiguration ^ phaseConfiguration = 
      Phx::Phases::PhaseConfiguration::New(pass->Configuration->Lifetime, L"New Pass-2 Phases");   

   phaseConfiguration->PhaseList->AppendPhase(C2::Phases::CxxILReaderPhase::New(phaseConfiguration));
   phaseConfiguration->PhaseList->AppendPhase(Phase::New(phaseConfiguration));

   pass->PhaseConfigurationNative = phaseConfiguration;
   pass->PhaseConfiguration = phaseConfiguration;
   return pass;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Execute is the pass's prime mover; 
//    Since this is not a C2 pass, cannot use C2::CallGraphCompilation. we have to 
//    create an empty functionUnit and apply phaseList to it. we follow top_down order
//    in the call graph.
//
// Arguments:
//
//    moduleUnit - [in] The moduleUnit to process.
//
//-----------------------------------------------------------------------------

bool
Pass::Execute
(
   Phx::ModuleUnit ^ moduleUnit
)
{
   Phx::Graphs::CallGraph ^ callGraph = moduleUnit->CallGraph;

   if (callGraph == nullptr) {
      Phx::Output::WriteLine(L"null call graph");
      return false;
   }

   // Compile it by following PostOrder (top-down order)

   Phx::Graphs::NodeFlowOrder ^ nodeOrder = Phx::Graphs::NodeFlowOrder::New(callGraph->Lifetime);
   nodeOrder->Build(callGraph, Phx::Graphs::Order::PostOrder);

   Phx::Targets::Runtimes::Runtime ^ runtime = moduleUnit->Runtime;
   unsigned int functionCount = 0;
      
   for(unsigned int i = 1; i <= nodeOrder->NodeCount; ++i)
   {
      Phx::Graphs::CallNode ^ node = nodeOrder->Node(i)->AsCallNode;
      if ((node == callGraph->UnknownCallerNode) || (node == callGraph->UnknownCalleeNode)
         || (node->FunctionSymbol == nullptr))
      {
         continue;
      }

      if (moduleUnit->IsPEModuleUnit) { // LTCG case
         moduleUnit = node->FunctionSymbol->CompilationUnitParentSymbol->Unit->AsModuleUnit;;
      }

      // Create an empty functionUnit, then DoPhaseList on it: 1st phase is CxxILReader

      Phx::FunctionUnit ^ functionUnit = Phx::FunctionUnit::New(
         moduleUnit->Lifetime, node->FunctionSymbol, Phx::CodeGenerationMode::Native,
         moduleUnit->TypeTable, runtime->Architecture, runtime, moduleUnit, functionCount++);

      Phx::Debug::Info::New(functionUnit->Lifetime, functionUnit); // attach Debug Info
      node->FunctionSymbol->FunctionUnit = functionUnit;           // bind functionUnit to symbol
      
      this->PhaseConfiguration->PhaseList->DoPhaseList(functionUnit); // execute phases

      functionUnit->Delete();       // delete it when it's done
   }
   return true;
}

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

   phase->Initialize(config, L"dump IR");
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
//-----------------------------------------------------------------------------

void
Phase::Execute
(
   Phx::Unit ^ unit
)
{
   if (!unit->IsFunctionUnit) {
      return;
   }

   Phx::FunctionUnit ^ functionUnit = unit->AsFunctionUnit;
   for each (Phx::IR::Instruction ^ instruction in Phx::IR::Instruction::Iterator(functionUnit))
   {
      instruction->Dump();
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
//    config - [in] A pointer to a Phases::PhaseConfiguration of C2 CodeGen Pass
//
// Remarks:
//
//    This is where your plug-in determines a new phase's place in the list of 
//    C2 CodeGen pass by locating an existing phase by name and inserting the new 
//    phase before or after it.
//
//-----------------------------------------------------------------------------

void
PlugIn::BuildPhases
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   Phx::Phases::Phase ^ basePhase;

   // InsertAfter       CxxIL Reader

   basePhase = config->PhaseList->FindByName(L"CxxIL Reader");
   if (basePhase == nullptr)
   {
      Phx::Output::WriteLine(L"CxxIL Reader phase "
         L"not found in phaselist:");
      Phx::Output::Write(config->ToString());

      return;
   }

   Phx::Phases::Phase ^ phase = Phase::New(config);
   basePhase->InsertAfter(phase);
   return; // you still can add/replace a phase in C2 CodeGen Pass (not the new pass)
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    BuildPasses is where the plug-in creates and initializes its pass
//    object(s), and inserts them into the pass list already created by the c2 host.
//
// Arguments:
//
//    config - [in] A pointer to a Passes::PassConfiguration that provides
//          properties for retrieving the initial pass list.
//
//-----------------------------------------------------------------------------

void
PlugIn::BuildPasses
(
   Phx::Passes::PassConfiguration ^ passConfiguration
)
{
   Phx::Passes::Pass ^ basePass = passConfiguration->PassList->FindByName(L"C2 Pass 0");
   if (basePass == nullptr) {
      Phx::Output::WriteLine(L"C2 Pass 0 not found in passlist:");
      Phx::Output::Write(passConfiguration->ToString());
      return;
   }

   C2Pass ^ newPass1 = C2Pass::New(passConfiguration);
   basePass->InsertAfter(newPass1);
   
   Pass ^ newPass2 = Pass::New(passConfiguration);
   newPass1->InsertAfter(newPass2);
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
	return L"ipoPlugin";
}
}
