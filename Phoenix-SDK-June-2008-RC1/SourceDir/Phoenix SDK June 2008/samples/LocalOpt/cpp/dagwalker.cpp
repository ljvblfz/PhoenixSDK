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

#include "dag.h"
#include "dagwalker.h"

namespace Phx
{

namespace Samples
{

namespace LocalOpt
{

//--------------------------------------------------------------------------
//
// Description:
//
//    Recursive implementation of a depth-first traverse on the given tree 
//
// Arguments:
//
//    dagNode - The dag node to walk
//
// Returns:
//
//    The walk control that indicates the result of the current walk pass
//
// Remarks:
//
//    Recursive implementation is easier to understand. However, stack
//    overflow is possible. Currently it seems OK for the benchmarks we
//    tested. If the stack overflow does happen, it has to be re-implemented
//
//--------------------------------------------------------------------------

WalkControl 
DagWalker::Walk
(
   DagNode ^ dagNode
)
{
#if defined(PHX_DEBUG_CHECKS)

   Phx::Asserts::Assert(dagNode != nullptr, "DagWalker.Walk, dagNode!=null");

#endif

   // Initialize the return state to be Continue

   WalkControl ret1 = WalkControl::Continue;

   // do PreAction if it's required 

   if (this->doPreAction)
   {
      ret1 = this->PreAction(dagNode);

      if (ret1 == WalkControl::Stop)
      {
         // something bad happened

         return ret1;
      }
   }

   // To improve the quality of generated code, we evaluate children edges
   // in the order of their prioirty. The orderDep edges have the highest 
   // priority to be evaluated since the current node must be evaluated
   // after those children node. The assignDep edges have the lowest 
   // priority since if the current node is a memory node, assignment must
   // be evaluated after all of its base/index/segment nodes are evaluated.
   // Value edges are sorted by the height of the toNode of this edge, deeper 
   // children are evaluated earlier so that better register usage can be
   // achieved.

   // Sort the ChildrenList based on their priority value

   dagNode->ChildrenList->Sort();

   // Evaluate each node in the sorted Children List

   for each (DagEdge ^ edge in dagNode->ChildrenList)
   {
      DagNode ^ nextChild = edge->ToNode;

      if ((ret1 == WalkControl::Skip) && (edge->Kind != DagEdgeKind::OrderDep))
      {
         // The current node is skipped, we don't need to process the whole
         // expression subtree. But we still need to evaluate children with
         // orderDep edges

         continue;
      }

      if (nextChild->VisitPass == currentPass)
      {
         // has been handled by other tree

         continue;
      }

      WalkControl ret2 = this->Walk(nextChild);

      if (ret2 == WalkControl::Stop)
      {
         // something bad happened

         return ret2;
      }
   }

   // Perform postAction if required

   if ((ret1 != WalkControl::Skip) && this->doPostAction)
   {
      ret1 = this->PostAction(dagNode);

      if (ret1 != WalkControl::Continue)
      {
         // something bad happened

         return ret1;
      }
   }

   // Update the visit pass of the dag node

   dagNode->VisitPass = currentPass;

   return WalkControl::Continue;

}

//--------------------------------------------------------------------------
//
// Description:
//
//    Actions will be performed before a node is processed
//
// Arguments:
//
//    dagNode - The current dag node
//
// Returns:
//
//    The walk control that indicates the result of the preAction
//
//--------------------------------------------------------------------------

WalkControl 
DagWalker::PreAction
(
   DagNode ^ dagNode
)
{
   // Not implemented in the base class

   return WalkControl::IllegalSentinel;
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Actions will be performed after a node is processed
//
// Arguments:
//
//    dagNode - The current dag node
//
// Returns:
//
//    The walk control that indicates the result of the postAction
//
//--------------------------------------------------------------------------

WalkControl 
DagWalker::PostAction
(
   DagNode ^ dagNode
)
{
   // Not implemented in the base class

   return WalkControl::IllegalSentinel;
}

//--------------------------------------------------------------------------
//
// Description:
//
//    A flag set by clients indicating whether preAction is required
//
//--------------------------------------------------------------------------

void 
DagWalker::DoPreAction::set
(
   bool value
)
{
   doPreAction = value;
}

//--------------------------------------------------------------------------
//
// Description:
//
//    A flag set by clients indicating whether postAction is required
//
//--------------------------------------------------------------------------

void 
DagWalker::DoPostAction::set
(
   bool value
)
{
   doPostAction = value;
}

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

int DagWalker::CurrentPass::get ()
{
   return currentPass;
}

void 
DagWalker::CurrentPass::set
(
   int value
)
{
   currentPass = value;
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Static constructor for the code generator walker
//
// Arguments:
//
//    functionUnit - The function unit that this node belongs to
//    theDag - The DAG constructed so far
//    baseInstruction - The base instruction before which new instructions will
//                be inserted
//
// Returns:
//
//    GenCodeWalker object.
//
//--------------------------------------------------------------------------

GenCodeWalker ^ 
GenCodeWalker::New
(
   Phx::FunctionUnit ^  functionUnit,
   DAG ^            theDag,
   Phx::IR::Instruction ^ baseInstruction
)
{
   GenCodeWalker ^ newWalker = gcnew GenCodeWalker();

   // Both PreAction and PostAction are required in this walker

   newWalker->DoPostAction = true;
   newWalker->DoPreAction = true;

   newWalker->functionUnit = functionUnit;
   newWalker->theDag = theDag;
   newWalker->baseInstruction = baseInstruction;

   CurrentPass++;

   return newWalker;
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Action taken before evaluating the dag node, used to do initialization
//    and decide whether we can skip to evaluate certain node if possible
//
// Arguments:
//
//    dagNode - The dag node to be evaluated
//
// Returns:
//
//    WalkControl.Skip if can be skipped, otherwise WalkControl.Continue
//
//--------------------------------------------------------------------------

WalkControl 
GenCodeWalker::PreAction
(
   DagNode ^ dagNode
)
{
   if (dagNode->CodeGenerated)
   {
      // the given node is already processed previously

      return WalkControl::Skip;
   }

   if (dagNode->MustGen)
   {
      // we always process node with MustGen flag set

      return WalkControl::Continue;
   }

   if (dagNode->IsDefNode)
   {
      if (dagNode->IsQueriedExprTmp)
      {
         // An exprTmp definition, skip it

         // We don't need to generate code for most of the ExprTmps since
         // they are not live out of the block. But since we have to stop
         // the DAG process for some special instructions, it's possible
         // that the ExpressionTemporary is used after the DAG is built. For example,
         //      t1 = x
         //      x  = 2
         //      t2 = CMP t1, 0...
         // The CMP instruction will stop the DAG consruction, while t1
         // is still dangling. Therefore, for an ExpressionTemporary, if it is never
         // queried, we still need to generate code for its subtress

         dagNode->CodeGenerated = true;
         return WalkControl::Skip;
      }

      if (dagNode->NumRealParent == 0)
      {
         // A definition node with no real parents

         // We use KilledBySelf to indicate a dead store candidate

         if (dagNode->IsKilledBySelf)
         {
            // a dead store

            dagNode->CodeGenerated = true;

            // We can't skip the whole subtree since some nodes in this tree 
            // become non-root only because of the evaluation order edges.

            return WalkControl::Continue;
         }
      }
   }

   if (dagNode->IsSimpleUseNode && (dagNode->Label->IsImmediateOperand
         || (dagNode->NumRealParent <= 1)))
   {
      // a simple leaf node that is either a constant or with only 
      // one parent, not necessary to generate code for it, skip the node.

      dagNode->CodeGenerated = true;
      return WalkControl::Skip;
   }

   return WalkControl::Continue;
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Action taken after the dag node is popped from the evaluation stack,
//    used to generate code for this node
//
// Arguments:
//
//    dagNode - The dag node to be evaluated
//
// Returns:
//
//    WalkControl.Stop if serious problem happens, otherwise WalkControl.Continue
//
//--------------------------------------------------------------------------

WalkControl 
GenCodeWalker::PostAction
(
   DagNode ^ dagNode
)
{
   if (dagNode->CodeGenerated)
   {
      // already processed previously

      return WalkControl::Continue;
   }

   if (dagNode->IsExprNode)
   {
      // An expression node

      return this->GenCodeForExprNode(dagNode);
   }

   if (dagNode->IsMemNode)
   {
      // A memory node, resolve the base/index/segment operands first

      theDag->ResolveMemNode(dagNode);
   }

   if (dagNode->IsDefNode)
   {
      // A definition node

      return this->GenCodeForDefNode(dagNode);
   }
   else
   {
      // An use node

      return this->GenCodeForUseNode(dagNode);
   }
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Generate code for an expression node. A new instruction is generated 
//    using the original opcode of the expression node. The dst operand of
//    the new instruction is either chosen from the AssignDep parents if
//    possible or a new generated temporary.
//
// Arguments:
//
//    dagNode - The expression node to be evaluated
//
// Returns:
//
//    WalkControl.Stop if serious problem happens, otherwise WalkControl.Continue
//
//--------------------------------------------------------------------------

WalkControl 
GenCodeWalker::GenCodeForExprNode
(
   DagNode ^ dagNode
)
{
#if defined(PHX_DEBUG_CHECKS)

   Phx::Asserts::Assert(dagNode->IsExprNode);

#endif

   if (dagNode->NoTmp)
   {
      // For some special insructions, no temporary could be its dst operand.
      // It will be handled when processing the definition node represents
      // its original dst operand.

      dagNode->CodeGenerated = true;

      return WalkControl::Continue;
   }

   // Generate a new instruction for it

   Phx::IR::Instruction ^ newInstruction = this->GenInstr(nullptr, dagNode);

   // It is possible that GenInstr() decides no instruction needs to 
   // be generated since it's a useless expression

   if (newInstruction != nullptr)
   {
      // A new instruction is generated, link it to the new instruction
      // list

      this->LinkInstr(newInstruction);
   }

   dagNode->CodeGenerated = true;

   return WalkControl::Continue;
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Generate an assignment instruction code for an use node. There are
//    two cases when this is necessary: 
//
//       (1) The use node is a memory operand and it's used as the base
//           operand of another memory operand. In this case, it must be
//           assigned to a temporary first. The MustGen flag should already
//           be set while constructing the DAG in this case.
//       (2) The use node is used more than one time and it's registable.
//           In this case, it might improve the code quality by assigning it
//           to a temporary first.
//
// Arguments:
//
//    dagNode - The use node to be evaluated
//
// Returns:
//
//    WalkControl.Stop if serious problem happens, otherwise WalkControl.Continue
//
//--------------------------------------------------------------------------

WalkControl 
GenCodeWalker::GenCodeForUseNode
(
   DagNode ^ dagNode
)
{
   // Right now, Phoenix does not support temporaries of some types and 
   // sizes in Hir and MIR. For example, The code generator cannot handle 
   // unenregisterable aggregate temporaries. For such types of operands,
   // they should not be assigned to a temporary. This will be fixed soon.

   if (dagNode->NoTmp)
   {
      // The dag node should not be assigned to a temporary. Simply return.
      // Code will be generated when processing its Definition node.

      return WalkControl::Continue;
   }

   // There might be some of its parents become dead store, or not necessary
   // to generate code any more. Recount the number of real parents. As a 
   // side-effect, this method also return a good dst candidate if it exists

   DagNode ^ goodDstCandidate;

   int numRealParent = this->CountRealParent(dagNode, goodDstCandidate);

   if (!dagNode->MustGen && (numRealParent < 2))
   {
      // Code generation is not enforced for this dag node and it does not
      // have benefit to assign it to a temporary first, simply return.

      dagNode->CodeGenerated = true;
      return WalkControl::Continue;
   }

   Phx::IR::Operand ^ newDestinationOperand;
   if (goodDstCandidate != nullptr)
   {
      newDestinationOperand = goodDstCandidate->NewDstOpnd;
      goodDstCandidate->CodeGenerated = true;
   }
   else
   {
      // We didn't find a good dst operand for it, create a new temporary

      newDestinationOperand = Phx::IR::VariableOperand::NewTemporary(this->functionUnit,
         dagNode->NewDstOpnd->Type);
   }

   // Generate a new assignment instruction

   Phx::IR::Instruction ^ newAssignInstr = Phx::IR::ValueInstruction::NewUnary(this->functionUnit,
      Common::Opcode::Assign, newDestinationOperand, dagNode->NewDstOpnd);

   // Update the NewDstOpnd of the given dag node unless it's a constant

   if (!dagNode->NewDstOpnd->IsImmediateOperand)
   {
      dagNode->NewDstOpnd = newDestinationOperand;
   }

   // Link the new instruction to the instruction list we are genrating

   this->LinkInstr(newAssignInstr);

   dagNode->CodeGenerated = true;

   return WalkControl::Continue;
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Generate code for an definition node. Usually an assignment is generated
//    between the given definition node and its target node. If the target node
//    represents some special instructions that couldn't be assigned to
//    a temporary, that special instruction is generated instead of an
//    normal assignment.
//
// Arguments:
//
//    dagNode - The definition node to be evaluated
//
// Returns:
//
//    WalkControl.Stop if serious problem happens, otherwise WalkControl.Continue
//
//--------------------------------------------------------------------------

WalkControl 
GenCodeWalker::GenCodeForDefNode
(
   DagNode ^ dagNode
)
{
#if defined(PHX_DEBUG_CHECKS)

   Phx::Asserts::Assert(dagNode->IsDefNode);
   Phx::Asserts::Assert(dagNode->TargetNode != nullptr,
      "GenCodeWalker.PostAction, dagNode.TargetNode != null");

#endif

   Phx::IR::Instruction ^ newInstruction = nullptr;

   DagNode ^        targetNode = dagNode->TargetNode;

   if (targetNode->IsExprNode && targetNode->NoTmp)
   {
      // No instruction was generated for the target node. Generate one
      // using the current definition node as l-value and the targetNode as r-value

      newInstruction = this->GenInstr(dagNode, targetNode);
   }
   else
   {
       // Generate a new assignment instruction

      newInstruction = Phx::IR::ValueInstruction::NewUnary(this->functionUnit,
         Common::Opcode::Assign, dagNode->NewDstOpnd, targetNode->NewDstOpnd);

      Phx::IR::Operand ^ newDestinationOperand = newInstruction->DestinationOperand;
      Phx::IR::Operand ^ newSourceOperand = newInstruction->SourceOperand;

      // It's possible that we merge two operands that have same names but
      // different types or fields to one dag node while constructing DAG
      // as long as they are compatible. So it's possible that several Definition
      // nodes with different types/fields point to a same target node.
      // While generating code from DAG, we need to set correct type/field
      // for both dst operands and src operands. One heuristic we take is
      // to set the type/field of the src operand using dst operand's type
      // and field if the types of two operands are not compatible 

      if (!newDestinationOperand->Type->IsAssignableFrom(newSourceOperand->Type))
      {
         newSourceOperand->Type = newDestinationOperand->Type;

         newSourceOperand->Field = newSourceOperand->Field->GetField(newDestinationOperand->Type,
            newSourceOperand->Field->BitOffset);
      }

      if (!newSourceOperand->IsImmediateOperand && !newSourceOperand->IsTemporary && newDestinationOperand->IsTemporary)
      {
         // It's a better candidate as the dst operand of its target node

         targetNode->NewDstOpnd = newDestinationOperand;
      }
   }

   this->LinkInstr(newInstruction);

   dagNode->CodeGenerated = true;

   if (!dagNode->MustGen)
   {
      return WalkControl::Continue;
   }
   else
   {
      // An instruction must be generated using the current node as RHS.
      // This usually happens when this node is a memory operand, and it's
      // used as a base operand of another memory operand.

      return this->GenCodeForUseNode(dagNode);
   }
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Generate one instruction given dag nodes represents its left-hand-side
//    and right-hand-side.
//
// Arguments:
//
//    leftHand - The dag node that represents the dst operand of the new 
//             instruction. If it's null, the dst operand is chosen from
//             right-hand-side node's parent list or a new temporary is 
//             created.
//
//    rightHang - The expression node that represents the righ-hand-side 
//             of the new instruction.
//
// Returns:
//
//    WalkControl.Stop if serious problem happens, otherwise WalkControl.Continue
//
//--------------------------------------------------------------------------

Phx::IR::Instruction ^ 
GenCodeWalker::GenInstr
(
   DagNode ^ leftHand,
   DagNode ^ rightHand
)
{
   // Get the original instruction for reference

   Phx::IR::Instruction ^ oldInstruction = rightHand->NewDstOpnd->Instruction;

   // Decide whether we need to generate code for this expression node
   // and if so, try to choose a good dstOperand for it 

   DagNode ^ goodDstNode;

   int numRealParent = this->CountRealParent(rightHand, goodDstNode);

   // If the given expression node has no real parent, i.e., it becomes
   // a dead expression, try to eliminate it if this optimization is on.

   if ((numRealParent == 0) && !rightHand->MustGen
      && rightHand->Label->Instruction->IsValueInstruction)
   {
      // A useless expression, no instruction needs to be generated

      // Decrease the numRealParent of its children and return null

      for each (DagEdge ^ edge in rightHand->ChildrenList)
      {
         if (edge->Kind != DagEdgeKind::ValueDep)
         {
            continue;
         }

         edge->ToNode->NumRealParent--;
      }

      return nullptr;
   }

   // Create a new instruction using the opcode of the old instruction

   Phx::IR::Instruction ^ newInstruction = oldInstruction->Copy();

   // Set the correct debug tag

   newInstruction->DebugTag = oldInstruction->DebugTag;

   // Generate src operands for the new instruction based on the list of
   // src dag node in the expression node.

   for each (DagNode ^ nextSrc in rightHand->SrcList)
   {
      Phx::IR::Operand ^ newSourceOperand = nextSrc->NewDstOpnd;
      if (nextSrc->Label->IsLabelOperand)
      {
         // Label operands require a special append interface.

         newInstruction->AppendLabelSource(Phx::IR::LabelOperandKind::Handler,
            newSourceOperand->Copy()->AsLabelOperand);
      }
      else if (newInstruction->IsCallInstruction && nextSrc->IsCallTarget)
      {
         // Code target operands require a special care too.

         if (oldInstruction->AsCallInstruction->CallTargetOperand->IsMemoryOperand)
         {
            // Update the base/index/segment operands if necessary

            theDag->ResolveMemNode(nextSrc);
            newSourceOperand = nextSrc->NewDstOpnd;
         }

         newInstruction->AsCallInstruction->CallTargetOperand = newSourceOperand;

         if (!newInstruction->AsCallInstruction->CallTargetOperand->IsFunctionOperand)
         {
            // If the call is an indirect call, we need to set the type
            // of the new CallTargetOperand to be the old CallTargetOperand's
            // type so that FunctionType of the call instruction is setup
            // correctly.

            newInstruction->AsCallInstruction->CallTargetOperand->Type = 
               oldInstruction->AsCallInstruction->CallTargetOperand->Type;
         }
      }
      else
      {
         // Normal src operand, append the operands in sequential order.

         newInstruction->AppendSource(newSourceOperand);
      }
   }

   //Pick a good explicit dst operand and append those implicit ones

   for each (Phx::IR::Operand ^ dstOperand in oldInstruction->DestinationOperands)
   {
      if (dstOperand->IsExplicit)
      {
         Phx::IR::Operand ^ newDestinationOperand = nullptr;

         if (leftHand != nullptr)
         {
            // If a dst operand is specified, use it

            if (leftHand->IsMemNode)
            {
               theDag->ResolveMemNode(leftHand);
            }

            newDestinationOperand = leftHand->NewDstOpnd;
         }
         else if ((goodDstNode != nullptr) && (goodDstNode->Label->IsTemporary
               || (numRealParent < 2)))
         {
            // We found a good candidate in the previous search process
            // and it's either a temporary in which case we can use it 
            // directly or it is the only definition node of this expression.
            // In both cases, this candidate can be used as dst operand

            newDestinationOperand = goodDstNode->NewDstOpnd;
            goodDstNode->CodeGenerated = true;
         }
         else
         {
            // we didn't found a good dstOperand or have more than one
            // definition nodes, create a temporary to avoid reload

            newDestinationOperand = Phx::IR::VariableOperand::NewTemporary(this->functionUnit,
               rightHand->NewDstOpnd->Type);
         }

         rightHand->NewDstOpnd = newDestinationOperand;

         newInstruction->AppendDestination(newDestinationOperand);
      }
      else
      {
         // For implicit dst operands, we simply append them to the new
         // instruction one by one.

         newInstruction->AppendDestination(dstOperand->Copy());
      }
   }

   return newInstruction;
}

//--------------------------------------------------------------------------
//
// Description:
//
//    For a dag node, it's possible that some of its parents become dead 
//    store, or not necessary to generate code any more. This method check 
//    the parent list and recount the numRealParent property. As a side
//    effect, a good candidate of dst operand is picked if possible
//
// Arguments:
//
//    In:  dagNode - The dag node to be evaluated
//    Out: goodDstCandidate - A parent node that is a good candidate to be
//          used as the dst operand for the current node
//
// Returns:
//
//    The current number of "real" parent
//
//--------------------------------------------------------------------------

int 
GenCodeWalker::CountRealParent
(
   DagNode ^                                           dagNode,
   [System::Runtime::InteropServices::Out()]DagNode ^% goodDstCandidate
)
{
   // Assume we don't have any good dst candidate

   goodDstCandidate = nullptr;

   int                 numRealParent = dagNode->NumRealParent;
   for each (DagEdge ^ edge in dagNode->ParentList)
   {
      if (edge->Kind != DagEdgeKind::AssignDep)
      {
         // Only parents with AssignDep are candidate dst operand

         continue;
      }

      DagNode ^ parent = edge->FromNode;

      if (parent->IsQueriedExprTmp || ((parent->NumRealParent == 0)
            && parent->IsKilledBySelf))
      {
         // parent is a dead store or an ExpressionTemporary and has use in the dag

         numRealParent--;
         parent->CodeGenerated = true;
      }

      if (parent->Label->IsTemporary && !parent->IsQueriedExprTmp)
      {
         // We favor temporary variables as new dst operand since we are not
         // sure whether they are live out of the block and have to
         // generate assignment for them anyway. Therefore, if we find
         // a parent with temporary variable as its label, we've found our
         // good dst candidate

         goodDstCandidate = parent;
      }

      if (dagNode->IsExprNode)
      {
         // For use nodes, it's not a must to pick a dst node for it if
         // it's not beneficial to assign it to something else first because
         // it always can be used directly. However, for expression node,
         // We always need to pick a dst operand for it. Therefore, we need
         // to put more effort to find a good candidate dst operand here.

         if (!parent->IsDstCandidate)
         {
            // Not all of this definition node's children are ready

            continue;
         }

         if (parent->IsMemNode)
         {

            theDag->ResolveMemNode(parent);
         }

         if (goodDstCandidate == nullptr)
         {
            // The first good dst node so far, cache it.

            goodDstCandidate = parent;
         }
         else if (goodDstCandidate->IsMemNode && !parent->IsMemNode)
         {
            // We prefer dst operand that is not MemoryOperand

            goodDstCandidate = parent;
         }
      }
   }

   return numRealParent;
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Insert the newly generated instruction into the instruction list
//
// Arguments:
//
//    newInstruction - The new instruction generated
//
// Returns:
//
//    Nothing.
//
//--------------------------------------------------------------------------

void 
GenCodeWalker::LinkInstr
(
   Phx::IR::Instruction ^ newInstruction
)
{
   baseInstruction->InsertBefore(newInstruction);
}

} // namespace LocalOpt
} // namespace Samples
} // namespace Phx
