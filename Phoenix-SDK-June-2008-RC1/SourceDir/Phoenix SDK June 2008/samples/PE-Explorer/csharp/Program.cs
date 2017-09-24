using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace PEExplorer
{
   static class Program
   {
      /// <summary>
      /// The main entry point for the application.
      /// </summary>
      [STAThread]
      static int Main(string[] arguments)
      {
         Phx.Term.Mode termMode = Phx.Term.Mode.Normal;

         // Initialize the Phoenix framework.
         GlobalData.InitializePhoenix(arguments);
         
         Application.EnableVisualStyles();
         Application.SetCompatibleTextRenderingDefault(false);

         // Run the application in a try/catch block.
         // An unhandled exception will be reported to the user
         // through a message box.
         try
         {
            Application.Run(new Viewer());
         }
         catch (Exception e)
         {
            MessageBox.Show(string.Format(
                  "Unhandled exception: {0}{1}Stack trace:{1}{2}", 
                  e.Message, Environment.NewLine, e.StackTrace),
               "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
               
            termMode = Phx.Term.Mode.Fatal;
         }
         
         GlobalData.Shutdown();
         
         Phx.Term.All(termMode);
         return (termMode == Phx.Term.Mode.Normal ? 0 : 1);
      }      
   }
}