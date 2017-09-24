//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Defines the runtime library routines for function display management.
//    Function displays allow us to implement nested procedures in Pascal.
//
//    Each Pascal user function starts with a call to display_enter. The 
//    caller of this function passes the address of each of its local 
//    variables. The runtime then pushes this call information to the top
//    of a stack for that function. When a child (nested) function needs to
//    access a variable of one of its parents, it calls the display_get_address
//    function, which returns the address of the most recent activation of that
//    variable. Because variables are referenced by address, assignments to 
//    parent local variables persist after any child procedure(s) exit.
//
//    Directly before a function exists, it calls the display_leave to clean 
//    up the most recent activation context.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include <assert.h>

// Forward declaration for the set_filename function.
extern "C"
void __cdecl
set_filename(char *, int);

//
// display_context_node
//

// Holds the list of variable addresses for a single procedure
// call.
struct display_context_node
{
   void * address_list;
   size_t address_size;
   display_context_node * next;
};

// Allocates a new display_context_node and its address list.
display_context_node *  __cdecl
alloc_display_context_node(size_t size)
{
   display_context_node * context_node = 
      (display_context_node *)::malloc(sizeof display_context_node);

   context_node->address_list = ::malloc(size);
   context_node->address_size = size;
   context_node->next = NULL;
   return context_node;
}

// Frees the given display_context_node and its address list.
void __cdecl
free_display_context_node(display_context_node * context_node)
{   
   ::free(context_node->address_list);
   ::free(context_node);
}

//
// function_display_table
//

// Holds a stack of activation contexts.
struct function_display_table
{   
   display_context_node * active_context;
   function_display_table * next;
};

// Allocates a new function_display_table.
function_display_table * __cdecl
alloc_function_display_table()
{
   function_display_table * table = 
      (function_display_table *)::malloc(sizeof function_display_table);

   table->active_context = NULL;
   table->next = NULL;

   return table;
}

// Frees the given function_display_table.
void __cdecl
free_function_display_table(function_display_table * table, 
                              int display_table_index)
{
   // The table must not have an associated context.
   if (table->active_context != NULL)
   {
      char message[256];
      ::sprintf_s(message, 256,
         "internal runtime error: display table %d contains active context.",
         display_table_index);
      fatal_error(message, display_table_index, 0);
   }
   ::free(table);
}

// Pushes the given display_context_node onto the activation stack.
void __cdecl
push_display_context_node(function_display_table * table, 
                           display_context_node * context_node)
{   
   context_node->next = table->active_context;
   table->active_context = context_node;   
}

// Removes the active display_context_node from the given display table.
void __cdecl
pop_display_context_node(function_display_table * table)
{   
   display_context_node * next_active_context = table->active_context->next;
   free_display_context_node(table->active_context);
   table->active_context = next_active_context;
}

// 
// file_index_node
//

// Holds the head of the function_display_table list for
// a given file.
struct file_index_node
{   
   function_display_table * tables;
};

// Pointer to the list of all function display tables
static file_index_node ** file_index_nodes = NULL;

// Retrieves the function_display_table at the given index
// in the given file_index_node.
function_display_table * __cdecl
get_function_display_table(file_index_node * node, int index)
{
   function_display_table * table = node->tables;
   for (int i = 0; i < index; ++i)
   {
      // Each function is assigned a unique sequential index.
      // To keep the list sequential, we may need to allocate 
      // display tables for functions that have not yet called
      // this function.
      if (table->next == NULL)
      {
         table->next = alloc_function_display_table();
      }
      table = table->next;
   }
   return table;
}

//
// External display functions.
//

// Pushes a activation context for the function associated with the given
// file and function index.
extern "C"
void __cdecl
display_enter(char * filename, int file_index, 
   int function_index, int arg_count, ...)
{
   va_list arg_list;
   va_start(arg_list, arg_count);

   // Ensure at file name is associated with the provided
   // file index.
   set_filename(filename, file_index);
   
   // Allocate a display node for the context.
   display_context_node * context_node = 
      alloc_display_context_node(sizeof(void *) * arg_count);

   // Retrieve the display table for this function and push the 
   // new context.
   function_display_table * table = 
      get_function_display_table(file_index_nodes[file_index], function_index);

   push_display_context_node(table, context_node);

   // Copy the address of each variable to the address list.
   unsigned char ** address_ptr = 
      (unsigned char **) context_node->address_list;
   while (arg_count > 0)
   {
      void * address = va_arg(arg_list, void *);
      *address_ptr = (unsigned char *) address;
      address_ptr++;      
      arg_count--;
   } 
}

// Removes the active context for the function associated with the given
// file and function index.
extern "C"
void __cdecl
display_leave(int file_index, int function_index)
{ 
   // Get the display table for the function and pop its context.

   function_display_table * table = 
      get_function_display_table(file_index_nodes[file_index], function_index);

   pop_display_context_node(table);
}

// Retrieves the address of the variable at the given index for the function 
// associated with the given file and function index.
extern "C"
void * __cdecl
display_get_address(int file_index, int function_index, int variable_index)
{ 
   // Get the display table for the function.
   function_display_table * table = 
      get_function_display_table(file_index_nodes[file_index], function_index);

   // Retrieve the active context.
   display_context_node * context_node = table->active_context;

   // Return the address at the requested index.
   unsigned char ** address_ptr = (unsigned char **) context_node->address_list;
   address_ptr += variable_index;
   return (void *) *address_ptr;
}

//
// Internal maintenance functions.
//

// Initializes the function display tables.
void __cdecl 
runtime_init_display()
{
   // Create a file_index_node for each possible file.

   file_index_nodes = (file_index_node **)
      ::malloc(sizeof (file_index_node *) * MAX_FILES);
   for (int i = 0; i < MAX_FILES; ++i)
   {    
      file_index_nodes[i] = (file_index_node *) ::malloc(sizeof file_index_node);
      file_index_nodes[i]->tables = alloc_function_display_table();
   }
}

// Frees the function display tables.
void __cdecl 
runtime_free_display()
{
   for (int i = 0; i < MAX_FILES; ++i)
   {
      if (file_index_nodes[i]->tables != NULL)
      {
         free_function_display_table(file_index_nodes[i]->tables, i);
         file_index_nodes[i]->tables = NULL;
      }
   }
}