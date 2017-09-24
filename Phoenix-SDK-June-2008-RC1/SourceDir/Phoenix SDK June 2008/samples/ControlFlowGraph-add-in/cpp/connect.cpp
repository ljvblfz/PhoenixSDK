//------------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Primary add-in controller class.
//
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "connect.h"
#include "iconconverter.h"

namespace ControlFlowGraph
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
      // Note that the "default" property for Command is Name

      if (!CmdName->CompareTo(this->execCommand->default))
      {
         // Check the status of the current selection.  Determine
         // whether it is a CodeFunction.  If it is, enable the named
         // command.  If not (or there is no selection), disable the command.

         try
         {
            Object ^ selection = nullptr;

            if (this->applicationObject->SelectedItems->
               SelectionContainer->Count > 0)
            {
               selection = this->applicationObject->SelectedItems->
                  SelectionContainer->Item(1); 
            }

            // Attempt to cast the selection to a CodeFunction

            if ((dynamic_cast<EnvDTE::CodeFunction ^>(selection)) != nullptr)
            {
               // It is a code function, enable the command

               StatusOption = (vsCommandStatus)(
                  vsCommandStatus::vsCommandStatusSupported +
                  vsCommandStatus::vsCommandStatusEnabled);
            }
            else
            {
               // Not a CodeFunction.

               StatusOption = (vsCommandStatus)(
                  vsCommandStatus::vsCommandStatusUnsupported);
            }
         }
         catch(Exception ^ e)
         {
            System::Diagnostics::Debug::Print(e->ToString());

            // Something went wrong when getting the selection, disable
            // the command.

            StatusOption = (vsCommandStatus)(
               vsCommandStatus::vsCommandStatusUnsupported);
         }
      }
      if (!CmdName->CompareTo(this->showWindowCommand->default))
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
   }
}

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
      // The execCommand's default parameter is the name of the command.
      // This command (ExecControlFlowGraph) shows the control flow graph
      // visualization.

      if (!CmdName->CompareTo(this->execCommand->default))
      {
         CodeFunction ^ function = nullptr;
         try
         {
            SelectedItems ^ selectedItems =
               this->applicationObject->SelectedItems;
            Object ^ selection =
               selectedItems->SelectionContainer->Item(1);
            function = dynamic_cast<EnvDTE::CodeFunction ^>(selection);
         }
         catch(Exception ^ e)
         {
            System::Diagnostics::Debug::Print(e->ToString());

            // Problems loading the control graph, usually because
            // the item we attempt to load the flow graph for was
            // not a function.  Let the flow graph display, it will simply
            // display a blank window.
         }

         // Load the control flow graph into the window.

         this->DisplayFlowGraph(function);

         handled = true;
         return;
      }

      // The showWindowCommand's default parameter is the name of the command

      if (!CmdName->CompareTo(this->showWindowCommand->default))
      {
         //Show the tool window

         this->controlFlowGraphWindow->Visible = true;
         handled = true;
         return;
      }
   }
}

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
      gcnew ResourceManager("Control-Flow-Graph.commandbar",
         System::Reflection::Assembly::GetExecutingAssembly());

   System::Threading::Thread ^ thread =
   System::Threading::Thread::CurrentThread;

   this->cultureInfo = thread->CurrentCulture;

   // All of the code that will only be executed once in the entire add-in 
   // cycle (install, operation, uninstall)

   if (connectMode == ext_ConnectMode::ext_cm_UISetup)
   {
      try
      {
         // Add the set of named commands first

         this->AddNamedCommands();

         // Add the set of menu items next

         this->AddMenus();

         // And finally add the set of toolbars and associated controls

         this->AddToolbars();

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

   try
   {

      // We need to get the commands before we do anything else

      this->FindNamedCommands();

      // First initialize the tool window

      this->InitializeToolWindows();

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

   // Add a command to the Commands collection to launch the 
   // add-in, with no icon.

   this->execCommand = commands->AddNamedCommand2(this->addInInstance,
      "ExecControlFlowGraph", "Control Flow Graph",
      "Creates and displays a control flow graph for built binaries.",
      true, nullptr, contextGUIDs,
      (int)vsCommandStatus::vsCommandStatusSupported +
      (int)vsCommandStatus::vsCommandStatusEnabled,
      (int)vsCommandStyle::vsCommandStylePictAndText,
      vsCommandControlType::vsCommandControlTypeButton);

   // Add another command to the Commands collection that
   // shows the tool window.  It's invisible though.

   this->showWindowCommand = commands->AddNamedCommand2(this->addInInstance,
      "ControlFlowGraphToolWindow", "Control Flow Graph Window",
      "Shows the Control Flow Graph Window", true, nullptr,
      contextGUIDs,
      (int)vsCommandStatus::vsCommandStatusInvisible +
      (int)vsCommandStatus::vsCommandStatusUnsupported,
      (int)vsCommandStyle::vsCommandStylePictAndText,
      vsCommandControlType::vsCommandControlTypeButton);
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

   this->execCommand =
      commands->Item(
         "ControlFlowGraph.Connect.ExecControlFlowGraph",
         -1);

   this->showWindowCommand =
      commands->Item(
         "ControlFlowGraph.Connect.ControlFlowGraphToolWindow",
         -1);
}

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

   // The class view item command bar

   CommandBar ^ classViewItemCommandBar;

   // The command bar for the 'members' context menu that pops up in
   // the object browser

   CommandBar ^      objectBrowserCommandBar;
   CommandBarPopup ^ viewPopup;
   CommandBarPopup ^ otherWindowsPopup;

   // Our AxHost derived class for icon conversion

   IconConverter ^ converter = gcnew IconConverter();

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

   String ^ viewMenuName, ^ otherWindowsMenuName;
   String ^ classViewItemMenuName, ^ objectBrowserMenuName;

   try
   {
      // The names of common menus and toolbars are provided by the 
      // resource file CommandBar.resx, which is localized by language.
      // To find a menu item, concatenate the two-letter ISO language name
      // to the English name of the menu or toolbar.

      String ^ langName = this->cultureInfo->TwoLetterISOLanguageName;
      viewMenuName =
         this->resourceManager->GetString(langName + "View");
      otherWindowsMenuName =
         this->resourceManager->GetString(langName + "Other Windows");
      classViewItemMenuName =
         this->resourceManager->GetString(langName + "Class View Member");
      objectBrowserMenuName =
         this->resourceManager->GetString(langName +
            "Object Browser Members Pane");
   }
   catch(Exception ^ e)
   {
      System::Diagnostics::Debug::Print(e->ToString());

      // We tried to find a localized version of a menu name, 
      // but one was not found.  Default to the en-US word, which may work 
      // for the current culture.

      viewMenuName = "View";
      otherWindowsMenuName = "Other Windows";
      classViewItemMenuName = "Class View Member";
      objectBrowserMenuName = "Object Browser Members Pane";
   }

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

   // Add two new menu items using the AddMenuItem helper method.  Connect both
   // to the ExecControlFlowGraph named command.

   classViewItemCommandBar = commandBars[classViewItemMenuName];

   // Add the menu item

   AddMenuItem(classViewItemCommandBar, this->execCommand, icon, mask, 1, true);

   // Set new group for previous menu item too (see above comments for
   // explanation).

   if (classViewItemCommandBar->Controls->Count > 1)
   {
      classViewItemCommandBar->Controls[2]->BeginGroup = true;
   }

   objectBrowserCommandBar = commandBars[objectBrowserMenuName];

   // Add the menu item

   this->AddMenuItem(objectBrowserCommandBar,
      this->execCommand, icon, mask, 1, true);

   // Set new group for previous menu item too (see above comments for
   // explanation).

   if (objectBrowserCommandBar->Controls->Count > 1)
   {
      objectBrowserCommandBar->Controls[2]->BeginGroup = true;
   }
}

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

   ControlFlowGraph::IconConverter ^ converter =
      gcnew ControlFlowGraph::IconConverter();

   // Note: casting to object to prevent "implicit boxing" warning

   CommandBar ^ addinToolbar = commandBars->Add("Control Flow Graph",
      MsoBarPosition::msoBarFloating, (Object ^) false, (Object ^) false);

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
   showWindowButton->Mask = (stdole::StdPicture ^) mask;
}

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

   this->controlFlowGraphWindow = this->InitializeToolWindow(
      "ControlFlowGraph.ControlFlowGraphControl",
      "Control Flow Graph Tool Window",
      "{1CF3599E-A091-410E-A355-51E7820134FE}",
      hostedControlObject);
   this->controlFlowGraphControl =
      (ControlFlowGraphControl ^) hostedControlObject;

   // And now show the window.  Window must be set to visible before you
   // can do anything else with it

   this->controlFlowGraphWindow->Visible = true;

   // The GUID is used by Visual Studio to keep track of the properties of a 
   // tool window, such as size and position, across instances of the IDE.  
   // The GUID should be unique.  Generate a new one for each tool window you 
   // add to the IDE using a tool such as guidgen.exe

   this->irWindow = this->InitializeToolWindow(
      "ControlFlowGraph.IRWindowControl",
      "Phoenix IR Window",
      "{35FFD38E-A004-45c6-B6B6-3879AE554709}",
      hostedControlObject);
   this->irControl = (IRWindowControl ^) hostedControlObject;

   // Now call initialize on our control flow graph window.  We need
   // to also pass it the ir window and user control instances, so that it
   // can show and hide these when responding to events.

   this->controlFlowGraphControl->Initialize(this->applicationObject,
      this->irWindow, this->irControl);
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

