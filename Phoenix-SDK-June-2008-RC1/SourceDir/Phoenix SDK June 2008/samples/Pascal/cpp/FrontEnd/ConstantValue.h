//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//     Definition of the ConstantValue class.
//
//-----------------------------------------------------------------------------

#pragma once

using namespace System;
using namespace System::Diagnostics;

//-----------------------------------------------------------------------------
//
// Description:
// 
//    Class for holding compile-time constant values.
//
// Remarks:
//    
//
//-----------------------------------------------------------------------------

namespace Pascal 
{

ref class ConstantValue
{
public:

   ConstantValue(bool value)
      : value(value)
      , type(Type::Boolean)
   {
   }
   ConstantValue(char value)
      : value(value)
      , type(Type::Character)
   {
   }
   ConstantValue(int value)
      : value(value)
      , type(Type::Integer)
   {
   }
   ConstantValue(double value)
      : value(value)
      , type(Type::Real)
   {
   }
   
   // Retrieves the constant Boolean value from this instance.
   property bool BooleanValue
   {
      bool get()
      {
         Debug::Assert(this->type == Type::Boolean);
         return static_cast<bool>(this->value);
      }
   }

   // Retrieves the constant character value from this instance.
   property char CharacterValue
   {
      char get()
      {
         Debug::Assert(this->type == Type::Character);
         return static_cast<char>(this->value);
      }
   }   

   // Retrieves the constant integer value from this instance.
   property int IntegerValue
   {
      int get()
      {
         Debug::Assert(this->type == Type::Integer);
         return static_cast<int>(this->value);
      }
   }

   // Retrieves the constant floating-point value from this instance.
   property double RealValue
   {
      double get()
      {
         Debug::Assert(this->type == Type::Real);
         return static_cast<double>(this->value);
      }
   }

   // Retrieves the constant string value from this instance.
   property Phx::Symbols::Symbol ^ StringSymbol
   {
      Phx::Symbols::Symbol ^ get()
      {
         Debug::Assert(this->type == Type::String);
         return safe_cast<Phx::Symbols::Symbol ^>(this->value);
      }
   }
     
   enum class Type
   {
      Boolean,
      Character,
      Integer,
      Real,
      String
   };

   // Retrieves the type of value this instance holds.
   Type GetType()
   {
      return this->type;
   }

   virtual String ^ ToString() override
   {
      return this->value->ToString();
   }

   // Creates a new ConstantValue object from the provided
   // immediate operand.
   static ConstantValue ^
   FromImmediateOperand
   (
      Phx::IR::ImmediateOperand ^ operand
   )
   {
      if (operand->IsIntImmediate)
         return gcnew ConstantValue(operand->IntValue32);
      else if (operand->IsFloatImmediate)
         return gcnew ConstantValue(operand->FloatValue);

      Debug::Assert(false);
      return nullptr;
   }

   // Creates a new ConstantValue object from the provided
   // Boolean symbol.
   static ConstantValue ^
   FromBoolean
   (
      Phx::Symbols::Symbol ^ booleanSymbol
   )
   {
      if (booleanSymbol->NameString->Equals("true"))
      {
         return gcnew ConstantValue(true);
      }
      else
      {
         Debug::Assert(booleanSymbol->NameString->Equals("false"));
         return gcnew ConstantValue(false);
      }
   }

   // Creates a new ConstantValue object from the provided
   // string symbol.
   static ConstantValue ^
   FromString
   (
      Phx::Symbols::Symbol ^ stringSymbol
   )
   {
      return gcnew ConstantValue(stringSymbol);
   }

private:

   ConstantValue(Phx::Symbols::Symbol ^ value)
      : value(value)
      , type(Type::String)
   {
   }

private:

   Type type;
   Object ^ value;
};

} // namespace Pascal