using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Lispkit
{  
   /// <summary>
   /// Central data store for tracking line and column
   /// numbers.
   /// </summary>
   class Location
   {
      // Parallel lists of line and column numbers.
      private static List<int> lines = new List<int>();
      private static List<int> columns = new List<int>();

      // Maps the given line and column number to the given 
      // numeric identifier.
      public static void Add(int ID, int line, int column)
      {
         // Ensure capacity of the lists.
         while (lines.Count <= ID)
         {
            lines.Add(-1);
            columns.Add(-1);
         }
         Debug.Assert(lines.Count == columns.Count);

         // Add the line and column information.
         lines[ID] = line;
         columns[ID] = column;
      }

      // Retrieves the line number that maps to the given 
      // numeric identifier.
      public static int GetLine(int ID)
      {
         return lines[ID];
      }

      // Retrieves the column number that maps to the given 
      // numeric identifier.
      public static int GetColumn(int ID)
      {
         return columns[ID];
      }
   }  
}