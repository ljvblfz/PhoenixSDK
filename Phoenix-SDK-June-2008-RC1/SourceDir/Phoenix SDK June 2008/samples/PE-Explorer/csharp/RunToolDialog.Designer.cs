namespace PEExplorer
{
   partial class RunToolDialog
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
         this.textBoxCommandLine = new System.Windows.Forms.TextBox();
         this.textBoxResultPath = new System.Windows.Forms.TextBox();
         this.label1 = new System.Windows.Forms.Label();
         this.label2 = new System.Windows.Forms.Label();
         this.buttonSelectPath = new System.Windows.Forms.Button();
         this.buttonOK = new System.Windows.Forms.Button();
         this.buttonCancel = new System.Windows.Forms.Button();
         this.buttonToolPath = new System.Windows.Forms.Button();
         this.label3 = new System.Windows.Forms.Label();
         this.textBoxToolPath = new System.Windows.Forms.TextBox();
         this.SuspendLayout();
         // 
         // textBoxCommandLine
         // 
         this.textBoxCommandLine.Location = new System.Drawing.Point(12, 64);
         this.textBoxCommandLine.Name = "textBoxCommandLine";
         this.textBoxCommandLine.Size = new System.Drawing.Size(354, 20);
         this.textBoxCommandLine.TabIndex = 0;
         // 
         // textBoxResultPath
         // 
         this.textBoxResultPath.Location = new System.Drawing.Point(12, 108);
         this.textBoxResultPath.Name = "textBoxResultPath";
         this.textBoxResultPath.Size = new System.Drawing.Size(354, 20);
         this.textBoxResultPath.TabIndex = 2;
         this.textBoxResultPath.TextChanged += new System.EventHandler(this.textBoxResultPath_TextChanged);
         // 
         // label1
         // 
         this.label1.AutoSize = true;
         this.label1.Location = new System.Drawing.Point(9, 48);
         this.label1.Name = "label1";
         this.label1.Size = new System.Drawing.Size(73, 13);
         this.label1.TabIndex = 3;
         this.label1.Text = "Command line";
         // 
         // label2
         // 
         this.label2.AutoSize = true;
         this.label2.Location = new System.Drawing.Point(9, 92);
         this.label2.Name = "label2";
         this.label2.Size = new System.Drawing.Size(77, 13);
         this.label2.TabIndex = 4;
         this.label2.Text = "Result file path";
         // 
         // buttonSelectPath
         // 
         this.buttonSelectPath.Location = new System.Drawing.Point(367, 106);
         this.buttonSelectPath.Name = "buttonSelectPath";
         this.buttonSelectPath.Size = new System.Drawing.Size(26, 23);
         this.buttonSelectPath.TabIndex = 7;
         this.buttonSelectPath.Text = "...";
         this.buttonSelectPath.UseVisualStyleBackColor = true;
         this.buttonSelectPath.Click += new System.EventHandler(this.buttonSelectPath_Click);
         // 
         // buttonOK
         // 
         this.buttonOK.DialogResult = System.Windows.Forms.DialogResult.OK;
         this.buttonOK.Enabled = false;
         this.buttonOK.Location = new System.Drawing.Point(239, 139);
         this.buttonOK.Name = "buttonOK";
         this.buttonOK.Size = new System.Drawing.Size(75, 23);
         this.buttonOK.TabIndex = 9;
         this.buttonOK.Text = "&OK";
         this.buttonOK.UseVisualStyleBackColor = true;
         this.buttonOK.Click += new System.EventHandler(this.buttonOK_Click);
         // 
         // buttonCancel
         // 
         this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
         this.buttonCancel.Location = new System.Drawing.Point(321, 139);
         this.buttonCancel.Name = "buttonCancel";
         this.buttonCancel.Size = new System.Drawing.Size(75, 23);
         this.buttonCancel.TabIndex = 10;
         this.buttonCancel.Text = "&Cancel";
         this.buttonCancel.UseVisualStyleBackColor = true;
         // 
         // buttonToolPath
         // 
         this.buttonToolPath.Location = new System.Drawing.Point(367, 23);
         this.buttonToolPath.Name = "buttonToolPath";
         this.buttonToolPath.Size = new System.Drawing.Size(26, 23);
         this.buttonToolPath.TabIndex = 13;
         this.buttonToolPath.Text = "...";
         this.buttonToolPath.UseVisualStyleBackColor = true;
         this.buttonToolPath.Click += new System.EventHandler(this.buttonToolPath_Click);
         // 
         // label3
         // 
         this.label3.AutoSize = true;
         this.label3.Location = new System.Drawing.Point(9, 9);
         this.label3.Name = "label3";
         this.label3.Size = new System.Drawing.Size(52, 13);
         this.label3.TabIndex = 12;
         this.label3.Text = "Tool path";
         // 
         // textBoxToolPath
         // 
         this.textBoxToolPath.Location = new System.Drawing.Point(12, 25);
         this.textBoxToolPath.Name = "textBoxToolPath";
         this.textBoxToolPath.Size = new System.Drawing.Size(354, 20);
         this.textBoxToolPath.TabIndex = 11;
         this.textBoxToolPath.TextChanged += new System.EventHandler(this.textBoxToolPath_TextChanged);
         // 
         // RunToolDialog
         // 
         this.AcceptButton = this.buttonOK;
         this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
         this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
         this.CancelButton = this.buttonCancel;
         this.ClientSize = new System.Drawing.Size(408, 174);
         this.Controls.Add(this.buttonToolPath);
         this.Controls.Add(this.label3);
         this.Controls.Add(this.textBoxToolPath);
         this.Controls.Add(this.buttonCancel);
         this.Controls.Add(this.buttonOK);
         this.Controls.Add(this.buttonSelectPath);
         this.Controls.Add(this.label2);
         this.Controls.Add(this.label1);
         this.Controls.Add(this.textBoxResultPath);
         this.Controls.Add(this.textBoxCommandLine);
         this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
         this.MaximizeBox = false;
         this.MinimizeBox = false;
         this.Name = "RunToolDialog";
         this.ShowIcon = false;
         this.ShowInTaskbar = false;
         this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
         this.Text = "Run Tool";
         this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.RunToolDialog_FormClosed);
         this.ResumeLayout(false);
         this.PerformLayout();

      }

      #endregion

      private System.Windows.Forms.TextBox textBoxCommandLine;
      private System.Windows.Forms.TextBox textBoxResultPath;
      private System.Windows.Forms.Label label1;
      private System.Windows.Forms.Label label2;
      private System.Windows.Forms.Button buttonSelectPath;
      private System.Windows.Forms.Button buttonOK;
      private System.Windows.Forms.Button buttonCancel;
      private System.Windows.Forms.Button buttonToolPath;
      private System.Windows.Forms.Label label3;
      private System.Windows.Forms.TextBox textBoxToolPath;
   }
}