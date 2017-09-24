using System;
using System.Text;
using System.IO;
using System.Diagnostics;

namespace Lispkit
{
   /// <summary>
   /// The main application class. This class initializes the Phoenix 
   /// framework, parses the command line, and initiates the 
   /// compilation of the provided source file.
   /// </summary>
   class Program
   {
      // Delegate for StringControl broadcast messages.
      delegate void ProcessStringControl(string command);

      /// <summary>
      /// Handler class for StringControl objects.
      /// We use this to handle the "default" string control.
      /// The first strings that is passed on the command-line that 
      /// is otherwise unrecognized is treated as the source file name.
      /// Subsequent strings are treated as arguments to the 
      /// interpreted program.
      /// </summary>
      class StringControl : Phx.Controls.DefaultControl
      {
         // Callback delegate.
         private ProcessStringControl callback;

         // Creates a new instance of the StringControl class.
         public StringControl(ProcessStringControl callback)
         {
            this.callback = callback;
         }

         // Processes a non-switch argument
         public override void Process(string s)
         {
            this.callback(s);
         }
      }

      // The user-specified source file name.
      private static string sourceFileName = string.Empty;
      
      // The user-specified argument list for interpreted programs.
      private static string arguments = string.Empty;

      // Retrieves the name of the "standard input" file.
      public static string ConsoleFileName
      {
         get { return "stdin"; }
      }

      // Runtime objects that are registered by the application.
      private static Phx.Targets.Runtimes.Runtime x86Runtime;
      private static Phx.Targets.Runtimes.Runtime msilRuntime;

      // Command-line controls.
      private static Phx.Controls.SetBooleanControl interpret;
      private static Phx.Controls.SetBooleanControl debugMode;
      private static Phx.Controls.SetBooleanControl verbose;
      private static Phx.Controls.SetBooleanControl coverage;
      
      // Determines whether the compiler is running in verbose mode.
      // Enabling verbose mode enables trace messages.
      public static bool VerboseMode
      {
         get { return verbose.GetValue(null); }
      }

      // Determines whether the "report coverage" control is enabled.
      // When enabled, the compiler reports which instructions are 
      // executed when run in interpreted mode.
      public static bool ReportCoverage
      {
         get { return coverage.GetValue(null); }
      }
            
      // Generates a parse tree for the provided input stream.
      private static Ast.Node ParseInputStream(Stream input, 
         out int parseErrors)
      {
         LexParser.Parser parser = new LexParser.Parser();
         LexScanner.Scanner scanner = new LexScanner.Scanner(input);
         parser.scanner = scanner;
         parser.Parse();
         parseErrors = scanner.ErrorCount;
         return parser.AstRootNode;
      }

      // Reads the source program listing.
      private static Stream ReadInputStream()
      {
 	      Stream input = null;
      
         // Open a file stream if the user provided a file name 
         // on the command line.
         if (sourceFileName.Length > 0)
         {
            Debug.Assert(File.Exists(sourceFileName));            
            input = new FileStream(sourceFileName, FileMode.Open, 
               FileAccess.Read, FileShare.Read);

            Output.ReportMessage(string.Format(
               "Compiling {1}...{0}", Environment.NewLine, sourceFileName));
         }
         // Otherwise, read the source program from standard input.
         else
         {
            string conin = Console.In.ReadToEnd();
            sourceFileName = ConsoleFileName;          
            input = new MemoryStream((new UnicodeEncoding()).GetBytes(conin));
         }
         
         return input;
      }
      
      // Prints the usage string to the console.
      static void Usage()      
      {
         System.Console.WriteLine(
            "Usage:\r\n" +
            "Lispkit.exe [-i] [-pdb] [-verbose] [-coverage] " +
            "[source-file-name [arguments]]\r\n" +
            " arguments in brackets are optional.");
      }
      
