using System;
using System.Collections.Generic;
using System.Text;

namespace PEExplorer
{
   /// <summary>
   /// Allows the user to browse for a file.
   /// </summary>
   interface IFileBrowser
   {
      /// <summary>
      /// Allows the user to browse for the file with the given name.
      /// </summary>
      string Browse(string originalFile);
   }
}
