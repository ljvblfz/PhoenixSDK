using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.IO;

namespace PEExplorer
{
   /// <summary>
   /// Holds global data that is valid throughout the lifetime
   /// of the application.
   /// </summary>
   static class GlobalData
   {
      // Global phase configuration.
      private static Phx.Phases.PhaseConfiguration phaseConfiguration;

      // File path to function assembly listings.
      private static string listingFileName;
     
      /// <summary>
      /// Custom function unit states. The values of this enumeration
      /// extend Phx.IR.State.
      /// </summary>
      enum CustomIRStates
      {
         Source = 16,
         IL = 32,
      }

      // A FunctionUnitState for source file listing.
      static Phx.FunctionUnitState sourceFunctionUnitState;

      // A FunctionUnitState for IL listing.
      static Phx.FunctionUnitState ilFunctionUnitState;

      /// <summary>
      /// Gets the FunctionUnitState for source file listings.
      /// </summary>
      public static Phx.FunctionUnitState SourceFunctionUnitState
      {
         get { return GlobalData.sourceFunctionUnitState; }
      }

      /// <summary>
      /// Gets the FunctionUnitState for IL listings.
      /// </summary>
      public static Phx.FunctionUnitState IlFunctionUnitState
      {
         get { return GlobalData.ilFunctionUnitState; }
      }
      
      /// <summary>
      /// Gets the global phase configuration.
      /// </summary>
      internal static Phx.Phases.PhaseConfiguration PhaseConfiguration
      {
         get { return GlobalData.phaseConfiguration; }
      }
      
      /// <summary>
      /// Gets the file path to function assembly listings.
      /// </summary>
      internal static string ListingFileName
      {
         get { return GlobalData.listingFileName; }
      }
      
      /// <summary>
      /// Initializes the Phoenix framework.
      /// </summary>
      internal static void InitializePhoenix(string[] arguments)
      {
         Phx.Targets.Architectures.Architecture architecture;
         Phx.Targets.Runtimes.Runtime runtime;

         // Register x86 architecture and runtime.
         architecture = Phx.Targets.Architectures.X86.Architecture.New();
         runtime = 
            Phx.Targets.Runtimes.Vccrt.Win32.X86.Runtime.New(architecture);
         Phx.GlobalData.RegisterTargetArchitecture(architecture);
         Phx.GlobalData.RegisterTargetRuntime(runtime);

         // Register Msil architecture and runtime.
         architecture = Phx.Targets.Architectures.Msil.Architecture.New();
         runtime = 
            Phx.Targets.Runtimes.Vccrt.Win.Msil.Runtime.New(architecture);
         Phx.GlobalData.RegisterTargetArchitecture(architecture);
         Phx.GlobalData.RegisterTargetRuntime(runtime);

         Phx.Initialize.BeginInitialization();

         // Register custom controls for the program.

         RegisterControls();

         // Allow the framework to process any user-provided controls.
         
         Phx.Initialize.EndInitialization("PHX|*|_PHX_", arguments);
         
         // Enable the -listingfilename control so we can display
         // function assembly listings.

         GlobalData.listingFileName = Path.GetRandomFileName();
         string argument = string.Concat("-listingfilename:",
            GlobalData.listingFileName);         
         Phx.Controls.Parser.ParseArgumentString(null, argument);

         // Create the global phase configuration.
         GlobalData.phaseConfiguration = Phx.Phases.PhaseConfiguration.New(
            Phx.Lifetime.New(Phx.LifetimeKind.Global, null),
            "PE-Explorer Phases");
            
         Phx.GlobalData.BuildPlugInPhases(phaseConfiguration);

         // We use the following FunctionUnitState objects as tags for 
         // 'raising to source' and 'raising to IL'. 
         // We do not pass this structure to the framework.
         GlobalData.sourceFunctionUnitState.IRState = 
            (Phx.IR.State)CustomIRStates.Source;
         GlobalData.ilFunctionUnitState.IRState =
            (Phx.IR.State)CustomIRStates.IL;         
      }

      /// <summary>
      /// Disposes of global objects.
      /// </summary>
      internal static void Shutdown()
      {
         // Delete the file that the framework used to write
         // assembly listings.
         File.Delete(GlobalData.listingFileName);
      }

      /// <summary>
      /// Registers custom controls used by the application.
      /// </summary>
      private static void RegisterControls()
      {
         // In this sample, we do not need to register any custom controls.
         
         // TODO: If you extend this sample and want to add custom
         // controls, this is a good place to do so.
      }      
   }
}
