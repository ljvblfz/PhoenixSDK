//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Simple Managed Phoenix based application to add nop's to an existing 
//    program
//
//    Input file must be compiled with a phoenix based compiler and -Zi
//
//-----------------------------------------------------------------------------

//--------------------------------------------------------------------------
//
// Description:
//
//    Create control. If enabled, it will dump IR for every functionUnit.
//
//--------------------------------------------------------------------------

ref class DumpIRControl
{

public:

   static void StaticInitialize();

public:

   static Phx::Controls::SetBooleanControl ^ ShowResultsCtrl;
};

//--------------------------------------------------------------------------
//
// Description:
//
//    A helper class does command-line parsing, also works as a holder of
//    some global variables
//
//    Right now, these options are parsed:
//
//       inFile:  The name of the input binary that we want to instrument.
//                This option is required.
//
//       outFile: The name of the instrumented binary that will be created.
//                This option is required.
//
//--------------------------------------------------------------------------

public
ref class CmdLineParser
{
public:

   // The binary to be instrumented

   static String ^ inFile;

   // The intrumented binary

   static String ^ outFile;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    A control that captures the command-line arguments that do not
   //    match any other controls.
   //
   //--------------------------------------------------------------------------

   ref class DefaultControl : Phx::Controls::DefaultControl
   {
   public:
      virtual void
      Process
      (
         String ^ string
      ) override;
   };

   static Phx::Term::Mode
   ParseCmdLine
   (
      array<String ^> ^ argv
   );

   static void
   Usage();
};
