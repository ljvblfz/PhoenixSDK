//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//     Definitions of application exception classes.
//
//-----------------------------------------------------------------------------

#pragma once

using namespace System;

namespace Pascal
{

//-----------------------------------------------------------------------------
//
// Description:
// 
//    The exception that is thrown when a fatal compiler error occurs. 
//
// Remarks:
//
//    The compiler throws this error when it encounters an error that 
//    it cannot recover from.
//
//-----------------------------------------------------------------------------

ref class FatalErrorException : public System::Exception
{
public:

   FatalErrorException
   (
      int         lineNumber,
      String ^    message
   )
      :  System::Exception(message)
      ,  lineNumber(lineNumber)
   {
   }

   // Retrieves the source line number associated with the 
   // fatal error.

   property int LineNumber
   {
      int get()
      {
         return this->lineNumber;
      }
   }

private:

   int lineNumber;
};

}  // namespace Pascal