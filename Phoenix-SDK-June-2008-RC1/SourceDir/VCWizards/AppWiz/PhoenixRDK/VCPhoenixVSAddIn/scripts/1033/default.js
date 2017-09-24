// Copyright (c) Microsoft Corporation. All rights reserved.

function OnFinish(selProj, selObj)
{
    try
    {
        //We need to add a GUID
        var strToolWindowGuid = wizard.CreateGuid();
        wizard.AddSymbol("TOOL_WINDOW_GUID", wizard.FormatGuid(strToolWindowGuid, eFormat.Format1));
    
        var strAxHostGuid = wizard.CreateGuid();
        wizard.AddSymbol("AXHOST_GUID", wizard.FormatGuid(strAxHostGuid, eFormat.Format1));
    
        // And add a symbol indicating whether we need named commands
        var addNamedCommands = 
            wizard.FindSymbol("TOOLS_MENU_ITEM") || 
            (wizard.FindSymbol("OTHER_WINDOWS_MENU_ITEM") && wizard.FindSymbol("TOOL_WINDOW")) ||
            (wizard.FindSymbol("TOOLBAR_ITEM") && wizard.FindSymbol("TOOL_WINDOW"));
    
        wizard.AddSymbol("ADD_NAMED_COMMANDS", addNamedCommands);
    
        var addShowWindowCommands = 
            wizard.FindSymbol("OTHER_WINDOWS_MENU_ITEM") ||
            (wizard.FindSymbol("TOOLBAR_ITEM") && wizard.FindSymbol("TOOL_WINDOW"));
        
        wizard.AddSymbol("ADD_SHOW_WINDOW_COMMANDS", addShowWindowCommands);
    
        //The last thing to do is to take the add-in text and replace all \r\n or \n's with
        //textual versions of \r\n, for the xml file
        strAboutBoxText = wizard.FindSymbol("ABOUT_BOX_TEXT");
    
        //Make sure we get both carriage return and newline replaced.
        while(strAboutBoxText.search("\r\n") != -1)
                strAboutBoxText = strAboutBoxText.replace("\r\n", "\\r\\n");
        while(strAboutBoxText.search("\n") != -1)
                strAboutBoxText = strAboutBoxText.replace("\n", "\\r\\n");
    
        wizard.AddSymbol("ABOUT_BOX_TEXT", strAboutBoxText);
                    
        var strProjectPath = wizard.FindSymbol("PROJECT_PATH");
        var strProjectName = wizard.FindSymbol("PROJECT_NAME");

        var proj = CreateManagedProject(strProjectName, strProjectPath);
        AddManagedConfigsForDLL(proj, strProjectName);
        AddReferencesForApp(proj);
        
        AddSpecificConfig(proj, strProjectName);
            
        AddFilesToProjectWithInfFile(proj, strProjectName);
        
        proj.Object.Save();
    }
    catch(e)
    {
        if (e.description.length != 0)
            SetErrorInfo(e);
        return e.number
    }
}

function SetFileProperties(projfile, strName)
{
    if(strName == "toolwindowcontrol.h")
    {
        projfile.Object.FileType = eFileTypeCppControl;
        return;
    }
}

function GetTargetName(strName, strProjectName, strResPath, strHelpPath)
{
    var shell = new ActiveXObject("WScript.Shell");
    var env = shell.Environment("process");
    var appdata = env("APPDATA");
    
    try
    {
        var strTarget = strName;
            
        switch(strName)
        {
            case "DeploymentLocalCopy.AddIn":
                strTarget = strProjectName + ".AddIn";
                break;
            case "DeploymentVSCopy.AddIn":
                strTarget = appdata + "\\Microsoft\\MSEnvShared\\Addins\\" + strProjectName + "- To Deploy.AddIn";
                break;
        }
        return strTarget; 
    }
    catch(e)
    {
        throw e;
    }
}

function AddSpecificConfig(proj, strProjectName)
{
    try
    {   
        var oProjObject = proj.Object;
        oProjObject.AddAssemblyReference("EnvDTE.dll");
        oProjObject.AddAssemblyReference("EnvDTE80.dll");
        oProjObject.AddAssemblyReference("extensibility.dll");
        oProjObject.AddAssemblyReference("Microsoft.VisualStudio.CommandBars.dll");
        oProjObject.AddAssemblyReference("stdole.dll");
        oProjObject.AddAssemblyReference("System.Drawing.dll");
        oProjObject.AddAssemblyReference("System.Windows.Forms.dll");
        
        oProjObject.AddAssemblyReference("phx.dll");
        oProjObject.AddAssemblyReference("architecture-msil.dll");
        oProjObject.AddAssemblyReference("architecture-x86.dll");
        oProjObject.AddAssemblyReference("runtime-vccrt-win-msil.dll");
        oProjObject.AddAssemblyReference("runtime-vccrt-win32-x86.dll");
        
        oProjObject.RootNamespace = wizard.FindSymbol("SAFE_NAMESPACE_NAME");
        
        var oConfigs = proj.Object.Configurations;
            for (var nCntr = 1; nCntr <= oConfigs.Count; nCntr++)
        {
                //The first thing to do is set up the Output directories
        //correctly for an add-in
            var config = oConfigs(nCntr);
            config.OutputDirectory = "$(ProjectDir)bin";
            
            config.DebugSettings.Command = "$(DevEnvDir)devenv.exe";	  
        }
    }
    catch(e)
    {
        throw e;
    }
}

