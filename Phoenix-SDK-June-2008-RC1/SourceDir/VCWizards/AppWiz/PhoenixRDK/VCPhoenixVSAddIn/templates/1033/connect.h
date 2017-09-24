//-----------------------------------------------------------------------------
//
// Description:
//
//    Primary add-in controller class.
//
//-----------------------------------------------------------------------------

#pragma once

[!if TOOL_WINDOW]
#include "toolwindowcontrol.h"
[!endif]
#include "phoenixprovider.h"

#pragma region Using Namespace Declarations

using namespace System;
using namespace System::Globalization;
using namespace System::Resources;
using namespace Extensibility;
using namespace EnvDTE;
using namespace EnvDTE80;
using namespace Microsoft::VisualStudio::CommandBars;

#pragma endregion

namespace [!output SAFE_NAMESPACE_NAME]
{
   
//------------------------------------------------------------------------------
//
// Description:
//
//    The primary controller class for the add-in.
//
// Remarks:
//
//    The Connect class is the primary controller class in the
//    add-in.  It also contains the implementation of the 
//    IDTExtensibility and IDTCommandTarget interfaces that Visual Studio 
//    uses to hook up add-ins.
//
//------------------------------------------------------------------------------

public ref class 
[!if ADD_NAMED_COMMANDS]
Connect : public IDTExtensibility2, public IDTCommandTarget
[!else]
Connect : public IDTExtensibility2
[!endif]
{
public:

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Implementation for the IDTExtensibility2 interface.  Called
   //    when an add-in is loaded or unloaded from Visual Studio.
   //
   // Arguments:
   //
   //    custom - An empty array that you can use to pass host-specific 
   //    data for use in the add-in.
   //
   //---------------------------------------------------------------------------

   virtual void
   OnAddInsUpdate
   (
      Array ^% custom
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Implementation for the IDTExtensibility2 interface.  Called
   //    when Visual Studio is shutting down while an add-in is loaded.
   //
   // Arguments:
   //
   //    custom - An empty array that you can use to pass host-specific 
   //    data for use in the add-in.
   //
   //---------------------------------------------------------------------------

   virtual void
   OnBeginShutdown
   (
      Array ^% custom
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Implementation for the IDTExtensibility2 interface.  Called
   //    when Visual Studio is loads the add-in, if it was set to load
   //    on startup.
   //
   // Arguments:
   //
   //    custom - An empty array that you can use to pass host-specific 
   //    data for use in the add-in.
   //
   //---------------------------------------------------------------------------

   virtual void
   OnStartupComplete
   (
      Array ^% custom
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Implementation for the IDTExtensibility2 interface.  Called
   //    when Visual Studio unloads the add-in.
   //
   // Arguments:
   //
   //    removeMode - The manner in which the add-in was unloaded
   //    custom - An empty array that you can use to pass host-specific 
   //    data for use in the add-in.
   //
   //---------------------------------------------------------------------------

   virtual void
   OnDisconnection
   (
      ext_DisconnectMode removeMode,
      Array ^%           custom
   );

[!if ADD_NAMED_COMMANDS]
   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Implementation for the IDTCommandTarget interface.  Called
   //    to determine the status of a named command.
   //
   // Arguments:
   //
   //    CmdName - The name of the command.
   //    NeededText - The desired return information from QueryStatus.
   //    StatusOption - The status of the command.
   //    CommandText - The text to return if vsCommandStatusTextWantedStatus 
   //    is specified for NeededText.
   //
   //---------------------------------------------------------------------------

   virtual void
   QueryStatus
   (
      String ^                  CmdName,
      vsCommandStatusTextWanted NeededText,
      vsCommandStatus %         StatusOption,
      Object ^%                 CommandText
   );

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Implementation for the IDTCommandTarget interface.  Called
   //    when a named command created by the add-in is executed.
   //
   // Arguments:
   //
   //    CmdName - The name of the command.
   //    ExecuteOption - The command execution options.
   //    VariantIn - A value passed to the command.
   //    VariantOut - A value passed back to the invoker Exec method after 
   //    the command executes.
   //    handled - A value indicating whether the command was handled by
   //    this Exec implementation.
   //
   //---------------------------------------------------------------------------

   virtual void
   Exec
   (
      String ^            CmdName,
      vsCommandExecOption ExecuteOption,
      Object ^%           VariantIn,
      Object ^%           VariantOut,
      bool %              handled
   );
[!endif]

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Implementation for the IDTExtensibility2 interface.  Called
   //    when Visual Studio is loads the add-in.
   //
   // Arguments:
   //
   //    application - The DTE instance for Visual Studio.
   //    connectMode - The manner in which the add-in was loaded by the IDE.
   //    addInInst - An instance of EnvDTE::AddIn representing this add-in.
   //    custom - An empty array that you can use to pass host-specific 
   //    data for use in the add-in.
   //
   //---------------------------------------------------------------------------

   virtual void
   OnConnection
   (
      Object ^        application,
      ext_ConnectMode connectMode,
      Object ^        addInInst,
      Array ^%        custom
   );

private:

[!if ADD_NAMED_COMMANDS]
   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Adds named commands to the IDE.
   //
   // Remarks:
   //
   //    This method is called only the first time the add-in is loaded
   //    after being installed.  It initializes the named commands that
   //    the add-in will use to interact with the user.
   //
   //---------------------------------------------------------------------------

   void
   AddNamedCommands();

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Retrieves the named commands created by the 
   //
   // Remarks:
   //
   //    This method is called every time the add-in is loaded into the IDE.
   //    It retrieves the named commands created by AddNamedCommands so that
   //    if needed, they may be directly manipulated.
   //
   //---------------------------------------------------------------------------

   void
   FindNamedCommands();
[!endif]

[!if TOOLS_MENU_ITEM || OTHER_WINDOWS_MENU_ITEM]
   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Adds menu items to the IDE.
   //
   // Remarks:
   //
   //    This method is called only the first time the add-in is loaded
   //    after being installed.  It initializes the menu items for the
   //    add-in.
   //
   //---------------------------------------------------------------------------

   void
   AddMenus();

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Helper method that creates a menu item button on the specified 
   //    command bar, attaches it to a specific command, and gives it a
   //    picture.
   //
   // Arguments:
   //
   //    commandBar - The command bar that the menu button should be added
   //    to.
   //    command - The named command that this menu item will execute when it
   //    is clicked.
   //    icon - The icon for the menu button.
   //    mask - The transparency mask for the menu button.
   //    insertionIndex - The index at which to insert the menu command.
   //    Use 1 to insert at the top of the menu.
   //    startNewGroup - If true, this menu item will start a new group.
   //    It will be seperated from the menu item above it.
   //
   //---------------------------------------------------------------------------

   CommandBarButton ^
   AddMenuItem
   (
      CommandBar ^           commandBar,
      Command ^              command,
      stdole::IPictureDisp ^ icon,
      stdole::IPictureDisp ^ mask,
      int                    insertionIndex,
      bool                   startNewGroup
   );
[!endif]

[!if TOOLBAR_ITEM]
   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Adds toolbars to the IDE.
   //
   // Remarks:
   //
   //    This method is called only the first time the add-in is loaded.  It
   //    permanantly adds toolbars and toolbar buttons to the IDE.
   //
   //---------------------------------------------------------------------------

   void
   AddToolbars();
[!endif]

[!if TOOL_WINDOW]
   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Creates the tool windows for the control flow graph.
   //
   // Remarks:
   //
   //    This method is called every time the add-in is loaded into the IDE.
   //    Tool windows are not persistent across instances of the IDE, unlike
   //    named commands or menu items.
   //
   //---------------------------------------------------------------------------

   void
   InitializeToolWindows();

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Helper method that creates a tool window.
   //
   // Arguments:
   //
   //    controlClass - The qualified class name of the control that
   //    the tool window should host.
   //    caption - The caption of the tool window.
   //    guidString - The guid that identifies the window.
   //    hostedControlObject - [out] The instance of the class specified by
   //    controlClass that the tool window creates.
   //
   // Returns:
   //
   //    The Window instance created by the IDE that houses the tool window.
   //
   //---------------------------------------------------------------------------

   Window ^
   InitializeToolWindow
   (
      String ^  controlClass,
      String ^  caption,
      String ^  guidString,
      Object ^% hostedControlObject
   );
[!endif]

   //---------------------------------------------------------------------------
   //
   // Description:
   //
   //    Events the add-in needs to consume
   //
   // Remarks:
   //
   //    Add your events here.
   //
   //---------------------------------------------------------------------------

   void 
   InitializeEvents();

   // DTE instance

   DTE2 ^ applicationObject;

   // Instance of this add-in

   AddIn ^ addInInstance;

[!if ADD_NAMED_COMMANDS]
   // The set of commands that we added using named commands.
   // These are retrieved on Connect

   Command ^ execCommand;
   Command ^ showWindowCommand;
[!endif]

[!if TOOL_WINDOW]
   // The primary tool window instance

   Window ^ toolWindow;

   // And the associated user control

   ToolWindowControl^ toolWindowControl;
[!endif]

   // Provides a front-end to the Phoenix SDK for the add-in

   PhoenixProvider ^ phoenixProvider;

   // The resource manager that is used throughout the add-in

   ResourceManager ^ resourceManager;

   // And the neccesary associated culture information

   CultureInfo ^ cultureInfo;
};

} // [!output SAFE_NAMESPACE_NAME]