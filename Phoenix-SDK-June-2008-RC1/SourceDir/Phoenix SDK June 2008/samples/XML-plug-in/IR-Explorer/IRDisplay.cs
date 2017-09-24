using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.IO;

namespace IRExplorer
{
/// <summary>
/// Summary description for IRDisplay.
/// </summary>
public class IRDisplay : System.Windows.Forms.Form
{
   int               id;
   FunctionUnit      functionUnit;
   int               curPhaseIndex;
   ArrayList         sourceText;

   ArrayList         jumpStack;

   bool              orderByPhases;
   Font              font, monoFont;

   SolidBrush        backBrush, textBrush;
   SolidBrush        blockBrush, blockHitBrush, instrHitBrush;
   SolidBrush        debugBrush;
   Pen               redPen;
   Point[]           bezPts;
   Pen               dnPen, dnWidePen, upPen, upWidePen;
   
   float             topMargin, leftMargin, leftGutter;
   float             leading;
   float             vertBlockGap;
   float             dstTab, dstWidth;
   float             opcodeTab, opcodeWidth;
   float             srcTab, srcWidth;
   float             instrMargin;
   float             horzOpndGap;
   float             funcUnitHeight;
   
   bool              leftLeftMouseDown, rightLeftMouseDown;
   int               leftPrevMX, leftPrevMY;
   int               rightPrevMX, rightPrevMY;

   int               currentBlock, curInstr, curOpnd;
   int               curSourceLine;

   int               srcTop, srcBgn, srcEnd, srcHit;
   int               blockHit, instrHit, opndHit;
   Instruction             hotInstr;
   Operand              hotOpnd;
   int               hotOpndY;

   private System.Windows.Forms.StatusBar statusBar;
   private System.Windows.Forms.Panel topPanel;
   private System.Windows.Forms.PictureBox logoBox;
   private System.Windows.Forms.Panel bodyPanel;
   private System.Windows.Forms.TreeView treeView;
   private System.Windows.Forms.Splitter treeViewSplitter;
   private System.Windows.Forms.Panel leftPanel;
   private System.Windows.Forms.Splitter sheetSplitter;
   private System.Windows.Forms.Panel rightPanel;
   private System.Windows.Forms.Label leftLabel;
   private System.Windows.Forms.Label rightLabel;
   private Sheet leftSheet;
   private Sheet rightSheet;
   private System.Windows.Forms.GroupBox orderGroup;
   private System.Windows.Forms.RadioButton orderPhase;
   private System.Windows.Forms.RadioButton orderFunc;
	/// <summary>
	/// Required designer variable.
	/// </summary>
	private System.ComponentModel.Container components = null;

	public IRDisplay(int myId, Font f)
	{
      id = myId;
      InitializeComponent();

      orderByPhases = false;
      BuildNavigationTree ();
		
      leftSheet.LabelCtrl = leftLabel;
      rightSheet.LabelCtrl = rightLabel;
		
      //logoBox.Image = new Bitmap (@"c:\_playpen\PhxLogo.jpg");

      font = f;
      UpdateFonts (font);

      backBrush = new SolidBrush (Color.White);
      textBrush = new SolidBrush (Color.Black);
      blockBrush = MainForm.blockBrush;
      blockHitBrush = MainForm.blockHitBrush;
      instrHitBrush = MainForm.instrHitBrush;
      debugBrush = new SolidBrush (Color.Tomato);
      redPen = new Pen (Color.Red);

      bezPts = new Point[7];
      dnWidePen = new Pen (Color.Blue, 3.0f);
      dnPen = new Pen (Color.White, 1.0f);
      upWidePen = new Pen (Color.DarkSalmon, 3.0f);
      upPen = new Pen (Color.White, 1.0f);
      
      sheetSplitter.SplitPosition = topPanel.Width - 2;

      leftLeftMouseDown = rightLeftMouseDown = false;
      leftPrevMX = leftPrevMY = -1;

      jumpStack = new ArrayList ();
   }


   public void NewColors ()
   {
      blockBrush = MainForm.blockBrush;
      blockHitBrush = MainForm.blockHitBrush;
      instrHitBrush = MainForm.instrHitBrush;
      leftSheet.Invalidate ();
      rightSheet.Invalidate ();
   }



