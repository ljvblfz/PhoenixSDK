//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the ModuleBuilder class.
//
//-----------------------------------------------------------------------------

#pragma once

using namespace System;
using namespace System::Diagnostics;
using namespace System::Collections::Generic;

namespace Pascal
{

// Enumeration containing supported directives
// for Pascal procedures and functions.

enum class DirectiveType
{
   None,
   Forward,
   External,
};

// Forward declaration for internal-use class.

ref class FunctionUnitData;

//-----------------------------------------------------------------------------
//
// Description: Helper class for building module (ModuleUnit) and function
//              (FunctionUnit) related information.
//
// Remarks:
//              For convenience, this class is implemented as a static
//              utilty class. Because it does maintain state information
//              about each compilation unit, we provide the NewContext method 
//              to reset state information between compilation units.
//
//-----------------------------------------------------------------------------

ref class ModuleBuilder
{
public:

   // Resets state information between compilation units.

   static void NewContext();

   // Creates a new ModuleUnit object for the current compiland.
   
   static Phx::ModuleUnit ^ CreateModuleUnit
   (
      String ^ sourceFileName
   );

   // Adds a parameter to the list of program parameters for 
   // the entire program.

   static void
   AddProgramParameter
   (
      String ^ parameterName,
      int sourceLineNumber
   );

   // Determines whether the provided name has been declared
   // as a program parameter.

   static bool
   IsProgramParameter
   (
      String ^ parameterName     
   );

   // Adds the given file name to the overall list of 
   // processed source files.

   static void AddSourceFilePath
   (
      String ^ sourceFilePath
   );
  
   // Adds a new global variable symbol to the symbol table
   // associated with the given ModuleUnit object.

   static Phx::Symbols::GlobalVariableSymbol ^ 
   AddGlobalVariable
   (
      Phx::ModuleUnit ^ module,
      String ^ name,
      Phx::Types::Type ^ type,
      unsigned char * data,
      unsigned byteSize
   );

   // Retrieves a global variable symbol for the given
   // global string constant.

   static Phx::Symbols::GlobalVariableSymbol ^
   GetStringSymbol
   (  
      Phx::ModuleUnit ^ moduleUnit,
      String ^ value
   );

   // Retrieves a non-local variable symbol for the given
   // global string constant.

   static Phx::Symbols::NonLocalVariableSymbol ^
   GetNonLocalStringSymbol
   (      
      Phx::FunctionUnit ^ functionUnit,
      String ^ value
   );

   // Retrieves a non-local variable symbol for the given
   // global variable.

   static Phx::Symbols::NonLocalVariableSymbol ^
   MakeProxyInFunctionSymbolTable
   ( 
      Phx::Symbols::GlobalVariableSymbol ^ global,
      Phx::Symbols::Table ^ functionSymbolTable
   );

   // Retrieves the formal parameters associated with the given
   // function name.

   static array<FormalParameter ^> ^
   GetFormalParameters
   (
      String ^ functionName
   );

   // Retrieves the directive type associated with the given
   // function unit.

   static DirectiveType 
   GetDirectiveType
   (
      Phx::FunctionUnit ^ functionUnit
   );

   // Sets the directive type associated with the given
   // function unit.

   static void
   SetDirectiveType
   (
      Phx::FunctionUnit ^ functionUnit,
      DirectiveType directive
   );   

   // Appends a label instruction to the given function unit.

   static void
   BuildLabel
   (
      Phx::FunctionUnit ^ functionUnit, 
      int label,
      int sourceLineNumber
   );

   // Retrieves the label instruction associated with the
   // given label identifier in the provided function unit.

   static Phx::IR::LabelInstruction ^ 
   GetLabelInstruction
   (
      Phx::FunctionUnit ^ functionUnit, 
      int label,
      int sourceLineNumber
   );

   // Determines whether the given label has been declared
   // within the given function unit.

   static bool 
   IsLabelDeclared
   (
      Phx::FunctionUnit ^ functionUnit, 
      int label
   );

   // Adds a reference to the given label in the provided
   // function unit.

   static void
   AddLabelReference
   (
      Phx::FunctionUnit ^ functionUnit, 
      int label
   );

   // Adds an entry to the goto fixup list for the given
   // function unit.

   static void
   AddGotoLabelFixup
   (
      Phx::FunctionUnit ^ functionUnit, 
      int label, 
      Phx::IR::BranchInstruction ^ gotoInstruction,
      int sourceLineNumber
   );

   // Adds the given label to the declaration list associated
   // with the given function unit.

   static bool
   AddLabelDeclaration
   (
      Phx::FunctionUnit ^ functionUnit, 
      int label
   );

   // Retrieves the FunctionUnit object associated with the given 
   // function name.

   static Phx::FunctionUnit ^ 
   FindFunctionUnit
   (
      String ^ functionUnitName
   );

   // Builds a function symbol from the given parameters.

   static Phx::Symbols::FunctionSymbol ^
   BuildFunctionSymbol
   (
      Phx::Lifetime ^ lifetime,
      Phx::Types::Table ^ typeTable,
      Phx::Symbols::Table ^ symbolTable,
      String ^ signature,
      Phx::Types::CallingConventionKind callingConvention,
      Phx::Symbols::Visibility visibility,
      array<Phx::Types::Type ^> ^ argumentList,
      bool isEllipsis,
      Phx::Types::Type ^ returnValue
   );

   // Builds a FunctionUnit and its beginning part.

   static Phx::FunctionUnit ^ 
   BuildBeginFunction
   (
      Phx::ModuleUnit ^ moduleUnit,
      String ^ name,
      Phx::Types::CallingConventionKind callingConvKind,
      Phx::Symbols::Visibility visibility,
      array<FormalParameter ^> ^ formalParameters,
      Phx::Types::Type ^ returnType,
      DirectiveType directive,
      int sourceLineNumber
   );

   // Builds the end part of a given FunctionUnit.

   static void 
   BuildEndFunction
   (
      Phx::FunctionUnit ^ functionUnit,
      int sourceLineNumber
   );   
        
   // Activates the given FunctionUnit object for editing.

   static List<Phx::IR::Instruction ^> ^
   BeginFunctionEdit
   (
      Phx::FunctionUnit ^ functionUnit,
      int sourceLineNumber
   );

   // Completes edit of the current FunctionUnit object.

   static void EndFunctionEdit
   (
      List<Phx::IR::Instruction ^> ^ unlinkedInstructions,
      int sourceLineNumber
   );
      
   // Pushes the given symbol table onto the symbol stack.

   static void 
   PushScope
   (
      Phx::Symbols::Table ^ symbolTable
   );

   // Pops the current symbol table from the symbol stack.

   static void 
   PopScope();

   // Performs a narrow to wide scope search for a symbol with the given name.

   static Phx::Symbols::Symbol ^ 
   LookupSymbol
   (      
      Phx::FunctionUnit ^ functionUnit,
      String ^ symbolName
   );

   // Performs a narrow to wide scope search for the type symbol with the 
   // given name.

   static Phx::Symbols::TypeSymbol ^ 
   LookupTypeSymbol
   (
      Phx::FunctionUnit ^ functionUnit, 
      String ^ typeName
   );

   // Creates a new, non-user variable symbol with the given type and
   // storage class.

   static Phx::Symbols::Symbol ^
   AddInternalVariableDeclarationSymbol
   (
      Phx::FunctionUnit ^ functionUnit, 
      Phx::Types::Type ^ type, 
      Phx::Symbols::StorageClass storageClass,
      int sourceLineNumber
   );

   // Creates a new user variable symbol with the given type and
   // storage class.

   static Phx::Symbols::Symbol ^ 
   AddVariableDeclarationSymbol
   (
      Phx::FunctionUnit ^ functionUnit,
      String ^ name,
      Phx::Types::Type ^ type,
      Phx::Symbols::StorageClass storageClass,
      int sourceLineNumber
   );

   // Marks the given symbol as a local constant by mapping the provided symbol
   // to the provided ConstantValue object.

   static void
   AddLocalConstant
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Symbols::Symbol ^ sym,
      ConstantValue ^ value
   );

