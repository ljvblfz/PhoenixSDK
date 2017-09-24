using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;
using System.IO;
using System.Collections;

namespace PEExplorer
{
   /// <summary>
   /// Presentation layer for the application.
   /// </summary>
   public partial class Viewer : Form, IFileBrowser
   {
      // Processes PE modules.
      private Driver driver;

      // Used to display line numbers and offsets
      // next to the Disassmbly window.
      private int[] lineNumbers;
      private int[] msilOffsets;
      
      // The name of the source PE file.
      private string sourceFileName;
      
      // Text used in the title bar.
      private string originalTitle;

      // Used in the status strip to report the number 
      // of classes and methods.
      private uint classCount;
      private uint methodCount;

      // A mapping of objects (stored in the TreeNode.Tag property) to 
      // their corresponding TreeNode objects. This is used to 
      // help speed-up tree view insertion operations.
      Dictionary<object, TreeNode> reverseMapping;

      // State to raise function unit IR to.
      private Phx.FunctionUnitState raiseToState;

      // Items that are filtered from the symbol list view.
      private Dictionary<string, List<ListViewItem>> filteredItems;
         
      /// <summary>
      /// Constructs a new Form1 object.
      /// </summary>
      public Viewer()
      {
         InitializeComponent();

         // Initialize the call graph viewer.
         this.InitializeCallGraphView();

         // Remember title.
         this.originalTitle = this.Text;

         // Ensure that the rich text box and line number label use the 
         // same font.
         this.label1.Font = this.richTextBox1.Font;
                  
         // Set the initial function unit raise level to 'MIR'.
         this.SetFunctionUnitRaiseLevel(this.mIRToolStripMenuItem);
         
         // Initialize symbol list view filtering.
         this.filteredItems = new Dictionary<string, List<ListViewItem>>();
         this.PopulateSymbolFilter();
      }
                  
      delegate void ProcessManifestDelegate(Phx.Manifest manifest);
      
      /// <summary>
      /// Adds manifest information to the tree view.
      /// </summary>
      private void AddAssemblyManifest(Phx.Manifest manifest)
      {
         // Call this method on the owning thread if called 
         // from a worker thread.
         if (this.InvokeRequired)
         {
            this.Invoke(new ProcessManifestDelegate(this.AddAssemblyManifest),
               new object[] { manifest });
            return;
         }
               
         if (manifest != null)
         {
            // Add assembly version information to the 
            // manifest text box.            
            Phx.Version version = manifest.Version;
            
            textBoxManifest.Text = string.Format(
               ".assembly {1}{0}{{{0}  .ver {2}:{3}:{4}:{5}{0}}}",
               Environment.NewLine, manifest.Name, 
               version.MajorVersion, version.MinorVersion,
               version.BuildNumber, version.RevisionNumber);
         }
         else
         {
            textBoxManifest.Text = string.Empty;
         }

         // Add an entry for the manifest to the tree view.
         if (this.treeView1.Nodes.Count > 0)
         {
            ElementNode parentNode = this.treeView1.Nodes[0].Tag as ElementNode;
            ElementNode newNode = new ElementNode(null, "M A N I F E S T", 
               string.Empty, ElementType.Manifest);

            parentNode.Children.Add(newNode);
            this.AddElementNode(newNode, parentNode);
         }
      }
      
      delegate void ProcessElementNodeDelegate(ElementNode newNode, ElementNode parentNode);

      /// <summary>
      /// Adds a new node to the tree view.
      /// </summary>
      private void AddElementNode(ElementNode newNode, ElementNode parentNode)
      {
         // Exit early if the background thread worker is cancelling.
         if (this.backgroundWorker1.CancellationPending)
         {
            return;
         }

         // Call this method on the owning thread if called 
         // from a worker thread.
         if (this.InvokeRequired)
         {
            this.Invoke(new ProcessElementNodeDelegate(AddElementNode), 
               new object[] { newNode, parentNode });            
            return;
         }

         TreeNode newTreeNode = null;
         
         if (newNode == null)
         {
            return;
         }

         // If the parent node is null, then this is the first
         // member of the tree view.
         if (parentNode == null)
         {
            Debug.Assert(this.treeView1.Nodes.Count == 0);
            newTreeNode = this.treeView1.Nodes.Add(newNode.Name); 
         }
         // Otherwise, insert the new node into the tree view.
         else
         {         
            TreeNode insertNode = null; // TreeNode to insert into.
            
            // Get the TreeNode whose Tag property matches
            // the provided parent ElementNode.
            insertNode = this.reverseMapping[parentNode];
            Debug.Assert(insertNode != null);

            try
            {
                // Add it to the tree view.
                newTreeNode = this.InsertTreeNode(insertNode, newNode);
            }
            catch (ArgumentException)
            {
                return;
            }
         }

         // Set additional properties on the new TreeNode object.
         newTreeNode.Name = newTreeNode.Text;
         newTreeNode.Tag = newNode;         
         newTreeNode.ImageIndex = (int)newNode.ElementType;
         newTreeNode.SelectedImageIndex = newTreeNode.ImageIndex;
         if (newNode.ElementType == ElementType.Manifest)
         {
            newTreeNode.ForeColor = Color.OliveDrab;
         }
         
         // Add the TreeNode and its tag to the lookup map.
         this.reverseMapping[newNode] = newTreeNode;

         // Add any children of the node to the tree view.
         foreach (ElementNode childElementNode in newNode.Children)
         {
            this.AddElementNode(childElementNode, newNode);
         }

         // Increment the class/method counter (these values are 
         // used for display purposes in the UI).
         switch (newNode.ElementType)
         {
            case ElementType.Interface:
            case ElementType.GenericInterface:
            case ElementType.Class:
            case ElementType.GenericClass:
            case ElementType.ValueType:
            case ElementType.GenericValueType:
            case ElementType.Enum:
            case ElementType.GenericEnum:
               this.classCount++;
               break;

            case ElementType.InstanceMethod:
            case ElementType.GenericInstanceMethod:
            case ElementType.NativeMethod:
            case ElementType.StaticMethod:
            case ElementType.GenericStaticMethod:
               this.methodCount++;
               break;
         }
      }

      /// <summary>
      /// Gets the order in which the given ElementType should be inserted
      /// into the tree view.
      /// </summary>
      private static int GetTreeNodeOrder(ElementType elementType)
      {
         // An item with a lower order number is inserted above
         // an item with a higher order number.
         switch (elementType)
         {
         case ElementType.Manifest:
            return 0;
         case ElementType.Info:
            return 1;
         case ElementType.Assembly:
            return 2;
         case ElementType.Attribute:
            return 3;
         case ElementType.Override:
            return 4;
         case ElementType.Namespace:
            return 5;
         case ElementType.Interface:
         case ElementType.GenericInterface:
            return 6;
         case ElementType.Class:
         case ElementType.GenericClass:
            return 7;
         case ElementType.ValueType:
         case ElementType.GenericValueType:
            return 8;
         case ElementType.Enum:
         case ElementType.GenericEnum:
            return 9;
         case ElementType.Global:
            return 10;
         case ElementType.Field:
            return 11;
         case ElementType.StaticField:
            return 12;
         case ElementType.NativeMethod:
            return 13;
         case ElementType.InstanceMethod:
         case ElementType.GenericInstanceMethod:
            return 14;
         case ElementType.StaticMethod:
         case ElementType.GenericStaticMethod:
            return 15;
         case ElementType.Event:
            return 16;
         case ElementType.Property:
            return 17;         
         }
         
         return int.MaxValue;
      }
      
      /// <summary>
      /// Inserts a new tree node with the given ElementNode tag
      /// into the tree view under the provided parent node.
      /// </summary>
      private TreeNode InsertTreeNode(TreeNode parentNode, ElementNode elementNode)
      {
         TreeNode treeNode = null;

         TreeNodeCollection nodes = parentNode == null ? 
            treeView1.Nodes[0].Nodes : 
            parentNode.Nodes;
         
         // If the element type of the node is a type, ensure that the tree
         // contains any associated namespace elements.

         if (elementNode.Symbol != null &&
             elementNode.Symbol.IsTypeSymbol)
         {
            // Check for namespace or nested classes.
            string[] tokens = elementNode.Name.Split(new char[] { '.' });
            
            if (tokens.Length >= 2)
            {
               // If a namespace or nested class declaration exists,
               // begin insertion under the root node.
               parentNode = this.treeView1.Nodes[0];
               
               // Create entries for parent tokens if needed.
               // For example, "System.IO.Path" is split into three entries:
               //  * System
               //  * System.IO
               //  * System.IO.Path
               for (int i = 0; i < tokens.Length - 1; i++)
               {
                  string token = tokens[0];
                  for (int j = 1; j <= i; j++)
                  {
                     token = string.Format("{0}.{1}", token, tokens[j]);
                  }                  

                  // Check whether the TreeNode for this token already
                  // exists.
                  TreeNode[] namespaceNodes = parentNode.Nodes.Find(token, false);
                  Debug.Assert(namespaceNodes.Length < 2);
                  
                  // If the token exists, update the parent node and
                  // continue.                  
                  if (namespaceNodes.Length == 1)
                  {
                     parentNode = namespaceNodes[0];
                  }
                  // Otherwise, insert a new tree node for the parent
                  // before continuing.
                  else
                  {
                     ElementNode newNode = 
                        new ElementNode(null, token, string.Empty, ElementType.Namespace);

                     // Set additional properties on the new TreeNode object.
                     parentNode = this.InsertTreeNode(parentNode, newNode);
                     parentNode.Name = parentNode.Text;
                     parentNode.Tag = newNode;
                     parentNode.ImageIndex = (int)newNode.ElementType;
                     parentNode.SelectedImageIndex = parentNode.ImageIndex;

                     // Add the TreeNode and its tag to the lookup map.
                     this.reverseMapping[newNode] = parentNode;
                  }
               
                  nodes = parentNode.Nodes;
               }
            }
         }

         // Limit the tree view to contain a single reference to a type.
         if (parentNode.Nodes.ContainsKey(elementNode.Name))
         {
             throw new ArgumentException();
         }
         
         // Insert the TreeNode into the tree view. We do this by
         // obtaining the numeric 'order' which ranks how high in the 
         // tree we should insert the node. Note that this also allows
         // us to group similar node types together.
         
         int elementOrder = GetTreeNodeOrder(elementNode.ElementType);

         nodes = parentNode == null ? 
            treeView1.Nodes[0].Nodes : 
            parentNode.Nodes;

         foreach (TreeNode childNode in nodes)
         {  
            // Get the order of the current child.
            int order;
            try
            {
               order = GetTreeNodeOrder((childNode.Tag as ElementNode).ElementType);
            }
            catch (NullReferenceException)
            {
               order = int.MinValue;
            }
            
            // If we've landed in the same group, insert the node now.
            if (elementOrder == order)
            {
               // Insert 'Info' nodes as we see them (not alphabetically).
               if (elementNode.ElementType == ElementType.Info)
               {
                  treeNode = nodes.Add(elementNode.Name);
                  break;
               }
               
               // Insert the node alphabetically. We do this by 
               // traversing the nodes until we find the proper insertion 
               // point or find the end of the group.
               int index = childNode.Index;
               while (index < nodes.Count)
               {
                  try
                  {
                     order = GetTreeNodeOrder((nodes[index].Tag as ElementNode).ElementType);
                  }
                  catch (NullReferenceException)
                  {
                     order = int.MinValue;
                  }

                  if (elementOrder != order)
                  {
                     index = index--;
                     break;
                  }
                  else
                  {
                     if (elementNode.Name.CompareTo(nodes[index].Text) < 0)
                     {
                        break;
                     }
                  }
                  index++;
               }

               if (index < nodes.Count)
                  treeNode = nodes.Insert(index, elementNode.Name);
                  
               break;
            }            
            else if (elementOrder <= order)
            {
               treeNode = nodes.Insert(childNode.Index, elementNode.Name);
               break;
            }
         }

         // If we hit the end of the tree, append the node now.
         if (treeNode == null)
         {
            treeNode = nodes.Add(elementNode.Name);
         }
         
         return treeNode;
      }

