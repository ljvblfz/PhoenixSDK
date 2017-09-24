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

#include "blockdisplayinfo.h"

namespace ControlFlowGraph
{

//------------------------------------------------------------------------------
//
// Description:
//
//    Default constructor.  Constructs a new block.
//
//------------------------------------------------------------------------------

BlockDisplayInfo::BlockDisplayInfo()
{
   this->nodesOut = gcnew List<BlockDisplayInfo ^>;
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
BlockDisplayInfo::Update
(
   Phx::Graphs::BasicBlock ^ block
)
{
   this->UpdateBasicInfo(block);
   this->UpdateGraphicsInfo();
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

bool
BlockDisplayInfo::DrawNodeBasics
(
   Graphics ^          context,
   Rectangle           windowRect,
   Drawing2D::Matrix ^ transformation,
   Font ^              blockTextFont,
   bool                highlight
)
{
   array<PointF> ^ points = gcnew array<PointF>(2);
   points[0] = PointF(0.f, 0.f);
   points[1] = PointF(this->blockSize.Width, this->blockSize.Height);

   // Transform the node rectangle to its final position by the transformation
   // matrix

   transformation->TransformPoints(points);
   Rectangle finalRect = Rectangle((int)points[0].X, (int)points[0].Y,
      (int)(points[1].X - points[0].X), (int)(points[1].Y - points[0].Y));

   this->transformedRect = finalRect;

   if (windowRect.IntersectsWith(finalRect))
   {
      this->DrawNodeBox(context, finalRect, highlight);
      this->DrawNodeText(context, finalRect, blockTextFont);

      return true;
   }

   return false;
}

//------------------------------------------------------------------------------
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
//    highlight - If true, this node is highlighted, as if it was
//    selection.
//
// Remarks:
//
//    The trayIncrementWidth determines how far an arc should extend 
//    away from the nodes depending on how far the start node and end
//    node of a connection are away from each other.
//
//------------------------------------------------------------------------------

void
BlockDisplayInfo::DrawNodeConnections
(
   Graphics ^    context,
   unsigned int  trayIncrementWidth,
   int           highlightBlockId
)
{
   for each (BlockDisplayInfo ^ outBlock in this->nodesOut)
   {
      this->DrawNodeConnector(context, this, outBlock,
         trayIncrementWidth, highlightBlockId);
   }
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Gets the size of this block.
//
//------------------------------------------------------------------------------

SizeF
BlockDisplayInfo::BlockSize::get()
{
   return blockSize;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Sets the size of this block.
//
// Arguments:
//
//    newSize - The new size of this block.
//
//------------------------------------------------------------------------------

void
BlockDisplayInfo::BlockSize::set
(
   SizeF newSize
)
{
   blockSize = newSize;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Gets a list of blocks that this block links to.
//
// Returns:
//
//    A list of blocks that this block links to.
//
//------------------------------------------------------------------------------

List<BlockDisplayInfo ^ > ^ BlockDisplayInfo::NodesOut::get()
{
   return this->nodesOut;
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

unsigned int BlockDisplayInfo::Id::get()
{
   return this->id;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Gets the ir for this basic block.
//
// Returns:
//
//    The ir for this basic block.
//
//------------------------------------------------------------------------------

String ^ BlockDisplayInfo::BlockIRText::get()
{
   return this->blockIRText;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Gets the transformed position of this block.
//
// Returns:
//
//    The transformed rectangle of this block.
//
//------------------------------------------------------------------------------

Rectangle
BlockDisplayInfo::TransformedRect::get()
{
   return this->transformedRect;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Helper method that updates the graphical parameters of this block
//    in response to the current set of block information.
//
//------------------------------------------------------------------------------

void
BlockDisplayInfo::UpdateGraphicsInfo()
{
   this->numberLines = this->opCodeLines->Length + 2;
   this->displayText = "";

   for each (String ^ line in this->opCodeLines)
   {
      this->displayText += line + "\n";
   }

   if (this->numberLines < 3)
   {
      this->numberLines = 3;
   }

   blockSize = SizeF(1.f, (float)this->numberLines);
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Helper method that updates the basic block information for this
//    BlockDisplayInfo instance, given an instance of a
//    Phx::Graphs::BasicBlock.
//
//------------------------------------------------------------------------------

void
BlockDisplayInfo::UpdateBasicInfo
(
   Phx::Graphs::BasicBlock ^ block
)
{
   this->id = block->Id;

   if (block->FirstInstruction->Opcode ==
      Phx::Common::Opcode::Start)
   {
      this->blockStyle = this->BlockStyle::Start;
   }
   else if (block->FirstInstruction->Opcode ==
      Phx::Common::Opcode::End)
   {
      this->blockStyle = this->BlockStyle::End;
   }
   else
   {
      this->blockStyle = this->BlockStyle::Internal;
   }

   // Set up the opcode lines (used to calculate the display
   // text from the available rectangle space

   this->opCodeLines = gcnew array <String ^ >(block->InstructionCount);

   // The current instruct index

   unsigned int curInstrIndex = 0;

   System::Text::StringBuilder ^ ir = gcnew System::Text::StringBuilder();

   Phx::IR::Instruction ^ curInstr;

   // And the IR for the functionUnit

   curInstr = block->FirstInstruction;
   while ((curInstr != nullptr) && (curInstrIndex < block->InstructionCount))
   {
      if (curInstr->IsLabelInstruction)
      {
         ir->AppendLine();
      }
      ir->AppendLine(curInstr->ToString());

      // Create a new opcode string

      this->opCodeLines[curInstrIndex] = curInstr->OpcodeToString();

      curInstr = curInstr->Next;
      curInstrIndex++;
   }

   this->blockIRText = ir->ToString();
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Helper method that draws a single node connector between the start
//    block and the end block, with the given tray increment, and whether
//    the connector should be highlighted.
//
//------------------------------------------------------------------------------

void
BlockDisplayInfo::DrawNodeConnector
(
   Graphics ^         context,
   BlockDisplayInfo ^ startBlock,
   BlockDisplayInfo ^ endBlock,
   unsigned int       trayIncrementWidth,
   int                highlightBlockId
)
{
   float        arrowEdgeSize = 5.f;
   int          idDiff = startBlock->Id - endBlock->Id;
   unsigned int startOffset, endOffset;
   float        penWidth = 0.5f;
   Color        penColor = Color::Black;

   // Turn on antialiasing for smoother arcs.

   context->SmoothingMode = Drawing2D::SmoothingMode::AntiAlias;

   // Depending on whether the jump is forward or backward in the function,
   // the arc should either be drawn on the left or the right of the
   // rectangle.  Determine the start values of the start and end of the
   // arc.

   if (idDiff < 0)
   {
      startOffset = (int)(2.f * startBlock->TransformedRect.Height / 3.f);
      endOffset = (int)(endBlock->TransformedRect.Height / 3.f);
   }
   else
   {
      startOffset = (int)(startBlock->TransformedRect.Height / 3.f);
      endOffset = (int)(2.f * endBlock->TransformedRect.Height / 3.f);
   }

   // Special highlighting for highlighted nodes

   if (highlightBlockId == startBlock->Id)
   {
      penWidth = 3.0f;
      penColor = Color::Blue;
   }
   else if (highlightBlockId == endBlock->Id)
   {
      penWidth = 3.0f;
      penColor = Color::Red;
   }

   // Create a new pen to draw the arc with with an arrow for the cap.

   Drawing2D::AdjustableArrowCap ^ arrows =
      gcnew Drawing2D::AdjustableArrowCap(arrowEdgeSize * .5f,
         arrowEdgeSize);
   Pen ^ pen =
      gcnew Pen(penColor, penWidth);
   pen->CustomEndCap = arrows;
   pen->DashStyle = Drawing2D::DashStyle::Custom;
   pen->DashPattern = gcnew array<float>
   {
      5.f, 5.f
   };

   Point startPoint, endPoint, cp1, cp2;

   // Calculation of the end point.

   if (idDiff > 0)
   {

      Point endBlockLeftSidePoint = Point(endBlock->TransformedRect.X,
         endBlock->TransformedRect.Y  + endOffset);
      Point startBlockLeftSidePoint = Point(startBlock->TransformedRect.X,
         startBlock->TransformedRect.Y + startOffset);

      startPoint = startBlockLeftSidePoint;
      endPoint = endBlockLeftSidePoint;
      cp1 = Point(startBlockLeftSidePoint.X - trayIncrementWidth * idDiff,
         startBlockLeftSidePoint.Y);
      cp2 = Point(endBlockLeftSidePoint.X - trayIncrementWidth * idDiff,
         endBlockLeftSidePoint.Y);
   }
   else
   {
      Point endBlockRightSidePoint =
         Point(endBlock->TransformedRect.X + endBlock->TransformedRect.Width,
            endBlock->TransformedRect.Y  + endOffset);
      Point startBlockRightSidePoint =
         Point(startBlock->TransformedRect.X +
            startBlock->TransformedRect.Width,
            startBlock->TransformedRect.Y + startOffset);

      startPoint = startBlockRightSidePoint;
      endPoint = endBlockRightSidePoint;
      cp1 = Point(startBlockRightSidePoint.X - trayIncrementWidth * idDiff,
         startBlockRightSidePoint.Y);
      cp2 = Point(endBlockRightSidePoint.X - trayIncrementWidth * idDiff,
         endBlockRightSidePoint.Y);
   }

   context->DrawBezier(pen, startPoint, cp1, cp2, endPoint);

   delete pen;

   context->SmoothingMode = Drawing2D::SmoothingMode::Default;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Helper method that draws the shaded box for this node, given the
//    transformed rectangle.
//
//------------------------------------------------------------------------------

void
BlockDisplayInfo::DrawNodeBox
(
   Graphics ^ context,
   Rectangle  boxRect,
   bool       highlight
)
{
   Color startColor, endColor;
   float penWidth;

   penWidth = .5f;

   // Check for special highlighting of selected items

   if (highlight)
   {
      penWidth = 3.0f;
      startColor = Color::LightGray;
      startColor = Color::Green;
   }
   else if (this->blockStyle == BlockDisplayInfo::BlockStyle::Internal)
   {
      startColor = Color::LightGray;
      endColor = Color::LightBlue;
   }
   else
   {
      startColor = Color::LightGray;
      endColor = Color::LightSalmon;
   }

   // Create a new gradient brush in which to render the box.

   Pen ^ pen =
      gcnew Pen(Color::Black, penWidth);

   Drawing2D::LinearGradientBrush ^ brush = gcnew
      Drawing2D::LinearGradientBrush(
         Point(boxRect.X, boxRect.Y),
         Point(boxRect.X + boxRect.Width,
            boxRect.Y + boxRect.Height),
         startColor, endColor);

   context->FillRectangle(brush, boxRect);
   context->DrawRectangle(pen, boxRect);

   // Make sure to release any GDI resources that were created, so prevent
   // leakage.

   delete brush;
   delete pen;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Helper method that renders the node text, with correct font sizing.
//
//------------------------------------------------------------------------------

void
BlockDisplayInfo::DrawNodeText
(
   Graphics ^ context,
   Rectangle  boxRect,
   Font ^     blockTextFont
)
{
   // First we ned to determine what size text we need to use to fit
   // all the lines of text in the box, with some margins.

   StringFormat ^ format = gcnew StringFormat(StringFormatFlags::FitBlackBox);
   format->LineAlignment = StringAlignment::Near;
   format->Alignment = StringAlignment::Near;

   // Calculate the font height, in pixels.  Approximately 25% of the 
   // space of the font must go to the "tray width".

   float fontheight = boxRect.Height / (this->numberLines * 1.25f);

   // If the font height won't be positive, don't bother drawing anything

   if (fontheight <= 0)
   {
      return;
   }

   // Create a new font from the old font and set a new size.

   Font ^ newFont = gcnew Font(blockTextFont->FontFamily,
      fontheight, blockTextFont->Style, GraphicsUnit::Pixel);

   // Draw the node id

   context->DrawString("[" + this->id.ToString() + "]", newFont, Brushes::Black,
      boxRect, format);

   format->LineAlignment = StringAlignment::Center;
   format->Alignment = StringAlignment::Center;

   // Draw the instruction text.

   context->DrawString(this->displayText, newFont, Brushes::Black,
      boxRect, format);
}

} // ControlFlowGraph
