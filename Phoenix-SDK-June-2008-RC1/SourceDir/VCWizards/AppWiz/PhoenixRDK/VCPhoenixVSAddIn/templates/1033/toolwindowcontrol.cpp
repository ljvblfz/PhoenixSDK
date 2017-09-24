//------------------------------------------------------------------------------
//
// Description:
//
//    User control for the tool window.
//
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "toolwindowcontrol.h"

namespace [!output SAFE_NAMESPACE_NAME]
{   

//------------------------------------------------------------------------------
//
// Description:
//
//    Default constructor.  Initializes the control.
//
//------------------------------------------------------------------------------

ToolWindowControl::ToolWindowControl()
{
   this->InitializeComponent();
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Initializes this control.
//
// Arguments:
//
//    dte - The DTE instance for the IDE
//
// Remarks:
//
//    Because CreateToolWindow2 does not call anything but the default
//    constructor, there is a separate initialization function. 
//
//------------------------------------------------------------------------------

void 
ToolWindowControl::Initialize
(
   DTE2 ^ dte
)
{
   this->applicationObject = dte;
   
   // And finally initialize the events

   this->InitializeEvents();
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Event helper method.
//
//------------------------------------------------------------------------------

void 
ToolWindowControl::InitializeEvents()
{
   // TODO: Add your events  here.
}

} // [!output SAFE_NAMESPACE_NAME]