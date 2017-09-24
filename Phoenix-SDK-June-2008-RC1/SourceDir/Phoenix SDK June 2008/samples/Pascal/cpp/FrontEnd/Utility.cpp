//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Implementation of the Utility class.
//
// Remarks:
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vcclr.h>

using namespace System;
using namespace System::Diagnostics;

//-----------------------------------------------------------------------------
//
// Description:
//
//    Executes the given process.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

int 
Utility::ExecuteProcess
(
   String ^ fileName, 
   String ^ arguments,
   bool waitForExit
)
{
   Process ^ process = gcnew Process();
   ProcessStartInfo ^ processStartInfo = gcnew ProcessStartInfo(fileName);
   processStartInfo->UseShellExecute = false;
   processStartInfo->Arguments = arguments;
   process->StartInfo = processStartInfo;      
   try
   {
       process->Start();
       if (waitForExit)
       {
          process->WaitForExit();
          return process->ExitCode;
       }
   }
   catch (System::ComponentModel::Win32Exception ^ e)
   {
      Pascal::Output::ReportError(
         -1,
         Pascal::Error::ProcessNotFound,         
         fileName, 
         e->Message, 
         Environment::NewLine
      );
      return 1;
   }

   return 0;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Converts the given managed string to a native string.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool
Utility::Utf8Encode
(
   String ^ string,
   char * buffer,
   int & bufferByteSize
)
{
	if (string == nullptr)
	{
      return false;
	}
	
	interior_ptr<const wchar_t> ip = ::PtrToStringChars(string);

	// Pin string so we can pass to the native layer.

	pin_ptr<const wchar_t> ptr = ip;
   
	int length = ::WideCharToMultiByte(CP_UTF8, 0L, ptr, -1,
         (char *) buffer, bufferByteSize, NULL, NULL);

   // If the caller passed 0 for the buffer size, then set the 
   // buffer size to the required length.

   if (bufferByteSize == 0)
   {
      bufferByteSize = length;
      return true;
   }

   // Otherwise, return the result of WideCharToMultiByte.

   return (length > 0);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Translates the given grammar operator to a ConditionCode object.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

Phx::ConditionCode
Utility::GetConditionCode
(
   int conditionCode
)
{
   switch (conditionCode)
   {
   case 313:   // EQUAL
      return Phx::ConditionCode::EQ;
   case 314:   // NOTEQUAL
      return Phx::ConditionCode::NE;
   case 315:   // LT
      return Phx::ConditionCode::LT;
   case 316:   // GT
      return Phx::ConditionCode::GT;
   case 317:   // LE
      return Phx::ConditionCode::LE;
   case 318:   // GE
      return Phx::ConditionCode::GE;

   case 319:   // IN
   default:
      Debug::Assert(false);
      return Phx::ConditionCode::IllegalSentinel;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Translates the given ConditionCode object to its floating-point
//    equivalent.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

Phx::ConditionCode
Utility::TranslateToFloatConditionCode
(
   Phx::ConditionCode conditionCode
)
{
   switch (conditionCode)
   {
   case Phx::ConditionCode::GE:
      return Phx::ConditionCode::FGE;
   case Phx::ConditionCode::GT:
      return Phx::ConditionCode::FGT;
   case Phx::ConditionCode::LE:
      return Phx::ConditionCode::FLE;
   case Phx::ConditionCode::LT:
      return Phx::ConditionCode::FLT;
   }

   return conditionCode;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Translates the given grammar operator to a string.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

String ^
Utility::GetOperatorString
(
   unsigned op
)
{
   switch (op)
   {
   case 287:   // PLUS
      return "+";
   case 278:   // MINUS
      return "-";
   case 296:   // SLASH
      return "/";
   case 297:   // STAR
      return "*";
   case 298:   // STARSTAR
      return "**";
   case 320:   // DIV
      return "div";
   case 321:   // MOD
      return "mod";
   case 322:   // AND
      return "and";

   case 313:   // EQUAL
      return "=";
   case 314:   // NOTEQUAL
      return "<>";
   case 317:   // LE
      return "<=";
   case 318:   // GE
      return ">=";   
   case 315:   // LT
      return "<";
   case 316:   // GT
      return ">";
   case 319:   // IN
      return "in";

   default:
      Debug::Assert(false);
      return "?";
   }
}