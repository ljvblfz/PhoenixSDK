namespace PEExplorer
{
   partial class Viewer
   {
      /// <summary>
      /// Required designer variable.
      /// </summary>
      private System.ComponentModel.IContainer components = null;

      /// <summary>
      /// Clean up any resources being used.
      /// </summary>
      /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
      protected override void Dispose(bool disposing)
      {
         if (disposing && (components != null))
         {
            components.Dispose();
         }
         base.Dispose(disposing);
      }

      #region Windows Form Designer generated code

      /// <summary>
      /// Required method for Designer support - do not modify
      /// the contents of this method with the code editor.
      /// </summary>
      private void InitializeComponent()
      {
         this.components = new System.ComponentModel.Container();
         System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Viewer));
         this.menuStrip1 = new System.Windows.Forms.MenuStrip();
         this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.dumpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.dumpTreeViewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.viewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.raiseToToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.showToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.sourceListingToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.iLListingToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
         this.hIRToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.mIRToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.lIRToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.eIRToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
         this.selectOperandColorToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.resetOperandColorToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.symbolsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.filterToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.showAllToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.hideAllToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.callGraphToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.callDepthToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem5 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem6 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem7 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolStripMenuItem8 = new System.Windows.Forms.ToolStripMenuItem();
         this.toolsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.runToolToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.panel1 = new System.Windows.Forms.Panel();
         this.tabControl1 = new System.Windows.Forms.TabControl();
         this.tabPageDisassembly = new System.Windows.Forms.TabPage();
         this.panel3 = new System.Windows.Forms.Panel();
         this.richTextBox1 = new System.Windows.Forms.RichTextBox();
         this.panel2 = new System.Windows.Forms.Panel();
         this.label1 = new System.Windows.Forms.Label();
         this.tabPageSymbols = new System.Windows.Forms.TabPage();
         this.listView1 = new System.Windows.Forms.ListView();
         this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
         this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
         this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
         this.columnHeader4 = new System.Windows.Forms.ColumnHeader();
         this.columnHeader5 = new System.Windows.Forms.ColumnHeader();
         this.columnHeader6 = new System.Windows.Forms.ColumnHeader();
         this.imageList = new System.Windows.Forms.ImageList(this.components);
         this.tabPageCallGraph = new System.Windows.Forms.TabPage();
         this.panel4 = new System.Windows.Forms.Panel();
         this.splitter3 = new System.Windows.Forms.Splitter();
         this.textBoxCallGraph = new System.Windows.Forms.TextBox();
         this.splitter2 = new System.Windows.Forms.Splitter();
         this.treeView1 = new System.Windows.Forms.TreeView();
         this.splitter1 = new System.Windows.Forms.Splitter();
         this.textBoxManifest = new System.Windows.Forms.TextBox();
         this.statusStrip1 = new System.Windows.Forms.StatusStrip();
         this.toolStripProgressBar1 = new System.Windows.Forms.ToolStripProgressBar();
         this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
         this.backgroundWorker1 = new System.ComponentModel.BackgroundWorker();
         this.backgroundWorker2 = new System.ComponentModel.BackgroundWorker();
         this.toolTip1 = new System.Windows.Forms.ToolTip(this.components);
         this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
         this.syncTreeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
         this.menuStrip1.SuspendLayout();
         this.panel1.SuspendLayout();
         this.tabControl1.SuspendLayout();
         this.tabPageDisassembly.SuspendLayout();
         this.panel3.SuspendLayout();
         this.panel2.SuspendLayout();
         this.tabPageSymbols.SuspendLayout();
         this.tabPageCallGraph.SuspendLayout();
         this.statusStrip1.SuspendLayout();
         this.contextMenuStrip1.SuspendLayout();
         this.SuspendLayout();
         // 
         // menuStrip1
         // 
         this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.viewToolStripMenuItem,
            this.toolsToolStripMenuItem,
            this.helpToolStripMenuItem});
         this.menuStrip1.Location = new System.Drawing.Point(0, 0);
         this.menuStrip1.Name = "menuStrip1";
         this.menuStrip1.Size = new System.Drawing.Size(632, 24);
         this.menuStrip1.TabIndex = 0;
         this.menuStrip1.Text = "menuStrip1";
         // 
         // fileToolStripMenuItem
         // 
         this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openToolStripMenuItem,
            this.dumpToolStripMenuItem,
            this.dumpTreeViewToolStripMenuItem,
            this.exitToolStripMenuItem});
         this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
         this.fileToolStripMenuItem.Size = new System.Drawing.Size(35, 20);
         this.fileToolStripMenuItem.Text = "&File";
         // 
         // openToolStripMenuItem
         // 
         this.openToolStripMenuItem.Name = "openToolStripMenuItem";
         this.openToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.O)));
         this.openToolStripMenuItem.Size = new System.Drawing.Size(186, 22);
         this.openToolStripMenuItem.Text = "&Open";
         this.openToolStripMenuItem.Click += new System.EventHandler(this.openToolStripMenuItem_Click);
         // 
         // dumpToolStripMenuItem
         // 
         this.dumpToolStripMenuItem.Enabled = false;
         this.dumpToolStripMenuItem.Name = "dumpToolStripMenuItem";
         this.dumpToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.D)));
         this.dumpToolStripMenuItem.Size = new System.Drawing.Size(186, 22);
         this.dumpToolStripMenuItem.Text = "&Dump";
         this.dumpToolStripMenuItem.Click += new System.EventHandler(this.dumpToolStripMenuItem_Click);
         // 
         // dumpTreeViewToolStripMenuItem
         // 
         this.dumpTreeViewToolStripMenuItem.Enabled = false;
         this.dumpTreeViewToolStripMenuItem.Name = "dumpTreeViewToolStripMenuItem";
         this.dumpTreeViewToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.T)));
         this.dumpTreeViewToolStripMenuItem.Size = new System.Drawing.Size(186, 22);
         this.dumpTreeViewToolStripMenuItem.Text = "Dump &TreeView";
         this.dumpTreeViewToolStripMenuItem.Click += new System.EventHandler(this.dumpTreeViewToolStripMenuItem_Click);
         // 
         // exitToolStripMenuItem
         // 
         this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
         this.exitToolStripMenuItem.ShortcutKeys = ((System.Windows.Forms.Keys)((System.Windows.Forms.Keys.Control | System.Windows.Forms.Keys.X)));
         this.exitToolStripMenuItem.Size = new System.Drawing.Size(186, 22);
         this.exitToolStripMenuItem.Text = "E&xit";
         this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
         // 
         // viewToolStripMenuItem
         // 
         this.viewToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.raiseToToolStripMenuItem,
            this.symbolsToolStripMenuItem,
            this.callGraphToolStripMenuItem});
         this.viewToolStripMenuItem.Name = "viewToolStripMenuItem";
         this.viewToolStripMenuItem.Size = new System.Drawing.Size(41, 20);
         this.viewToolStripMenuItem.Text = "&View";
         // 
         // raiseToToolStripMenuItem
         // 
         this.raiseToToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.showToolStripMenuItem,
            this.toolStripMenuItem2,
            this.selectOperandColorToolStripMenuItem,
            this.resetOperandColorToolStripMenuItem});
         this.raiseToToolStripMenuItem.Name = "raiseToToolStripMenuItem";
         this.raiseToToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
         this.raiseToToolStripMenuItem.Text = "&Disassembly";
         // 
         // showToolStripMenuItem
         // 
         this.showToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.sourceListingToolStripMenuItem,
            this.iLListingToolStripMenuItem,
            this.toolStripMenuItem1,
            this.hIRToolStripMenuItem,
            this.mIRToolStripMenuItem,
            this.lIRToolStripMenuItem,
            this.eIRToolStripMenuItem});
         this.showToolStripMenuItem.Name = "showToolStripMenuItem";
         this.showToolStripMenuItem.Size = new System.Drawing.Size(184, 22);
         this.showToolStripMenuItem.Text = "S&how";
         // 
         // sourceListingToolStripMenuItem
         // 
         this.sourceListingToolStripMenuItem.Name = "sourceListingToolStripMenuItem";
         this.sourceListingToolStripMenuItem.Size = new System.Drawing.Size(140, 22);
         this.sourceListingToolStripMenuItem.Text = "&Source Listing";
         this.sourceListingToolStripMenuItem.Click += new System.EventHandler(this.showDisassemblyMenuItem_Click);
         // 
         // iLListingToolStripMenuItem
         // 
         this.iLListingToolStripMenuItem.Name = "iLListingToolStripMenuItem";
         this.iLListingToolStripMenuItem.Size = new System.Drawing.Size(140, 22);
         this.iLListingToolStripMenuItem.Text = "IL Listing";
         this.iLListingToolStripMenuItem.Click += new System.EventHandler(this.showDisassemblyMenuItem_Click);
         // 
         // toolStripMenuItem1
         // 
         this.toolStripMenuItem1.Name = "toolStripMenuItem1";
         this.toolStripMenuItem1.Size = new System.Drawing.Size(137, 6);
         // 
         // hIRToolStripMenuItem
         // 
         this.hIRToolStripMenuItem.Name = "hIRToolStripMenuItem";
         this.hIRToolStripMenuItem.Size = new System.Drawing.Size(140, 22);
         this.hIRToolStripMenuItem.Text = "&High-level IR";
         this.hIRToolStripMenuItem.Click += new System.EventHandler(this.showDisassemblyMenuItem_Click);
         // 
         // mIRToolStripMenuItem
         // 
         this.mIRToolStripMenuItem.Name = "mIRToolStripMenuItem";
         this.mIRToolStripMenuItem.Size = new System.Drawing.Size(140, 22);
         this.mIRToolStripMenuItem.Text = "&Mid-level IR";
         this.mIRToolStripMenuItem.Click += new System.EventHandler(this.showDisassemblyMenuItem_Click);
         // 
         // lIRToolStripMenuItem
         // 
         this.lIRToolStripMenuItem.Name = "lIRToolStripMenuItem";
         this.lIRToolStripMenuItem.Size = new System.Drawing.Size(140, 22);
         this.lIRToolStripMenuItem.Text = "&Low-level IR";
         this.lIRToolStripMenuItem.Click += new System.EventHandler(this.showDisassemblyMenuItem_Click);
         // 
         // eIRToolStripMenuItem
         // 
         this.eIRToolStripMenuItem.Name = "eIRToolStripMenuItem";
         this.eIRToolStripMenuItem.Size = new System.Drawing.Size(140, 22);
         this.eIRToolStripMenuItem.Text = "&Encoded IR";
         this.eIRToolStripMenuItem.Click += new System.EventHandler(this.showDisassemblyMenuItem_Click);
         // 
         // toolStripMenuItem2
         // 
         this.toolStripMenuItem2.Name = "toolStripMenuItem2";
         this.toolStripMenuItem2.Size = new System.Drawing.Size(181, 6);
         // 
         // selectOperandColorToolStripMenuItem
         // 
         this.selectOperandColorToolStripMenuItem.Name = "selectOperandColorToolStripMenuItem";
         this.selectOperandColorToolStripMenuItem.Size = new System.Drawing.Size(184, 22);
         this.selectOperandColorToolStripMenuItem.Text = "Select Variable &Color...";
         this.selectOperandColorToolStripMenuItem.Click += new System.EventHandler(this.selectOperandColorToolStripMenuItem_Click);
         // 
         // resetOperandColorToolStripMenuItem
         // 
         this.resetOperandColorToolStripMenuItem.Name = "resetOperandColorToolStripMenuItem";
         this.resetOperandColorToolStripMenuItem.Size = new System.Drawing.Size(184, 22);
         this.resetOperandColorToolStripMenuItem.Text = "&Reset Variable Color";
         this.resetOperandColorToolStripMenuItem.Click += new System.EventHandler(this.resetOperandColorToolStripMenuItem_Click);
         // 
         // symbolsToolStripMenuItem
         // 
         this.symbolsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.filterToolStripMenuItem});
         this.symbolsToolStripMenuItem.Name = "symbolsToolStripMenuItem";
         this.symbolsToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
         this.symbolsToolStripMenuItem.Text = "&Symbols";
         // 
         // filterToolStripMenuItem
         // 
         this.filterToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.showAllToolStripMenuItem,
            this.hideAllToolStripMenuItem});
         this.filterToolStripMenuItem.Name = "filterToolStripMenuItem";
         this.filterToolStripMenuItem.Size = new System.Drawing.Size(100, 22);
         this.filterToolStripMenuItem.Text = "&Show";
         // 
         // showAllToolStripMenuItem
         // 
         this.showAllToolStripMenuItem.Name = "showAllToolStripMenuItem";
         this.showAllToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
         this.showAllToolStripMenuItem.Text = "&Show All";
         this.showAllToolStripMenuItem.Click += new System.EventHandler(this.showAllToolStripMenuItem_Click);
         // 
         // hideAllToolStripMenuItem
         // 
         this.hideAllToolStripMenuItem.Name = "hideAllToolStripMenuItem";
         this.hideAllToolStripMenuItem.Size = new System.Drawing.Size(114, 22);
         this.hideAllToolStripMenuItem.Text = "&Hide All";
         this.hideAllToolStripMenuItem.Click += new System.EventHandler(this.hideAllToolStripMenuItem_Click);
         // 
         // callGraphToolStripMenuItem
         // 
         this.callGraphToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.callDepthToolStripMenuItem});
         this.callGraphToolStripMenuItem.Name = "callGraphToolStripMenuItem";
         this.callGraphToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
         this.callGraphToolStripMenuItem.Text = "&Call Graph";
         // 
         // callDepthToolStripMenuItem
         // 
         this.callDepthToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItem4,
            this.toolStripMenuItem5,
            this.toolStripMenuItem6,
            this.toolStripMenuItem7,
            this.toolStripMenuItem8});
         this.callDepthToolStripMenuItem.Name = "callDepthToolStripMenuItem";
         this.callDepthToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
         this.callDepthToolStripMenuItem.Text = "Call &Depth";
         // 
         // toolStripMenuItem4
         // 
         this.toolStripMenuItem4.CheckOnClick = true;
         this.toolStripMenuItem4.Name = "toolStripMenuItem4";
         this.toolStripMenuItem4.Size = new System.Drawing.Size(152, 22);
         this.toolStripMenuItem4.Text = "1";
         this.toolStripMenuItem4.Click += new System.EventHandler(this.callDepthMenuItem_Clicked);
         // 
         // toolStripMenuItem5
         // 
         this.toolStripMenuItem5.CheckOnClick = true;
         this.toolStripMenuItem5.Name = "toolStripMenuItem5";
         this.toolStripMenuItem5.Size = new System.Drawing.Size(152, 22);
         this.toolStripMenuItem5.Text = "2";
         this.toolStripMenuItem5.Click += new System.EventHandler(this.callDepthMenuItem_Clicked);
         // 
         // toolStripMenuItem6
         // 
         this.toolStripMenuItem6.CheckOnClick = true;
         this.toolStripMenuItem6.Name = "toolStripMenuItem6";
         this.toolStripMenuItem6.Size = new System.Drawing.Size(152, 22);
         this.toolStripMenuItem6.Text = "3";
         this.toolStripMenuItem6.Click += new System.EventHandler(this.callDepthMenuItem_Clicked);
         // 
         // toolStripMenuItem7
         // 
         this.toolStripMenuItem7.CheckOnClick = true;
         this.toolStripMenuItem7.Name = "toolStripMenuItem7";
         this.toolStripMenuItem7.Size = new System.Drawing.Size(152, 22);
         this.toolStripMenuItem7.Text = "4";
         this.toolStripMenuItem7.Click += new System.EventHandler(this.callDepthMenuItem_Clicked);
         // 
         // toolStripMenuItem8
         // 
         this.toolStripMenuItem8.CheckOnClick = true;
         this.toolStripMenuItem8.Name = "toolStripMenuItem8";
         this.toolStripMenuItem8.Size = new System.Drawing.Size(152, 22);
         this.toolStripMenuItem8.Text = "5";
         this.toolStripMenuItem8.Click += new System.EventHandler(this.callDepthMenuItem_Clicked);
         // 
         // toolsToolStripMenuItem
         // 
         this.toolsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.runToolToolStripMenuItem});
         this.toolsToolStripMenuItem.Name = "toolsToolStripMenuItem";
         this.toolsToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
         this.toolsToolStripMenuItem.Text = "&Tools";
         // 
         // runToolToolStripMenuItem
         // 
         this.runToolToolStripMenuItem.Name = "runToolToolStripMenuItem";
         this.runToolToolStripMenuItem.Size = new System.Drawing.Size(116, 22);
         this.runToolToolStripMenuItem.Text = "&Run Tool";
         this.runToolToolStripMenuItem.Click += new System.EventHandler(this.runToolToolStripMenuItem_Click);
         // 
         // helpToolStripMenuItem
         // 
         this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem});
         this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
         this.helpToolStripMenuItem.Size = new System.Drawing.Size(40, 20);
         this.helpToolStripMenuItem.Text = "&Help";
         // 
         // aboutToolStripMenuItem
         // 
         this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
         this.aboutToolStripMenuItem.Size = new System.Drawing.Size(103, 22);
         this.aboutToolStripMenuItem.Text = "&About";
         this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
         // 
         // panel1
         // 
         this.panel1.Controls.Add(this.tabControl1);
         this.panel1.Controls.Add(this.splitter2);
         this.panel1.Controls.Add(this.treeView1);
         this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.panel1.Location = new System.Drawing.Point(0, 24);
         this.panel1.Name = "panel1";
         this.panel1.Size = new System.Drawing.Size(632, 337);
         this.panel1.TabIndex = 1;
         // 
         // tabControl1
         // 
         this.tabControl1.Controls.Add(this.tabPageDisassembly);
         this.tabControl1.Controls.Add(this.tabPageSymbols);
         this.tabControl1.Controls.Add(this.tabPageCallGraph);
         this.tabControl1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.tabControl1.Location = new System.Drawing.Point(163, 0);
         this.tabControl1.Name = "tabControl1";
         this.tabControl1.SelectedIndex = 0;
         this.tabControl1.Size = new System.Drawing.Size(469, 337);
         this.tabControl1.TabIndex = 2;
         this.tabControl1.Selected += new System.Windows.Forms.TabControlEventHandler(this.tabControl1_Selected);
         // 
         // tabPageDisassembly
         // 
         this.tabPageDisassembly.Controls.Add(this.panel3);
         this.tabPageDisassembly.Controls.Add(this.panel2);
         this.tabPageDisassembly.Location = new System.Drawing.Point(4, 22);
         this.tabPageDisassembly.Name = "tabPageDisassembly";
         this.tabPageDisassembly.Padding = new System.Windows.Forms.Padding(3);
         this.tabPageDisassembly.Size = new System.Drawing.Size(461, 311);
         this.tabPageDisassembly.TabIndex = 0;
         this.tabPageDisassembly.Text = "Disassembly";
         this.tabPageDisassembly.UseVisualStyleBackColor = true;
         // 
         // panel3
         // 
         this.panel3.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
         this.panel3.Controls.Add(this.richTextBox1);
         this.panel3.Dock = System.Windows.Forms.DockStyle.Fill;
         this.panel3.Location = new System.Drawing.Point(43, 3);
         this.panel3.Name = "panel3";
         this.panel3.Size = new System.Drawing.Size(415, 305);
         this.panel3.TabIndex = 5;
         // 
         // richTextBox1
         // 
         this.richTextBox1.BackColor = System.Drawing.Color.Beige;
         this.richTextBox1.BorderStyle = System.Windows.Forms.BorderStyle.None;
         this.richTextBox1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.richTextBox1.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
         this.richTextBox1.Location = new System.Drawing.Point(0, 0);
         this.richTextBox1.Margin = new System.Windows.Forms.Padding(3, 0, 3, 0);
         this.richTextBox1.Name = "richTextBox1";
         this.richTextBox1.ReadOnly = true;
         this.richTextBox1.Size = new System.Drawing.Size(413, 303);
         this.richTextBox1.TabIndex = 2;
         this.richTextBox1.Text = "";
         this.richTextBox1.WordWrap = false;
         this.richTextBox1.Resize += new System.EventHandler(this.richTextBox1_Resize);
         this.richTextBox1.MouseMove += new System.Windows.Forms.MouseEventHandler(this.richTextBox1_MouseMove);
         this.richTextBox1.VScroll += new System.EventHandler(this.richTextBox1_VScroll);
         // 
         // panel2
         // 
         this.panel2.Controls.Add(this.label1);
         this.panel2.Dock = System.Windows.Forms.DockStyle.Left;
         this.panel2.Font = new System.Drawing.Font("Courier New", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
         this.panel2.Location = new System.Drawing.Point(3, 3);
         this.panel2.Name = "panel2";
         this.panel2.Size = new System.Drawing.Size(40, 305);
         this.panel2.TabIndex = 4;
         // 
         // label1
         // 
         this.label1.Location = new System.Drawing.Point(0, 3);
         this.label1.Name = "label1";
         this.label1.Size = new System.Drawing.Size(33, 167);
         this.label1.TabIndex = 3;
         // 
         // tabPageSymbols
         // 
         this.tabPageSymbols.Controls.Add(this.listView1);
         this.tabPageSymbols.Location = new System.Drawing.Point(4, 22);
         this.tabPageSymbols.Name = "tabPageSymbols";
         this.tabPageSymbols.Padding = new System.Windows.Forms.Padding(3);
         this.tabPageSymbols.Size = new System.Drawing.Size(461, 311);
         this.tabPageSymbols.TabIndex = 1;
         this.tabPageSymbols.Text = "Symbols";
         this.tabPageSymbols.UseVisualStyleBackColor = true;
         // 
         // listView1
         // 
         this.listView1.BackColor = System.Drawing.Color.Beige;
         this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2,
            this.columnHeader3,
            this.columnHeader4,
            this.columnHeader5,
            this.columnHeader6});
         this.listView1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.listView1.FullRowSelect = true;
         this.listView1.LargeImageList = this.imageList;
         this.listView1.Location = new System.Drawing.Point(3, 3);
         this.listView1.MultiSelect = false;
         this.listView1.Name = "listView1";
         this.listView1.Size = new System.Drawing.Size(455, 305);
         this.listView1.SmallImageList = this.imageList;
         this.listView1.Sorting = System.Windows.Forms.SortOrder.Ascending;
         this.listView1.TabIndex = 0;
         this.listView1.UseCompatibleStateImageBehavior = false;
         this.listView1.View = System.Windows.Forms.View.Details;
         // 
         // columnHeader1
         // 
         this.columnHeader1.Text = "Name";
         this.columnHeader1.Width = 86;
         // 
         // columnHeader2
         // 
         this.columnHeader2.Text = "Rva";
         this.columnHeader2.Width = 73;
         // 
         // columnHeader3
         // 
         this.columnHeader3.Text = "Section";
         this.columnHeader3.Width = 48;
         // 
         // columnHeader4
         // 
         this.columnHeader4.Text = "ExternId";
         this.columnHeader4.Width = 113;
         // 
         // columnHeader5
         // 
         this.columnHeader5.Text = "LocalId";
         this.columnHeader5.Width = 100;
         // 
         // columnHeader6
         // 
         this.columnHeader6.Text = "Type";
         // 
         // imageList
         // 
         this.imageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imageList.ImageStream")));
         this.imageList.TransparentColor = System.Drawing.Color.White;
         this.imageList.Images.SetKeyName(0, "blank.bmp");
         this.imageList.Images.SetKeyName(1, "field.bmp");
         this.imageList.Images.SetKeyName(2, "sfield.bmp");
         this.imageList.Images.SetKeyName(3, "interface.bmp");
         this.imageList.Images.SetKeyName(4, "ginterface.bmp");
         this.imageList.Images.SetKeyName(5, "class.bmp");
         this.imageList.Images.SetKeyName(6, "gclass.bmp");
         this.imageList.Images.SetKeyName(7, "valuetype.bmp");
         this.imageList.Images.SetKeyName(8, "gvaluetype.bmp");
         this.imageList.Images.SetKeyName(9, "enum.bmp");
         this.imageList.Images.SetKeyName(10, "genum.bmp");
         this.imageList.Images.SetKeyName(11, "imethod.bmp");
         this.imageList.Images.SetKeyName(12, "gimethod.bmp");
         this.imageList.Images.SetKeyName(13, "nmethod.bmp");
         this.imageList.Images.SetKeyName(14, "smethod.bmp");
         this.imageList.Images.SetKeyName(15, "gsmethod.bmp");
         this.imageList.Images.SetKeyName(16, "property.bmp");
         this.imageList.Images.SetKeyName(17, "info.bmp");
         this.imageList.Images.SetKeyName(18, "attribute.bmp");
         this.imageList.Images.SetKeyName(19, "override.bmp");
         this.imageList.Images.SetKeyName(20, "manifest.bmp");
         this.imageList.Images.SetKeyName(21, "event.bmp");
         this.imageList.Images.SetKeyName(22, "namespace.bmp");
         this.imageList.Images.SetKeyName(23, "field.bmp");
         this.imageList.Images.SetKeyName(24, "local.bmp");
         this.imageList.Images.SetKeyName(25, "global.bmp");
         this.imageList.Images.SetKeyName(26, "bad.bmp");
         // 
         // tabPageCallGraph
         // 
         this.tabPageCallGraph.Controls.Add(this.panel4);
         this.tabPageCallGraph.Controls.Add(this.splitter3);
         this.tabPageCallGraph.Controls.Add(this.textBoxCallGraph);
         this.tabPageCallGraph.Location = new System.Drawing.Point(4, 22);
         this.tabPageCallGraph.Name = "tabPageCallGraph";
         this.tabPageCallGraph.Size = new System.Drawing.Size(461, 311);
         this.tabPageCallGraph.TabIndex = 2;
         this.tabPageCallGraph.Text = "Call Graph";
         this.tabPageCallGraph.UseVisualStyleBackColor = true;
         // 
         // panel4
         // 
         this.panel4.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
         this.panel4.Dock = System.Windows.Forms.DockStyle.Fill;
         this.panel4.Location = new System.Drawing.Point(0, 0);
         this.panel4.Name = "panel4";
         this.panel4.Size = new System.Drawing.Size(461, 248);
         this.panel4.TabIndex = 1;
         // 
         // splitter3
         // 
         this.splitter3.Dock = System.Windows.Forms.DockStyle.Bottom;
         this.splitter3.Location = new System.Drawing.Point(0, 248);
         this.splitter3.Name = "splitter3";
         this.splitter3.Size = new System.Drawing.Size(461, 3);
         this.splitter3.TabIndex = 5;
         this.splitter3.TabStop = false;
         // 
         // textBoxCallGraph
         // 
         this.textBoxCallGraph.Dock = System.Windows.Forms.DockStyle.Bottom;
         this.textBoxCallGraph.Location = new System.Drawing.Point(0, 251);
         this.textBoxCallGraph.Multiline = true;
         this.textBoxCallGraph.Name = "textBoxCallGraph";
         this.textBoxCallGraph.ReadOnly = true;
         this.textBoxCallGraph.ScrollBars = System.Windows.Forms.ScrollBars.Both;
         this.textBoxCallGraph.Size = new System.Drawing.Size(461, 60);
         this.textBoxCallGraph.TabIndex = 4;
         // 
         // splitter2
         // 
         this.splitter2.Location = new System.Drawing.Point(160, 0);
         this.splitter2.Name = "splitter2";
         this.splitter2.Size = new System.Drawing.Size(3, 337);
         this.splitter2.TabIndex = 1;
         this.splitter2.TabStop = false;
         // 
         // treeView1
         // 
         this.treeView1.BackColor = System.Drawing.Color.Beige;
         this.treeView1.Dock = System.Windows.Forms.DockStyle.Left;
         this.treeView1.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
         this.treeView1.HideSelection = false;
         this.treeView1.ImageIndex = 0;
         this.treeView1.ImageList = this.imageList;
         this.treeView1.ItemHeight = 17;
         this.treeView1.Location = new System.Drawing.Point(0, 0);
         this.treeView1.Name = "treeView1";
         this.treeView1.SelectedImageIndex = 0;
         this.treeView1.Size = new System.Drawing.Size(160, 337);
         this.treeView1.TabIndex = 0;
         this.treeView1.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.treeView1_AfterSelect);
         this.treeView1.BeforeSelect += new System.Windows.Forms.TreeViewCancelEventHandler(this.treeView1_BeforeSelect);
         // 
         // splitter1
         // 
         this.splitter1.Cursor = System.Windows.Forms.Cursors.HSplit;
         this.splitter1.Dock = System.Windows.Forms.DockStyle.Bottom;
         this.splitter1.Location = new System.Drawing.Point(0, 361);
         this.splitter1.Name = "splitter1";
         this.splitter1.Size = new System.Drawing.Size(632, 3);
         this.splitter1.TabIndex = 2;
         this.splitter1.TabStop = false;
         // 
         // textBoxManifest
         // 
         this.textBoxManifest.Dock = System.Windows.Forms.DockStyle.Bottom;
         this.textBoxManifest.Location = new System.Drawing.Point(0, 364);
         this.textBoxManifest.Multiline = true;
         this.textBoxManifest.Name = "textBoxManifest";
         this.textBoxManifest.ReadOnly = true;
         this.textBoxManifest.ScrollBars = System.Windows.Forms.ScrollBars.Both;
         this.textBoxManifest.Size = new System.Drawing.Size(632, 60);
         this.textBoxManifest.TabIndex = 3;
         // 
         // statusStrip1
         // 
         this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripProgressBar1,
            this.toolStripStatusLabel1});
         this.statusStrip1.Location = new System.Drawing.Point(0, 424);
         this.statusStrip1.Name = "statusStrip1";
         this.statusStrip1.Size = new System.Drawing.Size(632, 22);
         this.statusStrip1.TabIndex = 4;
         this.statusStrip1.Text = "statusStrip1";
         // 
         // toolStripProgressBar1
         // 
         this.toolStripProgressBar1.Name = "toolStripProgressBar1";
         this.toolStripProgressBar1.Size = new System.Drawing.Size(100, 16);
         this.toolStripProgressBar1.Style = System.Windows.Forms.ProgressBarStyle.Marquee;
         this.toolStripProgressBar1.Visible = false;
         // 
         // toolStripStatusLabel1
         // 
         this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
         this.toolStripStatusLabel1.Size = new System.Drawing.Size(38, 17);
         this.toolStripStatusLabel1.Text = "Ready";
         // 
         // backgroundWorker1
         // 
         this.backgroundWorker1.WorkerSupportsCancellation = true;
         this.backgroundWorker1.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorker1_DoWork);
         // 
         // backgroundWorker2
         // 
         this.backgroundWorker2.WorkerSupportsCancellation = true;
         this.backgroundWorker2.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorker2_DoWork);
         // 
         // toolTip1
         // 
         this.toolTip1.AutoPopDelay = 5000;
         this.toolTip1.InitialDelay = 1000;
         this.toolTip1.ReshowDelay = 1;
         this.toolTip1.ShowAlways = true;
         // 
         // contextMenuStrip1
         // 
         this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.syncTreeToolStripMenuItem});
         this.contextMenuStrip1.Name = "contextMenuStrip1";
         this.contextMenuStrip1.Size = new System.Drawing.Size(153, 48);
         // 
         // syncTreeToolStripMenuItem
         // 
         this.syncTreeToolStripMenuItem.Name = "syncTreeToolStripMenuItem";
         this.syncTreeToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
         this.syncTreeToolStripMenuItem.Text = "&Sync Tree";
         this.syncTreeToolStripMenuItem.Click += new System.EventHandler(this.syncTreeToolStripMenuItem_Click);
         // 
         // Form1
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.ClientSize = new System.Drawing.Size(632, 446);
         this.Controls.Add(this.panel1);
         this.Controls.Add(this.splitter1);
         this.Controls.Add(this.menuStrip1);
         this.Controls.Add(this.textBoxManifest);
         this.Controls.Add(this.statusStrip1);
         this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
         this.MainMenuStrip = this.menuStrip1;
         this.Name = "Form1";
         this.Text = "Phoenix PE Explorer";
         this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
         this.menuStrip1.ResumeLayout(false);
         this.menuStrip1.PerformLayout();
         this.panel1.ResumeLayout(false);
         this.tabControl1.ResumeLayout(false);
         this.tabPageDisassembly.ResumeLayout(false);
         this.panel3.ResumeLayout(false);
         this.panel2.ResumeLayout(false);
         this.tabPageSymbols.ResumeLayout(false);
         this.tabPageCallGraph.ResumeLayout(false);
         this.tabPageCallGraph.PerformLayout();
         this.statusStrip1.ResumeLayout(false);
         this.statusStrip1.PerformLayout();
         this.contextMenuStrip1.ResumeLayout(false);
         this.ResumeLayout(false);
         this.PerformLayout();

      }

      #endregion

      private System.Windows.Forms.MenuStrip menuStrip1;
      private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
      private System.Windows.Forms.Panel panel1;
      private System.Windows.Forms.TreeView treeView1;
      private System.Windows.Forms.Splitter splitter1;
      private System.Windows.Forms.TextBox textBoxManifest;
      private System.Windows.Forms.TabControl tabControl1;
      private System.Windows.Forms.TabPage tabPageDisassembly;
      private System.Windows.Forms.TabPage tabPageSymbols;
      private System.Windows.Forms.Splitter splitter2;
      private System.Windows.Forms.RichTextBox richTextBox1;
      private System.Windows.Forms.Label label1;
      private System.Windows.Forms.Panel panel2;
      private System.Windows.Forms.ImageList imageList;
      private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
      private System.Windows.Forms.Panel panel3;
      private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem dumpToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem dumpTreeViewToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem viewToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem raiseToToolStripMenuItem;
      private System.Windows.Forms.StatusStrip statusStrip1;
      private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
      private System.ComponentModel.BackgroundWorker backgroundWorker1;
      private System.Windows.Forms.ToolStripProgressBar toolStripProgressBar1;
      private System.Windows.Forms.ToolStripMenuItem toolsToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem runToolToolStripMenuItem;
      private System.Windows.Forms.ListView listView1;
      private System.Windows.Forms.ColumnHeader columnHeader1;
      private System.Windows.Forms.ColumnHeader columnHeader2;
      private System.Windows.Forms.ColumnHeader columnHeader3;
      private System.Windows.Forms.ColumnHeader columnHeader4;
      private System.Windows.Forms.ColumnHeader columnHeader5;
      private System.Windows.Forms.ColumnHeader columnHeader6;
      private System.ComponentModel.BackgroundWorker backgroundWorker2;
      private System.Windows.Forms.ToolTip toolTip1;
      private System.Windows.Forms.ToolStripMenuItem showToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem sourceListingToolStripMenuItem;
      private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
      private System.Windows.Forms.ToolStripMenuItem hIRToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem mIRToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem lIRToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem eIRToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem symbolsToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem filterToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem showAllToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem hideAllToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem selectOperandColorToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem resetOperandColorToolStripMenuItem;
      private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
      private System.Windows.Forms.TabPage tabPageCallGraph;
      private System.Windows.Forms.TextBox textBoxCallGraph;
      private System.Windows.Forms.Panel panel4;
      private System.Windows.Forms.Splitter splitter3;
      private System.Windows.Forms.ToolStripMenuItem callGraphToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem callDepthToolStripMenuItem;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem4;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem5;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem6;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem7;
      private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem8;
      private System.Windows.Forms.ToolStripMenuItem iLListingToolStripMenuItem;
      private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
      private System.Windows.Forms.ToolStripMenuItem syncTreeToolStripMenuItem;
   }
}

