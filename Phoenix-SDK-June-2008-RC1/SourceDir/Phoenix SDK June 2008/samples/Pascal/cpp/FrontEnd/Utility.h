//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the Utility class.
//
//-----------------------------------------------------------------------------

#pragma once

using namespace System;

//-----------------------------------------------------------------------------
//
// Description: Helper class for performing miscellaneous functions,
//              such as invoking the linker.
//
// Remarks:
//
//
//-----------------------------------------------------------------------------

ref class Utility sealed
{
public:

   // Executes the given process.

   static int 
   ExecuteProcess
   (
      String ^ fileName, 
      String ^ arguments,
      bool waitForExit
   );

   // Converts the given managed string to a native string.

   static bool 
   Utf8Encode
   (
      String ^ string,
      char * buffer,
      int & bufferByteSize
   );

   // Translates the given grammar operator to a string.

   static String ^
   GetOperatorString
   (
      unsigned op
   );

   // Translates the given grammar operator to a ConditionCode object.

   static Phx::ConditionCode
   GetConditionCode
   (
      int conditionCode
   );

   // Translates the given ConditionCode object to its floating-point
   // equivalent.

   static Phx::ConditionCode
   TranslateToFloatConditionCode
   (
      Phx::ConditionCode conditionCode
   );
};