//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    User control for the IR window.
//
//------------------------------------------------------------------------------

#include "irwindowcontrol.h"

namespace ControlFlowGraph
{

//------------------------------------------------------------------------------
//
// Description:
//
//    Default constructor.
//
//------------------------------------------------------------------------------

IRWindowControl::IRWindowControl()
{
   InitializeComponent();

   this->selectionBackColor = Color::Yellow;
   this->normalBackColor = SystemColors::Window;

   this->funcIRExtents = gcnew List <BlockIRExtents ^>();
   this->curHighlightedBlockIndex = -1;

   // Set double buffering to true

   this->SetStyle(ControlStyles::UserPaint | ControlStyles::DoubleBuffer |
      ControlStyles::AllPaintingInWmPaint, true);

   this->irTextBox->MouseClick +=
      gcnew MouseEventHandler(this,
         &IRWindowControl::OnMouseClick);
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
IRWindowControl::SetIR
(
   FuncDisplayInfo ^ functionInfo
)
{
   irTextBox->Clear();

   // Reinitialize the ir extents array

   this->funcIRExtents->Clear();

   // And set the selected IR to none (-1);

   this->curHighlightedBlockIndex = -1;

   for (int i = 0; i < functionInfo->DisplayBlocks->Count; i++)
   {
      BlockIRExtents ^ irExtents = gcnew BlockIRExtents();

      // Set the start position

      irExtents->IRStart = irTextBox->TextLength;

      // Set the text

      irTextBox->Text += functionInfo->DisplayBlocks[i]->BlockIRText;

      // Set the length.  Remember this is not neccesarily equal to the
      // length of the IR string.

      irExtents->IRLength =
         irTextBox->TextLength - irExtents->IRStart;
      this->funcIRExtents->Insert(i, irExtents);
   }
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Highlights the IR of the block given by an id.  Also scrolls
//    to the block if necessary.
//
// Arguments:
//
//    blockIndex - The id of the block to highlight.
//    scroll - If true, the window will scroll to show the highlighted
//    block.
//
//------------------------------------------------------------------------------

void
IRWindowControl::HighlightIR
(
   int  blockIndex,
   bool scroll
)
{
   if (this->curHighlightedBlockIndex != -1)
   {
      BlockIRExtents ^ irBlock =
         this->funcIRExtents[this->curHighlightedBlockIndex];

      // Un-highlight anything that was previous highlighted

      irTextBox->SelectionStart = irBlock->IRStart;
      irTextBox->SelectionLength = irBlock->IRLength;
      irTextBox->SelectionBackColor = this->normalBackColor;
      irTextBox->DeselectAll();
   }

   // Reset the currently highlighted block.

   this->curHighlightedBlockIndex = -1;

   // Set the new highlight (if valid)

   if ((blockIndex >= 0) || (blockIndex < this->funcIRExtents->Count))
   {
      this->curHighlightedBlockIndex = blockIndex;
   }

   // Now highlight the block

   if (this->curHighlightedBlockIndex != -1)
   {
      BlockIRExtents ^ irBlock =
         this->funcIRExtents[this->curHighlightedBlockIndex];
      irTextBox->SelectionStart = irBlock->IRStart;
      irTextBox->SelectionLength = irBlock->IRLength;
      if (scroll)
      {
         irTextBox->ScrollToCaret();
      }
      irTextBox->SelectionBackColor = this->selectionBackColor;
      irTextBox->DeselectAll();
   }
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

int
IRWindowControl::HighlightIR
(
   Point mousePosition
)
{
   unsigned int index = irTextBox->GetCharIndexFromPosition(mousePosition);

   for (int i = 0; i < this->funcIRExtents->Count; i++)
   {
      BlockIRExtents ^ block = this->funcIRExtents[i];
      if ((block->IRStart <= index)
         && (index <= (block->IRStart + block->IRLength)))
      {
         if (this->curHighlightedBlockIndex != i)
         {
            this->HighlightIR(i, false);
         }
         return i;
      }
   }
   return -1;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Destructor for IRWindowControl.  Cleans up the components it owns.
//
//------------------------------------------------------------------------------

IRWindowControl::~IRWindowControl()
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
//    Event handler for the MouseClick event raised by the RIchTextEdit
//    control.
//
// Arguments:
//
//    sender - The instance that invoked this message.
//    arguments - The standard mouse event arguments.
//
//------------------------------------------------------------------------------

void
IRWindowControl::OnMouseClick
(
   Object ^         sender,
   MouseEventArgs ^ arguments
)
{
   unsigned int highlighted = HighlightIR(arguments->Location);
   OnHighlightIRTextBlock(highlighted);
}

#pragma region Windows Form Designer generated code

// Required method for Designer support - do not modify
// the contents of this method with the code editor.

void
IRWindowControl::InitializeComponent()
{
   this->irTextBox = (gcnew RichTextBox());
   this->SuspendLayout();

   // irTextBox

   this->irTextBox->AutoWordSelection = true;
   this->irTextBox->BackColor = SystemColors::Window;
   this->irTextBox->DetectUrls = false;
   this->irTextBox->Dock = DockStyle::Fill;
   this->irTextBox->Font =
      gcnew System::Drawing::Font(L"Courier New", 10.2F, FontStyle::Regular,
         GraphicsUnit::Point, static_cast<Byte>(0));
   this->irTextBox->Location = Point(0, 0);
   this->irTextBox->Name = L"irTextBox";
   this->irTextBox->ReadOnly = true;
   this->irTextBox->Size = System::Drawing::Size(154, 150);
   this->irTextBox->TabIndex = 0;
   this->irTextBox->Text = L"";
   this->irTextBox->WordWrap = false;

   // IRWindowControl

   this->AutoScaleDimensions = SizeF(8, 16);
   this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
   this->Controls->Add(this->irTextBox);
   this->Name = L"IRWindowControl";
   this->Size = System::Drawing::Size(154, 150);
   this->ResumeLayout(false);
}
#pragma endregion

} // ControlFlowGraph
