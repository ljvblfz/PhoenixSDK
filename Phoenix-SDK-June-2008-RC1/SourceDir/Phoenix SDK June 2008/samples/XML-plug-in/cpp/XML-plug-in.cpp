//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------

using namespace System;

#include "XML-plug-in.h"

//-----------------------------------------------------------------------------
//
// Description:
//
//    RegisterObjects initializes the plug-in's environment.
//
// Remarks:
//
//    Normally, this includes defining your command-line switches (controls)
//    that should be handled by Phoenix.  Phoenix calls this method early,
//    upon loading the plug-in's DLL.
//
//    The RegisterObjects method is not the place to deal with phase-specific
//    issues, because the client host has not yet built its phase list.
//    However, controls CAN be phase-specific.  Because your phase object does
//    not exist yet, your phase's controls must be static fields, accessable
//    from here.
//
//    Plug-in controls, in cl, are prefixed by "-d2" on the command line.
//    Control "foo" is used as "-d2foo:<value>".  "-d2 foo:..." also works.
//    String values may be quoted to preserve characters otherwise interpreted
//    by the command shell.
//
//    A string control, called xml-directory, is created here, and used in
//    XMLPlugIn::BuildPhases, to specify the XML dump directory.  For example:
//    -d2xml-directory:"c:\temporary\xmldir" will put the XML files in that directory.
//
//    You should be conscientious about providing useful descriptions of
//    your controls.  A user can request the full list of defined controls,
//    along with their descriptions, with the "dumpctrls" control
//    (available in Debug or Test builds of Phoenix).
//
//-----------------------------------------------------------------------------

