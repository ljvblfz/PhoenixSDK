//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Defines the runtime library routines for file management.
//
//    Pascal is known for its inefficiency in accessing files. Calls to 'put'
//    and 'get' act on atomic data, and 'read(ln)' and 'write(ln)' calls 
//    translate to multiple calls to get and put, respectively. 
//
//    The runtime functions in the module favor a simple and straight-forward
//    file implementation over a more complex (but perhaps more efficient) one.
//
//    We use the Win32 HANDLE type for files for its flexible support for 
//    memory, disk, and console I/O.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include <assert.h>
#include <math.h>

static const int READ_SIZE = 512;

// Single shared buffer for both file reads and writes.
static char buffer[READ_SIZE+1] = {'\0'};

// Describes the mode of a file. We mark a file as 'binary' if it contains
// any unprintable characters, and 'text' otherwise. The built-in 'input'
// and 'output' files are always marked as 'text' files.
// For binary files, we can read and write the data bits directly. For
// text files, we translate the data bits to text format by using the
// sprintf and sscanf functions. 
enum file_mode
{
   FM_TEXT = 0,
   FM_BINARY = 1,
};

// Describes a single user file. We use the HANDLE value directly to 
// set the left and right children.
struct file_node
{
   HANDLE hFile;
   char path[MAX_PATH+1];
   char valid;
   char eof;
   int modifier[2]; // specifiers for writing formatted data.
   file_mode mode;

   file_node * left;
   file_node * right;
};

// Head of the file tree.
static file_node * head_file_node = NULL;

// Handles to standard input, output, and error files. These files
// map to the console, and require special treatment.
static HANDLE hStdInput = (HANDLE)0;
static HANDLE hStdOutput = (HANDLE)0;
static HANDLE hStdError = (HANDLE)0;

// Tests the given result code for errors. If an error is detected,
// this function will issues a fatal runtime error.
void
test_file_result(DWORD dwResult, LPSTR lpszFunction, 
                  int file_index, int source_line_number)
{
   if (dwResult != NO_ERROR)
      fatal_error(dwResult, lpszFunction, file_index, source_line_number);
}

// Detects the file mode of the given file.
file_mode
detect_file_mode(HANDLE hFile, int file_index, int source_line_number)
{
   // The standard files are always treated as text files.
   if (hFile == hStdInput ||
       hFile == hStdOutput ||
       hFile == hStdError)
   {
      return FM_TEXT;
   }

   // Assume text mode.
   file_mode mode = FM_TEXT;

   // Reset the file.
   ::SetFilePointer(hFile, 0L, NULL, FILE_BEGIN);
   test_file_result(::GetLastError(), "detect_file_mode", 
      file_index, source_line_number);

   // Scan the file for non-printable characters.
   int eof = 0;
   while (! eof)
   {
      DWORD nBytesRead = 0L;
      ::ReadFile(hFile, buffer, READ_SIZE, &nBytesRead, NULL);

      DWORD dw = ::GetLastError();
      if (dw == ERROR_HANDLE_EOF)
         eof = 1;
      else if (dw != NO_ERROR)
         fatal_error(dw, "detect_file_mode", file_index, source_line_number);

      if (nBytesRead == 0)
         eof = 1;

      char * ptr = buffer;
      for (dw = 0L; dw < nBytesRead; ++dw)
      {
         char c = *ptr;
         if (c < 0x20 && c != 0x0a && c != 0x0d && c != 0x09)
         {
            // Detected a non-printable character; set the mode
            // to binary and break.
            mode = FM_BINARY;
            break;
         }
         ++ptr;
      }
   }   

   // Reset the file.
   ::SetFilePointer(hFile, 0L, NULL, FILE_BEGIN);
   test_file_result(::GetLastError(), "detect_file_mode", 
      file_index, source_line_number);

   return mode;
}

// Allocates a file_node structure and sets its initial values.
file_node *
new_file_node(HANDLE hFile, char * path)
{
   file_node * node = (file_node *) alloc_runtime(sizeof(file_node));

   node->hFile = hFile;   
   ::strcpy_s(node->path, 261, path);
   node->valid = 1;
   node->eof = 0;
   node->modifier[0] = 0;
   node->modifier[1] = 0;
   node->mode = detect_file_mode(hFile, -1, 0);
   node->left = NULL;
   node->right = NULL;
     
   return node;
}

