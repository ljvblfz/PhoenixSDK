//------------------------------------------------------------------------------
//
// Description:
//
//    User control for the tool window.
//
//------------------------------------------------------------------------------

#pragma once

#pragma region Using Namespaces

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace Extensibility;
using namespace EnvDTE;
using namespace EnvDTE80;

#pragma endregion

namespace [!output SAFE_NAMESPACE_NAME]
{

//------------------------------------------------------------------------------
//
// Description:
//
//    User control that displays a control flow graph.
//
//------------------------------------------------------------------------------

public ref class 
ToolWindowControl : public UserControl
{
public:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Default constructor.  Initializes the control.
   //
   //---------------------------------------------------------------------------

   ToolWindowControl();
   
   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Initializes this control.
   //
   // Arguments:
   //
   //    dte - The DTE instance for the IDE.
   //
   // Remarks:
   //
   //    Because CreateToolWindow2 does not call anything but the default
   //    constructor, there is a separate initialization function.
   //
   //---------------------------------------------------------------------------

   void 
   Initialize
   (
      DTE2 ^ dte
   );

private:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Event helper method.
   //
   //---------------------------------------------------------------------------

   void 
   InitializeEvents();

   // The dte interface instance

   DTE2 ^ applicationObject;

   // Required designer variable

   System::ComponentModel::Container ^ components;

#pragma region Windows Form Designer generated code

   void InitializeComponent(void)
   {
      this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
   }

#pragma endregion

};

} // [!output SAFE_NAMESPACE_NAME]