void
XMLPlugIn::RegisterObjects()
{
   System::String ^ defaultXMLDir = L".";
   System::String ^ descriptor;

   // The string control named "xml-directory" for specifying the XML directory.
   // (Am using the XML convention of lower-case hyphenated names.)

   descriptor = String::Format(
      L"XML-plug-in will write all XML files to this directory."
      L" Default is \"{0}\"", defaultXMLDir);

   XMLDirCtrl = Phx::Controls::StringControl::New(L"xml-directory",
      defaultXMLDir,
      descriptor,
      L"XML-plug-in.cpp");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    BuildPhases is where the plug-in creates and initializes its phase
//    object(s) and inserts them into the phase list already created by the
//    client host.
//
// Arguments:
//
//    config - [in] A pointer to a Phases::PhaseConfiguration that provides
//       properties for retrieving the initial phase list, and holds the pre-
//       and post-phase event lists.
//
// Remarks:
//
//    Your plug-in determines a new phase's place in the list by locating an
//    existing phase by name and inserting the new phase before or after it.
//    A phase is created by its static New method.  One may place the
//    insertion code here in BuildPhases, or delegate it to the phase's New
//    method.  Phoenix places few requirements on phase construction, so you
//    are free to choose the most reasonable approach for your needs.  See the
//    comments for both XMLPlugIn::New versions.
//
//    In our case, we want to run after every phase so we are adding a
//    post-phase event handler so we don't have much work to do. An alternative
//    implementation would be to create a component control (ComponentControl) and
//    allow the user to specify the phases they want to replace or insert on
//    the command line. This is left as an exercise for the reader.
//
//    The Phoenix framework has parsed the command line, so control values are
//    available at this point.
//
//-----------------------------------------------------------------------------

void
XMLPlugIn::BuildPhases
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   // Retrieve the user's desired XML dump directroy.

   this->XMLDir = this->XMLDirCtrl->GetValue(nullptr);
   if (System::IO::Directory::Exists(this->XMLDir)) 
   {
      // Add our post-phase event handler to the post-phase event of the 
      // client's phase configuration.

      config->PostPhaseEvent.Insert(
         gcnew Phx::Phases::PhaseConfiguration::InterPhaseEventDelegate(this,
            &XMLPlugIn::PostPhaseEventHandler));
   }
   else
   {
      // If the specified directory does not exist, emit a message and do not
      // insert the plug-in in the phase list.

      System::Console::WriteLine(
         "Error: argument {0} to command-line option -{1} is not a valid "
         "directory; skipping plug-in", 
         this->XMLDir, this->XMLDirCtrl->NameString);
   } 
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    PostPhaseEventHandler is the delegate for the phase configuration's
//    post-phase event.
//
// Arguments:
//
//    unit - [in] The unit to process.
//    phase - [in] Phase we're performing checks before/after.
//
// Remarks:
//
//    This function is a "callback": as the client host compiles each
//    FunctionUnit, passing it from phase to phase, the plug-in post-phase event
//    handler is called to do its work.
//
//    Since IR exists only at the FunctionUnit level, we ignore ModuleUnits.
//
//    The order of units in a compiland passed to Execute is indeterminate.
//
//-----------------------------------------------------------------------------

void
XMLPlugIn::PostPhaseEventHandler
(
   Phx::Unit ^          unit,
   Phx::Phases::Phase ^ phase
)
{
   System::String ^ outputFileName;

   if (!unit->IsFunctionUnit)
   {
      // Only interested in FuncUnits since that's where the IR is.

      return;
   }

   if (!this->ShouldDumpPhase(phase))
   {
      return;
   }

   // Build the XML file name from the previous phase name and unit's name.

   this->fileName = String::Format(L"{0}\\[{1}]{2}",
      XMLPlugIn::XMLDir,
      phase->NameString,
      XMLPlugIn::SanitizeDecoratedName(unit->NameString));

   // Create the XMLWriter object and emit the top-level XML.

   this->xml = gcnew XMLWriter();
   outputFileName = String::Format(L"{0}.xml", this->fileName);
   this->xml->Open(outputFileName);
   this->xml->EmitSimpleHeader(L"phx-xml.xsl");
   this->xml->IndentWith(L"   ");

   this->xml->EmitSimpleElementOpen(L"root");

   // Emit the phase list before the FunctionUnit. Since each phase handler
   // instance emits the phase list, which may not be of importance, the
   // following call to EmitPhases could be made conditional.

   this->EmitPhases(phase->Configuration);
   this->EmitFuncUnit(unit->AsFunctionUnit, phase);

   this->xml->EmitElementClose(L"root");
   this->xml->Close();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determine whether or not we should dump a phase.
//
// Arguments:
//
//    phase - [in] Phase we're performing checks before/after.
//
// Remarks:
//
//    We only want to dump phases that are executed.
//
//-----------------------------------------------------------------------------

bool
XMLPlugIn::ShouldDumpPhase
(
   Phx::Phases::Phase ^ phase
)
{

#if defined(PHX_DEBUG_CHECKS)

   if (phase->PhaseControl != nullptr)
   {
      if (Phx::Controls::DebugControls::OnControl->IsEnabled(phase->PhaseControl,
            Phx::GlobalData::GlobalUnit))
      {
         // Phase explicitly enabled.

         return true;
      }

      if (Phx::Controls::DebugControls::OffControl->IsEnabled(phase->PhaseControl,
            Phx::GlobalData::GlobalUnit))
      {
         // Phase explicitly disabled.

         return false;
      }
   }

#endif

   return phase->IsOnByDefault;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    EmitPhases outputs the XML <phases> element and its children by walking
//    the phase list.
//
// Remarks:
//
//    We use a visitor pattern to visit all the phase lists and phases.
//
//-----------------------------------------------------------------------------

void
XMLPlugIn::EmitPhases
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   // This clause simply introduces the outer-most <phases> element.

   this->xml->EmitSimpleElementOpen(L"phases");
   if (config != nullptr)
   {
      XMLPhaseVisitor ^ visitor = gcnew XMLPhaseVisitor;

      visitor->xml = this->xml;
      visitor->plugIn = this;
      visitor->WalkPhases(config->PhaseList);
   }
   this->xml->EmitElementClose(L"phases");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Emits the FunctionUnit's IR as XML.
//
// Arguments:
//
//    unit - [in] The FunctionUnit whose IR is to be emitted.
//
// Remarks:
//
//    The following is shows the output in XML pseudo-schema syntax:
//
//       <function-unit>
//          <blocks>
//             {
//             <block>
//                (
//                <instruction>
//                   (<src-operand-list>  (<*-operand />)+  </src-operand-list>)?
//                   (<dst-operand-list>  (<*-operand />)+  </dst-operand-list>)?
//                </instruction>
//                )+
//                <block-summary>
//                ...
//                </block-summary>
//             </block>
//             )+
//          </blocks>
//          <function-unit-summary>
//          ...
//          </function-unit-summary>
//       </function-unit>
//
//    Each XML element has several attributes, as appropriate; see the code.
//    Each unit's XML is output to its own file, whose name is composed of the
//    preceeding phase name and the unit's decorated name:
//
// Arguments:
//
//    unit - [in] The FunctionUnit whose IR is to be emitted.
//    phase - [in] The Phase that ran immediately before us.
//
//-----------------------------------------------------------------------------

void
XMLPlugIn::EmitFuncUnit
(
   Phx::FunctionUnit ^      unit,
   Phx::Phases::Phase ^ phase
)
{
   int                        firstFuncLineNum, lastFuncLineNum, lineNumber;
   int                        firstBlockLineNum, lastBlockLineNum;
   bool                       noPrevFlowGraph;
   bool                       doLineNums;
   Phx::Graphs::FlowEdge    ^ edge;
   Phx::IR::Instruction           ^ instruction;

   // Build the flow graph from which we will learn the basic-block structure.
   // This permits us to encapsulate the IR into block elements.  Since the
   // FunctionUnit may already have a flow graph, (its FlowGraph property would be
   // non-null), we must be careful not to delete a previous one when we're
   // done.

   noPrevFlowGraph = unit->FlowGraph == nullptr;
   if (noPrevFlowGraph)
   {
      unit->BuildFlowGraph();
   }

   // The following optional statement writes the flow graph for input to
   // AT&T's DOT graph visualization software.

   //unit->FlowGraph->DumpDot(String::Format(L"{0}.dot", this->fileName),
   //   "XML Dumper");

   // We would like source line numbers, if available, per instruction.

#if defined(PHX_DEBUG_DUMPS)
   doLineNums = Phx::Controls::DebugControls::DumpControl->IsEnabled(
      Phx::IR::Statics::LineNumbersControl, unit)
      && (unit->DebugInfo != nullptr);
#else
   doLineNums = false;
#endif // PHX_DEBUG_DUMPS

   // Viewer programs might like source line ranges, and we will maintain
   // line ranges for blocks, as well.

   firstFuncLineNum = 0;
   lastFuncLineNum = 0;
   if (doLineNums)
   {
      firstFuncLineNum = unit->FirstInstruction->GetLineNumber();
      lastFuncLineNum = firstFuncLineNum;
   }

   // Emit the top-level element for the FunctionUnit, with some useful attributes.

   this->xml->EmitElementOpen(L"function-unit");
   this->xml->EmitAttribute(L"name", this->SanitizeDecoratedName(unit->NameString));
   this->xml->EmitAttribute(L"phase", phase->NameString);
   this->xml->EmitAttribute(L"ir-state", unit->IRState.ToString());

   if (unit->DebugInfo != nullptr)
   {
      this->xml->EmitAttribute(L"source",
         unit->DebugInfo->GetFileName(unit->FirstInstruction->DebugTag));
   }

   this->xml->EmitAttributeClose();

   // Walk the basic-block list.  For each basic block, we then walk its
   // instructions (IR), emitting useful attributes for each instruction, as
   // well as each one's source and destination operand lists.  The
   // instruction list is available as a blocks's FirstInstruction property.

   // Note that the complete IR may also be discovered directly from the
   // unit's FirstInstruction property, one of many properties of FunctionUnit and its
   // base class, Unit.  Each block's FirstInstruction merely points into the full
   // IR list.

   this->xml->EmitSimpleElementOpen(L"blocks");

   for each (Phx::Graphs::BasicBlock ^ basicBlock in unit->FlowGraph->BasicBlocks)
   {
      this->xml->EmitElementOpen(L"block");

      // Check whether this basic block includes exception handlers.

      if (basicBlock->HasHandlerEdges)
      {
         this->xml->EmitAttribute(L"eh", L"true");
      }

      this->xml->EmitAttributeClose();

      // We'll gather the source line range for each block.  Initialized here.

      firstBlockLineNum = 0;
      lastBlockLineNum = 0;
      if (doLineNums)
      {
         firstBlockLineNum = basicBlock->FirstInstruction->GetLineNumber();
         lastBlockLineNum = firstBlockLineNum;
      }

      // Begin the per-block instruction traversal.

      instruction = basicBlock->FirstInstruction;
      for (unsigned int i = 0;  i < basicBlock->InstructionCount;  i++)
      {
         this->xml->EmitElementOpen(L"instruction");

#if defined(PHX_DEBUG_DUMPS)
         this->xml->EmitAttribute(L"op", instruction->OpcodeToString());
#endif

         // Emit condition codes for all conditional instructions.

         if (instruction->IsCompareInstruction || instruction->IsBranchInstruction)
         {
            if (instruction->ConditionCode != (safe_cast<int>(Phx::ConditionCode::IllegalSentinel)))
            {
               this->xml->EmitAttribute(L"cc",
                  Phx::Output::ConditionCodeToString(instruction->ConditionCode,
                     instruction->Unit->Architecture));
            }
         }

         this->xml->EmitAttribute(L"is-expression",
            instruction->IsExpression ? L"true" : L"false");

         if (instruction->HasSideEffect)
         {
            this->xml->EmitAttribute(L"side-effect", L"true");
         }

         // Update source line numbers.  Note: line numbers are not
         // monotonic by instruction order within a block.

         if (doLineNums)
         {
            lineNumber = instruction->GetLineNumber();

            if (lineNumber < firstBlockLineNum)
            {
               firstBlockLineNum = lineNumber;
            }
            else if (lineNumber > lastBlockLineNum)
            {
               lastBlockLineNum = lineNumber;
            }

            this->xml->EmitAttribute(L"line",
               Phx::Output::ToString(lineNumber));
         }

         this->xml->EmitAttributeClose();

         // Emit the source and destination operand lists, if present.

         this->EmitOpndList(instruction, Phx::IR::OperandListSelector::Source);
         this->EmitOpndList(instruction, Phx::IR::OperandListSelector::Destination);

         this->xml->EmitElementClose(L"instruction");

         instruction = instruction->Next;
      }

      this->xml->EmitSimpleElementOpen(L"block-summary");
      {
         // Output block's source line range.

         this->xml->EmitElementOpen(L"source-line-range");
         this->xml->EmitAttribute(L"first",
            Phx::Output::ToString(firstBlockLineNum));
         this->xml->EmitAttribute(L"last",
            Phx::Output::ToString(lastBlockLineNum));
         this->xml->EmitElementClose();

         // Output block's successor edge list.  This is sufficient to
         // recreate the control flow graph, if desired by a tool.  Each edge
         // has a "flags" attribute (flags="CEFU", where each letter position
         // indicates a possible attribute of the edge; see the code below)
         // and a "label" attribute whose value is the symbol on the leading
         // LABEL instruction of the destination block.

         edge = basicBlock->SuccessorEdgeList;
         if (edge != nullptr)
         {
            this->xml->EmitSimpleElementOpen(L"succ-edge-list");
            while (edge != nullptr)
            {
               this->xml->EmitElementOpen(L"edge");
               this->xml->EmitAttribute(L"flags",
                  String::Format(L"{0}{1}",
                     String::Format(L"{0}{1}",
                        (System::String ^)
                        ((edge->IsConditional)   ? L"C" : L"-"),
                        (System::String ^)
                        ((edge->IsException)        ? L"E" : L"-")),
                     String::Format(L"{0}{1}",
                        (System::String ^)
                        ((edge->IsFallThrough)   ? L"F" : L"-"),
                        (System::String ^)
                        ((edge->IsUnconditional) ? L"U" : L"-"))));
               this->xml->EmitAttribute(L"label", (edge->LabelOperand != nullptr) ?
                  this->SanitizeDecoratedName(edge->LabelOperand->NameString) :
                  L"n/a");
               this->xml->EmitElementClose();
               edge = edge->NextSuccessorEdge;
            }
            this->xml->EmitElementClose(L"succ-edge-list");
         }
      }

      this->xml->EmitElementClose(L"block-summary");

      this->xml->EmitElementClose(L"block");

      // Update FunctionUnit's source line range, using this block's interval.

      if (firstBlockLineNum < firstFuncLineNum)
      {
         firstFuncLineNum = firstBlockLineNum;
      }
      else if (lastBlockLineNum > lastFuncLineNum)
      {
         lastFuncLineNum = lastBlockLineNum;
      }
   }

   this->xml->EmitElementClose(L"blocks");

   this->xml->EmitSimpleElementOpen(L"function-unit-summary");
   {
      this->xml->EmitElementOpen(L"source-line-range");
      this->xml->EmitAttribute(L"first",
         Phx::Output::ToString(firstFuncLineNum));
      this->xml->EmitAttribute(L"last", Phx::Output::ToString(lastFuncLineNum));
      this->xml->EmitElementClose();
   }
   this->xml->EmitElementClose(L"function-unit-summary");

   this->xml->EmitElementClose(L"function-unit");

   if (noPrevFlowGraph)
   {
      unit->DeleteFlowGraph();
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    EmitOpndList outputs the XML element and children for an instruction's
//    source operand list (<src-operand-list>) or the destination operand list
//    (<dst-operand-list>), depending on the OperandListSelector parameter.  A list
//    is comprised of one or more <*-operand> elements, each with attributes that
//    are relevant to a study of the IR.
//
// Arguments:
//
//    instruction - [in] Emit this instruction's operand list.
//    listSel - [in] If OperandListSelector::Source, emit the source operand list;
//            otherwise, emit the destination operand list.
//
// Remarks:
//
//    Each IR instruction may have a source and a destination operand list,
//    either of which may be empty depending on the instruction.  We don't
//    emit a <*-operand-list> element if it's empty.
//
//    "for each" is used to walk the operand lists.  The loop uses a
//    switch to select handling for each kind of operand, since each has its
//    specific attributes.  The switch shows the use of a common approach in
//    Phoenix for identifying kinds of objects, "kind bits."  In this case,
//    the base class of operands, Operand, has boolean "IsFoo" properties for
//    each subclass, such as IsVariableOperand and IsImmediateOperand.  But it also reports a
//    subclass' type by an enumeration, the OperandKind property, which is easier
//    to use in switch statements.
//
//-----------------------------------------------------------------------------

void
XMLPlugIn::EmitOpndList
(
   Phx::IR::Instruction           ^ instruction,
   Phx::IR::OperandListSelector  listSel
)
{
   Phx::IR::Operand  ^ operandList =
      (listSel == Phx::IR::OperandListSelector::Source)
      ? instruction->SourceOperandList
      : instruction->DestinationOperandList;

   System::String ^ element = (listSel == Phx::IR::OperandListSelector::Source)
      ? L"src-operand-list"
      : L"dst-operand-list";

   if (operandList != nullptr)
   {
      this->xml->EmitSimpleElementOpen(element);

      // Walk the operand list, emitting attributes specific to each kind.

      for each (Phx::IR::Operand ^ operand in Phx::IR::Operand::Iterator(operandList))
      {
         // Ignore address-mode operands since they aren't interesting for our
         // purposes.

         if (operand->IsAddressModeOperand)
         {
            continue;
         }

         switch (operand->OperandKind)
         {
            case Phx::IR::OperandKind::VariableOperand:

               this->EmitVarOpnd(operand->AsVariableOperand);
               break;

            case Phx::IR::OperandKind::MemoryOperand:

               this->EmitMemOpnd(operand->AsMemoryOperand);
               break;

            case Phx::IR::OperandKind::ImmediateOperand:

               this->EmitImmOpnd(operand->AsImmediateOperand);
               break;

            case Phx::IR::OperandKind::LabelOperand:

               this->EmitLabelOpnd(operand->AsLabelOperand);
               break;

            case Phx::IR::OperandKind::FunctionOperand:

               this->EmitFuncOpnd(operand->AsFunctionOperand);
               break;

            case Phx::IR::OperandKind::AliasOperand:

               this->EmitModRefOpnd(operand->AsAliasOperand);
               break;

            default:

#if defined(PHX_DEBUG_CHECKS)
               Phx::Asserts::AssertFailed(L"Unreached",
                  L"Unknown operand kind in XMLPlugIn::EmitOpndList",
                  L"XML-plug-in.cpp", __LINE__);
#endif
               break;
         }
      }

      this->xml->EmitElementClose(element);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    EmitVarOpnd outputs the XML <variable-operand> element and its attributes.
//
// Arguments:
//
//    operand - [in] The VariableOperand to dump.
//
// Remarks:
//
//    This and the remaining operand-specific emit methods simply emit certain
//    property values as XML attributes.  For most properties, we use their
//    ToString methods.
//
//-----------------------------------------------------------------------------

void
XMLPlugIn::EmitVarOpnd
(
   Phx::IR::VariableOperand ^ operand
)
{
   this->xml->EmitElementOpen(L"variable-operand");
   this->xml->EmitAttribute(L"type", operand->Type->ToString());

   if (operand->IsMemory)
   {
      this->xml->EmitAttribute(L"sym", operand->Symbol->ToString());
   }
   else if (operand->IsTemporary)
   {
      this->xml->EmitAttribute(L"temporary", Phx::Output::ToString(operand->TemporaryId));
   }

   if (operand->IsRegister)
   {
      this->xml->EmitAttribute(L"reg", operand->Register->ToString());
   }
   else if (operand->IsAddress)
   {
      this->xml->EmitAttribute(L"is-address", L"true");
   }

   // Rather than force tools to duplicate the logic to produce display
   // text for all situations, we'll emit what is already provided by the
   // internal formatter.

   this->xml->EmitAttribute(L"display", operand->ToString());
   this->xml->EmitElementClose();
}

void
XMLPlugIn::EmitMemOpnd
(
   Phx::IR::MemoryOperand ^ operand
)
{
   this->xml->EmitElementOpen(L"memory-operand");
   this->xml->EmitAttribute(L"type", operand->Type->ToString());

   if (operand->Symbol)
   {
      this->xml->EmitAttribute(L"sym", operand->Symbol->ToString());
   }

   if (operand->BaseOperand)
   {
      this->xml->EmitAttribute(L"base", operand->BaseOperand->ToString());
   }

   if (operand->SegmentOperand)
   {
      this->xml->EmitAttribute(L"segment", operand->SegmentOperand->ToString());
   }

   if (operand->IndexOperand)
   {
      this->xml->EmitAttribute(L"index", operand->IndexOperand->ToString());
      this->xml->EmitAttribute(L"scale", Phx::Output::ToString(operand->Scale));
   }

   if (operand->ByteOffset != 0)
   {
      this->xml->EmitAttribute(L"offset",
         Phx::Output::ToString(operand->ByteOffset));
   }

   if (operand->IsAddress)
   {
      this->xml->EmitAttribute(L"is-address", L"true");
   }

   if (operand->IsGCCheckedBarrier)
   {
      this->xml->EmitAttribute(L"barrier", L"checked");
   }
   else if (operand->IsGCUncheckedBarrier)
   {
      this->xml->EmitAttribute(L"barrier", L"unchecked");
   }

   // Rather than force tools to duplicate the logic to produce display
   // text for all situations, we'll emit what is already provided by the
   // internal formatter.

   this->xml->EmitAttribute(L"display", operand->ToString());
   this->xml->EmitElementClose();
}

void
XMLPlugIn::EmitImmOpnd
(
   Phx::IR::ImmediateOperand ^ operand
)
{
   this->xml->EmitElementOpen(L"imm-operand");

   if (operand->IsSymbolicImmediate)
   {
      this->xml->EmitAttribute(L"sym", operand->Symbol->NameString);
   }
   else if (operand->IsPointer)
   {
      this->xml->EmitAttribute(L"pointer",
         Phx::Output::ToHexString(operand->IntValue, operand->Type->ByteSize ^ 2,
            false));
   }
   else if (operand->IsFloatImmediate)
   {
      this->xml->EmitAttribute(L"float",
         Phx::Output::ToString(operand->FloatValue));
   }
   else
   {
      this->xml->EmitAttribute(L"value", Phx::Output::ToString(operand->IntValue));
   }

   this->xml->EmitElementClose();
}

void
XMLPlugIn::EmitLabelOpnd
(
   Phx::IR::LabelOperand ^ operand
)
{
   this->xml->EmitElementOpen(L"label-operand");
   this->xml->EmitAttribute(L"name", this->SanitizeDecoratedName(operand->NameString));
   this->xml->EmitAttribute(L"kind", operand->LabelOperandKind.ToString());
   this->xml->EmitElementClose();
}

void
XMLPlugIn::EmitFuncOpnd
(
   Phx::IR::FunctionOperand ^ operand
)
{
   Phx::Symbols::FunctionSymbol ^ sym = operand->FunctionSymbol;

   this->xml->EmitElementOpen(L"function-operand");
   this->xml->EmitAttribute(L"sym",
      this->SanitizeDecoratedName(sym->ToString()));

   if (sym->IsDefinition)
   {
      this->xml->EmitAttribute(L"is-definition", "true");
   }

   if (sym->IsPublic || sym->IsPrivate)
   {
      this->xml->EmitAttribute(L"access",
         (sym->IsPublic) ? L"public" : L"private");
   }

   this->xml->EmitElementClose();
}

void
XMLPlugIn::EmitModRefOpnd
(
   Phx::IR::AliasOperand ^ operand
)
{
   this->xml->EmitElementOpen(L"modref-operand");
#if defined(PHX_DEBUG_DUMPS)
   this->xml->EmitAttribute(L"alias-tag", operand->ToString());
#endif // PHX_DEBUG_DUMPS
   this->xml->EmitElementClose();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Sanitize a decorated symbol name such that it is suitable for use a file
//    name.
//
// Arguments:
//
//    origName - [in] Any string, but intended to be a decorated name.
//
// Returns:
//
//    The original string with all '?<>:' replaced by '${}_'.
//
// Remarks:
//
//    This function simply replaces leading "?"s with the same number of "$"s.
//    "?" is an illegal character in file names, but we wish to use a
//    FunctionUnit's name as part of its XML file name.
//
//-----------------------------------------------------------------------------

System::String ^
XMLPlugIn::SanitizeDecoratedName
(
   System::String ^ origNameString
)
{
   System::String ^ resultString = origNameString->Replace("?", "$");
   resultString = resultString->Replace(":", "_");
   resultString = resultString->Replace("<", "{");
   resultString = resultString->Replace(">", "}");

   return resultString;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    XMLWriter::Open creates a new Phx::TextOutput object/file for subsequent
//    output.
//
// Arguments:
//
//    filename - [in] The name of the newly-created file.
//
// Remarks:
//
//    This uses the only instance of a lifetime in this sample.  A lifetime is
//    merely a memory allocation mechanism.  In our case, the TextOutput
//    object requires a lifetime when opening a file.  We will release the
//    lifetime when we close the file.
//
//    The file should be closed with XMLWriter::Close.
//
//-----------------------------------------------------------------------------

void
XMLWriter::Open
(
   System::String ^ filename
)
{
   this->lifetime = Phx::Lifetime::New(Phx::LifetimeKind::Temporary, nullptr);

   this->textOutputFile = Phx::TextOutput::New(this->lifetime, filename, true);

#if defined(PHX_DEBUG_CHECKS)
   Phx::Asserts::Assert(this->textOutputFile != nullptr);
#endif

   this->indentLevel = 0;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    XMLWriter::Close closes the file opened by XMLWriter::Open.
//
// Remarks:
//
//    This method also releases the lifetime that was created when the file
//    was opened.
//
//-----------------------------------------------------------------------------

void
XMLWriter::Close()
{
   this->textOutputFile->Delete();
   this->lifetime->Release();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    XMLWriter::EmitSimpleHeader outputs the standard boilerplate for an
//    XML file, including an optional xml-stylesheet directive.
//
// Arguments:
//
//    stylesheet - [in] The name of the XSLT stylesheet, or nullptr if no
//                 stylesheet directive is desired.
//
//-----------------------------------------------------------------------------

void
XMLWriter::EmitSimpleHeader
(
   System::String ^ stylesheet
)
{
   this->textOutputFile->Write
      (L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
   if (stylesheet != nullptr)
   {
      this->textOutputFile->Write(L"<?xml-stylesheet type=\"text/xsl\"");
      this->textOutputFile->Write
         (String::Format(L" href=\"{0}\"?>\n", stylesheet));
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    XMLWriter::IndentWith sets the string used at each level of indentation.
//    This would normally be a small number of blanks.  The default string is
//    three blanks.
//
// Arguments:
//
//    string - [in] A string which is repeated for each indentation level.
//          Turn off indentation, for compaction, with nullptr.
//
//-----------------------------------------------------------------------------

void
XMLWriter::IndentWith
(
   System::String ^   string
)
{
   this->indentStr = string;
   doIndent = string != nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    XMLWriter::EmitIndent outputs the indentation, without a
//    terminating linefeed, for the current indentation level.
//
//-----------------------------------------------------------------------------

void
XMLWriter::EmitIndent()
{
   if (doIndent)
   {
      for (int i = 0;  i < this->indentLevel;  i++)
      {
         this->textOutputFile->Write(indentStr);
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    XMLWriter::EmitSimpleElementOpen writes a "<tag-name>" element.  This
//    element has no attributes and must be closed by a subsequent element.
//    It is preceeded by the current indentation and followed by a linefeed.
//
// Arguments:
//
//    tag - [in] The tag name.
//
//-----------------------------------------------------------------------------

void
XMLWriter::EmitSimpleElementOpen
(
   System::String ^ tag
)
{
   this->EmitIndent();
   this->textOutputFile->Write(String::Format(L"<{0}>\n", tag));
   this->indentLevel++;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    XMLWriter::EmitElementOpen writes the start of an element,
//    "<tag-name", preceeded by the current indentation.  This may be
//    followed by subsequent attributes (see EmitAttribute).  It must be
//    terminated (see EmitAttributeClose) and subsequently closed, or simply
//    closed (see EmitElementClose).
//
// Arguments:
//
//    tag - [in] The tag name.
//
//-----------------------------------------------------------------------------

void
XMLWriter::EmitElementOpen
(
   System::String ^ tag
)
{
   this->EmitIndent();
   this->textOutputFile->Write(String::Format(L"<{0}", tag));
   this->indentLevel++;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    XMLWriter::EmitElementClose closes the most-recently opened element.
//    The overloads can close either an empty or non-empty element.  With
//    no argument, it will close an element opened with EmitElementOpen.
//    With an argument, it will close the element opened by
//    EmitSimpleElementOpen with the same tag.
//
// Arguments:
//
//    tag - [in] The tag name of the most recently opened element.
//
// Remarks:
//
//    This method also decrements the indentation level.
//
//-----------------------------------------------------------------------------

void
XMLWriter::EmitElementClose
(
   System::String ^ tag
)
{
   this->indentLevel--;
   this->EmitIndent();
   this->textOutputFile->Write(String::Format(L"</{0}>\n", tag));
}

void
XMLWriter::EmitElementClose()
{
   this->indentLevel--;
   this->textOutputFile->Write(L" />\n");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    XMLWriter::EmitAttribute outputs a 'attribute="value"' string to an
//    element begun with EmitElementOpen.
//
// Arguments:
//
//    attribute - [in] The attribute's name.
//    value - [in] The attribute's value.  Double quotes will be provided
//            around the string.
//
//-----------------------------------------------------------------------------

void
XMLWriter::EmitAttribute
(
   System::String ^ attribute,
   System::String ^ value
)
{
   System::String ^ quotedValue = this->XMLize(value);
   this->textOutputFile->Write
      (String::Format(L" {0}=\"{1}\"", attribute, quotedValue));
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    XMLWriter::EmitAttributeClose simply outputs a ">" followed by a newline.
//    It is intended to finish off a sequence of EmitAttribute's, leaving
//    the element open for content and subsequent closing.
//
//-----------------------------------------------------------------------------

void
XMLWriter::EmitAttributeClose()
{
   this->textOutputFile->Write(L">\n");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    XMLWriter::EmitText outputs an arbitrary string at the current
//    indentation level, followed by a newline.  It is provided for
//    generating element contents.
//
// Arguments:
//
//    text - [in] Any string.
//
//-----------------------------------------------------------------------------

void
XMLWriter::EmitText
(
   System::String ^   text
)
{
   this->EmitIndent();
   this->textOutputFile->Write(text);
   this->textOutputFile->WriteLine();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    XMLWriter::XMLize replaces characters that have special meaning
//    for XML/HTML with the appropriate escape sequence (entities).
//    Currently, handles &, <, >, ", ', and all control characters.
//
// Arguments:
//
//    string - [in] Any string.
//
// Returns:
//
//    The input string with special characters coverted to their entities.
//
//-----------------------------------------------------------------------------

System::String ^
XMLWriter::XMLize
(
   System::String ^ string
)
{
   int              stringLength;
   int              runBgn;
   int              i;
   wchar_t          ch;
   System::String ^ strOut = String::Empty;
   System::String ^ replacement;

   // Find runs of normal characters terminated by a special character.
   // Copy the run and the replacement to a new string.

   stringLength = 0;
   if (string != nullptr)
   {
      stringLength = string->Length;
   }

   runBgn = 0;
   replacement = String::Empty;

   for (i = 0; i < stringLength; i++)
   {
      ch = string[i];

      switch (ch)
      {
         case L'&':

            replacement = L"&amp;";
            break;

         case L'<':

            replacement = L"&lt;";
            break;

         case L'>':

            replacement = L"&gt;";
            break;

         case L'"':

            replacement = L"&quot;";
            break;

         case L'\'':

            replacement = L"&apos;";
            break;

         default:

            if ((ch < 0x20) && (ch != 9) && (ch != 10) && (ch != 13))
            {
               replacement = String::Format(L"&#{0};",
                  Phx::Output::ToHexString(ch, 2, true));
            }
            break;
      }

      if (replacement != String::Empty)
      {
         strOut = String::Format(L"{0}{1}{2}",
            strOut,
            string->Substring(runBgn, i - runBgn),
            replacement);
         runBgn = i + 1;
         replacement = String::Empty;
      }
   }

   if (runBgn < stringLength)
   {
      strOut = String::Format(L"{0}{1}", strOut, string->Substring(runBgn));
   }

   return strOut;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Visit a phase in the phase lists.
//
// Arguments:
//
//    phase - [in] The phase we're visiting.
//
// Returns:
//
//    We emit the phase name to the XML file.
//
//-----------------------------------------------------------------------------

void
XMLPhaseVisitor::Visit
(
   Phx::Phases::Phase ^ phase
)
{
   if (this->plugIn->ShouldDumpPhase(phase))
   {
      this->xml->EmitElementOpen(L"phase");
      this->xml->EmitAttribute(L"name", phase->NameString);
      this->xml->EmitElementClose();
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Visit a phase list before visiting its phases.
//
// Arguments:
//
//    phaseList - [in] The phaseList we're visiting.
//
// Returns:
//
//    We open an XML element to contain the phase list and emit the phase list
//    name to the XML file.
//
//-----------------------------------------------------------------------------

void
XMLPhaseVisitor::PreVisit
(
   Phx::Phases::PhaseList ^ phaseList
)
{
   this->xml->EmitElementOpen(L"subphase-list");
   this->xml->EmitAttribute(L"name", phaseList->NameString);
   this->xml->EmitAttributeClose();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Visit a phase list after visiting its phases.
//
// Arguments:
//
//    phaseList - [in] The phaseList we're visiting.
//
// Returns:
//
//    We close the XML element we opened during the pre-visit.
//
//-----------------------------------------------------------------------------

void
XMLPhaseVisitor::PostVisit
(
   Phx::Phases::PhaseList ^ /*phaseList*/
)
{
   this->xml->EmitElementClose(L"subphase-list");
}
