//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the TypeBuilder class.
//
//-----------------------------------------------------------------------------

#pragma once

using namespace System;
using namespace System::Collections::Generic;

namespace Pascal 
{

enum class NativeType
{
   Boolean,
   Char,
   Integer,
   Real,
   File,
   Text,       // maps to 'file of char'
   NilPointer, // 'pointer to nothing' type
   Set,
   Epsilon,    // the empty set type
   Unknown,    // unknown type
};

//-----------------------------------------------------------------------------
//
// Description: Helper class for generating Pascal types.
//
// Remarks:
//              Pascal supports types such as set, file, and subrange. 
//              These types are implemented in Phoenix by using aggregate types,
//              primitive types, and so on. This class tracks generated types
//              by category so we can easily identify types and 
//              so we can perform fast static type checking.
//
//-----------------------------------------------------------------------------

ref class TypeBuilder sealed
{
public:

   // Registers the given type with the provided value range.

   static void
   RegisterTypeRange
   (
      Phx::Types::Type ^ type,
      ValueRange ^ range,
      int sourceLineNumber
   );

   // Retrieves the value range associated with the provided type.

   static ValueRange ^
   GetValueRange
   (
      Phx::Types::Type ^ type
   );
   
   // Performs static (compile-time) range checking on the
   // provided operand.

   static bool
   StaticCheckRange
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Types::Type ^ type,
      Phx::IR::ImmediateOperand ^ valueOperand,
      int sourceLineNumber
   );

   // Performs dynamic (run-time) range checking on the
   // provided operand.

   static void
   DynamicCheckRange
   (
      Phx::FunctionUnit ^ functionUnit,      
      Phx::IR::Operand ^ valueOperand,
      ValueRange ^ range,
      int sourceLineNumber
   );

   // Performs dynamic (run-time) range checking on the
   // provided operand.

