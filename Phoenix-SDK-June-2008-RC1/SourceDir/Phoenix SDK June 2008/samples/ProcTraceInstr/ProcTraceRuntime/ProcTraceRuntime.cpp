//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Runtime for the simple Phoenix based application to add a instrumentation 
//       for a simple ProcTrace profiler.
//
//    Input file must be compiled with a phoenix based compiler and -Zi
//
// Usage:
//
//    Usage: ProcTraceInstr /out <filename> /pdbout <filename> [/funcname] 
//              /in <image-name>
//
// Remarks:
//
//    We have to use _stdcall here as this is the only calling convention that
//    will allow us to preserve the registers and flags across the call.
//    This is important as we are not raising the IR high enough to have the 
//    lower process smooth any of the issues over.
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include "windows.h"

//-----------------------------------------------------------------------------
//
// Description:
//
//    Simple output to the commandline - no parameters
//
// Returns:
//
//    Nothing
//
//-----------------------------------------------------------------------------

extern "C"
__declspec(naked)
__declspec(dllexport)
void _stdcall
Shout()
{
   _asm
   {
      // Set up the stack frame

      push    ebp
      mov     ebp, esp

      // Store the flags etc

      pushad
      pushfd

      // Clear directional flag

      cld

      // Save Win32 error flag

      call dword ptr [GetLastError]

      mov ebx, eax
   }

   // Do work

   ::wprintf(L"In Shout()\n");

   // End do work

   _asm
   {
      // Reset win32 error flag

      push ebx

      call dword ptr [SetLastError]

      // Restore flags etc

      popfd
      popad

      // Restore frame pointer

      pop ebp

      // Return

      ret
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Takes an unsigned int and outputs it to the commandline
//
// Returns:
//
//    Nothing
//
//-----------------------------------------------------------------------------

extern "C"
__declspec(naked)
__declspec(dllexport)
void _stdcall
ShoutAddr
(
   unsigned int funcAddress    // Address of the Function
)
{
   _asm
   {
      // Set up the stack frame

      push    ebp
      mov     ebp, esp

      // Store the flags etc

      pushad
      pushfd

      // Clear directional flag

      cld

      // Save Win32 error flag

      call dword ptr [GetLastError]

      mov ebx, eax
   }

   // Do work

   ::wprintf(L"In Function @ 0x%08X\n", funcAddress);
   ::fflush(stdout);

   // End do work

   _asm
   {
      // Reset win32 error flag

      push ebx

      call dword ptr [SetLastError]

      // Restore flags etc

      popfd
      popad

      // Restore frame pointer

      pop ebp

      // Pop argument off stack

      ret    0x4
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Takes an unsigned int and a string and outputs them to the commandline
//
// Returns:
//
//    Nothing
//
//-----------------------------------------------------------------------------

extern "C"
__declspec(naked)
__declspec(dllexport)
void _stdcall
ProcTrace
(
   unsigned int funcAddress,   // Address of the Function
   wchar_t *    functionName       // Function name as unicode string 
)
{
   _asm
   {
      // Set up the stack frame

      push    ebp
      mov     ebp, esp

      // Store the flags etc

      pushad
      pushfd

      // Clear directional flag

      cld

      // Save Win32 error flag

      call dword ptr [GetLastError]

      mov ebx, eax
   }

   // Do work

   ::wprintf(L"0x%08X, %s\n", funcAddress, functionName);
   ::fflush(stdout);

   // End do work

   _asm
   {
      // Reset win32 error flag

      push ebx

      call dword ptr [SetLastError]

      // Restore flags etc

      popfd
      popad

      // Restore frame pointer

      pop ebp

      // Pop both arguments off stack

      ret    0x8
   }
}

