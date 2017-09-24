//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Implementation of class RankedSymExtensionObject.
//
//-----------------------------------------------------------------------------


#include "RankedSymExtensionObject.h"

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieve the RankedSymExtensionObject attached to a Symbol.
//
// Remarks:
//
//    This function assumes there is a RankedSymExtensionObject attached
//    to the argument. It will throw an InvalidCastException if there is none.
//
// Returns:
//
//    The RankedSymExtensionObject or nil.
//
//-----------------------------------------------------------------------------


RankedSymExtensionObject ^ 
RankedSymExtensionObject::Get
(
   Phx::Symbols::Symbol ^sym
)
{
   return safe_cast<RankedSymExtensionObject^>(
      sym->FindExtensionObject(RankedSymExtensionObject::typeid));
}

