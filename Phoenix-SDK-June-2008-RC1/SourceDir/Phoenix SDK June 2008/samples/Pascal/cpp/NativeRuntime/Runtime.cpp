//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Defines the internal startup, exit, memory management, and error 
//    handling functions.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

// Terminates the application with a failure error code.
extern "C"
void __cdecl
halt()
{
   ::exit(EXIT_FAILURE);
}

// Storage for each source file name of the program. 
// We report the source file name and line number in the case of a 
// runtime error.
// Increase the value of MAX_FILES if your program requires more files.
static char filenames[MAX_FILES][256];

// Stores the provided file path at the given index.
extern "C"
void __cdecl
set_filename(char * filename, int index)
{
   ::strcpy_s(filenames[index], 256, filename);
}

// Retrieves the file path at the given index.
char * get_filename(int index)
{
   return filenames[index];
}

// Displays the given error message and terminates the application.
void fatal_error(char * message, int file_index, int source_line_number)
{
   // Write the error message to the console.

   char buffer[257];
   ::sprintf_s(
      buffer, 
      257,
      "%s(%d) : %s Exiting...",
      get_filename(file_index), source_line_number, message);

   ::printf("\r\n%s\r\n", buffer);

   // Terminate the application.

   halt();
}

// Displays the given error message and terminates the application.
void fatal_error(DWORD dwError, LPSTR lpszFunction, int file_index, 
                  int source_line_number)
{
   CHAR szBuf[80]; 
   LPVOID lpMsgBuf;
   
   // Format the system error code to a string.

   ::FormatMessageA(
     FORMAT_MESSAGE_ALLOCATE_BUFFER | 
     FORMAT_MESSAGE_FROM_SYSTEM,
     NULL,
     dwError,
     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
     (LPSTR) &lpMsgBuf,
     0, 
     NULL
  );

   // Write the error message to the console.

   ::sprintf_s(szBuf, 80,
      "%s(%d) : %s failed with error %d: %s", 
      get_filename(file_index), source_line_number, lpszFunction, dwError, lpMsgBuf); 
 
   ::printf("\r\n%s\r\n", szBuf);
      


   ::LocalFree(lpMsgBuf);

   // Terminate the application.

   halt();
}

// Tracks the number of bytes allocated by the user program.
static int user_bytes_allocated = 0;

// Tracks the number of bytes allocated by the runtime.
static int runtime_bytes_allocated = 0;

// Holds a single chunk of user- or runtime-allocated memory.
// This structure holds the size of the memory chunk, whether it was 
// allocated by the user, and links to child memory nodes.
// Nodes are added to the binary tree by using a hash function on the 
// memory location itself.
// We track the size of memory allocations so we can report the number
// of leaked bytes when the application terminates.
// This implementation is provided for illustration purposes and not for
// performance.
struct memory_node
{
   void * memory;
   size_t size;
   char user;
   char valid;
   memory_node * left;
   memory_node * right;
};

// Head of the memory heap.
static memory_node * memory_head = NULL;

// Allocates a new memory node from the CRT heap.
memory_node * 
new_memory_node(void * memory, size_t size, char user)
{
   // Allocate a new memory node.
   memory_node * node = (memory_node *)::malloc(sizeof memory_node);
   if (node != NULL)
   {
      // Increment total runtime allocation.
      runtime_bytes_allocated += sizeof memory_node;
          
      // Fill in structure memberes.
      node->memory = memory;
      node->valid = 1;
      node->size = size;
      node->user = user;
      node->left = NULL;
      node->right = NULL;
   }
   return node;
}

// Frees the memory allocated to the given memory node 
// and all its children.
void 
free_memory_node(memory_node * node)
{
   if (node)
   {
      // Free left.
      if (node->left)
      {
         free_memory_node(node->left);
         node->left = NULL;
      }
      // Free right.
      if (node->right)
      {
         free_memory_node(node->right);
         node->right = NULL;
      }
      
      // If the memory was allocated by the runtime, free it.
      // Otherwise, we allow the memory to leak and we report the
      // error at shutdown.
      if (! node->user && node->memory)
      {
         if (node->valid)
         {
            ::free(node->memory);
            node->valid = 0;
         
            // Decrement runtime allocation count.
            runtime_bytes_allocated -= node->size;
         }
      }

      // Free the node structure itself and decrement the
      // allocation count.
      ::free(node);
      runtime_bytes_allocated -= sizeof memory_node;
   }
}

// Defines the hash function for inserting and searching for
// memory nodes.
#define HASH(memory) (void *)(((size_t)memory & 0xff))

// Inserts the given memory node into the memory node tree.
void
insert_memory_node(memory_node * node)
{
   if (memory_head == NULL)
   {
      memory_head = node;
      return;
   }

   void * memory = HASH(node->memory);
   memory_node * ptr = memory_head;
   while (1)
   {
      if (memory < HASH(ptr->memory))
      {
         if (ptr->left)
            ptr = ptr->left;
         else
         {
            ptr->left = node;
            break;
         }
      }
      else
      {
         if (ptr->right)
            ptr = ptr->right;
         else
         {
            ptr->right = node;
            break;
         }
      }
   }
}

// Finds the memory node associated with the given memory location.
memory_node *
find_memory_node(void * memory)
{
   if (memory_head == NULL)
   {
      return NULL;
   }
   memory_node * ptr = memory_head;
   void * hash_memory = HASH(memory);
   while (ptr)
   {
      if (ptr->valid && memory == ptr->memory)
         return ptr;

      if (hash_memory < HASH(ptr->memory))
         ptr = ptr->left;
      else
         ptr = ptr->right;
   }
   return NULL;
}

// Allocates memory for either the user or the runtime.
void * alloc_any(size_t size, int & alloc_count, char user)
{
   void * memory = ::malloc(size);
   if (memory)
   {  insert_memory_node(new_memory_node(memory, size, user));
      alloc_count += (int) size;
   }
   return memory;
}

// Frees memory for either the user or the runtime.
void free_any(void * memory, int & alloc_count)
{
   memory_node * node = find_memory_node(memory);
   if (node && node->valid)
   {  alloc_count -= (int) node->size;
      ::free(node->memory);
      node->valid = 0;
      node->size = 0;
   }
}

// Allocates memory for the user.
void * alloc_user(size_t size)
{
   return alloc_any(size, user_bytes_allocated, 1);
}

// Frees memory for the user.
void free_user(void * memory)
{
   free_any(memory, user_bytes_allocated);
}

// Allocates memory for the runtime.
void * alloc_runtime(size_t size)
{
   return alloc_any(size, runtime_bytes_allocated, 0);
}

// Frees memory for the runtime.
void free_runtime(void * memory)
{
   free_any(memory, runtime_bytes_allocated);
}

// Initializes the Pascal runtime library.
extern "C"
void __cdecl
runtime_init()
{
   // Initialize the function display tables.
   runtime_init_display();
}

// Cleans-up the Pascal runtime library.
extern "C"
void __cdecl
runtime_exit()
{
   // Cleanup function display structures.
   runtime_free_display();

   // Release memory allocated for memory management.
   free_memory_node(memory_head);

   // Report a runtime error if any user or runtime memory was not released.
   if (user_bytes_allocated != 0 || runtime_bytes_allocated != 0)
   {
      char buffer[256];
      ::sprintf_s(buffer, 256,
         "memory leaks detected!\r\n\t%d user bytes\r\n\t%d runtime bytes\r\n",
         user_bytes_allocated, runtime_bytes_allocated);
      fatal_error(buffer, -1, 0);
   }
}