   static void
   DynamicCheckRange
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Types::Type ^ type,
      Phx::IR::Operand ^ valueOperand,
      int sourceLineNumber
   );

   // Determines whether the two provided Type objects are equivalent.

   static bool
   AreEquivalentTypes
   (
      Phx::Types::Type ^ first,
      Phx::Types::Type ^ second
   );

   // Retrieves the primitive ordinal type with the given size.

   static Phx::Types::PrimitiveType ^
   GetOrdinalType
   (
      int byteSize
   );

   // Retrieves the type that results when the provided 
   // arithmetic operator is applied to operands of the given types.

   static Phx::Types::Type ^ 
   GetBinaryArithmeticResult
   (
      Phx::Types::Type ^ first,
      Phx::Types::Type ^ second,
      unsigned op
   );
   
   // Determines whether the provided types are compatible with
   // the provided relational operator.

   static bool
   AreRelationalTypesCompatible
   (
      Phx::Types::Type ^ firstType,
      Phx::Types::Type ^ secondType,
      unsigned relationalOperator
   );

   // Determines whether the given type is an ordinal type.

   static bool
   IsOrdinalType
   (
      Phx::Types::Type ^ type
   );

   // Determines whether the given type is a real type.
   
   static bool
   IsRealType
   (
      Phx::Types::Type ^ type
   );

   // Determines whether the given type is a simple type.

   static bool
   IsSimpleType
   (
      Phx::Types::Type ^ type
   );

   // Determines whether the given type is a string type.

   static bool
   IsStringType
   (      
      Phx::Types::Type ^ type
   );

   // Retrieves the constant length of the given string type.

   static int
   GetStringLength
   (
      Phx::Symbols::Symbol ^ symbol,
      Phx::Types::Type ^ stringType
   );
  
   // Determines whether the given type is a set type.

   static bool
   IsSetType
   (
      Phx::Types::Type ^ type
   );

   // Determines whether the given type is a file type.

   static bool
   IsFileType
   (
      Phx::Types::Type ^ type
   );

   // Determines whether the given type is a pointer type.

   static bool
   IsPointerType
   (
      Phx::Types::Type ^ type
   );
   
   // Determines whether the given type is a subrange type.

   static bool
   IsSubrangeType
   (
      Phx::Types::Type ^ type
   );

   // Determines whether the given type is a valid base type for
   // a set type.

   static bool 
   IsValidSetBaseType
   (
      Phx::Types::Type ^ type
   );

   // Retrieves the Type object associated with the given Pascal
   // program type name.

   static Phx::Types::Type ^ 
   GetTargetType
   (
      String ^ sourceType
   );
   
   // Retrieves the Type object associated with the given Pascal
   // program type name.

   static Phx::Types::Type ^ 
   TryGetTargetType
   (
      String ^ sourceType, 
      bool % succeeded
   );

   // Retrieves the Type object associated with the given Pascal
   // NativeType value.

   static Phx::Types::Type ^ 
   GetTargetType
   (
      NativeType sourceType
   );

   // Retrieves the Pascal program type name associated with the given
   // Type object.

   static String ^ 
   GetSourceType
   (
      Phx::Types::Type ^ targetType
   );
   
   // Registers the given enum type and its symbols with the type system.

   static void
   RegisterEnumType
   (
      Phx::Types::EnumType ^ type, 
      List<Phx::Symbols::Symbol ^> ^ scalarSymbols
   );

   // Registers the given subrange type and its range with the type system.

   static void
   RegisterSubrangeType
   (
      Phx::Types::Type ^ type,
      ValueRange ^ range,
      int sourceLineNumber
   );

   // Registers the given aggregate type with the type system.

   static Phx::Symbols::Table ^
   RegisterAggregateType
   (
      Phx::Types::AggregateType ^ type
   );

   // Adds a new field to the given aggregate type.

   static Phx::Types::Field ^
   AddFieldToType
   (
      Phx::Lifetime ^ lifetime,
      Phx::Types::AggregateType ^ type,
      Phx::Types::Type ^ fieldType,
      String ^ fieldName,
      int absoluteBitOffset,
      int sourceLineNumber
   );

   // Retrieves the field symbol with the given name from
   // the given aggregate type.

   static Phx::Symbols::FieldSymbol ^ 
   FindFieldSymbol
   (
      Phx::Types::AggregateType ^ aggregateType,
      String ^ fieldName
   );   

   // Changes the type of the field with the given name.

   static Phx::Symbols::FieldSymbol ^ 
   ChangeFieldType
   (
      Phx::Types::AggregateType ^ aggregateType,
      String ^ fieldName,
      Phx::Types::Type ^ newType
   );   
 
   // Retrieves the symbol table associated with the given 
   // aggregate type.

   static Phx::Symbols::Table ^
   GetSymbolTable
   (
      Phx::Types::AggregateType ^ type
   );
   
   // Retrieves the range of valid values for the given 
   // subrange type.

   static ValueRange ^
   GetSubrangeTypeRange
   (
      Phx::Types::Type ^ type
   );

   // Registers the given array type with the type system.

   static void
   RegisterArrayType
   (
      Phx::Types::Type ^ type,
      array<TypedValueRange ^> ^ indexRanges,
      int sourceLineNumber
   );

   // Retrieves the typed index ranges for each rank of the
   // given array type.

   static array<TypedValueRange ^> ^
   GetArrayIndexRanges
   (
      Phx::Types::UnmanagedArrayType ^ type
   );

   // Retrieves the ordinal value associated with the symbol
   // of the given enum type.

   static Int32
   GetEnumOrdinalValue
   (
      Phx::Types::EnumType ^ enumType, 
      Phx::Symbols::Symbol ^ scalarSymbol
   );

   // Retrieves the upper bound of the given enum type.

   static Int32
   GetEnumUpperBound
   (
      Phx::Types::EnumType ^ enumType
   );

   // Generates a unique type name.

   static String ^
   GetUniqueTypeName();

   // Registers the given set type with the type system.

   static Phx::Types::Type ^
   RegisterSetType
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ typeName,
      Phx::Types::Type ^ baseType
   );

   // Registers the given file type with the type system.

   static Phx::Types::Type ^
   RegisterFileType
   (
      Phx::Types::Table ^ typeTable,
      Phx::Symbols::Table ^ symbolTable,
      String ^ typeName,
      Phx::Types::Type ^ componentType,
      int sourceLineNumber
   );

   // Retrieves the component type of the given file type.
   
   static Phx::Types::Type ^
   GetFileComponentType
   (
      Phx::Types::Type ^ fileType
   );

   // Registers the given pointer type with the type system.

   static Phx::Types::PointerType ^
   RegisterPointerType
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ typeName,
      Phx::Types::Type ^ domainType,
      int sourceLineNumber
   );

   // Unregisters the given pointer type from the type system.

   static bool
   UnregisterPointerType
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Types::PointerType ^ pointerType
   );

   // Replaces all references to the first pointer type with 
   // the second within a function unit.

   static int
   ReplaceAggregatePointerReferences
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Types::PointerType ^ oldPointerType,
      Phx::Types::PointerType ^ newPointerType
   );
   
   // Retrieves the pointer type with the given type name.

   static Phx::Types::PointerType ^
   GetPointerType
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ typeName
   );
       
   // Retrieves the set type for the given base type.

   static Phx::Types::Type ^
   GetSetType
   (
      Phx::Types::Type ^ baseType
   );
       
