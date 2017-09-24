//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Declaration of common memory allocation functions.
//
//-----------------------------------------------------------------------------

#pragma once

// Allocates the given number of bytes in the user memory pool.

void * __cdecl
alloc_user(size_t size);

// Frees the memory from the user memory pool.

void __cdecl 
free_user(void * memory);

// Allocates the given number of bytes in the runtime memory pool.

void * __cdecl
alloc_runtime(size_t size);

// Frees the memory from the runtime memory pool.

void __cdecl 
free_runtime(void * memory);

// Displays the given error message and terminates the application.

void __cdecl
fatal_error(char * message, int file_index, int source_line_number);

// Displays the given error message and terminates the application.

void __cdecl
fatal_error(DWORD dwError, LPSTR lpszFunction, int file_index, int source_line_number);

// Initializes the function display tables.

void __cdecl 
runtime_init_display();

// Frees the function display tables.

void __cdecl 
runtime_free_display();