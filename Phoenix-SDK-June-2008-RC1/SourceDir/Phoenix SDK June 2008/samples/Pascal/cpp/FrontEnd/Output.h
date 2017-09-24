//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the Output class.
//
//-----------------------------------------------------------------------------

#pragma once

using namespace System;
using namespace System::IO;
using namespace System::Diagnostics;
using namespace System::Collections::Generic;

namespace Pascal
{

//-----------------------------------------------------------------------------
//
// Description:
//
//    Defines all errors and warnings for the compiler.
//
// Remarks: 
//
//    
//-----------------------------------------------------------------------------

enum class Error
{
   // 1000-level errors refer to errors that occur 
   // at the application level.

   EmptyFileList                       = 1000,
   InvalidFile                         = 1001,
   ProcessNotFound                     = 1002,
   InternalCompilerError               = 1003,
   UnknownInternalCompilerError        = 1004,
   FatalError                          = 1005,  // reserved for internal use

   // 2000-level errors refer to lex/parse errors.

   ParseSyntaxError                    = 2002,
   InvalidEscapeSequence               = 2003,

   // 3000-level errors refer to compilation errors.

   UnknownIndexType                    = 3000,
   ZeroIndexRange                      = 3001,
   NegativeIndexRange                  = 3002,

   BadVariableAssignment               = 3003,
   ConstantAssignment                  = 3004,
   FileAssignment                      = 3005,
   FunctionAssignment                  = 3006,

   IncompatibleCaseIndexType           = 3007,
   IncompatibleTypes                   = 3008,
   IncompatibleOperandTypes            = 3009,

   CaseStatementRange                  = 3010,
   CaseStatementValue                  = 3011,

   UnknownType                         = 3012,
   GeneralTypeExpected                 = 3013,
   TypeRedefinition                    = 3014,
   RangeRedefinition                   = 3015,
   StaticRangeCheckFailure             = 3016,
   UnknownRange                        = 3017,

   InvalidPointerAccess                = 3018,
   InvalidFieldAccess                  = 3019,

   UndeclaredLabel                     = 3020,
   UndefinedLabel                      = 3021,
   LabelRedeclaration                  = 3022,
   LabelRedefinition                   = 3023,
   InvalidLabelLength                  = 3024,
   DeclaredUndefinedLabel              = 3025,
   UnreferencedLabel                   = 3026,

   ZeroSizeRecord                      = 3027,
   RecursiveRecordDefinition           = 3028,
   FieldRedeclaration                  = 3029,
   
   UnknownSetBaseType                  = 3030,
   InvalidSetType                      = 3031,
   IncompatibleSetTypes                = 3032,
   IncompatibleSetTypesForMembership   = 3033,

   MismatchedSubrangeLimit             = 3034,
   MismatchedMemberDesignators         = 3035,

   UndeclaredIdentifier                = 3036,

   AmbiguousFileAccess                 = 3037,

   IndexListCount                      = 3038,
   MismatchedIndexType                 = 3039,

   MismatchedArrayElementTypes         = 3040,
   MismatchedArraySizes                = 3041,

   InvalidWithField                    = 3042,

   ProgramParameterRedeclaration       = 3043,
   UndeclaredProgramParameter          = 3044,

   FormalParameterRedeclaration        = 3045,
   MustBeVarParameter                  = 3046,
   ConstantVarParameter                = 3047,

   InvalidArgument                     = 3048,   
   InvalidArgumentCount                = 3049,
   InvalidProcedureArgument            = 3050,

   SymbolRedeclaration                 = 3051,
   FunctionRedeclaration               = 3052,
   InvalidReturnType                   = 3053,
   FunctionUnreachable                 = 3054,
   UndeclaredFunction                  = 3055,

   UnknownDirective                    = 3056,

   InvalidExternalFunctionDefinition   = 3057,
   InvalidForwardFunctionDefinition    = 3058,

   UnknownTypeSymbol                   = 3059,

   InvalidWidthSpecifier               = 3060,

   FileArrayUnsupported                = 3061,
   InvalidSubrange                     = 3062,
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes error and warning diagnostic messages to an output stream.
//
// Remarks: 
//
//    
//-----------------------------------------------------------------------------

ref class Output sealed
{
public:

   // Writes the given message to the output stream.

   static void ReportMessage(String ^ message);

   static void ReportMessage(int lineNumber, String ^ message);
   
   // Writes the given warning message to the output stream.

   static void ReportWarning(int lineNumber, Error warning, Object ^ arg0);

   // Writes the given error message to the output stream.

   static void ReportError(int lineNumber, Error error);
   static void ReportError(int lineNumber, Error error, Object ^);
   static void ReportError(int lineNumber, Error error, Object ^, Object ^);
   static void ReportError(int lineNumber, Error error, Object ^, Object ^, 
      Object ^);

   // Writes the given fatal error message to the output stream.

   static void ReportFatalError(Error error);
   static void ReportFatalError(Error error, Object ^);
   static void ReportFatalError(int lineNumber, Error error);
   static void ReportFatalError(int lineNumber, Error error, Object ^);
   static void ReportFatalError(int lineNumber, Error error, Object ^, 
      Object ^);
   static void ReportFatalError(int lineNumber, Error error, Object ^, 
      Object ^, Object ^);

   // Writer object for messages.
   
   static property TextWriter ^ MessageWriter;

   // Writer object for errors and warnings.

   static property TextWriter ^ ErrorWriter;

   // The name of the file that is currently being compiled.

   static property String ^ CurrentSourceFileName;

   // Retrieves the default system message writer.

   static property TextWriter ^ DefaultWriter
   {
      TextWriter ^ get();
   }

   // The current error count.

   static property int ErrorCount;

   // The current warning count.

   static property int WarningCount;

private: // methods

   static Output()
   {
      MessageWriter = nullptr;
      
      ErrorCount = 0;
      WarningCount = 0;

      InitializeErrorMap();
   }

   // Internal method for writing strings to the output stream.

   static void InternalReportMessage
   (
      String ^ messageType, 
      int      lineNumber, 
      String ^ message,
      int      messageNumber
   );

   // Initializes the error map.

   static void InitializeErrorMap();

private: // data

   // Prefix strings for warnings and errors.

   literal String ^ warning = "warning";
   literal String ^ error = "error";
   literal String ^ fatalError = "fatal error";

   // Maps Error values to their string representations.

   static Dictionary<Error, String ^> ^ errorMap;
};

}  // namespace Pascal