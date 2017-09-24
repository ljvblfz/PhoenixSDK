//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    User control for the IR window.
//
//------------------------------------------------------------------------------

#pragma once

#include "funcdisplayinfo.h"

#pragma region Using namespaces

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections::Generic;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

#pragma endregion

namespace ControlFlowGraph
{

//------------------------------------------------------------------------------
//
// Description:
//
//    Delegate for OnHighlightIRTextBlock
//
// Arguments:
//
//    blockId - the id of the newly highlighted block.
//
//------------------------------------------------------------------------------

public delegate void HighlightIRTextBlockHandler(unsigned int blockId);

//------------------------------------------------------------------------------
//
// Description:
//
//    User control class for the IR tool window.
//
//------------------------------------------------------------------------------

public
ref class IRWindowControl : public UserControl
{
private:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Simple structure that contains the textual IR start and IR length
   //    properties for a given block inside the RichTextEdit control.  This
   //    aids in highlighting IR.
   //
   //---------------------------------------------------------------------------

   ref struct BlockIRExtents
   {
      unsigned int IRStart;
      unsigned int IRLength;
   };

public:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Event that is invoked when the user clicks on a block of text in
   //    the RichTextEditControl.  It is invoked with the Id of the 
   //    highlighted block id.
   //
   //---------------------------------------------------------------------------

   event HighlightIRTextBlockHandler ^ OnHighlightIRTextBlock;

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Default constructor.
   //
   //---------------------------------------------------------------------------

   IRWindowControl();

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Sets the ir for the IR window given a FuncDisplayInfo instance.
   //
   // Arguments:
   //
   //    functionInfo - The functional unit that the IR window should be updated
   //    to reflect.
   //
   //---------------------------------------------------------------------------

   void
   SetIR
   (
      FuncDisplayInfo ^ functionInfo
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Highlights the IR of the block given by an id.  Also scrolls
   //    to the block if neccesary.
   //
   // Arguments:
   //
   //    blockIndex - The id of the block to highlight.
   //    scroll - If true, the window will scroll to show the highlighted
   //    block.
   //
   //---------------------------------------------------------------------------

   void
   HighlightIR
   (
      int  blockIndex,
      bool scroll
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Highlights the IR of the block given by a mouse position.
   //
   // Arguments:
   //
   //    mousePosition - The position of the mouse that indicates
   //    which block should be highlighted.
   //
   //---------------------------------------------------------------------------

   int
   HighlightIR
   (
      Point mousePosition
   );

protected:

   //------------------------------------------------------------------------------
   //
   // Description:
   //
   //    Destructor for IRWindowControl.  Cleans up the components it owns.
   //
   //------------------------------------------------------------------------------

   virtual ~IRWindowControl();

private:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Event handler for the MouseClick event raised by the RIchTextEdit
   //    control.
   //
   // Arguments:
   //
   //    sender - The instance that invoked this message.
   //    arguments - The standard mouse event arguments.
   //
   //---------------------------------------------------------------------------

   void
   OnMouseClick
   (
      Object ^         sender,
      MouseEventArgs ^ arguments
   );

   // The colors used to highlight the text.

   Color selectionBackColor;
   Color normalBackColor;

   // The extents of the current function IR's block start and end
   // positions, for highlighting.

   List<BlockIRExtents ^ > ^ funcIRExtents;
   int                       curHighlightedBlockIndex;

   System::ComponentModel::Container ^ components;
   RichTextBox ^                       irTextBox;

   void
   InitializeComponent();

};

} // ControlFlowGraph
