// Copyright (c) Microsoft Corporation. All rights reserved.

function OnFinish(selProj, selObj)
{
   try
   {
      var strProjectPath = wizard.FindSymbol("PROJECT_PATH");
      var strProjectName = wizard.FindSymbol("PROJECT_NAME");

      var proj = CreateManagedProject(strProjectName, strProjectPath);
      AddManagedConfigsForDLL(proj, strProjectName);
      AddReferencesForApp(proj);
      AddFilesToProjectWithInfFile(proj, strProjectName);
      AddSpecificConfig(proj, strProjectName);

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
   return false;
}

function GetTargetName(strName, strProjectName, strResPath, strHelpPath)
{
  var strTarget = strName;

  if (strName == "BackendPhase.cpp") 
  {
     strTarget = wizard.FindSymbol("SAFE_PROJECT_NAME")+".cpp";
  }
  
  if (strName == "BackendPhase.h") 
  {
     strTarget = wizard.FindSymbol("SAFE_PROJECT_NAME")+".h";
  }

  return strTarget; 
}

function AddSpecificConfig(proj, strProjectName)
{
    var oProjObject = proj.Object;
    oProjObject.AddAssemblyReference("phxd.dll");
    oProjObject.AddAssemblyReference("architecture-msild.dll");
    oProjObject.AddAssemblyReference("architecture-x86d.dll");
    oProjObject.AddAssemblyReference("runtime-vccrt-win-msild.dll");
    oProjObject.AddAssemblyReference("runtime-vccrt-win32-x86d.dll");

    var oConfigs = oProjObject.Configurations;
    for (var i = 1; i <= oConfigs.Count; i++)
    {
      var config = oConfigs(i);
      if (config.Name.indexOf("Debug") != -1)
      {
         var CLTool = config.Tools("VCCLCompilerTool");
         var strDefines = CLTool.PreprocessorDefinitions;
         if (strDefines != "")
         {
            strDefines += ";"
         }
         strDefines += "PHX_DEBUG_SUPPORT;PHX_DEBUG_DUMPS;PHX_DEBUG_CHECKS";
         CLTool.PreprocessorDefinitions = strDefines;
      }
      else if (config.Name.indexOf("Release") != -1)
      {
         var CLTool = config.Tools("VCCLCompilerTool");
         var strDefines = CLTool.PreprocessorDefinitions;
         if (strDefines != "")
         {
            strDefines += ";"
         }
         strDefines += "PHX_DEBUG_SUPPORT;PHX_DEBUG_DUMPS";
         CLTool.PreprocessorDefinitions = strDefines;
      }
    }
}
