//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Implementation of the TypeBuilder class.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "YaccDeclarations.h"

using namespace System::Diagnostics;

namespace Pascal
{

ValueRange ^ 
ValueRange::FromBitSize 
(
   int bitSize
)
{
   int upper = (1 << (bitSize-1))-1;
   int lower = (-upper)-1;
      
   return gcnew ValueRange(lower, upper);
}

TypedValueRange ^ 
TypedValueRange::FromBitSize 
(
   Phx::Types::Type ^ type,
   int bitSize
)
{
   int upper = (1 << (bitSize-1))-1;
   int lower = (-upper)-1;

   return gcnew TypedValueRange(type, lower, upper);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Add the provided set symbol to the list of set symbols mapped
//    to the given function unit.
//
// Remarks:
//
//    We register type ranges for subrange and enumerated types so we can
//    perform range checking.
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
TypeBuilder::RegisterTypeRange
(
   Phx::Types::Type ^ type,
   ValueRange ^ range,
   int sourceLineNumber
)
{
   // Check for existing mapping.

   if (typeRanges->ContainsKey(type))
   {
      int poolIndex = typeRanges[type];
      ValueRange ^ existingRange = rangePool[poolIndex];

      // A mapping already exists. Report an error if the provided
      // value range does not match the existing one.

      if (existingRange->Lower != range->Lower ||
          existingRange->Upper != range->Upper)
      {
         String ^ nameString = 
            type->TypeSymbol ? type->TypeSymbol->NameString : 
                               "unknown."+type->ToString();

         Output::ReportError(
            sourceLineNumber,
            Error::RangeRedefinition,
            nameString,
            existingRange,
            range
         );         
      }
      return;
   }

   // Add a range to the range pool and map the index to the
   // provided type.

   int poolIndex = GetRangePoolIndex(range);
   typeRanges->Add(type, poolIndex);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the value range associated with the provided type.
//
// Remarks:
//
//
// Returns:
//
//   The ValueRange object associated with the provided type.
//
//-----------------------------------------------------------------------------

ValueRange ^
TypeBuilder::GetValueRange
(
   Phx::Types::Type ^ type
)
{
   int poolIndex = typeRanges[type];
   return rangePool[poolIndex];
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Performs static (compile-time) range checking on the
//    provided operand.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

bool
TypeBuilder::StaticCheckRange
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Types::Type ^ type,
   Phx::IR::ImmediateOperand ^ valueOperand,
   int sourceLineNumber
)
{
   int poolIndex;
   try
   {
      poolIndex = typeRanges[type];
   }
   catch (KeyNotFoundException ^)
   {
      // Type range not mapped; report the error.

      String ^ nameString = 
         type->TypeSymbol ? type->TypeSymbol->NameString : 
                            "unknown."+type->ToString();

      Output::ReportWarning(
         sourceLineNumber,
         Error::UnknownRange,
         nameString
      );
      return true;
   }

   // Perform range check.
    
   ValueRange ^ range = rangePool[poolIndex];

   int value = valueOperand->IntValue32;
   if (value < range->Lower || value > range->Upper)
   {
      // Range check failed; report error.

      String ^ valueString = value.ToString();
      String ^ lowerString = range->Lower.ToString();
      String ^ upperString = range->Upper.ToString();

      if (type->Equals(GetTargetType(NativeType::Char)))
      {
         valueString = Char::ConvertFromUtf32((int)value);
         lowerString = "'" + Char::ConvertFromUtf32((int)range->Lower) + "'";
         upperString = "'" + Char::ConvertFromUtf32((int)range->Upper) + "'";
      }

      Output::ReportError(
         sourceLineNumber,
         Error::StaticRangeCheckFailure,
         valueString, 
         lowerString, 
         upperString
      );
      return false;
   }
   return true;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Performs dynamic (run-time) range checking on the
//    provided operand.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
TypeBuilder::DynamicCheckRange
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Types::Type ^ type,
   Phx::IR::Operand ^ valueOperand,
   int sourceLineNumber
)
{
   int poolIndex = typeRanges[type];
   ValueRange ^ range = rangePool[poolIndex];

   // Defer to the runtime to perform the range check.

   ModuleBuilder::Runtime->RuntimeCheckRange(
      functionUnit,
      Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) range->Lower
         ),
      Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) range->Upper
         ),
      valueOperand,
      Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->ParentModuleUnit->RegisterIntType,
            (int) sourceLineNumber
         ),
      sourceLineNumber
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Performs dynamic (run-time) range checking on the
//    provided operand.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
TypeBuilder::DynamicCheckRange
(
   Phx::FunctionUnit ^ functionUnit,      
   Phx::IR::Operand ^ valueOperand,
   ValueRange ^ range,
   int sourceLineNumber
)
{
   // Defer to the runtime to perform the range check.

   ModuleBuilder::Runtime->RuntimeCheckRange(
      functionUnit,
      Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) range->Lower
         ),
      Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) range->Upper
         ),
      valueOperand,
      Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->ParentModuleUnit->RegisterIntType,
            (int) sourceLineNumber
         ),
      sourceLineNumber
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the two provided Type objects are equivalent.
//
// Remarks:
//
//
// Returns:
//
//   true if the types are compatible; false otherwise.
//
//-----------------------------------------------------------------------------