   // Determines whether the given symbol is mapped to a constant value.

   static bool
   IsLocalConstant
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Symbols::Symbol ^ sym
   );

   // Retrieves the ConstantValue object mapped to the provided symbol.

   static ConstantValue ^
   GetLocalConstantValue
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Symbols::Symbol ^ sym
   );
 
   // Generates a unique identifier name or internal use.

   static String ^
   GenerateUniqueSymbolName();

   // Retrieves an operand that contains the runtime address of 
   // the given symbol.

   static Phx::IR::MemoryOperand ^
   GetNonLocalSymbolAddress
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Symbols::Symbol ^ symbol,
      int sourceLineNumber
   );
  
   // Allocates the backing store for Pascal file objects.

   static void
   AllocateFile
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Symbols::Symbol ^ fileSymbol,
      Phx::Types::Type ^ staticType,
      int sourceLineNumber
   );

   // Allocates the backing store for Pascal set objects.

   static void
   AllocateSet
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Symbols::Symbol ^ setSymbol,
      Phx::Types::Type ^ baseType,
      int sourceLineNumber
   );

   // Allocates the backing store for an array of Pascal set objects.

   static void
   AllocateSetArray
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Symbols::Symbol ^ setArraySymbol,
      int sourceLineNumber
   );

   // Add the provided set symbol to the list of set symbols mapped
   // to the given function unit.

   static void
   AddSetDefinitionSymbol
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Symbols::Symbol ^ symbol,
      Phx::Types::Type ^ staticType
   );

   // Determines whether the provided set symbol is mapped in the list 
   // of set symbols of the given function unit.
    
   static bool
   IsSetDefinitionSymbol
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Symbols::Symbol ^ symbol
   );
  
   // Marks the given set symbol for memory release after the given
   // function exists.

   static void
   AddSetSymbolToReleaseList
   (
      Phx::FunctionUnit ^ functionUnit,
      Phx::Symbols::Symbol ^ varSymbol
   );
  
   // Determines whether the given symbol represents a pass-by-reference 
   // procedure parameter.

   static bool
   IsVariableParameterSymbol
   (
      Phx::Symbols::Symbol ^ symbol
   );

   // Determines whether the given function is callable from the current
   // location.

   static bool
   IsCallable
   (
      Phx::Symbols::FunctionSymbol ^ functionSymbol
   );   

