
var wscShell = WScript.CreateObject("WScript.Shell");
try
{
   var installDir = wscShell.RegRead("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\VisualStudio\\VSIP\\8.0\\InstallDir");
}
catch(exc)
{
   // Not install. No need to worry uninterested user.
   WScript.Quit(1);
}
WScript.Echo("@set VsSdkInstallDir="+ installDir +"VisualStudioIntegration\\Tools\\Build\\");


