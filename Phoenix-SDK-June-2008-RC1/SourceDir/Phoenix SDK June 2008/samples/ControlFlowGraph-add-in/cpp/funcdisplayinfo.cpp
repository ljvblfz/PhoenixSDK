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

#include "funcdisplayinfo.h"

namespace ControlFlowGraph
{

//------------------------------------------------------------------------------
//
// Description:
//
//    Default constructor.
//
//------------------------------------------------------------------------------

FuncDisplayInfo::FuncDisplayInfo()
{
   this->displayBlocks = gcnew List <BlockDisplayInfo ^ >();

   // Set the basic properties

   this->windowLength = 100.f;
   this->zoomFactor = 1.f;
   this->verticalPixelsPerUnit = 50;
   this->curHighlightedBlockIndex = -1;
   this->firstDrawnBlockIndex = this->lastDrawnBlockIndex = -1;
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
FuncDisplayInfo::Update
(
   Phx::Graphs::FlowGraph ^ newGraph
)
{

   // First kill the existing display info and do some reset

   this->displayBlocks->Clear();
   this->curHighlightedBlockIndex = -1;

   if (newGraph == nullptr)
   {
      return;
   }

   this->displayBlocks->Capacity = newGraph->NodeCount;

   // First, we insert all the base elements into the this->displayBlocks list,
   // and do the base initialization too.

   // A "unit" is 1.0. There should be a quarter unit between each of the
   // nodes

   this->windowLength = 1.0f;

   Phx::Graphs::BasicBlock ^ currentBlock = newGraph->StartBlock;

   while (currentBlock != nullptr)
   {
      BlockDisplayInfo ^ curBlockInfo = gcnew BlockDisplayInfo();
      curBlockInfo->Update(currentBlock);
      this->windowLength += curBlockInfo->BlockSize.Height + 1.f;

      this->displayBlocks->Add(curBlockInfo);

      // Iterate to the next block

      currentBlock = currentBlock->Next;
   }

   // Now go back through the list, and hook up the jumps
   // now that we have them all in place.

   currentBlock = newGraph->StartBlock;
   int currentIndex = 0;

   while (currentBlock != nullptr)
   {
      BlockDisplayInfo ^      blockInfo;
      Phx::Graphs::FlowEdge ^ curEdge;

      blockInfo = this->displayBlocks[currentIndex];

      // Then connect up the out/in nodes

      curEdge = currentBlock->SuccessorEdgeList;
      while (curEdge != nullptr)
      {
         Phx::Graphs::BasicBlock ^ successorBlock = curEdge->SuccessorNode;
         BlockDisplayInfo ^        succBlockInfo = nullptr;

         // Do a linear search to find the connecting block by
         // id (not index), and then connect it up.

         for each (BlockDisplayInfo ^ block in this->displayBlocks)
         {
            if (block->Id == successorBlock->Id)
            {
               succBlockInfo = block;
               break;
            }
         }
         if (succBlockInfo != nullptr)
         {
            // Add one for going out

            blockInfo->NodesOut->Add(succBlockInfo);
         }

         curEdge = curEdge->NextSuccessorEdge;
      }

      // Go to the next bb and next displayblocks node

      currentBlock = currentBlock->Next;
      currentIndex++;
   }
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
FuncDisplayInfo::Draw
(
   Graphics ^   context,
   Size         windowSize,
   VScrollBar ^ vScroll
)
{
   if (this->displayBlocks->Count == 0)
   {
      // Early exit, no draw

      return;
   }

   // Update the windowSize to not include the width of the vertical scroll
   // bar

   Size adjustedWindowSize =
      Size(windowSize.Width - vScroll->Width,
         windowSize.Height);

   // The origin of the window

   Point origin =
      Point(adjustedWindowSize.Width / 2, 0);

   int hSize = (int)(this->zoomFactor * adjustedWindowSize.Width / 3.f);
   int hTranslate = origin.X - hSize / 2;
   int vTranslate = vScroll->Value * this->verticalPixelsPerUnit;

   // We construct a matrix to map between the untransformed size of the blocks
   // to the final screen position of the basic block.

   Drawing2D::Matrix ^ matrixTransform =
      gcnew Drawing2D::Matrix();
   matrixTransform->Scale(this->zoomFactor *
      (float)adjustedWindowSize.Width / 3.f,
      (float)this->zoomFactor * this->verticalPixelsPerUnit,
      Drawing2D::MatrixOrder::Append);
   matrixTransform->Translate((float)hTranslate,
      this->verticalPixelsPerUnit -
      (float)vTranslate,
      Drawing2D::MatrixOrder::Append);

   // Set the min and maximum drawn block indices's 
   // (for faster mouse selection on large)
   // functions

   this->firstDrawnBlockIndex = this->lastDrawnBlockIndex = -1;

   Rectangle windowRect = Rectangle(Point(0, 0), adjustedWindowSize);

   for (int i = 0; i < this->displayBlocks->Count; i++)
   {
      BlockDisplayInfo ^ currentBlock = this->displayBlocks[i];

      bool drawn =
         currentBlock->DrawNodeBasics(context, windowRect,
            matrixTransform, this->blockTextFont,
            i == this->curHighlightedBlockIndex);
      if (drawn)
      {
         if (this->firstDrawnBlockIndex == -1)
         {
            this->firstDrawnBlockIndex = i;
         }
         this->lastDrawnBlockIndex = i;
      }
      matrixTransform->Translate(0.f,
         (currentBlock->BlockSize.Height
            + 1.f) * this->zoomFactor * this->verticalPixelsPerUnit,
         Drawing2D::MatrixOrder::Append);
   }

   // Calculate the tray increment width.  This is equal to
   // a third of the window width*this->zoomFactor/numberNodes

   unsigned int trayIncrement =
      (int)(adjustedWindowSize.Width / (2 * (this->displayBlocks->Count - 2)));

   // Retrieve the ID of the current highlighted block (because the
   // individual blocks don't have any concept of index)

   int highlightedBlockId = -1;
   if (this->curHighlightedBlockIndex != -1)
   {
      highlightedBlockId =
         this->displayBlocks[this->curHighlightedBlockIndex]->Id;
   }

   for each (BlockDisplayInfo ^ blockInfo in this->displayBlocks)
   {
      blockInfo->DrawNodeConnections(context, trayIncrement,
         highlightedBlockId);
   }
}

//------------------------------------------------------------------------------
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
//    The basic block index of the block at mousePosition, or -1 if no
//    block exists at this position.
//
//------------------------------------------------------------------------------

int
FuncDisplayInfo::GetMouseBlockIntersectionIndex
(
   Point mousePosition
)
{
   if ((this->firstDrawnBlockIndex < 0)
      || (this->firstDrawnBlockIndex >= this->displayBlocks->Count)
      || (this->lastDrawnBlockIndex < 0)
      || (this->lastDrawnBlockIndex >= this->displayBlocks->Count))
   {
      // Invalid indices or no drawn blocks

      return -1;
   }

   // Check boxes (only those visible though).

   for (int i = this->firstDrawnBlockIndex; i <= this->lastDrawnBlockIndex; i++)
   {
      if (this->displayBlocks[i]->TransformedRect.Contains(mousePosition))
      {
         return i;
      }
   }
   return -1;
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
FuncDisplayInfo::UpdateScrollBar
(
   VScrollBar ^ vScroll,
   int          scrollToIndex
)
{
   // Set the scroll bar up so that if scrolled to the top, it shows the 
   // first element, and if scrolled to the bottom, it shows the last element

   vScroll->Maximum = (int)(this->zoomFactor * this->windowLength);
   vScroll->LargeChange = 5;
   vScroll->SmallChange = 2;

   if ((scrollToIndex >= 0) && (scrollToIndex < this->displayBlocks->Count))
   {
      BlockDisplayInfo ^ last =
         this->displayBlocks[this->displayBlocks->Count - 1];
      BlockDisplayInfo ^ first = this->displayBlocks[0];

      float minWindowPos = (float)(first->TransformedRect.Y);
      float maxWindowPos = (float)(last->TransformedRect.Y +
         last->TransformedRect.Height);
      float transformedWindowLength = maxWindowPos - minWindowPos;

      float scrollToPos =
         (float)this->displayBlocks[scrollToIndex]->TransformedRect.Y -
         minWindowPos;
      float newValue =
         (float)vScroll->Maximum * scrollToPos / transformedWindowLength;
      vScroll->Value = (int)newValue;
   }
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

List<BlockDisplayInfo ^ > ^
FuncDisplayInfo::DisplayBlocks::get()
{
   return this->displayBlocks;
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

unsigned int
FuncDisplayInfo::PixelsPerUnit::get()
{
   return this->verticalPixelsPerUnit;
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
FuncDisplayInfo::PixelsPerUnit::set
(
   unsigned int pixels
)
{
   this->verticalPixelsPerUnit = pixels;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Gets the zoom factor for the flow graph.
//
// Returns:
//
//    The zoom factor for the flow graph.
//
//------------------------------------------------------------------------------

float
FuncDisplayInfo::ZoomFactor::get()
{
   return this->zoomFactor;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Sets the zoom factor for the flow graph.
//
// Arguments:
//
//    zoom - The zoom factor for the flow graph.
//
//------------------------------------------------------------------------------

void
FuncDisplayInfo::ZoomFactor::set
(
   float zoom
)
{
   this->zoomFactor = zoom;
   if (this->zoomFactor > FuncDisplayInfo::MaxZoom)
   {
      this->zoomFactor = FuncDisplayInfo::MaxZoom;
   }
   else if (this->zoomFactor <= FuncDisplayInfo::MinZoom)
   {
      this->zoomFactor = FuncDisplayInfo::MinZoom;
   }
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Sets the font used to draw the text in the flow graph.
//
// Arguments:
//
//    font - The font used in the basic blocks of the flow graph.
//
//------------------------------------------------------------------------------

void
FuncDisplayInfo::BlockTextFont::set
(
   Font ^ font
)
{
   this->blockTextFont = font;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Gets the highlighted block's index.
//
// Returns:
//
//    The highlighted block's index.
//
//------------------------------------------------------------------------------

int
FuncDisplayInfo::HighlightedBlockIndex::get()
{
   return this->curHighlightedBlockIndex;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Sets the highlighted block.
//
// Arguments:
//
//    index - The index of the block to highlight.
//
//------------------------------------------------------------------------------

void
FuncDisplayInfo::HighlightedBlockIndex::set
(
   int index
)
{
   if ((index == -1) || ((index >= 0) && (index < this->displayBlocks->Count)))
   {
      this->curHighlightedBlockIndex = index;
   }
}

} // ControlFlowGraph