public: // properties

   // Retrieves the current FunctionUnit object.

   static property Phx::FunctionUnit ^ CurrentFunctionUnit
   {
      Phx::FunctionUnit ^ get();
   }

   // Sets or retrieves a global Resolve object for performing
   // run time conversions.

   static property Phx::Types::Resolve ^ Resolve;

   // Retrieves the current (most recently added) source file name.

   static property String ^ SourceFileName
   {
      String ^ get();
   }

   // Retrieves the index of the current source file name.

   static property int SourceFileIndex
   {
      int get();
   }

   // Maps the external identifier of string constants to their
   // corresponding string lengths.

   static property Dictionary<unsigned int, int> ^ StringSymbolLengths;

   // Retrieves the current symbol table from the symbol stack.

   static property Phx::Symbols::Table ^ TopScope
   {
      Phx::Symbols::Table ^ get();
   }

   // The current RuntimeLibrary object for making calls to 
   // the runtime.

   static property IRuntimeLibrary ^ Runtime;

   // The function unit for the compiler-generated main() function.

   static property Phx::FunctionUnit ^ MainFunctionUnit;

   // Global counter for creating FunctionUnit objects.

   static property int FunctionCounter;

   // Retrieves the list of all FunctionUnit objects for the
   // entire module.

   static property IEnumerable<Phx::FunctionUnit ^> ^ FunctionUnits
   {
      IEnumerable<Phx::FunctionUnit ^> ^ get();
   }