// Inserts the given file_node into the file tree.
void
file_insert_aux(file_node * node, file_node * head)
{
   // Compare the nodes by using the value of the file handle field.
   if (node->hFile < head->hFile)
   {
      if (head->left == NULL)
         head->left = node;
      else
         file_insert_aux(node, head->left);
   }
   else
   {
      if (head->right == NULL)
         head->right = node;
      else
         file_insert_aux(node, head->right);
   }
}

// Inserts the given file_node into the file tree.
void 
file_insert(file_node * node)
{
   if (head_file_node == NULL)
      head_file_node = node;
   else
      file_insert_aux(node, head_file_node);
}

// Searches for the file_node with the given file handle.
file_node *
find_file_node_aux(HANDLE hFile, file_node * node)
{
   if (node == NULL)
      return NULL;
   else if (hFile == node->hFile && node->valid)
      return node;
   else if (hFile < node->hFile)
      return find_file_node_aux(hFile, node->left);
   else
      return find_file_node_aux(hFile, node->right);
}

// Searches for the file_node with the given file handle.
file_node *
find_file_node(HANDLE hFile)
{   
   return find_file_node_aux(hFile, head_file_node);
}

// Updates the end-of-file marker for the given file node.
void
update_eof(file_node * node, int file_index, int source_line_number)
{
   HANDLE hFile = node->hFile;

   // eof is always false for the standard input file.
   if (hFile == hStdInput)
   {
      node->eof = 0;
      return;
   }
   // eof is always true for the standard output and error files.
   else if (hFile == hStdOutput ||
            hFile == hStdError)
   {
      node->eof = 1;
      return;
   }

   // Test the file by attempting to read a single byte.
   DWORD nBytesRead = 0L;
   if (0 == ::ReadFile(hFile, buffer, 1, &nBytesRead, NULL))
   {
      DWORD dw = ::GetLastError();
      if (dw == ERROR_HANDLE_EOF)
      {
         // We reached eof.
         node->eof = 1;
         return;
      }
      else
         // Another error occurred; fail.
         fatal_error(dw, "eof", file_index, source_line_number);
   }
   if (nBytesRead == 0)
   {
      node->eof = 1;
      return;
   }
  
   // Reset the file to its position before the read.
   ::SetFilePointer(hFile, -1, NULL, FILE_CURRENT);
   test_file_result(::GetLastError(), "eof", file_index, source_line_number);

   node->eof = 0;
}

// Resets the file pointer to the beginning for reading.
extern "C"
void __cdecl
file_reset(HANDLE hFile, int file_index, int source_line_number)
{
   // Nothing to do for the standard files.
   if (hFile == hStdInput ||
       hFile == hStdOutput ||
       hFile == hStdError)
   {
      return;
   }

   // Reset the file pointer to the beginning of the file.
   DWORD new_file_pointer = ::SetFilePointer(hFile, 0L, NULL, FILE_BEGIN);
   test_file_result(::GetLastError(), "reset", 
      file_index, source_line_number);
   
   // Update the eof marker.
   update_eof(find_file_node(hFile), file_index, source_line_number);
}

// Resets the file pointer to the beginning for writing.
extern "C"
void __cdecl
file_rewrite(HANDLE hFile, int file_index, int source_line_number)
{  
   // Nothing to do for the standard files.
   if (hFile == hStdInput ||
       hFile == hStdOutput ||
       hFile == hStdError)
   {
      return;
   }

   // Reset the file pointer to the beginning of the file.
   ::SetFilePointer(hFile, 0L, NULL, FILE_BEGIN);
   test_file_result(::GetLastError(), "rewrite", 
      file_index, source_line_number);

   // Mark the current position of the file as the end-of-file.
   if (0 == ::SetEndOfFile(hFile))
   {
      test_file_result(::GetLastError(), "rewrite", 
         file_index, source_line_number);
   }

   // Update the eof marker.
   update_eof(find_file_node(hFile), file_index, source_line_number);
}

// Determines whether the given file is at the end-of-file position.
extern "C"
int __cdecl
file_eof(HANDLE hFile)
{
   return (int) find_file_node(hFile)->eof;
}

