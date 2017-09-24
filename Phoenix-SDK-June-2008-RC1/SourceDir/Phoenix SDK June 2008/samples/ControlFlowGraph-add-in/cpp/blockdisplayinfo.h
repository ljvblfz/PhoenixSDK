//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Describes the way a basic block should be displayed by the Control Flow
//    Flow Graph add-in, and includes functionality to render the block and
//    its connector lines.
//
//------------------------------------------------------------------------------

#pragma once

// We will not 'use' any of the Phoenix namespaces in order to show the
// explicit class layout

#pragma region Using Namespaces

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Drawing;

#pragma endregion

namespace ControlFlowGraph
{

//------------------------------------------------------------------------------
// Description:
//
//    Describes the way a basic block should be displayed by the Control
//    Flow Graph.  Also included is functionality to draw the basic block
//    based on the description.
//
//------------------------------------------------------------------------------

public
ref class BlockDisplayInfo
{
public:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    The style of the block should be represented in.
   //
   // Remarks:
   //
   //    The style of the block refers primarily to the colors that the
   //    block is drawn with.  For instance, the start and end blocks
   //    are drawn with a color that is different from all internal blocks.
   //
   //---------------------------------------------------------------------------

   enum class BlockStyle
   {
      Start,
      End,
      Internal
   };

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Default constructor.  Constructs a new block.
   //
   //---------------------------------------------------------------------------

   BlockDisplayInfo();

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Updates/Sets this block's information, based on the 
   //    Phx::Graphics::BasicBlock passed.  Also updates the graphical 
   //    associated display parameters.
   //
   // Arguments:
   //
   //    block - The Phx::Graphs::BasicBlock that this control flow graph
   //    node should represent
   //
   //---------------------------------------------------------------------------

   void
   Update
   (
      Phx::Graphs::BasicBlock ^ block
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Draws the basic layout for this node, not including the node
   //    connectors.
   //
   // Arguments:
   //
   //    context - The Graphics context that will be used
   //    to paint the drawing surface.
   //    windowRect - The rectangle representing the extents of the drawing
   //    surface.
   //    transformation - The transformation that should be applied to the
   //    display extents.  This is used to described such transformations
   //    as the scaling and translation of a block to the center of the 
   //    screen.
   //    blockTextFont - The font that should be used to draw the text
   //    inside the block.
   //    highlight - If true, this node is highlighted, as if it was
   //    selection.
   //
   // Returns:
   //
   //    Whether this node was drawn based on the transformed position
   //    and the current window extents.
   //
   //---------------------------------------------------------------------------

   bool
   DrawNodeBasics
   (
      Graphics ^          context,
      Rectangle           windowRect,
      Drawing2D::Matrix ^ transformation,
      Font ^              blockTextFont,
      bool                highlight
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Draws the node connectors leaving this node and going to another
   //    node.
   //
   // Arguments:
   //
   //    context - The Graphics context that will be used
   //    to paint the drawing surface.
   //    trayIncrementWidth - The incremental arc width.
   //    highlightBlockId - The Id of the highlighted block.  If this block
   //    is the block being rendered, all outgoing nodes are colored a 
   //    specific color, and if not, any outgoing arcs to the highlighted
   //    block are colored a specific color
   //
   // Remarks:
   //
   //    The trayIncrementWidth determines how far an arc should extend 
   //    away from the nodes depending on how far the start node and end
   //    node of a connection are away from each other.
   //
   //---------------------------------------------------------------------------

   void
   DrawNodeConnections
   (
      Graphics ^   context,
      unsigned int trayIncrementWidth,
      int          highlightBlockId
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Gets or sets the size of this block.
   //
   // Remarks:
   //
   //    This is the untransformed size of the block.  Usually, all blocks
   //    have the same width, to keep the connection arcs simple and clean.
   //
   //---------------------------------------------------------------------------

   property SizeF BlockSize
   {
      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    Gets the size of this block.
      //
      // Returns:
      //
      //    The size of this block.
      //
      //---------------------------------------------------------------------

      SizeF
      get();

      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    Sets the size of this block.
      //
      // Arguments:
      //
      //    newSize - The new size of this block.
      //
      //---------------------------------------------------------------------

      void
      set(SizeF newSize);
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Gets a list of blocks that this block links to.
   //
   //---------------------------------------------------------------------------

   property List<BlockDisplayInfo ^> ^ NodesOut
   {
      //--------------------------------------------------------------------
      //
      // Description:
      //
      //    Gets a list of blocks that this block links to.
      //
      // Returns:
      //
      //    A list of blocks that this block links to.
      //
      //---------------------------------------------------------------------

      List<BlockDisplayInfo ^> ^
      get();
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Gets the id of this block.
   //
   // Remarks:
   //
   //    This id is the same as the original id of the BasicBlock in the
   //    FlowGraph.  The id is 1 based.
   //
   //---------------------------------------------------------------------------

   property unsigned int Id
   {
      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    Gets the id of this block.
      //
      // Remarks:
      //
      //    This id is the same as the original id of the BasicBlock in the
      //    FlowGraph.  The id is 1 based.
      //
      // Returns:
      //
      //    The id of this block.
      //
      //---------------------------------------------------------------------

      unsigned int
      get();
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Gets the ir for this basic block.
   //
   //---------------------------------------------------------------------------

   property String ^ BlockIRText
   {
      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    Gets the ir for this basic block.
      //
      // Returns:
      //
      //    The ir for this basic block.
      //
      //---------------------------------------------------------------------

      String ^
      get();
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Gets the transformed screen position of this block
   //
   //---------------------------------------------------------------------------

   property Rectangle TransformedRect
   {
      //---------------------------------------------------------------------
      //
      // Description:
      //
      //    Gets the transformed position of this block.
      //
      // Returns:
      //
      //    The transformed rectangle of this block.
      //
      //---------------------------------------------------------------------

      Rectangle
      get();
   }

private:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Helper method that updates the graphical parameters of this block
   //    in response to the current set of block information.
   //
   //---------------------------------------------------------------------------

   void
   UpdateGraphicsInfo();

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Helper method that updates the basic block information for this
   //    BlockDisplayInfo instance, given an instance of a
   //    Phx::Graphs::BasicBlock.
   //
   //---------------------------------------------------------------------------

   void
   UpdateBasicInfo
   (
      Phx::Graphs::BasicBlock ^ block
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Helper method that draws a single node connector between the start
   //    block and the end block, with the given tray increment, and whether
   //    the connector should be highlighted.
   //
   //---------------------------------------------------------------------------

   void
   DrawNodeConnector
   (
      Graphics ^         context,
      BlockDisplayInfo ^ startBlock,
      BlockDisplayInfo ^ endBlock,
      unsigned int       trayIncrementWidth,
      int                highlightBlockId
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Helper method that draws the shaded box for this node, given the
   //    transformed rectangle.
   //
   //---------------------------------------------------------------------------

   void
   DrawNodeBox
   (
      Graphics ^ context,
      Rectangle  boxRect,
      bool       highlight
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Helper method that renders the node text, with correct font sizing.
   //
   //---------------------------------------------------------------------------

   void
   DrawNodeText
   (
      Graphics ^ context,
      Rectangle  boxRect,
      Font ^     blockTextFont
   );

private:

   // The size of the block

   SizeF blockSize;

   // The transformed rectangle for this block, from the last time the 
   // node was drawn.  This is used for the node connectors, since we need 
   // the transformed position of individual nodes, which is not possible to 
   // get without first evaluating each node.

   Rectangle transformedRect;

   // The style of the block

   BlockStyle blockStyle;

   // The list of nodes that this node jumps to

   List<BlockDisplayInfo ^ > ^ nodesOut;

   // The id of this node

   unsigned int id;

   // The block display text

   String^ displayText;

   // The array of opcode lines (used to calculate the displayText
   // internally, instead of iterating the BasicBlock's instructions
   // each time.

   array<String ^ > ^ opCodeLines;

   // The IR disassembly text for this functionUnit

   String^ blockIRText;

   // The number of lines that this basic block shows (used to calculate
   // the required font size

   int numberLines;
};

} // ControlFlowGraph