      // Handler for the default string control.
      static void SetFilename(string fileName)
      {
         // Set the source file name if it has not been set already.
         if (sourceFileName.Length == 0)
         {
            sourceFileName = fileName;
            Output.SourceFileName = sourceFileName;
         }
         // Otherwise, append the argument to the argument list.
         else
         {
            arguments += fileName + " ";
         }
      }
      
      // Initializes the Phoenix framework.      
      private static void InitializePhoenix(string[] arguments)
      {
         Phx.Targets.Architectures.Architecture architecture;
         Phx.Targets.Runtimes.Runtime runtime;

         // Register x86 architecture and runtime.
         architecture = Phx.Targets.Architectures.X86.Architecture.New();
         runtime =
            Phx.Targets.Runtimes.Vccrt.Win32.X86.Runtime.New(architecture);
         Phx.GlobalData.RegisterTargetArchitecture(architecture);
         Phx.GlobalData.RegisterTargetRuntime(runtime);

         x86Runtime = runtime;

         // Register Msil architecture and runtime.
         architecture = Phx.Targets.Architectures.Msil.Architecture.New();
         runtime =
            Phx.Targets.Runtimes.Vccrt.Win.Msil.Runtime.New(architecture);
         Phx.GlobalData.RegisterTargetArchitecture(architecture);
         Phx.GlobalData.RegisterTargetRuntime(runtime);

         msilRuntime = runtime;

         Phx.Initialize.BeginInitialization();

         // Register custom controls for the program.

         RegisterControls();

         // Allow the framework to process any user-provided controls.

         Phx.Initialize.EndInitialization("PHX|*|_PHX_", arguments);

         // Enable some controls. We want to see the names of types
         // line numbers in our IR dumps, and linenumbers
         // in our object file.

         Phx.Controls.Parser.ParseArgumentString(null,
            "-dumptypesym -dump:linenumbers -lineleveldebug");
      }

      // Terminates the Phoenix framework and returns the application
      // return code.
      static int Exit(Phx.Term.Mode termMode)
      {
         Phx.Term.All(termMode);

         // If the debugger is attached, wait for the user to 
         // press a key.
         if (System.Diagnostics.Debugger.IsAttached)
         {
            System.Console.Write(
               "{0}Press any key to exit: ", Environment.NewLine);
            System.Console.ReadKey(true);
         }

         return termMode == Phx.Term.Mode.Normal ? 0 : -1;
      }

      // Registers the custom controls that are used by the compiler.
      static void RegisterControls()
      {
         // We want to supply a plain filename on the command line and not
         // have to prepend Phoenix control syntax 
         // (such as "-input:source.lispkit").
         // To do this we create a default control that processes any
         // command-line arguments that Phoenix doesn't recognize as a control.
         
         Phx.Controls.Parser.RegisterDefaultControl(
            new StringControl(
               new ProcessStringControl(Program.SetFilename)
            )
         );

         // Boolean control to build SECD machine.                  
         interpret = Phx.Controls.SetBooleanControl.New(
            "i", "Run the interpreter.", "");

         // Boolean control to generate a .pbd for the program.
         debugMode = Phx.Controls.SetBooleanControl.New(
            "pdb", "Generate debugging information.", "");

         // Boolean control to enable trace messages.
         verbose = Phx.Controls.SetBooleanControl.New(
            "verbose", "Produce verbose status messages.", "");

         // Boolean control to report instruction coverage in 
         // interpreted programs.
         coverage = Phx.Controls.SetBooleanControl.New(
            "coverage", "Report instruction coverage.", "");
      }

