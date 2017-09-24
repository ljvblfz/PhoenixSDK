//------------------------------------------------------------------------------
//
// Description:
//
//    Phoenix subsystem of the add-in.  Provides Phoenix services to the
//    rest of the add-in.
//
//------------------------------------------------------------------------------

#include "PhoenixProvider.h"

namespace [!output SAFE_NAMESPACE_NAME]
{

//------------------------------------------------------------------------------
//
// Description:
//
//    Default constructor.
//
//------------------------------------------------------------------------------

PhoenixProvider::PhoenixProvider()
{
   if (!Phx::GlobalData::PhoenixIsInitialized)
   {
      InitializePhx();
   }
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Initializes Phoenix.
//
//------------------------------------------------------------------------------

void 
PhoenixProvider::InitializePhx()
{
   // Initialize the targets first

   InitializeTargets();

   // Start initializing Phoenix before doing anything else

   Phx::Initialize::BeginInitialization();

   // Initialize the controls

   Phx::Initialize::EndInitialization(L"PHX|*|_PHX_|", 0, nullptr);
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Registers the targets available in the RDK.
//
//------------------------------------------------------------------------------

void
PhoenixProvider::InitializeTargets()
{
   // Register all the targets and architectures that
   // are possible with the RDK

   Phx::Targets::Architectures::Architecture ^ x86Arch =
      Phx::Targets::Architectures::X86::Architecture::New();
   Phx::Targets::Runtimes::Runtime ^ win32x86Runtime =
      Phx::Targets::Runtimes::Vccrt::Win32::X86::Runtime::New(x86Arch);

   Phx::GlobalData::RegisterTargetArchitecture(x86Arch);
   Phx::GlobalData::RegisterTargetRuntime(win32x86Runtime);

   Phx::Targets::Architectures::Architecture ^ msilArch =
      Phx::Targets::Architectures::Msil::Architecture::New();
   Phx::Targets::Runtimes::Runtime ^ winMSILRuntime =
      Phx::Targets::Runtimes::Vccrt::Win::Msil::Runtime::New(msilArch);
   Phx::GlobalData::RegisterTargetArchitecture(msilArch);
   Phx::GlobalData::RegisterTargetRuntime(winMSILRuntime);
}

} // [!output SAFE_NAMESPACE_NAME]