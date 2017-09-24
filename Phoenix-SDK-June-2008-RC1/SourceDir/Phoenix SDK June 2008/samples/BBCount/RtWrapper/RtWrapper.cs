#region Using directives

using System;
using System.Runtime.InteropServices;

#endregion

//--------------------------------------------------------------------------
//
// Description:
//
//    A managed wrapper class of all the APIs that are exported by the 
//    profiling runtime. 

//    The profiling runtime is implemented as a native DLL so that we can 
//    initialize profiling data structures in the beginning and dump out 
//    the profiling data at the end of execution, using DLLMain.
//    
//    Right now, there is only one exported function
//--------------------------------------------------------------------------

public class RtWrapper
{
   [DllImport("ProfilerRt.dll")]
   public static extern void BBCount(uint Id);

}