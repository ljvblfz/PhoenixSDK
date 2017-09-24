VS Addins get in a bad state very easily in development. Here are some tips to
ease your pain.

I.
	Don't have the addin load at startup. This will cause a problem because the devenv.exe instance
you use to debug with will load the addin dll and break your build. Instead, after starting a new
instance of devenv.exe under the debugger, load the addin via "Tools->Add-in Manager".

II.
	Use macros to clean things up.

Load the macros:
1. View->Other Windows->Macro Explorer
2. Right-click on "Macros" and select "Load Macro Project..." and give it the path to ControlFlowGraph.vsmacros
3. Expand macros, right-click on CleanRegKey and select "Edit"
4. Update the key value name with the path to your Control-Flow-Graph-To-Deploy.addin file (see comment)
5. Run macro "AllCleanUp" between debugger invocations

Notes:
Due to changes in Visual Studio 2005, the Control-flow-graph addin only works on C++ projects.
Also, it only works in ClassView. It is no longer supported by Object Browser.