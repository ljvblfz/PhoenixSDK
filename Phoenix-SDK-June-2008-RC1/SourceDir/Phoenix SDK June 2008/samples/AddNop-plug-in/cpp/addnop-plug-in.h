#pragma once

//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Phoenix Plug-in sample.
//
// Remarks:
//
//    Registers "addnops" component control.
//
//    Injects a phase after Lower which injects a NOP prior to every
//    instruction.
//
// Usage:
//
//    cl ... -d2plugin:addnop-plug-in.dll
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
// Description:
//
//    Class designed to interface our phase with a Phoenix
//    based compiler.
//
// Remarks:
//
//    To interface with a Phoenix based compiler, a DLL contains a class
//    derived from Phx::Plugin.  This class must implement two methods:
//    RegisterObjects and BuildPhases.  RegisterObjects is called when the
//    plug-in is loaded and is responsible for registering any component
//    controls the plug-in might need.  BuildPhases is responsible for
//    modifying the phase list of the compiler, perhaps including a new phase
//    or replacing an existing one.
//
//------------------------------------------------------------------------------

public 
ref class AddNOPPlugIn : public Phx::PlugIn
{

public:

   virtual void RegisterObjects() override;

   virtual void
   BuildPhases
   (
      Phx::Phases::PhaseConfiguration ^ config
   ) override;

   property System::String ^ NameString
   {
      virtual System::String ^ get() override { return L"Add NOP"; }
   }
};

//------------------------------------------------------------------------------
//
// Description:
//
//    Phase that injects nop's before every instruction.
//
// Remarks:
//
//    Each phase must implement two methods: New and Execute.  New, a
//    static method, should initialize any phase controls and return a new
//    instance of the phase.  Execute is responsible for performing the
//    actual work of the phase.
//
//------------------------------------------------------------------------------

public 
ref class AddNOPPhase : public Phx::Phases::Phase
{
public:

   static AddNOPPhase ^
   New
   (
      Phx::Phases::PhaseConfiguration ^ config
   );

   //---------------------------------------------------------------------------
   // 
   // Description:
   //
   //    Component control providing standard Phoenix controls for this phase.
   //
   //---------------------------------------------------------------------------

#if defined(PHX_DEBUG_SUPPORT)
   static Phx::Controls::ComponentControl ^ AddNOPCtrl;
#endif

protected:

   virtual void
   Execute
   (
      Phx::Unit ^ unit
   ) override;

};