   public void BuildNavigationTree ()
   {
      int      p, f, i;
      string   name;
      FunctionUnit functionUnit;
      TreeNode node;

      treeView.BeginUpdate ();
      treeView.Nodes.Clear ();

      if (orderByPhases)
      {
         for (p = 0;  p < MainForm.allPhaseNames.Count;  p++)
         {

            if (MainForm.funcTable[p].Count == 0)
            {
               treeView.Nodes.Add ((string) MainForm.allPhaseNames[p]);
               treeView.Nodes[p].ForeColor = Color.Gray;
               treeView.Nodes[p].Tag = null;
            }
            else  // a phase with FuncUnits
            {
               treeView.Nodes.Add (new TreeNode ((string) MainForm.allPhaseNames[p]));
               treeView.Nodes[p].Tag = null;

               for (f = 0;  f < MainForm.funcTable[p].Count;  f++)
               {
                  treeView.Nodes[p].Nodes.Add ((string) MainForm.funcTable[p].GetKey (f));
                  node = treeView.Nodes[p].Nodes[f];
                  functionUnit = (FunctionUnit) MainForm.funcTable[p].GetByIndex (f);
                  node.Tag = functionUnit;
               }
            }
         }
      }
      else
      {
         for (f = 0;  f < MainForm.usedFuncNames.Count;  f++)
         {
            name = (string) MainForm.usedFuncNames.GetKey (f);
            // We know this functionUnit is in at least one phase.
            treeView.Nodes.Add (new TreeNode (name));
            treeView.Nodes[f].Tag = null;
            // If this function appears in a phase's function list, add phase to node.
            for (p = 0;  p < MainForm.allPhaseNames.Count;  p++)
            {
               treeView.Nodes[f].Nodes.Add ((string) MainForm.allPhaseNames[p]);
               node = treeView.Nodes[f].Nodes[p];
               i = FuncIndex (name, MainForm.funcTable[p]);
               if (i < 0)
               {
                  node.ForeColor = Color.Gray;
                  node.Tag = null;
               }
               else
               {
                  // Point tree node to function unit.
                  functionUnit = (FunctionUnit) MainForm.funcTable[p].GetByIndex (i);
                  node.Tag = functionUnit;
               }
            }
         }
      }

      treeView.EndUpdate ();
   }


   public void SelectInTree()
   {
      // Find current functionUnit's TreeNode in nav tree.

      TreeNode node;
      int   p, f;

      if (functionUnit == null)   return;

      p = functionUnit.phaseTreeIndex;

      if (orderByPhases)
      {
         f = FuncIndex (functionUnit.unitName, MainForm.funcTable[p]);
         node = treeView.Nodes[p].Nodes[f];
      }
      else
      {
         f = FuncIndex (functionUnit.unitName, MainForm.usedFuncNames);
         node = treeView.Nodes[f].Nodes[p];
      }

      node.EnsureVisible ();
   }


   public int FuncIndex (string functionName, SortedList phaseFuncNames)
   {
      for (int i = 0;  i < phaseFuncNames.Count;  i++)
      {
         if (functionName == (string) phaseFuncNames.GetKey (i))   return i;
      }
      return -1;
   }



   public void NewFont (Font f)
   {
      UpdateFonts (f);
      leftSheet.Invalidate();
      rightSheet.Invalidate();
   }



   public void UpdateFonts (Font font)
   {
      float       emSpace;
      
      monoFont = font;

      leftSheet.Font = rightSheet.Font = font;
      leftLabel.Font = font;
      leftLabel.Height = 2 * font.Height;
      rightLabel.Font = font;
      rightLabel.Height = 2 * font.Height;

      emSpace = font.Size;

      topMargin = 0.0f;
      leftGutter = 3 * emSpace;;
      leftMargin = 3 * emSpace;;
      leading = font.Height;
      
      vertBlockGap = 1.5f * font.Height;
      
      horzOpndGap = emSpace;
      dstTab = leftGutter + leftMargin;
      opcodeTab = dstTab + 20 * emSpace;
      srcTab = opcodeTab + 15 * emSpace;
      instrMargin = srcTab + 40 * emSpace;

      dstWidth = opcodeTab - dstTab - emSpace;
      opcodeWidth = srcTab - opcodeTab - emSpace;
      srcWidth = instrMargin - srcTab;
      
      if (functionUnit != null)
      {
         rightSheet.SetScrollbar (sourceText.Count * rightSheet.Font.Height +
            rightSheet.Height/2);
         leftSheet.SetScrollbar (SizeUpIR(functionUnit));
      }
   }


   public void DisplayFuncUnit ()
   {
      LoadSource (functionUnit.sourceFileName);
      rightSheet.Invalidate ();
      LoadIR (functionUnit);
   }



   public void LoadSource (string fileName)
   {
      string         localString;
      StreamReader   file;

      sourceText = new ArrayList();
      try 
      {
         file = new StreamReader (fileName);

         localString = file.ReadLine ();
         while (localString != null)   
         {
            sourceText.Add (localString);
            localString = file.ReadLine ();
         }
      }
      catch 
      {
         sourceText.Add (String.Format ("can't read {0}", fileName));
      }
      
      sourceText.TrimToSize ();

      srcTop = -1;
      srcBgn = -1;
      srcEnd = -1;
      curSourceLine = -1;
      
      rightSheet.ShouldEraseBG = false;
      rightSheet.SetScrollbar (sourceText.Count * rightSheet.Font.Height);
      rightSheet.LabelCtrl.Text = functionUnit.sourceFileName;
   }





   public void LoadIR (FunctionUnit functionUnit)
   {
      srcTop = -1;
      srcBgn = -1;
      srcEnd = -1;
      curSourceLine = -1;
      blockHit = -1;
      currentBlock = -1;
      curInstr = -1;
      curOpnd = -1;

      leftSheet.ShouldEraseBG = false;
      leftSheet.SetScrollbar (SizeUpIR(functionUnit));
      leftSheet.LabelCtrl.Text = functionUnit.unitName +
         " from " + MainForm.allPhaseNames[functionUnit.phaseTreeIndex];

      leftSheet.VertOffset = 0.0f;
      leftSheet.Invalidate ();
   }


   
   public int SizeUpIR (FunctionUnit functionUnit)
   {
      int      b;
      int      depth;
      Block    blk;
      
      depth = (int) topMargin;

      if (functionUnit != null)
      {
         for (b = 0;  b < functionUnit.nBlocks;  b++)
         {
            blk = functionUnit.blockList[b];
            blk.minY = depth;
            depth += functionUnit.blockList[b].nInstrs * leftSheet.Font.Height;
            blk.maxY = depth;
            depth += (int) vertBlockGap;
         }
      }

      return depth;
   }



