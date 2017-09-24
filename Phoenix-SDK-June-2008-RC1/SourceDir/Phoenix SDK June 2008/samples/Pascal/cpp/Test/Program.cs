using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace Test
{
    class Program
    {
        private static int errorCount = 0;

        static void Main(string[] args)
        {
            if (Options.ParseCommandLine(args) > 0)
            {
                Options.PrintUsage();
                return;
            }

            DateTime startTime = DateTime.Now;
            Log.WriteLine("Opening new test log.");

            // Obtain listing of all pascal (*.p,*.pas) source files.

            List<string> sourceFiles = new List<string>();
            sourceFiles.AddRange(Directory.GetFiles(Environment.CurrentDirectory, "*.p"));
            sourceFiles.AddRange(Directory.GetFiles(Environment.CurrentDirectory, "*.pas"));
            for (int i = 0; i < sourceFiles.Count; ++i)
                sourceFiles[i] = Path.GetFileName(sourceFiles[i]);

            // Remove ReadOnly attribute from all files.

            List<string> allFiles = new List<string>();
            allFiles.AddRange(Directory.GetFiles(Environment.CurrentDirectory));
            for (int i = 0; i < allFiles.Count; ++i)
            {
                FileAttributes attrs = File.GetAttributes(allFiles[i]);
                attrs &= ~FileAttributes.ReadOnly;
                File.SetAttributes(allFiles[i], attrs);
            }

            // Remove exluded tests from run.

            FilterExcludeFiles(sourceFiles);

            // Process runs.

            foreach (RunKind runKind in Options.RunKinds)
            {
                switch (runKind)
                {
                    case RunKind.Baseline:
                    case RunKind.Verify:
                        RunTests(sourceFiles, runKind);
                        break;
                    case RunKind.Print:
                        PrettyPrint(sourceFiles);
                        break;
                    case RunKind.Visits:
                        PrintVisits(sourceFiles);
                        break;
                }
            }

            // Print elapsed time and exit.

            string message = String.Format("Completed with {0} errors. Elapsed time was {1}s.",
                errorCount, DateTime.Now.Subtract(startTime).TotalSeconds);
            
            Log.WriteLine(message);
            Log.Close();

            return;
        }

        /// <summary>
        /// Removes all files specified in the exclude file from the specified
        /// source file list.
        /// </summary>
        /// <param name="sourceFiles"></param>
        private static void FilterExcludeFiles(List<string> sourceFiles)
        {
            if (File.Exists("Exclude.txt"))
            {
                string[] excludeFiles = File.ReadAllLines("Exclude.txt");

                List<string> removeFromSource = new List<string>();
                foreach (string excludeFile in excludeFiles)
                {
                    // Split file name from optional comment.
                    string excludeFileName = excludeFile.Split(new char[] { ';' })[0].Trim();

                    string lwr = excludeFileName.ToLower();
                    foreach (string sourceFile in sourceFiles)
                    {
                        if (lwr.Equals(Path.GetFileName(sourceFile).ToLower()))
                        {
                            removeFromSource.Add(sourceFile);
                        }
                    }
                }
                foreach (string sourceFile in removeFromSource)
                    sourceFiles.Remove(sourceFile);
            }
        }

        /// <summary>
        /// Performs a single test pass.
        /// </summary>
        /// <param name="testPass"></param>
        static void ExecuteTest(TestPass testPass)
        {
            // Extract items from test pass.

            string commandLine = testPass.CommandLine;
            string outputFile = testPass.OutputFile;
            string exeFile = testPass.ExeFile;

            // Build a list of source files to compile.

            StringBuilder sourceFilesBuilder = new StringBuilder();
            foreach (string sourceFile in testPass.FileNames)
                sourceFilesBuilder.Append(" " + sourceFile);

            using (StreamWriter outputWriter = new StreamWriter(outputFile))
            {
                string sourceFiles = sourceFilesBuilder.ToString();
                commandLine = 
                    string.Format("{0} {1}", commandLine, sourceFiles);

                // Run the compiler.

                Log.Write(string.Format("Running '{0}'...", commandLine));
                Utility.ExecuteProcess(
                    "msp.exe", 
                    commandLine, 
                    outputWriter, 
                    true
                );
                outputWriter.WriteLine();
                Log.WriteLine();

                // Perform custom action if one was specified from XML.

                if (testPass.CustomAction.Length > 0)
                {
                    Log.WriteLine(string.Format("Running '{0}'...", testPass.CustomAction));

                    int space = testPass.CustomAction.IndexOf(' ');
                    string command = testPass.CustomAction.Substring(0,space);
                    string args = testPass.CustomAction.Substring(space);

                    Utility.ExecuteProcess(command, args, outputWriter, true);
                    outputWriter.WriteLine();
                    Log.WriteLine();
                }

                // Run the output executable file if it exists.

                if (exeFile.Length > 0 && File.Exists(exeFile))
                {
                    Log.Write(string.Format(">Running '{0}'...", exeFile));
                    Utility.ExecuteProcess(exeFile, "", outputWriter, true);
                    Log.WriteLine();
                }
            }
        }

        /// <summary>
        /// Performs a test pass over each source file in the specified
        /// source file list. 
        /// </summary>
        /// <param name="sourceFiles"></param>
        /// <param name="runKind"></param>
        private static void RunTests(List<string> sourceFiles, RunKind runKind)
        {
            string extension = null;
            switch (runKind)
            {
                case RunKind.Baseline:
                    extension = ".baseline";
                    break;
                case RunKind.Verify:
                    extension = ".run";
                    break;
            }

            // For each source file, look for an XML file that drives
            // execution of the test pass. If an XML file does not exist,
            // use default settings.

            foreach (string sourceFile in sourceFiles)
            {
                TestPass testPass;
                string xmlFile = Path.ChangeExtension(sourceFile, ".xml");
                if (File.Exists(xmlFile))
                {
                    testPass = XmlReader.ProcessXml(xmlFile);
                }
                else
                {
                    testPass = new TestPass(sourceFile);
                    testPass.CommandLine = "/Oe /d -dumpchktype";
                    testPass.ExeFile = Path.ChangeExtension(sourceFile, ".exe");
                }
                testPass.OutputFile = Path.ChangeExtension(sourceFile, extension);
                ExecuteTest(testPass);
            }

            // If in verification mode, compare the .run file with 
            // the corresponding .baseline file.

            if (runKind == RunKind.Verify)
            {
                Log.WriteLine("Verifying baseline...");
                foreach (string sourceFile in sourceFiles)
                {
                    VerifyBaseline(sourceFile);
                }
                Log.WriteLine("Done.");
            }
        }

        /// <summary>
        /// Pretty-prints each file in the specified source file list.
        /// </summary>
        /// <param name="sourceFiles"></param>
        private static void PrettyPrint(List<string> sourceFiles)
        {
            foreach (string sourceFile in sourceFiles)
            {
                TestPass testPass = new TestPass(sourceFile);
                testPass.CommandLine = "/nc /p";
                testPass.OutputFile = Path.ChangeExtension(sourceFile, ".pretty");
                testPass.ExeFile = "";

                ExecuteTest(testPass);
            }
        }

        /// <summary>
        /// Prints visitation metric for all files in the specified source file list.
        /// </summary>
        /// <param name="sourceFiles"></param>
        private static void PrintVisits(List<string> sourceFiles)
        {
            foreach (string sourceFile in sourceFiles)
            {
                TestPass testPass = new TestPass(sourceFile);
                testPass.CommandLine = "/nc /v";
                testPass.OutputFile = Path.ChangeExtension(sourceFile, ".visits");
                testPass.ExeFile = "";

                ExecuteTest(testPass);
            }

            string longest = string.Empty;

            Dictionary<string, int> visitMap = new Dictionary<string, int>();
            foreach (string visitFile in 
                Directory.GetFiles(Environment.CurrentDirectory, "*.visits"))
            {
                string[] lines = File.ReadAllLines(visitFile);
                foreach (string line in lines)
                {
                    if (line.Length == 0 || line[0] == '*')
                        break;
                    else
                    {
                        string[] entries =
                            line.Split((char[])null, StringSplitOptions.RemoveEmptyEntries);

                        if (entries.Length > 3)
                            break;

                        if (!visitMap.ContainsKey(entries[0]))
                        {
                            if (entries[0].Length > longest.Length)
                                longest = entries[0];
                            visitMap.Add(entries[0], 0);
                        }

                        visitMap[entries[0]] += int.Parse(entries[entries.Length - 1]);
                    }
                }
            }

            int usedCount = 0;
            foreach (String key in visitMap.Keys)
            {
                StringBuilder builder = new StringBuilder(key);
                while (builder.Length < longest.Length)
                    builder.Append(' ');
                int visitCount = visitMap[key];
                if (visitCount > 0) usedCount++;
                Log.WriteLine(string.Format("{0}: {1}", builder.ToString(), visitCount));
            }

            int totalCount = visitMap.Keys.Count;
            int percentage;
            try
            {
                percentage = 100 * usedCount / totalCount;
            }
            catch (DivideByZeroException)
            {
                percentage = 0;
            }
            Log.WriteLine(string.Format("***Coverage = {0}/{1} ({2}%)",
                usedCount, totalCount, percentage)
              );
        }

        /// <summary>
        /// Compares a .run file against its corresponding .baseline file.
        /// This method increments the error counter and prints an error
        /// message if the files do not match.
        /// </summary>
        /// <param name="sourceFile"></param>
        private static void VerifyBaseline(string sourceFile)
        {
            string baselineFile = Path.ChangeExtension(sourceFile, ".baseline");
            string runFile = Path.ChangeExtension(sourceFile, ".run");

            if (File.Exists(baselineFile) && File.Exists(runFile))
            {
                string baselineOutput = File.ReadAllText(baselineFile);
                string compareOutput = File.ReadAllText(runFile);

                if (!baselineOutput.Equals(compareOutput))
                {
                    Log.WriteLine(string.Format(
                       "Baseline file '{0}' did not match run file '{1}'.",
                       baselineFile, runFile
                       )
                    );
                    ++errorCount;
                }
            }
        }
    }
}