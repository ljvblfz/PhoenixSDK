//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    The DAG implementation for the local optimization plug in
//
//-----------------------------------------------------------------------------


namespace Phx
{

namespace Samples
{

namespace LocalOpt
{

// Use more readable names for the generics classes

using DagEdgeList = System.Collections.Generic.List<DagEdge>;
using DagNodeList = System.Collections.Generic.List<DagNode>;

using OpndToOpndMap = Hashtable<OpndKey, Phx.IR.Operand>;
using OpndToNodeMap = Hashtable<OpndKey, DagNode>;
using ExprToNodeMap = Hashtable<NodeKey, DagNode>;
using TagToDefNodeMap = Hashtable<int, DagNode>;

using TagToUseNodeMap = 
   Hashtable<int, System.Collections.Generic.List<DagNode>>;

//-----------------------------------------------------------------------------
//
// Description:
//
//    The data structure handles constructions of DAGs and queries of DagNode
//
//-----------------------------------------------------------------------------

public class DAG
{

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Static constructor for a DAG.
   //
   // Arguments:
   //
   //    functionUnit - The function unit in current context
   //    tableSize - The initial size of all hashtables in DAG
   //
   // Returns:
   //
   //    DAG object.
   //
   //--------------------------------------------------------------------------

   public static DAG
   New
   (
      Phx.FunctionUnit functionUnit,
      int tableSize
   )
   {
      DAG dag = new DAG();
      dag.functionUnit = functionUnit;
      dag.aliasInfo = functionUnit.AliasInfo;

      // Initialize members of a DAG

      dag.rootList = new DagNodeList();
      
      dag.exprToNodeMap = new ExprToNodeMap(tableSize);
      
      dag.opndToNodeMap = new OpndToNodeMap(tableSize);

      dag.tagToUseNodeMap = new TagToUseNodeMap(functionUnit.AliasInfo.NumberLocationTags);

      dag.tagToDefNodeMap = new TagToDefNodeMap(functionUnit.AliasInfo.NumberLocationTags);

      return dag;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Lookup the DagNode for the given operand in the opndToNodeMap. If the
   //    result node is a definition node, the target node that the definition node points
   //    to will be returned to enable copy propogation if possible.
   //
   // Arguments:
   //
   //    operand - The operand to look up
   //    hashCode - The hash code of the node to search or create
   //
   // Returns:
   //
   //    The DagNode if found, null otherwise
   //
   //--------------------------------------------------------------------------

   public DagNode
   LookupOpnd
   (
      Phx.IR.Operand operand,
      int hashCode
   )
   {
      // Lookup in the opndToNodeMap

      DagNode node = this.opndToNodeMap.Lookup(OpndKey.New(operand, hashCode));
      
      if (node == null || node.IsInvalid)
      {
         // No node is found or the result node is already invalidated
         return null;
      }

      // Some operands may be defined in the given instruction range, but never
      // be used. For a temporary, it's possible that they will be used out of
      // the range, in which case we should generate code for it even it's an 
      // ExpressionTemporary, to guarantee correctness. We use whether it's ever queried or 
      // not to indicate that.

      node.IsQueried = true;

      if (operand.IsMemoryOperand && !Phx.IR.Operand.Compare(operand, node.Label))
      {
         // It's possible that two memory operands like t[109] and t[110] are
         // recognized as same operands by the Expression.Table.Compare() directly. We
         // should flag its base/index/segment nodes (e.g., t110 here) as queried 
         // to avoid redundent assignments.

         if (operand.BaseOperand != null)
         {
            DagNode baseNode = this.opndToNodeMap.Lookup(
               OpndKey.New(operand.BaseOperand, GetHashCode(operand.BaseOperand)));
            
            if(baseNode != null)
            {
               baseNode.IsQueried = true;
            }
         }
         if (operand.IndexOperand != null)
         {
            DagNode indexNode = this.opndToNodeMap.Lookup(
               OpndKey.New(operand.IndexOperand, GetHashCode(operand.IndexOperand)));
            
            if(indexNode != null)
            {
               indexNode.IsQueried = true;
            }
         }
         if (operand.SegmentOperand != null)
         {
            DagNode segNode = this.opndToNodeMap.Lookup(
               OpndKey.New(operand.SegmentOperand, GetHashCode(operand.SegmentOperand)));
            
            if(segNode != null)
            {
               segNode.IsQueried = true;
            }
         }
      }

      // If the node has target node, we return the target node if safe.

      if (node.TargetNode != null)
      {
         return this.ResolveTarget(node);
      }
      
      return node;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Find or create the dag node for the given operand
   //
   // Arguments:
   //
   //    operand - The operand to process
   //    nodeKind - The kind of node to search or create
   //
   // Returns:
   //
   //    The dag node found or created
   //
   //--------------------------------------------------------------------------

   public DagNode
   FindOrCreateNode
   (
      Phx.IR.Operand operand,          
      DagNodeKind nodeKind
   )
   {
      if ((nodeKind & DagNodeKind.Expression) != 0)
      {
         return this.FindOrCreateExprNode(operand, nodeKind);
      }
      else if (operand.IsMemoryOperand)
      {
         nodeKind |= DagNodeKind.Memory;
         return this.FindOrCreateMemNode(operand.AsMemoryOperand, nodeKind);
      }
      else
      {
         return this.FindOrCreateSimpleNode(operand, nodeKind);
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Find or create the dag node for the given simple operand
   //
   // Arguments:
   //
   //    operand - The operand to process
   //    nodeKind - The kind of node to search or create. It could be either
   //          Use or Definition, but not Memory or Expression, which "simple" means here.
   //
   // Returns:
   //
   //    The dag node found or created
   //
   //--------------------------------------------------------------------------

   private DagNode
   FindOrCreateSimpleNode
   (
      Phx.IR.Operand operand,
      DagNodeKind nodeKind
   )
   {
      
#if (PHX_DEBUG_CHECKS)

      // The nodeKind should not be Memory or Expression

      Phx.Asserts.Assert((nodeKind & (DagNodeKind.Memory | DagNodeKind.Expression)) ==0,
         "DAG.FindOrCreateSimpleNode, nodeKind is not Memory or Expression");

#endif

      // Get hash code for the given operand

      int hashCode = DAG.GetHashCode(operand);

      DagNode node = null;

      bool isCSECandidate = DAG.IsSimpleCSECandidate(operand, nodeKind);

      if (isCSECandidate)
      {
         // The given operand is a candidate for CSE
         
         // look up in the opndToNodeMap for any cached information
         
         node = this.LookupOpnd(operand, hashCode);

         // a node is found, simply return it

         if (node != null)
         {
            return node;
         }
      }


      // No DAG information cached for the operand, create a new dag node.
      // We don't need to look up in the exprNodeMap since this is a simple
      // node 

      DagNode newNode = DagNode.New(operand, hashCode, nodeKind);
     
      if (isCSECandidate)
      {
         // The given operand is a candidate for CSE

         if (operand.IsDataflow && !operand.IsTemporary)
         {
            // For operands that are not temporaries, we should enforce 
            // evaluation order between this use node and its latest definition
            // node

            this.EnforceEvlOrderForUse(newNode);
         }

         // Cache the mapping information the the opndToNodemap

         this.opndToNodeMap.Insert(OpndKey.New(operand, hashCode), newNode);
      
      }

      // Currently, Phoenix does not support arbtrary type of temporaries.
      // Some types of operands should not be assigned to a temporary.
      // Set the NoTmp flag here.

      newNode.NoTmp = !DAG.CanAssignTmp(newNode);

      // Add the new node to the root node list since a new node is always a
      // root node in the DAG

      this.rootList.Add(newNode);

      return newNode;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Find or create the dag node for the given memory operand
   //
   // Arguments:
   //
   //    operand - The operand to process
   //    nodeKind - The kind of node to search or create. It could be either
   //          Use or Definition, and must be Memory, which "Memory" means here.
   //
   // Returns:
   //
   //    The dag node found or created
   //
   //--------------------------------------------------------------------------

   private DagNode
   FindOrCreateMemNode
   (
      Phx.IR.MemoryOperand operand,
      DagNodeKind nodeKind
   )
   {

#if (PHX_DEBUG_CHECKS)

      // The nodeKind must be Memory
      Phx.Asserts.Assert((nodeKind & (DagNodeKind.Memory)) != 0,
         "DAG.FindOrCreateMemNode, nodeKind is Memory");

#endif

      // Get hash code for the given operand

      int hashCode = GetHashCode(operand);

      DagNode node = null;

      // look up in the opndToNodeMap for any cached information

      if (DAG.IsSimpleCSECandidate(operand, nodeKind))
      {
         node = this.LookupOpnd(operand, hashCode);

         if (node != null)
         {
            // A node found, return it.

            return node;
         }
      }

      // No DAG information cached for the operand, create a dag node and
      // look up in the exprToNodeMap for common expression matching

     
      DagNode newNode = DagNode.New(operand, hashCode, nodeKind);

      // Add dependency edges between this node and its base/index/segment operands

      if (operand.BaseOperand != null)
      {
         DagNode baseNode = 
            this.FindOrCreateNode(operand.BaseOperand, DagNodeKind.Use);

         if(DAG.IsMustGenNode(baseNode))
         {
            // If the baseNode reprensents something too complex to be used
            // as the baseOperand of a memory operan, set its MustGen to true

            baseNode.MustGen = true;
         
         }

         newNode.BaseNode = baseNode;

         // Add data dependency between the memory operand node and its
         // base node

         this.AddEdge(newNode, baseNode, DagEdgeKind.ValueDep);
      }

      if (operand.IndexOperand != null)
      {
         DagNode indexNode = 
            this.FindOrCreateNode(operand.IndexOperand, DagNodeKind.Use);

         if (DAG.IsMustGenNode(indexNode))
         {
            indexNode.MustGen = true;
         }

         newNode.IndexNode = indexNode;

         this.AddEdge(newNode, indexNode, DagEdgeKind.ValueDep);
      }

      if (operand.SegmentOperand != null)
      {
         DagNode segNode = 
            this.FindOrCreateNode(operand.SegmentOperand, DagNodeKind.Use);

         if (DAG.IsMustGenNode(segNode))
         {
            segNode.MustGen = true;
         }

         newNode.SegNode = segNode;
         
         this.AddEdge(newNode, segNode, DagEdgeKind.ValueDep);
      }

      // After resolve all of its base/index/segment operand, try look up in the
      // opndToNodeMap again

      this.ResolveMemNode(newNode);

      operand = newNode.NewDstOpnd.AsMemoryOperand;

      // hashCode might change if base/index/segment operands change

      hashCode = GetHashCode(operand);

      if (DAG.IsSimpleCSECandidate(operand, nodeKind))
      {

         node = this.LookupOpnd(operand, hashCode);

         if (node != null)
         {
            // Unlink the newNode from all of its children

            this.RemoveNode(newNode);

            return node;

         }
      }

      // Search in the expression map

      NodeKey searchKey = NodeKey.New(newNode, hashCode);

      if ((nodeKind & DagNodeKind.Use) != 0)
      {
         // We always create a new definition node, so no need to lookup for definition node

         node = this.exprToNodeMap.Lookup(searchKey);

         if(node != null)
         {
            // common expression found

            // remove the dependency edges between this created query node and 
            // it's base and index operands since this new node will not be 
            // inserted to the DAG

            this.RemoveNode(newNode);

            // Cache the mapping information in the opndToNodeMap

            this.opndToNodeMap.Insert(OpndKey.New(operand, hashCode), node);

            // If the found node has target node, which is its congruence 
            // class, return the target node if it is safe

            DagNode targetNode = node.TargetNode;

            if (targetNode != null && !targetNode.IsMemNode)
            {
               node = this.ResolveTarget(node);
            }
            
            return node;
         }
      }


      // No commone expression found, insert it to the expression map

      this.exprToNodeMap.Insert(searchKey, newNode);

      if ((nodeKind & DagNodeKind.Use) != 0)
      {
         // We handle the definition nodes later

         // Enforce the evaluation order between the new use node and its 
         // latest definition node

         this.EnforceEvlOrderForUse(newNode);
         
         // Cache dag information
        
         this.opndToNodeMap.Insert(OpndKey.New(operand, hashCode), newNode);
      }

      // Currently, Phoenix does not support arbtrary type of temporaries.
      // Some types of operands should not be assigned to a temporary.
      // Set the NoTmp flag here.

      newNode.NoTmp = !DAG.CanAssignTmp(newNode);

      // Add the new node to the root list since a new node is always a root
      this.rootList.Add(newNode);

      return newNode;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Find or create the dag node for the given instruction
   //
   // Arguments:
   //
   //    operand - The dst operand of the instruction to process
   //    nodeKind - The kind of node to search or create. It must be Expression.
   //
   // Returns:
   //
   //    The dag node found or created
   //
   //--------------------------------------------------------------------------
   private DagNode
   FindOrCreateExprNode
   (
      Phx.IR.Operand operand,
      DagNodeKind nodeKind
   )
   {

#if (PHX_DEBUG_CHECKS)

      Phx.Asserts.Assert((nodeKind & (DagNodeKind.Expression)) != 0,
         "DAG.FindOrCreateExprNode, nodeKind is Expression");
      Phx.Asserts.Assert(operand.IsDefinition, "DAG.FindOrCreateExprNode, operand is Definition");

#endif

      Phx.IR.Instruction instruction = operand.Instruction;
     
      DagNode node = null;

      int hashCode = GetHashCode(instruction);

      DagNode newNode = DagNode.New(operand, hashCode, DagNodeKind.Expression);

      // Add data dependency between this expression to its src operands


      // Perform constant folding.
     
      DagNode newNode1;
      
      bool foldSucceed = this.Fold(instruction, out newNode1);
      
      if (foldSucceed)
      {
         // Unlink the new node from all of its children
      
         this.RemoveNode(newNode);
      
         // return the result constant node instead of the expression node
      
         return newNode1;
      }

      // Perform simple strength reduction.

      DagNode newNode2;
      
      bool reductionSucceed = this.Simplify(instruction, out newNode2);
      
      if (reductionSucceed)
      {
         // Unlink the new node from all of its children
      
         this.RemoveNode(newNode);
      
         // return the result simpler node instead of the original node
      
         return newNode2;
      }
      
      // Iterate through the src operand list and add its corresponding dag
      // nodes to the expression node.

      // Some memory src operands might be resolved to something else, thus,
      // it's editing-while-iterating situation. Simple for loop doesn't work.

      Phx.IR.Operand srcOperand = instruction.SourceOperand;
      Phx.IR.Operand prevSrcOpnd = srcOperand;
      bool firstSrc = true;

      while (srcOperand != null)
      {
         if (!srcOperand.IsAddressModeOperand)
         {
            // Addression mode operands are handled automatically when handling
            // their corresponding memory operands

            if (srcOperand.IsAliasOperand)
            {
               // A side-effect operand
               // Add evaluation order constraints between this node and all nodes
               // that are possibly used by this instruction

               int aliasTag = srcOperand.AliasTag;
               Phx.Alias.MemberPosition memberPosition = new Phx.Alias.MemberPosition();

               int srcTag;

               for (srcTag = this.aliasInfo.GetFirstMayPartialAlias(srcOperand.AliasTag, ref memberPosition);
                  srcTag != (int)Alias.Constants.InvalidTag;
                  srcTag = this.aliasInfo.GetNextMayPartialAlias(aliasTag, ref memberPosition))
               {
                  DagNode implicitSrcNode = this.tagToDefNodeMap.Lookup(srcTag);
                  
                  if(implicitSrcNode != null)
                  {
                     this.AddEdge(newNode, implicitSrcNode, DagEdgeKind.OrderDep);
                  }
               }
            }

            DagNode srcNode = this.FindOrCreateNode(srcOperand, DagNodeKind.Use);

            if (srcOperand.IsCodeTargetOperand)
            {
               // CallTargetOperand needs special care while code generation. Set
               // IsCallTarget flag here.
               
               srcNode.IsCallTarget = true;
            }

            // Add data dependency between the expression node and the src node

            this.AddEdge(newNode, srcNode, DagEdgeKind.ValueDep);
            
            newNode.SrcList.Add(srcNode);

         }

         // It is possible that nextSourceOperand is unlinked due to resolving
         // of the current srcOperand if it's a memoryOperand. Therefore we need
         // a handler of the previous src operand to correctly iterate.
         
         if (firstSrc)
         {
            //first srcOperand
            srcOperand = instruction.SourceOperand.Next;
            prevSrcOpnd = instruction.SourceOperand;
            firstSrc = false;
         }
         else
         {
            if (prevSrcOpnd.Next == null)
            {
               break;
            }
            srcOperand = prevSrcOpnd.Next.Next;
            prevSrcOpnd = prevSrcOpnd.Next;
         }
      }

      // Now look up the new expression in the exprToNodeMap for common
      // expression match.

      Phx.IR.Operand dstOperand = instruction.DestinationOperand;

      if (instruction.IsValueInstruction && dstOperand.IsExplicit)
      {
         // Only value insruction is candidate for common expression match.

         // Look up the common expression

         NodeKey searchKey = NodeKey.New(newNode, hashCode);
         
         node = this.exprToNodeMap.Lookup(searchKey);

         if(node != null)
         {
            // common expression found

            this.RemoveNode(newNode);

            return node;
         }

         // put the new expression in the hash table 

         this.exprToNodeMap.Insert(searchKey, newNode);
      }

      // Enforce evaluation order before calls

      if (instruction.IsCallInstruction)
      {
         if (this.lastCallNode != null)
         {
            this.AddEdge(newNode, this.lastCallNode, DagEdgeKind.OrderDep);
         }
         this.lastCallNode = newNode;
      }

      // The result of some expression should not be assigned to a temporary.
      // Set the NoTmp flag if the given expression belongs to this case.

      newNode.NoTmp = !DAG.CanAssignTmp(newNode);

      // Add the new node to the root node list 

      this.rootList.Add(newNode);

      return newNode;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A helper function that determines whether the given operand and 
   //    nodeKind is candidate of simple common sub-expression (CSE), in
   //    which case we can look up the opndToNodeMap first.
   //
   // Arguments:
   //
   //    operand - The operand to look up
   //    nodeKind - The kind of node to search or create
   //
   // Returns:
   // 
   //    True if the given operand is a simple CSE candidate, otherwise false
   //
   //--------------------------------------------------------------------------
   
   private static bool
   IsSimpleCSECandidate
   (
      Phx.IR.Operand operand,
      DagNodeKind nodeKind
   )
   {
      if ((nodeKind & DagNodeKind.Use) == 0)
      {
         // Only uses node can be matched in the opndToNodeMap.
         // For expression nodes, we need to look op the exprToNodeMap.
         // For definition nodes, we always create a new node for a definition operand, and
         // thus no need to lookup
         
         return false;
      }

      if ((nodeKind & DagNodeKind.Memory) != 0)
      {
         // Memory operand is always cached in opndToNodeMap
         return true;
      }

      // we only look up dataflow/address/Immediate operand since
      // other kinds of operands should not be reused.

      return !operand.IsAliasOperand
         && (operand.IsDataflow || operand.IsAddress || operand.IsImmediateOperand);

   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A helper function that determines whether the given node must be
   //    assigned to another temporary before it's used by other instruction
   //
   // Arguments:
   //
   //    node - The dag node to examine
   //
   // Returns:
   // 
   //    True if the given node a must-gen dag node, otherwise false
   //
   //--------------------------------------------------------------------------

   private static bool
   IsMustGenNode
   (
      DagNode node
   )
   {
      if(node.IsExprNode)
      {
         return true;
      }

      if (node.Label.IsIndirect)
      {
         return true;
      }

      return false;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A helper function that determines whether the given operand or 
   //    expression could be assigned to an arbitrary temporary or not.
   //
   // Arguments:
   //
   //    node - The dag node to examine
   //
   // Returns:
   // 
   //    True if it can be assigned to a temporary, otherwise false
   //
   //--------------------------------------------------------------------------

   private static bool
   CanAssignTmp
   (
      DagNode node
   )
   {
      if (node.IsExprNode)
      {
         return true;
      }
      else
      {
         Phx.IR.Operand operand = node.Label;

         if (operand.Type != null && operand.Type.IsAggregate
           && operand.FunctionUnit.Architecture.CanEnregisterType(operand.Type))
         {
            // Large aggregate type
            return false;
         }

         if (operand.BitSize % 8 != 0 || operand.BitSize > 32)
         {
            // Unnatural-sized operand
            return false;
         }
         
         // What else?

         return true;
      }

   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A helper function that determines whether the given expression can
   //    be foled to some constant, if so, do the folding.
   //
   // Arguments:
   //
   //    instruction - The instruction to fold
   //    out newNode - If folding succeeds, newNode will be the new dag node
   //          represents the result constant. Otherwise, it's null
   //
   // Returns:
   // 
   //    True if the expression can be folded to constant, otherwise false
   //
   // Remarks:
   //
   //    we only fold expressions with integer values
   //
   //--------------------------------------------------------------------------
   
   private bool
   Fold
   (
      Phx.IR.Instruction instruction,
      out DagNode newNode
   )
   {
      newNode = null;

      Phx.IR.Operand dstOperand = instruction.DestinationOperand;

      if (dstOperand == null || dstOperand.IsImplicit || 
         ((!dstOperand.IsInt && !dstOperand.IsPointer)
            && (!instruction.IsCompareInstruction || instruction.SourceOperand.IsFloat)))
      {
         // Only handle the folding of integral operations for now.

         return false;
      }

      Phx.Constant.Table constantTable = instruction.FunctionUnit.ConstantTable;

      Phx.Constant.ArithmeticValue[] srcValue = new Phx.Constant.ArithmeticValue[3];

      int numSrc = 0;

      foreach (Phx.IR.Operand srcOperand in
         Phx.IR.Operand.IteratorEditingExplicitSource(instruction))
      {
         if( numSrc > 2)
         {
            // We only support folding up to 3 integral operands
            return false;
         }
         
         DagNode srcNode = this.FindOrCreateNode(srcOperand, DagNodeKind.Use);

         Phx.IR.Operand newLabel = srcNode.Label;

         if (!newLabel.IsIntImmediate)
         {
            // We only support integral operands
            return false;
         }
         else
         {
            srcValue[numSrc++] = constantTable.LookupOrInsert(newLabel.AsImmediateOperand.IntValue);
         }
      }

      // The expression can be folded to a single integer value
      
      // Try to fold the expression

      Phx.Constant.ArithmeticValue dstValue;

      Phx.Constant.Result  result = new Phx.Constant.Result();
      
      switch(numSrc)
      {
         case 1:
            dstValue = Phx.Constant.Info.EvaluateUnary(ref result, instruction.Opcode,
               instruction.SourceOperand1.Type, dstOperand.Type, srcValue[0]);

            break;
            
         case 2:
            dstValue = Phx.Constant.Info.EvaluateBinary(ref result, instruction.Opcode,
               instruction.ConditionCode, instruction.SourceOperand1.Type, srcValue[0], srcValue[1]);

            break;

         case 3:
            dstValue = Phx.Constant.Info.EvaluateTernary(ref result, instruction.Opcode,
               instruction.SourceOperand1.Type, srcValue[0], srcValue[1], srcValue[2]);

            break;

         default:

            return false;
      }

      if(dstValue == null || result.HasRestrictions)
      {
         return false;
      }

      // Folding succeeds

      // return the result constant node instead of the expression node
         
      Phx.IR.Operand newImmediateOperand = Phx.IR.ImmediateOperand.New(instruction.FunctionUnit,
            instruction.SourceOperand.Type, dstValue);
      
      newNode = this.FindOrCreateSimpleNode(
         newImmediateOperand, DagNodeKind.Use);

      return true;
   }


   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A helper function that determines whether the given expression can
   //    be simplified to simpler operation, if so, do the reduction.
   //
   // Arguments:
   //
   //    instruction - The instruction to simplify
   //    out newNode - If simplification succeeds, newNode will be the new dag 
   //       node represents the result constant. Otherwise, it's null
   //
   // Returns:
   // 
   //    True if some simplification is performed, otherwise false
   //
   // Remarks:
   //
   //    we only implement some very simple optimizatoins here. Readers are
   //    encouraged to extend this function to perform many other useful 
   //    simplifications. Right now, cases are handled are
   //
   //       MUL X, 2^N  -> SHL X, N
   //       DIV X, 2^N  -> SHR X, N
   //
   //--------------------------------------------------------------------------

   private bool
   Simplify
   (
      Phx.IR.Instruction instruction,
      out DagNode newNode
   )
   {
      newNode = null;

      Phx.Common.Opcode.Index opcode = Common.Opcode.GetIndex(instruction.Opcode);

      if ((opcode != Phx.Common.Opcode.Index.Multiply)
            && (opcode != Phx.Common.Opcode.Index.Divide))
      {
         // we only handle MUL and DIV now
         return false;
      }

      Phx.IR.Operand power2Opnd = null;
      long power2Value = -1;

      foreach (Phx.IR.Operand srcOperand in
         Phx.IR.Operand.IteratorEditingExplicitSource(instruction))
      {

         // Find the congruence class of the src operand first.
         // This might change some src operand to constant, which instroduces
         // optimization opportunities.

         DagNode srcNode = this.FindOrCreateNode(srcOperand, DagNodeKind.Use);

         Phx.IR.Operand newLabel = srcNode.Label;

         if (!newLabel.IsImmediateOperand)
         {
            // non-constant src operand

            if (!srcOperand.IsInt && !srcOperand.IsPointer
               || srcOperand.BitSize != 32)
            {
               // We limit ourselves to simple integeral values
               return false;
            }
         }
         else
         {
            // constant src operand
             Phx.IR.ImmediateOperand constOp = newLabel.AsImmediateOperand;

             if (constOp.IntValue > 0
               && Phx.Utility.IsPowerOf2(constOp.IntValue)
               && (opcode != Phx.Common.Opcode.Index.Divide
                  || srcOperand != instruction.SourceOperand))
            {
               // For DIV, we can only perform strength reduction if the 
               // second src operand is a power of 2
               power2Opnd = srcOperand;
               power2Value = constOp.IntValue;
            }
            else
            {
               return false;
            }
         }
      }


      // The expression can be folded to a single integer value

      if (power2Value > 1)
      {
         // tranformation: x* (2^N) -> shl X N

         if (opcode == Phx.Common.Opcode.Index.Multiply)
         {
            instruction.Opcode = Phx.Common.Opcode.ShiftLeft;
         }
         else
         {
            instruction.Opcode = Phx.Common.Opcode.ShiftRight;
         }

         int logValue = Phx.Utility.Log2(power2Value);

         Phx.IR.Operand newSourceOperand =
            Phx.IR.ImmediateOperand.New(functionUnit, power2Opnd.Type, logValue);

         instruction.UnlinkSource(power2Opnd);
         instruction.AppendSource(newSourceOperand);

         // create a new dag node for the new instruction

         newNode = this.FindOrCreateExprNode(instruction.DestinationOperand, DagNodeKind.Expression);

         // simplification succeeds

         return true;

      }

      return false;

   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Kill all dag nodes that might be affected by the dst operands of this 
   //    instruction. Create new Definition nodes for the explicit dst operands and 
   //    attach it to the newTargetNode
   //
   // Arguments:
   //
   //    instruction - The instruction to process
   //    newTargetNode - The dag node represents right hand side
   //
   // Returns:
   // 
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   public void
   HandleDstOpnd
   (
      Phx.IR.Instruction instruction,
      DagNode newTargetNode 
   )
   {
      DagNode newExplicitDefNode = null;

      // Kill nodes that are affected by the side-effect dst operand first
      // Perform this first before generating definition node for the explicit dst
      // operand to avoid the new explicit definition node is also killed by the
      // side-effect operand.

      foreach (Phx.IR.Operand dst in instruction.DestinationOperands)
      {
         if (dst.IsAliasOperand)
         {
            // For the side-effect dst operand, use the dag node of the right
            // hand side in the instruction to establish evaluation order
            
            this.EnforceEvlOrderForDef(dst, newTargetNode);
            break;
         
         }
      }

      // We assume there is only one explicit dst operand for 
      // instructions we are handling here. Instructions with more
      // than one explicit dst operand are handled by Transfer()

      Phx.IR.Operand dstOperand = instruction.DestinationOperand;
   
      if (!dstOperand.IsExplicit || !dstOperand.IsDataflow)
      {
         // We don't care about non-dataflow operand
         return;
      }

      // Create a definition node for the explicit dst operand

      newExplicitDefNode = this.FindOrCreateNode(dstOperand, DagNodeKind.Definition);

      if (!dstOperand.IsTemporary)
      {
         // We need to enforce evaluation order between this definition node and all
         // previous use nodes and the latest definition node for the given operand

         this.EnforceEvlOrderForDef(dstOperand, newExplicitDefNode);

      }

      // Cache the mapping information in the opndToNodeMap

      this.opndToNodeMap.Insert(OpndKey.New(dstOperand, GetHashCode(dstOperand)),
         newExplicitDefNode);

      // set the targetNode for the new definition node and add the assignment 
      // relationship between them.

      newExplicitDefNode.TargetNode = newTargetNode;

      this.AddEdge(newExplicitDefNode, newTargetNode, DagEdgeKind.AssignDep);

   }


   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Mark a dag node as killed. Also invalidated all nodes that have data
   //    dependency on the given node.
   //
   // Arguments:
   //
   //    toKill - the dag node to be killed
   //    killer - the dag node causes the killing
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   private void
   InvalidateNode
   (
      DagNode toKill,      
      DagNode killer      
   )
   {

#if (PHX_DEBUG_CHECKS)

      Phx.Asserts.Assert(toKill != killer && killer != null,
         "DAG.InvalidateNode, toKill != killer");

#endif

      // We need to invalidate all nodes that are dependent on this
      // node since their expressions are not valid any more

      // Node killer must be evaluated after all value parents of
      // the being killed node. Add evaluation order constraints.

      // Use two passes so that nodes close to leaf will be evaluated earlier

      foreach(DagEdge parent in toKill.ParentList)
      {
         // First pass handles those nodes having true value dependency

         if (parent.Kind != DagEdgeKind.ValueDep)
         {
            continue;
         }

         if (parent.FromNode != killer)
         {
            // recursively invalidate nodes

            this.InvalidateNode(parent.FromNode, killer);
         }
      }


      foreach (DagEdge parent in toKill.ParentList)
      {
         // Second pass handles those nodes having assignment dependency

         if (parent.Kind != DagEdgeKind.AssignDep)
         {
            continue;
         }

         // don't need to kill the definition nodes, but add evaluation order
         // It is not a must, but will improve code quality
         
         this.AddEdge(killer, parent.FromNode, DagEdgeKind.OrderDep);
      
      }

      // Finally add evaluation order between the killer and toKill node
      
      this.AddEdge(killer, toKill, DagEdgeKind.OrderDep);

      // Invalidate the node, avoid common expression match
      toKill.IsInvalid = true;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Resolve the baseOperand and indexOperand of the given memory operand node
   //
   // Returns:
   //
   //    The memory operand resolved
   //
   //--------------------------------------------------------------------------

   public Phx.IR.Operand
   ResolveMemNode
   (
      DagNode dagNode
   )
   {
      // Resolve the base/index/segment operand according to dag nodes
      
      Phx.IR.Operand label = dagNode.NewDstOpnd;

      if (label.BaseOperand != null)
      {
         Phx.IR.Operand newBaseOperand = dagNode.BaseNode.NewDstOpnd;
         
         if (!newBaseOperand.IsIndirect &&
            !this.aliasInfo.MustExactlyOverlap(label.BaseOperand, newBaseOperand))
         {
            label.Instruction.ReplaceAndUnlinkSource(
               label.BaseOperand, newBaseOperand.AsVariableOperand);
         }
      }

      if (label.IndexOperand != null)
      {
         Phx.IR.Operand newIndexOperand = dagNode.IndexNode.NewDstOpnd;
         
         if (!newIndexOperand.IsIndirect &&
            !this.aliasInfo.MustExactlyOverlap(label.IndexOperand, newIndexOperand))
         {
            label.Instruction.ReplaceAndUnlinkSource(
               label.IndexOperand, newIndexOperand.AsVariableOperand);
         }
      }

      if (label.SegmentOperand != null)
      {
         Phx.IR.Operand newSegmentOperand = dagNode.SegNode.NewDstOpnd;
      
         if (!newSegmentOperand.IsIndirect &&
            !this.aliasInfo.MustExactlyOverlap(label.SegmentOperand, newSegmentOperand))
         {
            label.Instruction.ReplaceAndUnlinkSource(
               label.IndexOperand, newSegmentOperand.AsVariableOperand);
         }
      }

      return label;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Resolve the given node to its target node if possible
   //
   // Returns:
   //
   //    The targetNode of the given node if safe, otherwise the orignal node
   //
   //--------------------------------------------------------------------------

   public DagNode
   ResolveTarget
   (
      DagNode node
   )
   {

#if (PHX_DEBUG_CHECKS)

      Phx.Asserts.Assert(node.TargetNode != null,
         "DAG.ResolveTarget(), node.TargetNode != null");

#endif

      DagNode targetNode = node.TargetNode;

      if (!node.Label.IsTemporary && targetNode.IsInvalid)
      {
         // If the target node is already invalidated, i.e., its value is not
         // valid any more, we return the original node. However, if the given
         // node is a Definition node for a temporary, it's safe to resolve to its
         // target node even the target node is invalidated.

         return node;
         
      }

      Phx.IR.Operand targetOperand = targetNode.Label;

      if(!node.Label.IsTemporary && targetOperand.IsTemporary && targetOperand.IsFloat
         && targetNode.IsExprNode)
      {
         // Special handling for floating point operands. Any copying of temps
         // through memory (of temps) cannot be done on float types since on 
         // x86 it changes the representation to f80. For example,
         //
         //    t1 = MUL t2, 4
         //    x  = t1
         //    t3 = CONV x
         //
         // It is not correct to change it to
         //
         //    tv1 = MUL t2, 4
         //    x  = tv1
         //    t3 = CONV tv1

         return node;
      }

      return targetNode;
   }
   
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Update the renameMap using the result of current analysis.
   //
   // Arguments:
   //
   //    renameMap - The renameMap to be updated
   //
   // Returns:
   //
   //    Nothing.
   //
   // Remarks:
   //
   //    Code generation of DAG will rename temporaries. The renameMap is 
   //    used for keeping mapping between original name of temporaries and
   //    their new names. It will be used when processing special instructions
   //    that cannot be handled by DAG.
   //
   //--------------------------------------------------------------------------

   public void
   UpdateRenameMap
   (
      OpndToOpndMap renameMap
   )
   {
      // Iterate all items in the opndToNodeMap and update the renameMap
      // accordingly

      foreach(DagNode node in this.opndToNodeMap.Values)
      {
         if (!node.IsDefNode)
         {
            // Only definition node needs to be cached in the rename map
            continue;
         }

         Phx.IR.Operand operand = node.Label;

         // Get the target node that this definition node points to

         DagNode targetNode = this.ResolveTarget(node);

         if (operand.IsTemporary || !node.IsInvalid && !targetNode.IsInvalid)
         {
            // If the given node is a Definition node for an ExpressionTemporary, it's safe to
            // resolve to its target node even the target node is invalidated.

            Phx.IR.Operand targetOperand = targetNode.NewDstOpnd;

            // create a new mapping between operand and targetOperand

            renameMap.Insert(OpndKey.New(operand, GetHashCode(operand)), targetOperand);


            // For cases like t1 = &_x, we create a mapping between
            // t1 and &_x too so that we can perform simplification
            // linke [&_x] => _x later if possible

            Phx.IR.Operand targetOldOpnd = targetNode.Label;

            if (targetOldOpnd.IsVariableAddress)
            {
               renameMap.Insert(OpndKey.New(targetOperand, GetHashCode(targetOperand)),
                  targetOldOpnd);
            }
         }
      }

   }


   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Generate hashCode for the input operend
   //
   // Returns:
   //
   //    The hash code generated.
   //
   //--------------------------------------------------------------------------

   public static int 
   GetHashCode
   (
      Phx.IR.Operand operand
   )
   {
      int hashCode = 0;

      Phx.Alias.Info aliasInfo = operand.FunctionUnit.AliasInfo;

      switch (operand.OperandKind)
      {
         case Phx.IR.OperandKind.VariableOperand:
            {
               Phx.IR.Operand variableOperand = operand.AsVariableOperand;

               if (variableOperand.IsVariableAddress)
               {
                  hashCode = aliasInfo.GetPrimaryTag(variableOperand.AliasTag);
                  hashCode += (hashCode << 4);
               }
               else
               {
                  hashCode = aliasInfo.MustExactTag(variableOperand.AliasTag);
               }

               break;
            }

         case Phx.IR.OperandKind.MemoryOperand:
            {
               Phx.IR.MemoryOperand memoryOperand = operand.AsMemoryOperand;

               hashCode = aliasInfo.MustExactTag(memoryOperand.AliasTag);

               if (memoryOperand.IsAddress)
               {
                  hashCode += (hashCode << 4);
               }

               // Incorporate the immediate bit-offset from the base
               // operand. Bits are better than bytes for hashing.

               hashCode += (int)(memoryOperand.Field.BitOffset
                  + Phx.Utility.BytesToBits((uint)memoryOperand.ByteOffset));

               // We use the type of base/indes/segment operands as a signature
               // of the given memory operand

               if(memoryOperand.BaseOperand != null)
               {
                  hashCode += memoryOperand.BaseOperand.Type.GetHashCode();
               }
               if (memoryOperand.IndexOperand != null)
               {
                  hashCode += memoryOperand.IndexOperand.Type.GetHashCode();
               }
               if (memoryOperand.SegmentOperand != null)
               {
                  hashCode += memoryOperand.SegmentOperand.Type.GetHashCode();
               }
               
               // Contribute something none zero to give this a distinct
               // code from an expression temporary operand.

               hashCode += 7;
               break;
            }

         case Phx.IR.OperandKind.AliasOperand:
            {
               Phx.IR.AliasOperand aliasOperand = operand.AsAliasOperand;

               hashCode = aliasInfo.MustExactTag(aliasOperand.AliasTag);

               if (hashCode == (int)Alias.Constants.InvalidTag)
               {
                  // This alias tag has no equivalent exact reference.

                  hashCode = aliasOperand.AliasTag;
               }
               break;
            }

         case Phx.IR.OperandKind.ImmediateOperand:
            {
               Phx.IR.ImmediateOperand immediateOperand = operand.AsImmediateOperand;

               // Hash the immediate value.
               if (immediateOperand.IsSymbolicImmediate)
               {
                  // Generally not resolved so hash on the external id.
                  hashCode = (int)immediateOperand.Symbol.ExternId;
               }
               else
               {
                  Phx.Types.Type type = immediateOperand.Type;

                  if (type.IsFloat)
                  {
                     
                     hashCode = (int) immediateOperand.ReinterpretAsIntValue();

                  }
                  else if (type.IsPointer || type.IsInt || type.IsConditionCode)
                  {
                     // Use the intergral bit-representation for hashing.

                     hashCode = (int)immediateOperand.IntValue;
                  }
                  else
                  {
                     throw new System.Exception("Bad immediate type."
                        +" Don't know how to generate hash code");
                  }
                  
                  // distinguish ImmOpnds by their type
                  
                  hashCode += (immediateOperand.Type.GetHashCode() << 4);
               }
               break;
            }

         case Phx.IR.OperandKind.FunctionOperand:
            {
               Phx.IR.FunctionOperand functionOperand = operand.AsFunctionOperand;
               hashCode = (int)functionOperand.Symbol.ExternId;
               break;
            }

         case Phx.IR.OperandKind.LabelOperand:
            {
               Phx.IR.LabelOperand labelOperand = operand.AsLabelOperand;

               hashCode = (int)labelOperand.LabelId;

               if (labelOperand.LabelSymbol != null)
               {
                  hashCode += (int)labelOperand.LabelSymbol.ExternId;
               }
               break;
            }

         default:

            throw new System.Exception("Unimplemented type of operand!"
                +" Don't know how to generate hash code");
      }

      return hashCode;

   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Generate hashCode for the input instruction. 
   //
   // Returns:
   //
   //    The hashcode generated
   //
   //--------------------------------------------------------------------------

   public static int
   GetHashCode
   (
      Phx.IR.Instruction instruction
   )
   {
      int hashCode = 0;

      // opcode is the most important part of an instruction

      int opcodeHashCode = instruction.Opcode.GetHashCode();

      hashCode += (opcodeHashCode + (opcodeHashCode << 10));

      // use ConditionCode as part of hash code if it's a branch instruction

      if (instruction.IsBranchInstruction)
      {
         hashCode += instruction.ConditionCode;
      }

      foreach (Phx.IR.Operand srcOperand in instruction.SourceOperands)
      {
         // We use the type of each srcOperand as a signature of the expression  
         if (srcOperand.Type != null)
         {
            hashCode += srcOperand.Type.GetHashCode();
         }
      }

      return hashCode;

   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Compare two given operands lexically
   //
   // Returns:
   //
   //    True if the given two operands are identical, false otherwise
   //
   //--------------------------------------------------------------------------

   public static bool
   CompareOperand
   (
      Phx.IR.Operand opndA,
      Phx.IR.Operand opndB
   )
   {
      // perform more expensive comparison based on alias tag and other
      
      return Phx.Expression.Table.Compare(opndA, opndB, Phx.Expression.CompareControl.Lexical);
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Compare two given expression nodes structurally
   //
   // Returns:
   //
   //    True if the expression of the given two nodes matches, false otherwise
   //
   //--------------------------------------------------------------------------

   public static bool
   CmpExprNode
   (
      DagNode nodeA,
      DagNode nodeB
   )
   {
      Phx.IR.Instruction instruction1 = nodeA.Label.Instruction;
      Phx.IR.Instruction instruction2 = nodeB.Label.Instruction;

      if (instruction1 == instruction2)
      {
         return true;
      }

      if ((instruction1.Opcode != instruction2.Opcode)
         || (instruction1.IsReal != instruction2.IsReal)
         || (instruction1.IsInlineAsm != instruction2.IsInlineAsm))
      {
         // The given instructions are obviously different
         
         return false;
      }

      if (instruction1.HasOpcodeSideEffect || instruction2.HasOpcodeSideEffect)
      {
         // Instructions with operation side-effects are not equal.

         return false;
      }


      // We only handle ValueInstruction and CallInstruction in DAG

      switch (instruction1.InstructionKind)
      {
         case Phx.IR.InstructionKind.ValueInstruction:

            // no additional conditions for value instructions
            break;

         case Phx.IR.InstructionKind.CallInstruction:

            if ((instruction1.AsCallInstruction.IsNotPrototyped 
               != instruction2.AsCallInstruction.IsNotPrototyped)
            || (instruction1.AsCallInstruction.CallSiteTokenSymbol 
               != instruction2.AsCallInstruction.CallSiteTokenSymbol))
            {
               // The calls must have the same characteristics.

               return false;
            }

            break;
        
         default:

            throw new System.Exception(
               "Unexpected Instruction kind while comparing expression nodes");
      }


      // Now compare each destination operand for equality.
      Phx.Alias.Info aliasInfo = instruction1.FunctionUnit.AliasInfo;

      Phx.IR.Operand dstOperand1 = instruction1.DestinationOperand;
      Phx.IR.Operand dstOperand2 = instruction2.DestinationOperand;

      while ((dstOperand1 != null) && (dstOperand2 != null))
      {
         if (dstOperand1.IsProxy)
         {
            // Skip proxy operands since they correspond to explicit
            // operands.

            dstOperand1 = dstOperand1.Next;
            continue;
         }
         else if (dstOperand2.IsProxy)
         {
            // Skip proxy operands since they correspond to explicit
            // operands.

            dstOperand2 = dstOperand2.Next;
            continue;
         }

         
         
         if (dstOperand1.Type != dstOperand2.Type)
         {
            // Compare the operation types but not their locations.

            if (!Phx.Alias.Info.TypeEqual(dstOperand1.Field, dstOperand2.Field, true))
            {
               return false;
            }
         }

         if (dstOperand1.IsAliasOperand && dstOperand2.IsAliasOperand)
         {
            if (!aliasInfo.MustExactlyOverlap(dstOperand1, dstOperand2))
            {
               return false;
            }
         }

         if (dstOperand1.IsNonNullExplicit != dstOperand2.IsNonNullExplicit)
         {
            // The instructions do not have the same number of explicit
            // destination operands.

            return false;
         }

         // Advance to the next operand to compare.

         dstOperand1 = dstOperand1.Next;
         dstOperand2 = dstOperand2.Next;
      }

      // For src operands of an instruction, we need to compare the src list
      // of the dag nodes for equality test

      if (nodeA.SrcList.Count != nodeB.SrcList.Count)
      {
         return false;
      }

      for(int i = 0; i < nodeA.SrcList.Count; i++)
      {
         if(nodeA.SrcList[i] != nodeB.SrcList[i])
         {
            return false;
         }
      }

      return true;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Compare two given memory nodes structurally
   //
   // Returns:
   //
   //    True if the given two memory nodes matches, false otherwise
   //
   //--------------------------------------------------------------------------

   public static bool
   CmpMemNode
   (
      DagNode nodeA,
      DagNode nodeB
   )
   {
      Phx.IR.MemoryOperand memoryOperand1 = nodeA.Label.AsMemoryOperand;
      Phx.IR.MemoryOperand memoryOperand2 = nodeB.Label.AsMemoryOperand;
      
      Phx.Types.Field field1 = memoryOperand1.Field;
      Phx.Types.Field field2 = memoryOperand2.Field;

      if (field1 != field2)
      {
         // Check whether the types are for compatible values.

         if ((field1.BitOffset != field2.BitOffset)
            || !Phx.Alias.Info.TypeEqual(field1, field2, true))
         {
            return false;
         }
      }

      if (memoryOperand1.IsAddress != memoryOperand2.IsAddress)
      {
         return false;
      }

      if (memoryOperand1.Barrier != memoryOperand2.Barrier)
      {
         return false;
      }

      if (memoryOperand1.AddressUpdate != memoryOperand2.AddressUpdate)
      {
         return false;
      }

      Phx.Alias.Info aliasInfo = memoryOperand1.FunctionUnit.AliasInfo;

      int tag1 = memoryOperand1.AliasTag;
      int tag2 = memoryOperand2.AliasTag;

      if (!aliasInfo.MustExactlyOverlap(tag1, tag2))
      {
         // The alias package cannot confirm that they are
         // identical references so compare the operands and
         // address expressions.

         if (memoryOperand1.AddressMode != memoryOperand2.AddressMode)
         {
            return false;
         }

         if (memoryOperand1.ByteOffset != memoryOperand2.ByteOffset)
         {
            return false;
         }

         if (memoryOperand1.AddressAdjustment != memoryOperand2.AddressAdjustment)
         {
            return false;
         }

         if (memoryOperand1.AddressShifter != memoryOperand2.AddressShifter)
         {
            return false;
         }

         if (!Phx.Alignment.Compare(memoryOperand1.Alignment, memoryOperand2.Alignment))
         {
            return false;
         }

         if (memoryOperand1.Symbol != memoryOperand2.Symbol)
         {
            return false;
         }

         if ((memoryOperand1.Symbol == null) && (memoryOperand1.BaseOperand == null))
         {
            // Indirection PHIs have naked memory operands that
            // should not be equivalent unless they have the
            // same indirect variable tag checked above.

            return false;
         }
      }

      // compare base/index/segment node

      if (nodeA.BaseNode != nodeB.BaseNode
         || nodeA.IndexNode != nodeB.IndexNode
         || nodeA.SegNode != nodeB.SegNode)
      {
         return false;
      }

      return true;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Remove the given node from the dag structure.
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   private void 
   RemoveNode
   (
      DagNode node
   )
   {
      // Remove it from the root list if it's a root

      if (node.IsRoot)
      {
         this.rootList.Remove(node);
      }

      // Update the status of its children

      if (node.ChildrenList.Count != 0)
      {

         foreach(DagEdge childEdge in node.ChildrenList)
         {
            DagNode child = childEdge.ToNode;

            child.ParentList.Remove(childEdge);

            if (childEdge.Kind != DagEdgeKind.OrderDep)
            {
               child.NumRealParent--;
            }

            if (child.ParentList.Count == 0)
            {
               child.IsRoot = true;
               this.rootList.Add(child);
            }
         }
      }
    }

    //--------------------------------------------------------------------------
    //
    // Description:
    //
    //    Add evaluation order between the given node and the latest definition node that 
    //    has alias tag overlaping with it
    //
    // Arguments:
    //
    //    node  - The new use node
    //
    // Returns:
    //
    //    Nothing.
    //
    //--------------------------------------------------------------------------

   private void 
   EnforceEvlOrderForUse
   (
      DagNode node
   )
   {
 
#if (PHX_DEBUG_CHECKS)

      Phx.Asserts.Assert(node.IsUseNode,
         "DAG.EnforceEvlOrderForUse, node is a use node");

#endif

      int aliasTag = node.Label.AliasTag;

      Phx.Alias.MemberPosition memberPosition = new Phx.Alias.MemberPosition();

      for (int mayPOTag = this.aliasInfo.GetFirstMayPartialAlias(aliasTag, ref memberPosition);
         mayPOTag != (int)Alias.Constants.InvalidTag;
         mayPOTag = this.aliasInfo.GetNextMayPartialAlias(aliasTag, ref memberPosition))
      {
         // Look up the latest definition node for this use
         
         DagNode latestDefNode = this.tagToDefNodeMap.Lookup(mayPOTag);
         
         if(latestDefNode != null)
         {

#if (PHX_DEBUG_CHECKS)

            Phx.Asserts.Assert(node != latestDefNode,
               "DAG.EnforceEvlOrderForUse, node != latestDefNode");

#endif

            // Add evaluation order between this use and last definition 
            
            this.AddEdge(node, latestDefNode, DagEdgeKind.OrderDep);
            
         }

         // Insert it as a new use for the given tag

         if(this.tagToUseNodeMap.ContainsKey(mayPOTag))
         {
            // A use node list already exists, simply insert to it

            DagNodeList nodelist = this.tagToUseNodeMap[mayPOTag];
            
            nodelist.Add(node);
         }
         else
         {
            // First use node for the given tag, create a node list
            // and insert it to the map

            DagNodeList nodelist = new DagNodeList();
            
            nodelist.Add(node);
            
            this.tagToUseNodeMap.Add(mayPOTag, nodelist);
         }
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Add evaluation order between the given definition node and recent use nodes
   //    and the previous definition nodes that have alias tag overlaping with it
   //
   // Arguments:
   //
   //    operand - The dst operand to process. It can be either an explicit dst
   //          operand or an ModRef operand that implies side-effects
   //    node  - The dag node that defines a new value of a location. It can 
   //          be either the definition node for the explicit dst operand or the 
   //          expression node for the instruction.
   //
   // Returns:
   //
   //    Nothing.
   //
   // Remarks:
   //
   //    If the input dst operand is the explicit dst operand of its 
   //    instruction, the input node will be the definition node that represents this
   //    dst operand. If dst operand is a AliasOperand of its instruction, which 
   //    represents the set of alias tag that might be affected by the 
   //    instruction, the input node is the expression node of the instruction, 
   //    instead of the definition node in normal cases.
   //
   //--------------------------------------------------------------------------
   
   private void
   EnforceEvlOrderForDef
   (
      Phx.IR.Operand operand,
      DagNode node
   )
   {

#if (PHX_DEBUG_CHECKS)

      Phx.Asserts.Assert(!node.IsUseNode,
         "DAG.EnforceEvlOrderForDef, node is not a use node");

#endif

      int aliasTag = operand.AliasTag;

      Phx.Alias.MemberPosition memberPosition = new Phx.Alias.MemberPosition();

      for (int mayPOTag = this.aliasInfo.GetFirstMayPartialAlias(aliasTag, ref memberPosition);
         mayPOTag != (int)Alias.Constants.InvalidTag;
         mayPOTag = this.aliasInfo.GetNextMayPartialAlias(aliasTag, ref memberPosition))
      {
         // Look up the recent use nodes first

         DagNodeList nodeList = this.tagToUseNodeMap.Lookup(mayPOTag);

         if(nodeList != null)
         {
            foreach (DagNode useNode in nodeList)
            {
               // Double check whether the two operands may overlap partially

                if (!this.aliasInfo.MayPartiallyOverlap(useNode.Label, operand)
			         || useNode.IsKilled)
               {
                  // The overlap is not direct, or the use node is already 
		            // processed previously, we simply add evaluation order 
		            // between them
	               this.AddEdge(node, useNode, DagEdgeKind.OrderDep);
                  continue;
               }

               // There are some overlap between the use operand and the
               // new definition operand. The use operand becomes invalid now.


               // Invalidate all nodes that are depentdent on the useNode
               // Evaluation order is enforced during the invalidation

               this.InvalidateNode(useNode, node);

               // Set the useNode to IsKilled so that we don't need to 
               // process it if it appears again. Note, one aliag tag
               // might be overlapping several tags, so it's possible that
               // one use node appears in several entries in the
               // tagToUseNodeMap.

               useNode.IsKilled = true;

               // If an identifier is assigned multiple times, it's possible
               // to eliminate dead stores. But due to the conservative 
               // killing process, we need to make sure that an identifier 
               // is killed by the same identifier before we elimiate the 
               // dead stores. IsKilledBySelf is a property for this purpose

               if (DAG.CanKillEachOther(useNode, node))
               {
                  // We do structural comparison to decide the equality of
                  // two memory operands. Otherwise, lexical comparison is
                  // performed.

                  useNode.IsKilledBySelf = true;
               }
            }
         }

         // Look up the latest definition node for this use

         DagNode latestDefNode = this.tagToDefNodeMap.Lookup(mayPOTag);
         
         if(latestDefNode != null)
         {

#if (PHX_DEBUG_CHECKS)

            Phx.Asserts.Assert(node != latestDefNode,
               "DAG.EnforceEvlOrderForDef, node != latestDefNode");

#endif

            if (latestDefNode.IsKilled || latestDefNode.IsExprNode
            	|| !this.aliasInfo.MayPartiallyOverlap(latestDefNode.Label, operand))
            {
		         this.AddEdge(node, latestDefNode, DagEdgeKind.OrderDep);
	         }
	         else
	         {
               // If the old node has not been processed yet and is an definition
               // node, invalidate the old node and all of its value parents 
               // so that they won't be recognized as common expression any 
               // more. Evaluation order is enforced during the invalidation

               // For an expression node, invalidation is not necessary. Simply 
               // enforcing evaluation order between the two nodes is enough

               this.InvalidateNode(latestDefNode, node);

               latestDefNode.IsKilled = true;

               if (DAG.CanKillEachOther(latestDefNode, node))
               {
                  // killed by the operand with same name
                  latestDefNode.IsKilledBySelf = true;
               }

            }
         }

         // Insert it as the latest node for the given tag
         
         this.tagToDefNodeMap.Insert(mayPOTag, node);
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Determine whether an assignement to the second node will cause the
   //    first node become invalid or useless.
   //
   // Returns:
   //
   //    True if the two nodes can kill each other. False otherwise.
   //
   //--------------------------------------------------------------------------

   private static bool
   CanKillEachOther
   (
      DagNode node1,
      DagNode node2
    )
   {
      if(node1.Label.IsMemoryOperand && node2.Label.IsMemoryOperand)
      {
         return DAG.CmpMemNode(node1, node2);
      }
      else
      {
         return DAG.CompareOperand(node1.Label, node2.Label);
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Add an edge between the parent node and the child node
   //
   // Arguments:
   //
   //    parent - The parent dag node (From node of the new edge)
   //    child - The child dag node (To node of the new edge)
   //    edgeKind - The kind of the new edge
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   private void
   AddEdge
   (
      DagNode parent,
      DagNode child,
      DagEdgeKind edgeKind
   )
   {
      if (child.IsRoot)
      {
         // it's not root any more
         child.IsRoot = false;
         this.rootList.Remove(child);
      }

      DagEdge newEdge = DagEdge.New(parent, child, edgeKind); 

      if (edgeKind != DagEdgeKind.OrderDep)
      {
         if (edgeKind == DagEdgeKind.AssignDep)
         {
            // Remove any orderDep edge between the given parent and child
            // inserted previously so that the assign edge always the last
            // edge to evaluate.
            // Note, edges are compared by their from/to nodes, not kind

            parent.ChildrenList.Remove(newEdge);
            child.ParentList.Remove(newEdge);

         }
         else
         {
            // Update the height of the parent node to the bigger of its old
            // height and the new child's height plus 1
            // Note, only valueDep edges change the height of the parent

            if ((child.Height + 1) > parent.Height)
            {
               parent.Height = child.Height + 1;
            }
         }

         parent.ChildrenList.Add(newEdge);
         
         child.ParentList.Add(newEdge);

         // update the number of real parents for the child
         child.NumRealParent++;
      }
      else
      {
         // The purpose of OrderDep edges are to enforce the ToNode of the edge
         // to be evaluated before the FromNode of the edge. All kinds of edges
         // will enforce this evaluation order. Therefore, we only need add one
         // OrderDep edge when there is no edge between the given two nodes.

         if (!parent.ChildrenList.Contains(newEdge))
         {
            parent.ChildrenList.Add(newEdge);
            child.ParentList.Add(newEdge);
         }
      }

   }
   
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A list of all roots in the DAG which is a forest
   //
   //--------------------------------------------------------------------------

   public DagNodeList RootList
   {
      get { return this.rootList; }
   }
   private DagNodeList rootList;

   // A hash map from an operand to the dag node it attaches to
   // It is used to query dag node information of an operand

   private OpndToNodeMap opndToNodeMap;

   // A hash map from an expression to an existing dag node
   // It is used to match common expression
   
   private ExprToNodeMap exprToNodeMap;

   // A hash map from alias tag to its may-partial-overlap definition nodes
   // It is used to enforce correct evaluation order between new uses and the
   // latest definitions, and new definitions and recent uses, and new definitions and old definitions
   // It is a single-value hashtable, i.e., we only keep the latest definition in 
   // the hashtable
   
   private TagToDefNodeMap tagToDefNodeMap;

   // A hash map from alias tag to its may-partial-overlap use nodes
   // It is used to enforce correct evaluation order between new definition and the
   // recent uses. It is a multi-value hashtable

   private TagToUseNodeMap tagToUseNodeMap;

   // Cached functionUnit and its aliasInfo
   
   private Phx.FunctionUnit functionUnit;
   private Phx.Alias.Info aliasInfo;

   // The previous call node. Used to enforce evaluation order between calls
   
   private DagNode lastCallNode;

}

//-----------------------------------------------------------------------------
//
// Description:
//
//    The kind of a DAG node
//
//-----------------------------------------------------------------------------

[System.Flags]
public enum DagNodeKind
{
   Use = 1,      // a constant/symbol/memory location refered as src operand
   Definition = 1<<1,      // an identifier refered as dst operand. It corresponds to
          // one entry in the identifier list mentioned in the dragon book 9.8
   Expression = 1<<2,     // an interior node representing a computation
   Memory = 1<<3       // a memory src or dst operand
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    A node in the DAG
//
//-----------------------------------------------------------------------------

public class DagNode 
{
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Static constructor for the DagNode
   //
   // Arguments:
   //
   //    label - The representative operand of this DagNode.
   //    hashCode - The hashCode of this DagNode
   //    nodeKind - The kind of this DagNode
   //
   // Returns:
   //
   //    DagNode  object.
   //
   //--------------------------------------------------------------------------

   public static DagNode
   New
   (
      Phx.IR.Operand label,
      int hashCode,
      DagNodeKind nodeKind
   )
   {
      DagNode newNode = new DagNode();
      newNode.label = label;
      newNode.newDestinationOperand = label;
      newNode.hashCode = hashCode;
      newNode.kind = nodeKind;
      
      // Initialize the height of the DagNode. At the time it's created, it's
      // always a leaf and leaf nodes always have height 1.

      newNode.height = 1;

      // Initialize members of the new DagNode instance

      newNode.parentList = new DagEdgeList();
      newNode.childrenList = new DagEdgeList();

      if (nodeKind == DagNodeKind.Expression)
      {
         newNode.srcList = new DagNodeList();
      }

      // At the time a DagNode is created, it's always a root in the DAG.

      newNode.isRoot = true;

      return newNode;
   }

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

   public Phx.IR.Operand Label
   {
      get { return label; }
      set { label = value; }
   }
   private Phx.IR.Operand label;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The parent list of this node 
   //
   //--------------------------------------------------------------------------

   public DagEdgeList ParentList
   {
      get { return parentList; }
   }
   private DagEdgeList parentList;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The number of non-OrderDep parents
   //
   //--------------------------------------------------------------------------
   public int NumRealParent
   {
      get { return numRealParent; }
      set { numRealParent = value; }
   }
   private int numRealParent;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The children list of this node 
   //
   //--------------------------------------------------------------------------

   public DagEdgeList ChildrenList
   {
      get { return childrenList; }
      set { childrenList = value; }
   }
   private DagEdgeList childrenList;

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

   public DagNodeList SrcList
   {
      get { return srcList; }
   }
   private DagNodeList srcList;


   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The kind of this dag node
   //
   //--------------------------------------------------------------------------

   public DagNodeKind Kind
   {
      get { return kind; }
   }
   private DagNodeKind kind;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Some properties for querying kind of the DagNode
   //
   //--------------------------------------------------------------------------

   public bool IsDefNode
   {
      get { return (kind & DagNodeKind.Definition) != 0; }
   }

   public bool IsUseNode
   {
      get { return (kind & DagNodeKind.Use) != 0; }
   }

   public bool IsExprNode
   {
      get { return (kind & DagNodeKind.Expression) != 0; }
   }

   public bool IsMemNode
   {
      get { return (kind & DagNodeKind.Memory) != 0; }
   }

   public bool IsSimpleUseNode
   {
      get 
      { 
         return ((kind & DagNodeKind.Use) != 0 
               && (kind & DagNodeKind.Memory) == 0);
      }
   }

   public bool IsSimpleDefNode
   {
      get 
      { 
         return ((kind & DagNodeKind.Definition) != 0 
               && (kind & DagNodeKind.Memory) == 0);
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

   public uint Height
   {
      get { return height; }
      set { height = value; }
   }
   private uint height;

  

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

   public Phx.IR.Operand NewDstOpnd
   {
      get { return newDestinationOperand; }
      set { newDestinationOperand = value; }
   }
   private Phx.IR.Operand newDestinationOperand;


   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The hashcode of this node
   //
   //--------------------------------------------------------------------------

   public int HashCode
   {
      get { return hashCode; }
      set { hashCode = value; }
   }
   private int hashCode;


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

   public bool NoTmp
   {
      get { return noTmp; }
      set { noTmp = value; }
   }
   private bool noTmp;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates whether the node is visited or not in current pass
   //
   //--------------------------------------------------------------------------

   public int VisitPass
   {
      get { return visitPass; }
      set { visitPass = value; }
   }
   private int visitPass;
   
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates whether new instructions are generated for the node   
   //
   //--------------------------------------------------------------------------

   public bool CodeGenerated
   {
      get { return codeGenerated; }
      set { codeGenerated = value; }
   }
   private bool codeGenerated;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates whether instructions must be generated for the node   
   //
   //--------------------------------------------------------------------------

   public bool MustGen
   {
      get { return mustGen; }
      set { mustGen= value; }
   }
   private bool mustGen;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates that the expression of this node has become invalid
   //    due to some changes in its subtree
   //
   //--------------------------------------------------------------------------

   public bool IsInvalid
   {
      get { return isInvalid; }
      set { isInvalid = value; }
   }
   private bool isInvalid;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates that the identifier of this node may have been 
   //    assigned a new value 
   //
   //--------------------------------------------------------------------------

   public bool IsKilled
   {
      get { return isKilled; }
      set { isKilled = value; }
   }
   private bool isKilled;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates that this node is queried at least once
   //    Used to decide whether to generate code for an ExpressionTemporary
   //
   //--------------------------------------------------------------------------

   public bool IsQueried
   {
      set { isQueried = value; }
   }
   private bool isQueried;

   public bool IsQueriedExprTmp
   {
      get
      {
         return (this.label.IsExpressionTemporary && this.isQueried);
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates that this node is killed by an operand with same
   //    name. This is used for safe dead store elimination
   //
   //--------------------------------------------------------------------------

   public bool IsKilledBySelf
   {
      get { return isKilledBySelf; }
      set { isKilledBySelf = value; }
   }
   private bool isKilledBySelf;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates that the node represents a code target
   //--------------------------------------------------------------------------

   public bool IsCallTarget
   {
      get { return isCallTarget; }
      set { isCallTarget = value; }
   }
   private bool isCallTarget;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A flag indicates whether this node is a root or not   
   //
   //--------------------------------------------------------------------------

   public bool IsRoot
   {
      get { return isRoot; }
      set { isRoot = value; }
   }
   private bool isRoot;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Check whether a dag node can be used as the dst operand for its
   //    target node, i.e., all of its children exception the target node 
   //    are processed
   //
   //--------------------------------------------------------------------------

   public bool IsDstCandidate
   {
      get
      {
         if (mustGen || isKilled)
         {
            // If a Definition node needs special care or it might be a dead store,
            // it's not a good candidate to be used as dst operand
            return false;
         }

         foreach(DagEdge edge in this.ChildrenList)
         {
            DagNode child = edge.ToNode;

            if (!child.CodeGenerated &&  this.targetNode != child
               && (child.MustGen
                  || child.NumRealParent > 0 
                  || !child.IsKilled))
            {
               // Some child is not ready yet
               return false;
            }
         }

         return true;
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The target value an identifier node (Definition) points to
   //
   //--------------------------------------------------------------------------

   public DagNode TargetNode
   {
      get
      {
         return targetNode;
      }
      set
      {
         targetNode = value;
      }
   }
   private DagNode targetNode;


   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The base/index/segment node of a memory operand node
   //
   //--------------------------------------------------------------------------

   public DagNode BaseNode
   {
      get
      {
         return baseNode;
      }
      set
      {
         baseNode = value;
      }
   }
   private DagNode baseNode;

   public DagNode IndexNode
   {
      get
      {
         return indexNode;
      }
      set
      {
         indexNode = value;
      }
   }
   private DagNode indexNode;

   public DagNode SegNode
   {
      get
      {
         return segNode;
      }
      set
      {
         segNode = value;
      }
   }
   private DagNode segNode;

}

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

public enum DagEdgeKind
{
   ValueDep = 0,     // an edge indicates value dependency between dagnodes
   OrderDep,   // an edge enforces evaluation order
   AssignDep   // an edge indicates parent will be assigned with value of child
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    An edge in the DAG
//
//-----------------------------------------------------------------------------

public class DagEdge : System.IComparable<DagEdge>, System.IEquatable<DagEdge>
{
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Static constructor for a dag edge.
   //
   // Arguments:
   //
   //    from - The From node of the new edge
   //    to - The To node of the new edge
   //    edgeKind - The kind of the new edge
   //
   // Returns:
   //
   //    DagEdge object.
   //
   //--------------------------------------------------------------------------

   public static DagEdge
   New
   (
      DagNode from,
      DagNode to,
      DagEdgeKind edgeKind
   )
   {
      DagEdge newEdge = new DagEdge();
      newEdge.fromNode = from;
      newEdge.toNode = to;
      newEdge.kind = edgeKind;

      // Initialize the priority of the new edge based on its kind.
      // Priority of an edge will be used to determine evaluation order

      if (edgeKind == DagEdgeKind.OrderDep)
      {
         // OrderDep edges have the highest priority
         newEdge.priority = int.MaxValue;
      }
      else if (edgeKind == DagEdgeKind.AssignDep)
      {
         // AssignDep edges always are evaluated last
         newEdge.priority = 0;
      }
      else
      {
         // ValueDep edges are ordered based on the height of their toNode
         newEdge.priority = newEdge.toNode.Height;
      }
      
      return newEdge;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The start node of this edge
   //
   //--------------------------------------------------------------------------

   public DagNode FromNode
   {
      get { return fromNode; }
   }
   private DagNode fromNode;


   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The end node of this edge
   //
   //--------------------------------------------------------------------------

   public DagNode ToNode
   {
      get { return toNode; }
   }
   private DagNode toNode;


   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The kind of dependency relationship this edge represents 
   //
   //--------------------------------------------------------------------------

   public DagEdgeKind Kind
   {
      get { return kind; }
   }
   private DagEdgeKind kind;

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

   public uint Priority
   {
      get { return priority; }
   }
   private uint priority;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Implement interface System.IComparable<DagEdge>
   //
   // Remarks:
   //
   //    Edges are sorted by their priority. Edges with higher priority are
   //    closer to the beginning of the list
   //
   //--------------------------------------------------------------------------

   public int CompareTo(DagEdge that)
   {
      if(this.Equals(that))
      {
         return 0;
      }
      else if (this.Priority > that.Priority)
      {
         return -1;
      }
      else
      {
         return 1;
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Implement interface System.IEquable<DagEdge>
   //
   //--------------------------------------------------------------------------

   public bool Equals(DagEdge that)
   {
      return (this.fromNode == that.fromNode && this.toNode == that.toNode);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    A wrapper of operand when it is used as a key into a hash map or a list.
//
//-----------------------------------------------------------------------------

public class OpndKey : System.IEquatable<OpndKey>
{
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
   
   public static OpndKey
   New
   (
      Phx.IR.Operand operand,
      int hashCode
   )
   {
      OpndKey newKey = new OpndKey();
      newKey.operand = operand;
      newKey.hashCode = hashCode;

      return newKey;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The operand this key represents   
   //
   //--------------------------------------------------------------------------

   public Phx.IR.Operand Operand
   {
      get { return operand; }
      set { operand = value; }
   }
   private Phx.IR.Operand operand;

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

   public override int GetHashCode()
   {
      return hashCode;
   }
   private int hashCode;

   
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Implement interface System.IEquable<OpndKey>
   //
   //--------------------------------------------------------------------------

   public bool Equals(OpndKey that)
   {
      if(this.GetHashCode() != that.GetHashCode())
      {
         return false;
      }

      Phx.IR.Operand opndA = this.Operand;
      Phx.IR.Operand opndB = that.Operand;

      
      return DAG.CompareOperand(opndA, opndB);
   }
}

//----------------------------------------------------------------------------
//
// Description:
//
//    A key of the node hash map. This class handles common subexpression 
//    recognization
//
//-----------------------------------------------------------------------------

public class NodeKey : System.IEquatable<NodeKey>
{
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

   public static NodeKey
   New
   (
      DagNode node,
      int hashCode
   )
   {
      NodeKey newKey = new NodeKey();
      newKey.node = node;
      newKey.hashCode = hashCode;

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
   
   public override int GetHashCode()
   {
      return hashCode;
   }
   private int hashCode;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Implement interface System.IEquable<LeaderOpndKey>
   //    The core methods that supports common expression recognization
   //
   //--------------------------------------------------------------------------

   public bool Equals(NodeKey that)
   {
      if (this.node.IsInvalid || that.node.IsInvalid)
      {
         // Invalidated node cannot be used as commond expression
         return false;
      }

      if (this.GetHashCode() != that.GetHashCode())
      {
         return false;
      }

      DagNode nodeA = this.node;
      DagNode nodeB = that.node;

      if (nodeA.IsDefNode)
      {
         // We always create a new definition node
         return false;
      }

      if (nodeA.IsMemNode != nodeB.IsMemNode
         || nodeA.IsExprNode != nodeB.IsExprNode)
      {
         // Not same kinds of nodes
         return false;
      }

      if (nodeA.IsExprNode)
      {
         // an expression, compare the two instructions structurely
         return DAG.CmpExprNode(nodeA, nodeB);
      }
      else if (nodeA.IsMemNode)
      {
         // memory operand node, compare the two node structurely
         return DAG.CmpMemNode(nodeA, nodeB);
      }
      else
      {
         // A simple node, compare labels of the two nodes lexically
         return DAG.CompareOperand(nodeA.Label, nodeB.Label);
      }
   }

   // The node this key represents   
   private DagNode node;
  
}


} // namespace LocalOpt
} // namespace Samples
} // namespace Phx