   public void FindBlockInstrOpnd (int x, int y)
   {
      int      absY;
      Block    blk;
      Instruction    instruction;
      Operand     operand;

      blockHit = instrHit = opndHit = -1;
      hotInstr = null;
      hotOpnd = null;
      hotOpndY = 0;

      if (functionUnit != null)
      {
         // Put y in the absolute space of the FunctionUnit.
         absY = y - (int) leftSheet.VertOffset;

         for (int b = 0;  b < functionUnit.nBlocks;  b++)
         {
            blk = functionUnit.blockList[b];
            if (absY >= blk.minY  &&  absY <= blk.maxY)
            {
               blockHit = b;
               instrHit = (absY - blk.minY) / leftSheet.Font.Height;
               if (instrHit < 0)
                  instrHit = 0;
               else if (instrHit >= blk.nInstrs)
                  instrHit = blk.nInstrs - 1;
               instruction = hotInstr = blk.instructionList[instrHit];
               if (x < opcodeTab)
               {
                  for (int op = 0;  op < instruction.nDstOpnds;  op++)
                  {
                     operand = instruction.dstOperandList[op];
                     if (x >= operand.minX  &&  x <= operand.maxX)
                     {
                        opndHit = op;
                        hotOpnd = operand;
                        hotOpndY = y;
                     }
                  }
               }
               else if (x > srcTab)
               {
                  for (int op = 0;  op < instruction.nSrcOpnds;  op++)
                  {
                     operand = instruction.srcOperandList[op];
                     if (x >= operand.minX  &&  x <= operand.maxX)
                     {
                        opndHit = 1000 + op;
                        hotOpnd = operand;
                        hotOpndY = y;
                     }
                  }
               }
               if (hotOpnd != null)
               {
                  if (hotOpnd.operandKind == OperandKind.Variable)
                  {
                     this.statusBar.Text = "   " + (hotOpnd as VariableOperand).type;
                  }
                  else if (hotOpnd.operandKind == OperandKind.Memory)
                  {
                     this.statusBar.Text = "   " + (hotOpnd as MemoryOperand).type;
                  }
                  else
                  {
                     this.statusBar.Text = "";
                  }
               }
               else
               {
                  this.statusBar.Text = "";
               }
            }
         }
      }

      if (hotOpnd == null)
      {
         this.statusBar.Text = "";  //  all these can't really be needed...
      }
   }



   public void DisplayIR (Graphics gr)
   {
      bool     isHotInstr;
      int      b, i, j;
      float    x, y, dy, fh;
      string   localString;
      Block    block;
      Instruction    instruction;
      RectangleF  clipRect;
      Graphics g;
      Sheet    sheet;
      Brush    brush;

      sheet = leftSheet;
      g = sheet.Gr;

      funcUnitHeight = functionUnit.blockList[functionUnit.nBlocks - 1].maxY;

      y = topMargin + sheet.VertOffset;
      dy = leading;
      fh = sheet.Font.Height;
      
      g.FillRectangle (backBrush, 0, 0, sheet.Width, sheet.Height);
      
      clipRect = new RectangleF (0.0f, 0.0f, 1.0f, 1.0f);

      for (b = 0;  b < functionUnit.nBlocks; b++)   
      {
      
         block = functionUnit.blockList[b];
         
         g.ResetClip();
         brush = (b == blockHit) ? blockHitBrush : blockBrush;
         g.FillRectangle (brush, dstTab - 5.0f, y - fh/4.0f,
            instrMargin - dstTab + 10.0f, block.nInstrs * fh + fh/2.0f);
            
         for (i = 0;  i < block.nInstrs;  i++)   
         {
         
            g.ResetClip();

            isHotInstr = b == blockHit  &&  i == instrHit;
            if (isHotInstr)
            {
               g.FillRectangle (instrHitBrush, dstTab - 5.0f, y,
                  instrMargin - dstTab + 10.0f, fh);
            }

            clipRect.X = dstTab;
            clipRect.Width = dstWidth;
            clipRect.Y = y;
            clipRect.Height = fh;
            g.IntersectClip (clipRect);
         
            x = dstTab;
            instruction = block.instructionList[i];
            for (j = 0;  j < instruction.nDstOpnds;  j++)   
            {
               localString = instruction.dstOperandList[j].display;
               instruction.dstOperandList[j].minX = (int) x;
               brush = (isHotInstr && j == opndHit) ? debugBrush : textBrush;
               g.DrawString (localString, sheet.Font, brush, x, y);
               x += g.MeasureString (localString, sheet.Font).Width;
               instruction.dstOperandList[j].maxX = (int) x;
               x += horzOpndGap;
            }
            
            g.ResetClip();
            clipRect.X = opcodeTab;
            clipRect.Width = opcodeWidth;
            g.IntersectClip (clipRect);
            
            x = opcodeTab;
            localString = instruction.opName;
            if (instruction.ccName != null)
            {
               localString += "(" + instruction.ccName + ")";
            }
            g.DrawString (localString, sheet.Font, textBrush, x, y);

            g.ResetClip();
            clipRect.X = srcTab;
            clipRect.Width = srcWidth;
            g.IntersectClip (clipRect);
            
            x = srcTab;
            for (j = 0;  j < instruction.nSrcOpnds;  j++)   
            {
               localString = instruction.srcOperandList[j].display;
               instruction.srcOperandList[j].minX = (int) x;
               brush = (isHotInstr && j == opndHit-1000) ? debugBrush : textBrush;
               g.DrawString (localString, sheet.Font, brush, x, y);
               x += g.MeasureString (localString, sheet.Font).Width;
               instruction.srcOperandList[j].maxX = (int) x;
               x += horzOpndGap;
            }
                        
            y += dy;
         }
         y += vertBlockGap;
      }
      
      g.ResetClip();

      for (b=functionUnit.nBlocks - 1;  b >= 0 ;  b--)
      {
         block = functionUnit.blockList[b];
         for (i=0;  i < block.nSuccEdges;  i++)
         {
            DisplayGraphEdge(sheet, b, block.successorEdgeList[i].linkBlock);
         }
      }

      sheet.SwapBuffers (gr);
   }


