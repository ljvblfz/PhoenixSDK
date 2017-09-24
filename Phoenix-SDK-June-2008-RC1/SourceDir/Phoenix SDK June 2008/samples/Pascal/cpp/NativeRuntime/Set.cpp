//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Defines the runtime library routines for set functionality.
//
//    Pascal set types are native to the language, but are implemented as
//    complex data types. This file contains the backing operations for working
//    with set types.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"

#include "Runtime.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const int SET_ELEMENT_SIZE = 512;
const int BITS_PER_ELEMENT = sizeof(int) * 8;
const int SET_ARRAY_SIZE = SET_ELEMENT_SIZE / BITS_PER_ELEMENT;

// Counts the number of allocated set objects.
static int alloc_count = 0;

// Describes a single set object.
struct set_node
{
   int size;    // number of physical elements.
   int offset;  // for speed, the first element of a set is aligned to 
                // a multiple of 32. This field specifies the offset
                // to the first logical element.
   int first;   // the first logical element of the set.
   unsigned int // the actual set data.
      bits[SET_ARRAY_SIZE];
   int refs;    // reference counter.
};

// Allocates a new set_node structure.
extern "C"
set_node * __cdecl
new_set(int lower, int upper, int file_index, int source_line_number)
{  
   // Per the standard, if the range is negative, both upper- and
   // lower-bounds are set to 0.
   if (lower > upper)
      lower = upper = 0;

   // Allocate a new set_node object.
   set_node * set = (set_node *) alloc_runtime(sizeof(set_node));
   ::memset(set, 0, sizeof(set_node));
   
   // Calculate the first element.
   set->first = (BITS_PER_ELEMENT * (lower / BITS_PER_ELEMENT));

   // Adjust for negative lower-bound.
   if (lower < 0 && set->first > lower)
      set->first -= BITS_PER_ELEMENT;

   // Set logical offset.
   set->offset = lower - set->first;

   // Set physical element size. Report a fatal error if the 
   // size exceeds the set limit.
   set->size = (upper - set->first + 1);
   if (set->size > SET_ELEMENT_SIZE)
   {
      char message[256];
      ::sprintf_s(message, 256, "invalid set size (%d..%d).", lower, upper);
      fatal_error(message, file_index, source_line_number);
   }
   
   set->refs = 1;
   alloc_count++;

   return set;
}

// Creates a set_node for the empty set.
extern "C"
set_node * __cdecl
empty_set(int file_index, int source_line_number)
{
   static struct set_node empty_set_node;
   static int did_memset = 0;
   if (! did_memset)
   {
      ::memset(&empty_set_node, 0, sizeof(set_node));
      did_memset = 1;
   }

   empty_set_node.refs++;
   return &empty_set_node;
}

// Creates full copy of the given set_node.
set_node * __cdecl
copy_set(set_node * set, int file_index, int source_line_number)
{
   set_node * s = new_set(
      set->first, 
      set->first + set->size-1,
      file_index, 
      source_line_number);

   ::memcpy(s->bits, set->bits, sizeof(set->bits));

   return s;
}

// Frees the given set node.
extern "C"
void __cdecl
free_set(set_node * set)
{   
   // Free the set memory.
   free_runtime(set);
   alloc_count--;
}

// Adds one reference to the given set.
extern "C"
void __cdecl
ref_set(set_node * set)
{
   // Increment reference count.
   set->refs++;
}

// Decrements one reference from the given set.
extern "C"
void __cdecl
release_set(set_node * set)
{
   // Decrement reference count.
   set->refs--;
   // If reference count drops to zero, free the set.
   if (set->refs == 0)
      free_set(set);
}

// Calculates the highest set bit in the given set.
int
hi_bit(set_node * set)
{
   int i = SET_ARRAY_SIZE - 1;
   int n = SET_ELEMENT_SIZE;

   // Back backwards through the array and look for a 
   // non-zero element.
   while (i >= 0)
   {
      if (set->bits[i])
         break;
      n -= BITS_PER_ELEMENT;
      i--;
   }

   // Now count the number of shifts needed to 
   // clear each array element to the end
   // of the array.
   while (i < SET_ARRAY_SIZE)
   {      
      int bits = set->bits[i];
      while (bits)
      {
         bits <<= 1;
         n++;
      }    
      i++;
   }
   
   return n;
}