//------------------------------------------------------------------------------
//
// Description:
//
//    Hooks the events the add-in needs to consume
//
// Remarks:
//
//    Adds a hook for the solution closing event.
//
//------------------------------------------------------------------------------

void
Connect::InitializeEvents()
{
   // First, grab the instances of the event handler references

   this->solutionEvents = this->applicationObject->Events->SolutionEvents;

   // Add one for the solution close event.

   this->solutionEvents->BeforeClosing +=
      gcnew EnvDTE::_dispSolutionEvents_BeforeClosingEventHandler(this,
         &Connect::SolutionClosing);

   // TODO: Add your event hooks from the IDE here.
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Helper method that retrieves the Phoenix flow graph for a 
//    CodeFunction.
//
// Arguments:
//
//    The function that should be loaded into the tool window.
//
// Returns:
//
//    The Phoenix flow graph for the function
//
//------------------------------------------------------------------------------

Phx::Graphs::FlowGraph ^
Connect::GetFlowGraph
(
   EnvDTE::CodeFunction^ function
)
{

   // If the function does not exist in the project system,
   // attempting to query the project system for the binary will fail,
   // and we can't display the flow graph, so just return null

   if ((function == nullptr) || (function->ProjectItem == nullptr))
   {
      return nullptr;
   }

   // Query the project system to figure out the pdb and executable that
   // are generated.

   Project ^ proj = function->ProjectItem->ContainingProject;

   // Get the current configuration.

   EnvDTE::Configuration ^ config =
      proj->ConfigurationManager->ActiveConfiguration;

   // Get the set of output groups for this configuration.

   OutputGroups ^ groups = config->OutputGroups;

   // Get the target path for the project.  We will prefix any output
   // files with this path.
   
   EnvDTE::Property ^ property = config->Properties->Item("OutputPath");

#if !defined(PHX_BETA2_BUILD)
   Object ^ valueObject = property->default;
#else
   Object ^ valueObject = property->Value;
#endif

   String ^ targetPath = (String ^) valueObject;

   String ^ binaryFile, ^pdbFile;

   // We find the binary name in the "Built" output group

   OutputGroup ^ binGroup = groups->Item("Built");
   if (binGroup->FileCount > 0)
   {
      Array ^ fileNames = (Array^)binGroup->FileNames;

      // grab the first output file.

      binaryFile = targetPath +
         fileNames->GetValue(0);
   }

   // The pdb is located in the "Symbols" output gruop

   OutputGroup ^ symGroup = groups->Item("Symbols");
   if (symGroup->FileCount > 0)
   {
      Array ^ fileNames = (Array ^) symGroup->FileNames;

      // grab the first output file.

      pdbFile = targetPath + fileNames->GetValue(0);
   }

   // Set the caption for the flow graph window to show that the flow
   // graph is being generated.

#if !defined(PHX_BETA2_BUILD)
   this->controlFlowGraphWindow->default = "Generating...";
#else
   this->controlFlowGraphWindow->Caption = "Generating...";
#endif

   // Get the flow graph from phoenix

   Phx::Graphs::FlowGraph ^ graph =
      this->phoenixProvider->GetFlowGraph(binaryFile, pdbFile,
         function->ProjectItem->FileNames[0], function->StartPoint->Line);

   return graph;
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Displays the flow graph for a given function.
//
// Arguments:
//
//    The function that should be loaded into the tool window.
//
//------------------------------------------------------------------------------

void
Connect::DisplayFlowGraph
(
   CodeFunction ^ function
)
{

   // First attempt to get the flow graph, using the helper method
   // GetFlowGraph(...)

   Phx::Graphs::FlowGraph ^ flowGraph = this->GetFlowGraph(function);

   if (!flowGraph)
   {
      // Could not retrieve the flow graph, update the flow graph as such

#if !defined(PHX_BETA2_BUILD)
      this->controlFlowGraphWindow->default = "Error loading flow graph";
#else
      this->controlFlowGraphWindow->Caption = "Error loading flow graph";
#endif
   }
   else
   {
      // The tool window title should be the prototype of the function
      // that is being graphed.

      int prototypeFormat = (int)(vsCMPrototype::vsCMPrototypeFullname +
         vsCMPrototype::vsCMPrototypeParamNames +
         vsCMPrototype::vsCMPrototypeParamTypes +
         vsCMPrototype::vsCMPrototypeType);
#if !defined(PHX_BETA2_BUILD)
      this->controlFlowGraphWindow->default =
         function->Prototype[prototypeFormat];
#else
      this->controlFlowGraphWindow->Caption =
         function->Prototype[prototypeFormat];
#endif
   }

   // Update the flow graph tool window with the new flow graph.

   this->controlFlowGraphControl->UpdateFlowGraph(flowGraph);

   // Activate (show) the window.

   this->controlFlowGraphWindow->Activate();

   // Repaint it.

   this->controlFlowGraphControl->Refresh();
}

//------------------------------------------------------------------------------
//
// Description:
//
//    Event handler for the BeforeClosing solution event.
//
// Remarks:
//
//    This event handler is used to clear the flow graph and IR windows
//    when the solution is closed.
//
//------------------------------------------------------------------------------

void
Connect::SolutionClosing()
{
   // Refresh the control flow graph window with nothing.

#if !defined(PHX_BETA2_BUILD)
   this->controlFlowGraphWindow->default =
      "Control Flow Graph Window";
#else
   this->controlFlowGraphWindow->Caption =
      "Control Flow Graph Window";
#endif
   this->controlFlowGraphControl->UpdateFlowGraph(nullptr);

   // Repaint the window.

   this->controlFlowGraphControl->Refresh();
}

} // ControlFlowGraph
