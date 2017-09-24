using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace Test
{
    static class Log
    {
        public static void WriteLine(string message)
        {
            if (message.Length == 0)
                _log.WriteLine();
            else
                Write(message + Environment.NewLine);
            isNewline = true;
        }

        public static void WriteLine()
        {
            Write(Environment.NewLine);
            isNewline = true;
        }

        public static void Write(string message)
        {
            if (isNewline)
            {
                message = string.Format("{0} {1} - {2}",
                     DateTime.Now.ToShortDateString(),
                     DateTime.Now.ToShortTimeString(),
                     message);
                isNewline = false;
            }
            _log.Write(message);
            Console.Write(message);
        }

        public static void Close()
        {
            _log.Flush();
            _log.Close();
        }

        private static StreamWriter _log = new StreamWriter("log.txt");
        private static bool isNewline = true;
    }
}