      /// <summary>
      /// Opens and reads the contents of a PE file.
      /// </summary>
      private void OpenFile()
      {
         this.UseWaitCursor = true;

         // Create a new map of Tag -> TreeNode objects.
         this.reverseMapping = new Dictionary<object, TreeNode>();

         // Create a new Driver object.
         this.driver = new Driver(this);

         // Set the form title.
         this.Text = string.Format("{0} - {1}", 
            this.sourceFileName, this.originalTitle);

         // Reset the number of parsed classes and methods.
         this.classCount = 0;
         this.methodCount = 0;

         // Enable the progress bar.
         this.toolStripStatusLabel1.Text = 
            string.Format("Processing {0}...", this.sourceFileName);
         this.toolStripProgressBar1.Visible = true;
         this.toolStripProgressBar1.Enabled = true;

         // Disable the UI.
         this.menuStrip1.Enabled = false;
         this.panel1.Enabled = false;

         // To keep the UI responsive, open and read the PE file
         // on a worker thread.
         this.backgroundWorker1.RunWorkerAsync();

         // Wait for the BackgroundWorker object to finish.
         while (this.backgroundWorker1.IsBusy)
         {
            // Keep UI messages moving, so the form remains 
            // responsive during the asynchronous operation.
            Application.DoEvents();
            
            System.Threading.Thread.Sleep(100);

            // If the user closed the form during processing,
            // inform the Driver object to cancel.
            if (this.backgroundWorker1.CancellationPending)
            {
               this.driver.CancellationPending = true;
               break;
            }
         }

         // If the Driver cancelled processing, exit now.
         if (this.driver == null || this.driver.CancellationPending)
         {
            return;
         }

         // Update status label.
         this.toolStripStatusLabel1.Text = string.Format(
            "Done. Found {0} types and {1} methods", this.classCount, this.methodCount);

         // Disable the progress bar.
         this.toolStripProgressBar1.Visible = false;
         this.toolStripProgressBar1.Enabled = false;

         // Enable the UI.
         this.menuStrip1.Enabled = true;
         this.panel1.Enabled = true;

         // Expand the tree view by one level.
         if (this.treeView1.Nodes.Count > 0)
         {
            this.treeView1.Nodes[0].Expand();
         }
         this.treeView1.Select();

         // Enable the 'Dump' and 'Dump TreeView' menu items.
         this.dumpToolStripMenuItem.Enabled = true;
         this.dumpTreeViewToolStripMenuItem.Enabled = true;

         this.UseWaitCursor = false;
      }

      /// <summary>
      /// Opens and reads a PE file on a background thread.
      /// </summary>
      private void backgroundWorker1_DoWork(object sender, DoWorkEventArgs eventArgs)
      {
         try
         {
            // Process the PE file.
            // This will generate a hierarchy of ElementNode objects.
            ElementNode rootNode = this.driver.ProcessModule(sourceFileName);

            // Update status bar.
            this.SetStatusText("Generating tree view...");
            
            // Populate the tree view with the ElementNode tree and 
            // manifest information.
            this.AddElementNode(rootNode, null);
            this.AddAssemblyManifest(this.driver.ModuleUnit.Manifest);
         }
         // Catch errors thrown by the framework.
         catch (Phx.FatalError fatalError)
         {
            MessageBox.Show(fatalError.MessageString, "Error", 
               MessageBoxButtons.OK, MessageBoxIcon.Error);
         }
         // Catch errors thrown by the application.
         catch (Exception e)
         {
            MessageBox.Show(string.Format("Unexpected error: {0}", e.Message), "Error", 
               MessageBoxButtons.OK, MessageBoxIcon.Error);
         }
      }

      /// <summary>
      ///  Closes the current PE file.
      /// </summary>            
      private void CloseFile()
      {
         // Invalidate the current source file name.
         this.sourceFileName = string.Empty;
         
         // Reset title text.
         this.Text = this.originalTitle;

         // Clear UI text.
         this.textBoxManifest.Clear();
         this.listView1.Items.Clear();
         this.treeView1.Nodes.Clear();
         this.richTextBox1.Clear();
         this.label1.Text = string.Empty;

         // Disable the 'Dump' and 'Dump TreeView' menu items.
         this.dumpToolStripMenuItem.Enabled = false;
         this.dumpTreeViewToolStripMenuItem.Enabled = false;

         // Close the current ModuleUnit and invalidate our Driver object.
         if (this.driver != null)
         {
            this.driver.CloseModuleUnit();
            this.driver = null;
         }
      }
      
      /// <summary>
      /// Called when the Disassmbly text box is scrolled vertically.
      /// </summary>
      private void richTextBox1_VScroll(object sender, EventArgs e)      
      {
         // Synchronize the contents of the text box with the 
         // label that contains the line and offset information.
      
         // Adjust the label for the number of pixels moved by the scrollbar.

         int n = this.richTextBox1.GetPositionFromCharIndex(0).Y % 
            this.richTextBox1.Font.Height;

         this.label1.Location = new Point(0, n);
         if (n < 0)
         {
            this.label1.Height = this.label1.Parent.Height - n;
         }
         else         
         {
            this.label1.Height = this.label1.Parent.Height;
         }

         this.UpdateNumberLabel();
      }

      /// <summary>
      /// Called when the Disassmbly text box is resized.
      /// </summary>
      private void richTextBox1_Resize(object sender, EventArgs e)
      {
         // Synchronize the contents of the text box with the 
         // label that contains the line and offset information.
         
         // Adjust the label for the number of pixels moved by the scrollbar.

         int n = this.richTextBox1.GetPositionFromCharIndex(0).Y %
            this.richTextBox1.Font.Height;

         this.label1.Location = new Point(0, n);
         if (n < 0)
         {
            this.label1.Height = this.label1.Parent.Height - n;
         }
         else
         {
            this.label1.Height = this.label1.Parent.Height;
         }

         this.UpdateNumberLabel();
      }
      
      /// <summary>
      /// Updates the label that contains line and offset information 
      /// </summary>
      private void UpdateNumberLabel()
      {
         // Get the index of the first visible character and the
         // number of the first visible line.
         Point pos = new Point(0, 0);
         int firstIndex = this.richTextBox1.GetCharIndexFromPosition(pos);
         int firstLine = this.richTextBox1.GetLineFromCharIndex(firstIndex);

         // Get the index of the last visible character and the
         // number of the last visible line.
         pos.X = this.ClientRectangle.Width;
         pos.Y = this.ClientRectangle.Height;
         int lastIndex = this.richTextBox1.GetCharIndexFromPosition(pos);
         int lastLine = this.richTextBox1.GetLineFromCharIndex(lastIndex);

         // Use the Y value of the last visible character to calculate
         // the label size.
         pos = this.richTextBox1.GetPositionFromCharIndex(lastIndex);

         // Update the contents of the label.
         StringBuilder stringBuilder = new StringBuilder();
         if (this.lineNumbers != null &&
             this.msilOffsets != null)
         {
            for (int i = firstLine; i <= lastLine; i++)
            {               
               if (i >= 0 && i < this.lineNumbers.Length)
               {
                  int lineNumber = this.lineNumbers[i];
                  int msilOffset = this.msilOffsets[i];
                  
                  // Append a blank line if the current line of text
                  // has no associated line number.
                  if (lineNumber < 0)
                  {
                     stringBuilder.AppendLine();
                  }
                  else
                  {
                     // Append just the line number if the current line
                     // has no associated offset.
                     if (msilOffset < 0)
                     {
                        stringBuilder.AppendLine(lineNumber.ToString());
                     }
                     else
                     {
                        // Append both the line number and the offset 
                        // if the current line has both.
                        stringBuilder.AppendLine(string.Format("{0},{1}",
                           lineNumber, msilOffset));
                     }
                  }
               }
               else
               {
                  stringBuilder.AppendLine();
               }
            }
         }
         this.label1.Text = stringBuilder.ToString();
      }

