//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Static utility functions.
//
// Remarks:
//
//    Some of these functions exist because of logged bugs and are currently
//    workarounds.
//
// Usage:
//
//    This is compiled to a .dll which should be used by other tools.
//
//-----------------------------------------------------------------------------

namespace PhxUtilities
{
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Static class for static functions.
   //
   //--------------------------------------------------------------------------

   public static class 
   Utilities
   {
      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Build the fully qualified name of a function given its function
      //    symbol.
      //
      // Remarks:
      //
      //    This function peels off enclosing aggregate types and
      //    concatenates their names to build the fully qualified name.
      //    The parts of the name are delimited entirely by periods.
      //
      // Parameters:
      //
      //   myFunctionSymbol - Function symbol to build the fully qualified 
      //                      name for.
      //
      // Returns:
      //
      //    String which is the fully qualified name.
      //
      // Internal:
      //
      //    This function exists because of logged bug 275327 with title:
      //    "Fully qualified name of functions in nested classes lacks 
      //    information". Once this bug is fixed, this function will no longer
      //    be necessary. When that occurs this function may simply be replaced
      //    with "return myFunctionSymbol.QualifiedName"
      //
      //-----------------------------------------------------------------------

      public static string
      BuildFullyQualifiedName
      (
         Phx.Symbols.FunctionSymbol myFunctionSymbol
      )
      {
         // Return the empty string if pointers are null.
         if (myFunctionSymbol == null || myFunctionSymbol.NameString == null)
         {
            return "";
         }

         string fullQualName              = 
            myFunctionSymbol.NameString;
         Phx.Types.AggregateType aggType  = 
            myFunctionSymbol.EnclosingAggregateType;

         while (aggType != null)
         {
#if (PHX_DEBUG_CHECKS)
            Phx.Asserts.Assert(aggType.TypeSymbol != null);
            Phx.Asserts.Assert(aggType.TypeSymbol.NameString != null);
#endif
            fullQualName = aggType.TypeSymbol.NameString + "." + fullQualName;
            aggType = aggType.EnclosingAggregateType;
         }

         return fullQualName;
      }

      //-----------------------------------------------------------------------
      //
      // Description:
      //
      //    Get the namespace string for the function symbol.
      //
      // Remarks:
      //
      //    The namespace string is assumed to be the substring of the most
      //    enclosing aggregate type from the beginning of the string to the
      //    first period. Note that if a function is inherited by an
      //    aggregate type but not overriden then this function will return
      //    the namespace of where the function was defined.
      //
      // Parameters:
      //
      //    funcSym - Function symbol to get the namespace string for.
      //
      // Returns:
      //
      //    String which is the namespace for the function.
      //
      //-----------------------------------------------------------------------

      public static string
      GetNamespaceFromFunctionSymbol
      (
         Phx.Symbols.FunctionSymbol myFunctionSymbol
      )
      {
         if (myFunctionSymbol == null)
         {
            return "";
         }

         int i;
         string ns;
         Phx.Types.AggregateType at = myFunctionSymbol.EnclosingAggregateType;
         
         // Function symbol with no enclosing aggregate type case.
         if (at == null)
         {
            ns = BuildFullyQualifiedName(myFunctionSymbol);
            i = ns.LastIndexOf(".");

            if (i < 0)
            {
               return "";
            }
            else
            {
               return ns.Substring(0, i);
            }
         }
         
         // Find the inner-most enclosing aggregate type.
         while (at.EnclosingAggregateType != null)
         {
            at = at.EnclosingAggregateType;
         }

#if (PHX_DEBUG_CHECKS)
         Phx.Asserts.Assert(at.TypeSymbol != null);
         Phx.Asserts.Assert(at.TypeSymbol.NameString != null);
#endif
         ns = at.TypeSymbol.NameString;
         i = ns.LastIndexOf(".");

         // No namespace case.
         if (i < 0)
         {
            return "";
         }

         ns = ns.Substring(0, i);

         // Catch the generics case.
         if (ns.Contains("<") || ns.Contains(">"))
         {
            i = ns.LastIndexOf(".");

            if (i < 0)
            {
               return "";
            }

            ns = ns.Substring(0, i);
         }

         return ns;
      }
   }
}
