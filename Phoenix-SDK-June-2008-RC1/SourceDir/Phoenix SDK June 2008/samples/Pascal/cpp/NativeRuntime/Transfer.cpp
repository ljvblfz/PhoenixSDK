//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Defines the runtime library transfer routines.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include <math.h>

// Returns the ordinal value of the given value. Although this function 
// is essentially a no-op, we implement it for parity with the other 
// runtime functions.
extern "C"
int __cdecl
ord(int n)
{
   return n;
}

// Returns the character value for the given integer, or -1 if the 
// integer is out of character range.
extern "C"
char __cdecl
chr(int n)
{
   if (n < 0x00 || n > 0xff)
      return (char)-1;
   return (char)n;
}

// Returns the predecessor of the given integer.
extern "C"
int __cdecl
pred(int n)
{
   return n-1;
}

// Returns the successor of the given integer.
extern "C"
int __cdecl
succ(int n)
{
   return n+1;
}

// Loads the given array, a, at the given offset, into array z.
extern "C"
void __cdecl
pack(void * a, void * z, int offset, int size)
{ 
   ::memcpy(z, ((char *)a)+offset, (size_t) size);
}

// Loads the given array, z, into array a at the given offset.
extern "C"
void __cdecl
unpack(void * a, void * z, int offset, int size)
{
   ::memcpy(((char *)a)+offset, z, (size_t) size);
}

// Truncates the floating-point part of the given double value.
extern "C"
int __cdecl
trunc(double x)
{  
   double n;
   ::modf(x,&n);
   return (int)n;
}

// Rounds given double value to the nearest integer.
extern "C"
int __cdecl
round(double x)
{  
   if (x > 0.0f)
      return trunc(x + 0.5f);
   return trunc(x - 0.5f);
}