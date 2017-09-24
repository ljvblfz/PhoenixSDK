using System;
using System.Collections.Generic;
using System.Text;

namespace SliceAnalysis
{
   // Interface to evaluate the value of a variable in the context of
   // debugging/dynamic slicing.
   public interface IEvaluator
   {
      int Evaluate(string variable);
   }
}