      /// <summary>
      /// Occurs before the tree node is selected. 
      /// </summary>
      private void treeView1_BeforeSelect(object sender, TreeViewCancelEventArgs e)
      {
         // If a tree node is currently selected, delete its disassembly
         // information before selecting the new tree node.
         
         if (this.treeView1.SelectedNode != null)
         {
            ElementNode elementNode = this.treeView1.SelectedNode.Tag as ElementNode;
            if (elementNode != null)
            {
               this.driver.EvictDisassembly(elementNode);
            }
         }
      }     
      
      /// <summary>
      /// Occurs after the tree node is selected. 
      /// </summary>
      private void treeView1_AfterSelect(object sender, TreeViewEventArgs e)
      {
         // Fill the Disassmbly and Symbols windows with the ElementNode
         // object associated with the selected TreeNode.
         
         if (e.Node.Tag != null)
         {
            Debug.Assert(e.Node.Tag is ElementNode);
            ElementNode elementNode = e.Node.Tag as ElementNode;

            this.UpdateTabControl(elementNode);            
         }
      }

      /// <summary>
      /// Updates the selected tab of the main tab control.
      /// </summary>
      private void UpdateTabControl(ElementNode elementNode)
      {
         // We call FillDisassemblyWindow even if the 
         // Disassembly window is not the active tab because 
         // this method builds function unit information that may
         // be needed by the other views.
         this.FillDisassemblyWindow(elementNode);
         
         if (this.tabControl1.SelectedTab == this.tabPageSymbols)
         {
            this.FillSymbolsWindow(elementNode);
         }
         else if (this.tabControl1.SelectedTab == this.tabPageCallGraph)
         {
            this.FillCallGraphWindow(elementNode);
         }
      }

      /// <summary>
      /// Occurs when a tab is selected. 
      /// </summary>
      private void tabControl1_Selected(object sender, TabControlEventArgs e)
      {
         TreeNode selected = this.treeView1.SelectedNode;
         if (selected != null)
         {
            Debug.Assert(selected.Tag is ElementNode);
            ElementNode elementNode = selected.Tag as ElementNode;
            this.UpdateTabControl(elementNode);
         }
      }
      
      /// <summary>
      /// Populates the Call Graph window with symbol information 
      /// that is associated with the given ElementNode object.
      /// </summary>
      private void FillCallGraphWindow(ElementNode elementNode)
      {         
         Phx.Symbols.Symbol symbol = elementNode.Symbol;
         Phx.Graphs.CallGraph callGraph = this.driver.ModuleUnit.CallGraph;

         // Use the default (blank) graph if no call graph is available
         // or the symbol is not a function.
         if (callGraph == null ||
             symbol == null || !symbol.IsFunctionSymbol)
         {
            this.callGraphViewer.Graph = this.defaultGraph;
            return;
         }
         
         Phx.Symbols.FunctionSymbol functionSymbol = 
            symbol.AsFunctionSymbol;
         
         // Create a graph object.
         Microsoft.Glee.Drawing.Graph graph =
            new Microsoft.Glee.Drawing.Graph("graph");
            
         // Expand the aspect ratio to fill the entire panel.
         graph.GraphAttr.AspectRatio = (double)this.panel4.Width /
            (double)this.panel4.Height;
            
         // Set the background color.
         graph.GraphAttr.Backgroundcolor = 
            this.defaultGraph.GraphAttr.Backgroundcolor;

         // Clear the previous call relationship mapping.
         this.callRelationships.Clear();

         // Fill the graph.
         Microsoft.Glee.Drawing.Node node =
            this.ExpandCallNode(graph, 
               functionSymbol.CallNode, this.callGraphDepth);
         // Use a distinct shape and fill color for the 'central' node.
         if (node != null)
         {            
            node.Attr.Shape = Microsoft.Glee.Drawing.Shape.House;
            node.Attr.Fillcolor = Microsoft.Glee.Drawing.Color.MistyRose;
         }
                           
         // Bind the graph to the viewer.
         this.callGraphViewer.Graph = graph;                  
      }

      /// <summary>
      /// Adds caller/callee relationships to the given graph 
      /// for the provided call node and call depth.
      /// </summary>
      private Microsoft.Glee.Drawing.Node ExpandCallNode(
         Microsoft.Glee.Drawing.Graph graph,
         Phx.Graphs.CallNode callNode, int depth)
      {
         if (depth <= 0 || callNode == null)
         {
            return null;
         }

         // Get display name for the node.
         string displayName = Driver.GetFullName(callNode.FunctionSymbol);
         
         // Add the node to the graph. We normally do not need to explicity
         // add the nodes (this is automatically done by Graph.AddEdge). 
         // However, for the case where there are no callers/callees 
         // associated with the node, ensure that at least the central
         // node is shown.
         Microsoft.Glee.Drawing.Node node = 
            graph.AddNode(displayName);
          
         // GLEE requires at least one edge to be associated with each node.
         // Add an edge from the node to itself.
         Microsoft.Glee.Drawing.Edge selfEdge = graph.AddEdge(
            displayName, displayName);
         // Make the edge transparent because it is a false edge.
         selfEdge.EdgeAttr.Color = 
            Microsoft.Glee.Drawing.Color.Transparent;
         // Associate the call node with the graph node.
         node.UserData = callNode;
         
         // Add call predecessors and successors to the graph.
         this.AddCallPredecessors(graph, callNode, depth);
         this.AddCallSuccessors(graph, callNode, depth);

         return node;
      }
      
      /// <summary>
      /// Determines whether a caller/callee relationship has been
      /// added to the relationship map.
      /// </summary>
      private bool CallRelationshipExists(Phx.Graphs.CallNode caller,
         Phx.Graphs.CallNode callee)
      {
         List<Phx.Graphs.CallNode> calleeList;
         if (this.callRelationships.TryGetValue(caller, 
            out calleeList))
         {           
            return calleeList.Contains(callee);
         }
         return false;
      }
      
      /// <summary>
      /// Adds a new caller/callee relationship to the relationship map.
      /// </summary>
      private void AddCallRelationship(Phx.Graphs.CallNode caller,
         Phx.Graphs.CallNode callee)
      {
         // Add new list to the map if needed.
         if (! this.callRelationships.ContainsKey(caller))
         {
            this.callRelationships.Add(caller,
               new List<Phx.Graphs.CallNode>());
         }

         // Add the entry to the list.

         List<Phx.Graphs.CallNode> calleeList = 
            this.callRelationships[caller];
         
         Debug.Assert(!calleeList.Contains(callee));
         calleeList.Add(callee);
      }

      /// <summary>
      /// Connects all callers to the function symbol associated with the given
      /// call node within the provided Graph object.
      /// </summary>
      private void AddCallPredecessors(Microsoft.Glee.Drawing.Graph graph,
         Phx.Graphs.CallNode callNode, int depth)
      {
         // Get the function symbol that is associated with the call node.
         Phx.Symbols.FunctionSymbol callSymbol = callNode.FunctionSymbol;
         // Create a label name for the node.
         string callName = Driver.GetFullName(callSymbol);
         
         // Create a connection from each caller to the function
         // in the graph.
         for (uint i = 0; i < callNode.PredecessorCount; ++i)
         {
            Phx.Graphs.Node node = callNode.NthPredecessorNode(i);
            if (node.IsCallNode)
            {
               // Get the CallNode and FunctionSymbol objects that are
               // associated with the caller node.
               Phx.Graphs.CallNode callerCallNode = node.AsCallNode;
               Phx.Symbols.FunctionSymbol callerCallSymbol =
                  callerCallNode.FunctionSymbol;

               // Generate node name.
               string callerName = Driver.GetFullName(callerCallSymbol);
               
               // Create a new association if the relationship does
               // not already exist. This folds multiple calls from one method
               // to another into a single graph edge.
               if (!this.CallRelationshipExists(callerCallNode, callNode))
               {
                  // Add edge to graph.                  
                  Microsoft.Glee.Drawing.Edge edge = this.AddGraphEdge(
                     graph, callerName, callName, callerCallNode, callNode);

                  // Mark all caller->call associations with a blue edge.
                  edge.EdgeAttr.Color = Microsoft.Glee.Drawing.Color.RoyalBlue;

                  // Add relationship to mapping.
                  this.AddCallRelationship(callerCallNode, callNode);
               }

               // Recurse.
               if (depth > 1)
               {
                  this.AddCallPredecessors(graph, callerCallNode, depth - 1);
               }
            }
         }         
      }

      /// <summary>
      /// Connects all callees from the function symbol associated with the 
      /// given call node within the provided Graph object.
      /// </summary>
      private void AddCallSuccessors(Microsoft.Glee.Drawing.Graph graph, 
         Phx.Graphs.CallNode callNode, int depth)
      {
         // Get the function symbol that is associated with the call node.
         Phx.Symbols.FunctionSymbol callSymbol = callNode.FunctionSymbol;
         // Create a label name for the node.
         string callName = Driver.GetFullName(callSymbol);

         // Create a connection from the function to each callee 
         // in the graph.
         for (uint i = 0; i < callNode.SuccessorCount; ++i)
         {
            Phx.Graphs.Node node = callNode.NthSuccessorNode(i);
            if (node.IsCallNode)
            {
               // Get the CallNode and FunctionSymbol objects associated
               // with the callee node.
               Phx.Graphs.CallNode calleeCallNode = node.AsCallNode;
               Phx.Symbols.FunctionSymbol calleeCallSymbol =
                  calleeCallNode.FunctionSymbol;

               // Generate node name.
               string calleeName = Driver.GetFullName(calleeCallSymbol);
               
               // Create a new association if the relationship does
               // not already exist.
               if (!this.CallRelationshipExists(callNode, calleeCallNode))
               {                  
                  // Add edge to graph.                  
                  Microsoft.Glee.Drawing.Edge edge = this.AddGraphEdge(
                     graph, callName, calleeName, callNode, calleeCallNode);

                  // Mark all call->callee associations with a red edge.
                  edge.EdgeAttr.Color = Microsoft.Glee.Drawing.Color.Red;

                  // Add relationship to mapping.
                  this.AddCallRelationship(callNode, calleeCallNode);
               }

               // Recurse.
               if (depth > 1)
               {
                  this.AddCallSuccessors(graph, calleeCallNode, depth - 1);
               }
            }
         }
      }

