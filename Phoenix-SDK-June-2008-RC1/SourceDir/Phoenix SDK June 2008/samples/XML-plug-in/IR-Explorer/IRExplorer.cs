using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;
using System.IO;
using System.Xml;

namespace IRExplorer
{

public class MainForm : System.Windows.Forms.Form
{
   static ArrayList                displays;
   static public ArrayList         allPhaseNames;
   static public SortedList[]      funcTable;
   static public SortedList        usedFuncNames;
   static public SolidBrush        blockBrush, blockHitBrush, instrHitBrush;
   static public string            defaultXmlDirectory;
   static public string            fontFace;
   static public Font              font;

   string            xmlDirectory;
   XmlDocument       xmlDoc;

   string            xmlPhaseNamePrefix;


   private System.Windows.Forms.MainMenu mainMenu;
   private System.Windows.Forms.MenuItem fileMenu;
   private System.Windows.Forms.MenuItem fileOpen;
   private System.Windows.Forms.ProgressBar progressBar;
   private System.Windows.Forms.MenuItem fileNew;
   private System.Windows.Forms.MenuItem prefMenu;
   private System.Windows.Forms.MenuItem prefFont;
   private System.Windows.Forms.MenuItem menuItem1;
   private System.Windows.Forms.MenuItem colorsSubtle;
   private System.Windows.Forms.MenuItem colorsNotSoSubtle;
   private System.Windows.Forms.Label progressLabel;
	/// <summary>
	/// Required designer variable.
	/// </summary>
	private System.ComponentModel.Container components = null;



	public MainForm()
	{
      InitializeComponent();

      xmlPhaseNamePrefix = "XML from";
      displays = new ArrayList ();
      defaultXmlDirectory = @"C:\";
      fontFace = "Trebuchet MS";
      font = new Font(fontFace, 9.0f);
      //blockBrush = new SolidBrush (Color.AliceBlue);
      //blockHitBrush = new SolidBrush (Color.LavenderBlush);
      //instrHitBrush = new SolidBrush (Color.Lavender);
      blockBrush = new SolidBrush (Color.LightSkyBlue);
      blockHitBrush = new SolidBrush (Color.Pink);
      instrHitBrush = new SolidBrush (Color.Lavender);
   }



   public void NewDirectory (string userPath)
   {
      xmlDirectory = userPath;
      LoadAllFuncUnitsInDir (xmlDirectory);
   }


   public void LoadAllFuncUnitsInDir (string directory)
   {
      bool        initialized = false;
      int         p, phaseBgn, phaseEnd;
      string[]    files;
      string      file, phaseName, functionName;
      string      msg;
      FunctionUnit    functionUnit;

      files = Directory.GetFiles (directory, "*.xml");

      progressBar.Maximum = files.Length;
      progressBar.Visible = true;
      progressLabel.Visible = true;

      // Builds funcTable[p][f], which holds the sorted list of function names
      // for every phase.  Also builds usedFuncNames, the sorted list of all
      // function names appearing for any phase.  These are enough to build the
      // navigation tree either function-by-phase or phase-by-function.

      usedFuncNames = new SortedList ();

      for (int i = 0;  i < files.Length;  i++)
      {
         file = files[i];

         progressBar.Value = i;
         msg = String.Format ("Scanning XML file {0}", file);

         // Skip files whose names don't have the correct form:
         //    path\[phasename]funcname.xml
         phaseBgn = file.LastIndexOf ('[') + 1;
         phaseEnd = file.LastIndexOf (']');
         if (phaseBgn > 0  &&  phaseEnd > phaseBgn)
         {
            if (! initialized)
            {
               // Load any file to find the full phase list; the first will do.
               // allPhaseNames is built in this function.
               CreateFuncUnitFromXML (file);
               initialized = true;

               // Create sorted lists for phases' FuncUnits.
               funcTable = new SortedList[allPhaseNames.Count];
               for (p = 0;  p < allPhaseNames.Count;  p++)
               {
                  funcTable[p] = new SortedList ();
               }
            }

            phaseName = file.Substring (phaseBgn, phaseEnd - phaseBgn);
            functionName = file.Substring (phaseEnd + 1, file.Length - phaseEnd - ".xml]".Length);

            if (usedFuncNames[functionName] == null)
            {
               usedFuncNames.Add (functionName, 0); // inserted in sort position
            }

            // could check initialized to avoid 1st file twice; then set here.
            p = PhaseIndex (phaseName);

            if (p > -1)
            {
               functionUnit = CreateFuncUnitFromXML (file);
               functionUnit.phaseName = phaseName;
               functionUnit.phaseTreeIndex = p;

               // inserted in sort position

               funcTable[p].Add (functionName, functionUnit);
            }
         }
      }

      usedFuncNames.TrimToSize ();  // function = usedFuncs.GetKey[i]

      progressBar.Visible = false;
      progressLabel.Visible = false;
   }



