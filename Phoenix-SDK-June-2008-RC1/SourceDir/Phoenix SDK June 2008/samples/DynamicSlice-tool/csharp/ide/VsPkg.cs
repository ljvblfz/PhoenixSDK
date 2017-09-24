using System;
using System.Diagnostics;
using System.Globalization;
using System.Runtime.InteropServices;
using System.ComponentModel.Design;
using System.Collections;
using System.Collections.Generic;
using Microsoft.Win32;

using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;
using Microsoft.VisualStudio.TextManager.Interop;
using Microsoft.VisualStudio.Debugger.Interop;
// using Microsoft.VisualStudio.VCProject;
// using Microsoft.VisualStudio.VCProjectEngine;
using EnvDTE;

using SliceAnalysis;

namespace SliceIDE
{
   // These attributes make the build system register this package with VS
   [PackageRegistration(UseManagedResourcesOnly = true)]
   [DefaultRegistryRoot("Software\\Microsoft\\VisualStudio\\9.0")]
   [InstalledProductRegistration(
      false, "#100", "#102", "1.0", IconResourceID = 400)]
   [ProvideLoadKey("Standard", "1", "SliceTool", "n/a", 106)]

   // This attribute tells the shell that this package exposes some menus
   [ProvideMenuResource(1000, 1)]

   // This attribute tells VS that this package provides SliceMarkerService
   [ProvideService(typeof(SliceMarkerService))]

   // This attribute makes the add-in load when a solution is loaded
   [ProvideAutoLoad(UIContextGuids.SolutionExists)]