      static int Main(string[] args)
      {
         // Initialize the Phoenix library for code generation.
         // This step also processes the command-line arguments.
         InitializePhoenix(args);

         // If the provided source file name does not exist, report 
         // a fatal error.
         try
         {
            if (sourceFileName.Length > 0 &&
               !File.Exists(sourceFileName))
            {
               Output.ReportFatalError(Error.FileNotFound, sourceFileName);
            }
         }
         catch (FatalErrorException)
         {
            // ReportFatalError throws this exception.
            // Report usage to the user and exit.
            Usage();
            return Exit(Phx.Term.Mode.Fatal);
         }

         // Tracks the total number of errors. 
         int totalErrors = 0;

         // Read the input stream (either the contents of the source file,
         // or what is typed on the command line).
         Stream input = Program.ReadInputStream();

         // Generate an AST from the input stream.
         Output.TraceMessage("Parsing input stream...");
         int parseErrors;
         Ast.Node rootNode = Program.ParseInputStream(input, out parseErrors);
         input.Dispose();
         totalErrors += parseErrors;

         // Process the AST if parsing succeeded.
         if (totalErrors == 0)
         {
            // Run the AST evaluator to translate the AST into 
            // an S-expression.
            Ast.Evaluator evaluator = new Ast.Evaluator();
            rootNode.Accept(evaluator);
            Sexp.SExp sexp = evaluator.SExp;

            // Create Compiler and Interpreter objects.

            Compiler compiler;
            Interpreter interpreter;

            // Create a Secd.Compiler object if the user specified
            // the interpret (-i) control. The Secd.Compiler class
            // inherits from the Compiler class and implements the
            // Interpreter interface.
            if (interpret.GetValue(null))
            {
               Secd.Compiler secdCompiler = new Secd.Compiler();
               compiler = secdCompiler;
               interpreter = secdCompiler;
            }
            // Otherwise, create an Msil.Compiler object for MSIL generation.
            else
            {
               compiler = new CodeGen.Msil.Compiler(sourceFileName,
                  msilRuntime, Program.debugMode.GetValue(null));
               interpreter = null;
            }

            // Generate a compiled S-expression from the 
            // S-expression parse tree.
            try
            {
               sexp = compiler.Compile(sexp);

               string s = sexp.ToString();
               Output.TraceMessage("Compiled S-expression:\r\n" + s + "\r\n");
            }
            catch (FatalErrorException)
            {
               // If a fatal error was raised during compilation, invalidate 
               // the Interpreter object and continue.
               interpreter = null;
            }

            // Run the interpreter if it is available.
            if (interpreter != null)
            {
               // The argument list was built during Phoenix initialization.
               // Trim whitespace from the argument list.
               arguments = arguments.Trim();

               // We allow a mix of literal strings and file names as the 
               // command-line arguments. For example, if we assume that the 
               // file that is named 'params.txt' contains the 
               // string "a b c d", then the argument list:
               // ( params.txt )
               // is read as the S-expression "(a b c d)".

               // Tokenize the argument list. If the token refers to a valid
               // file name, then append the contents of the file to the 
               // argument list. Otherwise, append the literal token to the
               // argument list.
               string[] tokens = arguments.Split(
                  new char[] { ' ', '\t', '\n', '\r' },
                  StringSplitOptions.RemoveEmptyEntries);
               arguments = string.Empty;
               foreach (string token in tokens)
               {
                  if (File.Exists(token))
                  {
                     arguments += File.ReadAllText(token);
                  }
                  else
                  {
                     arguments += token + " ";
                  }
               }
               arguments = arguments.Trim();

               // Translate the argument list string into an S-expression list.

               Sexp.SExp argList = new Sexp.SExpReader(arguments).Process();
               argList = new Sexp.Cons(argList, Sexp.SymAtom.NIL);

               // Run the interpreter.
               Sexp.SExp result = interpreter.Execute(argList, sexp);

               // Print the result the output stream.
               if (result != null)
               {
                  Output.ReportMessage(result.ToString() + "\r\n");
               }
               else
               {
                  Output.ReportMessage("(null)\r\n");
               }
            }

            // Increment the error counter by the total number of 
            // errors that we encountered.
            totalErrors += Output.ErrorCount;
         }

         // Print the final error and warning count to the console.
         Output.ReportMessage(
            string.Format("\n{0} error(s), 0 warnings.", totalErrors));

         // Free the framework and return the appropriate exit code.
         return Exit(totalErrors == 0 ?
            Phx.Term.Mode.Normal : Phx.Term.Mode.Fatal);
      }
   }
}