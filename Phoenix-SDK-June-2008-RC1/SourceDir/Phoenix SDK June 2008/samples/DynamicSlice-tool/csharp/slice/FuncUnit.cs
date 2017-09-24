using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;

namespace SliceAnalysis
{
   /// <summary>
   /// FunctionUnit wraps around the Phoenix FunctionUnit class.  Most analysis on a
   /// FunctionUnit is done in this class, and is done automatically when an
   /// instance of the class is instantiated.  
   /// 
   /// Currently, this class performs the following computation:
   /// 1. The program dependence graph, control dependence graph, and data
   ///    dependence graph are constructed.
   /// 2. Unreacheable blocks are identified.
   /// </summary>
   class FunctionUnit
   {
      #region Member Variables
      /// <summary>
      /// The wrapped phoenix FunctionUnit object.
      /// </summary>
      private Phx.FunctionUnit _funcUnit;

      /// <summary>
      /// Returns the wrapped Phoenix FunctionUnit object.
      /// </summary>
      internal Phx.FunctionUnit PhxFuncUnit { get { return _funcUnit; } }

      private Dictionary<uint, List<Phx.IR.Instruction>> _lineMappings;

      /// <summary>
      /// Returns the map from source code line numbers to IR instructions.
      /// </summary>
      internal Dictionary<uint, List<Phx.IR.Instruction>> LineMap
      { get { return _lineMappings; } }

      /// <summary>
      /// The dependence graphs we build.
      /// </summary>
      private Phx.Graphs.Graph _programDepGraph;

      /// <summary>
      /// Returns the program dependence graph.
      /// </summary>
      internal Phx.Graphs.Graph ProgramDepGraph
      { get { return _programDepGraph; } }

      /// Mapping from an instruction in this FunctionUnit to a node in the 
      /// program dependence graph.
      /// </summary>
      private Dictionary<Phx.IR.Instruction, Phx.Graphs.Node> _progiton =
          new Dictionary<Phx.IR.Instruction, Phx.Graphs.Node>();

      internal Dictionary<Phx.IR.Instruction, Phx.Graphs.Node> InstrToNode
      { get { return _progiton; } }

      /// <summary>
      /// Mapping from a node in the program dependence graph to an
      /// instruction in this FunctionUnit.
      /// </summary>
      private Dictionary<Phx.Graphs.Node, Phx.IR.Instruction> _progntoi =
          new Dictionary<Phx.Graphs.Node, Phx.IR.Instruction>();

      internal Dictionary<Phx.Graphs.Node, Phx.IR.Instruction> NodeToInstr
      { get { return _progntoi; } }

      #endregion

      internal void CreateLineMappings()
      {
         if (_lineMappings != null) return;  // Only create the mappings once.

         _lineMappings = new Dictionary<uint, List<Phx.IR.Instruction>>();

         foreach (Phx.IR.Instruction instruction in _funcUnit.Instructions)
         {
            //System.Diagnostics.Debug.Print(instruction.ToString());
            List<Phx.IR.Instruction> list;
            uint lineNumber = instruction.GetLineNumber();
            if (!_lineMappings.ContainsKey(lineNumber))
            {
               list = new List<Phx.IR.Instruction>();
               _lineMappings.Add(lineNumber, list);
            }
            else
            {
               list = _lineMappings[lineNumber];
            }
            list.Add(instruction);
         }
      }

      internal void CreateNodeMappings()
      {
         _programDepGraph = Phx.Graphs.Graph.New(_funcUnit);
         foreach (Phx.IR.Instruction instruction in _funcUnit.Instructions)
         {
            // Create a node in the graph for each instruction in the function 
            // unit, and create mappings internally.
            Phx.Graphs.Node n3 = _programDepGraph.NewNode();
            _progiton.Add(instruction, n3);
            _progntoi.Add(n3, instruction);
         }
      }

      // ExpressionAnalyzer is actually called during the raise method. However (I
      // think) it expects the IR to be in Hir (which apparently isn't 
      // happening) so I'm trying to mimic it over here minimally.
      internal void MimicExprAnalyzer()
      {
         Phx.Alias.Info aliasInfo = _funcUnit.AliasInfo;

         foreach (Phx.IR.Instruction instruction in _funcUnit.Instructions)
         {
            foreach (Phx.IR.Operand operand in instruction.DestinationAndSourceOperands)
            {
               if (operand.IsAddress)
               {
                  Phx.Symbols.Symbol sym = operand.Symbol;

                  if ((sym != null) && sym.IsLocalVariableSymbol)
                  {
                     Phx.IR.Operand indirectOperand =
                         Phx.Expression.Info.FindRootMemoryOperand(operand);

                     if ((indirectOperand == null) || (indirectOperand.IsAddress == true))
                     {
                        Phx.Symbols.LocalVariableSymbol localVariableSymbol = sym.AsLocalVariableSymbol;
                        Phx.Types.Field field = localVariableSymbol.Type.PrimaryField;

                        aliasInfo.MarkAddressTakenTag(aliasInfo.GetTag(
                            localVariableSymbol.Id, field));
                     }
                  }
               }
            }
         }
      }

