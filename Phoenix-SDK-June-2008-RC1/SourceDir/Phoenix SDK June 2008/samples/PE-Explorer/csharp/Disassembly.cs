using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

namespace PEExplorer
{
   /// <summary>
   /// Describes the disassembly of an element. A disassembly consists
   /// of a text string together with line number and offset information.
   /// </summary>
   internal class Disassembly
   {
      // The disassembly text.
      private string text;
      // Array of line numbers. Each element corresponds to a line of text.
      private int[] lineNumbers;
      // Array of offsets. Each element corresponds to a line of text.
      private int[] msilOffsets;
      // Maps identifiers to their types.
      private Dictionary<string, string> identifierTypeMap;

      /// <summary>
      /// Gets the text string associated with the disassembly.
      /// </summary>
      internal string Text
      {
         get { return this.text; }
      }

      /// <summary>
      /// Gets the line numbers associated with the disassembly.
      /// </summary>
      internal int[] LineNumbers
      {
         get { return this.lineNumbers; }
      }

      /// <summary>
      /// Gets the offsets associated with the disassembly.
      /// </summary>
      internal int[] MsilOffsets
      {
         get { return this.msilOffsets; }
      }

      /// <summary>
      /// Gets or sets the identifier type map associated with the disassembly.
      /// </summary>
      internal Dictionary<string, string> IdentifierTypeMap
      {
         set { this.identifierTypeMap = value; }
         get { return this.identifierTypeMap; }
      }
      
      /// <summary>
      /// Constructs a new Disassembly object.
      /// </summary>
      internal Disassembly(string text)
         : this(text, null, null)
      {
      }

      /// <summary>
      /// Constructs a new Disassembly object.
      /// </summary>
      internal Disassembly(string text, int[] lineNumbers, int[] msilOffsets)
      {
         if (lineNumbers == null)
            lineNumbers = new int[] { -1 };
         if (msilOffsets == null)
            msilOffsets = new int[] { -1 };
         Debug.Assert(lineNumbers.Length == msilOffsets.Length);
         
         this.text = text;
         this.lineNumbers = lineNumbers;
         this.msilOffsets = msilOffsets;
         this.identifierTypeMap = null;
      }
   }
}