   public void DisplayGraphEdge (Sheet sheet, int bBgn, int bEnd)
   {
      bool     down;
      int      x0, x1, yOrig, dy, y0, y1, y2;
      float    bulge;
      Block    blockBgn, blockEnd;

      const int ledge = 0;

      if (bBgn >= functionUnit.nBlocks  ||  bEnd >= functionUnit.nBlocks)   return;
      if (bBgn == bEnd  ||  bEnd < 0)   return;

      blockBgn = functionUnit.blockList[bBgn];
      blockEnd = functionUnit.blockList[bEnd];

      yOrig = (int) (topMargin + sheet.VertOffset);
      dy = blockEnd.minY - blockBgn.maxY;

      down = bEnd > bBgn;

      bulge = (float) Math.Abs (dy) / funcUnitHeight;
      // If much taller than a screen, increase the edge bulge.
      if (funcUnitHeight > 1500.0f)
      {
         //bulge *= 2.0f;
      }

      x0 = (int) (instrMargin + 5) + ledge;
      x1 = x0 + (int) (100.0f * bulge);
      y0 = yOrig + blockBgn.maxY;
      y2 = y0 + dy;
      y1 = (y0 + y2) / 2;

      bezPts[0].X = x0;
      bezPts[0].Y = y0;
      bezPts[1].X = x1;
      bezPts[1].Y = y0;
      bezPts[2].X = x1;
      bezPts[2].Y = y1 - (y1 - y0)/2;
      bezPts[3].X = x1;
      bezPts[3].Y = y1;
      bezPts[4].X = x1;
      bezPts[4].Y = y1 + (y1 - y0)/2;
      bezPts[5].X = x1;
      bezPts[5].Y = y2;
      bezPts[6].X = x0;
      bezPts[6].Y = y2;

      if (down)
      {
         sheet.Gr.DrawLine (dnWidePen, x0-ledge, y0, x0, y0);
         sheet.Gr.DrawLine (dnWidePen, x0-ledge, y2, x0, y2);
         sheet.Gr.DrawBeziers (dnWidePen, bezPts);
         sheet.Gr.DrawLine (dnPen, x0-ledge, y0, x0, y0);
         sheet.Gr.DrawLine (dnPen, x0-ledge, y2, x0, y2);
         sheet.Gr.DrawBeziers (dnPen, bezPts);
      }
      else
      {
         sheet.Gr.DrawLine (upWidePen, x0-ledge, y0, x0, y0);
         sheet.Gr.DrawLine (upWidePen, x0-ledge, y2, x0, y2);
         sheet.Gr.DrawBeziers (upWidePen, bezPts);
         sheet.Gr.DrawLine (upPen, x0-ledge, y0, x0, y0);
         sheet.Gr.DrawLine (upPen, x0-ledge, y2, x0, y2);
         sheet.Gr.DrawBeziers (upPen, bezPts);
      }
   }



   public void DisplaySource (Graphics gr)
   {
      int         i;
      float       x, y, dy;
      float       numEdge, numWidth;
      string      number;
      Graphics    g;
      Sheet       sheet;
      Brush       brush;

      sheet = rightSheet;
      g = sheet.Gr;

      dy = leading;
      x = leftGutter + leftMargin;

      if (srcTop >= 0)
      {
         y = - (srcTop-1) * dy;
         sheet.VertOffset = y;
         srcTop = -1;
      }
      else
      {
         y = sheet.VertOffset;
      }

      y += topMargin;

      g.FillRectangle (backBrush, 0, 0, sheet.Width, sheet.Height);

      numEdge = leftGutter + leftMargin - monoFont.Size;

      for (i = 1;  i <= sourceText.Count;  i++)   
      {
         number = i.ToString();
         numWidth = g.MeasureString (number, monoFont).Width;
         g.DrawString (number, monoFont, Brushes.Teal,
            numEdge - numWidth, y);
         if (i >= srcBgn  &&  i <= srcEnd)
         {
            brush = (i == srcHit) ? instrHitBrush : blockHitBrush;
            g.FillRectangle (brush, x, y, 1000, dy);
         }
         g.DrawString (sourceText[i-1] as string, sheet.Font, textBrush, x, y);
         y += dy;
      }

      g.DrawLine (Pens.Teal, numEdge, 0, numEdge, 2048);
      
      sheet.SwapBuffers (gr);
   }