   public int PhaseIndex (string phaseName)
   {
      for (int i = 0;  i < allPhaseNames.Count;  i++)
      {
         if (phaseName == (string) allPhaseNames[i])   return i;
      }
      return -1;
   }


   public FunctionUnit CreateFuncUnitFromXML (string fileName)
   {
      XmlElement        xmlRoot;
      XmlNodeList       xmlPhasesElemList;
      XmlNode           xmlFuncUnit;

      xmlDoc = new XmlDocument();
      try
      {
         xmlDoc.Load(fileName);
      }
      catch (XmlException)
      {
         return null;
      }

      xmlRoot = xmlDoc.DocumentElement;

      xmlPhasesElemList = xmlRoot.SelectNodes ("phases");
      LoadPhases (xmlPhasesElemList);

      xmlFuncUnit = xmlRoot.SelectSingleNode ("function-unit");
      return LoadFuncUnit (xmlFuncUnit);
   }



   public void LoadPhases (XmlNodeList xmlPhasesElemList)
   {
      string         phaseName;
      XmlNodeList    xmlPhases;

      allPhaseNames = new ArrayList ();

      foreach (XmlNode xmlPhasesElem in xmlPhasesElemList)
      {
         xmlPhases = xmlPhasesElem.SelectNodes ("//phase");
         foreach (XmlNode p in xmlPhases)
         {
            phaseName = p.Attributes["name"].Value;
            if (phaseName.Length <= xmlPhaseNamePrefix.Length  ||
               phaseName.Substring (0,
                  xmlPhaseNamePrefix.Length) != xmlPhaseNamePrefix)
            {
               allPhaseNames.Add (phaseName);
            }
         }
      }

      allPhaseNames.TrimToSize ();
   }



   public FunctionUnit LoadFuncUnit (XmlNode xmlFuncUnit)
   {
      FunctionUnit functionUnit = new FunctionUnit();
      functionUnit.unitName       = xmlFuncUnit.Attributes["name"].Value;
      functionUnit.irState        = xmlFuncUnit.Attributes["ir-state"].Value;
      //functionUnit.phaseName      = xmlFuncUnit.Attributes["phase"].Value;
      functionUnit.sourceFileName = xmlFuncUnit.Attributes["source"].Value;

      LoadBlocks (xmlFuncUnit, functionUnit);

      FixLinks (functionUnit);

      return functionUnit;
   }



   public void LoadBlocks (XmlNode xmlFuncUnit, FunctionUnit functionUnit)
   {
      int            b;
      XmlNodeList    xmlBlocks, xmlEdges;
      XmlNode        xmlBlockSummary, node;
      Block          block;

      xmlBlocks = xmlFuncUnit.SelectNodes ("blocks/block");
      functionUnit.nBlocks = xmlBlocks.Count;
      functionUnit.blockList = new Block[functionUnit.nBlocks];

      for (b = 0;  b < functionUnit.nBlocks;  b++)
      {
         block = functionUnit.blockList[b] = new Block ();

         LoadBlockInstrs (xmlBlocks[b], block);

         xmlBlockSummary = xmlBlocks[b].SelectSingleNode ("block-summary");

         node = xmlBlockSummary.SelectSingleNode ("source-line-range");
         block.firstSourceLine = int.Parse (node.Attributes["first"].Value);
         block.lastSourceLine = int.Parse (node.Attributes["last"].Value);

         node = xmlBlockSummary.SelectSingleNode ("succ-edge-list");
         if (node != null)
         {
            xmlEdges = node.SelectNodes ("edge");
            block.nSuccEdges = xmlEdges.Count;
            block.successorEdgeList = new Edge[block.nSuccEdges];
            LoadEdges (xmlEdges, block.successorEdgeList);
         }

         block.minY = block.maxY = 0;
      }
   }



