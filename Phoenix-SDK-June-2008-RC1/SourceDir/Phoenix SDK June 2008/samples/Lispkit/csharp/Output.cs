using System;
using System.Collections.Generic;
using System.IO;

namespace Lispkit
{
   /// <summary>
   /// The exception that is thrown when a fatal compiler error occurs. 
   /// The compiler throws this error when it encounters an error that 
   /// it cannot recover from.
   /// </summary>
   class FatalErrorException : System.Exception
   {
      private int lineNumber;

      // Retrieves the source line number that is associated with the 
      // fatal error.
      public int LineNumber
      {
         get { return this.lineNumber; }
      }

      public FatalErrorException(int lineNumber, string message)
         :  base(message)
      {
         this.lineNumber = lineNumber;
      }     
   };

   /// <summary>
   /// Defines all errors and warnings for the compiler.
   /// </summary>
   enum Error
   {
      FatalError = 1000,            // Used to report fatal errors.
      InternalError = 1001,         // Used to report an internal error 
                                    // in the compiler.      
      FileNotFound = 2000,          // File not found error.
      TooManyArguments = 2001,      // Used to report cases where too many 
                                    // arguments are passed to a higher-order
                                    // function.
      UndeclaredIdentifier = 2002,  // Used to report undeclared identifiers.
   }
      
   /// <summary>
   /// Writes error and warning diagnostic messages to an output stream.
   /// </summary>
   static class Output
   {
      // Writes the given message to the output stream.
      internal static void ReportMessage(string message)
      {
         ReportMessage(-1, message);
      }

      // Writes the given message to the output stream.
      internal static void ReportMessage(int lineNumber, string message)
      {
         // Format message string.
         if (lineNumber >= 0)
         {
            message = string.Format("{0}({1}) : {2}", 
               SourceFileName, lineNumber, message);
         }
         else
         {
            message = string.Format("{0}", message);
         }

         // Write the message to the message writer if one was specified;
         // otherwise, write the message to the console.

         if (MessageWriter != null)
         {
            MessageWriter.WriteLine(message);
         }
         else
         {
            Console.ForegroundColor = System.ConsoleColor.Cyan;
            Console.WriteLine(message);
            Console.ResetColor();
         }
      }

      // Writes the given message to the output stream if trace
      // mode if enabled.
      internal static void TraceMessage()
      {
         TraceMessage(string.Empty);
      }

      // Writes the given message to the output stream if trace
      // mode if enabled.
      internal static void TraceMessage(string message)
      {
         TraceMessage(-1, message);
      }

      // Writes the given message to the output stream if trace
      // mode if enabled.
      internal static void TraceMessage(int lineNumber, string message)
      {
         if (!Program.VerboseMode)
         {
            return;
         }

         if (message.Length == 0)
         {
            if (MessageWriter != null)
            {
               MessageWriter.WriteLine();
            }
            else
            {
               Console.WriteLine();
            }
            return;
         }

         // Format message string.
         if (lineNumber >= 0)
         {
            message = string.Format("{0}({1}) : {2}",
               SourceFileName, lineNumber, message);
         }
         else
         {
            message = string.Format("{0}", message);
         }

         // Write the message to the message writer if one was specified;
         // otherwise, write the message to the console.

         if (MessageWriter != null)
         {
            MessageWriter.WriteLine(message);
         }
         else
         {
            Console.ForegroundColor = System.ConsoleColor.Yellow;
            Console.WriteLine(string.Format("{0}: {1}", trace, message));
            Console.ResetColor();
         }
      }
      
      // Writes the given warning message to the output stream.
      internal static void ReportWarning(int lineNumber, Error warning, 
         object arg0)
      {
         string message = string.Format(errorMap[warning], arg0);
         InternalReportMessage(Output.warning, lineNumber, message, 
            (int)warning);
         WarningCount++;
      }

      // Writes the given error message to the output stream.
      internal static void ReportError(int lineNumber, Error error)
      {
         InternalReportMessage(Output.error, lineNumber, 
            errorMap[error], (int)error);
         ErrorCount++;
      }

      // Writes the given error message to the output stream.
      internal static void ReportError(int lineNumber, Error error, 
         object arg0)
      {
         string message = string.Format(errorMap[error], arg0);
         InternalReportMessage(Output.error, lineNumber, message, (int)error);
         ErrorCount++;
      }

      // Writes the given error message to the output stream.
      internal static void ReportError(int lineNumber, Error error, 
         object arg0, object arg1)
      {
         string message = string.Format(errorMap[error], arg0, arg1);
         InternalReportMessage(Output.error, lineNumber, message, (int)error);
         ErrorCount++;
      }

      // Writes the given error message to the output stream.
      internal static void ReportError(int lineNumber, Error error, 
         object arg0, object arg1, object arg2)
      {
         string message = string.Format(errorMap[error], arg0, arg1, arg2);
         InternalReportMessage(Output.error, lineNumber, message, (int)error);
         ErrorCount++;
      }

      // Writes the given fatal error message to the output stream.
      internal static void ReportFatalError(Error error)
      {
         ReportFatalError(-1, error);
      }

