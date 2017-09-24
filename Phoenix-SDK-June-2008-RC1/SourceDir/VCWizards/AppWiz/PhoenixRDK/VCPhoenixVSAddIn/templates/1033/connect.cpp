//-----------------------------------------------------------------------------
//
// Description:
//
//    Primary add-in controller class.
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "connect.h"
[!if TOOLS_MENU_ITEM || OTHER_WINDOWS_MENU_ITEM || TOOLBAR_ITEM]
#include "iconconverter.h"
[!endif]

namespace [!output SAFE_NAMESPACE_NAME]
{

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
Connect::OnAddInsUpdate
(
   Array ^% custom
)
{
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
Connect::OnBeginShutdown
(
   Array ^% custom
)
{
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
Connect::OnStartupComplete
(
   Array ^% custom
)
{
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
Connect::OnDisconnection
(
   ext_DisconnectMode removeMode,
   Array ^%           custom
)
{
}

[!if ADD_NAMED_COMMANDS]
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
Connect::QueryStatus
(
   String ^                  CmdName,
   vsCommandStatusTextWanted NeededText,
   vsCommandStatus %         StatusOption,
   Object ^%                 CommandText
)
{
   if (NeededText == vsCommandStatusTextWanted::vsCommandStatusTextWantedNone)
   {
[!if TOOLS_MENU_ITEM]
      // Note that the "default" property for Command is Name

      if (!CmdName->CompareTo(this->execCommand->default))
      {
            // Enable the command

            StatusOption = (vsCommandStatus)(
               vsCommandStatus::vsCommandStatusSupported+
               vsCommandStatus::vsCommandStatusEnabled);
      }
[!endif]
[!if ADD_SHOW_WINDOW_COMMANDS]
      if(!CmdName->CompareTo(this->showWindowCommand->default))
      {
         // Only show the menu item if the add-in is connected.

         if (this->addInInstance->Connected)
         {
            StatusOption = (vsCommandStatus)(
               vsCommandStatus::vsCommandStatusSupported +
               vsCommandStatus::vsCommandStatusEnabled);
         }
         else
         {
            StatusOption = (vsCommandStatus)(
               vsCommandStatus::vsCommandStatusUnsupported);
         }
      }
[!endif]
   }
}
[!endif]

[!if ADD_NAMED_COMMANDS]
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
Connect::Exec
(
   String ^            CmdName,
   vsCommandExecOption ExecuteOption,
   Object ^%           VariantIn,
   Object ^%           VariantOut,
   bool %              handled
)
{
   handled = false;
   if (ExecuteOption == vsCommandExecOption::vsCommandExecOptionDoDefault)
   {
[!if TOOLS_MENU_ITEM]
      // The this->execCommand's default parameter is the name of the command.
      // This command (Exec[!output SAFE_NAMESPACE_NAME]) shows the control flow graph
      // visualization.

      if (!CmdName->CompareTo(this->execCommand->default))
      {
         handled = true;
         return;
      }
[!endif]
[!if ADD_SHOW_WINDOW_COMMANDS]
      // The this->showWindowCommand's default parameter is the name of the command

      if (!CmdName->CompareTo(this->showWindowCommand->default))
      {
         // Show the tool window

         this->toolWindow->Visible = true;
         handled = true;
         return;
      }
[!endif]
   }
}
[!endif]

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
Connect::OnConnection
(
   Object ^        application,
   ext_ConnectMode connectMode,
   Object ^        addInInst,
   Array ^%        custom
)
{
   this->applicationObject = (DTE2 ^)application;
   this->addInInstance = (AddIn ^)addInInst;

   // The resource manager used to get the menu item names and 
   // retrieve general resources.

   this->resourceManager =
      gcnew ResourceManager("[!output SAFE_NAMESPACE_NAME].commandbar",
      System::Reflection::Assembly::GetExecutingAssembly());

   System::Threading::Thread ^ thread =
   System::Threading::Thread::CurrentThread;

   this->cultureInfo = thread->CurrentCulture;

[!if ADD_NAMED_COMMANDS]
   // All of the code that will only be executed once in the entire add-in 
   // cycle (install, operation, uninstall)

   if (connectMode == ext_ConnectMode::ext_cm_UISetup)
   {
      try
      {
         // Add the set of named commands first

         this->AddNamedCommands();

[!if TOOLS_MENU_ITEM || OTHER_WINDOWS_MENU_ITEM]
         // Add the set of menu items next

         this->AddMenus();
[!endif]

[!if TOOLBAR_ITEM]
         // And finally add the set of toolbars and associated controls

         this->AddToolbars();
[!endif]

      }
      catch(Exception ^ e)
      {
         System::Diagnostics::Debug::Print(e->ToString());
      }

      // If this is the UI startup, we can exit here

      return;
   }

   // Here we have the code that will be executed each time you start the 
   // add-in after the initial install run.

   try {

      // We need to get the commands before we do anything else

   this->FindNamedCommands();
[!endif]

[!if TOOL_WINDOW]
   // First initialize the tool window

   this->InitializeToolWindows();
[!endif]

   // And now initialize the events handlers

      this->InitializeEvents();

   }
   catch(Exception ^ e)
   {
      System::Diagnostics::Debug::Print(e->ToString());
   }

   // Initialize the phoenix controller

   this->phoenixProvider = gcnew PhoenixProvider();
}

[!if ADD_NAMED_COMMANDS]
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
Connect::AddNamedCommands()
{
   array<Object ^> ^ contextGUIDs = gcnew array<Object ^>(0);

   // The set of VS commands

   Commands2 ^ commands = (Commands2 ^)this->applicationObject->Commands;

[!if TOOLS_MENU_ITEM]
   //Add a command to the Commands collection to launch the 
   //add-in, with no icon.

   this->execCommand = commands->AddNamedCommand2(this->addInInstance, 
      "Exec[!output SAFE_NAMESPACE_NAME]", "[!output ADDIN_NAME]", 
      "[!output ADDIN_DESCRIPTION]", 
      true, nullptr, contextGUIDs, 
      (int)vsCommandStatus::vsCommandStatusSupported+
      (int)vsCommandStatus::vsCommandStatusEnabled, 
      (int)vsCommandStyle::vsCommandStylePictAndText, 
      vsCommandControlType::vsCommandControlTypeButton);
[!endif]

[!if ADD_SHOW_WINDOW_COMMANDS]
   // Add another command to the Commands collection that
   // shows the tool window.  It's invisible though.

   this->showWindowCommand = commands->AddNamedCommand2(this->addInInstance,
      "[!output SAFE_NAMESPACE_NAME]ToolWindow", "[!output ADDIN_NAME] Window",
      "Shows the [!output ADDIN_NAME] Window", true, nullptr, 
      contextGUIDs,
      (int)vsCommandStatus::vsCommandStatusInvisible +
      (int)vsCommandStatus::vsCommandStatusUnsupported,
      (int)vsCommandStyle::vsCommandStylePictAndText,
      vsCommandControlType::vsCommandControlTypeButton);
[!endif]
}


//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
Connect::FindNamedCommands()
{
   // The set of VS commands

   Commands2 ^ commands = (Commands2 ^)this->applicationObject->Commands;

[!if TOOLS_MENU_ITEM]
   this->execCommand = 
      commands->Item(
      "[!output SAFE_NAMESPACE_NAME].Connect.Exec[!output SAFE_NAMESPACE_NAME]", 
      -1);
[!endif]

[!if ADD_SHOW_WINDOW_COMMANDS]
   this->showWindowCommand = 
      commands->Item(
      "[!output SAFE_NAMESPACE_NAME].Connect.[!output SAFE_NAMESPACE_NAME]ToolWindow", 
      -1);
[!endif]
}
[!endif]

[!if TOOLS_MENU_ITEM || OTHER_WINDOWS_MENU_ITEM]
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
Connect::AddMenus()
{
   // The set of command bars for visual studio (toolbars, menus, etc.)

   Microsoft::VisualStudio::CommandBars::_CommandBars ^ commandBars;

   // The main menu bar

   CommandBar ^ menuBarCommandBar;
   
[!if TOOLS_MENU_ITEM]
   CommandBarPopup ^ toolsPopup;
[!endif]

[!if OTHER_WINDOWS_MENU_ITEM]
   CommandBarPopup ^ viewPopup;
   CommandBarPopup ^ otherWindowsPopup; 
[!endif]

   // Our AxHost derived class for icon conversion

   [!output SAFE_NAMESPACE_NAME]::IconConverter ^ converter = 
      gcnew [!output SAFE_NAMESPACE_NAME]::IconConverter();

   // Get the set of command bars

   commandBars = (CommandBars ^)this->applicationObject->CommandBars;

   // Find the MenuBar command bar, which is the top-level command bar 
   // holding all the main menu items:

   menuBarCommandBar = commandBars["MenuBar"];

   // Retrieve the icons for the menu/toolbar items

   Bitmap ^ iconbmp =
      (Bitmap ^) this->resourceManager->GetObject("phxiconmain");
   stdole::IPictureDisp ^ icon = converter->GetIPictureDisp(iconbmp);

   Bitmap ^ maskbmp =
      (Bitmap ^) this->resourceManager->GetObject("phxiconmainmask");
   stdole::IPictureDisp ^ mask = converter->GetIPictureDisp(maskbmp);

[!if OTHER_WINDOWS_MENU_ITEM]
   String ^viewMenuName, ^otherWindowsMenuName;
[!endif]
[!if TOOLS_MENU_ITEM]
   String ^toolsMenuName;
[!endif]
   try
   {
      // The names of common menus and toolbars and provided in the 
      // CommandBar.resx resource file, localized by language.  To
      // find a menu item, concatenate the two letter ISO lengauge name
      // with the english name of the menu or toolbar.

      String ^ langName = cultureInfo->TwoLetterISOLanguageName;
[!if OTHER_WINDOWS_MENU_ITEM] 
      viewMenuName = 
         this->resourceManager->GetString(langName + "View");
      otherWindowsMenuName =
         this->resourceManager->GetString(langName + "Other Windows");
[!endif]
[!if TOOLS_MENU_ITEM]
      toolsMenuName = 
         this->resourceManager->GetString(langName + "Tools");
[!endif]
   }
   catch(Exception ^ e)
   {
      System::Diagnostics::Debug::Print(e->ToString());
      
      // We tried to find a localized version of a menu name, 
      // but one was not found.  Default to the en-US word, which may work 
      // for the current culture.

[!if OTHER_WINDOWS_MENU_ITEM]
      viewMenuName = "View";
      otherWindowsMenuName = "Other Windows";
[!endif]
[!if TOOLS_MENU_ITEM]
      toolsMenuName = "Tools";
[!endif]
   }
   
[!if OTHER_WINDOWS_MENU_ITEM]
   // We want to add a menu item to "View->Other Windows" to show our 
   // Window.

   viewPopup = (CommandBarPopup ^) menuBarCommandBar->Controls[viewMenuName];

   // Now attempt to to get the "Other Windows" menu item

   otherWindowsPopup =
      (CommandBarPopup ^) viewPopup->Controls[otherWindowsMenuName];

   // Add the menu item

   this->AddMenuItem(otherWindowsPopup->CommandBar,
      this->showWindowCommand, icon, mask, 1, true);

   // Since we're adding the menu item to the top of the menu, simply setting
   // startNewGroup to true will not create a separator, because this
   // property must ride along with the previous top menu item.  Set
   // this property if there are more one items in the menu.

   if (otherWindowsPopup->Controls->Count > 1)
   {
      otherWindowsPopup->Controls[2]->BeginGroup = true;
   }
[!endif]

[!if TOOLS_MENU_ITEM]

   // We want to add a menu item to "Tools menu" to launch/execute the
   // add-in.

   toolsPopup = (CommandBarPopup ^)menuBarCommandBar->Controls[toolsMenuName];

   // Add the menu item

   AddMenuItem(toolsPopup->CommandBar, this->execCommand, icon, mask, 1, true);

   // Since we're adding the menu item to the top of the menu, simply setting
   // startNewGroup to true will not create a separator, because this
   // property must ride along with the previous top menu item.  Set
   // this property if there are more one items in the menu.

   if(toolsPopup->Controls->Count > 1)
   {
      toolsPopup->Controls[2]->BeginGroup = true;
   }
[!endif]
}
[!endif]

[!if TOOLS_MENU_ITEM || OTHER_WINDOWS_MENU_ITEM]
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

CommandBarButton ^
Connect::AddMenuItem
(
   CommandBar ^           commandBar,
   Command ^              command,
   stdole::IPictureDisp ^ icon,
   stdole::IPictureDisp ^ mask,
   int                    insertionIndex,
   bool                   startNewGroup
)
{
   CommandBarButton ^ menuItemButton;
   Object ^           menuItemControlObject;

   menuItemControlObject = command->AddControl(commandBar, insertionIndex);

   menuItemButton =
      (CommandBarButton ^) menuItemControlObject;

   // Now we set up the icon for the menu item.

   menuItemButton->Picture = (stdole::StdPicture ^) icon;
   menuItemButton->Mask = (stdole::StdPicture ^) mask;
   menuItemButton->BeginGroup = startNewGroup;

   return menuItemButton;
}
[!endif]

[!if TOOLBAR_ITEM]
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

void
Connect::AddToolbars()
{
   // The set of command bars for visual studio (toolbars, menus, etc.)

   Microsoft::VisualStudio::CommandBars::_CommandBars ^ commandBars;
   commandBars = (CommandBars ^) this->applicationObject->CommandBars;

   // The icon converter

   IconConverter ^ converter = gcnew IconConverter();

   // Note: casting to object to prevent "implicit boxing" warning

   CommandBar ^ addinToolbar = commandBars->Add("[!output ADDIN_NAME]",
      MsoBarPosition::msoBarFloating, (Object ^)false, (Object ^)false);

[!if TOOL_WINDOW]
   // For show, add the show window command to the toolbar

   CommandBarControl ^ showWindowControl =
      (CommandBarControl ^)this->showWindowCommand->AddControl(addinToolbar, 1);

   addinToolbar->Visible = true;

   // And set up the icon

   CommandBarButton ^ showWindowButton = (CommandBarButton ^) showWindowControl;

   Bitmap ^ iconbmp =
      (Bitmap ^)this->resourceManager->GetObject("phxiconmain");
   stdole::IPictureDisp ^ icon = converter->GetIPictureDisp(iconbmp);
   showWindowButton->Picture = (stdole::StdPicture ^) icon;

   Bitmap ^ maskbmp =
      (Bitmap ^)this->resourceManager->GetObject("phxiconmainmask");
   stdole::IPictureDisp ^ mask = converter->GetIPictureDisp(maskbmp);
   showWindowButton->Mask = (stdole::StdPicture^)mask;
[!endif]
}
[!endif]

[!if TOOL_WINDOW]
//------------------------------------------------------------------------------
//
// Description:
//
//    Creates the tool windows for the add-in.
//
// Remarks:
//
//    This method is called every time the add-in is loaded into the IDE.
//    Tool windows are not persistent across instances of the IDE, unlike
//    named commands or menu items.
//
//------------------------------------------------------------------------------

void
Connect::InitializeToolWindows()
{
   // Now we need to add a new tool window for the use of the Add-In.
   // The hosted tool window control is a User Control that you may draw
   // on, add controls to it, etc.

   Object ^ hostedControlObject;

   this->toolWindow = this->InitializeToolWindow(
      "[!output SAFE_NAMESPACE_NAME].ToolWindowControl", 
      "[!output ADDIN_NAME] Tool Window",
      "{[!output TOOL_WINDOW_GUID]}",
      hostedControlObject);
   this->toolWindowControl = 
      (ToolWindowControl ^)hostedControlObject;

   // And now show the window.  Window must be set to visible before you
   // can do anything else with it

   this->toolWindowControl->Visible = true;

   // Now call initialize on our tool window.

   this->toolWindowControl->Initialize(applicationObject);
}

//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------

Window  ^
Connect::InitializeToolWindow
(
   String ^  controlClass,
   String ^  caption,
   String ^  guidString,
   Object ^% hostedControlObject
)
{
   String ^ thisAssemblyPath =
      System::Reflection::Assembly::GetExecutingAssembly()->Location;

   Windows2 ^ windows = (Windows2 ^)this->applicationObject->Windows;

   hostedControlObject = nullptr;

   return windows->CreateToolWindow2(
      this->addInInstance, thisAssemblyPath, controlClass, caption,
      guidString, hostedControlObject);
}
[!endif]

//------------------------------------------------------------------------------
//
// Description:
//
//    Events the add-in needs to consume
//
// Remarks:
//
//    Add your events here.
//
//------------------------------------------------------------------------------

void
Connect::InitializeEvents()
{

   // Events you need here.
   // First, grab the instances of the event handler references.
   //  The event handler references must be kept live
   // during the time in which you wish to consume the event.

   // Example:
   // buildEvents = applicationObject->Events->BuildEvents;

   // Add one for build done.
   // buildEvents->OnBuildDone += 
   //    gcnew EnvDTE::_dispBuildEvents_OnBuildDoneEventHandler(this, 
   //    &Connect::BuildFinish);

}

} // [!output SAFE_NAMESPACE_NAME]