private: // methods

   // Retrieves the FunctionUnitData node associated with the given function
   // unit.

   static FunctionUnitData ^ 
   FindFunctionUnitData
   (
      Phx::FunctionUnit ^ functionUnit     
   );

   // Retrieves the FunctionUnitData node associated with the given function
   // name.

   static FunctionUnitData ^ 
   FindFunctionUnitData
   (
      String ^ functionName
   );

   // Retrieves the FunctionUnitData node associated with the given function
   // unit.

   static FunctionUnitData ^ 
   FindFunctionUnitDataAux
   (
      Phx::FunctionUnit ^ unit, 
      FunctionUnitData ^ node
   );

   // Retrieves the FunctionUnitData node associated with the given function
   // name.

   static FunctionUnitData ^ 
   FindFunctionUnitDataAux
   (
      String ^ functionName,
      FunctionUnitData ^ node
   );

   // Retrieves the list of all FunctionUnit objects for the
   // entire module.

   static List<Phx::FunctionUnit ^> ^ 
   GetFunctionUnitList();

   // Recursively populates the list of all FunctionUnit objects for the
   // entire module.

   static void 
   PopulateFunctionUnitList
   (
      List<Phx::FunctionUnit ^> ^ list, 
      FunctionUnitData ^ node
   );
   
   // Retrieves the FunctionUnitData object associated with the current
   // function unit.

   static property FunctionUnitData ^ 
   CurrentFunctionUnitData
   {
      FunctionUnitData ^ get();
   }

   // Performs label post-processing for completed function units.

   static void PostProcessLabels
   (
      Phx::FunctionUnit ^ functionUnit,
      int sourceLineNumber
   );

   // Pushes an activation context for the provided function unit.
   
   static void
   BuildEnterDisplay
   (
      Phx::FunctionUnit ^ functionUnit,
      int sourceLineNumber
   );

   // Pops the current activation context for the provided function unit.

   static void
   BuildLeaveDisplay
   (
      Phx::FunctionUnit ^ functionUnit,
      int sourceLineNumber
   );

   // Initializes the global built-in constants for the given module unit.
   
   static void
   InitializeGlobalConstants
   (
      Phx::ModuleUnit ^ moduleUnit
   );

   // Creates a global symbol with the given initial string value.

   static Phx::Symbols::GlobalVariableSymbol ^
   CreateInitializedString
   (   
      Phx::ModuleUnit ^ moduleUnit,
      String ^ value
   );

   // Creates a section for storing global strings.

   static void 
   CreateStringSection
   (
      Phx::ModuleUnit ^ moduleUnit
   );

   // Performs a narrow to wide scope search for a symbol with the given name.

   static Phx::Symbols::Symbol ^ 
   LookupSymbolAux
   (
      String ^ name, 
      FunctionUnitData ^ node
   );

   // Performs a narrow to wide scope search for the type symbol with the 
   // given name.

   static Phx::Symbols::TypeSymbol ^ 
   LookupTypeSymbolAux
   (
      String ^ name, 
      FunctionUnitData ^ node
   );
   
   // Retrieves the formal parameter with the given name from the current
   // function unit.

   static FormalParameter ^
   GetFormalParameter
   (
      String ^ name
   );

private: // data

   // Character array used to tokenize symbol names.
   static array<wchar_t> ^ fieldSeparator = 
      gcnew array<wchar_t> { '.' };

   // The list of use-declared program parameters.
   static List<String ^> ^ programParameters = 
      gcnew List<String ^>();

   // The list of all source file names in the overall module.
   static List<String ^> ^ fileNames = 
      gcnew List<String ^>();

   // Caches the mapping between string values and global symbol objects.
   static Dictionary<String ^, 
      Phx::Symbols::GlobalVariableSymbol ^> ^ stringTable;

   // External identifier used when creating symbols for global variables.
   static unsigned externalIdCounter;   

   // SectionSymbol object for the global string section.
   static Phx::Symbols::SectionSymbol ^ stringSectionSymbol;

   // The head node of the function unit hierarchy.
   static FunctionUnitData ^ headFunctionUnitData;

   // Internal identifier used when creating unique symbol names.
   static int uniqueSymbolId;

   // Stack of symbol tables used to enforce scope rules.
   // Symbol tables are pushed onto this stack when function units are
   // activated and in cases such as entering Pascal 'with' blocks.
   static List<Phx::Symbols::Table ^> ^ scopeList;

   // The current hierarchy of function unit objects that we
   // are building.
   static List<Phx::FunctionUnit ^> ^ functionUnitStack;
};

} // namespace Pascal