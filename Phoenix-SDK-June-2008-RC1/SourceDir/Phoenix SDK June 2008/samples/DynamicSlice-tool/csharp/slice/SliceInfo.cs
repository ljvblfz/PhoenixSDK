using System;
using System.IO;
using System.Collections.Generic;
using System.Text;

namespace SliceAnalysis
{
   // Stores all relevant information about a slice.
   public class SliceInfo
   {
      private List<Phx.IR.Instruction> _instrList = new List<Phx.IR.Instruction>();

      public void Add(Phx.IR.Instruction instruction)
      {
         _instrList.Add(instruction);
      }

      public List<uint> DumpOffsets()
      {
         List<uint> Lines;

         Lines = new List<uint>();

         foreach (Phx.IR.Instruction Instruction in _instrList)
         {
            uint LineNumber = (uint)Instruction.OriginalInstructionByteOffset;
            if (Lines.Contains(LineNumber) == false &&
                Instruction.IsReal &&
                !Instruction.IsSsa)
               Lines.Add(LineNumber);
         }

         Lines.Sort();
         foreach (uint Line in Lines)
            System.Diagnostics.Debug.WriteLine(Line);

         return Lines;
      }

      public List<uint> DumpLine()
      {
         List<uint> Lines;

         Lines = new List<uint>();

         foreach (Phx.IR.Instruction Instruction in _instrList)
         {
            uint LineNumber = Instruction.GetLineNumber();
            if (Lines.Contains(LineNumber) == false)
               Lines.Add(LineNumber);
         }

         Lines.Sort();
         foreach (uint Line in Lines)
            System.Diagnostics.Debug.WriteLine(Line);

         return Lines;
      }

      public string DumpCode(string filePath)
      {
         string[] lines = File.ReadAllLines(filePath);
         StringBuilder builder = new StringBuilder();
         List<uint> lineNums = new List<uint>();
         foreach (Phx.IR.Instruction instruction in _instrList)
         {
            uint lineNumber = instruction.GetLineNumber();
            if (!lineNums.Contains(lineNumber))
            {
               lineNums.Add(lineNumber);
            }
         }
         lineNums.Sort();
         foreach (uint linenum in lineNums)
         {
            string code =
              string.Format("{0}\t{1}", linenum, lines[linenum - 1]);
            builder.AppendLine(code);
         }
         return builder.ToString();
      }

      public string DumpIR()
      {
         StringBuilder builder = new StringBuilder();
         if (_instrList.Count > 0)
         {
            // Assuming intra-procedural slices only.
            Phx.Unit unit = _instrList[0].Unit;
            int id = 1;
            foreach (Phx.IR.Instruction instruction in unit.Instructions)
            {
               string baseString = string.Format(" [{2}] L{0}\t{1}",
                   instruction.GetLineNumber(), instruction.ToString(), id++);
               if (_instrList.Contains(instruction))
               {
                  builder.Append("*");
               }
               else
               {
                  builder.Append(" ");
               }
               builder.AppendLine(baseString);
            }
         }
         return builder.ToString();
      }

      // This function takes in a list of unreachable basic blocks and 
      // throws away the intersection of the IR instructions in the 
      // slice and the unreachable basic blocks.
      internal void UnreacheableFilter(Phx.BitVector.Fixed blocks)
      {
         // Do nothing if there are no unreachable basic blocks.
         if (blocks != null)
         {
            List<Phx.IR.Instruction> tempList = new List<Phx.IR.Instruction>();
            foreach (Phx.IR.Instruction instruction in _instrList)
            {
               if (blocks.GetBit(instruction.BasicBlock.Id) == false)
               {
                  tempList.Add(instruction);
               }
            }
            _instrList = tempList;
         }
      }

      // This function discards any SSA instruction from the slice.
      internal void SSAInstrFilter()
      {
         List<Phx.IR.Instruction> tempList = new List<Phx.IR.Instruction>();
         foreach (Phx.IR.Instruction instruction in _instrList)
         {
            if (!instruction.IsSsa)
            {
               tempList.Add(instruction);
            }
         }
         _instrList = tempList;
      }

      // This function discards any instruction without line number 
      // information.
      internal void NoLineInstrFilter()
      {
         List<Phx.IR.Instruction> tempList = new List<Phx.IR.Instruction>();
         foreach (Phx.IR.Instruction instruction in _instrList)
         {
            // We compare with zero because 'no line number information'
            // is represented by GetLineNumber returning a zero.
            if (instruction.GetLineNumber() != 0)
            {
               tempList.Add(instruction);
            }
         }
         _instrList = tempList;
      }
   }
}
