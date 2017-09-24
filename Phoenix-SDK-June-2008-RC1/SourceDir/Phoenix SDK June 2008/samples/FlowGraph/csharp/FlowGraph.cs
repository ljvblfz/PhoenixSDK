//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    FlowGraph: A Phoenix plugin to build the flowgraph for each FunctionUnit
//    being compiled.  The Flowgraph is represented as a list of Basic Blocks,
//    and a list of Edges (representing the control-flow transfers between
//    those Blocks).
//
// Notes:
//
//    1.  This implementation is aimed to be easy to understand.  But it's very
//    inefficient, using linear searches along linked lists.  It's left as
//    an exercise for the reader to improve its performance, by choosing
//    more efficient data structures.
//
//    2.  Phoenix of course provides a builtin method to build the Flowgraph,
//    called via FunctionUnit.BuildFlowgraph().  This plugin is simply an
//    exercise for how you might build the flowgraph yourself.
//
// Usage:
//
//    cl -nologo -d2plugin:bin\debug\FlowGraph.dll <cpp_file>
//
//-----------------------------------------------------------------------------

using System;

namespace FlowGraph
{

   //-----------------------------------------------------------------------------
   //
   // Description:
   //
   //    Block descriptor.
   //    
   //-----------------------------------------------------------------------------

   public class Block 
   {
      public Block next;                      // next Block (no special order)
      private static int nextId = 1;          // ID for next-created Block
      public int id;                          // ID for current Block
      public int first, last;                 // first & last Instruction of Block

      public Block() 
      { 
         // Assign id's of 1, 2, 3, ...

         id = nextId++;
      }
   }

   //-----------------------------------------------------------------------------
   //
   // Description:
   //
   //    Graph class.
   //
   // Remarks: 
   //
   //    Represents a Control Flow Graph - a list of Blocks, and a list of Edges.
   //    
   //-----------------------------------------------------------------------------

   public class Graph
   {

      private int _nInstructions;                     // number of Instructions
      private  Phx.IR.Instruction[] _instructions;    // array of Instructions
      private  bool[] _leaders;                       // array of leader Instructions
      private  Block  _blocks;                        // linked list of Blocks
      private  Edge   _edges;                         // linked list of Edges

      //-----------------------------------------------------------------------------
      //
      // Description:
      //
      //    Nested class Edge
      //    
      //-----------------------------------------------------------------------------

      public class Edge
      {
         public Edge next;                         // next Edge (no special order)
         public int from, to;                      // ID of predecessor and successor

         //-----------------------------------------------------------------------------
         //
         // Description:
         //
         //    Constructor - f(rom) is the predecessor block id; t(o) is the successor
         //    block id.
         //    
         //-----------------------------------------------------------------------------

         public Edge(int f, int t)
         { 
            from = f;
            to = t;
         }
      }

      //-----------------------------------------------------------------------------
      //
      // Description:
      //
      //    Constructor
      //
      // Remarks:
      //
      //    Constructs a Control Flow Graph for a FunctionUnit.
      //
      //-----------------------------------------------------------------------------

      public Graph(Phx.FunctionUnit functionUnit)
      {           
         _nInstructions = CountInstructions(functionUnit);
         _instructions  = BuildArrayOfInstructions(functionUnit);
         _leaders = BuildLeaders();
         BuildBlocks();
         BuildEdges();
      }

      //-----------------------------------------------------------------------------
      //
      // Description:
      //
      //    Display the list of Basic Blocks that comprise the FlowGraph.
      //    
      //-----------------------------------------------------------------------------

      public void DumpBlocks()
      {              
         Block block = _blocks;
         for (; block != null; block = block.next)
         {
            System.Console.WriteLine("   Block {0} : first {1}={2} : last {3}={4}", block.id, 
               block.first, _instructions[block.first].Opcode.ToString(), 
               block.last,  _instructions[block.last].Opcode.ToString());
         }
      }

      //-----------------------------------------------------------------------------
      //
      // Description:
      //
      //    Display the list of Edges for the FlowGraph.
      //    
      //-----------------------------------------------------------------------------

      public void DumpEdges()
      {
         for (Edge e = _edges; e != null; e = e.next)
         {
            System.Console.WriteLine("   Edge : from-block {0} : to-block {1}", 
               e.from, e.to);
         }
      }

      //-----------------------------------------------------------------------------
      //
      // Description:
      //
      //    Return the count of instructions in the function unit.
      //    
      //-----------------------------------------------------------------------------

      private int CountInstructions(Phx.FunctionUnit functionUnit) {
         Phx.IR.Instruction i = functionUnit.FirstInstruction; int n = 0;
         while (i != null) { n++; i = i.Next; }
         return n;
      }

