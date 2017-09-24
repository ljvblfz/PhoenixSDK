//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    The DAG implementation for the local optimization plug in
//
//-----------------------------------------------------------------------------


#pragma once

#include "utility.h"

namespace Phx
{
namespace Samples
{
namespace LocalOpt
{

// forward declaration

ref class DagNode;
ref class DagEdge;
ref class OpndKey;
ref class NodeKey;

// Use more readable names for the generics classes


typedef System::Collections::Generic::List<DagEdge ^> DagEdgeList;
typedef System::Collections::Generic::List<DagNode ^> DagNodeList;
typedef Hashtable<OpndKey ^, Phx::IR::Operand ^> OpndToOpndMap;
typedef Hashtable<OpndKey ^, DagNode ^> OpndToNodeMap;
typedef Hashtable<NodeKey ^, DagNode ^> ExprToNodeMap;
typedef Hashtable<int, DagNode ^> TagToDefNodeMap;
typedef Hashtable<int, System::Collections::Generic::List<DagNode ^> ^>
TagToUseNodeMap;

//-----------------------------------------------------------------------------
//
// Description:
//
//    The kind of a DAG node
//
//-----------------------------------------------------------------------------

[System::Flags()]
public enum class DagNodeKind
{
   Use = 1,       // a constant/symbol/memory location refered as src operand
   Definition = 1 << 1,  // an identifier refered as dst operand. It corresponds to
          // one entry in the identifier list mentioned in the dragon book 9.8
   Expression = 1 << 2, // an interior node representing a computation
   Memory = 1 << 3   // a memory src or dst operand
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    The kind of an edge in the DAG. A ValueDep edge represents the true value
//    dependent relationship between the From node and the To node, e.g., the
//    edge between an expression node and its src operands nodes, or the 
//    memory node and its base/index/segment node. An OrderDep edge represents the
//    enforced evaluation order between two nodes. An AssignDep edge indicates
//    the assignment relationship between the From node and the To node, i.e.,
//    the From node will be assigned the value of the ToNode.
//
//-----------------------------------------------------------------------------

public enum class DagEdgeKind
{
   ValueDep = 0,  // an edge indicates value dependency between dagnodes
   OrderDep,   // an edge enforces evaluation order
   AssignDep   // an edge indicates parent will be assigned with value of child
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    The data structure handles constructions of DAGs and queries of DagNode
//
//-----------------------------------------------------------------------------

public ref class DAG
{

public:

   static DAG ^
   New
   (
      Phx::FunctionUnit ^ functionUnit,
      int             tableSize
   ) ;

   DagNode ^
   LookupOpnd
   (
      Phx::IR::Operand ^ operand,
      int             hashCode
   );

   DagNode ^
   FindOrCreateNode
   (
      Phx::IR::Operand ^ operand,
      DagNodeKind     nodeKind
   );

   void
   HandleDstOpnd
   (
      Phx::IR::Instruction ^ instruction,
      DagNode ^        newTargetNode
   );

   Phx::IR::Operand ^
   ResolveMemNode
   (
      DagNode ^ dagNode
   );

   DagNode ^
   ResolveTarget
   (
      DagNode ^ node
   );

   void
   UpdateRenameMap
   (
      OpndToOpndMap ^ renameMap
   );

   static int
   GetHashCode
   (
      Phx::IR::Operand ^ operand
   );

   static int
   GetHashCode
   (
      Phx::IR::Instruction ^ instruction
   );

   static bool
   CompareOperand
   (
      Phx::IR::Operand ^ opndA,
      Phx::IR::Operand ^ opndB
   );

   static bool
   CmpExprNode
   (
      DagNode ^ nodeA,
      DagNode ^ nodeB
   );

   static bool
   CmpMemNode
   (
      DagNode ^ nodeA,
      DagNode ^ nodeB
   );

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A list of all roots in the DAG which is a forest
   //
   //--------------------------------------------------------------------------

   property DagNodeList ^ RootList
   {
      DagNodeList ^ get() { return this->rootList; }
   }

private:

   DagNode ^
   FindOrCreateSimpleNode
   (
      Phx::IR::Operand ^ operand,
      DagNodeKind     nodeKind
   ) ;

   DagNode ^
   FindOrCreateMemNode
   (
      Phx::IR::MemoryOperand ^ operand,
      DagNodeKind        nodeKind
   ) ;

   DagNode ^
   FindOrCreateExprNode
   (
      Phx::IR::Operand ^ operand,
      DagNodeKind     nodeKind
   ) ;

   static bool
   IsSimpleCSECandidate
   (
      Phx::IR::Operand ^ operand,
      DagNodeKind     nodeKind
   ) ;

   static bool
   IsMustGenNode
   (
      DagNode ^ node
   );

   static bool
   CanAssignTmp
   (
      DagNode ^ node
   );

   bool
   Fold
   (
      Phx::IR::Instruction ^                                    instruction,
      [System::Runtime::InteropServices::Out()]DagNode ^% newNode
   ) ;

   bool
   Simplify
   (
      Phx::IR::Instruction ^                                    instruction,
      [System::Runtime::InteropServices::Out()]DagNode ^% newNode
   ) ;

   void
   InvalidateNode
   (
      DagNode ^ toKill,
      DagNode ^ killer
   );

   void RemoveNode (DagNode ^ node) ;

   void EnforceEvlOrderForUse (DagNode ^ node) ;

   void
   EnforceEvlOrderForDef
   (
      Phx::IR::Operand ^ operand,
      DagNode ^       node
   );

   static bool
   CanKillEachOther
   (
      DagNode ^ node1,
      DagNode ^ node2
   );

   void
   AddEdge
   (
      DagNode ^   parent,
      DagNode ^   child,
      DagEdgeKind edgeKind
   );

private:

   // The list of root nodes in the DAG

   DagNodeList ^ rootList;

   // A hash map from an operand to the dag node it attaches to
   // It is used to query dag node information of an operand

   OpndToNodeMap ^ opndToNodeMap;

   // A hash map from an expression to an existing dag node
   // It is used to match common expression

   ExprToNodeMap ^ exprToNodeMap;

   // A hash map from alias tag to its may-partial-overlap definition nodes
   // It is used to enforce correct evaluation order between new uses and the
   // latest definitions, and new definitions and recent uses, and new definitions and old definitions
   // It is a single-value hashtable, i.e., we only keep the latest definition in 
   // the hashtable

   TagToDefNodeMap ^ tagToDefNodeMap;

   // A hash map from alias tag to its may-partial-overlap use nodes
   // It is used to enforce correct evaluation order between new definition and the
   // recent uses. It is a multi-value hashtable

   TagToUseNodeMap ^ tagToUseNodeMap;

   // Cached functionUnit and its aliasInfo

   Phx::FunctionUnit ^ functionUnit;

   Phx::Alias::Info ^ aliasInfo;

   // The previous call node. Used to enforce evaluation order between calls

   DagNode ^ lastCallNode;

};

//-----------------------------------------------------------------------------
//
// Description:
//
//    A node in the DAG
//
//-----------------------------------------------------------------------------

public ref class DagNode
{

public:

   static DagNode ^
   New
   (
      Phx::IR::Operand ^ label,
      int             hashCode,
      DagNodeKind     nodeKind
   ) ;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The representative operand of this DagNode.
   //    If it is an Use node, it is either a constant, a symbol, or a memory 
   //    location. If it is an Expression node, it is the first dst operand of the
   //    instruction that this node represents. If it is a Definition node, it is the
   //    identifier that we need to generate assignment for it later.
   //
   //--------------------------------------------------------------------------

   property Phx::IR::Operand ^ Label
   {
      Phx::IR::Operand ^ get() { return label;}
      void set(Phx::IR::Operand ^ value){ label = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The parent list of this node 
   //
   //--------------------------------------------------------------------------

   property DagEdgeList ^ ParentList
   {
      DagEdgeList ^ get() { return parentList; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The number of non-OrderDep parents
   //
   //--------------------------------------------------------------------------

   property int NumRealParent
   {
      int get () { return numRealParent; }
      void set (int value) { numRealParent = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The children list of this node 
   //
   //--------------------------------------------------------------------------

   property DagEdgeList ^ ChildrenList
   {
      DagEdgeList ^ get () { return childrenList; }
      void set(DagEdgeList ^ value) { childrenList = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The list of src operands if the node is an expression node
   //
   // Remarks:
   // 
   //    The reason that we keep a separate src list other than the 
   //    ChildrenList is because ChildrenList is ordered by their priority so
   //    that we can evaluated them in an order that will benefit register
   //    allocation. However, for expression node, we need the original order
   //    of its src operands to regenerate code for it. 
   //
   //--------------------------------------------------------------------------

   property DagNodeList ^ SrcList
   {
      DagNodeList ^ get() { return srcList; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The kind of this dag node
   //
   //--------------------------------------------------------------------------

   property DagNodeKind Kind
   {
      DagNodeKind get () { return kind; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Some properties for querying kind of the DagNode
   //
   //--------------------------------------------------------------------------

   property bool IsDefNode
   {
      bool get () { return (kind & DagNodeKind::Definition) != (DagNodeKind)0;}
   }

   property bool IsUseNode
   {
      bool get () { return (kind & DagNodeKind::Use) != (DagNodeKind)0;}
   }

   property bool IsExprNode
   {
      bool get () { return (kind & DagNodeKind::Expression) != (DagNodeKind)0; }
   }

   property bool IsMemNode
   {
      bool get () { return (kind & DagNodeKind::Memory) != (DagNodeKind)0; }
   }

   property bool IsSimpleUseNode
   {
      bool get ()
      {
      return ((kind & DagNodeKind::Use) != (DagNodeKind)0
      && (kind & DagNodeKind::Memory) == (DagNodeKind)0);
      }
   }

   property bool IsSimpleDefNode
   {
      bool get ()
      {
      return ((kind & DagNodeKind::Definition) != (DagNodeKind)0
      && (kind & DagNodeKind::Memory) == (DagNodeKind)0);
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The Height of this node in the DAG. Leaf nodes have height 1. The 
   //    height of an interior node is the maximum height of all its children
   //    plus one. The height property is used as a hint of evaluation order. 
   //    We attempt to evaluate deeper subtree first to improve the quality of
   //    register usage.
   //
   //--------------------------------------------------------------------------

   property unsigned int Height
   {
      unsigned int get () { return height; }
      void set (unsigned int value) { height = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The new dst operand generated for this node.
   //
   // Remarks:
   //
   //    The NewDstOpnd of a DagNode is initialized to be same as the Label
   //    of the DagNode when it's created. It might be updated to something
   //    else during code generation
   //
   //--------------------------------------------------------------------------

   property Phx::IR::Operand ^ NewDstOpnd
   {
      Phx::IR::Operand ^ get () { return newDestinationOperand; }
      void set (Phx::IR::Operand ^ value) { newDestinationOperand = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The hashcode of this node
   //
   //--------------------------------------------------------------------------

   property int HashCode
   {
      int get () { return hashCode; }
      void set (int value) { hashCode = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates whether this node can be assigned to a temporary
   //
   // Remarks:
   //
   //    No assignment works for some types, eg. 4-bit value. For expression
   //    has such types, its value shouldn't be assigned to a temporary 
   //
   //--------------------------------------------------------------------------

   property bool NoTmp
   {
      bool get () { return noTmp; }
      void set (bool value) { noTmp = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates whether the node is visited or not in current pass
   //
   //--------------------------------------------------------------------------

   property int VisitPass
   {
      int get () { return visitPass; }
      void set (int value) { visitPass = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates whether new instructions are generated for the node   
   //
   //--------------------------------------------------------------------------

   property bool CodeGenerated
   {
      bool get () { return codeGenerated; }
      void set (bool value) { codeGenerated = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates whether instructions must be generated for the node   
   //
   //--------------------------------------------------------------------------

   property bool MustGen
   {
      bool get () { return mustGen; }
      void set (bool value) { mustGen = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates that the expression of this node has become invalid
   //    due to some changes in its subtree
   //
   //--------------------------------------------------------------------------

   property bool IsInvalid
   {
      bool get () { return isInvalid; }
      void set (bool value) { isInvalid = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates that the identifier of this node may have been 
   //    assigned a new value 
   //
   //--------------------------------------------------------------------------

   property bool IsKilled
   {
      bool get () { return isKilled; }
      void set (bool value) { isKilled = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates that this node is queried at least once
   //    Used to decide whether to generate code for an ExpressionTemporary
   //
   //--------------------------------------------------------------------------

   property bool IsQueried
   {
      bool get () { return isQueried; }
      void set (bool value) { isQueried = value; }
   }

   property bool IsQueriedExprTmp
   {
      bool get () { return (this->label->IsExpressionTemporary && this->isQueried); }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates that this node is killed by an operand with same
   //    name. This is used for safe dead store elimination
   //
   //--------------------------------------------------------------------------

   property bool IsKilledBySelf
   {
      bool get () { return isKilledBySelf; }
      void set (bool value) { isKilledBySelf = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates that the node represents a code target
   //--------------------------------------------------------------------------

   property bool IsCallTarget
   {
      bool get () { return isCallTarget; }
      void set (bool value) { isCallTarget = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates whether this node is a root or not   
   //
   //--------------------------------------------------------------------------

   property bool IsRoot
   {
      bool get () { return isRoot; }
      void set (bool value) { isRoot = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Check whether a dag node can be used as the dst operand for its
   //    target node, i.e., all of its children exception the target node 
   //    are processed
   //
   //--------------------------------------------------------------------------

   property bool IsDstCandidate
   {
      bool get () ;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The target value an identifier node (Definition) points to
   //
   //--------------------------------------------------------------------------

   property DagNode ^ TargetNode
   {
      DagNode ^ get () { return targetNode; }
      void set (DagNode ^ value) { targetNode = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The base/index/segment node of a memory operand node
   //
   //--------------------------------------------------------------------------

   property DagNode ^ BaseNode
   {
      DagNode ^ get () { return baseNode; }
      void set (DagNode ^ value) { baseNode = value; }
   }

   property DagNode ^ IndexNode
   {
      DagNode ^ get () { return indexNode; }
      void set (DagNode ^ value) { indexNode = value; }
   }

   property DagNode ^ SegNode
   {
      DagNode ^ get () { return segNode; }
      void set (DagNode ^ value) { segNode = value; }
   }

private:

   Phx::IR::Operand ^ label;

   DagEdgeList ^ parentList;

   int numRealParent;

   DagEdgeList ^ childrenList;

   DagNodeList ^ srcList;

   DagNodeKind kind;

   unsigned int height;

   Phx::IR::Operand ^ newDestinationOperand;

   int hashCode;

   bool noTmp;

   int visitPass;

   bool codeGenerated;

   bool mustGen;

   bool isInvalid;

   bool isKilled;

   bool isQueried;

   bool isKilledBySelf;

   bool isCallTarget;

   bool isRoot;

   DagNode ^ targetNode;

   DagNode ^ baseNode;

   DagNode ^ indexNode;

   DagNode ^ segNode;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    An edge in the DAG
//
//-----------------------------------------------------------------------------

public ref class DagEdge : System::IComparable<DagEdge ^>,
   System::IEquatable<DagEdge ^>
{

public:

   static DagEdge ^
   New
   (
      DagNode ^   from,
      DagNode ^   to,
      DagEdgeKind edgeKind
   );

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The start node of this edge
   //
   //--------------------------------------------------------------------------

   property DagNode ^ FromNode
   {
      DagNode ^ get () { return fromNode; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The end node of this edge
   //
   //--------------------------------------------------------------------------

   property DagNode ^ ToNode
   {
      DagNode ^ get () { return toNode; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The kind of dependency relationship this edge represents 
   //
   //--------------------------------------------------------------------------

   property DagEdgeKind Kind
   {
      DagEdgeKind get () { return kind; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The priority of this edge to be evaluated
   //
   // Remarks:
   //
   //    The orderDep edge has the highest priority to be evaluated. The 
   //    assignDep edge has the lowest priority to be evaluated. Value edges
   //    are sorted by the height of the toNode of this edge, deeper children
   //    are evaluated earlier so that registers can be used more efficiently.
   //
   //--------------------------------------------------------------------------

   property unsigned int Priority
   {
      unsigned int get () { return priority; }
   }

   // Implement interface System.IComparable<DagEdge>

   virtual int CompareTo (DagEdge ^ that) ;

   virtual bool Equals (DagEdge ^ that) ;

private:

   DagNode ^ fromNode;

   DagNode ^ toNode;

   DagEdgeKind kind;

   unsigned int priority;

};

//-----------------------------------------------------------------------------
//
// Description:
//
//    A wrapper of operand when it is used as a key into a hash map or a list.
//
//-----------------------------------------------------------------------------

public ref class OpndKey : System::IEquatable<OpndKey ^>
{

public:

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The static constructor
   //
   // Arguments:
   //
   //    operand - The operand in the key
   //    hashcode - The hash code of the key
   //
   // Returns:
   //
   //    The new instrance of OpndKey
   //
   //--------------------------------------------------------------------------

   static OpndKey ^
   New
   (
      Phx::IR::Operand ^ operand,
      int             hashCode
   )
   {
      OpndKey ^ newKey = gcnew OpndKey();

      newKey->operand = operand;
      newKey->hashCode = hashCode;

      return newKey;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The operand this key represents   
   //
   //--------------------------------------------------------------------------

   property Phx::IR::Operand ^ Operand
   {
      Phx::IR::Operand ^ get () { return operand; }
      void set (Phx::IR::Operand ^ value) { operand = value; }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Override GetHashCode() so that it returns the hashCode we generated
   //
   // Returns:
   //
   //    The hashcode of this key
   //
   //--------------------------------------------------------------------------

   virtual int GetHashCode () override { return hashCode; }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Implement interface System.IEquable<OpndKey>
   //
   //--------------------------------------------------------------------------

   virtual bool 
   Equals
   (
      OpndKey ^ that
   )
   {
      if (this->GetHashCode() != that->GetHashCode())
      {
         return false;
      }
      Phx::IR::Operand ^ opndA = this->Operand;
      Phx::IR::Operand ^ opndB = that->Operand;
      return DAG::CompareOperand(opndA, opndB);
   }

private:

   Phx::IR::Operand ^ operand;

   int hashCode;

};

//----------------------------------------------------------------------------
//
// Description:
//
//    A key of the node hash map. This class handles common subexpression 
//    recognization
//
//-----------------------------------------------------------------------------

public ref class NodeKey : System::IEquatable<NodeKey ^>
{

public:

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The static constructor  
   //
   // Arguments:
   //
   //    node - The dag node this key represents
   //    hashCode - The hashCode of the key
   //
   // Returns:
   //
   //    The new instance of NodeKey created
   //
   //--------------------------------------------------------------------------

   static NodeKey ^ 
   New
   (
      DagNode ^ node,
      int       hashCode
   )
   {
      NodeKey ^ newKey = gcnew NodeKey();

      newKey->node = node;
      newKey->hashCode = hashCode;

      return newKey;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Override GetHashCode() so that it returns the hashCode we generated
   //
   // Returns:
   //
   //    The hashcode of this key
   //
   //--------------------------------------------------------------------------


   virtual int GetHashCode () override
   {
   return hashCode;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Implement interface System.IEquable<LeaderOpndKey>
   //    The core methods that supports common expression recognization
   //
   //--------------------------------------------------------------------------

   virtual bool Equals (NodeKey ^ that);

private:

   int hashCode;

   // The node this key represents 

   DagNode ^ node;
};

} // namespace LocalOpt
} // namespace Samples
} // namespace Phx