// Calculates the lowest set bit in the given set.
int 
lo_bit(set_node * set)
{
   int n = 0;
   int i = 0;

   // Walk through the array and look for a set bit.
   while (i < SET_ARRAY_SIZE)
   {      
      int bits = set->bits[i];

      if (!bits)
      {
         n += BITS_PER_ELEMENT;
         ++i;
         continue;
      }
      while(1)
      {
         if (bits & 0x1)
            return n;
         bits >>= 1;
      }  
   }
   return 0;
}

// Calculates a bit index from a given value.
int
bit_index_from_value(set_node * set, int value)
{
   return (value - set->first) % BITS_PER_ELEMENT;
}

// Calculates a set index from a given value.
int 
set_index_from_value(set_node * set, int value)
{
   return (value - set->first) / BITS_PER_ELEMENT;
}

// Calculates the set value at a given bit.
int
set_value_at(set_node * set, int bit)
{
   return set->first + bit;
}

// Determines whether the bit for the given value is set.
int
is_bit_set(set_node * set, int value)
{
   if (value < set->first || value > set->first + set->size - 1)
      return 0;
   
   int index = set_index_from_value(set, value);
   assert(index >= 0 && index < SET_ARRAY_SIZE);
   unsigned int mask = 0x00000001 << bit_index_from_value(set, value);
   return (set->bits[index] & mask) != 0;
}

// Sets the bit for the given value.
void
set_bit(set_node * set, int value, int type_width, 
         int file_index, int source_line_number)
{
   assert(type_width == 4 || type_width == 1);

   // Check bounds.
   if (value < set->first || value > set->first + set->size - 1)
   {
      // Generate a format string based on the element type.
      char * format = type_width == 4 ?
         "set value '%d' outside of valid range (%d..%d)." :
         "set value '%c' outside of valid range ('%c'..'%c').";

      char message[256];
      ::sprintf_s(message, 256, format, value, 
         set->first+set->offset, set->first+set->size-1);

      fatal_error(message, file_index, source_line_number);
   }

   int index = set_index_from_value(set, value);
   assert(index >= 0 && index < SET_ARRAY_SIZE);
   unsigned int mask = 0x00000001 << bit_index_from_value(set, value);
   set->bits[index] |= mask;
}

// Sets the set values for a number of ranges.
extern "C"
void __cdecl
set_set_values(int file_index, int source_line_number, set_node * set,
                  int type_width, int range_count, ...)
{
   va_list arg_list;
   va_start(arg_list, range_count);
   
   char * ranges = NULL;
   
   if (range_count > 0)
   {
      // Allocate ranges.
      ranges = (char *)alloc_runtime(type_width *   // width of type, in bytes
                        range_count *            // number of ranges
                        2);                   // 2 elements per range
      
      char * ptr = ranges;

      // Type must be char- or integer-wide.
      assert(type_width == 1 || type_width == 4);

      for (int i = 0; i < range_count; ++i)
      {
         // Read each range pair from the argument list.
         // The type_width parameter tells us how wide each 
         // element is.

         if (type_width == 1)
         {
            char c1 = va_arg(arg_list, char);
            char c2 = va_arg(arg_list, char);

            // If user specifies range [m..n] where m > n, then this 
            // denotes the empty subset.

            if (c1 > c2)
            {
               c2 = c1 = 0;
            }

            *ptr = c1;
            ++ptr;
            *ptr = c2;
            ++ptr;
         }
         else
         {
            assert(type_width == 4);

            int n1 = va_arg(arg_list, int);
            int n2 = va_arg(arg_list, int);

            // If user specifies range [m..n] where m > n, then this 
            // denotes the empty subset.

            if (n1 > n2)
            {
               n2 = n1 = 0;
            }
            
            *((int*)ptr) = n1;
            ptr += 4;
            *((int*)ptr) = n2;
            ptr += 4;
         }
      }
   }
  
    // Now set the values.
   char * ptr = ranges;
   for (int i = 0; i < range_count; ++i)
   {
      int lower, upper;
      if (type_width == 1)
      {
         lower = (int)*ptr;
         ++ptr;
         upper = (int)*ptr;
         ++ptr;
      }
      else
      {
         lower = *((int*)ptr);
         ptr += 4;
         upper = *((int*)ptr);
         ptr += 4;
      }

      for (int j = lower; j <= upper; ++j)
         set_bit(set, j, type_width, file_index, source_line_number);
   }
}

