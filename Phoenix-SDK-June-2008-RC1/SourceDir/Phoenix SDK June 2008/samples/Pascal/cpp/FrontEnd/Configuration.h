//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//     Definition of the Configuration class.
//
//-----------------------------------------------------------------------------

#pragma once

using namespace System;

namespace Pascal
{

//-----------------------------------------------------------------------------
//
// Description: Holds global configuration data.
//
// Remarks:
//
//-----------------------------------------------------------------------------

ref class Configuration sealed
{
public:

   // Flags native or managed code generation.
   // This is specified through the /clr command-line option.

   static property bool IsManaged;

   // The program name, as specified in the Pascal source file.

   static property String ^ ProgramName;

   // Flags the compilation unit as a program or a module.

   static property bool IsProgram;

private:

   static Configuration()
   {
      IsManaged = false;
      ProgramName = String::Empty;
      IsProgram = false;
   }
};

}  // namespace Pascal
