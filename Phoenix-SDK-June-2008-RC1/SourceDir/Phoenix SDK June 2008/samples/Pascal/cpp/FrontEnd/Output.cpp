//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Implementation of the Output class.
//
//-----------------------------------------------------------------------------

using namespace System;
using namespace System::Diagnostics;

#include "stdafx.h"
#include "Ast.h"

namespace Pascal
{

TextWriter ^ 
Output::DefaultWriter::get()
{
   return System::Console::Out;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes the given message to the output stream.
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
Output::ReportMessage
(
   String ^ message
)
{
   ReportMessage(-1, message);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes the given message to the output stream.
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
Output::ReportMessage
(
   int lineNumber, 
   String ^ message
)
{
   // Format message string.

   if (lineNumber >= 0)
   {
      message = String::Format(
         "{0}({1}) : {2}",
         CurrentSourceFileName, 
         lineNumber, 
         message
      );
   }
   else
   {
      message = String::Format(
         "{0}",
         message);
   }

   // Write the message to the message writer if one was specified;
   // otherwise, write the message to the console.

   if (MessageWriter)
   {
      MessageWriter->WriteLine(message);
   }
   else
   {
      Console::ForegroundColor = System::ConsoleColor::Cyan;
      Console::WriteLine(message);
      Console::ResetColor();
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes the given warning message to the output stream.
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
Output::ReportWarning
(     
   int lineNumber,
   Error warning, 
   Object ^ arg0
)
{
   String ^ message = String::Format(errorMap[warning], arg0);
   InternalReportMessage(Output::warning, lineNumber, message, (int)warning);
   WarningCount++;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes the given error message to the output stream.
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
Output::ReportError
(
   int lineNumber, 
   Error error
)
{
   InternalReportMessage(Output::error, lineNumber, 
      errorMap[error], (int)error);
   ErrorCount++;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes the given error message to the output stream.
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
Output::ReportError
(
   int lineNumber, 
   Error error,
   Object ^ arg0
)
{
   String ^ message = String::Format(errorMap[error], arg0);
   InternalReportMessage(Output::error, lineNumber, message, (int)error);
   ErrorCount++;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes the given error message to the output stream.
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
Output::ReportError
(
   int lineNumber, 
   Error error,
   Object ^ arg0, 
   Object ^ arg1
)
{
   String ^ message = String::Format(errorMap[error], arg0, arg1);
   InternalReportMessage(Output::error, lineNumber, message, (int)error);
   ErrorCount++;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes the given error message to the output stream.
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
Output::ReportError
(
   int lineNumber, 
   Error error,
   Object ^ arg0, 
   Object ^ arg1, 
   Object ^ arg2
)
{
   String ^ message = String::Format(errorMap[error], arg0, arg1, arg2);
   InternalReportMessage(Output::error, lineNumber, message, (int)error);
   ErrorCount++;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes the given fatal error message to the output stream.
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
Output::ReportFatalError(Error error)
{
   ReportFatalError(-1, error);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes the given fatal error message to the output stream.
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
Output::ReportFatalError(Error error, Object ^ arg0)
{
   ReportFatalError(-1, error, arg0);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes the given fatal error message to the output stream.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void Output::ReportFatalError
(
   int lineNumber, 
   Error error
)
{
   String ^ message = errorMap[error];
   InternalReportMessage(Output::error, lineNumber, message, (int)error);
   ErrorCount++;

   InternalReportMessage(Output::fatalError, lineNumber, 
      "unable to recover from previous error(s); stopping compilation.",
      (int)Error::FatalError);

   // Throw fatal error message. This will be caught by the main() function.

   throw gcnew FatalErrorException(lineNumber, message);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes the given fatal error message to the output stream.
//
// Remarks:
//
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void Output::ReportFatalError
(
   int lineNumber, 
   Error error, 
   Object ^ arg0
)
{
   String ^ message = String::Format(errorMap[error], arg0);
   InternalReportMessage(Output::error, lineNumber, message, (int)error);
   ErrorCount++;

   InternalReportMessage(Output::fatalError, lineNumber, 
      "unable to recover from previous error(s); stopping compilation.", 
      (int)Error::FatalError);

   // Throw fatal error message. This will be caught by the main() function.

   throw gcnew FatalErrorException(lineNumber, message);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes the given fatal error message to the output stream.
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
Output::ReportFatalError
(
   int lineNumber,
   Error error,
   Object ^ arg0,
   Object ^ arg1
)
{
   String ^ message = String::Format(errorMap[error], arg0, arg1);
   InternalReportMessage(Output::error, lineNumber, message, (int)error);
   ErrorCount++;

   InternalReportMessage(Output::fatalError, lineNumber, 
      "unable to recover from previous error(s); stopping compilation.",
      (int)Error::FatalError);

   // Throw fatal error message. This will be caught by the main() function.

   throw gcnew FatalErrorException(lineNumber, message);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Writes the given fatal error message to the output stream.
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
Output::ReportFatalError
(
   int lineNumber,
   Error error,
   Object ^ arg0,
   Object ^ arg1,
   Object ^ arg2
)
{
   String ^ message = String::Format(errorMap[error], arg0, arg1, arg2);
   InternalReportMessage(Output::error, lineNumber, message, (int)error);
   ErrorCount++;

   InternalReportMessage(Output::fatalError, lineNumber, 
      "unable to recover from previous error(s); stopping compilation.",
      (int)Error::FatalError);

   // Throw fatal error message. This will be caught by the main() function.

   throw gcnew FatalErrorException(lineNumber, message);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Internal method for writing strings to the output stream.
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
Output::InternalReportMessage
(
   String ^ messageType, 
   int      lineNumber,
   String ^ message,
   int      messageNumber
)
{
   String ^ output;

   // Write source file / line information if a valid line number
   // was provided.

   if (lineNumber >= 0)
   {
      output = String::Format(
         "{0}({1}) ",
         CurrentSourceFileName, 
         lineNumber
      );

      if (MessageWriter)
         MessageWriter->Write(output);
      else
         Console::Write(output);
   }

   // Add in the message number if one was provided.

   if (messageNumber >= 0)
      output = String::Format("{0} P{1}", messageType, messageNumber);
   else
      output = messageType;

   // Write the output.

   if (MessageWriter)
      MessageWriter->Write(output);
   else
   {      
      if (messageType->Equals(error) || messageType->Equals(fatalError))
      {
         Console::ForegroundColor = System::ConsoleColor::Red;
      }
      Console::Write(output);
      Console::ResetColor();
   }

   output = String::Format(": {0}", message);

   if (MessageWriter)
      MessageWriter->WriteLine(output);
   else
   {      
      Console::WriteLine(output);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Initializes the error map.
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
Output::InitializeErrorMap()
{
   errorMap = gcnew Dictionary<Error, String ^>();

   errorMap[Error::EmptyFileList] = 
   "no file name(s) provided.";

   errorMap[Error::InvalidFile] = 
   "cannot open file {0}.";

   errorMap[Error::ProcessNotFound] = 
   "Failed to launch '{0}'{2}Reason: '{1}'{2}"
   "Ensure the application exists and is accessible from your system "
   "PATH environment variable.";

   errorMap[Error::InternalCompilerError] = 
   "{0}{2}Stack Trace:{2}{1}";

   errorMap[Error::UnknownInternalCompilerError] = 
   "Internal Compiler Error";

   errorMap[Error::InvalidEscapeSequence] = 
   "invalid escape sequence.";

   errorMap[Error::ParseSyntaxError] = 
   "syntax error: token '{0}'.";

   errorMap[Error::UnknownIndexType] = 
   "'{0}': unknown index type; expected scalar or subrange type.";

   errorMap[Error::FileArrayUnsupported] =
   "array of files is unsupported.";

   errorMap[Error::ZeroIndexRange] = 
   "index type range must be non-zero.";

   errorMap[Error::NegativeIndexRange] = 
   "index type range must be non-negative.";

   errorMap[Error::BadVariableAssignment] = 
   "variable '{0}' of type '{1}' is being assigned an "
   "expression of type '{2}'.";

   errorMap[Error::ConstantAssignment] = 
   "'{0}': you cannot assign to a variable that is constant.";

   errorMap[Error::FileAssignment] = 
   "'{0}': you cannot assign to a file variable.";

   errorMap[Error::FunctionAssignment] = 
   "'{0}': you cannot assign to a function or procedure.";

   errorMap[Error::GeneralTypeExpected] = 
   "expected {0} type."; 

   errorMap[Error::IncompatibleCaseIndexType] =
   "type '{0}' is not compatible with case index type '{1}'.";

   errorMap[Error::CaseStatementRange] =
   "'{0}..{1}': range already appears in case statement list.";
   
   errorMap[Error::CaseStatementValue] =
   "'{0}': value already appears in case statement list.";

   errorMap[Error::UnknownType] = 
   "type '{0}' not found.";

   errorMap[Error::TypeRedefinition] =
   "'{0}': type redefinition.";

   errorMap[Error::RangeRedefinition] =
   "redefinition of range for type '{0}'; ({1}) -> ({2}).";

   errorMap[Error::StaticRangeCheckFailure] =
   "value '{0}' exceeded legal bounds ({1}..{2}).";

   errorMap[Error::UnknownRange] = 
   "internal error: compiler could not find range mapping for '{0}'.";

   errorMap[Error::IncompatibleTypes] =
   "incompatible types.";

   errorMap[Error::IncompatibleOperandTypes] =
   "'{0} {1} {2}': operand types are not compatible.";

   errorMap[Error::InvalidPointerAccess] =
   "expected pointer access symbol '^' or '->'.";

   errorMap[Error::InvalidFieldAccess] =
   "'{0}' is not a member of '{1}'.";

   errorMap[Error::UndeclaredLabel] =
   "'{0}': undeclared label.";

   errorMap[Error::UndefinedLabel] =
   "'{0}': undefined label.";

   errorMap[Error::LabelRedeclaration] =
   "label '{0}' already declared.";

   errorMap[Error::LabelRedefinition] =
   "'{0}': label refinition.";

   errorMap[Error::InvalidLabelLength] =
   "'{0}': labels may contain at most 4 digits.";

   errorMap[Error::DeclaredUndefinedLabel] = 
   "label '{0}' was declared but never defined.";

   errorMap[Error::UnreferencedLabel] = 
   "label '{0}' was declared but never referenced.";

   errorMap[Error::ZeroSizeRecord] = 
   "zero-sized records are not allowed.";

   errorMap[Error::RecursiveRecordDefinition] = 
   "'{0}.{0}' : Recursive definitions of records are not allowed.";

   errorMap[Error::FieldRedeclaration] = 
   "'{0}' already declared as field with type '{1}'.";

   errorMap[Error::UnknownSetBaseType] =
   "unknown set base type.";

   errorMap[Error::InvalidSetType] =
   "'{0}': invalid set type; expected scalar or subrange type.";

   errorMap[Error::MismatchedSubrangeLimit] =
   "subrange limit types do not match ('{0}', '{1}').";

   errorMap[Error::InvalidSubrange] =
   "invalid subrange ({0}..{1}).";

   errorMap[Error::UndeclaredIdentifier] =
   "'{0}': undeclared identifier.";

   errorMap[Error::AmbiguousFileAccess] = 
   "'{0}': ambiguous file access.";

   errorMap[Error::IncompatibleSetTypes] = 
   "incompatible set types.";

   errorMap[Error::IncompatibleSetTypesForMembership] = 
   "incompatible types for set membership.";

   errorMap[Error::MismatchedMemberDesignators] = 
   "mismatched types in member designator list.";

   errorMap[Error::IndexListCount] =
   "index list count mismatch ({0}, {1}).";

   errorMap[Error::MismatchedIndexType] = 
   "mismatched index type (expected '{0}').";

   errorMap[Error::MismatchedArrayElementTypes] =
   "'{0}': mismatched array element types.";

   errorMap[Error::MismatchedArraySizes] =
   "'{0}': mismatched array sizes.";

   errorMap[Error::InvalidWithField] =
   "'{0}' : invalid field for with clause.";

   errorMap[Error::FormalParameterRedeclaration] =
   "redeclaration of formal parameter '{0}'.";

   errorMap[Error::ProgramParameterRedeclaration] =
   "redeclaration of program parameter '{0}'.";

   errorMap[Error::UndeclaredProgramParameter] = 
   "file '{0}' must be declared in program parameter list.";

   errorMap[Error::MustBeVarParameter] = 
   "'{0}': {1} parameters must be specified as var-parameters.";

   errorMap[Error::ConstantVarParameter] = 
   "'{0}': var-parameter '{1}' must be non-constant.";

   errorMap[Error::InvalidArgument] = 
   "'{0}': expected '{1}' type for argument {2}.";

   errorMap[Error::InvalidArgumentCount] = 
   "'{0}': expected {1} arguments; {2} provided.";

   errorMap[Error::InvalidProcedureArgument] = 
   "procedure and function arguments may only contain value parameters.";

   errorMap[Error::InvalidProcedureArgument] = 
   "procedure and function arguments may only contain value parameters.";

   errorMap[Error::SymbolRedeclaration] = 
   "'{0}' already declared as type '{1}'.";

   errorMap[Error::FunctionRedeclaration] =
   "there is already a {0} named '{1}'.";

   errorMap[Error::InvalidReturnType] =
   "function return type must be scalar, subrange, or pointer type.";

   errorMap[Error::FunctionUnreachable] = 
   "'{0}': {1} not reachable from current scope.";

   errorMap[Error::UndeclaredFunction] = 
   "'{0}': undeclared procedure or function.";

   errorMap[Error::UnknownDirective] = 
   "'{0}': unknown directive.";

   errorMap[Error::InvalidExternalFunctionDefinition] = 
   "'{0}': definition of external {1} must occur in separate module.";

   errorMap[Error::InvalidForwardFunctionDefinition] = 
   "'{0}': {1} is forward-declared; repetition of parameter list not allowed.";

   errorMap[Error::UnknownTypeSymbol] = 
   "Could not resolve symbol for type '{0}'";

   errorMap[Error::InvalidWidthSpecifier] = 
   "invalid width specifier.";
}

}  // namespace Pascal