   public void LoadEdges (XmlNodeList xmlEdges, Edge[] edgeList)
   {
      string   flags;
      Edge     edge;

      for (int e = 0;  e < xmlEdges.Count;  e++)
      {
         edge = edgeList[e] = new Edge ();
         flags = xmlEdges[e].Attributes[0].Value;
         edge.isConditional   = flags[0] == 'C';
         edge.isExcept        = flags[1] == 'E';
         edge.isFallThrough   = flags[2] == 'F';
         edge.isUnconditional = flags[3] == 'U';
         edge.linkLabel = xmlEdges[e].Attributes[1].Value;
      }
   }



   public void LoadBlockInstrs (XmlNode xmlBlock, Block block)
   {
      XmlNodeList    xmlInstrs;
      Instruction          instruction;

      xmlInstrs = xmlBlock.SelectNodes ("instruction");
      block.nInstrs = xmlInstrs.Count;
      block.instructionList = new Instruction[block.nInstrs];

      for (int i = 0;  i < block.nInstrs;  i++)
      {
         instruction = block.instructionList[i] = new Instruction ();
         LoadInstr (xmlInstrs[i], instruction);
      }
   }



   public void LoadInstr (XmlNode xmlInstr, Instruction instruction)
   {
      XmlNodeList  xmlOpnds;
      XmlAttribute attr;

      instruction.opName = xmlInstr.Attributes["op"].Value;
      attr = xmlInstr.Attributes["cc"];
      if (attr != null)
      {
         instruction.ccName = attr.Value;
      }
      instruction.sourceLine = int.Parse (xmlInstr.Attributes["line"].Value);

      xmlOpnds = xmlInstr.SelectNodes ("src-operand-list/*");
      if (xmlOpnds != null  &&  xmlOpnds.Count > 0)
      {
         instruction.nSrcOpnds = xmlOpnds.Count;
         instruction.srcOperandList = new Operand[instruction.nSrcOpnds];

         for (int i = 0;  i < instruction.nSrcOpnds;  i++)
         {
            LoadOpnd (xmlOpnds[i], out instruction.srcOperandList[i]);
         }
      }

      xmlOpnds = xmlInstr.SelectNodes ("dst-operand-list/*");
      if (xmlOpnds != null  &&  xmlOpnds.Count > 0)
      {
         instruction.nDstOpnds = xmlOpnds.Count;
         instruction.dstOperandList = new Operand[instruction.nDstOpnds];

         for (int i = 0;  i < instruction.nDstOpnds;  i++)
         {
            LoadOpnd (xmlOpnds[i], out instruction.dstOperandList[i]);
         }
      }
   }



