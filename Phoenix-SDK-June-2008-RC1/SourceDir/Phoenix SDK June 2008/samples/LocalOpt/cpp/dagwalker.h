//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    The DagNode Walker implementation for the local optimization plug in.
//    This file contains the major body of code generation process.
//
//-----------------------------------------------------------------------------

#pragma once

namespace Phx
{

namespace Samples
{

namespace LocalOpt
{

// forward declaration;

ref class DagEdge;
ref class DagNode;

// Use more readable names for the generics classes

typedef System::Collections::Generic::List<DagEdge ^> DagEdgeList;
typedef System::Collections::Generic::List<DagNode ^> DagNodeList;

//-----------------------------------------------------------------------------
//
// Description:
//
//    Controls that decide the next action of the DagWalker
//
//-----------------------------------------------------------------------------

public enum class WalkControl
{
   IllegalSentinel = 0,           // Illegal context.
   Continue,               // Continue this tree walk.
   Skip,                   // Skip the subtree.
   Stop                    // Stop this tree walk.
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    A Dep-first walker for a given DagNode
//
//-----------------------------------------------------------------------------

public ref class DagWalker
{
public: 
   
   WalkControl 
   Walk 
   (
      DagNode ^ dagNode
   ) ;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A static field used to distinguish node is visited by the current pass
   //    from those haven't
   //
   // Remarks:
   // 
   //    We assume the plug-in is used by only one thread at a time. If the 
   //    plug-in is used in a re-entrant style, we need to change this
   //    implementation
   //
   //--------------------------------------------------------------------------

   property int CurrentPass 
   {
      static int get () ;
      static void set (int value) ;
   }

protected: 
   
   virtual WalkControl 
   PreAction 
   (
      DagNode ^ dagNode
   );

   virtual WalkControl 
   PostAction 
   (
      DagNode ^ dagNode
   ) ;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag set by clients indicating whether preAction is required
   //
   //--------------------------------------------------------------------------

   property bool DoPreAction 
   {
      void set (bool value) ;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag set by clients indicating whether postAction is required
   //
   //--------------------------------------------------------------------------

   property bool DoPostAction 
   {
      void set (bool value) ;
   }

private: 
   
   bool doPreAction;

   bool doPostAction;

   static int currentPass = 0;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    A derived class from DagWalker, regenerating code for a given DagNode
//    including all nodes in its subtree
//
//-----------------------------------------------------------------------------

public ref class GenCodeWalker : DagWalker
{

public: 
   
   static GenCodeWalker ^ 
   New 
   (
      Phx::FunctionUnit ^  functionUnit, 
      DAG ^            theDag,
      Phx::IR::Instruction ^ baseInstruction
   ) ;

protected: 
   
   virtual WalkControl 
   PreAction 
   (
      DagNode ^ dagNode
   ) override ;

   virtual WalkControl 
   PostAction 
   (
      DagNode ^ dagNode
   ) override ;

private: 
   
   WalkControl 
   GenCodeForExprNode 
   (
      DagNode ^ dagNode
   ) ;

   WalkControl 
   GenCodeForUseNode 
   (
      DagNode ^ dagNode
   ) ;

   WalkControl 
   GenCodeForDefNode 
   (
      DagNode ^ dagNode
   ) ;

   Phx::IR::Instruction ^ 
   GenInstr 
   (
      DagNode ^ leftHand,
      DagNode ^ rightHand
   );

   int 
   CountRealParent 
   (
      DagNode ^                                           dagNode,
      [System::Runtime::InteropServices::Out()]DagNode ^% goodDstCandidate
   );

private: 
   
   void LinkInstr (Phx::IR::Instruction ^ newInstruction);

private:

   // The base instruction to insert new instruction before it

   Phx::IR::Instruction ^ baseInstruction;

   Phx::FunctionUnit ^ functionUnit;

   DAG ^ theDag;
};

} // namespace LocalOpt
} // namespace Samples
} // namespace Phx

