//------------------------------------------------------------------------------
//
// Description:
//
//    Converts images from the .NET Image format to the Office IPictureDisp
//    format.
//
//------------------------------------------------------------------------------

#include "iconconverter.h"

namespace [!output SAFE_NAMESPACE_NAME]
{

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

IPictureDisp ^
IconConverter::GetIPictureDisp
(
   Image ^ image
)
{
   return (IPictureDisp ^)GetIPictureDispFromPicture(image);
}

} // [!output SAFE_NAMESPACE_NAME]