      /// <summary>
      /// Adds a new edge to the call graph.
      /// </summary>
      private Microsoft.Glee.Drawing.Edge AddGraphEdge(
         Microsoft.Glee.Drawing.Graph graph,
         string source, string destination,
         Phx.Graphs.CallNode sourceNode, Phx.Graphs.CallNode destinationNode)
      {
         // Add edge to graph.
         Microsoft.Glee.Drawing.Edge edge = 
            graph.AddEdge(source, destination);
            
         // For both nodes associated with the edge, associate each 
         // node with a call graph node and set the node's fill color
         // if the node is not the 'central' node.
         
         Microsoft.Glee.Drawing.Node node;

         node = graph.FindNode(destination);
         // Associate the call node with the graph node.
         node.UserData = destinationNode;
         // Set fill color.
         if (node.Attr.Shape != Microsoft.Glee.Drawing.Shape.House)
            node.Attr.Fillcolor = Microsoft.Glee.Drawing.Color.LightGreen;

         node = graph.FindNode(source);
         // Associate the call node with the graph node.
         node.UserData = sourceNode;
         // Set fill color.
         if (node.Attr.Shape != Microsoft.Glee.Drawing.Shape.House)
            node.Attr.Fillcolor = Microsoft.Glee.Drawing.Color.LightGreen;
            
         return edge;
      }

      // Call graph viewer.
      private Microsoft.Glee.GraphViewerGdi.GViewer callGraphViewer;

      // Default graph for when a non-function symbol is selected.
      private Microsoft.Glee.Drawing.Graph defaultGraph;
      
      // Stores caller-callee relationships.
      private Dictionary<Phx.Graphs.CallNode, 
         List<Phx.Graphs.CallNode>> callRelationships;

      // The depth to recurse call graph information.
      private int callGraphDepth;

      /// <summary>
      /// Initializes the control to view call graphs.
      /// </summary>
      private void InitializeCallGraphView()
      {
         // Create a viewer object.
         this.callGraphViewer = new Microsoft.Glee.GraphViewerGdi.GViewer();

         // Set initial call depth.
         this.callDepthMenuItem_Clicked(this.toolStripMenuItem4, null);

         // Create the default graph object.
         this.defaultGraph = new Microsoft.Glee.Drawing.Graph("graph");
         this.defaultGraph.GraphAttr.Backgroundcolor = 
            Microsoft.Glee.Drawing.Color.Beige;
         this.callGraphViewer.Graph = this.defaultGraph;
            
         // Associate the viewer with the form.
         this.panel4.SuspendLayout();
         this.callGraphViewer.Dock = DockStyle.Fill;
         this.callGraphViewer.SelectionChanged += new EventHandler(callGraphViewer_SelectionChanged);
         this.callGraphViewer.MouseClick += new MouseEventHandler(callGraphViewer_MouseClick);
         this.panel4.Controls.Add(this.callGraphViewer);
         this.panel4.ResumeLayout();
         
         // Create caller-callee relationship mapping.
         this.callRelationships = new Dictionary<
            Phx.Graphs.CallNode, List<Phx.Graphs.CallNode>>();
      }

      /// <summary>
      /// Handles Call Graph -> Call Depth drop down menu.
      /// </summary>
      private void callDepthMenuItem_Clicked(object sender, EventArgs e)
      {
         // Remember previous value.
         int previousDepth = this.callGraphDepth;
   
         // Uncheck all items and check the selected one.
         foreach (ToolStripMenuItem item 
            in callDepthToolStripMenuItem.DropDownItems)
         {
            item.Checked = false;
         }
         
         ToolStripMenuItem menuItem = sender as ToolStripMenuItem;
         menuItem.Checked = true;
         
         // Convert the text value of the string to an integer, 
         // falling back to a safe value on error.
         if (!int.TryParse(menuItem.Text, out this.callGraphDepth))
         {
            this.callGraphDepth = 1;
         }

         // If the depth value actually changed, force the
         // UI to update.
         if (this.callGraphDepth != previousDepth)
         {            
            TreeNode selected = this.treeView1.SelectedNode;
            this.treeView1.SelectedNode = null;
            this.treeView1.SelectedNode = selected;
         }
      }

      /// <summary>
      /// Called when the graph selection has changed.
      /// </summary>
      void callGraphViewer_SelectionChanged(object sender, EventArgs e)
      {
         // If the selected object is a node, show the signature 
         // of the associated function in the text box.
         // Otherwise, clear the text box.
            
         string text = string.Empty;
         
         object selected = this.callGraphViewer.SelectedObject;         
         if (selected is Microsoft.Glee.Drawing.Node)
         {
            Microsoft.Glee.Drawing.Node node =
               selected as Microsoft.Glee.Drawing.Node;
            text = this.driver.GetMethodSignature(
               (node.UserData as Phx.Graphs.CallNode).FunctionSymbol);
         }
         
         this.textBoxCallGraph.Text = text;         
      }

      /// <summary>
      /// Called when the mouse is clicked over the graph control.
      /// </summary>
      void callGraphViewer_MouseClick(object sender, MouseEventArgs e)
      {
         if (this.callGraphViewer.SelectedObject
            is Microsoft.Glee.Drawing.Node)
         {
            // If the left mouse button was clicked over a node, 
            // show the full call graph for that node.
            if (e.Button == MouseButtons.Left)
            {
               // Set the control's graph to null so we can force
               // the UI to refresh.
               Microsoft.Glee.Drawing.Graph graph = this.callGraphViewer.Graph;
               this.callGraphViewer.Graph = null;
               
               Microsoft.Glee.Drawing.Node node = 
                  this.callGraphViewer.SelectedObject as Microsoft.Glee.Drawing.Node;
                  
               // Update the graph.
               this.ExpandCallNode(graph, 
                  node.UserData as Phx.Graphs.CallNode, this.callGraphDepth);

               // Because we added a new 'central' node, update its 
               // shape and fill color.
               node.Attr.Shape = Microsoft.Glee.Drawing.Shape.House;
               node.Attr.Fillcolor = Microsoft.Glee.Drawing.Color.MistyRose;
               
               // Set the graph.
               this.callGraphViewer.Graph = graph;
            }
            // If the left mouse button was clicked over a node, 
            // show the context menu.
            else if (e.Button == MouseButtons.Right)
            {
               this.contextMenuStrip1.Show(
                  this.callGraphViewer, e.Location);
            }
         }
      }

      /// <summary>
      /// Handles the 'Sync Tree' menu item.
      /// </summary>
      private void syncTreeToolStripMenuItem_Click(object sender, EventArgs e)
      {
         if (this.callGraphViewer.SelectedObject
            is Microsoft.Glee.Drawing.Node)
         {
            Microsoft.Glee.Drawing.Node node = 
               this.callGraphViewer.SelectedObject as Microsoft.Glee.Drawing.Node;

            Phx.Symbols.Symbol symbol = 
               ((node.UserData) as Phx.Graphs.CallNode).FunctionSymbol;

            TreeNode treeNode = this.FindTreeNode(symbol);
            
            if (treeNode == null)
            {
               MessageBox.Show(string.Format("Could not find tree node for '{0}'",
                  symbol.NameString), "Error", MessageBoxButtons.OK,
                  MessageBoxIcon.Information);
            }
            else
            {
               this.treeView1.SelectedNode = treeNode;
            }
         }
      }

      /// <summary>
      /// Searches the tree view for the node associated with the 
      /// given function symbol.
      /// </summary>
      /// <returns></returns>
      private TreeNode FindTreeNode(Phx.Symbols.Symbol symbol)
      {
         foreach (KeyValuePair<object, TreeNode> kvp in this.reverseMapping)
         {
            if (kvp.Key is ElementNode)
            {
               if ((kvp.Key as ElementNode).Symbol == symbol)
               {
                  return kvp.Value;
               }
            }
         }
         return null;
      }
      
      /// <summary>
      /// Populates the Symbols window with symbol information 
      /// associated with the given ElementNode.
      /// </summary>
      private void FillSymbolsWindow(ElementNode node)
      {
         // Clear the current view.
         this.listView1.Items.Clear();
         
         // Clear the previous group filter.
         this.filteredItems.Clear();
         
         Phx.Symbols.Symbol symbol;

         // Add the symbol that is associated with each child node
         // to the list view.
         foreach (ElementNode childNode in node.Children)
         {
            symbol = childNode.Symbol;
            if (symbol != null)
            {
               int imageIndex = (int)childNode.ElementType;
               
               // If the item is filtered, add it to the filtered list.
               if (this.IsFiltered(symbol.SymbolKind))
               {
                  ListViewItem item = new ListViewItem();
                  item.Tag = symbol;
                  item.ImageIndex = imageIndex;
                  
                  string key =
                     (item.Tag as Phx.Symbols.Symbol).SymbolKind.ToString();
                  if (!this.filteredItems.ContainsKey(key))
                  {
                     this.filteredItems.Add(key, new List<ListViewItem>());
                  }
                  this.filteredItems[key].Add(item);
               }
               else
               {
                  // Add the item to the list view.
                  this.AddListViewItem(symbol, imageIndex);
               }
            }
         }

         // The TreeView does not capture information for symbols such as 
         // local variables and parameters. If the symbol associated with 
         // the element is a function symbol, get the symbols that are
         // associated with its FunctionUnit and add them to the list view.
         symbol = node.Symbol;
         if (symbol != null && symbol.IsFunctionSymbol)
         {
            Phx.Symbols.Symbol[] localSymbols = 
               this.driver.GetFunctionSymbols(symbol.AsFunctionSymbol);
            foreach (Phx.Symbols.Symbol localVariableSymbol
               in localSymbols)
            {
               this.AddListViewItem(localVariableSymbol, (int)ElementType.Local);
            }
         }

         this.SortListView(this.listView1);         
      }
                  
