// Guids.cs
// MUST match guids.h
using System;

namespace SliceIDE
{
    static class GuidList
    {
        public const string guidSliceVSIPPkgString = "69263478-82f1-41b1-8c6f-80ee87f8e1d3";
        public const string guidSliceVSIPCmdSetString = "b487b76c-5b65-45cd-8e9d-26c5fdb275df";
        public const string guidSliceVSIPMarkerString = "0AE7B14F-7154-4043-97EE-6B63791A2062";
        public const string guidSliceVSIPVMarkerString = "18800B6A-4585-445e-A32E-2AEDC585A06E";
        public const string guidSliceVSIPMarkerServiceString = "26C4D39A-B305-4adf-B32C-D651C0D8A302";

        public static readonly Guid guidSliceVSIPPkg = new Guid(guidSliceVSIPPkgString);
        public static readonly Guid guidSliceVSIPCmdSet = new Guid(guidSliceVSIPCmdSetString);
        public static readonly Guid guidSliceVSIPMarker = new Guid(guidSliceVSIPMarkerString);
        public static readonly Guid guidSliceVSIPVMarker = new Guid(guidSliceVSIPVMarkerString);
        public static readonly Guid guidSliceVSIPMarkerService = new Guid(guidSliceVSIPMarkerServiceString);
    };
}