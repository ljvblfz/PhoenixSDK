//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Pereader plug-in for detecting multiple namespaces defined within one
//    file.
//
// Remarks:
//
//    This plug-in is useful as a demonstration of what the DependencyGraph
//    in PhxUtilities is capable of doing.
//
// Usage:
//
//    pereader /callgraph /plugin [PATH TO PLUGIN] [PATH TO BINARY]
//
//-----------------------------------------------------------------------------

namespace SingleFileMultipleNamespace
{
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    DetectMultipleNamespace class object and definitions.
   //
   // Remarks:
   //
   //    This class inherits from Phx.Phases.Phase as it is intended to be run
   //    as a phase which builds up the PhxUtilities.DependencyGraph and then runs
   //    an analysis on it.
   //
   //--------------------------------------------------------------------------

   public class 
   DetectMultipleNamespace : Phx.Phases.Phase
   {
      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Private data members.
      //
      // Remarks:
      //
      //    firstTimeInit - For initializing depGraph properly.
      //    depGraph - The DependencyGraph used for analysis.
      //    samplePluginControl - For hooking the plug-in in properly.
      //
      //-----------------------------------------------------------------------

      private bool firstTimeInit;
      private PhxUtilities.DependencyGraph depGraph;
      private Phx.Controls.ComponentControl samplePluginControl;

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Constructs a new DetectMultipleNamespace class.
      //
      // Returns:
      //
      //    Newly created object.
      //
      //-----------------------------------------------------------------------

      public static DetectMultipleNamespace 
      New()
      {
         DetectMultipleNamespace newPhase = new DetectMultipleNamespace();

         newPhase.samplePluginControl =
            Phx.Controls.ComponentControl.New("DetectMultipleNamespacePlugin",
               "mult ns plugin", "dmns-plug-in");

         newPhase.firstTimeInit = true;

         return newPhase;
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Sets up the phase in the phase list.
      //
      // Remarks:
      //
      //    This plug-in runs as a phase after the call graph edges are added.
      //    It is therefore important to specify /callgraph as a command line
      //    option for pereader.
      //
      // Parameters:
      //
      //    config - For inserting the phase properly.
      //
      // Returns:
      //
      //    true - Indicates success.
      //
      //-----------------------------------------------------------------------

      public bool 
      Setup
      (
         Phx.Phases.PhaseConfiguration config
      )
      {
         Phx.Phases.Phase usePhase;

         usePhase = config.PhaseList.FindByName("Add Call Graph Edges");
#if (PHX_DEBUG_CHECKS)
         Phx.Asserts.Assert(usePhase != null, "Phase not found.");
#endif
         this.Initialize(config, "DetectMultipleNamespacePhase");
         usePhase.InsertAfter(this);
         this.PhaseControl = samplePluginControl;

         // Relies on assertion in case of failure.
         return true;
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Builds up the DependencyGraph and calls DoAnalysis when there
      //    are no more function units to process.
      //
      // Parameters:
      //
      //    unit - The unit currently being processed by the phases.
      //
      //-----------------------------------------------------------------------

      protected override void 
      Execute
      (
         Phx.Unit unit
      )
      {
         if (unit.IsFunctionUnit)
         {
            // Initialize the DependencyGraph the first time.
            if (this.firstTimeInit)
            {
               this.depGraph = PhxUtilities.DependencyGraph.New(
                  unit.ParentPEModuleUnit);
               
               // Don't reinitialize the DependencyGraph.
               this.firstTimeInit = false;
            }

            Phx.FunctionUnit fu = unit.AsFunctionUnit;

            // Add the function unit to the DependencyGraph.
            this.depGraph.AddToDependencyGraph(fu);
         }
         
         // Check if there are no more units to process.
         if (unit.Next == null)
         {
            DoAnalysis();
         }
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Analyzes the DependencyGraph objects and prints analysis.
      //
      // Remarks:
      //
      //    TODO
      //
      //-----------------------------------------------------------------------

      private void 
      DoAnalysis()
      {
         // Iterate over each file node.
         for (Phx.Graphs.Node fileNode =
            depGraph.FileDependencyGraph.NodeList;
            fileNode != null;
            fileNode = fileNode.Next)
         {
            PhxUtilities.OverlayNode onFileNode =
               fileNode as PhxUtilities.OverlayNode;
#if (PHX_DEBUG_CHECKS)
            Phx.Asserts.Assert(onFileNode != null, "onFileNode != null");
#endif
            System.String fileName = onFileNode.NodeObject as System.String;

            // Skip the null case if there is one.
            if (fileName == null)
            {
               continue;
            }

            // Only process .cpp, .h, .cs, or .inl files.
            bool validFile = fileName.Contains(".cpp")
               || fileName.Contains(".h")
               || fileName.Contains(".cs")
               || fileName.Contains(".inl");

            if (!validFile)
            {
               continue;
            }

            // A dictionary is used to guarantee O(1) time complexity.
            System.Collections.Generic.Dictionary
               <System.String, System.String> nameSpaces;
            nameSpaces = new System.Collections.Generic.Dictionary
               <System.String, System.String>();

            // Iterate over each function node in the file.
            foreach (Phx.Graphs.Node funcNode in onFileNode.ContainsNodes)
            {
               PhxUtilities.OverlayNode onFuncNode2 =
                  funcNode as PhxUtilities.OverlayNode;
#if (PHX_DEBUG_CHECKS)
               Phx.Asserts.Assert(onFuncNode2 != null);
               Phx.Asserts.Assert(
                  onFuncNode2.MappingToNodes.ContainsKey("Namespace"));
#endif
               // Recall Namespace node to which function maps.
               Phx.Graphs.Node nsNode =
                  onFuncNode2.MappingToNodes["Namespace"];
#if (PHX_DEBUG_CHECKS)
               Phx.Asserts.Assert(nsNode != null, "nsNode != null");
#endif
               // Get the Namespace node as an PhxUtilities.OverlayNode.
               PhxUtilities.OverlayNode onNsNode = nsNode as PhxUtilities.OverlayNode;
#if (PHX_DEBUG_CHECKS)
               Phx.Asserts.Assert(onNsNode != null, "onNsNode != null");
#endif
               // Get the nameSpace string off the OverlayNode object.
               string nameSpace = onNsNode.NodeObject as System.String;
#if (PHX_DEBUG_CHECKS)
               Phx.Asserts.Assert(nameSpace != null, "nameSpace != null");
#endif
               // Ignore mangled and System nameSpaces.
               if (!nameSpace.StartsWith("System")
                  && !nameSpace.Contains("<")
                  && !nameSpace.Contains("?"))
               {
                  //-----------------------------------------------------------
                  //
                  // Remarks:
                  //
                  //    If a list had been used instead of a dictionary then
                  //    here is a location where the time complexity might be
                  //    made worse by requiring a linear search through a list.
                  //
                  //-----------------------------------------------------------
                  
                  nameSpaces[nameSpace] = "";
               }
            }

            // If more than one namespace was added.
            if (nameSpaces.Count > 1)
            {
               Phx.Output.Write("File: " + fileName);
               Phx.Output.Write(
                  " has definitions in the following namespaces:\n");

               foreach (System.String s in nameSpaces.Keys)
               {
                  Phx.Output.WriteLine(s);
               }

               Phx.Output.WriteLine();
            }
         }
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    This is the plug-in driver class for DetectMultipleNamespace.
   //
   //--------------------------------------------------------------------------

   public class 
   PlugInDriver : Phx.PlugIn
   {
      static DetectMultipleNamespace myPhase;

      public override void 
      RegisterObjects()
      {
         myPhase = DetectMultipleNamespace.New();
      }

      public override void
      BuildPhases
      (
         Phx.Phases.PhaseConfiguration config
      )
      {
         myPhase.Setup(config);
      }

      public override System.String 
      NameString
      {
         get
         {
            return "DetectMultipleNamespacePhase";
         }
      }
   }
}