      //-----------------------------------------------------------------------------
      //
      // Description:
      //
      //    Store a function unit's instruction list in an array.  This provides
      //    faster, and easier, lookup when locating the destination of branch
      //    instructions.
      //    
      //-----------------------------------------------------------------------------

      private Phx.IR.Instruction[] BuildArrayOfInstructions(Phx.FunctionUnit functionUnit)
      {
         _instructions = new Phx.IR.Instruction[_nInstructions];
         Phx.IR.Instruction instruction = functionUnit.FirstInstruction;

         int n = 0;
         while (instruction != null)
         {
            _instructions[n++] = instruction;
            instruction = instruction.Next;
         }
         return _instructions;
      }

      //-----------------------------------------------------------------------------
      //
      // Description:
      //
      //    Return a bitmap (array of bools) stating which instructions are leaders.
      //
      // Remarks: 
      //
      //    A leader is the 0th instruction, the target of a branch or the 
      //    instruction immediately following a branch.
      //    
      //-----------------------------------------------------------------------------

      private bool[] BuildLeaders()
      {
         _leaders = new bool[_nInstructions];
         _leaders[0] = true;                        // 0th instruction is a leader
         for (int n = 1; n < _instructions.Length; n++)
         {
            Phx.IR.Instruction instruction = _instructions[n];
            if (instruction.IsBranchInstruction)
            {
               Phx.IR.BranchInstruction branchInstruction = instruction.AsBranchInstruction;
               int j;
               if (branchInstruction.BranchTargetInstruction != null) // GOTO
               {    
                  j = Array.IndexOf(_instructions, branchInstruction.BranchTargetInstruction);
                  _leaders[j] = true;
               }
               if (branchInstruction.TrueLabelInstruction != null) // CBRANCH
               {       
                  j = Array.IndexOf(_instructions, branchInstruction.TrueLabelInstruction);
                  _leaders[j] = true;
               }
               if (branchInstruction.FalseLabelInstruction != null) // CBRANCH
               {      
                  j = Array.IndexOf(_instructions, branchInstruction.FalseLabelInstruction);
                  _leaders[j] = true;
               }
               _leaders[n+1] = true;                  // after-branch
            }
         }
         return _leaders;
      }
      
      //-----------------------------------------------------------------------------
      //
      // Description:
      //
      //    Construct a linked list of (basic) blocks.
      //
      //-----------------------------------------------------------------------------

      private void BuildBlocks() {

         // 0th instruction always leader

         Block block = new Block();
         block.first = 0;       

         int n = 1;                                // instruction index
         while (n < _nInstructions)
         {
            if (_leaders[n])
            {
               // finish old block

               block.last = n-1;
               PrependBlock(block);        

               // start new block

               block = new Block();
               block.first = n++;       
            }
            else
            {
               n++;                               // keep looking for next leader
            }
         }
         block.last = n-1; PrependBlock(block);   // finish final block
      }

      //-----------------------------------------------------------------------------
      //
      // Description:
      //
      //    Prepend block to front of _blocks.
      //    
      //-----------------------------------------------------------------------------

      private void PrependBlock
      (
         Block block
      )
      {
         block.next = _blocks;
         _blocks = block;
      }


      //-----------------------------------------------------------------------------
      //
      // Description:
      //
      //    Build a linked list of edges.  (An Edge is represented simply as a pair
      //    of ints, holding the id of the edge's predecessor block, and successor
      //    block.
      //
      //-----------------------------------------------------------------------------

      private void BuildEdges()
      { 
         for (Block b = _blocks; b != null; b = b.next)
         {
            Phx.IR.Instruction last = _instructions[b.last];

            if (!last.IsBranchInstruction)
            {
               continue;
            }

            Phx.IR.BranchInstruction branchInstruction = last.AsBranchInstruction;
            int target;

            if (branchInstruction.BranchTargetInstruction != null)
            {                      // GOTO
               target = Array.IndexOf(_instructions, branchInstruction.BranchTargetInstruction);
            }
            else if (branchInstruction.TrueLabelInstruction != null)
            {                      // CBRANCH
               target = Array.IndexOf(_instructions, branchInstruction.TrueLabelInstruction);
            }
            else if (branchInstruction.FalseLabelInstruction != null)
            {                      // CBRANCH
               target = Array.IndexOf(_instructions, branchInstruction.FalseLabelInstruction);
            }
            else
            {
               continue;
            }
            Edge e = new Edge(b.id, target);

            e.next = _edges;
            _edges = e;            
         }
      }  
   }                                       // end of class Graph

}                                         // end of namespace FlowGraph