   public void LoadOpnd (XmlNode xmlOpnd, out Operand operand)
   {
      string name = xmlOpnd.Name;

      if (name == "variable-operand")
      {
         VariableOperand variableOperand = new VariableOperand();
         variableOperand.operandKind = OperandKind.Variable;
         variableOperand.display  = xmlOpnd.Attributes["display"].Value;
         //variableOperand.sym      = xmlOpnd.Attributes["sym"].Value;
         variableOperand.type     = xmlOpnd.Attributes["type"].Value;
         //variableOperand.machineRegister      = xmlOpnd.Attributes["machineRegister"].Value;
         operand = variableOperand;
      }
      else if (name == "memory-operand")
      {
         MemoryOperand memoryOperand = new MemoryOperand();
         memoryOperand.operandKind = OperandKind.Memory;
         memoryOperand.display  = xmlOpnd.Attributes["display"].Value;
         //memoryOperand.sym      = xmlOpnd.Attributes["sym"].Value;
         memoryOperand.type     = xmlOpnd.Attributes["type"].Value;
         operand = memoryOperand;
      }
      else if (name == "imm-operand")
      {
         ImmediateOperand immediateOperand = new ImmediateOperand();
         immediateOperand.operandKind = OperandKind.Immediate;
         immediateOperand.value      = xmlOpnd.Attributes[0].Value;
         immediateOperand.display  = immediateOperand.value;
         operand = immediateOperand;
      }
      else if (name == "label-operand")
      {
         LabelOperand labelOperand = new LabelOperand();
         labelOperand.operandKind = OperandKind.Label;
         labelOperand.name     = xmlOpnd.Attributes["name"].Value;
         labelOperand.kind     = xmlOpnd.Attributes["kind"].Value;
         labelOperand.jumpBlock = -1;
         labelOperand.display  = labelOperand.name;
         operand = labelOperand;
      }
      else if (name == "function-operand")
      {
         FunctionOperand functionOperand = new FunctionOperand();
         functionOperand.operandKind = OperandKind.Function;
         functionOperand.sym      = xmlOpnd.Attributes["sym"].Value;
         functionOperand.display  = functionOperand.sym;
         operand = functionOperand;
      }
      else if (name == "modref-operand")
      {
         AliasOperand modrefOpnd = new AliasOperand();
         modrefOpnd.operandKind = OperandKind.ModRef;
         modrefOpnd.aliasTag = xmlOpnd.Attributes["alias-tag"].Value;
         modrefOpnd.display  = modrefOpnd.aliasTag;
         operand = modrefOpnd;
      }
      else
      {
         operand = null;
      }
   }



   public void FixLinks (FunctionUnit functionUnit)
   {
      int         b, i, so;
      string      localString;
      Block       block;
      Instruction       instruction;
      Operand        operand;
      LabelOperand   labelOperand;
      Hashtable   ht = new Hashtable();

      // Associate dest LabelOperand's with their blocks.
      for (b = 0;  b < functionUnit.nBlocks;  b++)
      {
         block = functionUnit.blockList[b];
         instruction = block.instructionList[0];
         // Ignore jumps (they don't have dst opnds).
         if (instruction.dstOperandList != null)
         {
            // LabelOpnds are always the final operand in list.
            operand = instruction.dstOperandList[instruction.nDstOpnds-1];
            // Ignore fall-through.
            if (operand.operandKind == OperandKind.Label)
            {
               localString = (operand as LabelOperand).name;
               ht.Add (localString, b);
            }
         }
      }

      for (b = 0;  b < functionUnit.nBlocks;  b++)
      {
         block = functionUnit.blockList[b];

         // Look up block numbers for src LabelOperand's.

         for (i = 0;  i < block.nInstrs;  i++)
         {
            instruction = block.instructionList[i];
            for (so = 0;  so < instruction.nSrcOpnds;  so++)
            {
               operand = instruction.srcOperandList[so];
               if (operand.operandKind == OperandKind.Label)
               {
                  labelOperand = operand as LabelOperand;
                  labelOperand.jumpBlock = (int) ht[labelOperand.name];
               }
            }
         }

         // Look up block numbers for flowgraph successor edges.

         for (i = 0;  i < block.nSuccEdges;  i++)
         {
            block.successorEdgeList[i].linkBlock = -1;

            localString = block.successorEdgeList[i].linkLabel;
            if (localString != null  &&  ht[localString] != null)
            {
               block.successorEdgeList[i].linkBlock = (int) ht[localString];
            }
         }

      }
   }


