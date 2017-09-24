using System;
using System.Collections.Generic;
using System.Text;
using OC = Phx.Targets.Architectures.X86.Opcode.Index;

namespace SliceAnalysis
{
   // Bridges between Phoenix and Visual Studio.
   class X86InsnLookupTable
   {
      private static IEvaluator evaluator;
      public static IEvaluator Evaluator
      {
         get
         {
            return evaluator;
         }
         set
         {
            evaluator = value;
         }
      }

      public static string OpndToName(Phx.IR.Operand operand)
      {
         if (operand.Symbol != null &&
             !operand.Symbol.Type.IsAggregate &&
             !operand.IsTemporary)
         {
            return operand.Symbol.NameString;
         }
         else
         {
            return null;
         }
      }

      public static Phx.Constant.IntValue EvaluateOpnd(Phx.IR.Operand operand)
      {
         int result = 0;
         try
         {
            result = evaluator.Evaluate(OpndToName(operand));
         }
         catch (EvaluatorException)
         {
            return null;
         }

         return Phx.Targets.Architectures.X86.X86IntValue.New(operand.FunctionUnit.ConstantTable, result);
      }

      // see ms-help://MS.VSCC.v80/MS.MSDN.v80/MS.VisualStudio.v80.en/dv_vclang/html/3691ceca-05fb-4b82-b1ae-5c4618cda91a.htm
      public static string DebuggerTypeToPhxType(string dbgType)
      {
         switch (dbgType)
         {
            case "char":
            case "bool":
               return "i8";
            case "short":
               return "i16";
            case "int":
            case "long":
               return "i32";
            case "__int64":
               return "i64";
            case "float":
               return "f32";
            case "double":
               return "f64";
            case "wchar_t":
               return "u16";
         }

         throw new UnknownFundamentalTypeException(dbgType);
      }

      public static Phx.IR.Operand Lookup(uint operandIndex, Phx.IR.Instruction instruction)
      {
         Phx.OpcodeTable table = instruction.Opcode.Table;
         Phx.IR.Operand operandList;

         switch ((OC)table.GetIndex(instruction.Opcode))
         {
            case OC.call:
            case OC.cmp:
            case OC.push:
               operandList = instruction.SourceOperandList;
               break;
            case OC.lea:
            case OC.mov:
            case OC.pop:
            case OC.setcc:
               if (operandIndex == 0)
               {
                  operandList = instruction.DestinationOperandList;
               }
               else
               {
                  operandList = instruction.SourceOperandList;
                  operandIndex = 0;
               }
               break;
            case OC.add:
            case OC.and:
            case OC.sub:
            case OC.xor:
               if (operandIndex == 0)
               {
                  operandList = instruction.DestinationOperandList;
               }
               else
               {
                  operandList = instruction.SourceOperandList;
               }
               break;
            default:
               throw new OpcodeNotImplementedException(instruction.Opcode.NameString);
         }

         Phx.IR.Operand operand = operandList;
         for (int i = 0; i < operandIndex; i++)
         {
            System.Diagnostics.Debug.Assert(operand != null);
            operand = operand.Next;
         }
         return operand;
      }
   }
}
