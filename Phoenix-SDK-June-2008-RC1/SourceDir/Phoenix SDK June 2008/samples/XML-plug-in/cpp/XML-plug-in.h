//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    XMLWriter implements a simple collection of methods for writing
//    XML elements and their attributes to a text file.
//
// Remarks:
//
//    This class uses string and output services provided by Phoenix.
//    An alternative provided by the Framework is XMLTextWriter.
//
//    To open a file for output:
//       Open(filename);
//
//    To close an opened file:
//       Close();
//
//    To emit a bare-bones XML declaration, with a stylesheet instruction
//    (pass a NULL if you do not want a stylesheet processing instruction):
//       EmitSimpleHeader(stylesheetFilename);
//
//    To specify an indentation string, repeated for each level (pass a
//    NULL if you do not want any indentation):
//       IndentWith(string);
//
//    To write arbitrary text, with the current indentation and a newline
//    automatically suplied:
//       EmitText(text);
//
//    Certain characters have special meaning to XML/HTML and must not
//    appear in attribute values or textual content.  These are the
//    apersand (&), angle brackets (<,>), single and double quote marks (',"),
//    and control codes.  To replace these with the proper character entities
//    (escape sequences):
//       outString = XMLize(inString);
//
//    Elements may be empty and may have attributes. Here are the various
//    possibilites:
//
//    To emit an empty element with attributes:
//          <tag attr1="..." attr2="..." />
//       EmitElementOpen(tag);
//       EmitAttribute(name, value);  // repeat as required
//       EmitElementClose();
//
//    To emit an empty element with no attributes:
//          <tag />
//       EmitElementOpen(tag);
//       EmitElementClose();
//
//    To emit an element with attributes:
//          <tag attr1="..." attr2="...">
//             contents
//          </tag>
//       EmitElementOpen(tag);
//       EmitAttribute(name, value); // repeat as required
//       EmitAttributeClose();
//       // Emit contents as required.
//       EmitElementClose(tag);
//
//    To emit an element with no attributes:
//          <tag>
//             contents
//          </tag>
//       EmitSimpleElementOpen(tag);
//       // Emit contents as required.
//       EmitElementClose(tag);
//
//-----------------------------------------------------------------------------

public
ref class XMLWriter
{
public:

   XMLWriter()
      : doIndent(true), indentStr(L"   ")
   {
   };

   void IndentWith(System::String ^ string);
   void Open(System::String ^ filename);
   void Close();
   void EmitSimpleHeader(System::String ^ stylesheet);
   void EmitIndent();
   void EmitSimpleElementOpen(System::String ^ tag);
   void EmitElementOpen(System::String ^ tag);
   void EmitElementClose(System::String ^ tag);
   void EmitElementClose();
   void EmitAttribute(System::String ^ name, System::String ^ value);
   void EmitAttributeClose();
   void EmitText(System::String ^ text);

   static System::String ^ XMLize(System::String ^ string);

private:

   bool              doIndent;    // If true, child elements are indented.
   int               indentLevel; // Logical nesting level, not # of blanks.
   System::String ^  indentStr;   // string for each indent level
   Phx::Lifetime ^   lifetime;
   Phx::TextOutput ^ textOutputFile;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    XML-plug-in -- A Phoenix plug-in and its phase that dumps the IR as XML.
//
// Usage:
//
//    Uses the standard Phoenix plug-in model for c2.  Example:
//       cl -d2plugin:<plugin.dll>  ...
//
// Remarks:
//
//    This simple plug-in demonstrates the construction of plug-ins
//    and phases.  Hence, comments are spread throughout the source.
//    To help orient you to the Phoenix namespace/class hierarchy,
//    we use fully-qualified names throughout.
//
//    To begin, the plug-in implements the two methods all plug-ins
//    must have, RegisterObjects and BuildPhases.  The new phase,
//    called XMLPhase, that converts IR to XML, implements the two
//    methods all phases must have, New and Execute.
//
//    A small amount of code is turned off in a release build (when
//    PHX_DEBUG_DUMPS is undefined), with a slight loss of information
//    in the resultant XML file.
//
//    A note on casing "XML": global items begin with "XML...", while a
//    camel-case local variable begins with "xml..."
//
//-----------------------------------------------------------------------------

public
ref class XMLPlugIn : public Phx::PlugIn
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
      virtual System::String ^ get() override { return L"XML IR Dump"; }
   }
   
   bool
   ShouldDumpPhase
   (
      Phx::Phases::Phase ^ phase
   );

public:

   static Phx::Controls::StringControl ^ XMLDirCtrl;
   static System::String ^         XMLDir;

protected:

   static System::String ^
   SanitizeDecoratedName
   (
      System::String ^ origName
   );

   void
   PostPhaseEventHandler
   (
      Phx::Unit ^          unit,
      Phx::Phases::Phase ^ phase
   );

   void
   EmitPhases
   (
      Phx::Phases::PhaseConfiguration ^ config
   );

   void
   EmitFuncUnit
   (
      Phx::FunctionUnit ^      unit,
      Phx::Phases::Phase ^ phase
   );

   void
   EmitOpndList
   (
      Phx::IR::Instruction ^          instruction,
      Phx::IR::OperandListSelector listSel
   );

   void
   EmitVarOpnd
   (
      Phx::IR::VariableOperand ^ operand
   );

   void
   EmitMemOpnd
   (
      Phx::IR::MemoryOperand ^ operand
   );

   void
   EmitImmOpnd
   (
      Phx::IR::ImmediateOperand ^ operand
   );

   void
   EmitLabelOpnd
   (
      Phx::IR::LabelOperand ^ operand
   );

   void
   EmitFuncOpnd
   (
      Phx::IR::FunctionOperand ^ operand
   );

   void
   EmitModRefOpnd
   (
      Phx::IR::AliasOperand ^ operand
   );

private:

   System::String ^ fileName;
   XMLWriter ^      xml;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    The XMLPhaseVisitor object is used to visit all phases in phase list.
//
// Remarks:
//
//    Phx::Phases::PhaseVisitor provides an implementation for walking every
//    phase in a phase list. This is precisely what we need in order to emit
//    the complete phase list as XML.
//
//-----------------------------------------------------------------------------

public
ref class XMLPhaseVisitor : public Phx::Phases::PhaseVisitor
{

public:

   virtual void
   Visit
   (
      Phx::Phases::Phase ^ phase
   ) override;

   virtual void
   PreVisit
   (
      Phx::Phases::PhaseList ^ phaseList
   ) override;

   virtual void
   PostVisit
   (
      Phx::Phases::PhaseList ^ phaseList
   ) override;

public:

   XMLWriter ^ xml;
   XMLPlugIn ^ plugIn;
};
