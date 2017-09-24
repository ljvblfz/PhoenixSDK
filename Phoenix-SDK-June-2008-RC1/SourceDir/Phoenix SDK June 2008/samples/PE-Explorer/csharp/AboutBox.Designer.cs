namespace PEExplorer
{
   partial class AboutBox
   {
      /// <summary>
      /// Required designer variable.
      /// </summary>
      private System.ComponentModel.IContainer components = null;

      /// <summary>
      /// Clean up any resources being used.
      /// </summary>
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
         System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AboutBox));
         this.tableLayoutPanel = new System.Windows.Forms.TableLayoutPanel();
         this.logoPictureBox = new System.Windows.Forms.PictureBox();
         this.labelProductName = new System.Windows.Forms.Label();
         this.labelVersion = new System.Windows.Forms.Label();
         this.labelCopyright = new System.Windows.Forms.Label();
         this.labelCompanyName = new System.Windows.Forms.Label();
         this.listView1 = new System.Windows.Forms.ListView();
         this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
         this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
         this.label1 = new System.Windows.Forms.Label();
         this.buttonExplore = new System.Windows.Forms.Button();
         this.okButton = new System.Windows.Forms.Button();
         this.tableLayoutPanel.SuspendLayout();
         ((System.ComponentModel.ISupportInitialize)(this.logoPictureBox)).BeginInit();
         this.SuspendLayout();
         // 
         // tableLayoutPanel
         // 
         this.tableLayoutPanel.ColumnCount = 2;
         this.tableLayoutPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 33F));
         this.tableLayoutPanel.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 67F));
         this.tableLayoutPanel.Controls.Add(this.logoPictureBox, 0, 0);
         this.tableLayoutPanel.Controls.Add(this.labelProductName, 1, 0);
         this.tableLayoutPanel.Controls.Add(this.labelVersion, 1, 1);
         this.tableLayoutPanel.Controls.Add(this.labelCopyright, 1, 2);
         this.tableLayoutPanel.Controls.Add(this.labelCompanyName, 1, 3);
         this.tableLayoutPanel.Controls.Add(this.listView1, 1, 5);
         this.tableLayoutPanel.Controls.Add(this.label1, 1, 4);
         this.tableLayoutPanel.Controls.Add(this.buttonExplore, 1, 6);
         this.tableLayoutPanel.Controls.Add(this.okButton, 1, 7);
         this.tableLayoutPanel.Dock = System.Windows.Forms.DockStyle.Fill;
         this.tableLayoutPanel.Location = new System.Drawing.Point(9, 9);
         this.tableLayoutPanel.Name = "tableLayoutPanel";
         this.tableLayoutPanel.RowCount = 8;
         this.tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 8F));
         this.tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 8F));
         this.tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 8F));
         this.tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 8F));
         this.tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 8F));
         this.tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 40F));
         this.tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 10.56604F));
         this.tableLayoutPanel.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 10.18868F));
         this.tableLayoutPanel.Size = new System.Drawing.Size(417, 265);
         this.tableLayoutPanel.TabIndex = 0;
         // 
         // logoPictureBox
         // 
         this.logoPictureBox.BackColor = System.Drawing.Color.Black;
         this.logoPictureBox.Dock = System.Windows.Forms.DockStyle.Fill;
         this.logoPictureBox.Image = ((System.Drawing.Image)(resources.GetObject("logoPictureBox.Image")));
         this.logoPictureBox.Location = new System.Drawing.Point(3, 3);
         this.logoPictureBox.Name = "logoPictureBox";
         this.tableLayoutPanel.SetRowSpan(this.logoPictureBox, 7);
         this.logoPictureBox.Size = new System.Drawing.Size(131, 231);
         this.logoPictureBox.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
         this.logoPictureBox.TabIndex = 12;
         this.logoPictureBox.TabStop = false;
         // 
         // labelProductName
         // 
         this.labelProductName.Dock = System.Windows.Forms.DockStyle.Fill;
         this.labelProductName.Location = new System.Drawing.Point(143, 0);
         this.labelProductName.Margin = new System.Windows.Forms.Padding(6, 0, 3, 0);
         this.labelProductName.MaximumSize = new System.Drawing.Size(0, 17);
         this.labelProductName.Name = "labelProductName";
         this.labelProductName.Size = new System.Drawing.Size(271, 17);
         this.labelProductName.TabIndex = 19;
         this.labelProductName.Text = "Product Name";
         this.labelProductName.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
         // 
         // labelVersion
         // 
         this.labelVersion.Dock = System.Windows.Forms.DockStyle.Fill;
         this.labelVersion.Location = new System.Drawing.Point(143, 21);
         this.labelVersion.Margin = new System.Windows.Forms.Padding(6, 0, 3, 0);
         this.labelVersion.MaximumSize = new System.Drawing.Size(0, 17);
         this.labelVersion.Name = "labelVersion";
         this.labelVersion.Size = new System.Drawing.Size(271, 17);
         this.labelVersion.TabIndex = 0;
         this.labelVersion.Text = "Version";
         this.labelVersion.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
         // 
         // labelCopyright
         // 
         this.labelCopyright.Dock = System.Windows.Forms.DockStyle.Fill;
         this.labelCopyright.Location = new System.Drawing.Point(143, 42);
         this.labelCopyright.Margin = new System.Windows.Forms.Padding(6, 0, 3, 0);
         this.labelCopyright.MaximumSize = new System.Drawing.Size(0, 17);
         this.labelCopyright.Name = "labelCopyright";
         this.labelCopyright.Size = new System.Drawing.Size(271, 17);
         this.labelCopyright.TabIndex = 21;
         this.labelCopyright.Text = "Copyright";
         this.labelCopyright.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
         // 
         // labelCompanyName
         // 
         this.labelCompanyName.Dock = System.Windows.Forms.DockStyle.Fill;
         this.labelCompanyName.Location = new System.Drawing.Point(143, 63);
         this.labelCompanyName.Margin = new System.Windows.Forms.Padding(6, 0, 3, 0);
         this.labelCompanyName.MaximumSize = new System.Drawing.Size(0, 17);
         this.labelCompanyName.Name = "labelCompanyName";
         this.labelCompanyName.Size = new System.Drawing.Size(271, 17);
         this.labelCompanyName.TabIndex = 22;
         this.labelCompanyName.Text = "Company Name";
         this.labelCompanyName.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
         // 
         // listView1
         // 
         this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1,
            this.columnHeader2});
         this.listView1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.listView1.FullRowSelect = true;
         this.listView1.GridLines = true;
         this.listView1.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
         this.listView1.Location = new System.Drawing.Point(140, 108);
         this.listView1.Name = "listView1";
         this.listView1.Size = new System.Drawing.Size(274, 99);
         this.listView1.Sorting = System.Windows.Forms.SortOrder.Descending;
         this.listView1.TabIndex = 25;
         this.listView1.UseCompatibleStateImageBehavior = false;
         this.listView1.View = System.Windows.Forms.View.Details;
         this.listView1.SelectedIndexChanged += new System.EventHandler(this.listView1_SelectedIndexChanged);
         // 
         // columnHeader1
         // 
         this.columnHeader1.Text = "Name";
         this.columnHeader1.Width = 130;
         // 
         // columnHeader2
         // 
         this.columnHeader2.Text = "Version";
         this.columnHeader2.Width = 202;
         // 
         // label1
         // 
         this.label1.AutoSize = true;
         this.label1.Dock = System.Windows.Forms.DockStyle.Fill;
         this.label1.Location = new System.Drawing.Point(140, 84);
         this.label1.Name = "label1";
         this.label1.Size = new System.Drawing.Size(274, 21);
         this.label1.TabIndex = 26;
         this.label1.Text = "Loaded Assemblies:";
         this.label1.TextAlign = System.Drawing.ContentAlignment.BottomLeft;
         // 
         // buttonExplore
         // 
         this.buttonExplore.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
         this.buttonExplore.Enabled = false;
         this.buttonExplore.Location = new System.Drawing.Point(262, 213);
         this.buttonExplore.Name = "buttonExplore";
         this.buttonExplore.Size = new System.Drawing.Size(152, 21);
         this.buttonExplore.TabIndex = 27;
         this.buttonExplore.Text = "Explore Selected Assembly";
         this.buttonExplore.UseVisualStyleBackColor = true;
         this.buttonExplore.Click += new System.EventHandler(this.buttonExplore_Click);
         // 
         // okButton
         // 
         this.okButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
         this.okButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
         this.okButton.Location = new System.Drawing.Point(339, 240);
         this.okButton.Name = "okButton";
         this.okButton.Size = new System.Drawing.Size(75, 22);
         this.okButton.TabIndex = 24;
         this.okButton.Text = "&OK";
         // 
         // AboutBox
         // 
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.ClientSize = new System.Drawing.Size(435, 283);
         this.Controls.Add(this.tableLayoutPanel);
         this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
         this.MaximizeBox = false;
         this.MinimizeBox = false;
         this.Name = "AboutBox";
         this.Padding = new System.Windows.Forms.Padding(9);
         this.ShowIcon = false;
         this.ShowInTaskbar = false;
         this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
         this.Text = "About Phoenix PE Explorer";
         this.tableLayoutPanel.ResumeLayout(false);
         this.tableLayoutPanel.PerformLayout();
         ((System.ComponentModel.ISupportInitialize)(this.logoPictureBox)).EndInit();
         this.ResumeLayout(false);

      }

      #endregion

      private System.Windows.Forms.TableLayoutPanel tableLayoutPanel;
      private System.Windows.Forms.PictureBox logoPictureBox;
      private System.Windows.Forms.Label labelProductName;
      private System.Windows.Forms.Label labelVersion;
      private System.Windows.Forms.Label labelCopyright;
      private System.Windows.Forms.Label labelCompanyName;
      private System.Windows.Forms.Button okButton;
      private System.Windows.Forms.ListView listView1;
      private System.Windows.Forms.ColumnHeader columnHeader1;
      private System.Windows.Forms.ColumnHeader columnHeader2;
      private System.Windows.Forms.Label label1;
      private System.Windows.Forms.Button buttonExplore;
   }
}
