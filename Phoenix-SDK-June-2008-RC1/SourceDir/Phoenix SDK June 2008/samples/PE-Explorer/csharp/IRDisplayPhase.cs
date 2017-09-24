using System;
using System.Collections.Generic;
using System.Text;

namespace PEExplorer
{
   /// <summary>
   /// Custom phase that builds a string representation for the 
   /// IR of a FunctionUnit object.
   /// </summary>
   class IrDisplayPhase : Phx.Phases.Phase
   {
      // Text that is appended before the IR.
      private string headerText;
      // Text that is appended after the IR.
      private string footerText;
      
      // The resulting Disassembly object.
      private Disassembly disassembly;

      /// <summary>
      /// Gets the resulting Disassembly object.
      /// </summary>
      public Disassembly Disassembly
      {
         get { return this.disassembly; }
      }

      /// <summary>
      /// Gets or sets the text to append before the 
      /// function unit IR.
      /// </summary>
      public string HeaderText
      {
         get { return this.headerText; }
         set { this.headerText = value; }
      }

      /// <summary>
      /// Gets or sets the text to append after the 
      /// function unit IR.
      /// </summary>
      public string FooterText
      {
         get { return this.footerText; }
         set { this.footerText = value; }
      }

      public static IrDisplayPhase
      New
      (
         Phx.Phases.PhaseConfiguration config
      )
      {
         IrDisplayPhase phase = new IrDisplayPhase();
         phase.Initialize(config, "IR Display");
         return phase;
      }

