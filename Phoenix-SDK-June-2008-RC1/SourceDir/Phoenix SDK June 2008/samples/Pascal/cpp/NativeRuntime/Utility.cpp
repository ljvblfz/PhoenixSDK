//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Defines the runtime library utility routines.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "Runtime.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Retrieves the lower bound of a list of integers.
extern "C"
int __cdecl
get_lower_bound(int count, ...)
{
   va_list arg_list;
   va_start(arg_list, count);
   
   int lower = INT_MAX;
   for (int i = 0; i < count; ++i)
   {
      int n = va_arg(arg_list, int);
      if (n < lower)
         lower = n;
   }

   return lower;
}

// Retrieves the upper bound of a list of integers.
extern "C"
int __cdecl
get_upper_bound(int count, ...)
{
   va_list arg_list;
   va_start(arg_list, count);
   
   int upper = INT_MIN;
   for (int i = 0; i < count; ++i)
   {
      int n = va_arg(arg_list, int);
      if (n > upper)
         upper = n;
   }

   return upper;
}

// Checks whether the given value lies within the given bounds.
// If the value is out-of-bounds, this function issues a fatal
// runtime error.
extern "C"
void __cdecl runtime_check_int_range
(
   int lower, 
   int upper, 
   int value,
   int file_index, 
   int source_line_number
)
{   
   if (! (value >= lower && value <= upper))
   {
      char buffer[257];
      ::sprintf_s(
         buffer, 
         257,
         "runtime value '%d' exceeded legal bounds (%d..%d).",
         value, lower, upper);

      fatal_error(buffer, file_index, source_line_number);
   }
}