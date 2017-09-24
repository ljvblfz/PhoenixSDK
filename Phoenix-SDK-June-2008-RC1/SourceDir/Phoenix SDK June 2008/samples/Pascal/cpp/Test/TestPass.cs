using System;
using System.Collections.Generic;
using System.Text;

namespace Test
{
    class TestPass
    {
        public TestPass()
        {
            FileNames = new List<string>();
            CommandLine = string.Empty;
            OutputFile = string.Empty;
            CustomAction = string.Empty;
            ExeFile = string.Empty;
        }
        public TestPass(string fileName) : this()
        {
            FileNames.Add(fileName);
        }

        public string CommandLine;
        public string OutputFile;
        public string CustomAction;
        public string ExeFile;
        public List<string> FileNames;
    }
}
