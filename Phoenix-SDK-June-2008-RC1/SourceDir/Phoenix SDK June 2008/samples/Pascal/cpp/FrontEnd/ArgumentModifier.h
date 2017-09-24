//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the ArgumentModifier class.
//
//-----------------------------------------------------------------------------

#pragma once

namespace Pascal 
{

//-----------------------------------------------------------------------------
//
// Description: Helper class for generating call to the standard
//              built-in 'write' and 'writeln' procedures.
//
// Remarks:
//              Each parameter to 'write' and 'writeln' takes the 
//              following form:
//
//                e
//                e:e1
//                e:e1:e2
//
//              e1 is the minimum field width specifier. It specifies the  
//              minimum number of characters to write to the output stream. 
//              The space character is used to create preceding blanks.
//              e1 can be used to control the minimum width of any allowable
//              type that can be passed to the write and writeln procedures.
//
//              e2 is the fraction length specifier. It only applies to real
//              values. It specifies the number of digits that follow the 
//              decimal point.
//
//              If no minimum field width is specified, the default value is 0.
//              If no fraction length is specified, the value is formatted
//              in decimal floating-point form.
//
//-----------------------------------------------------------------------------

ref struct ArgumentModifier
{
public:
	ArgumentModifier
	(
		Phx::IR::Operand ^ baseOperand,
		Phx::IR::Operand ^ minimumFieldWidth,
		Phx::IR::Operand ^ fractionLength
	)
		: baseOperand(baseOperand)
		, minimumFieldWidth(minimumFieldWidth)
		, fractionLength(fractionLength)
	{
	}

   // Retrieves the base operand to be written.

	property Phx::IR::Operand ^ BaseOperand
	{
		Phx::IR::Operand ^ get()
		{
			return this->baseOperand;
		}
	}

   // Retrieves the minimum field width specifier.

	property Phx::IR::Operand ^ MinimumFieldWidth
	{
		Phx::IR::Operand ^ get()
		{
			return this->minimumFieldWidth;
		}
	}

   // Retrieves the fraction length specifier.

	property Phx::IR::Operand ^ FractionLength
	{
		Phx::IR::Operand ^ get()
		{
			return this->fractionLength;
		}
	}

private:

	Phx::IR::Operand ^ baseOperand;
	Phx::IR::Operand ^ minimumFieldWidth;
	Phx::IR::Operand ^ fractionLength;
};

} // namespace Pascal