	/// <summary>
	/// Clean up any resources being used.
	/// </summary>
	protected override void Dispose( bool disposing )
	{
		if( disposing )
		{
			if(components != null)
			{
				components.Dispose();
			}
		}
		base.Dispose( disposing );
	}

	#region Windows Form Designer generated code
	/// <summary>
	/// Required method for Designer support - do not modify
	/// the contents of this method with the code editor.
	/// </summary>
	private void InitializeComponent()
	{
      this.statusBar = new System.Windows.Forms.StatusBar();
      this.topPanel = new System.Windows.Forms.Panel();
      this.orderGroup = new System.Windows.Forms.GroupBox();
      this.orderFunc = new System.Windows.Forms.RadioButton();
      this.orderPhase = new System.Windows.Forms.RadioButton();
      this.logoBox = new System.Windows.Forms.PictureBox();
      this.bodyPanel = new System.Windows.Forms.Panel();
      this.rightPanel = new System.Windows.Forms.Panel();
      this.rightSheet = new IRExplorer.Sheet();
      this.rightLabel = new System.Windows.Forms.Label();
      this.sheetSplitter = new System.Windows.Forms.Splitter();
      this.leftPanel = new System.Windows.Forms.Panel();
      this.leftSheet = new IRExplorer.Sheet();
      this.leftLabel = new System.Windows.Forms.Label();
      this.treeViewSplitter = new System.Windows.Forms.Splitter();
      this.treeView = new System.Windows.Forms.TreeView();
      this.topPanel.SuspendLayout();
      this.orderGroup.SuspendLayout();
      this.bodyPanel.SuspendLayout();
      this.rightPanel.SuspendLayout();
      this.leftPanel.SuspendLayout();
      this.SuspendLayout();
      // 
      // statusBar
      // 
      this.statusBar.Font = new System.Drawing.Font("Trebuchet MS", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
      this.statusBar.Location = new System.Drawing.Point(0, 599);
      this.statusBar.Name = "statusBar";
      this.statusBar.Size = new System.Drawing.Size(942, 22);
      this.statusBar.TabIndex = 0;
      // 
      // topPanel
      // 
      this.topPanel.BackColor = System.Drawing.SystemColors.Window;
      this.topPanel.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
      this.topPanel.Controls.Add(this.orderGroup);
      this.topPanel.Controls.Add(this.logoBox);
      this.topPanel.Dock = System.Windows.Forms.DockStyle.Top;
      this.topPanel.Location = new System.Drawing.Point(0, 0);
      this.topPanel.Name = "topPanel";
      this.topPanel.Size = new System.Drawing.Size(942, 96);
      this.topPanel.TabIndex = 1;
      // 
      // orderGroup
      // 
      this.orderGroup.Controls.Add(this.orderPhase);
      this.orderGroup.Controls.Add(this.orderFunc);
      this.orderGroup.Location = new System.Drawing.Point(8, 8);
      this.orderGroup.Name = "orderGroup";
      this.orderGroup.Size = new System.Drawing.Size(112, 72);
      this.orderGroup.TabIndex = 1;
      this.orderGroup.TabStop = false;
      this.orderGroup.Text = "Order by";
      // 
      // orderFunc
      // 
      this.orderFunc.Checked = true;
      this.orderFunc.Location = new System.Drawing.Point(8, 24);
      this.orderFunc.Name = "orderFunc";
      this.orderFunc.Size = new System.Drawing.Size(80, 24);
      this.orderFunc.TabIndex = 1;
      this.orderFunc.TabStop = true;
      this.orderFunc.Text = "FunctionUnit";
      // 
      // orderPhase
      // 
      this.orderPhase.Location = new System.Drawing.Point(8, 48);
      this.orderPhase.Name = "orderPhase";
      this.orderPhase.Size = new System.Drawing.Size(80, 16);
      this.orderPhase.TabIndex = 0;
      this.orderPhase.Text = "Phase";
      this.orderPhase.CheckedChanged += new System.EventHandler(this.orderPhase_CheckedChanged);
      // 
      // logoBox
      // 
      this.logoBox.BackColor = System.Drawing.SystemColors.Window;
      this.logoBox.Dock = System.Windows.Forms.DockStyle.Right;
      this.logoBox.Location = new System.Drawing.Point(865, 0);
      this.logoBox.Name = "logoBox";
      this.logoBox.Size = new System.Drawing.Size(75, 94);
      this.logoBox.TabIndex = 0;
      this.logoBox.TabStop = false;
      // 
      // bodyPanel
      // 
      this.bodyPanel.Controls.Add(this.rightPanel);
      this.bodyPanel.Controls.Add(this.sheetSplitter);
      this.bodyPanel.Controls.Add(this.leftPanel);
      this.bodyPanel.Controls.Add(this.treeViewSplitter);
      this.bodyPanel.Controls.Add(this.treeView);
      this.bodyPanel.Dock = System.Windows.Forms.DockStyle.Fill;
      this.bodyPanel.Location = new System.Drawing.Point(0, 96);
      this.bodyPanel.Name = "bodyPanel";
      this.bodyPanel.Size = new System.Drawing.Size(942, 503);
      this.bodyPanel.TabIndex = 2;
      // 
      // rightPanel
      // 
      this.rightPanel.Controls.Add(this.rightSheet);
      this.rightPanel.Controls.Add(this.rightLabel);
      this.rightPanel.Dock = System.Windows.Forms.DockStyle.Fill;
      this.rightPanel.Location = new System.Drawing.Point(520, 0);
      this.rightPanel.Name = "rightPanel";
      this.rightPanel.Size = new System.Drawing.Size(422, 503);
      this.rightPanel.TabIndex = 4;
      // 
      // rightSheet
      // 
      this.rightSheet.BackColor = System.Drawing.SystemColors.Window;
      this.rightSheet.Dock = System.Windows.Forms.DockStyle.Fill;
      this.rightSheet.LabelCtrl = null;
      this.rightSheet.Location = new System.Drawing.Point(0, 24);
      this.rightSheet.Name = "rightSheet";
      this.rightSheet.Size = new System.Drawing.Size(422, 479);
      this.rightSheet.TabIndex = 1;
      this.rightSheet.VertOffset = 0F;
      this.rightSheet.Resize += new System.EventHandler(this.rightSheet_Resize);
      this.rightSheet.MouseUp += new System.Windows.Forms.MouseEventHandler(this.rightSheet_MouseUp);
      this.rightSheet.Paint += new System.Windows.Forms.PaintEventHandler(this.rightSheet_Paint);
      this.rightSheet.MouseMove += new System.Windows.Forms.MouseEventHandler(this.rightSheet_MouseMove);
      this.rightSheet.MouseDown += new System.Windows.Forms.MouseEventHandler(this.rightSheet_MouseDown);
      // 
      // rightLabel
      // 
      this.rightLabel.BackColor = System.Drawing.SystemColors.Info;
      this.rightLabel.Dock = System.Windows.Forms.DockStyle.Top;
      this.rightLabel.Location = new System.Drawing.Point(0, 0);
      this.rightLabel.Name = "rightLabel";
      this.rightLabel.Size = new System.Drawing.Size(422, 24);
      this.rightLabel.TabIndex = 0;
      this.rightLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
      // 
      // sheetSplitter
      // 
      this.sheetSplitter.BackColor = System.Drawing.SystemColors.Desktop;
      this.sheetSplitter.Location = new System.Drawing.Point(512, 0);
      this.sheetSplitter.Name = "sheetSplitter";
      this.sheetSplitter.Size = new System.Drawing.Size(8, 503);
      this.sheetSplitter.TabIndex = 3;
      this.sheetSplitter.TabStop = false;
      // 
      // leftPanel
      // 
      this.leftPanel.Controls.Add(this.leftSheet);
      this.leftPanel.Controls.Add(this.leftLabel);
      this.leftPanel.Dock = System.Windows.Forms.DockStyle.Left;
      this.leftPanel.Location = new System.Drawing.Point(216, 0);
      this.leftPanel.Name = "leftPanel";
      this.leftPanel.Size = new System.Drawing.Size(296, 503);
      this.leftPanel.TabIndex = 2;
      // 
      // leftSheet
      // 
      this.leftSheet.BackColor = System.Drawing.SystemColors.Window;
      this.leftSheet.Dock = System.Windows.Forms.DockStyle.Fill;
      this.leftSheet.LabelCtrl = null;
      this.leftSheet.Location = new System.Drawing.Point(0, 24);
      this.leftSheet.Name = "leftSheet";
      this.leftSheet.Size = new System.Drawing.Size(296, 479);
      this.leftSheet.TabIndex = 1;
      this.leftSheet.VertOffset = 0F;
      this.leftSheet.Click += new System.EventHandler(this.leftSheet_Click);
      this.leftSheet.Resize += new System.EventHandler(this.leftSheet_Resize);
      this.leftSheet.MouseUp += new System.Windows.Forms.MouseEventHandler(this.leftSheet_MouseUp);
      this.leftSheet.Paint += new System.Windows.Forms.PaintEventHandler(this.leftSheet_Paint);
      this.leftSheet.DoubleClick += new System.EventHandler(this.leftSheet_DoubleClick);
      this.leftSheet.MouseMove += new System.Windows.Forms.MouseEventHandler(this.leftSheet_MouseMove);
      this.leftSheet.MouseDown += new System.Windows.Forms.MouseEventHandler(this.leftSheet_MouseDown);
      // 
      // leftLabel
      // 
      this.leftLabel.BackColor = System.Drawing.SystemColors.Info;
      this.leftLabel.Dock = System.Windows.Forms.DockStyle.Top;
      this.leftLabel.Location = new System.Drawing.Point(0, 0);
      this.leftLabel.Name = "leftLabel";
      this.leftLabel.Size = new System.Drawing.Size(296, 24);
      this.leftLabel.TabIndex = 0;
      this.leftLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
      // 
      // treeViewSplitter
      // 
      this.treeViewSplitter.BackColor = System.Drawing.SystemColors.Desktop;
      this.treeViewSplitter.Location = new System.Drawing.Point(208, 0);
      this.treeViewSplitter.Name = "treeViewSplitter";
      this.treeViewSplitter.Size = new System.Drawing.Size(8, 503);
      this.treeViewSplitter.TabIndex = 1;
      this.treeViewSplitter.TabStop = false;
      // 
      // treeView
      // 
      this.treeView.Dock = System.Windows.Forms.DockStyle.Left;
      this.treeView.ImageIndex = -1;
      this.treeView.Location = new System.Drawing.Point(0, 0);
      this.treeView.Name = "treeView";
      this.treeView.SelectedImageIndex = -1;
      this.treeView.Size = new System.Drawing.Size(208, 503);
      this.treeView.TabIndex = 0;
      this.treeView.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeView_AfterSelect);
      // 
      // IRDisplay
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6, 15);
      this.ClientSize = new System.Drawing.Size(942, 621);
      this.Controls.Add(this.bodyPanel);
      this.Controls.Add(this.topPanel);
      this.Controls.Add(this.statusBar);
      this.Name = "IRDisplay";
      this.Text = "IR Display";
      this.Closing += new System.ComponentModel.CancelEventHandler(this.IRDisplay_Closing);
      this.topPanel.ResumeLayout(false);
      this.orderGroup.ResumeLayout(false);
      this.bodyPanel.ResumeLayout(false);
      this.rightPanel.ResumeLayout(false);
      this.leftPanel.ResumeLayout(false);
      this.ResumeLayout(false);

   }
	#endregion