// Determines whether the given file is at the end-of-line position.
extern "C"
int __cdecl
file_eoln(HANDLE hFile, char c)
{
   return find_file_node(hFile)->eof || c == '\r' || c == '\n';
}

// Generalized template for reading file data.
template<typename T>
T
file_get_basic(HANDLE hFile, char * format, int file_index, int source_line_number)
{
   T t(0);  // return value.

   // Eat any leading whitespace.
   file_eat_white(hFile, file_index, source_line_number);

   // Get the file_node associated with the file handle.
   file_node * node = find_file_node(hFile);

   // If the file is at eof, the return value is undefined.
   update_eof(node, file_index, source_line_number);
   if (file_eof(hFile))
   {
      return t;
   }

   // Read the next element depending on the file mode.
   DWORD nBytesRead = 0L;
   if (node->mode == FM_TEXT)
   {      
      // For the standard 'input' file, the ReadFile function will read until
      // the user hits the Enter key.
      if (hFile == hStdInput)
      {
         DWORD nBytesToRead = READ_SIZE;
         if (0 == ::ReadFile(hFile, buffer, nBytesToRead, &nBytesRead, NULL))
         {
            fatal_error(::GetLastError(), "get", 
               file_index, source_line_number);
         }
         // Scan formatted value and return.
         ::sscanf_s(buffer, format, &t, sizeof(t));
         return t;
      }

      // Text mode is more complex than binary.
      // Because we don't know the expected length of the string 
      // (e.g. '1' vs. '4569312'), read one character at a time
      // until we reach breaking whitespace (or and edge case such as eof).

      DWORD nBytesToRead = 1;
      int done = 0;
      while (! done)
      {
#ifdef _DEBUG
         char * debug_ptr = buffer;
#endif

         // Read a byte.
         if (0 == ::ReadFile(hFile, buffer, nBytesToRead, &nBytesRead, NULL))
         {
            fatal_error(::GetLastError(), "get", file_index, source_line_number);
         }

         bool foundWhite = false;
         int n = (int)nBytesRead;

         // Check for whitespace character.
         if (n > 0L && ::isspace(buffer[n-1]))
         {
            foundWhite = true;
            buffer[n-1] = '\0';  
         }

         // Ensure buffer is 0-terminated.
         buffer[nBytesRead] = '\0';  

         // Try to scan the current value into a temporary variable.
         T _t;
         if (0 != ::sscanf_s(buffer, format, &_t, sizeof(_t)))
         {
            // Commit the temporary value.
            t = _t;
         }
         // Check for eof/whitespace condition.
         if (nBytesRead < nBytesToRead || foundWhite)
         {
            done = true;
         }
         else
         {
            // Advance read counter.
            nBytesToRead++;
            if (nBytesToRead > READ_SIZE)
               fatal_error("'get': buffer overrun.", 
                  file_index, source_line_number);

            // Reset the file pointer and prepare for the next read.
            ::SetFilePointer(hFile, -(LONG)nBytesRead, NULL, FILE_CURRENT);
            test_file_result(::GetLastError(), "get", 
               file_index, source_line_number);
         }
      }      
   }
   else
   {
      // Binary mode is simpler because the size of data is fixed.
      if (0 == ::ReadFile(hFile, buffer, sizeof T, &nBytesRead, NULL))
      {
         fatal_error(::GetLastError(), "get", file_index, source_line_number);
      }
      ::memcpy(&t, buffer, sizeof T);      
   }
   
   return t;
}

// Specialized template for reading a single character from a file.
template<>
char
file_get_basic(HANDLE hFile, char * format,
                  int file_index, int source_line_number)
{
   char t(0);
   
   // Get the file_node associated with the file handle.
   file_node * node = find_file_node(hFile);

   // If the file is at eof, the return value is undefined. 
   update_eof(node, file_index, source_line_number);
   if (file_eof(hFile))
   {
      return t;
   }

   // For both text and binary mode, just read a single byte.

   DWORD nBytesRead = 0L;
   if (0 == ::ReadFile(hFile, buffer, sizeof t, &nBytesRead, NULL))
   {
      fatal_error(::GetLastError(), "get", file_index, source_line_number);
   }

   // Translate the value and return.
   
   if (node->mode == FM_TEXT)
   {      
     ::sscanf_s(buffer, format, &t, sizeof(t));
   }
   else
   {    
      ::memcpy(&t, buffer, sizeof t);      
   }
   
   return t;
}

