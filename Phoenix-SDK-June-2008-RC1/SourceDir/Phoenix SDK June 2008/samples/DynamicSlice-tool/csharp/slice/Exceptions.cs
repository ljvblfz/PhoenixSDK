using System;
using System.Collections.Generic;
using System.Text;

namespace SliceAnalysis
{
   // Base class of all exceptions in the slice library.
   public class SliceException : Exception { }

   public class NoMatchingLineException : SliceException
   {
      private uint _lineNum;
      public NoMatchingLineException(uint lineNumber)
      {
         _lineNum = lineNumber;
      }

      public override string Message
      {
         get
         {
            return string.Format("There is no matching IR instruction for " +
                                 "the given source line number {0}.",
               _lineNum); ;
         }
      }
   }

   public class DefInstrNotFoundException : SliceException
   {
      private string _varName;

      public DefInstrNotFoundException(string variableName)
      {
         _varName = variableName;
      }

      public override string Message
      {
         get
         {
            return "DefinitionInstruction for " + _varName + " could not be found.";
         }
      }
   }

   public class OpcodeNotImplementedException : SliceException
   {
      private string _opcode;

      public OpcodeNotImplementedException(string opcode)
      {
         _opcode = opcode;
      }

      public override string Message
      {
         get
         {
            return "Opcode " + _opcode + " not implemented.";
         }
      }
   }

   public class UnknownFundamentalTypeException : SliceException
   {
      private string _type;

      public UnknownFundamentalTypeException(string type)
      {
         _type = type;
      }

      public override string Message
      {
         get
         {
            return "Unknown Fundamental Type " + _type;
         }
      }
   }

   public class EvaluatorException : SliceException
   {
      private string _var;

      public EvaluatorException(string variable)
      {
         _var = variable;
      }

      public override string Message
      {
         get
         {
            return "Variable " + _var + " could not be evaluated.";
         }
      }
   }
}