   private void leftSheet_Paint(object sender, System.Windows.Forms.PaintEventArgs e) 
   {
      if (functionUnit != null)   
      {
         DisplayIR (e.Graphics);
      }
   }



   private void rightSheet_Paint(object sender, System.Windows.Forms.PaintEventArgs e) 
   {
      if (functionUnit != null)   
      {
         DisplaySource (e.Graphics);
      }
   }



   private void leftSheet_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
   {
      if (e.Button == MouseButtons.Left)
      {
         leftLeftMouseDown = true;
         leftPrevMX = e.X;
         leftPrevMY = e.Y;
      }
      else
      {
         JumpStackEntry    jse;
         int               j = jumpStack.Count - 1;

         if (j >= 0)
         {
            jse = jumpStack[j] as JumpStackEntry;
            jumpStack.RemoveAt (j);

            if (jse.functionUnit == functionUnit)
            {
               leftSheet.VertOffset = jse.vertOffset;
               leftSheet.Invalidate ();
            }
            else
            {
               functionUnit = jse.functionUnit;
               DisplayFuncUnit ();
               SelectInTree ();
               leftSheet.VertOffset = jse.vertOffset;
               leftSheet.Invalidate ();
            }
         }
      }
   }



   private void leftSheet_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e) 
   {      
      if (leftLeftMouseDown)   
      {
         int      dy = e.Y - leftPrevMY;

         leftSheet.Cursor = Cursors.Hand;
         leftSheet.VertOffset += dy;
         leftPrevMY = e.Y;
         leftSheet.Invalidate();
      }
      else
      {
         int   k;

         FindBlockInstrOpnd (e.X, e.Y);

         if (blockHit == currentBlock && 
             instrHit == curInstr &&
             opndHit == curOpnd)
            return;

         currentBlock = blockHit;
         curInstr = instrHit;
         curOpnd = opndHit;

         leftSheet.Invalidate();

         if (blockHit >= 0)
         {
            curSourceLine = hotInstr.sourceLine;

            srcBgn = functionUnit.blockList[blockHit].firstSourceLine;
            srcEnd = functionUnit.blockList[blockHit].lastSourceLine;
            srcHit = functionUnit.blockList[blockHit].instructionList[instrHit].sourceLine;
            k = rightSheet.Height / 2;
            k /= rightSheet.Font.Height;
            if (srcTop < 0)
            {
               srcTop = (srcBgn > k) ? srcBgn - k : 1;

            }
            else
            {
               srcTop = (srcBgn > k) ? srcBgn - k : -1;
            }
         }
         else
         {
            srcTop = srcBgn = srcEnd = srcHit = -1;
         }

         rightSheet.Invalidate();
      }
   }



   private void leftSheet_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e) 
   {
      leftLeftMouseDown = false;
      leftSheet.Cursor = Cursors.Default;
   }



   private void rightSheet_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
   {
      rightLeftMouseDown = true;
      rightPrevMX = e.X;
      rightPrevMY = e.Y;
      rightSheet.Cursor = Cursors.Hand;
   }



   private void rightSheet_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e) 
   {
      if (rightLeftMouseDown)   
      {
         int      dy = e.Y - rightPrevMY;

         rightPrevMY = e.Y;
         rightSheet.Invalidate();
      }
   }



   private void rightSheet_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e) 
   {
      rightLeftMouseDown = false;
      rightSheet.Cursor = Cursors.Default;
   }



   private void topArea_Resize(object sender, System.EventArgs e) 
   {
   }



   private void sheetSplitter_SplitterMoved(object sender, System.Windows.Forms.SplitterEventArgs e)
   {
   
   }

   private void rightSheet_Resize(object sender, System.EventArgs e)
   {
      if (functionUnit != null)
      {
         rightSheet.SetScrollbar (sourceText.Count * rightSheet.Font.Height +
            rightSheet.Height/2);
      }
   }

   private void leftSheet_Resize(object sender, System.EventArgs e)
   {
      if (functionUnit != null)
      {
         leftSheet.SetScrollbar (SizeUpIR(functionUnit));
      }
   }

   private void leftSheet_Click(object sender, System.EventArgs e)
   {
   }

   private void leftSheet_DoubleClick(object sender, System.EventArgs e)
   {
      if (opndHit >= 1000  &&  hotOpnd.operandKind == OperandKind.Label)
      {
         int   newOffset;

         jumpStack.Add (new JumpStackEntry (functionUnit, leftSheet.VertOffset));
         newOffset =  - functionUnit.blockList[(hotOpnd as LabelOperand).jumpBlock].minY;
         newOffset += hotOpndY; // to put new label at same spot
         newOffset -= font.Height/2;  // to move up half a line
         if (newOffset > 0)   newOffset = 0;
         leftSheet.VertOffset = newOffset;
         opndHit = -1;
         leftSheet.Invalidate();
         return;
      } 
  
      if (hotInstr != null  &&  hotInstr.opName.ToLower() == "call")
      {
         string nextFuncUnitName = (hotInstr.srcOperandList[0] as FunctionOperand).sym;
         int   i = FuncIndex (nextFuncUnitName, MainForm.funcTable[curPhaseIndex]);

         if (i >= 0)
         {
            jumpStack.Add (new JumpStackEntry (functionUnit, leftSheet.VertOffset));
            functionUnit = (FunctionUnit) MainForm.funcTable[curPhaseIndex].GetByIndex (i);
            DisplayFuncUnit ();
            SelectInTree ();
            return;
         }
         else
         {
            return;
         }
      }
   }

   private void treeView_AfterSelect(object sender, System.Windows.Forms.TreeViewEventArgs e) {
      FunctionUnit nodeFunctionUnit;

      nodeFunctionUnit = (FunctionUnit) (e.Node.Tag);
      if (nodeFunctionUnit != null)
      {
         functionUnit = nodeFunctionUnit;
         curPhaseIndex = functionUnit.phaseTreeIndex;
         DisplayFuncUnit ();
      }
   }

   private void IRDisplay_Closing(object sender, System.ComponentModel.CancelEventArgs e)
   {
      MainForm.DisplayClosing (id);
   }

   private void orderPhase_CheckedChanged(object sender, System.EventArgs e)
   {
      orderByPhases = ! orderByPhases;
      BuildNavigationTree ();
      SelectInTree ();
   }
}


