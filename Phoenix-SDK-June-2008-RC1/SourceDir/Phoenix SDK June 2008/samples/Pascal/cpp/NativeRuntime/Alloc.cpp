//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Defines the runtime library routines for memory management.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

// Allocates a chunk of memory for the user program.
extern "C"
void * __cdecl
_new(int size, int file_index, int source_line_number)
{  
   // Allocate user memory.
   void * pointer = alloc_user((size_t) size);   

   // Report an error if the allocation failed.
   if (pointer == NULL)
      fatal_error("not enough memory.", file_index, source_line_number);

   return pointer;
}

// Frees a chunk of user program memory.
extern "C"
void __cdecl
dispose(void * pointer, int file_index, int source_line_number)
{
   // Free user memory.
   free_user(pointer);

   // Report an error if the deallocation failed.
   if (errno != 0)
   {
      char buffer[256];
      ::sprintf_s(buffer, 256, "dispose failed with error %d.", errno);
      fatal_error(buffer, file_index, source_line_number);
   }   
}
