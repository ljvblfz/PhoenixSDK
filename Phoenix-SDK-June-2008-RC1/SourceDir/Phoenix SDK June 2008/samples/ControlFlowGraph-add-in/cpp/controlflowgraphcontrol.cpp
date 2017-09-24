//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    User control for the control flow graph.
//
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "controlflowgraphcontrol.h"

namespace ControlFlowGraph
{

//------------------------------------------------------------------------------
//
// Description:
//
//    Default constructor.  Initializes the control.
//
//------------------------------------------------------------------------------

ControlFlowGraphControl::ControlFlowGraphControl()
{
   this->InitializeComponent();
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Initializes this control.
//
// Arguments:
//
//    dte - The DTE instance for the IDE.
//    this->irWindow - The parent window instance for the IR window.
//    irWindowControl - The user control hosted on the IR tool window.
//
// Remarks:
//
//    Because CreateToolWindow2 does not call anything but the default
//    constructor, there is a separate initialization function.
//
//------------------------------------------------------------------------------

void
ControlFlowGraphControl::Initialize
(
   DTE2 ^            dte,
   Window ^          irToolWindow,
   IRWindowControl ^ irWindowControl
)
{
   this->applicationObject = dte;
   this->irWindow = irToolWindow;
   this->irControl = irWindowControl;

   // Initialize the Block display structure

   this->funcDisplayInfo = gcnew FuncDisplayInfo();
   this->funcDisplayInfo->PixelsPerUnit = 25;
   this->funcDisplayInfo->BlockTextFont = this->Font;

   // Set double buffering to true

   this->SetStyle(ControlStyles::UserPaint | ControlStyles::DoubleBuffer |
      ControlStyles::AllPaintingInWmPaint, true);

   // And finally initialize the events

   this->InitializeEvents();
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Updates the control with the new flow graph
//
// Arguments:
//
//    newGraph - The new Phx::Graphs::FlowGraph that will be displayed
//
//------------------------------------------------------------------------------

void
ControlFlowGraphControl::UpdateFlowGraph
(
   Phx::Graphs::FlowGraph ^ newGraph
)
{
   this->vScroll->Value = 0;
   this->funcDisplayInfo->Update(newGraph);

   // Finally, we need to update the ir window's text.  It need to
   // calculate the position of each block so that when we go to
   // highlight a section, we don't have to refresh all the text (which
   // may be slow).

   this->irControl->SetIR(this->funcDisplayInfo);
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Destructor for ControlFlowGraphControl.  Cleans up the components it owns.
//
//------------------------------------------------------------------------------

ControlFlowGraphControl::~ControlFlowGraphControl()
{
   if (components)
   {
      delete components;
   }
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Repaints the control with the current flow graph.
//
// Arguments:
//
//    sender - The instance that invoked this message.
//    arguments - The standard paint event arguments.
//
//------------------------------------------------------------------------------

void
ControlFlowGraphControl::OnPaint
(
   Object ^         sender,
   PaintEventArgs ^ arguments
)
{
   // First we get the Graphics context from the paint event arguments

   Graphics ^ context = arguments->Graphics;

   // Clear the window to white

   context->Clear(Color::White);

   // Update the scroll bar extents

   this->funcDisplayInfo->UpdateScrollBar(this->vScroll, -1);

   // And draw the graph

   this->funcDisplayInfo->Draw(context,
      System::Drawing::Size(Width, Height), this->vScroll);
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Called when the mouse moves inside this control.  Checks to see
//    whether a block should be highlighted.
//
// Arguments:
//
//    sender - The instance that invoked this message.
//    arguments - The standard mouse event arguments.
//
//------------------------------------------------------------------------------

void
ControlFlowGraphControl::OnMouseMove
(
   Object ^         sender,
   MouseEventArgs ^ arguments
)
{
   int index =
      this->funcDisplayInfo->GetMouseBlockIntersectionIndex(arguments->Location);

   if ((index != this->funcDisplayInfo->HighlightedBlockIndex) && (index != -1))
   {
      this->funcDisplayInfo->HighlightedBlockIndex = index;

      // Highlight the IR window

      this->irControl->HighlightIR(this->funcDisplayInfo->HighlightedBlockIndex,
         true);
      this->Refresh();
   }
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Called when the user double clicks the mouse inside the window.
//    When they double click on a node, the IR window is opened.
//
// Arguments:
//
//    sender - The instance that invoked this message.
//    arguments - The standard mouse event arguments.
//
//------------------------------------------------------------------------------

void
ControlFlowGraphControl::OnMouseDoubleClick
(
   Object ^         sender,
   MouseEventArgs ^ arguments
)
{
   // Return if there is somehow no highlighted index or 
   // if the user clicked the right mouse button and not the left.

   if ((this->funcDisplayInfo->HighlightedBlockIndex == -1)
      || (arguments->Button != System::Windows::Forms::MouseButtons::Left))
   {
      return;
   }
   this->irWindow->Visible = true;
   this->irControl->HighlightIR(this->funcDisplayInfo->HighlightedBlockIndex,
      true);
   this->irWindow->Activate();
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Called when the user single clicks the mouse inside the window.
//    When they right click on a node, the zoom state changes from
//    full in to full out or vice versa.
//
// Arguments:
//
//    sender - The instance that invoked this message.
//    arguments - The standard mouse event arguments.
//
//------------------------------------------------------------------------------

void
ControlFlowGraphControl::OnMouseClick
(
   Object ^         sender,
   MouseEventArgs ^ arguments
)
{
   // Two actions can occur:  The user can right click 
   // and have the view zoom all the way in or out, or
   // they can left click in an empty area and un-highlight anything.

   if (arguments->Button == System::Windows::Forms::MouseButtons::Right)
   {
      // If the zoom is anything but the MaxZoom, zoom to MaxZoom
      // Otherwise, zoom to min zoom.

      if (this->funcDisplayInfo->ZoomFactor != FuncDisplayInfo::MaxZoom)
      {
         // Zoom all the way in

         this->funcDisplayInfo->ZoomFactor = FuncDisplayInfo::MaxZoom;
      }
      else
      {
         this->funcDisplayInfo->ZoomFactor = FuncDisplayInfo::MinZoom;
      }
      this->funcDisplayInfo->UpdateScrollBar(this->vScroll,
         this->funcDisplayInfo->HighlightedBlockIndex);
      this->Refresh();
   }
   else if (arguments->Button == System::Windows::Forms::MouseButtons::Left)
   {
      int index =
         this->funcDisplayInfo->GetMouseBlockIntersectionIndex(arguments->Location);
      if (index == -1)
      {
            //unhighlight the block and any IR.

         this->funcDisplayInfo->HighlightedBlockIndex = -1;
         this->irControl->HighlightIR(-1, true);
         this->Refresh();
      }
   }
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Called when the mouse wheel is scrolled inside the window, either
//    to zoom in (when control is pressed) or just to scroll the flow
//    graph.
//
// Arguments:
//
//    sender - The instance that invoked this message.
//    arguments - The standard mouse event arguments.
//
//------------------------------------------------------------------------------

void
ControlFlowGraphControl::OnMouseWheel
(
   Object ^         sender,
   MouseEventArgs ^ arguments
)
{
   // Zoom when control is pressed, otherwise scroll

   float delta = (float)arguments->Delta / 120.f;
   if (ModifierKeys == Keys::Control)
   {
      if (delta < 0)
      {
         this->funcDisplayInfo->ZoomFactor *= -delta * (.75f);
      }
      else
      {
         this->funcDisplayInfo->ZoomFactor *= delta * (1.25f);
      }
      this->funcDisplayInfo->UpdateScrollBar(this->vScroll,
         this->funcDisplayInfo->HighlightedBlockIndex);
   }
   else
   {

      // Only scroll as far up or down as possible.  While the
      // VScrollBar class will restrict the Value property to the Min
      // and Max values, it is possible to cause heap corruption
      // if the value is not checked.

      int curVal = this->vScroll->Value;
      curVal -= (int)delta * this->vScroll->LargeChange;
      if (curVal < this->vScroll->Minimum)
      {
         curVal = this->vScroll->Minimum;
      }
      if (curVal > this->vScroll->Maximum)
      {
         curVal = this->vScroll->Maximum;
      }
      this->vScroll->Value = curVal;
   }
   this->Refresh();
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Called when the scroll bar is moved.
//
// Arguments:
//
//    sender - The instance that invoked this message.
//    arguments - The standard scroll bar event arguments.
//
//------------------------------------------------------------------------------

void
ControlFlowGraphControl::OnScroll
(
   Object ^          sender,
   ScrollEventArgs ^ arguments
)
{
   this->Refresh();
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Called when a key is pressed while the control has focus.  This
//    handles the + and - keys for zoom functionality.
//
// Arguments:
//
//    sender - The instance that invoked this message.
//    arguments - The standard key press event arguments.
//
//------------------------------------------------------------------------------

void
ControlFlowGraphControl::OnKeyPress
(
   Object ^            sender,
   KeyPressEventArgs ^ arguments
)
{
   if (arguments->KeyChar == '-')
   {
      this->funcDisplayInfo->ZoomFactor *= .75f;
   }
   else if (arguments->KeyChar == '=')
   {
      this->funcDisplayInfo->ZoomFactor *= 1.25f;
   }
   this->funcDisplayInfo->UpdateScrollBar(this->vScroll,
      this->funcDisplayInfo->HighlightedBlockIndex);
   this->Refresh();
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Called when the user clicks on a new IR block in the IR window.
//
// Arguments:
//
//    blockIndex - The index of selected basic block
//
// Remarks:
//    
//    The event is hooked this even for whenever the user clicks on a 
//    new block, and then the current selected block is updated and
//    refreshed.
//
//------------------------------------------------------------------------------

void
ControlFlowGraphControl::OnIRHighlightBlock
(
   unsigned int blockIndex
)
{
   this->funcDisplayInfo->HighlightedBlockIndex = blockIndex;

      // Scroll the scroll bar to gaurantee the block is visible.

   this->funcDisplayInfo->UpdateScrollBar(this->vScroll,
      this->funcDisplayInfo->HighlightedBlockIndex);
   this->Refresh();
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Event hookup helper method.
//
//------------------------------------------------------------------------------

void
ControlFlowGraphControl::InitializeEvents()
{
   // Grab the paint event

   this->Paint +=
      gcnew PaintEventHandler(this,
         &ControlFlowGraphControl::OnPaint);
   this->MouseMove +=
      gcnew MouseEventHandler(this,
         &ControlFlowGraphControl::OnMouseMove);
   this->MouseDoubleClick +=
      gcnew MouseEventHandler(this,
         &ControlFlowGraphControl::OnMouseDoubleClick);
   this->MouseClick +=
      gcnew MouseEventHandler(this,
         &ControlFlowGraphControl::OnMouseClick);
   this->MouseWheel +=
      gcnew MouseEventHandler(this,
         &ControlFlowGraphControl::OnMouseWheel);
   this->vScroll->Scroll +=
      gcnew ScrollEventHandler(this,
         &ControlFlowGraphControl::OnScroll);
   this->KeyPress +=
      gcnew KeyPressEventHandler(this,
         &ControlFlowGraphControl::OnKeyPress);
   this->irControl->OnHighlightIRTextBlock +=
      gcnew HighlightIRTextBlockHandler(this,
         &ControlFlowGraphControl::OnIRHighlightBlock);
}

} // ControlFlowGraph
