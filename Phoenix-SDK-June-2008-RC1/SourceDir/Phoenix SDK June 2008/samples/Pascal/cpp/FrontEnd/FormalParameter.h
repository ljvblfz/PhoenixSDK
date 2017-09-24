//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//     Definition of the FormalParameter class.
//
//-----------------------------------------------------------------------------

#pragma once

namespace Pascal 
{

// Describes a type of formal parameter. In Pascal, parameters can be passed
// by value (value), by reference (variable), or by procedure or function.

enum class FormalParameterType
{
   Variable,
   Value,
   Procedure,
   Function
};

//-----------------------------------------------------------------------------
//
// Description:
// 
//    Describes a single formal parameter to a procedure or function.   
//
// Remarks:
//    
//
//-----------------------------------------------------------------------------

ref class FormalParameter
{
public:
   FormalParameter
   (
      String ^ name,
      Phx::Types::Type ^ type,
      FormalParameterType parameterType,
      int sourceLineNumber
   )
   {
      Name = name;
      Type = type;
      ParameterType = parameterType;
      SourceLineNumber = sourceLineNumber;
   }

   // The program name of the parameter.
   property String ^ Name;
   // The type of the parameter.
   property Phx::Types::Type ^ Type;
   // The kind of the parameter.
   property FormalParameterType ParameterType;
   // The source line number the parameter is declared at.
   property int SourceLineNumber;
};

} // namespace Pascal 