public class JumpStackEntry
{
   public FunctionUnit   functionUnit;
   public float      vertOffset;

   public JumpStackEntry (FunctionUnit newFunctionUnit, float offset)
   {
      functionUnit = newFunctionUnit;
      vertOffset = offset;  // will not be correct if font changes; needs smarts.
   }
}



//==========================================================   Sheet   =====

public class Sheet : Panel
{   
   public bool    ShouldEraseBG;
   public int     CurWidth, CurHeight;  // should be get-only

   public Sheet ()
   {
      myVSB = new VScrollBar();
      myVSB.Parent = this;
      myVSB.Dock = DockStyle.Right;
      myVSB.Visible = false;
      myVSB.SmallChange = Font.Height;
      myVSB.LargeChange = 5 * myVSB.SmallChange;
      myVSB.Scroll += new ScrollEventHandler(myVSB_Scroll);

      ShouldEraseBG = true;
   }

   public Graphics Gr
   {
      get {return bufGr;}
   }

   public Bitmap Im
   {
      get {return buffer;}
   }

   public Label LabelCtrl
   {
      set {myLabel = value;}
      get {return myLabel;}
   }

   public float VertOffset
   {
      set
      {
         if (-value >= (float) myVSB.Minimum  && 
            -value <= (float) myVSB.Maximum)
         {
            vertOffset = value;
            myVSB.Value = - (int) value;
         }
      }

      get
      {
         return vertOffset;
      }
   }