// Performs set assignment.
extern "C"
void __cdecl
set_assign(set_node * dst, set_node * src, int type_width, 
            int file_index, int source_line_number)
{
   // Clear previous values.
   ::memset(dst->bits, 0, sizeof dst->bits);

   // We could do a fast memory copy, but we want to ensure each value
   // is within range.
   for (int i = src->first + src->offset; i < src->first + src->size; ++i)
   {
      if (is_bit_set(src, i))
         set_bit(dst, i, type_width, file_index, source_line_number);
   }

   ref_set(dst);
}

// Calculates the union between two sets.
extern "C"
set_node * __cdecl
set_union(set_node * set1, set_node * set2, 
            int file_index, int source_line_number)
{
   int lo_value1 = set1->first + set1->offset;
   int hi_value1 = set1->first + set1->size - 1;

   int lo_value2 = set2->first + set2->offset;
   int hi_value2 = set2->first + set2->size - 1;

   int lower = lo_value1 < lo_value2 ? lo_value1 : lo_value2;
   int upper = hi_value1 > hi_value2 ? hi_value1 : hi_value2;

   // Create a new set to hold the union; note this may generate
   // a run-time error.

   set_node * _union = new_set(lower, upper, file_index, source_line_number);

   // Generate union values.

   int value;
   
   // First set.

   value = set1->first;
   for (int i = 0; i < SET_ARRAY_SIZE; ++i)
   {
      // Copy into empty set; we can do a plain assignment.

      _union->bits[set_index_from_value(_union, value)] = 
         set1->bits[set_index_from_value(set1, value)];

      value += BITS_PER_ELEMENT;
   }

   // Second set.

   value = set2->first;
   for (int i = 0; i < SET_ARRAY_SIZE; ++i)
   {
      // Copy into non-empty set; perform logical or.

      _union->bits[set_index_from_value(_union, value)] |= 
         set2->bits[set_index_from_value(set2, value)];

      value += BITS_PER_ELEMENT;
   }

   return _union;
}

// Calculates the intersection between two sets.
extern "C"
set_node * __cdecl
set_intersection(set_node * set1, set_node * set2, 
                  int file_index, int source_line_number)
{
   int upper1 = set1->first + set1->size;
   int upper2 = set2->first + set2->size;

   // Test for case where there is no possible intersection.

   if (upper1 < set2->first || set1->first > upper2)
      return empty_set(file_index, source_line_number);
   if (upper2 < set1->first || set2->first > upper1)
      return empty_set(file_index, source_line_number);


   int lo_value1 = set_value_at(set1, lo_bit(set1));
   int hi_value1 = set_value_at(set1, hi_bit(set1));

   int lo_value2 = set_value_at(set2, lo_bit(set2));
   int hi_value2 = set_value_at(set2, hi_bit(set2));

   int lower = lo_value1 < lo_value2 ? lo_value1 : lo_value2;
   int upper = hi_value1 > hi_value2 ? hi_value1 : hi_value2;

   // Create a new set to hold the intersection; note this may generate
   // a run-time error.

   set_node * intersection = new_set(lower, upper, file_index, source_line_number);

   // Generate intersection values.

   int value;
   
   // First set.

   value = set1->first;
   for (int i = 0; i < SET_ARRAY_SIZE; ++i)
   {
      // Copy into empty set; we can do a plain assignment.

      intersection->bits[set_index_from_value(intersection, value)] = 
         set1->bits[set_index_from_value(set1, value)];

      value += BITS_PER_ELEMENT;
   }

   // Second set.

   value = set2->first;
   for (int i = 0; i < SET_ARRAY_SIZE; ++i)
   {
      // Copy into non-empty set; perform logical and.

      intersection->bits[set_index_from_value(intersection, value)] &= 
         set2->bits[set_index_from_value(set2, value)];

      value += BITS_PER_ELEMENT;
   }

   return intersection;
}

