//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Extension object for VariableSymbol.
//
// Remarks:
//
//    Exposes property OrderNumber to indicate an order of syntactic
//    occurrence for local variables and formal parameters.
//    This order is the id that is used by ilasm to refer to local
//    variables.
//
//-----------------------------------------------------------------------------


#pragma once

//-----------------------------------------------------------------------------
//
// Description:
//
//    Extend a symbol with a syntactic occurrence index. 
//
// Remarks: 
//
//    Used to generate ilasm references to autos and parameters.
//
//-----------------------------------------------------------------------------

ref class RankedSymExtensionObject : public Phx::Symbols::SymbolExtensionObject
{
public:
   RankedSymExtensionObject
   (
      int orderNumber
   )
      : _orderNumber(orderNumber)
   {
   }

   // Retrieve a RankedSymExtensionObject from a Symbol

   static RankedSymExtensionObject ^ Get(Phx::Symbols::Symbol ^sym);

   // Syntactic occurrence order of a symbol.

   property int OrderNumber
   {
      int get()
      {
      return _orderNumber;
      }
   }

   // Public static property to manage the order number of local variables.

   static property int LocalIndex;

private:
   int                 _orderNumber;
};
