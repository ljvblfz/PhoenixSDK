//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Describes how a function should be displayed on the screen as a series
//    of basic blocks, and provides the generalized drawing functionality.
//
//------------------------------------------------------------------------------

#pragma once

#include "blockdisplayinfo.h"

#pragma region Using namespace declarations

using namespace System;
using namespace System::Windows::Forms;

#pragma endregion

namespace ControlFlowGraph
{

//------------------------------------------------------------------------------
//
// Description:
//
//    Describes a functional unit's display attributes and performs the
//    drawing operations.
//
//------------------------------------------------------------------------------

public
ref class FuncDisplayInfo
{
public:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Default constructor.
   //
   //---------------------------------------------------------------------------

   FuncDisplayInfo();

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Updates this functional unit flow graph representation with the
   //    specified flow graph.
   //
   // Arguments:
   //
   //    newGraph - The new flow graph with which this representation
   //    should be updated.
   //
   //---------------------------------------------------------------------------

   void
   Update
   (
      Phx::Graphs::FlowGraph ^ newGraph
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Draws this flow graph, given the graphics context and the window
   //    size.  Will also update the scroll bar's extents based on the
   //    size of the flow graph.
   //
   // Arguments:
   //
   //    context - The graphics context for the parent window.
   //    windowSize - The drawing size of the parent window.
   //    vScroll - The vertical scroll bar, which is updated based on the
   //    size of the flow graph and the current zoom levels.
   //
   //---------------------------------------------------------------------------

   void
   FuncDisplayInfo::Draw
   (
      Graphics ^   context,
      Size         windowSize,
      VScrollBar ^ vScroll
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Computes the block id at a specified position.
   //
   // Arguments:
   //
   //    mousePosition - The position at which the block Id should be
   //    determined.
   //
   // Returns:
   //
   //    The basic block ID of the block at mousePosition, or -1 if no
   //    block exists at this position.
   //
   //---------------------------------------------------------------------------

   int
   GetMouseBlockIntersectionIndex
   (
      Point mousePosition
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Updates the set of scroll bars based on the current
   //    extents of the window, and scrolls to the selected index
   //    (of a block) if desired.
   //
   // Arguments:
   //
   //    vScroll - The vertical scroll bar.
   //    scrollToIndex - If a valid index, the scrollbar value is updated
   //    to scroll to a specific block.
   //
   //---------------------------------------------------------------------------

   void
   UpdateScrollBar
   (
      VScrollBar ^ vScroll,
      int          scrollToIndex
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Gets the list of BasicBlockInfo structures that 
   //    make up this functional unit.
   //
   //---------------------------------------------------------------------------

   property List<BlockDisplayInfo ^> ^ DisplayBlocks
   {

      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    Gets the list of BasicBlockInfo structures that 
      //    make up this functional unit.
      //
      // Returns:
      //
      //    The list of BasicBlockInfo structures that make up this 
      //    functional unit.
      //
      //---------------------------------------------------------------------

      List<BlockDisplayInfo^>^
      get();
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Gets or sets the number of pixels that corresponds to a single 
   //    abstract “unit”.  Note that this only affects the vertical size 
   //    of a block.
   //
   //---------------------------------------------------------------------------

   property unsigned int PixelsPerUnit
   {
      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    Gets the number of pixels that corresponds to a single 
      //    abstract unit.  Note that this only affects the vertical size 
      //    of a block.
      //
      // Returns:
      //
      //    The number of pixels that correspond a vertical unit.
      //
      //---------------------------------------------------------------------

      unsigned int
      get();

      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    Gets the number of pixels that corresponds to a single 
      //    abstract unit.  Note that this only affects the vertical size 
      //    of a block.
      //
      // Arguments:
      //
      //    pixels - The number of pixels that correspond a vertical unit.
      //
      //---------------------------------------------------------------------

      void
      set(unsigned int pixels);
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Gets or sets the zoom factor for the flow graph.
   //
   //---------------------------------------------------------------------------

   property float ZoomFactor
   {
      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    Gets the zoom factor for the flow graph.
      //
      // Returns:
      //
      //    The zoom factor for the flow graph.
      //
      //---------------------------------------------------------------------

      float
      get();

      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    Sets the zoom factor for the flow graph.
      //
      // Arguments:
      //
      //    zoom - The zoom factor for the flow graph.
      //
      //---------------------------------------------------------------------

      void
      set(float zoom);
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Sets the font used to draw the text in the flow graph.
   //
   //---------------------------------------------------------------------------

   property Font ^ BlockTextFont
   {
      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    Sets the font used to draw the text in the flow graph.
      //
      // Arguments:
      //
      //    font - The font used in the basic blocks of the flow graph.
      //
      //---------------------------------------------------------------------

      void
      set(Font ^ font);
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Get or sets the block id of the highlighted block.
   //
   //---------------------------------------------------------------------------

   property int HighlightedBlockIndex
   {
      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    Gets the highlighted block's index.
      //
      // Returns:
      //
      //    The highlighted block's index.
      //
      //---------------------------------------------------------------------

      int
      get();

      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    Sets the highlighted block.
      //
      // Arguments:
      //
      //    index - The index of the block to highlight.
      //
      //---------------------------------------------------------------------

      void
      set(int index);
   }

   static const float MinZoom = .25f;
   static const float MaxZoom = 2.5f;

private:

   // The list of display node information.  The display node information
   // is built from the flow graph

   List<BlockDisplayInfo ^ > ^ displayBlocks;

   // The number of units that the entire set of boxes occupies as a whole.

   float windowLength;

   // The zoom factor

   float zoomFactor;

   // The minimum viewport value

   float viewportMin;

   // The maximum viewport vlaue

   float viewportMax;

   // The focus of the current viewport

   float viewportFocus;

   // The number of pixels per unit in the vertical direction.

   unsigned int verticalPixelsPerUnit;

   // The font of the parent window

   Font ^ blockTextFont;

   // The id of the highlighted block (-1 if no block is highlighted)

   int curHighlightedBlockIndex;

   int firstDrawnBlockIndex;
   int lastDrawnBlockIndex;

}; // FuncDisplayInfo

} // ControlFlowGraph