      // Unfortunately Phoenix currently requires that the IR be in Hir to 
      // perform alias analysis.  To do it at a lower level we need to 
      // manually massage the IR instructions a little bit.
	  // Note: Raise to High level IR is currently not implemented for certain 
      // architectures such as x86. So raise to symbolic IR and fixup the
      // alias info.	  
      internal void AliasAnalysis()
      {
         // Here's where pointer aliasing is being handled.
         MimicExprAnalyzer();

		 // Delete flow graph before building SSA
         _funcUnit.DeleteFlowGraph();

         // Build new SSA info based on the side effect information.
         _funcUnit.BuildSsaInfo(Phx.SSA.BuildOptions.DefaultAliased | Phx.SSA.BuildOptions.HashIndirections);

#if (PHX_DEBUG_CHECKS)
         Phx.Asserts.Assert(_funcUnit.SsaInfo != null);
#endif
      }

      private void BuildDataDependenceGraph()
      {
         // Create edges from instructions to their DefinitionInstruction's.
         foreach (Phx.IR.Instruction instruction in _funcUnit.Instructions)
         {
            foreach (Phx.IR.Operand operand in instruction.SourceOperands)
            {
               // For branch instructions, the operands are labels whose 
               // definitions are the labels themselves which we don't 
               // really need in our dependence graph.

               // Also for operands like constants don't have a definition
               // so we need to check for that too.
               if (operand.DefinitionInstruction != null && !operand.DefinitionInstruction.IsLabelInstruction)
               {
                  _programDepGraph.NewEdge(_progiton[instruction], 
                                           _progiton[operand.DefinitionInstruction]);
               }
            }
         }
      }

      private void BuildControlDependenceGraph()
      {
         // Build the flowgraph.
         _funcUnit.BuildFlowGraphWithoutEH();

         // Augment flowgraph to add false edges for unconditional jumps
         // to correctly slice arbitrary control flow graphs according to
         // Ball's paper.
         Phx.Graphs.FlowGraph F = _funcUnit.FlowGraph;

         for (Phx.Graphs.BasicBlock B = F.BlockList; B != null; B = B.Next)
         {
            if (B.LastInstruction is Phx.IR.BranchInstruction)
            {
               Phx.IR.BranchInstruction branchInstruction = B.LastInstruction as Phx.IR.BranchInstruction;

               if (branchInstruction.IsUnconditional)
               {
                  Phx.Graphs.BasicBlock C = B.LastInstruction.Next.BasicBlock;
                  // Don't add an edge if there already exists one.
                  bool edgeExists = false;
                  for (Phx.Graphs.Edge e = B.SuccessorEdgeList; e != null; e = e.NextSuccessorEdge)
                  {
                     if (e.SuccessorNode == C)
                     {
                        edgeExists = true;
                        break;
                     }
                  }

                  if (!edgeExists)
                  {
                     F.NewEdge(B, C);
                  }
               }
            }
         }
         
         _funcUnit.FlowGraph.BuildPostDominanceFrontiers();

         // iterate through every basic block in the flowgraph
         for (Phx.Graphs.Node node = _funcUnit.FlowGraph.NodeList;
             node != null;
             node = node.Next)
         {
            Phx.Graphs.BasicBlock Y = node.AsBasicBlock;

            // Get the post-dominance frontier of node
            Phx.BitVector.Sparse postdom = _funcUnit.FlowGraph.PostDominanceFrontier(node);

            // iterate through the post dominance frontier
            foreach (uint bitNumber in postdom)
            {
               Phx.Graphs.BasicBlock X = _funcUnit.FlowGraph.Node(bitNumber).AsBasicBlock;

               // iterate through every instruction of Y and make it control 
               // dependent on the last instruction of X
               foreach (Phx.IR.Instruction instruction in Y.Instructions)
               {
#if (PHX_DEBUG_CHECKS)
                  // assert last instruction of Y is a Jcc or a switch 
                  // instruction.
                  Phx.Asserts.Assert(X.LastInstruction.IsBranchInstruction 
                      || X.LastInstruction.IsSwitchInstruction);
#endif
                  _programDepGraph.NewEdge(_progiton[instruction], 
                                           _progiton[X.LastInstruction]);
               }
            }
         }
      }

      internal void BuildLoopInfo()
      {
         _funcUnit.BuildLoopInfo();
      }

      /// <summary>
      /// Based on the Phoenix FunctionUnit object provided, performs analysis 
      /// on the given FunctionUnit and keeps the result.
      /// </summary>
      /// <param name="functionUnit"></param>
      public FunctionUnit(Phx.FunctionUnit functionUnit)
      {
         _funcUnit = functionUnit;

         AliasAnalysis();
         CreateNodeMappings();
         BuildDataDependenceGraph();
         BuildControlDependenceGraph();
      }
   }
}
