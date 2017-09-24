//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    FuncNames: a minimal Phoenix plug-in that reports the name of each 
//    function being processed by the host.  (For example, when hosted by the
//    C2 backend, reports the name of each function as it is compiled)
//
// Usage:
//
//    Begin Snippet run
//    cl -nologo -d2plugin:FuncNames.dll functions.cpp
//    End Snippet
//
//-----------------------------------------------------------------------------

using System;

public class FuncNames
{

//-----------------------------------------------------------------------------
//
// Description:
//
//    The FuncNames PlugIn class
//
// Remarks:
//
//    A minimal Phoenix plugin
//
//-----------------------------------------------------------------------------

// Begin Snippet funcnames

public class PlugIn : Phx.PlugIn

// End Snippet
{

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The RegisterObjects method is typically used to register any 
   //    controls for the current plug-in (such controls are used to parse
   //    the command-line supplied to the host, extracting information on
   //    behalf of the plug-in).  However, the current plug-in has no
   //    controls, so we supply an empty body for this method.
   //
   //--------------------------------------------------------------------------

   // Begin Snippet regobjs

   public override void RegisterObjects() { }

   // End Snippet

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The BuildPhases method locates the last phase in the host's phase 
   //    list.  For the case of C2, this is called "Encoding".  It then calls
   //    FuncNamesPhase.New to create a new FuncNamePhase object and insert 
   //    it as the host's penultimate phase.
   //
   //--------------------------------------------------------------------------

   // Begin Snippet buildphases

   public override void
   BuildPhases
   (
      Phx.Phases.PhaseConfiguration config
   )
   {
      Phx.Phases.Phase encodingPhase;
      Phx.Phases.Phase funcNamesPhase;

      encodingPhase = config.PhaseList.FindByName("Encoding");
      funcNamesPhase = Phase.New(config);
      encodingPhase.InsertBefore(funcNamesPhase);
   }

   // End Snippet

   public override System.String NameString
   {
      get
      {
         return "FuncNames";
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    The FuncNamesPhase class
//
// Remarks:
//
//    This phase supplies the implementation for two methods (defined by
//    its Phase base class): New and Execute
//
//-----------------------------------------------------------------------------

// Begin Snippet phase

public class Phase : Phx.Phases.Phase

// End Snippet
{

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The New method creates a new FuncNamesPhase object and inserts 
   //    it into the host's phase list, immediately before the existing
   //    phase 'laterPhase'.
   //
   //--------------------------------------------------------------------------

   // Begin Snippet new

   public static Phx.Phases.Phase
   New
   (
      Phx.Phases.PhaseConfiguration config
   )
   {
      Phase phase = new Phase();
      phase.Initialize(config, "FuncNames");
      return phase;
   }

   // End Snippet

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    The Execute method is called by the host for this plug-in.  When
   //    called, it simply reports, to the console, the name of the current
   //    function being processed.
   //
   //--------------------------------------------------------------------------

   // Begin Snippet exec

   protected override void
   Execute
   (
      Phx.Unit unit
   )
   {
      if (!unit.IsFunctionUnit)
      {
         return;
      }

      Phx.FunctionUnit function = unit.AsFunctionUnit;

      Phx.Output.WriteLine("Function: {0}", function.NameString);
   }

   // End Snippet
}
}