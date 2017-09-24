using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Diagnostics;

namespace Test
{
    static class Utility
    {
        /// <summary>
        /// Executes an external process.
        /// </summary>
        /// <param name="fileName"></param>
        /// <param name="arguments"></param>
        /// <param name="output"></param>
        /// <param name="waitForExit"></param>
        /// <returns></returns>
        public static int ExecuteProcess
        (
            string fileName,
            string arguments,
            StreamWriter output,
            bool waitForExit
        )
        {
            Process process = new Process();
            ProcessStartInfo processStartInfo = new ProcessStartInfo(fileName);
            processStartInfo.UseShellExecute = false;
            processStartInfo.Arguments = arguments;
            processStartInfo.RedirectStandardOutput = true;
            processStartInfo.WorkingDirectory = Path.GetDirectoryName(fileName);
            process.StartInfo = processStartInfo;
            try
            {
                process.Start();
                if (waitForExit)
                {
                    output.Write(process.StandardOutput.ReadToEnd());
                    process.WaitForExit();
                    return process.ExitCode;
                }
            }
            catch (System.ComponentModel.Win32Exception e)
            {
                String message = String.Format(
                   "Failed to launch '{0}'{2}Reason: '{1}'{2}" +
                     "Ensure the application exists and is accessible from your system PATH{2}" +
                     "environment variable.",
                     fileName, e.Message, Environment.NewLine);
                Console.WriteLine(message);                
                return 1;
            }

            output.Write(process.StandardOutput.ReadToEnd());
            return 0;
        }
    }
}
