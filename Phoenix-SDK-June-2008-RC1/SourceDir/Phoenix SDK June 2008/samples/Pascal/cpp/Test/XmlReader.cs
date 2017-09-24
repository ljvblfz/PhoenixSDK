using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.Schema;
using System.IO;

namespace Test
{
    static class XmlReader
    {
        private static TestPass currentTest;

        /// <summary>
        /// Parses a TestPass object from the given XML file.
        /// </summary>
        /// <param name="xmlFile"></param>
        /// <returns></returns>
        public static TestPass ProcessXml(string xmlFile)
        {
            currentTest = null;

            string xmlFileContents =
                File.ReadAllText(Path.ChangeExtension(xmlFile, ".xml"));

            try
            {
                // Create the XmlParserContext.

                XmlParserContext context = new XmlParserContext(
                    null, null, "", XmlSpace.None);

                // Create the reader. 

                XmlReaderSettings settings = new XmlReaderSettings();
                settings.ValidationType = ValidationType.Schema;
                settings.Schemas.Add(null, "Test.xsd");
                settings.ValidationEventHandler += 
                    new ValidationEventHandler(ValidationCallBack);

                System.Xml.XmlReader reader =
                    System.Xml.XmlReader.Create(
                    new StringReader(xmlFileContents), settings, context);

                while (reader.Read())
                {
                    if (reader.IsStartElement())
                        ProcessName(reader.LocalName);
                    else
                        ProcessValue(reader.Value);
                }
                return currentTest;
            }
            catch (XmlException XmlExp)
            {
                Log.WriteLine(XmlExp.Message);
            }
            catch (XmlSchemaException XmlSchExp)
            {
                Log.WriteLine(XmlSchExp.Message);
            }
            catch (Exception GenExp)
            {
                Log.WriteLine(GenExp.Message);
            }
            return null;
        }

        private static void ValidationCallBack(object sender, 
            ValidationEventArgs args)
        {
            Log.WriteLine("Validation error: " + args.Message);
        }

        private static string currentLocalName = String.Empty;

        private static void ProcessName(string localName)
        {
            if (localName.Length == 0)
                return;

            currentLocalName = localName;

            switch (localName)
            {
                case "configuration":
                    currentTest = new TestPass();
                    break;
                case "files":
                    break;
                case "file-name":
                    break;
                case "exe-file":
                    break;
                case "command-line":
                    break;
                case "custom-action":
                    break;
            }
        }

        private static void ProcessValue(string value)
        {
            if (value.Length == 0)
                return;

            switch (currentLocalName)
            {
                case "configuration":
                    break;
                case "files":
                    currentTest.FileNames = new List<string>();
                    break;
                case "file-name":
                    currentTest.FileNames.Add(value);
                    break;
                case "exe-file":
                    currentTest.ExeFile = value;
                    break;
                case "command-line":
                    currentTest.CommandLine = value;
                    break;
                case "custom-action":
                    currentTest.CustomAction = value;
                    break;
            }
        }
    }
}
