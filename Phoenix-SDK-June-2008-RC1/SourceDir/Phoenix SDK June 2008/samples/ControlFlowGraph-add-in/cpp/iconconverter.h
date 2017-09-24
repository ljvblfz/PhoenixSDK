//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Converts images from the .NET Image format to the Office IPictureDisp
//    format.
//
//------------------------------------------------------------------------------

#pragma once

#pragma region Using namespace declarations

using namespace System::Drawing;
using namespace System::IO;
using namespace stdole;
using namespace System::Windows::Forms;

#pragma endregion

namespace ControlFlowGraph
{

//------------------------------------------------------------------------------
//
// Description:
//
//    Helper class which convert icons from Bitmap to IPictureDisp.
//
// Remarks:
//
//    The Visual Studio toolbar button interfaces will not deal with
//    with Image objects, only IPictureDisp.  This class uses AxHost to
//    perform the conversion between Image and IPictureDisp.
//
//------------------------------------------------------------------------------

public
ref class IconConverter : public AxHost
{
public:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Default constructor.
   //
   //---------------------------------------------------------------------------

   IconConverter()
      : AxHost("{6BA030F1-E0F9-40c2-A784-067C30C40600}")
   {
   }

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Converts an Image to an IPictureDisp.
   //
   // Arguments:
   //
   //    image - The image to convert.
   //
   // Remarks:
   //
   //    The converted image.
   //
   //---------------------------------------------------------------------------

   IPictureDisp ^
   GetIPictureDisp
   (
      Image ^ image
   );

};

} //ControlFlowGraph
