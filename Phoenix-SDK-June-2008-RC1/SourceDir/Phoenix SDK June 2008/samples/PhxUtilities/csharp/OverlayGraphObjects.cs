//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Overlay graph objects and definitions.
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
   //    Definition for OverlayGraph object.
   //
   // Remarks:
   //
   //    This is a new type of graph which inherits from Phx.Graphs.Graph.
   //
   //--------------------------------------------------------------------------

   public class 
   OverlayGraph : Phx.Graphs.Graph
   {
      //--------------------------------------------------------------------------
      //
      // Description:
      //
      //    Private member fields.
      //
      // Remarks:
      //
      //    The two member fields provide for an internal mapping of nodes for
      //    the graph. This internal mapping allows the GetOverlayNode function
      //    to run in O(1) time. The nullNode field is necessary because a
      //    dictionary cannot contain a null key.
      //
      //--------------------------------------------------------------------------

      private PhxUtilities.OverlayNode nullNode;
      private System.Collections.Generic.Dictionary
         <System.Object, PhxUtilities.OverlayNode> nodeHash;

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Construct an OverlayGraph object.
      //
      // Remarks:
      //
      //    This is the constructor function. The unit parameter is used for 
      //    its lifetime which is passed to the inherited Initialize function.
      //
      // Parameters:
      //
      //   unit - Unit to use in constructing the Graph objects.
      //
      // Returns:
      //
      //    OverlayGraph object.
      //
      //-----------------------------------------------------------------------

      new public static OverlayGraph 
      New
      (
         Phx.Unit unit
      )
      {
         OverlayGraph og = new OverlayGraph();

         og.nullNode = null;
         og.Initialize(unit.Lifetime, Phx.Graphs.GraphKind.Graph);
         og.nodeHash = new System.Collections.Generic.Dictionary
            <System.Object, OverlayNode>();

         return og;
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Return an OverlayNode apart of this graph with object field 'o'.
      //
      // Remarks:
      //
      //    This function either finds the existing node in the graph with
      //    a pointer to Object 'o' or creates a new node in the graph
      //    with that property.
      //
      // Parameters:
      //
      //    o - Object which the OverlayNode will point to.
      //
      // Returns:
      //
      //    OverlayNode object which points to 'o'.
      //
      //-----------------------------------------------------------------------

      public OverlayNode 
      GetOverlayNode
      (
         System.Object nodeObj
      )
      {
         if (nodeObj == null)
         {
            if (this.nullNode == null)
            {
               this.nullNode = new OverlayNode();
               this.nullNode.NodeObject = null;
               this.InitializeNode(this.nullNode);

               return this.nullNode;
            }
            else
            {
               return this.nullNode;
            }
         }
         else
         {
            if (this.nodeHash.ContainsKey(nodeObj))
            {
               return this.nodeHash[nodeObj];
            }

            OverlayNode on = new OverlayNode();
            on.NodeObject = nodeObj;
            this.InitializeNode(on);

            nodeHash[nodeObj] = on;

            return on;
         }
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Definition for OverlayNode object.
   //
   //--------------------------------------------------------------------------

   public class 
   OverlayNode : Phx.Graphs.Node
   {
      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Private member fields.
      //
      // Remarks:
      //
      //    These objects allow nodes to define complex relationships to other
      //    nodes. By being generically based on Phx.Graphs.Node, the 
      //    OverlayNode may overlay any other type of node including a node of
      //    the same type. The objects allow for two lists that define what
      //    the node contains and belongs to and a mapping that defines what
      //    nodes this node maps to based on a string key.
      //
      //-----------------------------------------------------------------------

      private System.Object nodeObj;
      private System.Collections.Generic.LinkedList
         <Phx.Graphs.Node> containsNodes;
      private System.Collections.Generic.LinkedList
         <Phx.Graphs.Node> belongsToNodes;
      private System.Collections.Generic.Dictionary
         <System.String, Phx.Graphs.Node> mappingToNodes;

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Accessor functions for private data.
      //
      //-----------------------------------------------------------------------

      public System.Object 
      NodeObject
      {
         get
         {
            return this.nodeObj;
         }
         set
         {
            this.nodeObj = value;
         }
      }

      public System.Collections.Generic.LinkedList<Phx.Graphs.Node>
      ContainsNodes
      {
         get
         {
            return this.containsNodes;
         }
      }

      public System.Collections.Generic.LinkedList<Phx.Graphs.Node>
      BelongsToNodes
      {
         get
         {
            return this.belongsToNodes;
         }
      }

      public
      System.Collections.Generic.Dictionary<System.String, Phx.Graphs.Node>
      MappingToNodes
      {
         get
         {
            return this.mappingToNodes;
         }
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Constructor for OverlayNode object.
      //
      //-----------------------------------------------------------------------

      public 
      OverlayNode()
      {
         this.containsNodes =
            new System.Collections.Generic.LinkedList<Phx.Graphs.Node>();
         this.belongsToNodes =
            new System.Collections.Generic.LinkedList<Phx.Graphs.Node>();
         this.mappingToNodes =
            new System.Collections.Generic.Dictionary
               <System.String, Phx.Graphs.Node>();

         // Initialize is an inherited member function.
         this.Initialize(Phx.Graphs.NodeKind.Node);
      }
   }
}
