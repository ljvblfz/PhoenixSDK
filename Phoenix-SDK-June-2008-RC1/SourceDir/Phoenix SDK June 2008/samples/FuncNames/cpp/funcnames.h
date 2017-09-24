//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    FuncNames: a minimal Phoenix plug-in that reports the name of each 
//    function being processed by the host.  (For example, when hosted by the
//    C2 backend, reports the name of each function as it is compiled)
//
// Usage:
//
//    Begin Snippet run
//    cl -nologo -d2plugin:FuncNames.dll functions.cpp
//    End Snippet
//
//-----------------------------------------------------------------------------

namespace FuncNames
{

//-----------------------------------------------------------------------------
//
// Description:
//
//    The FuncNames PlugIn class
//
// Remarks:
//
//    A minimal Phoenix plugin
//
//-----------------------------------------------------------------------------

public
ref class PlugIn : Phx::PlugIn
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
      virtual System::String ^ get() override { return L"FuncNames"; }
   }
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    The FuncNames Phase class
//
// Remarks:
//
//    This phase supplies the implementation for two methods (defined by
//    its Phase base class): New and Execute
//
//-----------------------------------------------------------------------------

public
ref class Phase : Phx::Phases::Phase
{
   
public:

   static Phx::Phases::Phase ^
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
}
