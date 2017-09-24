//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Phoenix subsystem of the add-in.  Provides Phoenix services to the
//    rest of the add-in.
//
//------------------------------------------------------------------------------

#pragma once

#pragma region Using namespace declarations

using namespace System;
using namespace System::Collections::Generic;

#pragma endregion

namespace ControlFlowGraph
{

//------------------------------------------------------------------------------
//
// Description:
//
//    Provides Phoenix functionality to the controller class, Connect.
//
// Remarks:
//
//    This class serves as a front end for the Phoenix subsystem of the
//    add-in.
//
//------------------------------------------------------------------------------

public
ref class PhoenixProvider
{
public:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Default constructor.
   //
   //---------------------------------------------------------------------------

   PhoenixProvider();

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Returns the flow graph of the function specified by the function
   //    name, type name, and parameter types.
   //
   // Arguments:
   //
   //    binaryFile - The full path of the binary file that contains the
   //    function
   //    pdbFile - The pdb that contains information about binaryFile.
   //    targetFile - The path of the file that contains the definition of
   //    of the function that we are trying to find.
   //    targetLine - The line number of the function that we are trying to
   //    find.
   //
   // Returns:
   //
   //    The flow graph of the function in targetFile closest to targetLine.
   //
   // Remarks:
   //
   //    GetFlowGraph loads the binary and pdb specified by the file name and
   //    pdb.  It then attempts to find the functional unit on or after
   //    targetLine in targetFile.
   //
   //---------------------------------------------------------------------------

   Phx::Graphs::FlowGraph ^
   GetFlowGraph
   (
      String ^      binaryFile,
      String ^      pdbFile,
      String ^      targetFile,
      unsigned int  targetLine
   );

private:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Helper method that locates the function in the assembly that is
   //    closest to the line indicated by targetLine in targetFile.
   //
   // Arguments:
   //
   //    targetFile - The file that contains the function that we are locating.
   //    targetLine - The line that the function is on.  This need not be exact.
   //    For instance, Intellisense will provide a line number that corresponds
   //    to the header line of the function definition, while the pdb line
   //    numbers correspond to the first executable line of the function.
   //    pe - The PEModuleUnit that contains the function.
   //
   // Returns:
   //
   //    The Phx::FunctionUnit of the function closest to the target line in the 
   //    target file.
   //
   // Remarks:
   //
   //    FindClosestFuncUnit walks the functional units in the pe module and
   //    finds the set closest to the target line in the target file.
   //
   //---------------------------------------------------------------------------

   Phx::FunctionUnit ^
   FindClosestFuncUnit
   (
      String ^            targetFile,
      unsigned int        targetLine,
      Phx::PEModuleUnit ^ pe
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Initializes Phoenix.
   //
   //---------------------------------------------------------------------------

   void
   InitializePhoenix();

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Registers the targets available in the RDK.
   //
   //---------------------------------------------------------------------------

   void
   InitializeTargets();

};

} // ControlFlowGraph
