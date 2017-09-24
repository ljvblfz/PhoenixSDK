//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the ValueRange class.
//
//-----------------------------------------------------------------------------

#pragma once

using namespace System;

namespace Pascal 
{

//-----------------------------------------------------------------------------
//
// Description: Defines a range of integer values.
//
// Remarks:
//    
//
//-----------------------------------------------------------------------------

ref class ValueRange
{
public:

   // Creates a new ValueRange object.

   ValueRange(int lower, int upper)
   {
      Lower = lower;
      Upper = upper;
   }

   // Determines whether this range contains the given value.

   bool Contains(int value)
   {
      return (value >= Lower && value <= Upper);
   }

   // Determines whether this range overlaps the given range.

   bool Contains(ValueRange ^ range)
   {
      return Contains(range->Lower) || Contains(range->Upper);
   }

   // Retrieves the range of this object.

   property int Range
   {
      int get() { return Upper - Lower + 1; }
   }

   // Creates a new ValueRange object whose range spans the given 
   // bit size.

   static ValueRange ^ 
   FromBitSize 
   (
      int bitSize
   );

   // Returns a String that represents the current Object. 

   virtual String ^ ToString() override
   {
      return String::Format("{0}..{1}", Lower, Upper);
   }

   // The lower and upper bounds of the range.

   property int Lower;
   property int Upper;
};

//-----------------------------------------------------------------------------
//
// Description: Defines a range of integer values that is associated with
//              a type.
//
// Remarks:
//    
//
//-----------------------------------------------------------------------------

ref class TypedValueRange : public ValueRange
{
public:

   // Creates a new TypedValueRange object.

   TypedValueRange(Phx::Types::Type ^ type, int lower, int upper)
      : ValueRange(lower, upper)
   {
      Type = type;
   }

   // Creates a new ValueRange object whose range spans the given 
   // bit size.

   static TypedValueRange ^ 
   FromBitSize 
   (
      Phx::Types::Type ^ type,
      int bitSize
   );

   // The Type associated with the range.

   property Phx::Types::Type ^ Type;
};

}  // namespace Pascal