   public void SetScrollbar (int absHeight)
   {
      myVSB.Minimum = 0;
      myVSB.Maximum = absHeight;
      myVSB.Visible = this.Height < absHeight;
   }

   public void AllocateBuffer (int newWidth, int newHeight, Graphics targetGr)
   {
      if (newWidth == 0  ||  newHeight == 0  ||
         (newWidth == CurWidth  &&  newHeight == CurHeight))
      {
         return;
      }
         
      if (buffer != null)
      {
         buffer.Dispose();
      }
   
      if (bufGr != null)
      {
         bufGr.Dispose();
      }
   
      buffer = new Bitmap (newWidth, newHeight, targetGr);
      bufGr = Graphics.FromImage (buffer);
      CurWidth = newWidth;
      CurHeight = newHeight;
   }

   public void SwapBuffers (Graphics targetGr)
   {
      targetGr.DrawImage (buffer as Image, 0.0f, 0.0f);
   }

   protected override void OnPaintBackground (PaintEventArgs pea)
   {
      if (ShouldEraseBG)
      {
         base.OnPaintBackground (pea);
      }
      return;
   }

   protected override void OnResize(EventArgs e)
   {
      AllocateBuffer (this.ClientRectangle.Width, this.ClientRectangle.Height,
         this.CreateGraphics());
      base.OnResize (e);
   }

   private void myVSB_Scroll (object sender, ScrollEventArgs e)
   {
      vertOffset = - myVSB.Value;
      Invalidate();
   }

   float             vertOffset;
   Bitmap            buffer;
   Graphics          bufGr;
   Label             myLabel;
   VScrollBar        myVSB;
}

}
