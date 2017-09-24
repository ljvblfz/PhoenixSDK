using System;
using System.IO;
using System.Reflection;

/// <summary>
/// The main application class. This class parses the command line,
/// calls the $Lispkit.$Main method, and prints the result to the console.
/// </summary>
class Program
{
	public static void Main(string[] args)
	{
      // We allow a mix of literal strings and file names as the 
      // command-line arguments. For example, if we assume that the 
      // file that is named 'params.txt' contains the 
      // string "a b c d", then the argument list:
      // ( params.txt )
      // is read as the S-expression "(a b c d)".

      string arguments = string.Empty;
      foreach (string argument in args)
      {
         if (File.Exists(argument))
         {
            arguments += File.ReadAllText(argument);
         }
         else
         {
            arguments += argument + " ";
         }
      }
      arguments = arguments.Trim();

      try
      {
         // Translate the argument list string into an S-expression list.

         Lispkit.Sexp.SExp argList = 
            new Lispkit.Sexp.SExpReader(arguments).Process();

         // The entry point for any program is $Lispkit.$Main. Call this method
         // by using Reflection.

         // Retrive method information for the $Lispkit.$Main method.
         Type type = Assembly.GetExecutingAssembly().GetType("$Lispkit");
         System.Reflection.MethodInfo mainMethodInfo = type.GetMethod("$Main");

         // Invoke the method with the argument list.
         object result = mainMethodInfo.Invoke(null, new object[] { argList });

         // Print the result of the program to the console.
         string s = (result == null) ? "(null)" : result.ToString();
         Console.WriteLine(s);
      }
      // Print the exception details to the console.
      catch (Exception e)
      {
         Console.WriteLine("error: {0}", e.Message);
         Console.WriteLine("stack trace: \n{0}", e.StackTrace);
         Console.WriteLine();
      }
	}
}
