//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    User control for the control flow graph.
//
//------------------------------------------------------------------------------

#pragma once

#include "irwindowcontrol.h"
#include "funcdisplayinfo.h"

#pragma region Using Namespaces

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace Extensibility;
using namespace EnvDTE;
using namespace EnvDTE80;

#pragma endregion

namespace ControlFlowGraph
{

//------------------------------------------------------------------------------
//
// Description:
//
//    User control that displays a control flow graph.
//
//------------------------------------------------------------------------------

public
ref class ControlFlowGraphControl : public UserControl
{
public:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Default constructor.  Initializes the control.
   //
   //---------------------------------------------------------------------------

   ControlFlowGraphControl();

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Initializes this control.
   //
   // Arguments:
   //
   //    dte - The DTE instance for the IDE.
   //    irWindow - The parent window instance for the IR window.
   //    irWindowControl - The user control hosted on the IR tool window.
   //
   // Remarks:
   //
   //    Because CreateToolWindow2 does not call anything but the default
   //    constructor, there is a separate initialization function.
   //
   //---------------------------------------------------------------------------

   void
   Initialize
   (
      DTE2 ^            dte,
      Window ^          irWindow,
      IRWindowControl ^ irWindowControl
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Updates the control with the new flow graph
   //
   // Arguments:
   //
   //    newGraph - The new Phx::Graphs::FlowGraph that will be displayed
   //
   //---------------------------------------------------------------------------

   void
   UpdateFlowGraph
   (
      Phx::Graphs::FlowGraph ^ newGraph
   );

protected:

   //------------------------------------------------------------------------------
   //
   // Description:
   //
   //    Destructor for ControlFlowGraphControl.  Cleans up the components it owns.
   //
   //------------------------------------------------------------------------------

   virtual ~ControlFlowGraphControl();

private:

   //---------------------------------------------------------------------------
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
   //---------------------------------------------------------------------------

   void
   OnPaint
   (
      Object ^         sender,
      PaintEventArgs ^ arguments
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Called when the mouse moved inside this control.  Checks to see.
   //    whether a block should be highlighted.
   //
   // Arguments:
   //
   //    sender - The instance that invoked this message.
   //    arguments - The standard mouse event arguments.
   //
   //---------------------------------------------------------------------------

   void
   OnMouseMove
   (
      Object ^         sender,
      MouseEventArgs ^ arguments
   );

   //---------------------------------------------------------------------------
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
   //---------------------------------------------------------------------------

   void
   OnMouseDoubleClick
   (
      Object ^         sender,
      MouseEventArgs ^ arguments
   );

   //---------------------------------------------------------------------------
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
   //---------------------------------------------------------------------------

   void
   OnMouseClick
   (
      Object ^         sender,
      MouseEventArgs ^ arguments
   );

   //---------------------------------------------------------------------------
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
   //---------------------------------------------------------------------------

   void
   OnMouseWheel
   (
      Object ^         sender,
      MouseEventArgs ^ arguments
   );

   //---------------------------------------------------------------------------
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
   //---------------------------------------------------------------------------

   void
   OnScroll
   (
      Object ^          sender,
      ScrollEventArgs ^ arguments
   );

   //---------------------------------------------------------------------------
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
   //---------------------------------------------------------------------------

   void
   OnKeyPress
   (
      Object ^            sender,
      KeyPressEventArgs ^ arguments
   );

   //---------------------------------------------------------------------------
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
   //---------------------------------------------------------------------------

   void
   OnIRHighlightBlock
   (
      unsigned int blockIndex
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Event hookup helper method.
   //
   //---------------------------------------------------------------------------

   void
   InitializeEvents();

   // The dte interface instance

   DTE2 ^ applicationObject;

   // The functional unit display info.

   FuncDisplayInfo ^ funcDisplayInfo;

   // The ir tool window control

   IRWindowControl ^ irControl;

   // The ir tool window Window instance

   Window ^ irWindow;

   // Required designer variable

   System::ComponentModel::Container ^ components;
   VScrollBar ^                        vScroll;

#pragma region Windows Form Designer generated code
   void
   InitializeComponent
   (
      void
   )
   {
      this->vScroll = (gcnew VScrollBar());
      this->SuspendLayout();

         // vScroll

      this->vScroll->Dock = DockStyle::Right;
      this->vScroll->Location = Point(781, 0);
      this->vScroll->Name = L"vScroll";
      this->vScroll->Size = System::Drawing::Size(21, 791);
      this->vScroll->TabIndex = 1;

         // ControlFlowGraphControl

      this->AutoScaleDimensions = SizeF(8, 16);
      this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
      this->Controls->Add(this->vScroll);
      this->Name = L"ControlFlowGraphControl";
      this->Size = System::Drawing::Size(802, 791);
      this->ResumeLayout(false);
   }
#pragma endregion
};

} // ControlFlowGraph