	/// <summary>
	/// Clean up any resources being used.
	/// </summary>
	protected override void Dispose( bool disposing )
	{
		if( disposing )
		{
			if (components != null)
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
      this.mainMenu = new System.Windows.Forms.MainMenu();
      this.fileMenu = new System.Windows.Forms.MenuItem();
      this.fileOpen = new System.Windows.Forms.MenuItem();
      this.fileNew = new System.Windows.Forms.MenuItem();
      this.prefMenu = new System.Windows.Forms.MenuItem();
      this.prefFont = new System.Windows.Forms.MenuItem();
      this.menuItem1 = new System.Windows.Forms.MenuItem();
      this.colorsSubtle = new System.Windows.Forms.MenuItem();
      this.colorsNotSoSubtle = new System.Windows.Forms.MenuItem();
      this.progressBar = new System.Windows.Forms.ProgressBar();
      this.progressLabel = new System.Windows.Forms.Label();
      this.SuspendLayout();
      //
      // mainMenu
      //
      this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                              this.fileMenu,
                                                                              this.prefMenu});
      //
      // fileMenu
      //
      this.fileMenu.Index = 0;
      this.fileMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                              this.fileOpen,
                                                                              this.fileNew});
      this.fileMenu.Text = "File";
      //
      // fileOpen
      //
      this.fileOpen.Index = 0;
      this.fileOpen.Text = "Select XML directory";
      this.fileOpen.Click += new System.EventHandler(this.fileOpen_Click);
      //
      // fileNew
      //
      this.fileNew.Index = 1;
      this.fileNew.Text = "New window";
      this.fileNew.Visible = false;
      this.fileNew.Click += new System.EventHandler(this.fileNew_Click);
      //
      // prefMenu
      //
      this.prefMenu.Index = 1;
      this.prefMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                              this.prefFont,
                                                                              this.menuItem1});
      this.prefMenu.Text = "Preferences";
      //
      // prefFont
      //
      this.prefFont.Index = 0;
      this.prefFont.Text = "Font";
      this.prefFont.Click += new System.EventHandler(this.prefFont_Click);
      //
      // menuItem1
      //
      this.menuItem1.Index = 1;
      this.menuItem1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
                                                                               this.colorsSubtle,
                                                                               this.colorsNotSoSubtle});
      this.menuItem1.Text = "Colors";
      //
      // colorsSubtle
      //
      this.colorsSubtle.Index = 0;
      this.colorsSubtle.Text = "Subtle";
      this.colorsSubtle.Click += new System.EventHandler(this.menuItem2_Click);
      //
      // colorsNotSoSubtle
      //
      this.colorsNotSoSubtle.Index = 1;
      this.colorsNotSoSubtle.Text = "Not so subtle";
      this.colorsNotSoSubtle.Click += new System.EventHandler(this.colorsNotSoSubtle_Click);
      //
      // progressBar
      //
      this.progressBar.Location = new System.Drawing.Point(40, 72);
      this.progressBar.Name = "progressBar";
      this.progressBar.Size = new System.Drawing.Size(376, 32);
      this.progressBar.TabIndex = 0;
      this.progressBar.Visible = false;
      //
      // progressLabel
      //
      this.progressLabel.Font = new System.Drawing.Font("Trebuchet MS", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
      this.progressLabel.Location = new System.Drawing.Point(40, 24);
      this.progressLabel.Name = "progressLabel";
      this.progressLabel.Size = new System.Drawing.Size(352, 24);
      this.progressLabel.TabIndex = 1;
      this.progressLabel.Text = "Scanning XML files";
      this.progressLabel.Visible = false;
      //
      // MainForm
      //
      this.AutoScaleDimensions = new System.Drawing.SizeF(6, 15);
      this.BackColor = System.Drawing.SystemColors.Window;
      this.ClientSize = new System.Drawing.Size(464, 152);
      this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                   this.progressLabel,
                                                                   this.progressBar});
      this.Menu = this.mainMenu;
      this.Name = "MainForm";
      this.Text = "Phoenix IR Explorer";
      this.ResumeLayout(false);

   }
	#endregion




   private void fileOpen_Click(object sender, System.EventArgs e)
   {
      //*
      FolderBrowserDialog  fbd = new FolderBrowserDialog ();

      fbd.SelectedPath = defaultXmlDirectory;
      fbd.ShowNewFolderButton = false;

      if (fbd.ShowDialog () == DialogResult.OK)
      {

         NewDirectory (fbd.SelectedPath);

         IRDisplay   ird = new IRDisplay (displays.Count, font);
         displays.Add (ird);
         ird.Visible = true;

         fileNew.Visible = true;
      }
      //*/

      /*
      OpenFileDialog    ofd = new OpenFileDialog ();

      ofd.Title = "Locate XML directory";

      ofd.Filter = "XML files (*.xml)|*.xml|All files (*.*)|*.*";

      if (ofd.ShowDialog() == DialogResult.OK)   {

         NewDirectory (ofd.FileName);

         IRDisplay   ird = new IRDisplay (displays.Count, font);
         displays.Add (ird);
         ird.Visible = true;

         fileNew.Visible = true;
      }
      //*/
   }

   private void fileNew_Click(object sender, System.EventArgs e)
   {
      IRDisplay   displacement = new IRDisplay (displays.Count, font);
      displays.Add (displacement);
      displacement.Visible = true;
   }

   static public void DisplayClosing (int id)
   {
      displays[id] = null;
   }

   private void prefFont_Click(object sender, System.EventArgs e)
   {
      FontDialog     fd = new FontDialog();

      fd.Font = font;

      if (fd.ShowDialog() == DialogResult.OK)
      {
         font = fd.Font;
         for (int i = 0;  i < displays.Count;  i++)
         {
            if (displays[i] != null)
            {
               (displays[i] as IRDisplay).NewFont (font);
            }
         }
      }
   }

   private void colorsNotSoSubtle_Click(object sender, System.EventArgs e)
   {
      blockBrush = new SolidBrush (Color.LightSkyBlue);
      blockHitBrush = new SolidBrush (Color.Pink);
      instrHitBrush = new SolidBrush (Color.Lavender);
      for (int i = 0;  i < displays.Count;  i++)
      {
         if (displays[i] != null)
         {
            (displays[i] as IRDisplay).NewColors ();
         }
      }
   }

   private void menuItem2_Click(object sender, System.EventArgs e)
   {
      blockBrush = new SolidBrush (Color.AliceBlue);
      blockHitBrush = new SolidBrush (Color.LavenderBlush);
      instrHitBrush = new SolidBrush (Color.Lavender);
      for (int i = 0;  i < displays.Count;  i++)
      {
         if (displays[i] != null)
         {
            (displays[i] as IRDisplay).NewColors ();
         }
      }
   }
}



   public class FunctionUnit {
      public string     unitName;
      public string     irState;
      public string     phaseName;
      public string     sourceFileName;
      public int        firstSourceLine, lastSourceLine;
      public Block[]    blockList;
      public int        nBlocks;
      public int        phaseTreeIndex;
   }



   public class   Block {
      public Instruction[]    instructionList;
      public int        nInstrs;
      public int        firstSourceLine, lastSourceLine;
      public int        minY, maxY;
      public Edge[]     successorEdgeList;
      public int        nSuccEdges;
   }



   public class   Instruction {
      public string     opName;
      public string     ccName;
      public int        sourceLine;
      public Operand[]     srcOperandList;
      public int        nSrcOpnds;
      public Operand[]     dstOperandList;
      public int        nDstOpnds;
   }



   public class   Edge {
      public string     linkLabel;
      public int        linkBlock;
      public bool       isConditional, isExcept, isFallThrough, isUnconditional;
   }



   public enum OperandKind {Variable, Memory, Immediate, Label, Function, ModRef};



   public class Operand {
      public OperandKind   operandKind;
      public string     display;
      public int        minX, maxX;
      public string     tip;
   }



   public class VariableOperand : Operand {
      public string     sym;
      public string     type;
      public string     machineRegister;
      public string     temporary;
   }



   public class MemoryOperand : Operand {
      public string     sym;
      public string     type;
      public string     machineRegister;
      public bool       isAddress;
   }



   public class ImmediateOperand : Operand {
      public string     value;
   }



   public class LabelOperand : Operand {
      public string     name;
      public string     kind;
      public int        jumpBlock;
   }



   public class FunctionOperand : Operand {
      public string     sym;
   }



   public class AliasOperand : Operand {
      public string     aliasTag;
   }



public class TheApp
{
   [STAThread]
   static void Main(string[] arguments) {
      MainForm mf = new MainForm();

      // First optional command line argument is initial XML directory.
      if (arguments.Length > 0)
         MainForm.defaultXmlDirectory = arguments[0];

      // Second optional argument is initial font size in points.
      if (arguments.Length > 1)
         MainForm.font = new Font(MainForm.fontFace, float.Parse(arguments[1]));

      Application.Run(mf);
   }
}

}
