//------------------------------------------------------------------------------
//
// Description:
//
//    Phoenix subsystem of the add-in.  Provides Phoenix services to the
//    rest of the add-in.
//
//------------------------------------------------------------------------------

#pragma once

#pragma region Using namespace declarations

using namespace System;

#pragma endregion

namespace [!output SAFE_NAMESPACE_NAME]
{

//------------------------------------------------------------------------------
//
// Description:
//
//    Provides Phoenix functionality to the controller class, Connect.
//
// Remarks:
//
//    This class serves as a front end for the Phoenix subsystem of the
//    add-in.
//
//------------------------------------------------------------------------------

public ref class 
PhoenixProvider
{
public:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Default constructor.
   //
   //---------------------------------------------------------------------------

   PhoenixProvider();

private:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Initializes Phoenix.
   //
   //---------------------------------------------------------------------------

   void
   InitializePhx();

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Registers the targets available in the RDK.
   //
   //---------------------------------------------------------------------------

   void 
   InitializeTargets();

};

} // [!output SAFE_NAMESPACE_NAME]