bool
TypeBuilder::AreEquivalentTypes
(
   Phx::Types::Type ^ first,
   Phx::Types::Type ^ second
)
{
   // Trivial case; types are equivalent.

   if (first->Equals(second))
   {
      return true;
   }

   // For set types, one may be the 'empty set' type.

   if (IsSetType(first) && IsSetType(second))
   {
      Phx::Types::Type ^ epsilonType = GetTargetType(NativeType::Epsilon);
      return first->Equals(epsilonType) || second->Equals(epsilonType);
   }

   // For pointer types, one may be the 'nil pointer' type.

   if (IsPointerType(first) && IsPointerType(second))
   { 
      return second->Equals(GetTargetType(NativeType::NilPointer));
   }

   Phx::Types::Type ^ realType = GetTargetType(NativeType::Real);
   Phx::Types::Type ^ integerType = GetTargetType(NativeType::Integer);

   // We allow implicit real <--> integer conversions

   if ( (first->Equals(realType) || first->Equals(integerType)) &&
        (second->Equals(realType) || second->Equals(integerType)) )
         return true;

   // Test for right-hand side being a function call,
   // such as: x := f

   if (second->IsFunctionType)
   {
      return AreEquivalentTypes(
         first, 
         second->AsFunctionType->ReturnType
      );
   }

   // Test for left-hand side begin a function call,
   // such as f(a)<0

   if (first->IsFunctionType)
   {
      return AreEquivalentTypes(
         first->AsFunctionType->ReturnType,
         second
      );
   }

   return false;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the primitive ordinal type with the given size.
//
// Remarks:
//
//    This method will return the built-in char or integer primitive type.
//
// Returns:
//
//   The primitive ordinal type with the given size.
//
//-----------------------------------------------------------------------------

Phx::Types::PrimitiveType ^
TypeBuilder::GetOrdinalType
(
   int byteSize
)
{
   array<Phx::Types::Type ^> ^ ordinalTypes = {
      GetTargetType(NativeType::Char),
      GetTargetType(NativeType::Integer),
   };

   for each(Phx::Types::Type ^ type in ordinalTypes)
   {
      if (type->ByteSize == byteSize)
         return type->AsPrimitiveType;
   }

   Debug::Assert(false);
   return nullptr;   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the type that results when the provided 
//    arithmetic operator is applied to operands of the given types.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

Phx::Types::Type ^ 
TypeBuilder::GetBinaryArithmeticResult
(
   Phx::Types::Type ^ first,
   Phx::Types::Type ^ second,
   unsigned op
)
{
   Phx::Types::Type ^ realType    = GetTargetType(NativeType::Real);
   Phx::Types::Type ^ integerType = GetTargetType(NativeType::Integer);
   Phx::Types::Type ^ booleanType = GetTargetType(NativeType::Boolean);

   // Test for function types,    
   // such as: x/f or f*x
   // where f designates a function call, and x designates
   // a type matching the return value of f.

   if (first->IsFunctionType)
      first = first->AsFunctionType->ReturnType;
   if (second->IsFunctionType)
      second = second->AsFunctionType->ReturnType;

   switch (op)
   {
   case PLUS:
   case MINUS:
   case STAR:
      
      // If either type is real, the result is real. Otherwise, the result
      // is integer.

      if (first->Equals(realType) || second->Equals(realType))
         return realType;
      return integerType;

   case DIV:

      // DIV always produces integer type.

      return integerType;

   case SLASH:

      // SLASH always produces real type.

      return realType;

   case AND:
   case OR:

      // AND and OR always produce Boolean type.

      return booleanType;

   default:
      __assume(0);
      Debug::Assert(false);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the provided types are compatible with
//    the provided relational operator.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool
TypeBuilder::AreRelationalTypesCompatible
(
   Phx::Types::Type ^ firstType,
   Phx::Types::Type ^ secondType,
   unsigned relationalOperator
)
{
   // Test for function types,    
   // such as: x < f or f > x
   // where f designates a function call, and x designates
   // a type matching the return value of f.

   if (firstType->IsFunctionType)
      firstType = firstType->AsFunctionType->ReturnType;
   if (secondType->IsFunctionType)
      secondType = secondType->AsFunctionType->ReturnType;
   
   switch (relationalOperator)
   {
   case EQUAL:
   case NOTEQUAL:

      if ( IsSetType(firstType) && IsSetType(secondType) )
      {
         // Types must contain same element type.

         return AreEquivalentTypes(firstType, secondType);
      }

      {
         bool areSimpleType = false;
         bool arePointerType = false;
         bool areStringType = false;

         areSimpleType = IsSimpleType(firstType) && 
            IsSimpleType(secondType);
         if (!areSimpleType)
         {
            arePointerType = IsPointerType(firstType) && 
               IsPointerType(secondType);
            if (!arePointerType)
            {
               areStringType = IsStringType(firstType) && 
                  IsStringType(secondType);
            }
         }

         // Both operands must be simple, pointer, set, or string type.

         if (areSimpleType)
            return true;

         if (arePointerType)
         {
            // Both types must refer to the same base type, or one or both can
            // be the 'nil pointer' type.

            Phx::Types::Type ^ nilPointerType = 
               GetTargetType(NativeType::NilPointer);

            if (firstType->Equals(nilPointerType) || 
                secondType->Equals(nilPointerType))
                  return true;

            return firstType->AsPointerType->ReferentType->Equals(
               secondType->AsPointerType->ReferentType);
         }

         if (areStringType)
            
            // Both types must contain the same element count.
            
            return firstType->ByteSize == secondType->ByteSize;
      }
      break;

   case LE:
   case GE:

      // This is LTE, GTE, or set inclusion.
      // Both operands must be simple, string, or set.

      if ( IsSetType(firstType) && IsSetType(secondType) )
      {
         // Types must contain same element type.

         return AreEquivalentTypes(firstType, secondType);
      }

      if ( IsSimpleType(firstType) && IsSimpleType(secondType) )
         return true;

      if ( IsStringType(firstType) && IsStringType(secondType) )
      
         // Both types must contain the same element count.

         return firstType->ByteSize == secondType->ByteSize;

      break;
   
   case LT:
   case GT:
      
      // Both operands must be simple or string.

      if ( IsSimpleType(firstType) && IsSimpleType(secondType) )
         return true;

      if ( IsStringType(firstType) && IsStringType(secondType) )
      
         // Both types must contain the same element count.

         return firstType->ByteSize == secondType->ByteSize;

      break;

   case IN:

      // For set membership, the first operand must be of ordinal
      // type and the second operand must be of set type.
      
      if ( (IsOrdinalType(firstType) && IsSetType(secondType)) )
      {         
         // Ordinal type must be compatible with set type's element type.

         Debug::Assert(secondType->IsPointerType);
         return AreEquivalentTypes(firstType, 
            secondType->AsPointerType->ReferentType);
      }
      break;

   default:
      Debug::Assert(false);
   }

   return false;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given type is an ordinal type.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool
TypeBuilder::IsOrdinalType
(
   Phx::Types::Type ^ type
)
{
   // Enumerated and subrange types are backed by the ordinal integer type.

   if (type->IsEnumType)
   {
      return true;
   }

   if (IsSubrangeType(type))
   {
      return true;
   }

   // Check for integer, Boolean, or char type.

   return type->Equals(TypeBuilder::GetTargetType(NativeType::Integer)) ||
          type->Equals(TypeBuilder::GetTargetType(NativeType::Boolean)) ||
          type->Equals(TypeBuilder::GetTargetType(NativeType::Char));
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given type is a real type.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool
TypeBuilder::IsRealType
(
   Phx::Types::Type ^ type
)
{
   return type->Equals(TypeBuilder::GetTargetType(NativeType::Real));
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given type is a simple type.
//
// Remarks:
//
//    A simple type is a real or ordinal type.
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool
TypeBuilder::IsSimpleType
(
   Phx::Types::Type ^ type
)
{
   return IsRealType(type) || IsOrdinalType(type);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given type is a string type.
//
// Remarks:
//
//
// Returns:
//
//    true if the provided type is a string type; false otherwise.
//
//-----------------------------------------------------------------------------

bool
TypeBuilder::IsStringType
(
   Phx::Types::Type ^ type
)
{
   Phx::Types::Type ^ charType = GetTargetType(NativeType::Char);

   // 'string' types may be either a declared as '[packed] array of char'
   // or used directly in calls to the write(ln) procedure.

   if (type->IsArray &&
       type->AsUnmanagedArrayType->ElementType->Equals(charType))
   {
      // Array of char.

      return true;
   }
   if (type->IsPointer &&
       type->AsPointerType->ReferentType->Equals(charType))
   {
      // Pointer to char.

      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the constant length of the given string type.
//
// Remarks:
//
//    The length of a Pascal string is part of its type.
//
// Returns:
//
//   The number of characters in the string, or -1 if the length could not
//   be determined.
//
//-----------------------------------------------------------------------------

int
TypeBuilder::GetStringLength
(
   Phx::Symbols::Symbol ^ symbol,
   Phx::Types::Type ^ stringType
)
{   
   Debug::Assert(IsStringType(stringType));

   // If the string type is an array type, the string length is 
   // simply the size of the array.

   if (stringType->IsArray)
   {
      return stringType->ByteSize;
   }

   // Otherwise, if the string has an associated symbol, look up
   // the string length in the 'string lengths' table.

   if (symbol)
   {
      // We expect the symbol to either be a proxy or an existing
      // global symbol.

      if (symbol->IsNonLocalVariableSymbol)
         symbol = symbol->AsNonLocalVariableSymbol->GlobalSymbol;
      Debug::Assert(symbol->IsGlobalVariableSymbol);

      try
      {
         return ModuleBuilder::StringSymbolLengths
            [symbol->AsGlobalVariableSymbol->ExternId];
      }
      catch (KeyNotFoundException ^)
      {
      }
   }

   Debug::Assert(stringType->IsPointer);
   // Unknown size.
   return -1;   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given type is a set type.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool
TypeBuilder::IsSetType
(
   Phx::Types::Type ^ type
)
{
   return registeredSetTypes && 
          registeredSetTypes->Contains(type);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given type is a file type.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool
TypeBuilder::IsFileType
(
   Phx::Types::Type ^ type
)
{
   // Trivial rejection: type must be an aggregate type.

   if (! type->IsAggregateType)
      return false;

   // Trivial rejection: type must have a valid type symbol and name.

   if (type->TypeSymbol == nullptr)
      return false;
   if (type->TypeSymbol->NameString == nullptr || 
       type->TypeSymbol->NameString->Length == 0)
      return false;

   // Look for a registered file type that equals the provided type.

   for each (Phx::Types::AggregateType ^ fileType in registeredFileTypes)
   {
      if (fileType->Equals(type) && 
          fileType->TypeSymbol->NameString->
            Equals(type->TypeSymbol->NameString))
         return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given type is a pointer type.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool
TypeBuilder::IsPointerType
(
   Phx::Types::Type ^ type
)
{
   // Check for the built-in 'nil' pointer type.

   if (type->Equals(GetTargetType(NativeType::NilPointer)))
   {
      return true;
   }

   // Check for a registered pointer type.

   if (registeredPointerTypes && type->IsPointerType)
   {
      return registeredPointerTypes->Contains(type->AsPointerType);
   }
   return false;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given type is a subrange type.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool
TypeBuilder::IsSubrangeType
(
   Phx::Types::Type ^ type
)
{
   return registeredSubrangeTypes &&
          registeredSubrangeTypes->Contains(type);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given type is a valid base type for
//    a set type.
//
// Remarks:
//
//    Set types may only contain ordinal or subrange types.
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool 
TypeBuilder::IsValidSetBaseType
(
   Phx::Types::Type ^ type
)
{
   return IsOrdinalType(type) || IsSubrangeType(type);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the Type object associated with the given Pascal
//    program type name.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

Phx::Types::Type ^ 
TypeBuilder::GetTargetType
(
   String ^ sourceType
)
{
   // Lazy create the type maps on initial usage.

   if (targetTypes == nullptr)
      InitializeTypeMaps();

   // Retrieve the type from our side table of 
   // registered types.

   try
   {
      return targetTypes[sourceType->ToLower()];
   }
   catch (KeyNotFoundException ^)
   {
      // Type not found; report an error and return the
      // 'unknown' type.

      Output::ReportFatalError(
         Error::UnknownType,
         sourceType
      );
      return GetTargetType(NativeType::Unknown);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the Type object associated with the given Pascal
//    program type name.
//
// Remarks:
//
//    Call this method when you want to check for the target type but 
//    you do not want to report an error on failure.
//
// Returns:
//
//
//-----------------------------------------------------------------------------

Phx::Types::Type ^ 
TypeBuilder::TryGetTargetType
(
   String ^ sourceType, 
   bool % succeeded
)
{
   // Lazy create the type maps on initial usage.

   if (targetTypes == nullptr)
      InitializeTypeMaps();

   // Retrieve the type from our side table of 
   // registered types.

   try
   {
      succeeded = true;      
      return targetTypes[sourceType->ToLower()];
   }
   catch (KeyNotFoundException ^)
   {
      // Type not found; report failure.

      succeeded = false;
      return nullptr;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the Type object associated with the given Pascal
//    NativeType value.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

Phx::Types::Type ^ 
TypeBuilder::GetTargetType
(
   NativeType sourceType
)
{
   // Lazy create the type maps on initial usage.

   if (targetTypes == nullptr)
      InitializeTypeMaps();

   // Translate the enum value to a string.

   String ^ sourceName = nullptr;

   switch (sourceType)
   {
   case NativeType::Text:
      sourceName = "file of char";
      break;
   
   case NativeType::NilPointer:
      sourceName = "nil";
      break;
   
   case NativeType::Boolean:
   case NativeType::Char:
   case NativeType::Integer:
   case NativeType::Real:
   case NativeType::File:   
   case NativeType::Set:
   case NativeType::Epsilon:
      sourceName = sourceType.ToString();
      break;

   default:
      Diagnostics::Debug::Assert(false);
   case NativeType::Unknown:
      sourceName = "(unknown)";
      break;
   }

   // Call the string version of the method to 
   // resolve the type.

   return GetTargetType(sourceName);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the Pascal program type name associated with the given
//    Type object.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

String ^
TypeBuilder::GetSourceType
(
   Phx::Types::Type ^ targetType
)
{   
   // Lazy create the type maps on initial usage.

   if (targetTypes == nullptr)
      InitializeTypeMaps();

   // Retrieve the type from our side table of 
   // registered types.

   try
   {
      String ^ result = sourceTypes[targetType];

      // We want to return a descriptive name to the caller.
      // If the type is an anonymous type, expand its name
      // to be more descriptive.

      if (result->StartsWith("$$unnamed"))
      {
         if (targetType->IsEnumType)
         {
            return "(unnamed enumeration)";
         }
         else if (IsSubrangeType(targetType))
         {
            return "(unnamed) subrange of " + 
               GetSourceType(
                  targetType->NotRenamedType);
         }
         else if (targetType->IsArray)
         {
            return "(unnamed) array of " + 
               GetSourceType(
                  targetType->AsUnmanagedArrayType->ElementType);
         }
         else if (IsSetType(targetType))
         {
            return "(unnamed) set of " + 
               GetSourceType(
                  targetType->AsPointerType->ReferentType);
         }
         else if (IsFileType(targetType))
         {
            return "(unnamed) file of " + 
               GetSourceType(
                  GetFileComponentType(targetType->AsAggregateType));
         }
         else if (targetType->IsPointerType)
         {
            return "(unnamed) pointer to " + 
               GetSourceType(
                  targetType->AsPointerType->ReferentType);
         }
         Debug::Assert(false);      
         return "unknown type";
      }
      return result;
   }
   catch (KeyNotFoundException ^)
   {
      // There was no mapping to the provided type. 
      // Try to figure out what the type is so we can provide
      // a descriptive name.

      if (targetType->IsPointerType)
      {
         targetType = targetType->AsPointerType->ReferentType;
         return "pointer to " + GetSourceType(targetType);
      }
      else if (IsStringType(targetType))
      {
         int elementCount = targetType->ByteSize /
            targetType->AsUnmanagedArrayType->ElementType->ByteSize;

         return "string(" + elementCount + ")";
      }      
      else if (targetType->IsFunctionType)
      {
         Phx::Types::FunctionType ^ functionType = targetType->AsFunctionType;
         Text::StringBuilder ^ sb = gcnew Text::StringBuilder();
         
         if (functionType->HasVoidReturn)         
            sb->Append("procedure");
         else
            sb->Append("function");

         if (functionType->Parameters.Count > 0)
         {
            sb->Append(" (");
            String ^ sep = String::Empty;
            for each (Phx::Types::Parameter parameter in functionType->Parameters)
            {
               sb->Append(sep);
               sep = "; ";
               sb->Append(GetSourceType(parameter.Type));
            }
            sb->Append(")");
         }

         if (! functionType->HasVoidReturn)         
         {
            sb->Append(" : ");
            sb->Append(GetSourceType(functionType->ReturnType));
         }

         return sb->ToString();
      }
      else if (targetType->IsUnknown)
      {
         return "unknown type";
      }

      Debug::Assert(false);
      return "unknown type";
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Registers the given enum type and its symbols with the type system.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
TypeBuilder::RegisterEnumType
(
   Phx::Types::EnumType ^ type, 
   List<Phx::Symbols::Symbol ^> ^ scalarSymbols
)
{
   Debug::Assert(type->IsEnumType);

   // Lazy create the enum type registration map.

   if (registeredEnumTypes == nullptr)
      registeredEnumTypes = gcnew List<EnumType ^>();

   // Create a container to hold the enum type and its member list.
   // The value of each symbol in the list is determined by its 
   // list index.

   EnumType ^ enumType = gcnew EnumType();
   enumType->Type = type;
   enumType->ScalarSymbols = scalarSymbols;

   registeredEnumTypes->Add(enumType);

   // Add to built-in type mappings.

   String ^ name = type->TypeSymbol->Name.NameString->ToLower();
   targetTypes[name] = type;
   sourceTypes[type] = name;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Registers the given subrange type and its range with the type system.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
TypeBuilder::RegisterSubrangeType
(
   Phx::Types::Type ^ type,
   ValueRange ^ range,
   int sourceLineNumber
)
{
   // Lazy create the subrange type registration map.

   if (registeredSubrangeTypes == nullptr)
      registeredSubrangeTypes = gcnew List<Phx::Types::Type ^>();
  
   if (! registeredSubrangeTypes->Contains(type))
   {
      registeredSubrangeTypes->Add(type);
   }

   // Register type range.

   RegisterTypeRange(
      type,
      range,
      sourceLineNumber
   );

   // Add to type range list.

   if (! typeRanges->ContainsKey(type))
   {
      typeRanges->Add(
         type,
         GetRangePoolIndex(range)
      );
   }

   // Add to built-in type mappings.

   String ^ name = type->TypeSymbol->Name.NameString->ToLower();
   targetTypes[name] = type;
   sourceTypes[type] = name;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Registers the given aggregate type with the type system.
//
// Remarks:
//
//
// Returns:
//
//   The symbol table associated with the aggregate type.
//
//-----------------------------------------------------------------------------

Phx::Symbols::Table ^
TypeBuilder::RegisterAggregateType
(
   Phx::Types::AggregateType ^ type
)
{
   // Create a symbol table for the aggregate type.

   Phx::Symbols::Table ^ symbolTable = 
      Phx::Symbols::Table::New(
         nullptr,
         64,
         true
      );

   symbolTable->AddMap(Phx::Symbols::NameMap::New(symbolTable, 64));

   // Add the type and symbol table to the type mapping.

   registeredAggregateTypes->Add(
      type, 
      symbolTable
   );

   // Add to built-in type mappings.

   String ^ name = type->TypeSymbol->Name.NameString->ToLower();
   targetTypes[name] = type;
   sourceTypes[type] = name;

   return symbolTable;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds a new field to the given aggregate type.
//
// Remarks:
//
//
// Returns:
//
//   The new field that was added to the provided aggregate type.
//
//-----------------------------------------------------------------------------

Phx::Types::Field ^
TypeBuilder::AddFieldToType
(
   Phx::Lifetime ^ lifetime,
   Phx::Types::AggregateType ^ type,
   Phx::Types::Type ^ fieldType,
   String ^ name,
   int absoluteBitOffset,
   int sourceLineNumber
)
{
   Phx::Name fieldName = Phx::Name::New(lifetime, name);

   // Ensure that the field type does not match 
   // the aggregate type.
   
   if (fieldType->Equals(type))
   {
      Output::ReportError(
         sourceLineNumber, Error::RecursiveRecordDefinition,
         TypeBuilder::GetSourceType(type));

      // Continue processessing so we can find other errors.
      // The error will prevent the compiler from 
      // generating code.
   }
   
   // If the field already exists, report the error.
   // Otherwise, create a new symbol for the identifier.

   Phx::Symbols::Table ^ symbolTable = 
      TypeBuilder::GetSymbolTable(type);

   Phx::Symbols::Symbol ^ symbol = 
      symbolTable->NameMap->Lookup(fieldName);
   
   if (symbol != nullptr)
   {
      Output::ReportError(
         sourceLineNumber, Error::FieldRedeclaration,
         fieldName.NameString, 
         TypeBuilder::GetSourceType(symbol->Type));
      return nullptr;
   }
   else
   {
      // Create a field symbol for the field.

      Phx::Symbols::FieldSymbol ^ fieldSymbol = 
         Phx::Symbols::FieldSymbol::New(
            symbolTable, 0, fieldName, fieldType);

      // If the bit offset is valid (non-negative),
      // set it. Otherwise, find the next available bit offset 
      // for the field.

      if (absoluteBitOffset >= 0)
      {
         fieldSymbol->BitOffset = absoluteBitOffset;
      }
      else
      {
         fieldSymbol->BitOffset = GetNextBitOffset(type);
      }

      // Pascal aggregate fields have public access by default.

      fieldSymbol->Access = Phx::Symbols::Access::Public;

      // Append the new field symbol and return.

      return type->AppendFieldSymbol(fieldSymbol);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the field symbol with the given name from
//    the given aggregate type.
//
// Remarks:
//
//
// Returns:
//
//   The field symbol that matches the given field name, or nullptr if
//   no symbol could be found.
//
//-----------------------------------------------------------------------------

Phx::Symbols::FieldSymbol ^ 
TypeBuilder::FindFieldSymbol
(
   Phx::Types::AggregateType ^ aggregateType,
   String ^ fieldName
)
{
   // Walk the field symbol list and look for a field symbol with the
   // provided name.

   Phx::Symbols::FieldSymbol ^ fieldSymbol = 
      aggregateType->FieldSymbolList;
   while (fieldSymbol && ! fieldSymbol->NameString->Equals(fieldName))
   {
      fieldSymbol = fieldSymbol->NextFieldSymbol;
   }
   return fieldSymbol;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Changes the type of the field with the given name.
//
// Remarks:
//
//    This method is used to morph the field types of the built-in 'input'
//    and 'output' files. Because these files can hold any data type at 
//    any time, this method allows us to produce valid typed IR.
//
// Returns:
//
//   The field symbol that corresponds to the provided field name.
//
//-----------------------------------------------------------------------------

Phx::Symbols::FieldSymbol ^ 
TypeBuilder::ChangeFieldType
(
   Phx::Types::AggregateType ^ aggregateType,
   String ^ fieldName,
   Phx::Types::Type ^ newType
)
{
   // Find the field symbol that corresponds to the provided field name.

   Phx::Symbols::FieldSymbol ^ fieldSymbol = FindFieldSymbol(
      aggregateType,
      fieldName
   );

   // If the search succeeded, overwrite the symbol's Type field.

   if (fieldSymbol)
   {
      fieldSymbol->Type = newType;
   }
   return fieldSymbol;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the symbol table associated with the given 
//    aggregate type.
//
// Remarks:
//
//
// Returns:
//
//   The symbol table assocated with the provided aggregate type.
//
//-----------------------------------------------------------------------------

Phx::Symbols::Table ^
TypeBuilder::GetSymbolTable
(
   Phx::Types::AggregateType ^ type
)
{
   return registeredAggregateTypes[type];
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the range of valid values for the given 
//    subrange type.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

ValueRange ^
TypeBuilder::GetSubrangeTypeRange
(
   Phx::Types::Type ^ type
)
{
   Debug::Assert(IsSubrangeType(type));
   Debug::Assert(typeRanges != nullptr);
   Debug::Assert(rangePool != nullptr);

   // The type range is mapped to a pool of ranges so that distinct
   // ranges are stored in memory only once.

   int poolIndex = typeRanges[type];
   return rangePool[poolIndex];
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Registers the given array type with the type system.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
TypeBuilder::RegisterArrayType
(
   Phx::Types::Type ^ type,
   array<TypedValueRange ^> ^ indexRanges,
   int sourceLineNumber
)
{
   Debug::Assert(type->IsArray);

   // Lazy create the array type registration map.

   if (registeredArrayTypes == nullptr)
      registeredArrayTypes = gcnew List<Phx::Types::Type ^>();
  
   // Add the type to the type mapping.

   registeredArrayTypes->Add(type);

   // Register subtype index ranges.

   array<ArrayIndexType ^> ^ rangeIndices = 
      gcnew array<ArrayIndexType ^>(indexRanges->Length);

   for (int i = 0; i < indexRanges->Length; ++i)
   {
      ArrayIndexType ^ arrayIndexType = gcnew ArrayIndexType();
      arrayIndexType->Type = indexRanges[i]->Type;
      arrayIndexType->PoolIndex = GetRangePoolIndex(
         gcnew ValueRange(
            indexRanges[i]->Lower, 
            indexRanges[i]->Upper)
         );

      rangeIndices[i] = arrayIndexType;
   }

   arrayTypeIndexRanges->Add(
      type,
      rangeIndices      
   );

   // Add to built-in type mappings.

   String ^ name = type->TypeSymbol->Name.NameString->ToLower();
   targetTypes[name] = type;
   sourceTypes[type] = name;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the typed index ranges for each rank of the
//    given array type.
//
// Remarks:
//
//    This method handles multiple-dimension arrays. For example, for the array
//       M[black..white][40..50]
//    this method returns two typed value ranges; the first for the enum range
//    and the second for the integer subrange.
//
// Returns:
//
//
//-----------------------------------------------------------------------------

array<TypedValueRange ^> ^
TypeBuilder::GetArrayIndexRanges
(
   Phx::Types::UnmanagedArrayType ^ type
)
{
   Debug::Assert(type->IsArray);
   Debug::Assert(arrayTypeIndexRanges != nullptr);

   // Get the array of index types for the array type.
   
   array<ArrayIndexType ^> ^ arrayIndexTypes = 
      arrayTypeIndexRanges[type];

   // Create a new array to hold the return values.

   array<TypedValueRange ^> ^ typedValueRanges = 
      gcnew array<TypedValueRange ^>(arrayIndexTypes->Length);

   // Add each registered range to the array.

   for (int i = 0; i < arrayIndexTypes->Length; ++i)
   {
      ValueRange ^ range = rangePool[arrayIndexTypes[i]->PoolIndex];

      typedValueRanges[i] = gcnew TypedValueRange(
         arrayIndexTypes[i]->Type,
         range->Lower,
         range->Upper
      );
   }

   return typedValueRanges;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the ordinal value associated with the symbol
//    of the given enum type.
//
// Remarks:
//
//
// Returns:
//
//   The ordinal value for the given enum member.
//
//-----------------------------------------------------------------------------

Int32
TypeBuilder::GetEnumOrdinalValue
(
   Phx::Types::EnumType ^ type, 
   Phx::Symbols::Symbol ^ scalarSymbol
)
{
   Debug::Assert(registeredEnumTypes != nullptr);
   
   // Search the registered enum type list for the 
   // enum type.

   int index = -1;
   for each (EnumType ^ enumType in registeredEnumTypes)
   {
      if (enumType->Type->Equals(type))
      {
         // The static value of the enum member is its index
         // in the symbol list.

         index = enumType->ScalarSymbols->IndexOf(scalarSymbol);
         break;
      }
   }

   Debug::Assert(index >= 0);
   return index;   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the upper bound of the given enum type.
//
// Remarks:
//
//    The upper bound of an enum type is the number of elements - 1.
//    The lower bound of all enum types is implicitly 0.
//
// Returns:
//
//   The ordinal value of the upper bound of the given enum type, or -1
//   if the type is not registered.
//
//-----------------------------------------------------------------------------

Int32
TypeBuilder::GetEnumUpperBound
(
   Phx::Types::EnumType ^ type
)
{
   Debug::Assert(registeredEnumTypes != nullptr);
   
   // Search the registered enum type list for the 
   // enum type.

   for each (EnumType ^ enumType in registeredEnumTypes)
   {
      if (enumType->Type->Equals(type))
      {
         // Return the symbol count - 1.

         return enumType->ScalarSymbols->Count - 1;
      }
   }

   Debug::Assert(false);
   return -1;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a unique type name.
//
// Remarks:
//
//    This method is useful for generating unique names for anonymous types.
//
// Returns:
//
//
//-----------------------------------------------------------------------------

String ^
TypeBuilder::GetUniqueTypeName()
{
   // Generate a name that is not accessible through user-code by using
   // characters that are invalid for a Pascal program.

   String ^ nameString = String::Format(
      "$$unnamed{0,-3:G}", uniqueTypeNameId);

   ++uniqueTypeNameId;
   return nameString;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Registers the given set type with the type system.
//
// Remarks:
//
//
// Returns:
//
//   The Type object for the new set type.
//
//-----------------------------------------------------------------------------

Phx::Types::Type ^
TypeBuilder::RegisterSetType
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ typeName,
   Phx::Types::Type ^ baseType
)
{
   // Lazy create the set type registration map.

   if (registeredSetTypes == nullptr)
   {
      registeredSetTypes  = gcnew List<Phx::Types::Type ^>(); 

      // Add the 'empty set' type to the mapping.

      registeredSetTypes->Add(GetTargetType(NativeType::Epsilon));      
   }

   // Create a new 'pointer to base type' type for the set type.
   // We create a unique type for compile-time type checking.

   Phx::Symbols::TypeSymbol ^ typeSymbol = 
      Phx::Symbols::TypeSymbol::New(
         functionUnit->SymbolTable, 0,
         Phx::Name::New(functionUnit->Lifetime, typeName)
      );

   Phx::Types::PointerType ^ setType = 
      Phx::Types::PointerType::New(
         functionUnit->TypeTable,
         Phx::Types::PointerTypeKind::UnmanagedPointer, 
         functionUnit->TypeTable->NativePointerBitSize,
         baseType,
         typeSymbol
      );

   // Add the type to the type mapping.

   registeredSetTypes->Add(setType);
   
   // Add to built-in type mappings.

   targetTypes[typeName] = setType;
   sourceTypes[setType] = typeName;

   return setType;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Registers the given file type with the type system.
//
// Remarks:
//
//    File types take the following pseudo-structure:
//       fileType {
//          $runtime_handle : HANDLE
//          $current_value : componentType
//       }
//
// Returns:
//
//   The Type object for the new file type.
//
//-----------------------------------------------------------------------------

Phx::Types::Type ^
TypeBuilder::RegisterFileType
(
   Phx::Types::Table ^ typeTable,
   Phx::Symbols::Table ^ symbolTable,
   String ^ typeName,
   Phx::Types::Type ^ componentType,
   int sourceLineNumber
)
{   
   Phx::Types::Type ^ handleType = 
      TypeBuilder::GetTargetType("HANDLE");
      
   // Lazy create the set type registration map.

   if (registeredFileTypes == nullptr)
   {
      registeredFileTypes  = gcnew List<Phx::Types::AggregateType ^>(); 
   }

   // Create a symbol for the new file type.

   Phx::Symbols::TypeSymbol ^ typeSymbol = 
      Phx::Symbols::TypeSymbol::New(
         symbolTable, 0,
         Phx::Name::New(typeTable->Lifetime, typeName)
      );

   // Calculate the overall size of the file aggregate structure.

   unsigned fileTypeBitSize = 
      handleType->BitSize + componentType->BitSize;

   // Create a new AggregateType object for the file type.

   Phx::Types::AggregateType ^ fileType = 
      Phx::Types::AggregateType::New(
         typeTable, fileTypeBitSize, typeSymbol
      );

   // Add the type to the type mapping.

   registeredFileTypes->Add(fileType);

   // Also register the file type as an aggregate type.

   RegisterAggregateType(fileType);

   // Set metaproperties for type.
   // In Pascal, structured types will always contain plain data.

   fileType->IsOnlyData = true;
   fileType->IsSealed = true;
   fileType->HasCompleteInheritanceInfo = true;
   fileType->HasCompleteOverrideInfo = true;

   // Add fields to type.

   AddFieldToType(typeTable->Lifetime, fileType, handleType,
      "$runtime_handle", -1, sourceLineNumber);

   AddFieldToType(typeTable->Lifetime, fileType, componentType,
      "$current_value", -1, sourceLineNumber);

   // We've filled in all members for the aggregate.

   fileType->HasCompleteMemberInfo = true;     
   
   // Add to built-in type mappings.

   targetTypes[typeName] = fileType;
   sourceTypes[fileType] = typeName;

   return fileType;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the component type of the given file type.
//
// Remarks:
//
//
// Returns:
//
//
//-----------------------------------------------------------------------------

Phx::Types::Type ^
TypeBuilder::GetFileComponentType
(
   Phx::Types::Type ^ fileType
)
{
   // Return the type of the .$current_value component of the file structure.

   return FindFieldSymbol(
      fileType->AsAggregateType,
      "$current_value"
   )->Type;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Registers the given pointer type with the type system.
//
// Remarks:
//
//
// Returns:
//
//   The Type object for the new pointer type.
//
//-----------------------------------------------------------------------------

Phx::Types::PointerType ^
TypeBuilder::RegisterPointerType
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ typeName,
   Phx::Types::Type ^ domainType,
   int sourceLineNumber
)
{
   if (registeredPointerTypes)
   {
      // Check for type redefinition.

      for each (Phx::Types::PointerType ^ pointerType in 
         registeredPointerTypes)
      {
         if (pointerType->TypeSymbol->NameString->Equals(typeName))
         {
            Output::ReportError(
               sourceLineNumber,
               Error::SymbolRedeclaration,
               typeName, 
               GetSourceType(pointerType)
            );
            return nullptr;
         }
      }
   }

   // Lazy create the pointer type registration map.

   if (registeredPointerTypes == nullptr)
   {
      registeredPointerTypes  = gcnew List<Phx::Types::PointerType ^>(); 
   }

   // Create a new PointerType object for the type.

   Phx::Symbols::TypeSymbol ^ typeSymbol = 
      Phx::Symbols::TypeSymbol::New(
         functionUnit->SymbolTable, 0,
         Phx::Name::New(functionUnit->Lifetime, typeName));

   Phx::Types::PointerType ^ pointerType = 
      Phx::Types::PointerType::New(
         functionUnit->TypeTable,
         Phx::Types::PointerTypeKind::UnmanagedPointer, 
         functionUnit->TypeTable->NativePointerBitSize,
         domainType, typeSymbol);

   // Add the type to the type mapping.

   registeredPointerTypes->Add(pointerType);
   
   // Add to built-in type mappings.

   targetTypes[typeName] = pointerType;
   sourceTypes[pointerType] = typeName;

   return pointerType;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Unregisters the given pointer type from the type system.
//
// Remarks:
//
//    This method is used to resolve forward declarations to 
//    undefined pointer types. For example:
//
//       link = ^person;
//    	person = record
//          next : link;
//       end;
//
//    In this example, we create a temporary type for 'link' until
//    the person type is defined. 
//
// Returns:
//
//
//-----------------------------------------------------------------------------

bool
TypeBuilder::UnregisterPointerType
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Types::PointerType ^ pointerType
)
{
   // Remove the pointer type from the type mapping.

   if (registeredPointerTypes && 
       registeredPointerTypes->Contains(pointerType))
   {
      registeredPointerTypes->Remove(pointerType);
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Replaces all references to the first pointer type with 
//    the second within a function unit.
//
// Remarks:
//
//    This method is used with the UnregisterPointerType method. After we
//    remove temporary pointer types we then replace references to the 
//    temporary type with the resolved type.
//
// Returns:
//
//   The number of replaced type instances.
//
//-----------------------------------------------------------------------------

int
TypeBuilder::ReplaceAggregatePointerReferences
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Types::PointerType ^ oldPointerType,
   Phx::Types::PointerType ^ newPointerType
)
{
   // Iterate through all registered aggregate types.

   int replaceCount = 0;
   for each (Phx::Types::AggregateType ^ aggregateType in 
      registeredAggregateTypes->Keys)
   {
      // Get the symbol table assocated with the aggregate type.

      Phx::Symbols::Table ^ symbolTable = 
         registeredAggregateTypes[aggregateType];

      // Walk the field symbol list assocated with the aggregate type
      // and replace references to the old type with the new one.

      Phx::Symbols::FieldSymbol ^ fieldSymbol = 
         aggregateType->FieldSymbolList; 
      while (fieldSymbol != nullptr)
      {
         if (fieldSymbol->Type->Equals(oldPointerType))
         {
            fieldSymbol->Type = newPointerType;
            ++replaceCount;
         }
         fieldSymbol = fieldSymbol->NextFieldSymbol;
      }      
   }

   return replaceCount;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the pointer type with the given type name.
//
// Remarks:
//
//
// Returns:
//
//   The pointer type for the given type name.
//
//-----------------------------------------------------------------------------

Phx::Types::PointerType ^
TypeBuilder::GetPointerType
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ typeName
)
{
   if (registeredPointerTypes)
   {
      // Look for a registered pointer type with the provided
      // type name.

      for each (Phx::Types::PointerType ^ pointerType in 
         registeredPointerTypes)
      {
         if (pointerType->TypeSymbol->NameString->Equals(typeName))
         {
            return pointerType;
         }
      }
   }
   return nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the set type for the given base type.
//
// Remarks:
//
//    For example, for the 'integer' base type, we return the 
//    'set of integer' set type.
//
// Returns:
//
//   The Type object for the given set type.
//
//-----------------------------------------------------------------------------

Phx::Types::Type ^
TypeBuilder::GetSetType
(
   Phx::Types::Type ^ baseType
)
{
   // Return the 'emtpy set' type if the base type is invalid.

   if (baseType == nullptr)
   {
      return GetTargetType(NativeType::Epsilon);
   }

   // Search for an existing registered set type.

   if (registeredSetTypes)
   {
      for each (Phx::Types::Type ^ type in registeredSetTypes)
      {
         Phx::Types::PointerType ^ pointerType = type->AsPointerType;
         if (pointerType->ReferentType->Equals(baseType))
         {
            return pointerType;
         }
      }
   }

   // This may be a temporary type used as part of an expression; create
   // the new type.

   RegisterSetType(
      ModuleBuilder::CurrentFunctionUnit,
      GetUniqueTypeName(),
      baseType
   );

   return GetSetType(baseType);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Initializes the type map for all built-in Pascal types.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void 
TypeBuilder::InitializeTypeMaps()
{
   targetTypes = gcnew Dictionary<String ^, Phx::Types::Type ^>();
   sourceTypes = gcnew Dictionary<Phx::Types::Type ^, String ^>();

   Phx::Types::Table ^ typeTable = Phx::GlobalData::TypeTable;
   Phx::Symbols::Table ^ symbolTable = Phx::GlobalData::SymbolTable;
  
   // Populate target type mappings.

   // The built-in 'integer', 'char', and 'real' types are 
   // just type definitions of built-in Phoenix types.

   targetTypes["integer"] = typeTable->Int32Type->NewRenamed(
      Phx::Symbols::TypeSymbol::New(symbolTable, 0, 
         Phx::Name::New(typeTable->Lifetime, "integer")));

   targetTypes["char"] = typeTable->Int8Type->NewRenamed(
      Phx::Symbols::TypeSymbol::New(symbolTable, 0,
         Phx::Name::New(typeTable->Lifetime, "char")));

   targetTypes["real"] = typeTable->Float64Type->NewRenamed(
      Phx::Symbols::TypeSymbol::New(symbolTable, 0,
         Phx::Name::New(typeTable->Lifetime, "real")));

   // Create the HANDLE type for use with Win32 native files.
   // The HANDLE type is defined as 'pointer to void'.

   Phx::Types::Type ^ handleType = 
      typeTable->GetUnmanagedPointerType(typeTable->VoidType)->
         NewRenamed(Phx::Symbols::TypeSymbol::New(
            symbolTable, 0,
            Phx::Name::New(typeTable->Lifetime, "HANDLE")));

   targetTypes["handle"] = handleType;

   // Create the set type.

   Phx::Types::Type ^ setType = typeTable->GetUnmanagedPointerType(
      typeTable->Int32Type
   );
   targetTypes["set"] = handleType->NewRenamed(
      Phx::Symbols::TypeSymbol::New(
         symbolTable,
         0,
         Phx::Name::New(typeTable->Lifetime, "set")
      )
   );
  
   // Create the 'epsilon' (empty set) type.

   targetTypes["epsilon"] = typeTable->NullPointerType->NewRenamed(
      Phx::Symbols::TypeSymbol::New(
         symbolTable,
         0,
         Phx::Name::New(typeTable->Lifetime, "epsilon")
      )
   );

   // Create the 'file of char' ('text') type.

   targetTypes["text"] = RegisterFileType(
      typeTable,
      symbolTable,
      "text",
      targetTypes["char"],
      0
    );  

   // Create the 'pointer to nothing' ('nil') type.

   targetTypes["nil"] = typeTable->NullPointerType->NewRenamed(
      Phx::Symbols::TypeSymbol::New(
         symbolTable,
         0,
         Phx::Name::New(typeTable->Lifetime, "nil")
      )
   );

   // Create a default 'unknown' type mapping.

   targetTypes["(unknown)"] = typeTable->UnknownType;
   
   // Populate source type mappings.
   // This is just a reverse mapping of the target type map.

   for each (String ^ typeName in targetTypes->Keys)
   {
      if (!sourceTypes->ContainsKey(targetTypes[typeName]))
         sourceTypes->Add(targetTypes[typeName], typeName);
   }

   // Initialize range mappings for built-in ordinal types (Boolean,
   // char, integer). Also initialize range mappings for the Phoenix
   // Int32 and Int8 types (these types are used internally to build
   // array index operands and the like).
   
   typeRanges->Add(typeTable->BooleanType, 
      GetRangePoolIndex(gcnew ValueRange(0, 1)));
   
   array<Phx::Types::PrimitiveType ^> ^ ordinalTypes = {
      typeTable->Int8Type,
      typeTable->Int32Type,
      targetTypes["char"]->AsPrimitiveType,      
      targetTypes["integer"]->AsPrimitiveType
   };
   for each (Phx::Types::PrimitiveType ^ ordinalType in ordinalTypes)
   {
      RegisterTypeRange(
         ordinalType,
         ValueRange::FromBitSize(ordinalType->BitSize),
         0
      );     
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the pool index of the given value range.
//
// Remarks:
//
//
// Returns:
//
//   The pool index of the given ValueRange object.
//
//-----------------------------------------------------------------------------

int
TypeBuilder::GetRangePoolIndex
(
   ValueRange ^ range
)
{
   for (int i = 0; i < rangePool->Count; ++i)
   {
      ValueRange ^ item = rangePool[i];
      if (item->Lower == range->Lower &&
          item->Upper == range->Upper)
      {
         return i;
      }
   }

   // Item not found. Add a new range to the list and return 
   // its index.

   rangePool->Add(range);
   return rangePool->Count - 1;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the next available bit offset in the given
//    aggregate type.
//
// Remarks:
//
//    This method is used to build field offsets within an aggregate
//    type. Because aggregate types are initially constructed with 
//    a constant size, we can calculate the 'next' available offset by
//    calculating the current size of the type.
//
// Returns:
//
//   The next available bit offset into the given aggregate type.
//
//-----------------------------------------------------------------------------

unsigned int
TypeBuilder::GetNextBitOffset
(
   Phx::Types::AggregateType ^ type
)
{
   unsigned int bitOffset = 0;

   Phx::Symbols::FieldSymbol ^ fieldSymbol = type->FieldSymbolList;
   while (fieldSymbol)
   {
      // Increment the offset by using the natural alignment of the
      // field symbol's type.

      Phx::Alignment ^ align = 
         Phx::Alignment::NaturalAlignment(fieldSymbol->Type);
      bitOffset += align->BitSize;

      fieldSymbol = fieldSymbol->NextFieldSymbol;
   }

   // The offset must be less than the overall size of the type.

   Debug::Assert(bitOffset < type->BitSize);

   return bitOffset;
}

}  // namespace Pascal
