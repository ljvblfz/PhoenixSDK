//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Pereader plug-in for detecting parent's dependencies on children in a
//    namespace hierarchy.
//
// Usage:
//
//    pereader /callgraph /plugin [PATH TO PLUGIN] [PATH TO BINARY]
//
//-----------------------------------------------------------------------------

namespace DowncallDetectorPlugin
{
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    DowncallDetector class object and definitions.
   //
   // Remarks:
   //
   //    This class inherits from phase and as such is intended to run as a
   //    phase just after the call graph edges are created.
   //
   //--------------------------------------------------------------------------

   public class 
   DowncallDetector : Phx.Phases.Phase
   {
      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    This ComponentControl is required to register this dll as a plugin
      //    to run as a Phase.
      //
      //-----------------------------------------------------------------------

      private Phx.Controls.ComponentControl samplePluginControl;

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Constructs a new DowncallDetector object and registers it as a
      //    plugin.
      //
      // Remarks:
      //
      //    This functions acts as a constructor and serves to register the
      //    component as a plugin and register the warning that it will signal
      //    errors with.
      //
      // Returns:
      //
      //    A new DowncallDetector object.
      //
      //-----------------------------------------------------------------------

      public static DowncallDetector 
      New()
      {
         // Create the new object.
         DowncallDetector newPhase = new DowncallDetector();
         
         // Register the object as a plugin.
         newPhase.samplePluginControl =
            Phx.Controls.ComponentControl.New("DowncallDetectorPlugin",
           "downcall detector plug-in", "dd-plug-in");

         // Register the warning.
         Phx.GlobalData.WarningManager.AddWarning(
            DowncallDetectorDiagnosticInfo.New(
               "Dependency violates namespace hierarchy.", 999));

         return newPhase;
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Inserts the phase after call graph edges are added.
      //
      // Remarks:
      //
      //    This function will fail if the call graph is not created. The
      //    /callgraph option must be specified at the command line for
      //    pereader in order for the necessary phase to exist.
      //
      // Parameters:
      //
      //    config - Used to look up the phase object and insert the phase.
      //
      // Returns:
      //
      //    true - Indicates success of Setup.
      //
      //-----------------------------------------------------------------------

      public bool 
      Setup
      (
         Phx.Phases.PhaseConfiguration configuration
      )
      {
         Phx.Phases.Phase usedPhase;

         usedPhase = configuration.PhaseList.FindByName("Add Call Graph Edges");

#if (PHX_DEBUG_CHECKS)
         Phx.Asserts.Assert(usedPhase != null, "Phase not found.");
#endif
         this.Initialize(configuration, "DowncallDetectorPhase");
         usedPhase.InsertAfter(this);
         this.PhaseControl = samplePluginControl;

         return true;
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Detects and signals warnings.
      //
      // Remarks:
      //
      //    This function processes each function unit and checks each 
      //    successor call edge to see if a called function should signal
      //    a warning.
      //
      // Parameters:
      //
      //    unit - The function unit to process.
      //
      //-----------------------------------------------------------------------

      protected override void 
      Execute
      (
         Phx.Unit unit
      )
      {
         if (unit.IsFunctionUnit)
         {
            // Only process function units.
            Phx.Symbols.FunctionSymbol currFuncSym =
               unit.AsFunctionUnit.FunctionSymbol;
#if (PHX_DEBUG_CHECKS)
            Phx.Asserts.Assert(currFuncSym != null);
#endif
            string currFuncFQN =
               PhxUtilities.Utilities.BuildFullyQualifiedName(currFuncSym);

            string currFuncNS =
               PhxUtilities.Utilities.GetNamespaceFromFunctionSymbol(currFuncSym);
#if (PHX_DEBUG_CHECKS)
            Phx.Asserts.Assert(currFuncSym.CallNode != null);
#endif
            // Iterate over all successor call edges of the function.
            foreach (Phx.Graphs.CallEdge callEdge in currFuncSym.CallNode.SuccessorEdges)
            {
               Phx.Graphs.CallNode succNode =
                  callEdge.SuccessorNode.AsCallNode;

               string succFQN =
                  PhxUtilities.Utilities.BuildFullyQualifiedName(
                     succNode.FunctionSymbol);

               string succNS =
                  PhxUtilities.Utilities.GetNamespaceFromFunctionSymbol(
                     succNode.FunctionSymbol);

               Phx.IR.CallInstruction callSiteInstruction =
                  GetCallInstruction(unit.AsFunctionUnit, callEdge.CallSiteReferenceList);

               // Determine whether dependency is invalid.
               bool invalidDependency = InvalidDependency(
                  callEdge.CallSiteReferenceList, 
                  callSiteInstruction, currFuncSym,
                  succNode.FunctionSymbol, currFuncFQN, currFuncNS,
                  succFQN, succNS);

               if (invalidDependency)
               {
                  // Build the error string.
                  System.String errDescription =
                     "Downward call in namespace hierarchy from " +
                     currFuncFQN + " in " + currFuncNS +
                     " to " + succFQN + " in " + succNS;

                  // Signal the warning.
                  Phx.Logging.Diagnostics.DiagnosticMessage.LogWarning(
                        Phx.GlobalData.WarningManager.FindWarningByNumber(999),
                        callSiteInstruction.GetSourceContext(),
                        errDescription);
               }
            }
         }
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Determine the call instruction for the specified callSiteReference.
      //
      // Parameters:
      //
      //    functionUnit - the containing functionUnit
      //    callSiteReference - the object whose call instruction we want.
      //
      // Returns:
      //
      //    The call instruction of the call-site.
      //
      //-----------------------------------------------------------------------

      private Phx.IR.CallInstruction
      GetCallInstruction
      (
         Phx.FunctionUnit functionUnit,
         Phx.Graphs.CallSiteReference callSiteReference
      )
      {
         foreach (Phx.IR.Instruction instruction in functionUnit.Instructions)
         {
            if (instruction.IsCallInstruction 
               && (instruction.AsCallInstruction.CallSiteReference == callSiteReference))
            {
               return instruction.AsCallInstruction;
            }
         }

         // Couldn't find the call instruction

         return null;
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Determine whether the dependency is invalid.
      //
      // Parameters:
      //
      //    callSiteReference - Links to the instruction where the call occurs.
      //    callSiteInstruction - the call instruction for the call-site
      //    currFuncSym - The current function symbol.
      //    succFuncSym - The successor function symbol.
      //    currFuncFQN - The fully qualified name of the current function.
      //    currFuncNS - The namespace of the current function.
      //    succFuncFQN - The fully qualified name of the successor function.
      //    succFuncNS - The namespace of the successor function.
      //
      // Returns:
      //
      //    Boolean indicating whether the dependency is invalid.
      //
      //-----------------------------------------------------------------------

      private bool 
      InvalidDependency
      (
         Phx.Graphs.CallSiteReference callSiteReference,
         Phx.IR.Instruction callSiteInstruction,
         Phx.Symbols.FunctionSymbol currentFunctionSymbol,
         Phx.Symbols.FunctionSymbol successorFunctionSymbol,
         string currentFullyQualifiedName,
         string currentFunctionNamespace,
         string successorFullyQualifiedName,
         string successorFunctionNamespace
      )
      {
         //--------------------------------------------------------------------
         //
         // Remarks:
         //
         //    Ignore cases where call site information in not available.
         //
         //--------------------------------------------------------------------

         if (callSiteReference == null
            || callSiteInstruction == null
            || callSiteInstruction.DebugTag == 0)

         {
            return false;
         }

         //--------------------------------------------------------------------
         //
         // Remarks:
         //
         //    Ignore cases where the function is virtual
         //
         //--------------------------------------------------------------------

         if (successorFunctionSymbol.IsVirtual)
         {
            return false;
         }

         //--------------------------------------------------------------------
         //
         // Remarks:
         //
         //    Ignore cases where the namespace strings contain mangled chars.
         //
         //--------------------------------------------------------------------

         if (currentFunctionNamespace == ""
            || currentFunctionNamespace.Contains("<")
            || currentFunctionNamespace.Contains(">")
            || currentFunctionNamespace.Contains("?")
            || successorFunctionNamespace == ""
            || successorFunctionNamespace.Contains("<")
            || successorFunctionNamespace.Contains(">")
            || successorFunctionNamespace.Contains("?"))
         {
            return false;
         }

         //--------------------------------------------------------------------
         //
         // Remarks:
         //
         //    Ignore cases where the namespace is the same.
         //
         //--------------------------------------------------------------------

         if (currentFunctionNamespace.Equals(successorFunctionNamespace)
            || !successorFunctionNamespace.StartsWith(currentFunctionNamespace))
         {
            return false;
         }

         return true;
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    This is the warning class needed to signal warnings.
   //
   //--------------------------------------------------------------------------

   public class
   DowncallDetectorDiagnosticInfo : Phx.Logging.Diagnostics.DiagnosticInfo
   {
      //--------------------------------------------------------------------------
      //
      // Description:
      //
      //    These private member variables are the name and number of the warning.
      //
      //--------------------------------------------------------------------------

      private string name;
      private ushort number;

      //--------------------------------------------------------------------------
      //
      // Description:
      //
      //    Accessor functions for private data.
      //
      //--------------------------------------------------------------------------

      public override string 
      Name
      {
         get
         {
            return name;
         }
      }

      public override ushort 
      Number
      {
         get
         {
            return number;
         }
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Constructs a new warning object.
      //
      // Parameters:
      //
      //    name - Name of the warning.
      //    number - Number of the warning.
      //
      // Returns:
      //
      //    Newly constructed object.
      //
      //-----------------------------------------------------------------------

      public static 
      DowncallDetectorDiagnosticInfo 
      New
      (
         string name, 
         ushort number
      )
      {
         DowncallDetectorDiagnosticInfo info;
         info = new DowncallDetectorDiagnosticInfo();

         info.name = name;
         info.number = number;

         return info;
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    This is the plug-in driver class for the DowncallDetector class.
   //
   //--------------------------------------------------------------------------

   public class 
   PlugInDriver : Phx.PlugIn
   {
      static DowncallDetector mySamplePhase;

      public override void 
      RegisterObjects()
      {
         mySamplePhase = DowncallDetector.New();
      }

      public override void
      BuildPhases
      (
         Phx.Phases.PhaseConfiguration config
      )
      {
         mySamplePhase.Setup(config);
      }

      public override System.String 
      NameString
      {
         get
         {
            return "DowncallDetectorPhase";
         }
      }
   }
}