// Reads a single character value from the given file.
extern "C"
char  __cdecl
file_get_char(HANDLE hFile, int file_index, int source_line_number)
{
   return file_get_basic<char>(hFile, "%c", file_index, source_line_number);
}

// Reads a single integer value from the given file.
extern "C"
int __cdecl
file_get_int(HANDLE hFile, int file_index, int source_line_number)
{
   return file_get_basic<int>(hFile, "%d", file_index, source_line_number);
}

// Reads a single Boolean value from the given file.
extern "C"
int __cdecl
file_get_bool(HANDLE hFile, int file_index, int source_line_number)
{
   return file_get_basic<int>(hFile, "%d", file_index, source_line_number) 
      == 0 ? 0 : 1;
}

// Reads a single double value from the given file.
extern "C"
double __cdecl
file_get_double(HANDLE hFile, int file_index, int source_line_number)
{
   return file_get_basic<double>(hFile, "%lf", file_index, source_line_number);
}

// Reads a string value from the given file.
extern "C"
char * __cdecl
file_get_string(HANDLE hFile, char * buffer, int string_length, 
                int file_index, int source_line_number)
{
   // Fill the entire string buffer with character data. Behavior is 
   // undefined if we reach eof.
   char * ptr = buffer;
   while (string_length--)
      *(ptr++) = file_get_char(hFile, file_index, source_line_number);
   return buffer;
}

// Generalized template for writing file data.
template<typename T>
void
file_put_basic(HANDLE hFile, T t, char * format, int file_index, int source_line_number)
{
   // Get the file_node associated with the file handle.
   file_node * node = find_file_node(hFile);

   // Reset the write modifiers. The calling function already set the format
   // string.
   node->modifier[0] = 0;
   node->modifier[1] = 0;

   // It is a fatal error to write to the middle of a file.
   if (! file_eof(hFile))
   {
      fatal_error("file write not at end of file.", 
         file_index, source_line_number);
   }

   // Write for both text and binary modes are fairly simple.
   
   if (node->mode == FM_TEXT)
   {
      // A text mode write requires formatting before WriteFile.
      int bytes_written = ::sprintf_s(buffer, 513, format, t);

      DWORD nBytesWritten = 0L;
      if (0 == ::WriteFile(hFile, buffer, bytes_written, &nBytesWritten, NULL))
      {
         fatal_error(::GetLastError(), "put", file_index, source_line_number);
      }      
   }
   else
   {
      // For binary mode, write the data directly.
      DWORD nBytesWritten = 0L;
      if (0 == ::WriteFile(hFile, (LPCVOID)&t, sizeof T, &nBytesWritten, NULL))
      {
         fatal_error(::GetLastError(), "put", file_index, source_line_number);
      }      
   }

   // Update the eof marker.
   update_eof(find_file_node(hFile), file_index, source_line_number);
}

// Writes a single character value to the given file.
extern "C"
void  __cdecl
file_put_char(HANDLE hFile, char value, int file_index, int source_line_number)
{
	char format[32];
	file_node * node = find_file_node(hFile);

   // Set the format string. If there is a format specifier, add it
   // in now.
	if (node->mode == FM_TEXT && node->modifier[0] > 0)
	{
		::sprintf_s(format, 32, "%%%dc", node->modifier[0]);
	}
	else
	{
		::strcpy_s(format, 32, "%c");
	}
	file_put_basic<char>(hFile, value, format, file_index, source_line_number);
}

// Writes a single integer value to the given file.
extern "C"
void __cdecl
file_put_int(HANDLE hFile, int value, int file_index, int source_line_number)
{
   char format[32];
	file_node * node = find_file_node(hFile);

   // Set the format string. If there is a format specifier, add it
   // in now.
	if (node->mode == FM_TEXT && node->modifier[0] > 0)
	{
		::sprintf_s(format, 32, "%%%dd", node->modifier[0]);
	}
	else
	{
		::strcpy_s(format, 32, "%d");
	}
   file_put_basic<int>(hFile, value, format, file_index, source_line_number);
}

