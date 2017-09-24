using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace PEExplorer
{
   public partial class RunToolDialog : Form
   {
      private static string previousToolpath = string.Empty;
      private static string previousCommandLine = string.Empty;
      private static string previousResultPath = string.Empty;

      public string ToolPath
      {
         get { return this.textBoxToolPath.Text; }
      }

      public string CommandLine
      {
         get { return this.textBoxCommandLine.Text; }
      }

      public string ResultPath
      {
         get { return this.textBoxResultPath.Text; }
      }

      public RunToolDialog()
      {
         InitializeComponent();

         this.textBoxToolPath.Text = previousToolpath;
         this.textBoxCommandLine.Text = previousCommandLine;
         this.textBoxResultPath.Text = previousResultPath;
      }

      private void textBoxResultPath_TextChanged(object sender, EventArgs e)
      {
         EnableOkButton();
      }

      private void textBoxToolPath_TextChanged(object sender, EventArgs e)
      {
         EnableOkButton();
      }

      private void EnableOkButton()
      {
         this.buttonOK.Enabled =
            this.textBoxToolPath.Text.Trim().Length > 0 &&
            this.textBoxResultPath.Text.Trim().Length > 0;
      }

      private void RunToolDialog_FormClosed(object sender, FormClosedEventArgs e)
      {
         if (this.DialogResult == DialogResult.OK)
         {
            previousToolpath = this.textBoxToolPath.Text;
            previousCommandLine = this.textBoxCommandLine.Text;
            previousResultPath = this.textBoxResultPath.Text;
         }
      }

      private void buttonOK_Click(object sender, EventArgs e)
      {
         this.Close();
      }

      private void buttonSelectPath_Click(object sender, EventArgs e)
      {         
         OpenFileDialog dialog = new OpenFileDialog();
         dialog.CheckFileExists = false;
         dialog.CheckPathExists = false;
         dialog.Multiselect = false;
         dialog.Filter = 
            "PE Files (*.exe;*.dll;*.mod;*.mdl)|*.exe;*.dll;*.mod;*.mdl|Any type (*.*)|*.*";
         if (dialog.ShowDialog() == DialogResult.OK)
         {
            this.textBoxResultPath.Text = dialog.FileName;
         }
      }

      private void buttonToolPath_Click(object sender, EventArgs e)
      {
         OpenFileDialog dialog = new OpenFileDialog();
         dialog.CheckFileExists = true;
         dialog.CheckPathExists = true;
         dialog.Multiselect = false;
         dialog.Filter = "Executable Files (*.exe)|*.exe|Any type (*.*)|*.*";
         if (dialog.ShowDialog() == DialogResult.OK)
         {
            this.textBoxToolPath.Text = dialog.FileName;
         }
      }      
   }
}