//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Dependency graph object and definitions.
//
// Usage:
//
//    This is compiled to a .dll which should be used by other tools.
//
//-----------------------------------------------------------------------------

namespace PhxUtilities
{
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Definition for DependencyGraph object.
   //
   //--------------------------------------------------------------------------

   public class 
   DependencyGraph
   {
      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Private member fields.
      //
      // Remarks:
      //
      //    The four private members overlay graphs which reflect the design
      //    at different levels. The most basic level is the function level
      //    which is modeled by the funcGraph. The next level up from that is
      //    the class level and the namespace level which are modeled by
      //    classGraph and nsGraph respectively. The fileGraph object is a set
      //    of nodes which overlay the funcGraph.
      //
      //-----------------------------------------------------------------------

      private PhxUtilities.OverlayGraph funcGraph;
      private PhxUtilities.OverlayGraph classGraph;
      private PhxUtilities.OverlayGraph nsGraph;
      private PhxUtilities.OverlayGraph fileGraph;

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Accessor functions for private data.
      //
      // Remarks:
      //
      //    The accessor functions provide read-only access for the pointers.
      //    This does not mean that external functions cannot change the 
      //    internal graphs arbitrarily.
      //
      //-----------------------------------------------------------------------

      public PhxUtilities.OverlayGraph 
      FunctionDependencyGraph
      {
         get
         {
            return funcGraph;
         }
      }

      public PhxUtilities.OverlayGraph 
      ClassDependencyGraph
      {
         get
         {
            return classGraph;
         }
      }

      public PhxUtilities.OverlayGraph 
      NamespaceDependencyGraph
      {
         get
         {
            return nsGraph;
         }
      }

      public PhxUtilities.OverlayGraph 
      FileDependencyGraph
      {
         get
         {
            return fileGraph;
         }
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Construct a DependencyGraph object.
      //
      // Remarks:
      //
      //    This is essentially the constructor for the DependencyGraph.
      //
      // Parameters:
      //
      //    unit - Unit to use in constructing the OverlayGraph objects.
      //
      // Returns:
      //
      //    DependencyGraph object.
      //
      //-----------------------------------------------------------------------

      public static DependencyGraph 
      New
      (
         Phx.Unit unit
      )
      {
         DependencyGraph newPhase = new DependencyGraph();

         newPhase.funcGraph = OverlayGraph.New(unit);
         newPhase.classGraph = OverlayGraph.New(unit);
         newPhase.nsGraph = OverlayGraph.New(unit);
         newPhase.fileGraph = OverlayGraph.New(unit);

         return newPhase;
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Adds a node to the internal graphs based on the properties of 
      //    the function unit that is passed in.
      //
      // Remarks:
      //
      //    This function creates the nodes for each level of graph that is 
      //    built by the DependencyGraph object. It also adds edges
      //    to each level except the fileGraph representing dependencies
      //    between nodes.
      //
      // Parameters:
      //
      //   newFunctionUnit - Provides information to construct build graph.
      //
      //-----------------------------------------------------------------------

      public void AddToDependencyGraph
      (
         Phx.FunctionUnit newFunctionUnit
      )
      {
#if (PHX_DEBUG_CHECKS)
         // Passing null to the function is not allowed.
         Phx.Asserts.Assert(funcUnit != null, "funcUnit != null");
#endif
         Phx.Symbols.FunctionSymbol funcSym = newFunctionUnit.FunctionSymbol;

#if (PHX_DEBUG_CHECKS)
         Phx.Asserts.Assert(funcSym != null, "funcSym != null");
#endif

         // Construct the node objects.
         Phx.Graphs.CallNode callNode;
         Phx.Types.AggregateType aggregateType;
         System.String namespaceString;
         System.String filenameString;

#if (PHX_DEBUG_CHECKS)
         Phx.Asserts.Assert(funcSym.CallNode != null);
         Phx.Asserts.Assert(funcUnit.DebugInfo != null);
         Phx.Asserts.Assert(funcUnit.FirstInstruction != null);
#endif

         callNode          = funcSym.CallNode;
         aggregateType     = funcSym.EnclosingAggregateType;
         namespaceString   =
            new System.String(
               PhxUtilities.Utilities.GetNamespaceFromFunctionSymbol(funcSym)
                  .ToCharArray());
         filenameString    =
            newFunctionUnit.DebugInfo.GetFileName(
               newFunctionUnit.FirstInstruction.DebugTag);
               
#if (PHX_DEBUG_CHECKS)
         // The call graph must exist.
         Phx.Asserts.Assert(callNode != null);
#endif

         // Create the graph nodes according to the objects.
         OverlayNode onFunc      =
            this.funcGraph.GetOverlayNode(callNode);
         OverlayNode onAggType   =
            this.classGraph.GetOverlayNode(aggregateType);
         OverlayNode onNamespace =
            this.nsGraph.GetOverlayNode(namespaceString);
         OverlayNode onFile      =
            this.fileGraph.GetOverlayNode(filenameString);

         // Set contains/belongs properties.
         if (!onFunc.ContainsNodes.Contains(callNode))
         {
            onFunc.ContainsNodes.AddFirst(callNode);
         }
         if (!onFunc.BelongsToNodes.Contains(onAggType))
         {
            onFunc.BelongsToNodes.AddFirst(onAggType);
         }

         if (!onAggType.ContainsNodes.Contains(onFunc))
         {
            onAggType.ContainsNodes.AddFirst(onFunc);
         }
         if (!onAggType.BelongsToNodes.Contains(onNamespace))
         {
            onAggType.BelongsToNodes.AddFirst(onNamespace);
         }

         if (!onNamespace.ContainsNodes.Contains(onAggType))
         {
            onNamespace.ContainsNodes.AddFirst(onAggType);
         }

         if (!onFile.ContainsNodes.Contains(onFunc))
         {
            onFile.ContainsNodes.AddFirst(onFunc);
         }

         // Set mappings.
         onFunc.MappingToNodes["AggregateType"] = onAggType;
         onFunc.MappingToNodes["Namespace"] = onNamespace;
         onFunc.MappingToNodes["File"] = onFile;

         // Variables for the loops.
         Phx.Graphs.CallNode     callNodeDep;
         Phx.Types.AggregateType aggregateTypeDep;
         System.String           namespaceStringDep;

         OverlayNode onFuncDep;
         OverlayNode onAggTypDep;
         OverlayNode onNSDep;

         // Loop over predecessor edges.
         for (Phx.Graphs.Edge e = callNode.PredecessorEdgeList;
            e != null;
            e = e.NextPredecessorEdge)
         {
            // Get the predecessor node objects.
            callNodeDep = e.PredecessorNode.AsCallNode;

#if (PHX_DEBUG_CHECKS)
            Phx.Asserts.Assert(callNodeDep.FunctionSymbol != null);
#endif

            aggregateTypeDep =
               callNodeDep.FunctionSymbol.EnclosingAggregateType;
            namespaceStringDep = new System.String(
               PhxUtilities.Utilities.GetNamespaceFromFunctionSymbol(
                  callNodeDep.FunctionSymbol).ToCharArray());

            // Create the predecessor nodes based on the objects.
            onFuncDep   =
               funcGraph.GetOverlayNode(callNodeDep);
            onAggTypDep =
               classGraph.GetOverlayNode(aggregateTypeDep);
            onNSDep     =
               nsGraph.GetOverlayNode(namespaceStringDep);

            // Create edges according to dependency relationships.
            funcGraph.NewEdge(onFuncDep, onFunc);
            classGraph.NewEdge(onAggTypDep, onAggType);
            nsGraph.NewEdge(onNSDep, onNamespace);
         }

         // Loop over successor edges.
         for (Phx.Graphs.Edge e = callNode.SuccessorEdgeList;
            e != null;
            e = e.NextSuccessorEdge)
         {
            // Get the successor node objects.
            callNodeDep = e.SuccessorNode.AsCallNode;

#if (PHX_DEBUG_CHECKS)
            Phx.Asserts.Assert(callNodeDep.FunctionSymbol != null);
#endif
            aggregateTypeDep =
               callNodeDep.FunctionSymbol.EnclosingAggregateType;
            namespaceStringDep = new System.String(
               PhxUtilities.Utilities.GetNamespaceFromFunctionSymbol(
                  callNodeDep.FunctionSymbol).ToCharArray());

            // Create the successor nodes based on the objects.
            onFuncDep   =
               funcGraph.GetOverlayNode(callNodeDep);
            onAggTypDep =
               classGraph.GetOverlayNode(aggregateTypeDep);
            onNSDep     =
               nsGraph.GetOverlayNode(namespaceStringDep);

            // Create edges according to dependency relationships.
            funcGraph.NewEdge(onFunc, onFuncDep);
            classGraph.NewEdge(onAggType, onAggTypDep);
            nsGraph.NewEdge(onNamespace, onNSDep);
         }
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Adds edges between file nodes based on dependencies between 
      //    the functions which are contained by those files.
      //
      // Remarks:
      //
      //    This function is necessary because edges cannot be added as they
      //    are for other graphs. This is because not all file nodes of the 
      //    fileGraph will necessarily exist until this function is called.
      //    It is assumed that this function will be called once after every
      //    function unit has been passed to the AddToDependencyGraph function.
      //
      //-----------------------------------------------------------------------

      public void 
      BuildFileDependencyInfo()
      {
         // Iterate over every file node in the file graph.
         for (Phx.Graphs.Node fileNode = fileGraph.NodeList;
            fileNode != null;
            fileNode = fileNode.Next)
         {
            System.String filename =
               ((fileNode as OverlayNode).NodeObject as System.String);

            // Iterate over every function node contained by the file node.
            foreach (Phx.Graphs.Node funcNode in
               (fileNode as OverlayNode).ContainsNodes)
            {

               // Iterate over all successor edges.
               for (Phx.Graphs.Edge e = funcNode.SuccessorEdgeList;
                  e != null;
                  e = e.NextSuccessorEdge)
               {
                  OverlayNode succNode = e.SuccessorNode as OverlayNode;

#if (PHX_DEBUG_CHECKS)
                  Phx.Asserts.Assert(succNode != null);
#endif

                  // File node may not exist for all function nodes.
                  if (succNode.MappingToNodes.ContainsKey("File"))
                  {
                     Phx.Graphs.Node succFileNode =
                        succNode.MappingToNodes["File"];

#if (PHX_DEBUG_CHECKS)
                     Phx.Asserts.Assert(succFileNode != null);
#endif
                     OverlayNode onSuccFileNode = succFileNode as OverlayNode;

#if (PHX_DEBUG_CHECKS)
                     Phx.Asserts.Assert(onSuccFileNode != null);
#endif

                     System.String succNodeFilename =
                        onSuccFileNode.NodeObject as System.String;

                     fileGraph.NewEdge(fileNode,
                        (succNode as Phx.Graphs.Node));
                  }
               }

               // Iterate over all predecessor edges.
               for (Phx.Graphs.Edge e = funcNode.PredecessorEdgeList;
                  e != null;
                  e = e.NextPredecessorEdge)
               {
                  OverlayNode predNode = e.PredecessorNode as OverlayNode;

#if (PHX_DEBUG_CHECKS)
                  Phx.Asserts.Assert(predNode != null);
#endif

                  // File node may not exist for all function nodes.
                  if (predNode.MappingToNodes.ContainsKey("File"))
                  {
                     Phx.Graphs.Node predFileNode =
                        predNode.MappingToNodes["File"];

#if (PHX_DEBUG_CHECKS)
                     Phx.Asserts.Assert(predFileNode != null);
#endif

                     OverlayNode onPredFileNode = predFileNode as OverlayNode;

#if (PHX_DEBUG_CHECKS)
                     Phx.Asserts.Assert(onPredFileNode != null);
#endif

                     System.String predNodeFilename =
                        onPredFileNode.NodeObject as System.String;

                     fileGraph.NewEdge((predNode as Phx.Graphs.Node),
                        fileNode);
                  }
               }
            }
         }
      }
   }
}