      /// <summary>
      /// Sorts the groups by using the ListViewGroupSorter class.
      /// </summary>      
      private void SortListView(ListView listView)
      {
         // Copy the groups for the column to an array.
         ListViewGroup[] groupsArray =
            new ListViewGroup[listView.Groups.Count];
         listView.Groups.CopyTo(groupsArray, 0);

         // Sort the groups.
         Array.Sort(groupsArray,
            new ListViewGroupSorter(listView.Sorting));

         // Remove and re-add the groups.
         listView.Groups.Clear();
         listView.Groups.AddRange(groupsArray);       
      }

      /// <summary>
      /// Adds an entry to the list view by using the provided symbol and 
      /// image index.
      /// </summary>
      private ListViewItem AddListViewItem(Phx.Symbols.Symbol symbol, int imageIndex)
      {
         // Get (or add) the group that is associated with the symbol's 
         // SymbolKind property.
         string key = symbol.SymbolKind.ToString();
         ListViewGroup group = this.listView1.Groups[key];
         if (group == null)
         {
            string headerText = string.Concat(key, " symbols");
            group = this.listView1.Groups.Add(key, headerText);
         }   
         
         // Add the list view item.               
         ListViewItem item = listView1.Items.Add(symbol.NameString, imageIndex);
         // Associate it with a group.
         item.Group = group;
         // Tag it with the symbol.        
         item.Tag = symbol;

         // Fill subitems.

         // Rva
         if (symbol.Rva == 0)
         {
            item.SubItems.Add(symbol.Rva.ToString());
         }
         else
         {
            item.SubItems.Add(string.Format("0x{0}", symbol.Rva.ToString("x8")));
         }                  

         // Section
         if (symbol.AllocationBaseSectionSymbol != null)
         {
            item.SubItems.Add(symbol.AllocationBaseSectionSymbol.NameString);
         }
         else
         {
            item.SubItems.Add(string.Empty);
         }

         // ExternId
         if (symbol.ExternId > 0x9)
         {
            item.SubItems.Add(string.Format("{0} (0x{1})", 
               symbol.ExternId, symbol.ExternId.ToString("x8")));
         }
         else
         {
            item.SubItems.Add(symbol.ExternId.ToString());
         }

         // LocalId
         if (symbol.LocalId > 0x9)
         {
            item.SubItems.Add(string.Format("{0} (0x{1})",
               symbol.LocalId, symbol.LocalId.ToString("x8")));
         }
         else
         {
            item.SubItems.Add(symbol.LocalId.ToString());
         }

         // Type
         if (symbol.IsGlobalDefinition)
         {
            item.SubItems.Add("Global definition");
         }
         else if (symbol.IsDefinition)
         {
            item.SubItems.Add("Definition");
         }
         else if (symbol.IsGlobalReference)
         {
            item.SubItems.Add("Global reference");
         }
         else if (symbol.IsReference)
         {
            item.SubItems.Add("Reference");
         }
         else
         {
            item.SubItems.Add(string.Empty);
         }

         return item;
      }

      /// <summary>
      /// Sorts ListViewGroup objects by header value. 
      /// </summary>
      private class ListViewGroupSorter : IComparer
      {
         private SortOrder order;

         /// <summary>
         /// Stores the sort order.
         /// </summary>
         public ListViewGroupSorter(SortOrder theOrder)
         {
            order = theOrder;
         }


         /// <summary>
         /// Compares the groups by header value, using the saved sort
         /// order to return the correct value.
         /// </summary>
         public int Compare(object x, object y)
         {
            int result = String.Compare(
                ((ListViewGroup)x).Header,
                ((ListViewGroup)y).Header
            );
            if (order == SortOrder.Ascending)
            {
               return result;
            }
            else
            {
               return -result;
            }
         }
      }

      /// <summary>
      /// Populates the Disassembly window with "disassembly" information 
      /// associated with the given ElementNode.
      /// </summary>
      private void FillDisassemblyWindow(ElementNode elementNode)
      {
         // Clear the current view.
         this.richTextBox1.Clear();

         // Build the diassembly for the node if it does not 
         // already exist.
         if (elementNode.Disassembly == null)
         {
            this.driver.BuildDisassembly(elementNode, this.raiseToState);
         }

         if (elementNode.Disassembly != null)
         {
            // Update the contents of the text box.
            this.identifierTypeMap = elementNode.Disassembly.IdentifierTypeMap;
            this.UpdateDisassmblyWindow(elementNode.Disassembly.Text);
            
            // Update the size (width) of the label that contains
            // line and offset information. 
            // The width of the label is based on the longest string
            // it needs to display.
            
            this.lineNumbers = elementNode.Disassembly.LineNumbers;
            this.msilOffsets = elementNode.Disassembly.MsilOffsets;

            string longest = string.Empty;
            for (int i = 0; i < this.lineNumbers.Length; i++)
            {
               int lineNumber = this.lineNumbers[i];
               int msilOffset = this.msilOffsets[i];

               string test = string.Format("{0},{1}",
                  lineNumber, msilOffset);

               if (test.Length > longest.Length)
                  longest = test;
            }

            Size requiredSize = TextRenderer.MeasureText(
               this.label1.CreateGraphics() as IDeviceContext,
               longest, this.label1.Font, this.label1.Size,
               TextFormatFlags.SingleLine
            );

            this.label1.Parent.Width = requiredSize.Width;
            this.label1.Width = requiredSize.Width;

            this.UpdateNumberLabel();
         } 
      }

      // The default text color in the Disassembly window.
      private static readonly Color NormalColor = Color.Black;
      // Keyword text color in the Disassembly window.
      private static readonly Color KeywordColor = Color.Blue;
      // Text color in the Disassembly window for keywords that can
      // also be used as identifiers.
      private static readonly Color DualKeywordColor = Color.OrangeRed;
      // Comment text color in the Disassembly window.
      private static readonly Color CommentColor = Color.Green;
      
      // Whitespace characters, used to find keywords and comments
      // in the disassembly text.
      private char[] whitespaceCharacters = null;
      
      // Additional word delimiter characters.
      private static readonly char[] wordDelimiters = {
         '(', ')', '[', ']', '<', '>', '*', '$', '^', '&', ',', ':', ';', '@'
      };

      /// <summary>
      /// Determines whether the given character is a word
      /// delimiter.
      /// </summary>
      private static bool IsWordDelimiter(char ch)
      {
         foreach (char wordDelimiter in wordDelimiters)
         {
            if (ch == wordDelimiter)
               return true;
         }
         return false;
      }

      /// <summary>
      /// Fills the disassembly window with the given text.
      /// </summary>
      private void UpdateDisassmblyWindow(string text)
      {
         // Populate the whitespace characters array on first use.
         if (this.whitespaceCharacters == null)
         {
            List<char> chars = new List<char>();
            
            // Although they are not strictly whitespace characters, 
            // the following will help us delimit words.
            chars.AddRange(wordDelimiters);
            
            for (char ch = Char.MinValue; ch < Char.MaxValue; ch++)
            {
               if (Char.IsWhiteSpace(ch))
               {
                  chars.Add(ch);
               }
            }
            this.whitespaceCharacters = chars.ToArray();
         }
         
         // Tokenize the input string into individual words. 
         // Highlight keywords and comments using the appropriate
         // colors.

         int startPosition = 0;

         bool inCComment = false; // tracks C-style comments.
         bool inCppComment = false; // tracks C++-style comments.
         
         // Look for initial comment string.
         if (text.Length > 2)
         {
            if (text[0] == '/' && text[1] == '/')
            {
               inCppComment = true;
            }
            else if (text[0] == '/' && text[1] == '*')
            {
               inCComment = true;
            }
         }

         // Process string tokens.
         int index = text.IndexOfAny(this.whitespaceCharacters, startPosition);
         while (index >= 0)
         {                       
            string token = text.Substring(startPosition, index - startPosition);

            // Look for comments.
            if (token.StartsWith("//"))
            {
               inCppComment = true;
            }
            else if (token.StartsWith("/*"))
            {
               inCComment = true;
            }

            // Write text to text box.
            this.AppendRichText(inCppComment, inCComment, token);

            // Look for comments and comment terminators.
            if (index < text.Length - 1)
            {
               // Look ahead for C++-comment.
               if (text[index + 1] == '/' && text[index + 2] == '/')
               {
                  inCppComment = true;
               }
               // Look ahead for C-comment.
               else if (text[index + 1] == '/' && text[index + 2] == '*')
               {
                  inCComment = true;
               }
               // Look ahead for end of C-comment.
               else if (text[index + 1] == '*' && text[index + 2] == '/')
               {
                  // Append the end of comment characters now so that they are
                  // formatted with the correct color.
                  this.AppendRichText(inCppComment, inCComment, text.Substring(index, 3));
                  inCComment = false;
                  // Advance character position.
                  index += 3;                  
               }              
               else if (text[index] == '\r' || text[index] == '\n')
               {
                  // Newline characters terminate a C++ comment.
                  inCppComment = false;
               }
            }
               
            // Ensure that we're using the normal text color if we're not
            // in a comment.
            if (!inCppComment && !inCComment)
               this.richTextBox1.SelectionColor = NormalColor;

            if (text[index] == '/')
            {
               index++;
            }
            else
            {
               // Eat remaining 'whitespace' characters until we find 
               // the start of the next token.
               while (index < text.Length &&
                  (Char.IsWhiteSpace(text[index]) ||
                   IsWordDelimiter(text[index])))
               {
                  if (text[index] == '\r' || text[index] == '\n')
                  {
                     // Newline characters terminate a C++ comment.
                     inCppComment = false;
                  }

                  if (text[index] != '\n')
                     this.richTextBox1.AppendText(text[index].ToString());
                  index++;
               }
            }
            
            startPosition = index;
            index = text.IndexOfAny(this.whitespaceCharacters, startPosition);            
         } 

         // There is no more whitespace remaining; append the remaining token.         
         if (startPosition < text.Length)
         {
            string token = text.Substring(startPosition);
            this.AppendRichText(inCppComment, inCComment, token);            
         }
      }

