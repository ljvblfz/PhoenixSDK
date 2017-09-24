using System;
using System.Collections.Generic;
using System.Text;

namespace Test
{
    public enum RunKind
    {
        Baseline,
        Print,
        Verify,
        Visits,
    }

    static class Options
    {
        private static List<RunKind> runKinds;

        public static RunKind[] RunKinds
        {
            get { return runKinds.ToArray(); }
        }
                              
        public static int ParseCommandLine(string[] args)
        {
            int errorCount = 0;
            List<string> unknownArgs = new List<string>();

            runKinds = new List<RunKind>();
            bool all = false;

            foreach (string arg in args)
            {
                if (arg.StartsWith("-") || arg.StartsWith("/"))
                {
                    string cleanArg = arg.Substring(1).ToLower();
                    if (cleanArg.Equals("all"))
                    {
                        all = true;
                        continue;
                    }
                    if (cleanArg.Equals("baseline"))
                    {
                        runKinds.Add(RunKind.Baseline);
                        continue;
                    }
                    else if (cleanArg.Equals("print"))
                    {
                        runKinds.Add(RunKind.Print);
                        continue;
                    }
                    else if (cleanArg.Equals("verify"))
                    {
                        runKinds.Add(RunKind.Verify);
                        continue;
                    }
                    else if (cleanArg.Equals("visits"))
                    {
                        runKinds.Add(RunKind.Visits);
                        continue;
                    }
                    else
                    {
                        unknownArgs.Add(cleanArg);
                        continue;
                    }
                }
                Console.WriteLine("'{0}': unknown command-line option.", arg);
                ++errorCount;
            }

            foreach (string unknownArg in unknownArgs)
            {
                Console.WriteLine("'{0}': unknown command-line option.", unknownArg);
                ++errorCount;
            }

            if (all)
            {
                runKinds.Clear();
                runKinds.AddRange(new RunKind[]{ RunKind.Print, RunKind.Visits,
                    RunKind.Baseline, RunKind.Verify });
            }

            if (runKinds.Count == 0)
            {
                ++errorCount;
            }

            return errorCount;
        }

        public static void PrintUsage()
        {
            Console.WriteLine();
            Console.WriteLine("Command-line syntax:\r\nmspt.exe [-baseline|print|quick|verify|visits|all]");

            Console.WriteLine("   baseline - generate test baseline.");
            Console.WriteLine("   print - pretty print source programs.");
            Console.WriteLine("   verify - verify tests against baseline.");
            Console.WriteLine("   visits - parse source and report AST usage.");
            Console.WriteLine("   all - perform baseline|print|verify|visits.");

            Console.WriteLine("Note: '/' can be used for '-'.");
            Console.WriteLine("Example: mspt.exe -all");
        }        
    }
}