   [Guid(GuidList.guidSliceVSIPPkgString)]
   public sealed class SlicePackage :
        Package, IVsDebuggerEvents, IEvaluator, IDebugEventCallback2
   {
      private SliceMarkerService sms;
      private PhoenixProvider phxProvider = null;

      private IVsDebugger debugger;
      private uint debuggerCookie;

      private MenuCommand cmdSelectSlice, cmdCtxSelectSlice;

      // keep a list of views we've marked so we can clear the marks later
      private List<IVsTextView> views = new List<IVsTextView>();

      // keep track of user-specified PE and PDB files
      private Dictionary<Document,string> binFiles = 
         new Dictionary<Document,string>();
      private Dictionary<Document, string> pdbFiles =
         new Dictionary<Document, string>();

      // used to keep track of where we are stopped in the program
      private uint currentFunctionOffset = 0;
      private string currentFunctionName = null;
      private IDebugThread2 currentDebugThread = null;


      // Initializes the package when VS loads it
      protected override void Initialize()
      {
         base.Initialize();

         // make sure the marker entires are present in the registry
         MarkerRegistryCheck();

         // Add command handlers for menu
         // (commands must exist in the .ctc file)
         OleMenuCommandService mcs =
            GetService(typeof(IMenuCommandService)) as OleMenuCommandService;         

         // Create the commands for the menu items.

         // the toolbar command
         CommandID menuCommandID = new CommandID(
            GuidList.guidSliceVSIPCmdSet,
            (int)PkgCmdIDList.cmdidSelectSlice);
         cmdSelectSlice = new MenuCommand(
            new EventHandler(MenuItemCallback),
            menuCommandID);
         cmdSelectSlice.Enabled = false;
         mcs.AddCommand(cmdSelectSlice);

         // the context menu command
         menuCommandID = new CommandID(
            GuidList.guidSliceVSIPCmdSet,
            (int)PkgCmdIDList.cmdidCtxSelectSlice);
         cmdCtxSelectSlice = new MenuCommand(
            new EventHandler(MenuItemCallback),
            menuCommandID);
         cmdCtxSelectSlice.Visible = false;
         mcs.AddCommand(cmdCtxSelectSlice);

         // create the marker service
         // and tell VS we are providing it
         sms = new SliceMarkerService(this);
         ((IServiceContainer)this).AddService(
            typeof(SliceMarkerService), sms, true);

         // hook into the debugger to get mode change events
         debugger = (IVsDebugger)GetService(typeof(SVsShellDebugger));
         debugger.AdviseDebuggerEvents(this, out debuggerCookie);
         debugger.AdviseDebugEventCallback(this);
      }

      // This is the menu callback for the toolbar and context menu items
      private void MenuItemCallback(object sender, EventArgs e)
      {
         MenuCommand cmd = (MenuCommand)sender;
         if (cmd.CommandID.ID == PkgCmdIDList.cmdidSelectSlice ||
            cmd.CommandID.ID == PkgCmdIDList.cmdidCtxSelectSlice)
         {
            IVsTextManager tm =
               (IVsTextManager)GetService(typeof(SVsTextManager));

            // If toolbar button is unchecked then we are selecting a slice              
            // UNLESS the event is from the context menu
            // the context menu always selects a new slice
            if (cmdSelectSlice.Checked == false ||
               cmd.CommandID.ID == PkgCmdIDList.cmdidCtxSelectSlice)
            {
               // get the DTE object
               DTE dte = (DTE)GetService(typeof(SDTE));

               // see if we're in a disassembly window               
               bool assemblyMode =
                  (dte.ActiveWindow.ObjectKind.Equals(
                  "{CF577B8C-4134-11D2-83E5-00C04F9902C1}"));

               // get the VSIP objects we need
               IVsTextView vw;
               IVsTextLines bf;
               tm.GetActiveView(0, null, out vw);
               vw.GetBuffer(out bf);

               // there are checks that need to be done only for unmanaged code
               
               // take different actions depending on what mode we're in
               try {
                 if (assemblyMode)
                    AssemblySlice(dte, tm, vw, bf);
                 else
                    SourceSlice(dte, tm, vw, bf);
               }
               catch (Exception) {
                 System.Windows.Forms.MessageBox.Show(
                        "The slicing tool failed. If your project is a C++ project," +
                        "Edit and Continue must be disabled and profiling enabled." +
                        "Please fix these in your project properties.",
                        "Slicing Tool",
                        System.Windows.Forms.MessageBoxButtons.OK,
                        System.Windows.Forms.MessageBoxIcon.Error);
               }      
            }
            else
            {
               // the button is checked
               // clear the marks and uncheck the button
               ClearMarks(tm);
               cmdSelectSlice.Checked = false;
            }
         }

      }

      // checks the registry for the marker registry keys
      private void MarkerRegistryCheck()
      {
         RegistryKey markers = Registry.LocalMachine;
         String regkey = this.ApplicationRegistryRoot.Name;
         regkey = regkey.Substring(
            regkey.IndexOf("\\") + 1) + "\\Text Editor\\External Markers";
         markers = markers.OpenSubKey(regkey, false);

         RegistryKey markerKey =
            markers.OpenSubKey(
            "{" + GuidList.guidSliceVSIPMarkerString + "}");

         if (markerKey != null)
            return;

         // the markers were not found


#if DEBUG
         // we now need readwrite access to the registry:
         try
         {

             markers = markers.OpenSubKey(regkey, true);
         }
         catch (System.Security.SecurityException)
         {
             System.Windows.Forms.MessageBox.Show(
                 "The slice markers were not installed in this registry hive.\n" +
                 "Please run VS as administrator to register these markers.\n" +
                 "This should only occur if you did not install Dynamic Slice using the installer.",
                 "Dynamic Slice Tool");
             return;
         }

         // add the markers to the registry
         // this should usually be done from the installer
         RegistryKey marker = markers.CreateSubKey(
            "{" + GuidList.guidSliceVSIPMarkerString + "}", 
            RegistryKeyPermissionCheck.ReadWriteSubTree);
         marker.SetValue(
            "DisplayName", 
            "Slice Marker", 
            RegistryValueKind.String);
         marker.SetValue(
            null, 
            "Slice Marker", 
            RegistryValueKind.String);
         marker.SetValue(
            "Package", 
            "{" + GuidList.guidSliceVSIPPkgString + "}", 
            RegistryValueKind.String);
         marker.SetValue(
            "Service", 
            "{" + GuidList.guidSliceVSIPMarkerServiceString + "}", 
            RegistryValueKind.String);

         markerKey = 
            markers.OpenSubKey("{" + GuidList.guidSliceVSIPVMarkerString + "}");
         if (markerKey != null)
            return;

         marker = markers.CreateSubKey(
            "{" + GuidList.guidSliceVSIPVMarkerString + "}", 
            RegistryKeyPermissionCheck.ReadWriteSubTree);
         marker.SetValue(
            "DisplayName", 
            "Slice Variable Marker", 
            RegistryValueKind.String);
         marker.SetValue(
            null, 
            "Slice Variable Marker", 
            RegistryValueKind.String);
         marker.SetValue(
            "Package", 
            "{" + GuidList.guidSliceVSIPPkgString + "}", 
            RegistryValueKind.String);
         marker.SetValue(
            "Service", 
            "{" + GuidList.guidSliceVSIPMarkerServiceString + "}", 
            RegistryValueKind.String);

         System.Windows.Forms.MessageBox.Show(
            "Visual Studio must be restarted before " + 
            "the slicing tool can be used.", 
            "Slicing Tool");
#else
         System.Windows.Forms.MessageBox.Show(
            "The Dyanmic Slicing Tool custom text markers were not found." +
            "Please reinstall the Dynamic Slicing Tool.", "Slicing Tool");
#endif
      }

      // Select a slice in a source code window
      private void SourceSlice(
         DTE dte, 
         IVsTextManager tm, 
         IVsTextView vw, 
         IVsTextLines bf)
      {
         // column numbers for the variable name
         int vstart, vend;

         SourceSliceSpec specification = new SourceSliceSpec();

         // Find the name of the selected variable
         specification.VarName = FindClosestVariable(vw, bf, out vstart, out vend);
         // Find the current line number in the source file
         specification.VarLine = (uint)FindCurrentLine(vw) + 1;

         // Fill in other info that slice needs
         try
         {
            FillSpec(dte,specification);
         }
         catch (PePdbNotFoundException)
         {
            return;
         }

         // make sure that the closest variable detected is really a variable
         if (specification.VarName.Length == 0 || !IsVariable(dte, specification.VarName))
         {
            System.Windows.Forms.MessageBox.Show(
               "The selected text is not a valid identifier " + 
               "in the current scope.",
               "Slicing Tool", 
               System.Windows.Forms.MessageBoxButtons.OK, 
               System.Windows.Forms.MessageBoxIcon.Error);
            return;
         }

         // we need to know the point where execution is currently stopped
         // we get this from the debugger code context
         UpdateCurrentFunctionOffset();
         specification.ExecOffset = currentFunctionOffset;         

         // SlicePackage is the evaluator for the slice
         specification.Evaluator = this;

         // prepare to compute the slice
         PreSlice(tm);

         // compute the slice
         SliceInfo si = null;
         try
         {
            si = phxProvider.Slice(specification);
         }
         catch (SliceException ex)
         {
            System.Windows.Forms.MessageBox.Show(
               "Failed to compute slice: " + ex.Message,
               "Slicing Tool",
               System.Windows.Forms.MessageBoxButtons.OK,
               System.Windows.Forms.MessageBoxIcon.Error);
            return;
         }

         // get all of the lines that should be marked
         List<uint> lines = si.DumpLine();

         // mark the lines
         foreach (uint i in lines)
            sms.MarkLine(tm, (int)i - 1);

         // mark the source variable
         sms.MarkVar(tm, (int)specification.VarLine - 1, vstart, vend);

         // actions to take when slice succeeds
         PostSlice(vw);
      }

      // Select a slice in a disassembly window
      private void AssemblySlice(
         DTE dte, IVsTextManager tm, IVsTextView vw, IVsTextLines bf)
      {
         AsmSliceSpec specification = new AsmSliceSpec();
         
         // Find the number of the selected operand
         specification.OpndIndex = FindOperandNumber(vw, bf);

         // Calling GetExpression causes the disassembly window to 
         // refresh the line numbers.
         // We force them to refresh now because otherwise it is possible
         // for the line numbers to change between now and when we
         // mark the lines.
         dte.Debugger.GetExpression("1", true, 100);

         // Find the current line number in the source file
         int currentLine = FindCurrentLine(vw);

         // Find the memory offset of the selected instruction
         // If this throws an exception it is because the function address
         // cannot be evaluated, implying that this is managed code         
         try
         {
            specification.InstrOffset = FindInsnOffsetByLine(dte, bf, currentLine);
         }
         catch (NoMemoryAddressException)
         {
            System.Windows.Forms.MessageBox.Show(
               "Could not find the memory address of the selected instruction. "
               + "Make sure that the memory address display is enabled and that"
               + " the current line is an assembly instruction.",
               "Slicing Tool",
               System.Windows.Forms.MessageBoxButtons.OK,
               System.Windows.Forms.MessageBoxIcon.Error);
            return;
         }
         catch (NoFunctionAddressException)
         {
            System.Windows.Forms.MessageBox.Show(
               "Slice tool cannot find the address of this function. Either your " +
               "project is managed code, or you have not enabled profiling and disabled Edit & Continue " +
               "in your project settings.",
               "Slicing Tool",
               System.Windows.Forms.MessageBoxButtons.OK,
               System.Windows.Forms.MessageBoxIcon.Error);
            return;
         }

         // Fill in other info that slice needs
         try
         {
            FillSpec(dte,specification);
         }
         catch (PePdbNotFoundException)
         {
            return;
         }

         // prepare to compute the slice
         PreSlice(tm);

         // compute the slice
         SliceInfo si = null;
         try
         {
            si = phxProvider.Slice(specification);
         }
         catch (SliceException ex)
         {
            System.Windows.Forms.MessageBox.Show(
               "Failed to compute slice: " + ex.Message,
               "Slicing Tool",
               System.Windows.Forms.MessageBoxButtons.OK,
               System.Windows.Forms.MessageBoxIcon.Error);
            return;
         }

         // get all of the instructions that should be marked
         List<uint> insns = si.DumpOffsets();

         // find starting address of current function
         uint funcAddress = 0;
         try
         {
            funcAddress = FindFuncAddress(dte);
         }
         catch (NoFunctionAddressException)
         {
            return;
         }
         uint lineAddress = 0;
         int line = currentLine;

         // mark the instructions
         // first mark lines above where we started
         do
         {
            lineAddress = MarkInsn(line,funcAddress,insns,bf,tm);

            // go to the previous line
            line--;
            // loop until we go above the function or hit beginning of buffer
         } while (lineAddress > funcAddress && line >= 0);

         // reset lineAddress if we go too far
         if (lineAddress < funcAddress)
            lineAddress = funcAddress;

         // some lines are not visible
         // remove them now
         for (int i = 0; i < insns.Count; i++)
         {
            if (insns[i] <= ((int)lineAddress - (int)funcAddress))
            {
               insns.RemoveAt(i);
               i--;
            }
         }

         // mark lines below where we started
         // loop until no insns are left
         line = currentLine + 1;
         while (insns.Count > 0)
         {
            MarkInsn(line,funcAddress,insns,bf,tm);

            // go to the next line
            line++;
         }

         // mark the operand
         MarkOperand(tm, bf, currentLine, specification.OpndIndex);

         // actions to take when slice succeeds
         PostSlice(vw);
      }

      // These methods provide functionality common to source and assembly
      #region Pre and post slice actions

      // helper method for marking instructions for assembly slice
      private uint MarkInsn(
         int line, 
         uint funcAddress, 
         List<uint> insns, 
         IVsTextLines bf,
         IVsTextManager tm)
      {
         int length = 0;
         string curLine = "";
         uint lineAddress = 0;

         bf.GetLengthOfLine(line, out length);
         bf.GetLineText(line, 0, line, length, out curLine);

         // try to get the address of the instruction at this line
         if (curLine.Length >= 8 && uint.TryParse(
            curLine.Substring(0, 8),
            NumberStyles.HexNumber, null, out lineAddress))
         {
            // if this line is a marked instruction
            if (insns.Contains(lineAddress - funcAddress))
            {
               // mark the line
               sms.MarkLine(tm, line);
               // remove the instruction from the list of insns
               insns.Remove(lineAddress - funcAddress);
            }
         }
         else
         {
            // this line is not an instruction

            // set lineAddress to be within this function
            // to stop the loop from exiting
            lineAddress = funcAddress + 1;
         }

         return lineAddress;
      }

      // clears existing markers and initializes Phoenix
      private void PreSlice(IVsTextManager tm)
      {
         // clear the existing markers if there are any
         ClearMarks(tm);

         // initialize Phoenix        
         if (phxProvider == null)
            phxProvider = new PhoenixProvider();
      }

      private void ClearMarks(IVsTextManager tm)
      {
         foreach (IVsTextView view in views)
            sms.ClearMarks(tm, view);
         views.Clear();
      }

      // checks the button because there is a slice selected
      private void PostSlice(IVsTextView vw)
      {
         cmdSelectSlice.Checked = true;
         views.Add(vw);
      }
      #endregion  

      // These methods gather data needed for computing the slice
      #region Find methods

      // fills in information common to source and assembly slice
      private void FillSpec(DTE dte, SliceSpec specification)
      {
         // Find the name of the current function
         specification.FuncName = dte.Debugger.CurrentStackFrame.FunctionName;
         // Find the filename of the current source file
         specification.SrcFile = dte.ActiveDocument.FullName;

         // Find the filenames for the PE file and PDB file
         string pdbFile = "";
         string peFile = "";
         if (!FindPdbFileAndBinFile(dte, out pdbFile, out peFile))
            throw new PePdbNotFoundException();
         specification.PdbFile = pdbFile;
         specification.PeFile = peFile;

         // find the argument types, for handling overloaded methods
         specification.ArgTypes = FindArgTypes(dte);
      }

      // Find the PE and PDB files for the program we're slicing on
      // returns true on success, false if the user cancels the process
      private bool FindPdbFileAndBinFile(
         DTE dte, out string pdbFile, out string binFile)
      {
         // Get the current project associated with the active document.
         Project proj = dte.ActiveDocument.ProjectItem.ContainingProject;

         if (proj == null || proj.ConfigurationManager == null)
         {
            // there is no project file
            // prompt the user for these files
            pdbFile = "";
            binFile = "";

            // if the user has already specified a PE+PDB for this program
            if (binFiles.ContainsKey(dte.ActiveDocument) && 
               pdbFiles.ContainsKey(dte.ActiveDocument))
            {
               binFile = binFiles[dte.ActiveDocument];
               pdbFile = pdbFiles[dte.ActiveDocument];
               return true;
            }

            System.Windows.Forms.DialogResult result = 
               System.Windows.Forms.MessageBox.Show(
               "DynamicSlice-tool cannot locate the PE and PDB files " + 
               "for this program. Would you like to locate them yourself?", 
               "Slicing Tool", 
               System.Windows.Forms.MessageBoxButtons.YesNo);
            if (result == System.Windows.Forms.DialogResult.No)
               return false;

            // prompt for PE file
            System.Windows.Forms.OpenFileDialog ofd =
               new System.Windows.Forms.OpenFileDialog();
            ofd.Title = "Find PE file...";
            ofd.Filter = "(*.exe;*.dll;*.scr)|*.exe;*.dll;*.scr";
            result = ofd.ShowDialog();
            if (result != System.Windows.Forms.DialogResult.OK)
               return false;
            binFile = ofd.FileName;
            binFiles.Add(dte.ActiveDocument, binFile);

            // prompt for PDB file
            ofd.Title = "Find PDB file...";
            ofd.Filter = "(*.pdb)|*.pdb";
            result = ofd.ShowDialog();
            if (result != System.Windows.Forms.DialogResult.OK)
               return false;
            pdbFile = ofd.FileName;
            pdbFiles.Add(dte.ActiveDocument, pdbFile);

            return true;
         }

         // Get the configuration object from the current project.
         Configuration conf = proj.ConfigurationManager.ActiveConfiguration;
         // Get the set of output groups for this configuration.
         OutputGroups groups = conf.OutputGroups;
         // Get the pdb file
         OutputGroup symGroup = groups.Item("Symbols");         
         pdbFile = GetFileNameFromOutputGroup(symGroup);

         // Get the PE file
         OutputGroup builtGroup = groups.Item("Built");
         binFile = GetFileNameFromOutputGroup(builtGroup);

         return true;
      }

      // helper method for getting filename from outputgroup
      private string GetFileNameFromOutputGroup(OutputGroup group)
      {
         if (group.FileCount == 0)
            return null;

         // we use the URL because it is an absolute path
         string name =
            (string)((object[])group.FileURLs).GetValue(0);
         // remove the "file://"
         return name.Substring(8);
      }

      // Finds the current line in the editor
      private int FindCurrentLine(IVsTextView vw)
      {
         int line, position;
         vw.GetCaretPos(out line, out position);
         return line;
      }

      // Finds the memory address of the current function
      private uint FindFuncAddress(DTE dte)
      {     
         // get the current function address
         Expression e;
         try
         {
            e = dte.Debugger.GetExpression(currentFunctionName, true, 100);
         }
         catch (COMException)
         {
            throw new NoFunctionAddressException();
         }
         // if this fails we're probably managed code
         if (!e.IsValidValue)
         {
            throw new NoFunctionAddressException();
         }
         return uint.Parse(e.Value.Substring(2, 8), NumberStyles.HexNumber);
      }

      // Finds the instruction offset of the current disassembly line
      private uint FindInsnOffsetByLine(DTE dte, IVsTextLines bf, int line)
      {
         // get the current function address
         uint funcAddress = FindFuncAddress(dte);

         // get the selected instruction's address
         int length = 0;
         string curLine = "";
         bf.GetLengthOfLine(line, out length);
         bf.GetLineText(line, 0, line, length, out curLine);
         uint lineAddress = 0;
         try
         {
            lineAddress = uint.Parse(
               curLine.Substring(0, 8), NumberStyles.HexNumber);
         }
         catch (ArgumentOutOfRangeException)
         {           
            throw new NoMemoryAddressException();
         }
         catch (FormatException)
         {
            throw new NoMemoryAddressException();
         }

         // return the offset
         return lineAddress - funcAddress;
      }

      // Used by FindClosestVariable() to see if this is a character 
      // that could be in a variable     
      private bool IsVarChar(char c, bool allowDot)
      {
         if (Char.IsLetterOrDigit(c))
            return true;
         if (c == '_')
            return true;
         if (allowDot && c == '.')
            return true;
         return false;
      }

      // Find the closest potential variable name
      // by looking left and right of the cursor
      private String FindClosestVariable(
         IVsTextView vw, 
         IVsTextLines bf, 
         out int start, 
         out int end)
      {
         int line, position, length;
         String text;

         // get the text of the current line and current cursor position
         vw.GetCaretPos(out line, out position);
         bf.GetLengthOfLine(line, out length);
         bf.GetLineText(line, 0, line, length, out text);

         // add characters from left until we hit a nonvariable character
         // then do the same to the right
         start = position;
         end = position;
         while (start > 0 && IsVarChar(text[start - 1], true))
            start--;
         while (end < length && IsVarChar(text[end], false))
            end++;

         // get the potential variable name
         String variable = text.Substring(start, end - start);

         return variable;
      }
      
      // Finds whether the first (0) or second (1) operand is selected
      private uint FindOperandNumber(IVsTextView vw, IVsTextLines bf)
      {
         int line, position, length;
         String text;

         // get the text of the current line and current cursor position
         vw.GetCaretPos(out line, out position);
         bf.GetLengthOfLine(line, out length);
         bf.GetLineText(line, 0, line, length, out text);

         // split the instruction by commas
         char[] s = { ',' };
         String[] tokens = text.Split(s);
         uint operand = 0;

         // see which operand our position is in
         foreach (String token in tokens)
         {
            if (position <= token.Length)
               return operand;
            position -= token.Length;
            operand++;
         }

         return operand - 1;
      }

      // create a list of argument types to pass to slice
      private List<string> FindArgTypes(DTE dte)
      {
         List<string> argumentTypes = new List<string>();
         Expressions arguments = dte.Debugger.CurrentStackFrame.Arguments;
         for (int i = 1; i <= arguments.Count; i++)
            argumentTypes.Add(arguments.Item(i).Type);
         return argumentTypes;
      }

      #endregion

      // checks if the given text is a valid variable name
      private bool IsVariable(DTE dte, String text)
      {
         // try evaluating it as an expression
         Expression expression;
         try
         {
            expression = dte.Debugger.GetExpression(text, true, 1000);
         }
         catch (COMException)
         {
            // if it has a syntax error, it's not a variable
            return false;
         }

         // if it can't be evaluated, it's not a variable
         if (!expression.IsValidValue)
            return false;

         // variables don't evaluate to themselves
         // this eliminates numeric constants
         if (expression.Value == text)
            return false;

         // therefore, it must be a variable
         return true;
      }

      // Finds and marks an operand
      private void MarkOperand(IVsTextManager tm, IVsTextLines bf,
         int currentLine, uint operandNumber)
      {
         int length, start, end;
         String text;
         bf.GetLengthOfLine(currentLine, out length);
         bf.GetLineText(currentLine, 0, currentLine, length, out text);

         if (operandNumber == 1)
         {
            // find columns for text after a comma
            start = text.IndexOf(',') + 1;
            end = text.TrimEnd().Length;
         }
         else
         {
            // find columns for text before a comma
            if (text.Contains(","))
               end = text.IndexOf(",");
            else
               end = text.TrimEnd().Length;
            start = end;
            while (!(text[start - 1] == ' ' && text[start - 2] == ' '))
               start--;
         }

         // mark the operand with a variable marker
         sms.MarkVar(tm, currentLine, start, end);
      }

      // from the IEvaluate interface
      // slice uses this to evaluate variables
      public int Evaluate(string variable)
      {
         DTE dte = (DTE)GetService(typeof(SDTE));
         Expression expression;
         try
         {
            expression = dte.Debugger.GetExpression(variable, true, 1000);
         }
         catch (COMException)
         {
            throw new EvaluatorException(variable);
         }
         if (!expression.IsValidValue)
            throw new EvaluatorException(variable);
         int result = 0;
         if (!int.TryParse(expression.Value, out result))
            throw new EvaluatorException(variable);
         return result;
      }

      // called whenever the debug mode changes, so we can update the UI
      // also to notify the slice DLL to deinitialize Phoenix
      public int OnModeChange(DBGMODE dbgmode)
      {
         // deinitialize Phoenix when we leave debug mode
         if (dbgmode == DBGMODE.DBGMODE_Design && phxProvider != null)
         {
            phxProvider.Deinitialize();
            phxProvider = null;
         }

         switch (dbgmode)
         {
            // disable the slice buttons and clear marks when not in break mode
            case DBGMODE.DBGMODE_Design:
            case DBGMODE.DBGMODE_Run:
               cmdSelectSlice.Enabled = false;
               cmdSelectSlice.Checked = false;

               cmdCtxSelectSlice.Visible = false;

               IVsTextManager tm = 
                  (IVsTextManager)GetService(typeof(SVsTextManager));
               ClearMarks(tm);
               break;

            // enable the slice buttons when in break mode
            case DBGMODE.DBGMODE_Break:
               cmdSelectSlice.Enabled = true;
               cmdCtxSelectSlice.Visible = true;
               break;
         }
         return 0;
      }

      // used to find where execution is currently stopped      
      private void UpdateCurrentFunctionOffset()
      {
         // we can't find the offset if we're not in a function
         if (currentDebugThread == null)
            return;         

         // get the code context info of the current thread

         IEnumDebugFrameInfo2 enumFif;
         currentDebugThread.EnumFrameInfo(
            (uint)enum_FRAMEINFO_FLAGS.FIF_FRAME, 10, out enumFif);
         if (enumFif == null)
            return;

         FRAMEINFO[] fif = new FRAMEINFO[1];
         uint fetched = 0;
         enumFif.Next(1, fif, ref fetched);

         IDebugStackFrame2 frame = fif[0].m_pFrame;
         if (frame == null)
            return;

         IDebugCodeContext2 ctxt;
         frame.GetCodeContext(out ctxt);
         if (ctxt == null)
            return;

         CONTEXT_INFO[] ctxtinfo = new CONTEXT_INFO[1];
         ctxt.GetInfo((uint)enum_CONTEXT_INFO_FIELDS.CIF_ALLFIELDS, ctxtinfo);

         // store full function name for later
         currentFunctionName = ctxtinfo[0].bstrFunction;
		 
		 int parenStart = currentFunctionName.IndexOf('(');
		 
		 if(parenStart != -1) 
		 {
			currentFunctionName = currentFunctionName.Substring(0, parenStart);
		 }

         // check to see if the address offset is set         
         string addressString = ctxtinfo[0].bstrAddressOffset;

         if (addressString != null)
         {
            currentFunctionOffset = uint.Parse(
               addressString.Substring(2), 
               NumberStyles.HexNumber, 
               null);
            return;
         }

         // if an offset wasn't given we can figure it out ourselves
         // from the address and funcaddress         
         DTE dte = (DTE)GetService(typeof(SDTE));
         if (dte.Debugger.CurrentStackFrame == null)
            return;
         uint address = uint.Parse(
            ctxtinfo[0].bstrAddress.Substring(2), 
            NumberStyles.HexNumber, 
            null);
         currentFunctionOffset = address - FindFuncAddress(dte);

         return;
      }

      // we used this AdviseDebugEventCallback to get the IDebugThread2
      // which is needed to find where execution is currently stopped
      public int Event(
         IDebugEngine2 pEngine,
         IDebugProcess2 pProcess,
         IDebugProgram2 pProgram,
         IDebugThread2 pThread,
         IDebugEvent2 pEvent,
         ref Guid riidEvent,
         uint dwAttrib)
      {
         if (pThread != null)
            currentDebugThread = pThread;

         UpdateCurrentFunctionOffset();

         return 0;
      }

   }

   public class PePdbNotFoundException : Exception { }
   public class NoMemoryAddressException : Exception { }
   public class NoFunctionAddressException : Exception { }
}