      /// <summary>
      /// Appends the provided string to the rich text box.
      /// </summary>
      private void AppendRichText(bool inCppComment, bool inCComment, string text)
      {
         if (text.Length == 0)
         {
            return;
         }

         // Change the selection back color if the text string
         // is in the identifier map.
         string identifier = text;
         if (Driver.IsQuotedText(text))
         {
            identifier = Driver.UnquoteText(text);
         }
         Color selectionBackColor = this.richTextBox1.SelectionBackColor;
         if (this.identifierTypeMap != null &&
             this.identifierTypeMap.ContainsKey(identifier))
         {
            this.richTextBox1.SelectionBackColor = this.variableColor;
         }

         // Select the appropriate text color.
         if (inCppComment || inCComment)
         {
            this.richTextBox1.SelectionColor = CommentColor;
         }
         else if (Keywords.IsKeyword(text))
         {
            this.richTextBox1.SelectionColor = KeywordColor;
         }
         else if (Keywords.IsDualKeyword(text))
         {
            this.richTextBox1.SelectionColor = DualKeywordColor;
         }

         // Append the string to the rich text box.
         this.richTextBox1.AppendText(text);
         
         // Restore previous selection back color.
         this.richTextBox1.SelectionBackColor = selectionBackColor;
      }
      
      // Maps identifiers to their respective types.
      private Dictionary<string, string> identifierTypeMap = null;

      /// <summary>
      /// Updates the tool tip within the rich text box.
      /// </summary>
      private void UpdateToolTip(Point cursorPosition)
      {
         Debug.Assert(this.identifierTypeMap != null);

         // Remember cursor position.
         this.richTextBoxCursorPosition = cursorPosition;
         
         // Get the position of the character that is closest to the cursor.
         int charPosition = 
            this.richTextBox1.GetCharIndexFromPosition(cursorPosition);
         if (charPosition < 0)
         {
            this.toolTip1.SetToolTip(this.richTextBox1, string.Empty);
            return;
         }

         // Because GetCharIndexFromPosition returns the closest character to
         // the cursor, make sure that the curor is actually near the
         // character.
         Point actualPosition = 
            this.richTextBox1.GetPositionFromCharIndex(charPosition);
         int eps = 15;
         if (Math.Abs(actualPosition.X - cursorPosition.X) > eps ||
             Math.Abs(actualPosition.Y - cursorPosition.Y) > eps)
         {
            this.toolTip1.SetToolTip(this.richTextBox1, string.Empty);
            return;
         }

         // Find the start of the word.
         string text = this.richTextBox1.Text;
         int start;
         for (start = charPosition; start > 0; --start)
         {
            char ch = text[start];
            // Allow letters, digits, and a few other characters.
            if (ch != '_' && ch != '\"' && ch != '.' && ch != '-' &&
               !Char.IsLetterOrDigit(ch))
            {
               break;
            }
         }
         ++start;

         // Find the end of the word.
         int end;
         for (end = charPosition; end < text.Length; ++end)
         {
            char ch = text[end];
            // Allow letters, digits, and a few other characters.
            if (ch != '_' && ch != '\"' && ch != '.' && ch != '-' &&
               !Char.IsLetterOrDigit(ch))
            {
               break;
            }
         }        

         // If the start and end positions are valid, check whether the 
         // identifier map contains an entry for the word.
         if (start <= end &&
             start < text.Length && end <= text.Length)
         {
            text = text.Substring(start, end - start);

            if (this.identifierTypeMap.ContainsKey(text))
            {
               this.toolTip1.SetToolTip(this.richTextBox1, 
                  this.identifierTypeMap[text]);
               return;
            }
         }

         // Clear tool tip text.
         this.toolTip1.SetToolTip(this.richTextBox1, string.Empty);
      }

      // Holds the previous cursor position within the rich text box.
      private Point richTextBoxCursorPosition;

      /// <summary>
      /// Called when the mouse moves within the rich text box.
      /// </summary>
      private void richTextBox1_MouseMove(object sender, MouseEventArgs e)
      {
         // Update the tool tip text only if the identifier type map 
         // is valid and the cursor position has actually changed.
         if (this.identifierTypeMap != null)
         {
            Point cursorPosition = 
               this.richTextBox1.PointToClient(Cursor.Position);
            if (cursorPosition != this.richTextBoxCursorPosition)
            {
               this.UpdateToolTip(cursorPosition);
            }
         }
      }

      /// <summary>
      /// Displays the About box.
      /// </summary>
      private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
      {
         // Show the About box.
         AboutBox aboutBox = new AboutBox();
         aboutBox.ShowDialog();

         // If the user selected a loaded assembly to view,
         // close the current file and open the selected assembly.
         if (aboutBox.SelectedAssemblyCodeBase.Length > 0)
         {
            this.CloseFile();
            this.sourceFileName = aboutBox.SelectedAssemblyCodeBase;
            this.OpenFile();
         }
      }
            
      /// <summary>
      /// Handles the 'Open' menu item.
      /// </summary>
      private void openToolStripMenuItem_Click(object sender, EventArgs e)
      {
         this.UseWaitCursor = true;

         // Allow the user to browse to a PE file on disk.
         OpenFileDialog dialog = new OpenFileDialog();
         dialog.CheckFileExists = true;
         dialog.CheckPathExists = true;
         dialog.Filter = "PE Files (*.exe;*.dll;*.mod;*.mdl)|*.exe;*.dll;*.mod;*.mdl|Any type (*.*)|*.*";
         dialog.Multiselect = false;
         
         // If the user clicks OK, close the current file and 
         // open the new one.
         if (dialog.ShowDialog() == DialogResult.OK)
         {
            this.CloseFile();
            this.sourceFileName = dialog.FileName;
            this.OpenFile();
         }

         this.UseWaitCursor = false;
      }

      /// <summary>
      /// Describes what information to dump to file.
      /// </summary>
      private enum DumpMode
      {
         Full, // dump full disassembly text
         TreeView // dump just the tree view 
      }
      
      // The current dump mode.      
      private DumpMode dumpMode;