      // Writes the given fatal error message to the output stream.
      internal static void ReportFatalError(Error error, object arg0)
      {
         ReportFatalError(-1, error, arg0);
      }

      // Writes the given fatal error message to the output stream.
      internal static void ReportFatalError(int lineNumber, Error error)
      {
         string message = errorMap[error];
         InternalReportMessage(Output.error, lineNumber, message, (int)error);
         ErrorCount++;

         InternalReportMessage(Output.fatalError, lineNumber, 
            "unable to recover from previous error(s); stopping compilation.",
            (int)Error.FatalError);

         // Throw fatal error message.
         throw new FatalErrorException(lineNumber, message);
      }

      // Writes the given fatal error message to the output stream.
      internal static void ReportFatalError(int lineNumber, Error error, 
         object arg0)
      {
         string message = string.Format(errorMap[error], arg0);
         InternalReportMessage(Output.error, lineNumber, message, (int)error);
         ErrorCount++;

         InternalReportMessage(Output.fatalError, lineNumber, 
            "unable to recover from previous error(s); stopping compilation.", 
            (int)Error.FatalError);

         // Throw fatal error message.
         throw new FatalErrorException(lineNumber, message);
      }

      // Writes the given fatal error message to the output stream.
      internal static void ReportFatalError(int lineNumber, Error error, 
         object arg0, object arg1)
      {
         string message = string.Format(errorMap[error], arg0, arg1);
         InternalReportMessage(Output.error, lineNumber, message, (int)error);
         ErrorCount++;

         InternalReportMessage(Output.fatalError, lineNumber, 
            "unable to recover from previous error(s); stopping compilation.",
            (int)Error.FatalError);

         // Throw fatal error message.
         throw new FatalErrorException(lineNumber, message);
      }

      // Writes the given fatal error message to the output stream.
      internal static void ReportFatalError(int lineNumber, Error error, 
         object arg0, object arg1, object arg2)
      {
         string message = string.Format(errorMap[error], arg0, arg1, arg2);
         InternalReportMessage(Output.error, lineNumber, message, (int)error);
         ErrorCount++;

         InternalReportMessage(Output.fatalError, lineNumber, 
            "unable to recover from previous error(s); stopping compilation.",
            (int)Error.FatalError);

         // Throw fatal error message.
         throw new FatalErrorException(lineNumber, message);
      }

      // Writer object for messages.      
      public static TextWriter MessageWriter;

      // The name of the file that is currently being compiled.
      public static string SourceFileName;

      // Retrieves the default system message writer.
      public static TextWriter DefaultWriter
      {
         get { return System.Console.Out; }
      }

      // The current error count.
      public static int ErrorCount;

      // The current warning count.
      static int WarningCount;

      static Output()
      {
         MessageWriter = null;
         
         ErrorCount = 0;
         WarningCount = 0;

         InitializeErrorMap();
      }

      // Internal method for writing strings to the output stream.
      private static void InternalReportMessage(
         string messageType, int lineNumber, string message, int messageNumber)
      {
         string output;

         // Write source file / line information if a valid line number
         // was provided.

         if (lineNumber >= 0)
         {
            output = String.Format("{0}({1}) ", SourceFileName, lineNumber);

            if (MessageWriter != null)
            {
               MessageWriter.Write(output);
            }
            else
            {
               Console.Write(output);
            }
         }

         // Add in the message number if one was provided.

         if (messageNumber >= 0)
            output = String.Format("{0} P{1}", messageType, messageNumber);
         else
            output = messageType;

         // Write the output.

         if (MessageWriter != null)
         {
            MessageWriter.Write(output);
         }
         else
         {
            if (messageType.Equals(error) || messageType.Equals(fatalError))
            {
               Console.ForegroundColor = System.ConsoleColor.Red;
            }
            Console.Write(output);
            Console.ResetColor();
         }

         output = String.Format(": {0}", message);

         if (MessageWriter != null)
         {
            MessageWriter.WriteLine(output);
         }
         else
         {
            Console.WriteLine(output);
         }
      }

      // Initializes the error map.
      private static void InitializeErrorMap()
      {
         errorMap = new Dictionary<Error, string>();

         errorMap[Error.UndeclaredIdentifier] =
            "'{0}': undeclared identifier";

         errorMap[Error.TooManyArguments] =
            "higher-order functions can have no more than {0} arguments.";

         errorMap[Error.FileNotFound] = 
            "'{0}': file not found.";
            
         errorMap[Error.InternalError] =
            "internal error: '{0}'.";
      }

      // Prefix strings for warnings and errors.

      private const string trace = "trace";
      private const string warning = "warning";
      private const string error = "error";
      private const string fatalError = "fatal error";

      // Maps Error values to their string representations.
      private static Dictionary<Error, string> errorMap;

      // Prepares the console for a block of output messages.
      internal static void BeginVerboseOutput()
      {
         Console.ForegroundColor = System.ConsoleColor.Yellow;         
      }

      // Resets console after a block of output messages.
      internal static void EndVerboseOutput()
      {
         Console.ResetColor();
      }
   }
}