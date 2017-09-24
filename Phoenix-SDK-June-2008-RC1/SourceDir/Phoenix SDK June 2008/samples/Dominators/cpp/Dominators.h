//-----------------------------------------------------------------------------
//
// Description:
//
//   Dominance Computation
//
// Remarks:
//
//    Header for the "Dominators" phase plugin.
//
//-----------------------------------------------------------------------------

namespace Dominators
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
   static Phx::Controls::ComponentControl ^ DominatorsControl;

#endif

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

//-----------------------------------------------------------------------------
//
// Description:
//
//    The DominanceData class holds the per-block data and the logic for merging
//    information from neighboring blocks. 
//    
//-----------------------------------------------------------------------------

ref class DominanceData : public Phx::Dataflow::Data
{
public:

   static DominanceData ^
   New
   (
      Phx::Lifetime ^ lifetime
   );

   virtual void Delete() override;

   virtual void
   Merge
   (
      Phx::Dataflow::Data ^     dependencyData,
      Phx::Dataflow::Data ^     blockData,
      Phx::Dataflow::MergeFlags flags
   ) override;

   virtual bool
   SamePrecondition
   (
      Phx::Dataflow::Data ^ blockData
   ) override;

   virtual bool
   SamePostCondition
   (
      Phx::Dataflow::Data ^ blockData
   ) override;

   virtual void
   Update
   (
      Phx::Dataflow::Data ^ temporaryData
   ) override;

   property Phx::BitVector::Sparse ^ InBitVector;
   property Phx::BitVector::Sparse ^ OutBitVector;
};


//-----------------------------------------------------------------------------
//
// Description:
//
//    The DominanceWalker is a specialization of the Phx::Dataflow::Walker that
//    helps orchestrate the solution of the dominance equations.
//    
//-----------------------------------------------------------------------------

ref class DominanceWalker : public Phx::Dataflow::Walker
{
public:

   static DominanceWalker ^ New(Phx::FunctionUnit ^ functionUnit);

   virtual void Delete() override;

   virtual void
   AllocateData
   (
      unsigned int numberElements
   ) override;

   virtual void
   EvaluateBlock
   (
      Phx::Graphs::BasicBlock ^ block,
      Phx::Dataflow::Data ^     temporaryData
   ) override;

   void 
   SetBoundaryConditions
   (
      Phx::Graphs::FlowGraph ^ flowGraph
   );

#if defined (PHX_DEBUG_SUPPORT)

   property Phx::Controls::ComponentControl ^ DebugControl
   {
      virtual Phx::Controls::ComponentControl ^ get() override { return Phase::DominatorsControl; }
   } 

#endif
};

}