private: // types

   ref struct EnumType
   {
      property Phx::Types::EnumType ^ Type;
      property List<Phx::Symbols::Symbol ^> ^ ScalarSymbols;
   };

   ref struct ArrayIndexType
   {
      property Phx::Types::Type ^ Type;
      property int PoolIndex;
   };

private: // methods

   // Initializes the type map for all built-in Pascal types.

   static void 
   InitializeTypeMaps();

   // Retrieves the pool index of the given value range.

   static int 
   GetRangePoolIndex
   (
      ValueRange ^ range
   );

   // Retrieves the next available bit offset in the given
   // aggregate type.

   static unsigned int
   GetNextBitOffset
   (
      Phx::Types::AggregateType ^ type
   );

private: // data

   // Bidirectional mapping from built-in Pascal types to 
   // their corresponding Phoenix Type objects.
   static Dictionary<Phx::Types::Type ^, String ^> ^ sourceTypes;
   static Dictionary<String ^, Phx::Types::Type ^> ^ targetTypes;

   // The list of registered enum types.
   static List<EnumType ^> ^ registeredEnumTypes;

   // The list of registered subrange types.
   static List<Phx::Types::Type ^> ^ registeredSubrangeTypes;

   // The list of registered array types.
   static List<Phx::Types::Type ^> ^ registeredArrayTypes;

   // The list of registered set types.
   static List<Phx::Types::Type ^> ^ registeredSetTypes;   

   // The list of registered file types.
   static List<Phx::Types::AggregateType ^> ^ registeredFileTypes;

   // The list of registered pointer types.
   static List<Phx::Types::PointerType ^> ^ registeredPointerTypes;

   // The mapping of registered aggregate types to symbol tables created
   // for each aggregate type.

   static Dictionary<Phx::Types::AggregateType ^, Phx::Symbols::Table ^> ^
      registeredAggregateTypes = gcnew
         Dictionary<Phx::Types::AggregateType ^, Phx::Symbols::Table ^>();

   // The mapping of types to their value ranges. To help conserve memory
   // consumption, we maintain distinct ValueRange objects in a separate
   // list and use the list index to identify the mapping.

   static Dictionary<Phx::Types::Type ^, int> ^ typeRanges = 
      gcnew Dictionary<Phx::Types::Type ^, int>();
   
   static List<ValueRange ^> ^ rangePool = gcnew List<ValueRange ^>();

   // Mapping of array types to their corresponding array indexing information.

   static Dictionary<Phx::Types::Type ^, array<ArrayIndexType ^> ^> ^
      arrayTypeIndexRanges = 
         gcnew Dictionary<Phx::Types::Type ^, array<ArrayIndexType ^> ^>();

   // The next identifier for unique type names.
   static int uniqueTypeNameId = 100;
};

}  // namespace Pascal