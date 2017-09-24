//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    IR Longevity plug-in.
//
// Remarks:
//
//    This sample demonstrates the following concepts:
//
//    . Plug-Ins
//    . Phases
//    . Extension objects
//    . Event delegates
//
//    The longevity of IR objects is tracked during compilation. We take note
//    of the birth and death phase for each object. The count of birth-death
//    pairs is printed on the console when the process terminates.
//
//    We keep separate instruction and operand counts, but the distinct kinds
//    within those hierarchies are conglomerated.
//
// Usage:
//
// cl ... -d2plugin:ir-longevity-plug-in.dll [-d2IRLongevityOutput:<file name>]
//    
//    Arguments in [] are optional.
//        IRLongevityOutput:<file name> -
//             Dumps the output from this sample into a file provided in
//             the command line. Otherwise, IR-Longevity.txt output file 
//             will be created by default.
//
//             Note: if you invoke c2.exe via cl and specify multiple source
//             files on the command line, the IR-Longevity output will only
//             show the results for one of the files.
//
//-----------------------------------------------------------------------------

namespace IRLongevity
{

//-----------------------------------------------------------------------------
//
// Description:
//
//    InstrBirthExtensionObject is attached to IR instructions to indicate the
//    phase in which they were constructed.
//
//-----------------------------------------------------------------------------

public ref class InstrBirthExtensionObject : Phx::IR::InstructionExtensionObject
{
public:

   // Boilerplate code

   static InstrBirthExtensionObject ^
   Get
   (
      Phx::IR::Instruction ^ instruction
   );

   Phx::Phases::Phase ^ BirthPhase;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    OpndBirthExtensionObject is attached to IR operands to indicate the
//    phase in which they were constructed.
//
//-----------------------------------------------------------------------------

public ref class OpndBirthExtensionObject : Phx::IR::OperandExtensionObject
{
public:

   // Boilerplate code

   static OpndBirthExtensionObject ^
   Get
   (
      Phx::IR::Operand ^ operand
   );

   Phx::Phases::Phase ^ BirthPhase;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    The IR longevity plug-in.
//
// Remarks:
//
//    The longevity of IR objects is tracked during compilation. We take note
//    of the birth and death phase for each object. The count of birth-death
//    pairs is printed on the console when the process terminates.
//
//    We keep separate instruction and operand counts, but the distinct kinds
//    within those hierarchies are conglomerated.
//    
//    The output will be redirected into a file provided in the command line.
//    Otherwise, IR-Longevity.txt will be created by default.
//
//-----------------------------------------------------------------------------

public ref class PlugIn : Phx::PlugIn
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
      virtual System::String ^ get() override { return L"IR Longevity"; }
   }

   //  This control should allow the user to provide the output file.

   static Phx::Controls::StringControl ^ LongevityOuputCtrl;
   static System::String ^         OutputFile;

   static bool FileOpened = false;

   void
   RemoveIREventHandlers
   (
      Phx::FunctionUnit ^ functionUnit
   );

   void
   DeleteInstructionEventHandler
   (
      Phx::IR::Instruction ^ instruction
   );

   void
   DeleteOperandEventHandler
   (
      Phx::IR::Operand ^ operand
   );

private:
   void
   EndInitialization();

   void
   NewUnitEventHandler
   (
      Phx::Unit ^ unit
   );

   void
   NewInstructionEventHandler
   (
      Phx::IR::Instruction ^ instruction
   );

   void
   NewOperandEventHandler
   (
      Phx::IR::Operand ^ operand
   );

   void
   NoteLifetime
   (
      array<unsigned int, 2> ^ longevityArray,
      Phx::Phases::Phase ^     birthPhase,
      Phx::Phases::Phase ^     deathPhase
   );

   void
   TermEventHandler
   (
      Phx::Term::Mode mode
   );

   void
   DisplayLongevityArray
   (
      System::String ^         typeName,
      array<unsigned int, 2> ^ longevityArray
   );

   Phx::DependencyObject ^ TermEventDependencyObject;
   Phx::DependencyObject ^ NewUnitDependencyObject;
   Phx::DependencyObject ^ NewInstrDependencyObject;
   Phx::DependencyObject ^ DeleteInstrDependencyObject;
   Phx::DependencyObject ^ NewOpndDependencyObject;
   Phx::DependencyObject ^ DeleteOperandDependencyObject;

   System::Collections::ArrayList ^ PhaseArrayList;
   array<unsigned int, 2> ^         InstrLongevityArray;
   array<unsigned int, 2> ^         OpndLongevityArray;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    SweepPhase walks the IR to "kill" all instructions and operands.
//
// Remarks:
//
//    Because of the way Phoenix does memory management, the IR existing at
//    the end of function phase processing is not deleted. This phase is
//    placed after encoding as the final resting place for the IR.
//
//    This code is packaged as a phase for demonstration purposes. To be more
//    accurate and flexible, we'd use the DeleteUnitEvent. Doing that would
//    require a minor change to the accounting/output code since we'd no
//    longer have a phase to hang our hat on.
//
//-----------------------------------------------------------------------------

public ref class SweepPhase : Phx::Phases::Phase
{
public:

   static SweepPhase ^
   New
   (
      Phx::Phases::PhaseConfiguration ^ config,
      IRLongevity::PlugIn ^      plugIn
   );

#if (PHX_DEBUG_SUPPORT)
   static Phx::Controls::ComponentControl ^ DebugControl;
#endif

protected:

   virtual void
   Execute
   (
      Phx::Unit ^ unit
   ) override;

private:

   PlugIn ^ plugIn;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    Phase visitor to collect all the phases in an array list.
//
// Remarks:
//
//    We ignore phase lists.
//
//-----------------------------------------------------------------------------

public ref class PhaseVisitor : Phx::Phases::PhaseVisitor
{
public:
   virtual void
   Visit
   (
      Phx::Phases::Phase ^ phase
   ) override;

   System::Collections::ArrayList ^ arrayList;
};

} // namespace IRLongevity
