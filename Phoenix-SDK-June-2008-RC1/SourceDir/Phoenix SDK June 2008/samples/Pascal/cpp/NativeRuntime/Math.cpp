//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Defines the runtime library math routines.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include <math.h>

// Determines whether the given number is odd.
extern "C"
int __cdecl
odd(int n)
{  
   return n%2;
}

// Performs modulus division between the two integer operands.
extern "C"
int __cdecl
mod(int n, int m)
{  
   return n%m;
}

// Calculates the square of the given integer value.
extern "C"
int __cdecl
sqr(int n)
{  
   return n*n;
}

// Calculates the square of the given double value.
extern "C"
double __cdecl
sqrf(double n)
{  
   return n*n;
}