inline int
find_string_length(char * string)
{
   char * p = string;
   while (*p)
      p++;
   return p-string;
}

// Writes a string value to the given file.
extern "C"
void __cdecl
file_put_string(HANDLE hFile, char * string_buffer, int string_length, 
                  int file_index, int source_line_number)
{   
   char format[32];
	file_node * node = find_file_node(hFile);

   // Set the format string. If there is a format specifier, add it
   // in now.
	if (node->mode == FM_TEXT && node->modifier[0] > 0)
	{
		::sprintf_s(format, 32, "%%%ds", node->modifier[0]);
	}
	else
	{
		::strcpy_s(format, 32, "%s");
	}
   if (string_length == -1)
   {
      string_length = find_string_length(string_buffer);
   }
   ::memcpy(buffer, string_buffer, string_length);
   buffer[string_length] = '\0';
   file_put_basic<char *>(hFile, buffer, format, file_index, source_line_number);
}

// Writes a single Boolean value to the given file.
extern "C"
void __cdecl
file_put_bool(HANDLE hFile, int value, int file_index, int source_line_number)
{
   file_node * node = find_file_node(hFile);

   // For text files, translate the value to a readable string.
   // Non-zero values will map to 'true'; a value of zero maps to 'false'.
   if (node->mode == FM_TEXT)
   {
      // If the user provided a minimum field width specifier, append
      // the remaining characters now.
      node->modifier[0] -= value ? ::strlen("true") : ::strlen("false");
      while (node->modifier[0] > 0)
      {
         file_put_char(hFile, ' ', file_index, source_line_number);
         node->modifier[0]--;
      }

      // Write the Boolean string.
      file_put_string(
         hFile, 
         value ? "true" : "false", 
         value ? 4 : 5,
         file_index, 
         source_line_number
      );
   }
   else
   {
      // For binary files, normalize the value to [0,1] and write the 
      // value.
      file_put_int(hFile, value ? 1 : 0, file_index, source_line_number);
   }
}

// Writes a single double value to the given file.
extern "C"
void __cdecl
file_put_double(HANDLE hFile, double value, int file_index, int source_line_number)
{
   char format[32];
	file_node * node = find_file_node(hFile);

   // Set the format string. If there is a format specifier, add it
   // in now.
	if (node->mode == FM_TEXT && 
      (node->modifier[0] > 0 || node->modifier[1] > 0))
	{
      int m0 = node->modifier[0];  // minimum field-width specifier
      int m1 = node->modifier[1];  // fraction-length specifier
      node->modifier[0] = 0;

      // Append the field-width characters against the base
      // of the value.
      double f = ::floor(value);
      ::sprintf_s(format, 32, "%d", (int)f);
      int n = m0 - ::strlen(format);
      while (n-- > 0)
         file_put_char(hFile, ' ', file_index, source_line_number);
      
      // The calls file_put_char will have cleared the fraction-length
      // specifier. Re-set the value.
      node->modifier[1] = m1;

      // Applyh the fraction-length specifier if it exists. 
      if (node->modifier[1] > 0)
         ::sprintf_s(format, 32, "%%.%dlf", node->modifier[1]);
      else
         ::strcpy_s(format, 32, "%lf");
	}
	else
	{
		::strcpy_s(format, 32, "%lf");
	}
   file_put_basic<double>(hFile, value, format, file_index, source_line_number);
}

// Retrieves the standard 'input' file handle.
extern "C"
HANDLE __cdecl
file_get_std_input(int file_index, int source_line_number)
{
   if (hStdInput)
      // Already initialized.
      return hStdInput;

   // Get the standard input handle and add a new file_node structure
   // to the file tree.

   hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
   file_node * node = new_file_node(hStdInput, "CONIN$");
   file_insert(node);
   update_eof(node, file_index, source_line_number);
   return hStdInput;
}

// Retrieves the standard 'output' file handle.
extern "C"
HANDLE __cdecl
file_get_std_output(int file_index, int source_line_number)
{
   if (hStdOutput)
      // Already initialized.
      return hStdOutput;

   // Get the standard output handle and add a new file_node structure
   // to the file tree.

   hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
   file_node * node = new_file_node(hStdOutput, "CONOUT$");
   file_insert(node);
   update_eof(node, file_index, source_line_number);
   return hStdOutput;
}