      /// <summary>
      /// Dumps the loaded assembly to file by using a background thread.
      /// </summary>      
      private void DoDumpWorker(DumpMode dumpMode)
      {
         // Allow the user to choose a file name to write to.
         SaveFileDialog dialog = new SaveFileDialog();
         dialog.CheckFileExists = false;
         dialog.CheckPathExists = true;
         dialog.DefaultExt = ".txt";
         dialog.Filter = "Text File (*.txt)|*.txt|Any type (*.*)|*.*";
         dialog.AddExtension = false;
         // Set the initial directory to the same directory as the 
         // source binary.
         dialog.InitialDirectory =
            Path.GetDirectoryName(Path.GetFullPath(this.sourceFileName));
         // Set default file name to the original binary file with
         // the default extension.
         dialog.FileName = Path.ChangeExtension(
            Path.GetFileName(this.sourceFileName), dialog.DefaultExt);

         if (dialog.ShowDialog() == DialogResult.OK)
         {
            // Disable the UI.
            this.UseWaitCursor = true;
            this.menuStrip1.Enabled = false;
            this.panel1.Enabled = false;
            
            // Enable the progress bar.
            this.toolStripProgressBar1.Visible = true;
            this.toolStripProgressBar1.Enabled = true;
            
            try
            {
               // Set the dump mode.
               this.dumpMode = dumpMode;

               // To keep the UI responsive, write the file 
               // on a worker thread.
               this.backgroundWorker2.RunWorkerAsync(dialog.FileName);

               // Wait for the BackgroundWorker object to finish.
               while (this.backgroundWorker2.IsBusy)
               {
                  // Keep UI messages moving, so the form remains 
                  // responsive during the asynchronous operation.
                  Application.DoEvents();
               }
            }
            catch (Exception)
            {
               MessageBox.Show("An error has occurred while writing to {0}. ",
                  "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            
            // Enable the UI.
            this.UseWaitCursor = false;
            this.menuStrip1.Enabled = true;
            this.panel1.Enabled = true;
            this.toolStripStatusLabel1.Text = "Ready";

            // Disable the progress bar.
            this.toolStripProgressBar1.Visible = false;
            this.toolStripProgressBar1.Enabled = false;
         }
      }
            
      /// <summary>
      /// Handles the 'Dump' menu item.
      /// </summary>
      private void dumpToolStripMenuItem_Click(object sender, EventArgs e)
      {
         this.DoDumpWorker(DumpMode.Full);
      }

      /// <summary>
      /// Handles the 'Dump TreeView' menu item.
      /// </summary>
      private void dumpTreeViewToolStripMenuItem_Click(object sender, EventArgs e)
      {
         this.DoDumpWorker(DumpMode.TreeView);
      }
      
      /// <summary>
      /// Dumps the assembly on a background thread.
      /// </summary>
      private void backgroundWorker2_DoWork(object sender, DoWorkEventArgs e)
      {
         switch (this.dumpMode)
         {
            case DumpMode.Full:
               this.Dump(e.Argument as string);
               break;
            case DumpMode.TreeView:
               this.DumpTreeView(e.Argument as string);
               break;
         }
      }

      private delegate void SetTextDelegate(string text);

      /// <summary>
      /// Sets the status strip text.
      /// </summary>
      private void SetStatusText(string text)
      {
         // Call this method on the owning thread if called 
         // from a worker thread.
         if (this.InvokeRequired)
         {
            this.Invoke(new SetTextDelegate(SetStatusText), 
               new object[] { text });            
            return;
         }
         
         this.toolStripStatusLabel1.Text = text;
      }
      
      /// <summary>
      /// Dumps the tree view to the specified file.
      /// </summary>
      private void DumpTreeView(string fileName)
      {
         using (StreamWriter writer = new StreamWriter(fileName))
         {
            // Recursively write the text of each node in the tree view.

            this.DumpTreeViewAux(writer, treeView1.Nodes, 0);
         }
      }

      /// <summary>
      /// Recursively writes the provided tree view nodes to the specified file.
      /// </summary>
      private void DumpTreeViewAux(StreamWriter writer, TreeNodeCollection nodes, int level)
      {
         foreach (TreeNode node in nodes)
         {
            // Exit if the background worker requests cancellation.
            if (this.backgroundWorker2.CancellationPending)
            {
               return;
            }

            // Update status bar.
            this.SetStatusText(
               string.Format("Writing {0}...", node.Text));

            // Write indent.
            for (int i = 0; i < level; i++)
            {
               writer.Write("   |");
            }
            
            // Write node.
            
            writer.WriteLine("{0} {1}",
               this.GetTreeViewDumpPrefix(node), node.Text);

            if (node.Nodes.Count > 0)
            {
               // Write children.

               this.DumpTreeViewAux(writer, node.Nodes, level + 1);

               // Write indent.
               for (int i = 0; i < level; i++)
               {
                  writer.Write("   |");
               }
               writer.WriteLine();
            }
         }
      }

      /// <summary>
      /// Gets a string that describes type of the provided TreeNode.
      /// </summary>
      private string GetTreeViewDumpPrefix(TreeNode node)
      {
         ElementNode elementNode = node.Tag as ElementNode;
         
         switch (elementNode.ElementType)
         {
            case ElementType.Blank:
            case ElementType.Bad:
            case ElementType.Attribute:
            case ElementType.Override:
            case ElementType.Info:
            case ElementType.Manifest:
            default:
               if (node.Text.StartsWith("."))
                  return "    "; // 4 spaces
               return "     "; // 5 spaces

            case ElementType.Field:
               return "___[FLD]";
            case ElementType.StaticField:
               return "___[STF]";
            case ElementType.Global:
               return "___[GLB]";   

            case ElementType.Interface:
            case ElementType.GenericInterface:
               return "___[INT]";
            case ElementType.Class:
            case ElementType.GenericClass:
               return "___[CLS]";
            case ElementType.ValueType:
            case ElementType.GenericValueType:
               return "___[VCL]";
            case ElementType.Enum:
            case ElementType.GenericEnum:
               return "___[ENU]";
               
            case ElementType.InstanceMethod:
            case ElementType.GenericInstanceMethod:
               return "___[MET]";
            case ElementType.NativeMethod:
               return "___[NVM]";
            case ElementType.StaticMethod:
            case ElementType.GenericStaticMethod:
               return "___[STM]";
               
            case ElementType.Property:
               return "___[PTY]";
               
            case ElementType.Event:
               return "___[EVT]";
               
            case ElementType.Namespace:
               return "___[NSP]";
               
            case ElementType.Assembly:
               return "___[MOD]";
               
            case ElementType.Local:
               return "___[LOC]";
         }
      }

      /// <summary>
      /// Dumps the assembly to the specified file.
      /// </summary>
      private void Dump(string fileName)
      {         
         using (StreamWriter writer = new StreamWriter(fileName)) 
         {
            // Write copyright banner.
            writer.WriteLine("//  {0} {1}.  Version {2}",
               AboutBox.AssemblyCompany, AboutBox.AssemblyProduct,
               AboutBox.AssemblyVersion);
            writer.WriteLine("//  {0}.  All rights reserved.{1}{1}",
               AboutBox.AssemblyCopyright, Environment.NewLine);

            // Recursively write the contents of each node in the tree view.

            this.DumpAux(writer, treeView1.Nodes);
         }
      }
                  
      /// <summary>
      /// Recursively writes the provided tree view nodes to the specified file.
      /// </summary>
      private void DumpAux(StreamWriter writer, TreeNodeCollection nodes)
      {
         foreach (TreeNode node in nodes)
         {
            // Exit if the background worker requests cancellation.
            if (this.backgroundWorker2.CancellationPending)
            {
               return;
            }

            // Update status bar.            
            this.SetStatusText(
               string.Format("Writing {0}...", node.Text));
        
            // Write node.

            ElementNode elementNode = node.Tag as ElementNode;
            if (elementNode != null)
            {
               bool builtMethodDisassmbly = false;
               
               // Build the disassembly for the element node,
               // if needed.
               if (elementNode.Disassembly == null)
               {
                  this.driver.BuildDisassembly(elementNode, this.raiseToState);
                  builtMethodDisassmbly = true;
               }

               // Write the disassembly string associated with this node.
               if (elementNode.Disassembly != null)
               {
                  writer.WriteLine(elementNode.Disassembly.Text);

                  // To conserve memory, release the disassembly information
                  // if we built it directly above.
                  if (builtMethodDisassmbly)
                  {
                     this.driver.EvictDisassembly(elementNode);
                  }
               
                  // Write newline.
                  writer.WriteLine();
               }
            }
                        
            // Write children.

            this.DumpAux(writer, node.Nodes);
         }
      }
      
      /// <summary>
      /// Handles the 'Exit' menu item.
      /// </summary>
      private void exitToolStripMenuItem_Click(object sender, EventArgs e)
      {         
         this.Close();
      }

      /// <summary>
      /// Sets the global FunctionUnitState and updates the main menu items.
      /// </summary>
      private void SetFunctionUnitRaiseLevel(ToolStripMenuItem menuItem)
      {
         // Remember the old state.
         Phx.FunctionUnitState previousRaiseToState =
            this.raiseToState;
      
         // Uncheck all menu items.
         foreach (ToolStripItem item 
            in this.showToolStripMenuItem.DropDownItems)
         {
            if (item is ToolStripMenuItem)
            {
               (item as ToolStripMenuItem).Checked = false;
            }
         }
         // Check the selected menu item.
         menuItem.Checked = true;

         // Set the global FunctionUnitState according to the selected
         // menu item.
         if (menuItem == this.sourceListingToolStripMenuItem)
         {
            this.raiseToState =
               GlobalData.SourceFunctionUnitState;
         }
         else if (menuItem == this.iLListingToolStripMenuItem)
         {
            this.raiseToState =
               GlobalData.IlFunctionUnitState;
         }
         else if (menuItem == this.hIRToolStripMenuItem)
         {
            this.raiseToState = 
               Phx.FunctionUnit.HighLevelIRFunctionUnitState;
         }
         else if (menuItem == this.lIRToolStripMenuItem)
         {
            this.raiseToState = 
               Phx.FunctionUnit.LowLevelIRBeforeLayoutFunctionUnitState;
         }
         else if (menuItem == this.eIRToolStripMenuItem)
         {
            this.raiseToState = 
               Phx.FunctionUnit.EncodedIROnlyFunctionUnitState;
         }
         else if (menuItem == this.mIRToolStripMenuItem)
         {
            this.raiseToState = 
               Phx.FunctionUnit.SymbolicFunctionUnitState;
         }
         else
         {
            Debug.Assert(false, "Unhandled");
         }

         // If the global FunctionUnitState has changed, evict all
         // method disassemblies and refresh the selected tree view item.
         if (!previousRaiseToState.Equals(this.raiseToState) &&
            this.treeView1.Nodes != null)
         {
            this.InvalidateMethodDisassemblies(this.treeView1.Nodes);
            
            TreeNode selected = this.treeView1.SelectedNode;
            this.treeView1.SelectedNode = null;
            this.treeView1.SelectedNode = selected;            
         }
      }

      /// <summary>
      /// Invalidates method disassembly information associated 
      /// with members of the given TreeNodeCollection.
      /// </summary>
      private void InvalidateMethodDisassemblies(TreeNodeCollection nodes)
      {
         foreach (TreeNode node in nodes)
         {
            // Evict the disassembly if it is associated with 
            // a function symbol.
            if (node.Tag != null && node.Tag is ElementNode)
            {
               ElementNode elementNode = node.Tag as ElementNode;
               if (elementNode.Symbol != null &&
                   elementNode.Symbol.IsFunctionSymbol)
               {
                  this.driver.EvictDisassembly(elementNode);                  
               }
            }
            
            // Invalidate children.
            this.InvalidateMethodDisassemblies(node.Nodes);            
         }
      }

      // Default color used to highlight operands in the disassembly window.
      private static readonly Color defaultVariableColor = Color.Azure;
      // Current color used to highlight operands in the disassembly window.      
      private Color variableColor = defaultVariableColor;

      /// <summary>
      /// Handles the 'Reset Operand Color' menu item.
      /// </summary>
      private void resetOperandColorToolStripMenuItem_Click(object sender, EventArgs e)
      {
         this.UpdateOperandColor(defaultVariableColor);
      }

      /// <summary>
      /// Handles the 'Select Operand Color' menu item.
      /// </summary>
      private void selectOperandColorToolStripMenuItem_Click(object sender, EventArgs e)
      {
         // Allow the user to select a new color from a ColorDialog.

         ColorDialog dialog = new ColorDialog();

         // Set initial color.
         dialog.Color = this.variableColor;
         // Show custom colors.
         dialog.FullOpen = true;

         if (dialog.ShowDialog() == DialogResult.OK)
         {
            this.UpdateOperandColor(dialog.Color);
         }
      }

      /// <summary>
      /// Updates the current operand color.
      /// </summary>
      private void UpdateOperandColor(Color color)
      {
         if (this.variableColor != color)
         {
            // Update color.
            this.variableColor = color;

            // Force UI update.
            TreeNode selected = this.treeView1.SelectedNode;
            this.treeView1.SelectedNode = null;
            this.treeView1.SelectedNode = selected;
         }
      }

      /// <summary>
      /// Handles Disassembly -> Show drop down menu.
      /// </summary>
      private void showDisassemblyMenuItem_Click(object sender, EventArgs e)
      {
         this.SetFunctionUnitRaiseLevel(sender as ToolStripMenuItem);
      }

      /// <summary>
      /// Occurs before the form is closed.
      /// </summary>
      private void Form1_FormClosing(object sender, FormClosingEventArgs e)
      {
         // Wait for any active background workers to complete before
         // closing the form.
         if (this.backgroundWorker1.IsBusy || this.backgroundWorker2.IsBusy)
         {
            this.backgroundWorker1.CancelAsync();
            this.backgroundWorker2.CancelAsync();
            while (this.backgroundWorker1.IsBusy ||
               this.backgroundWorker2.IsBusy)
            {
               Application.DoEvents();               
            }
         }

         // Close the current ModuleUnit and invalidate our Driver object.
         if (this.driver != null)
         {
            this.driver.CloseModuleUnit();
            this.driver = null;
         }
      }

      /// <summary>
      /// Handles the 'Run Tool' menu item.
      /// </summary>
      private void runToolToolStripMenuItem_Click(object sender, EventArgs e)
      {
         // Open a new RunToolDialog dialog.
         RunToolDialog dialog = new RunToolDialog();
         if (dialog.ShowDialog() == DialogResult.OK)
         {
            int result = -1;            
            try
            {
               // Run the tool.
               this.UseWaitCursor = true;
               result = ExecuteProcess(
                  dialog.ToolPath,
                  dialog.CommandLine,
                  true
               );
            }
            catch (System.ComponentModel.Win32Exception)
            {
            }
            finally
            {
               this.UseWaitCursor = false;
            }

            // If the tool returned a non-zero result (or an error occurred while
            // running it), ask the user if she would like to continue processing.
            if (result != 0)
            {
               string messageString = string.Format(
                  "An error occurred while running {0}, or the tool " +
                  "returned a non-zero exit code.{1}Continue processing output file?",
                  Path.GetFileName(dialog.ToolPath), Environment.NewLine);

               if (MessageBox.Show(messageString, "Error", 
                  MessageBoxButtons.YesNo, MessageBoxIcon.Question) == DialogResult.No)
               {
                  return;
               }
            }

            // Close the current file and open the selected assembly.
            this.CloseFile();
            this.sourceFileName = dialog.ResultPath;
            this.OpenFile();
         }
      }

      /// <summary>
      /// Executes the process at the given path with the given argument string.
      /// </summary>
      private static int ExecuteProcess(string path, string arguments, bool waitForExit)
      {
         Process process = new Process();
         ProcessStartInfo processStartInfo = new ProcessStartInfo(path);
         processStartInfo.UseShellExecute = false;
         processStartInfo.Arguments = arguments;
         process.StartInfo = processStartInfo;

         process.Start();
         if (waitForExit)
         {
            process.WaitForExit();
            return process.ExitCode;
         }
         return 0;
      }

      /// <summary>
      /// Populates the symbol filter dropdown menu.
      /// </summary>
      private void PopulateSymbolFilter()
      {
         // Create an array of all available symbol kinds.
         Phx.Symbols.SymbolKind[] symbolKinds = new Phx.Symbols.SymbolKind[] {
            Phx.Symbols.SymbolKind.Assembly,
            Phx.Symbols.SymbolKind.Resource,
            Phx.Symbols.SymbolKind.File,
            Phx.Symbols.SymbolKind.Attribute,
            Phx.Symbols.SymbolKind.Permission,
            Phx.Symbols.SymbolKind.Property,
            Phx.Symbols.SymbolKind.Event,
            Phx.Symbols.SymbolKind.Constant,
            Phx.Symbols.SymbolKind.Export,
            Phx.Symbols.SymbolKind.ImportModule,
            Phx.Symbols.SymbolKind.Import,
            Phx.Symbols.SymbolKind.Field,
            Phx.Symbols.SymbolKind.StaticField,
            Phx.Symbols.SymbolKind.Function,
            Phx.Symbols.SymbolKind.Variable,
            Phx.Symbols.SymbolKind.GlobalVariable,
            Phx.Symbols.SymbolKind.LocalVariable,
            Phx.Symbols.SymbolKind.NonLocalVariable,
            Phx.Symbols.SymbolKind.ProxyStaticField,
            Phx.Symbols.SymbolKind.Label,
            Phx.Symbols.SymbolKind.Section,
            Phx.Symbols.SymbolKind.Type,
            Phx.Symbols.SymbolKind.MsilType,
            Phx.Symbols.SymbolKind.TypeDefinition,
            Phx.Symbols.SymbolKind.CompilationUnit,
            Phx.Symbols.SymbolKind.Namespace,
            Phx.Symbols.SymbolKind.Scope,
         };
         // Sort them alphabetically.
         Array.Sort(symbolKinds, CompareSymbolKinds);
         
         // Add each item to the menu.
         ToolStripItemCollection dropDownItems = 
            this.filterToolStripMenuItem.DropDownItems;
         foreach (Phx.Symbols.SymbolKind kind in symbolKinds)
         {
            ToolStripMenuItem item =
               dropDownItems.Add(string.Concat(kind, " symbols"))
                  as ToolStripMenuItem;
               
            item.Tag = kind;
            item.Checked = true;
            item.CheckOnClick = true;
            item.CheckStateChanged += new EventHandler(filterItem_CheckStateChanged);             
         }

         // Move the 'Show All' and 'Hide All' items to the bottom of the list.
         dropDownItems.Add(new ToolStripSeparator());
         ToolStripItem first = dropDownItems[0];
         ToolStripItem second = dropDownItems[1];
         dropDownItems.RemoveAt(0);
         dropDownItems.RemoveAt(0);
         dropDownItems.Add(first);
         dropDownItems.Add(second);
      }
      
      /// <summary>
      /// Compares the provided SymbolKind objects by name.
      /// </summary>
      private static int CompareSymbolKinds(Phx.Symbols.SymbolKind x,
         Phx.Symbols.SymbolKind y)
      {
         return x.ToString().CompareTo(y.ToString());
      }

      /// <summary>
      /// Called when the checked stated of a symbol filter menu item 
      /// has changed.
      /// </summary>
      void filterItem_CheckStateChanged(object sender, EventArgs e)
      {
         ToolStripMenuItem menuItem = sender as ToolStripMenuItem;

         // The group key in the Symbols window is just the 
         // SymbolKind name.
         string key = menuItem.Text.Substring(0, 
            menuItem.Text.Length - " symbols".Length);

         // Toggle visibility of the selected group.
         if (menuItem.Checked)
         {
            this.ShowGroup(key);
         }
         else
         {
            this.HideGroup(key);
         }
      }

      /// <summary>
      /// Determines whether the provided SymbolKind is
      /// filtered from the symbol list.
      /// </summary>
      private bool IsFiltered(Phx.Symbols.SymbolKind symbolKind)
      {
         foreach (ToolStripItem item 
            in this.filterToolStripMenuItem.DropDownItems)
         {
            if (item.Tag != null && item.Tag.Equals(symbolKind))
            {
               return !(item as ToolStripMenuItem).Checked;
            }
         }
         return false;
      }

      /// <summary>
      /// Adds the group with the specified key back to the 
      /// symbol list view.
      /// </summary>
      private void ShowGroup(string key)
      {
         if (this.filteredItems.ContainsKey(key))
         {
            foreach (ListViewItem filteredItem in this.filteredItems[key])
            {
               this.AddListViewItem(filteredItem.Tag as Phx.Symbols.Symbol,
                  filteredItem.ImageIndex);
            }
            this.filteredItems.Remove(key);
         }
         this.SortListView(this.listView1);
      }

      /// <summary>
      /// Removes the group with the specified key from the 
      /// symbol list view.
      /// </summary>
      private void HideGroup(string key)
      {
         ListViewGroup group = this.listView1.Groups[key];
         if (group != null)
         {  
            ListViewItem[] items = new ListViewItem[group.Items.Count];
            group.Items.CopyTo(items, 0);

            if (! this.filteredItems.ContainsKey(key))
            {
               this.filteredItems.Add(key, new List<ListViewItem>());
            }
            this.filteredItems[key].AddRange(items);
            
            foreach (ListViewItem item in items)
            {
               this.listView1.Items.Remove(item);
            }
            group.Items.Clear();
            this.listView1.Groups.Remove(group);
         }
      }

      /// <summary>
      /// Handles the 'Show All' menu item.
      /// </summary>
      private void showAllToolStripMenuItem_Click(object sender, EventArgs e)
      {
         this.CheckAllFilterMenuItems(true);         
      }

      /// <summary>
      /// Handles the 'Hide All' menu item.
      /// </summary>
      private void hideAllToolStripMenuItem_Click(object sender, EventArgs e)
      {
         this.CheckAllFilterMenuItems(false);
      }

      /// <summary>
      /// Shows or hides all groups in the Symbol tab.
      /// </summary>
      private void CheckAllFilterMenuItems(bool check)
      {
         foreach (ToolStripItem item in
            this.filterToolStripMenuItem.DropDownItems)
         {
            if (item is ToolStripSeparator)
            {
               break;
            }
            ToolStripMenuItem menuItem = item as ToolStripMenuItem;
            if (menuItem.Checked != check)
            {
               menuItem.Checked = check;
            }
         }
      }

      #region IBrowseFile Members

      /// <summary>
      /// Allows the user to browse for the file with the given name.
      /// </summary>
      public string Browse(string originalFile)
      {         
         OpenFileDialog dialog = new OpenFileDialog();
         dialog.CheckFileExists = true;
         dialog.CheckPathExists = true;
         dialog.Filter = string.Format(
            "Source Files (*{0})|**{0}|Any type (*.*)|*.*",
            Path.GetExtension(originalFile));
         dialog.Multiselect = false;
         dialog.Title = string.Format("Original file: {0}", originalFile);

         if (dialog.ShowDialog() == DialogResult.OK)
         {
            return dialog.FileName;
         }
         return originalFile;
      }

      #endregion      
            
   }
}