// Calculates the difference between two sets.
extern "C"
set_node * __cdecl
set_difference(set_node * set1, set_node * set2, 
                  int file_index, int source_line_number)
{
   int upper1 = set1->first + set1->size;
   int upper2 = set2->first + set2->size;

   // Test for case where there is no possible intersection.

   if (upper1 < set2->first || set1->first > upper2 ||
       upper2 < set1->first || set2->first > upper1)
   {
      return copy_set(set1, file_index, source_line_number);      
   }

   // Generate the minimum span of values that are in 
   // set1 but not in set2.

   int diff_count = 0;
   int diff[SET_ELEMENT_SIZE];
   
   int value = set1->first + set1->offset;
   while (value < set1->first + set1->size)
   {
      if (is_bit_set(set1, value) && !is_bit_set(set2, value))
      {
         diff[diff_count] = value;
         diff_count++;
      }
      ++value;
   }

   if (diff_count == 0)
      return empty_set(file_index, source_line_number);

   set_node* difference = new_set(
      diff[0],
      diff[diff_count-1],
      file_index, 
      source_line_number
      );

   for (int i = 0; i < diff_count; ++i)
   {
      set_bit(difference, diff[i], 4, file_index, source_line_number);
   }

   return difference;
}

// Determines if two sets are equivalent.
extern "C"
int __cdecl
set_equality(set_node * set1, set_node * set2)
{
   // Trivial test for pointer equivalence.

   if (set1 == set2)
      return 1;

   // We could perform a simple memory-compare, but we want to allow
   // for situations where the sets have different possible ranges, but
   // contain the same values.

   int upper1 = set1->first + set1->size;
   int upper2 = set2->first + set2->size;

   // Test for case where there is no possible intersection.

   if (upper1 < set2->first || set1->first > upper2 ||
       upper2 < set1->first || set2->first > upper1)
   {
      return 0;
   }

   int value;

   value = set1->first + set1->offset;
   while (value < set1->first + set1->size)
   {
      if (is_bit_set(set1, value) && !is_bit_set(set2, value))
      {
         return 0;
      }
      ++value;
   }

   value = set2->first + set2->offset;
   while (value < set2->first + set2->size)
   {
      if (is_bit_set(set2, value) && !is_bit_set(set1, value))
      {
         return 0;
      }
      ++value;
   }

   return 1;
}

// Determines whether the given value is a member of the given set.
extern "C"
int __cdecl
set_membership(set_node * set, int value)
{
   return is_bit_set(set, value);
}

// Determines whether one set is a subset of the other.
extern "C"
int __cdecl
is_subset_of(set_node * set1, set_node * set2)
{
   // Trivial test for pointer equivalence.

   if (set1 == set2)
      return 1;

   // We could perform a simple memory-compare, but we want to allow
   // for situations where the sets have different possible ranges, but
   // contain the same values.

   int upper1 = set1->first + set1->size;
   int upper2 = set2->first + set2->size;

   // Test for case where there is no possible intersection.

   if (upper1 < set2->first || set1->first > upper2 ||
       upper2 < set1->first || set2->first > upper1)
   {
      return 0;
   }

   int value;

   value = set1->first + set1->offset;
   while (value < set1->first + set1->size)
   {
      if (is_bit_set(set1, value) && !is_bit_set(set2, value))
      {
         return 0;
      }
      ++value;
   }

   return 1;
}

// Determines whether one set is a superset of the other.
extern "C"
int __cdecl
is_superset_of(set_node * set1, set_node * set2)
{
   return is_subset_of(set2, set1);
}