// Retrieves the standard 'error' file handle.
extern "C"
HANDLE __cdecl
file_get_std_error(int file_index, int source_line_number)
{
   if (hStdError)
      // Already initialized.
      return hStdError;

   // Get the standard error handle and add a new file_node structure
   // to the file tree.

   hStdError = ::GetStdHandle(STD_ERROR_HANDLE);
   file_node * node = new_file_node(hStdError, "CONOUT$");
   file_insert(node);
   update_eof(node, file_index, source_line_number);
   return hStdError;
}

// Opens the file at the given path and adds a new file_node to 
// the file tree.
extern "C"
HANDLE __cdecl
file_open(char * path, int temporary, int file_index, int source_line_number)
{
   HANDLE hFile;

   // Set the file attributes flag.
   // In Pascal, a file is 'temporary' if it is not declared in the 
   // program argument list.
   // If the temporary flag is set, set the attributes flag to delete
   // the file when the handle is closed.
   // As an alternative, we could use a memory file to implement
   // temporary files.
   DWORD dwAttributeFlags = FILE_ATTRIBUTE_NORMAL;
   if (temporary)
      dwAttributeFlags = FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE;

   int new_file = false;

   // Attempt to open an existing file.
   hFile = ::CreateFileA(path,
      GENERIC_READ|GENERIC_WRITE,
      0,
      NULL,
      OPEN_EXISTING,
      dwAttributeFlags,
      NULL);

   if (hFile == INVALID_HANDLE_VALUE) 
   { 
      // Create a new file.
      hFile = CreateFileA(path,
         GENERIC_READ|GENERIC_WRITE,
         0,
         NULL,
         CREATE_ALWAYS,
         dwAttributeFlags,
         NULL);

      new_file = true;

      if (hFile == INVALID_HANDLE_VALUE) 
      { 
         // File still could not be opened. Generate a runtime failure.
         test_file_result(::GetLastError(), "file_open", 
            file_index, source_line_number);
      }
   }

   // Add a new file_node for the file to the file tree.
   file_node * node = new_file_node(hFile, path);
   file_insert(node);

   // If we were able to create the file from scratch, treat it as a binary
   // file.
   if (new_file)
      node->mode = FM_BINARY;

   return hFile;
}

// Closes the given file handle.
extern "C"
void __cdecl
file_close(HANDLE hFile)
{
   file_node * node = find_file_node(hFile);
   ::CloseHandle(node->hFile);
   node->valid = 0;
}

// Stub for the Pascal 'page' procedure.
extern "C"
void __cdecl
file_page(HANDLE)
{
   // Do nothing.
}

// Eats whitespace before the next element in the file.
extern "C"
void __cdecl
file_eat_white(HANDLE hFile, int file_index, int source_line_number)
{
   // Nothing to do for the standard files. 
   if (hFile == hStdInput ||
       hFile == hStdOutput ||
       hFile == hStdError)
   {
      return;
   }

   // Retrieve the file_node associated with the file.
   file_node * node = find_file_node(hFile);

   // Nothing to do for binary files.
   if (node->mode == FM_BINARY)
      return;

   // Remember the current eof marker.
   char eof = node->eof;

   // Eat characters until we find a non-whitespace character or 
   // we reach the end-of-file.
   char c;
   do
   {
      c = file_get_basic<char>(hFile, "%c", file_index, source_line_number);
   }  while (::isspace(c) && ! node->eof);

   // If we didn't reach end-of-file (e.g. a valid non-whitespace character),
   // rewind the file by one character.
   if (! node->eof)
   {
      ::SetFilePointer(hFile, -1, NULL, FILE_CURRENT);
      test_file_result(::GetLastError(), "file_eat_white",
         file_index, source_line_number);
   }

   // Re-set the eof marker.
   node->eof = eof;
}

// Sets the minimum field-width specifier or the fraction-length specifier
// for the next subsequent write operation.
extern "C"
void __cdecl
file_set_modifier(HANDLE hFile, int which, int modifier)
{
   assert(which == 0 || which == 1);
	find_file_node(hFile)->modifier[which] = modifier;
}