//-----------------------------------------------------------------------------
//
// Description:
//
//   Find loop bodies and loop exits
//
// Remarks:
//
//    Header for the "NaturalLoopBodiesAndExits" phase plugin.
//
//-----------------------------------------------------------------------------

namespace NaturalLoopBodiesAndExits
{

//-----------------------------------------------------------------------------
//
// Description:
//
//    A compiler phase.
//
// Remarks:
//
//    Each phase must implement two methods: New and Execute.  New, a
//    static method, should initialize any phase controls and return a new
//    instance of the phase.  Execute is responsible for performing the
//    actual work of the phase.
//
//-----------------------------------------------------------------------------

public
ref class Phase : Phx::Phases::Phase
{

public:

   static Phase ^
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

#if defined (PHX_DEBUG_SUPPORT)

public:
   static Phx::Controls::ComponentControl ^ NaturalLoopBodiesAndExitsControl;

#endif

private:

   void FindLoopBody(Phx::Graphs::BasicBlock ^ block);

};

//-----------------------------------------------------------------------------
//
// Description:
//
//    Class designed to interface our phase with a Phoenix-based compiler.
//
// Remarks:
//
//    To interface with a Phoenix based compiler, a DLL contains a class
//    derived from Phx::PlugIn.  This class must implement two methods:
//    RegisterObjects and BuildPhases.  RegisterObjects is called when the
//    plug-in is loaded and is responsible for registering any component
//    controls the plug-in might need.  BuildPhases is responsible for
//    modifying the phase list of the compiler, perhaps including a new phase
//    or replacing an existing one.  NameString returns the name of your plugin
//
//-----------------------------------------------------------------------------

public
ref class PlugIn : Phx::PlugIn
{

public:

   virtual void
   RegisterObjects() override;

   virtual void
   BuildPhases
   (
      Phx::Phases::PhaseConfiguration ^ config
   )override;
   
   property System::String^ NameString 
   {
		virtual System::String^ 
		get() override;
   }
};
}