      protected override void Execute(Phx.Unit unit)
      {
         if (!unit.IsFunctionUnit)
         {
            return;
         }

         Phx.FunctionUnit functionUnit = unit.AsFunctionUnit;

         StringBuilder stringBuilder = new StringBuilder();

         // In this method, we build line and offset information for 
         // each line of IR in the function unit.         
         List<int> lines = new List<int>();
         List<int> offsets = new List<int>();

         // Maps operands to their respective types.
         Dictionary<string, string> operandTypeMap = null;
         
         // Append header text.
         int index = headerText.IndexOf(Environment.NewLine, 0);
         while (index >= 0)
         {
            lines.Add(-1);
            offsets.Add(-1);            
            index = headerText.IndexOf(Environment.NewLine, index+1);
         }
         stringBuilder.Append(headerText);
         
         // If the function unit is in EIR form, we get the encoded data
         // from the EncodedIRVector property.
         if (functionUnit.HasOnlyEncodedIR)
         {
            stringBuilder.AppendLine();
            foreach (Phx.IR.EncodedIR encodedIR 
               in functionUnit.EncodedIRVector)
            {
               stringBuilder.AppendLine(
                  IrDisplayPhase.GetEncodedIRString(encodedIR));
            }
         }
         // Otherwise, iterate over the instruction list and record 
         // each instruction, together with its corresponding line and 
         // offset.
         else
         {
            // Build flow graph.
            functionUnit.BuildFlowGraphWithoutEH();

            // Map operands to their respective types.
            operandTypeMap = new Dictionary<string, string>();
            
            foreach (Phx.IR.Instruction instruction 
               in functionUnit.Instructions)
            {
               // Insert a line of whitespace between blocks.
               if (instruction.IsLabelInstruction)
               {
                  stringBuilder.AppendLine();
                  lines.Add(-1);
                  offsets.Add(-1);
               }
            
               // Add the instruction string, line, and offset to our 
               // mappings.
               
               string instructionString = instruction.ToString();
               int lineNumber = (int)instruction.GetLineNumber();
               int msilOffset = (int)instruction.GetMsilOffset();

               // Add operands to type map.
               foreach (Phx.IR.Operand operand
                  in instruction.SourceAndDestinationOperands)
               {                  
                  // Operand must have type.
                  if (operand.HasType)
                  {  
                     string displayName = operand.ToString();

                     if (!operandTypeMap.ContainsKey(displayName))
                     {
                        // Transform the value of the SymbolKind property 
                        // into a readable string. For example, format 
                        // "LocalVariable" as "(local variable)".
                        string symbolKind = string.Empty;

                        // If the operand represents a register, set
                        // the kind string to a descriptive term.
                        if (operand.IsRegister)
                        {
                           symbolKind = "(register)";
                        }
                        // If the operand is symbolic, set the 
                        // kind string to the SymbolKind of the symbol.
                        else if (operand.Symbol != null)
                        {
                           // Use the name of the enumeration member
                           // to create the initial string.
                           string rawSymbolKind = 
                              operand.Symbol.SymbolKind.ToString();
                           
                           // If the symbol represents a method parameter
                           // (a local variable symbol with Parameter 
                           // storage class) or the 'this' pointer, reset
                           // the kind string to a more descriptive term.
                           if (operand.Symbol.IsLocalVariableSymbol)
                           {
                              Phx.Symbols.LocalVariableSymbol local = 
                                 operand.Symbol.AsLocalVariableSymbol;
                              if (local.IsThisParameter)
                              {
                                 rawSymbolKind = "this";
                              }
                              else if (local.StorageClass == 
                                Phx.Symbols.StorageClass.Parameter)
                              {
                                 rawSymbolKind = "parameter";
                              }
                           }
                           
                           // Iterate over the string and insert a 
                           // space each time we encounter an 
                           // upper-case letter.
                           foreach (char ch in rawSymbolKind)
                           {
                              if (Char.IsUpper(ch))
                                 symbolKind += ' ';
                              symbolKind += ch;
                           }
                           
                           // Create final formatted string.
                           symbolKind = string.Format("({0}) ", 
                              symbolKind.Trim().ToLower());
                        }

                        // Add entry to map.
                        string type = string.Empty;
                        if (!operand.IsRegister)
                        {
                           type = Driver.GetFriendlyTypeName(operand.Type, false);
                        }
                        operandTypeMap.Add(displayName, string.Format("{0}{1} {2}",
                           symbolKind, type, displayName));
                     }
                  }
               }            
               
               stringBuilder.AppendLine(instructionString);
               lines.Add(lineNumber);
               offsets.Add(msilOffset);
               
               // If the instruction string uses multiple lines, add 
               // additional entries to the line number and offset mappings.
               index = instructionString.IndexOf(Environment.NewLine, 0);
               while (index >= 0)
               {
                  lines.Add(lineNumber);
                  offsets.Add(msilOffset);
                  index = instructionString.IndexOf(Environment.NewLine, 
                     index + 1);
               }            
            }
         }
         
         // Append footer text.
         index = footerText.IndexOf(Environment.NewLine, 0);
         while (index >= 0)
         {
            lines.Add(-1);
            offsets.Add(-1);
            index = footerText.IndexOf(Environment.NewLine, index+1);
         }
         stringBuilder.AppendLine(footerText);
                  
         this.disassembly = new Disassembly(
            stringBuilder.ToString(), lines.ToArray(), offsets.ToArray());
         this.disassembly.IdentifierTypeMap = operandTypeMap;
      }

      /// <summary>
      /// Converts the provided EncodedIR objects into a string
      /// representation.
      /// </summary>
      private static string GetEncodedIRString(Phx.IR.EncodedIR encodedIR)
      {
         StringBuilder stringBuilder = new StringBuilder();
         
         stringBuilder.AppendFormat("----- Encoded IR for {0} at {1}",
            encodedIR.FunctionSymbol.QualifiedName,
            Phx.Output.ToHexString(
               encodedIR.FunctionSymbol.OriginalRva, 8, false));
         stringBuilder.AppendLine();
         
         stringBuilder.AppendLine(encodedIR.PrimaryDataInstruction.ToString());

         if (encodedIR.HeaderDataInstruction != null)
         {
            stringBuilder.AppendLine("- Header");
            stringBuilder.AppendLine(encodedIR.HeaderDataInstruction.ToString());
         }
         if (encodedIR.EHDataInstruction != null)
         {
            stringBuilder.AppendLine("- EH data");
            stringBuilder.AppendLine(encodedIR.EHDataInstruction.ToString());
         }
         
         return stringBuilder.ToString().Trim();
      }
   }
}
