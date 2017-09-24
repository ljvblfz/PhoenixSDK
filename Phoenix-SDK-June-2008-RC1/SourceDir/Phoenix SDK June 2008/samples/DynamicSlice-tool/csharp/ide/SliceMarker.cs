using System;
using System.Collections.Generic;
using System.Text;

using Microsoft.VisualStudio.TextManager.Interop;
using Microsoft.VisualStudio.OLE.Interop;
using Microsoft.VisualStudio.Shell;

namespace SliceIDE
{
   // There are two custom marker types
   //   line marker - marks the source lines which make up the slice
   //   variable marker - marks the variable that was sliced on

   // This is the marker service, it provides the marker types to VS
   // It also has methods for creating and removing markers
   [PackageRegistration(UseManagedResourcesOnly = true)]
   [System.Runtime.InteropServices.Guid(
       GuidList.guidSliceVSIPMarkerServiceString
   )]
   public class SliceMarkerService : IVsTextMarkerTypeProvider
   {
      IVsPackageDefinedTextMarkerType marker;
      IVsPackageDefinedTextMarkerType vmarker;
      Package myPackage;

      public SliceMarkerService(Package p)
      {
         // instantiate the marker types
         // so we are ready when VS asks for them
         marker = new SliceMarker();
         vmarker = new SliceVMarker();
         myPackage = p;
      }

      // called by VS to get the marker type given a marker GUID
      public int GetTextMarkerType(
         ref Guid pguidMarker,
         out IVsPackageDefinedTextMarkerType ppMarkerType)
      {
         // check which marker VS is asking for
         // return the marker type in the out parameter
         if (pguidMarker == GuidList.guidSliceVSIPMarker)
            ppMarkerType = marker;
         else if (pguidMarker == GuidList.guidSliceVSIPVMarker)
            ppMarkerType = vmarker;
         else
            ppMarkerType = null;
         return 0;
      }

      // Sets a line marker on an editor line
      // Only the package class can use GetService
      // so we pass in a VsTextManager from the call in SlicePackage
      public void MarkLine(IVsTextManager tm, int line)
      {
         IVsTextView ppView;
         IVsTextLines tl;
         int r, markerID, length;
         String text;

         // get the marker ID for the line marker
         Guid guid = GuidList.guidSliceVSIPMarker;
         r = tm.GetRegisteredMarkerTypeID(ref guid, out markerID);

         r = tm.GetActiveView(0, null, out ppView);
         r = ppView.GetBuffer(out tl);

         // the markers look better if we don't mark leading whitespace
         tl.GetLengthOfLine(line, out length);
         tl.GetLineText(line, 0, line, length, out text);
         char[] ws = { ' ', '\t' };
         String trimmedText = text.TrimStart(ws);

         r = tl.CreateLineMarker(
            markerID,
            line,
            text.Length - trimmedText.Length,
            line,
            length,
            null,
            null);
      }

      // Sets a variable marker
      public void MarkVar(IVsTextManager tm, int line, int start, int end)
      {
         IVsTextView ppView;
         IVsTextLines tl;
         int r, markerID;

         Guid guid = GuidList.guidSliceVSIPVMarker;
         r = tm.GetRegisteredMarkerTypeID(ref guid, out markerID);

         r = tm.GetActiveView(0, null, out ppView);
         r = ppView.GetBuffer(out tl);

         r = tl.CreateLineMarker(
            markerID,
            line,
            start,
            line,
            end,
            null,
            null);
      }

      // Clears all of the markers in the given view
      public void ClearMarks(IVsTextManager tm, IVsTextView vw)
      {
         int markerID;

         Guid guid = GuidList.guidSliceVSIPMarker;
         tm.GetRegisteredMarkerTypeID(ref guid, out markerID);
         ClearMarksByMarkerID(tm, vw, markerID);

         guid = GuidList.guidSliceVSIPVMarker;
         tm.GetRegisteredMarkerTypeID(ref guid, out markerID);
         ClearMarksByMarkerID(tm, vw, markerID);
      }

      // Clears all of the markers in the given view with the given marker ID
      private void ClearMarksByMarkerID(
         IVsTextManager tm, IVsTextView vw, int markerID)
      {
         IVsTextLines tl;
         IVsEnumLineMarkers enumMarkers;
         IVsTextLineMarker marker;

         vw.GetBuffer(out tl);

         tl.EnumMarkers(
            0, 0, 0, 0,
            markerID,
            (uint)ENUMMARKERFLAGS.EM_ENTIREBUFFER,
            out enumMarkers);

         while (enumMarkers.Next(out marker) == 0)
            marker.Invalidate();
      }
   }

   // This is the marker type for the line marker
   [System.Runtime.InteropServices.Guid(GuidList.guidSliceVSIPMarkerString)]
   public class SliceMarker :
      IVsPackageDefinedTextMarkerType, IVsMergeableUIItem
   {
      public int DrawGlyphWithColors(
         IntPtr hdc,
         RECT[] pRect,
         int iMarkerType,
         IVsTextMarkerColorSet pMarkerColors,
         uint dwGlyphDrawFlags,
         int iLineHeight)
      {
         return 0;
      }

      public int GetBehaviorFlags(out uint pdwFlags)
      {
         pdwFlags = (uint)MARKERBEHAVIORFLAGS.MB_DEFAULT;
         return 0;
      }

      public int GetDefaultColors(
         COLORINDEX[] piForeground,
         COLORINDEX[] piBackground)
      {
         piForeground[0] = COLORINDEX.CI_WHITE;
         piBackground[0] = COLORINDEX.CI_PURPLE;
         return 0;
      }

      public int GetDefaultFontFlags(out uint pdwFontFlags)
      {
         pdwFontFlags = (uint)FONTFLAGS.FF_BOLD;
         return 0;
      }

      public int GetDefaultLineStyle(
         COLORINDEX[] piLineColor,
         LINESTYLE[] piLineIndex)
      {
         piLineColor[0] = COLORINDEX.CI_GREEN;
         piLineIndex[0] = LINESTYLE.LI_SOLID;
         return 0;
      }

      public int GetPriorityIndex(out int piPriorityIndex)
      {
         piPriorityIndex = 10001;
         return 0;
      }

      public int GetVisualStyle(out uint pdwVisualFlags)
      {
         pdwVisualFlags = (uint)(MARKERVISUAL.MV_COLOR_ALWAYS);
         return 0;
      }

      public int GetCanonicalName(out string pbstrNonLocalizeName)
      {
         pbstrNonLocalizeName = "Slice Marker";
         return 0;
      }

      public int GetDescription(out string pbstrDesc)
      {
         pbstrDesc = "Slice Marker";
         return 0;
      }

      public int GetDisplayName(out string pbstrDisplayName)
      {
         pbstrDisplayName = "Slice Marker";
         return 0;
      }

      public int GetMergingPriority(out int piMergingPriority)
      {
         piMergingPriority = 0x400;
         return 0;
      }

   }

   // This is the marker type for the variable marker
   [System.Runtime.InteropServices.Guid(GuidList.guidSliceVSIPVMarkerString)]
   public class SliceVMarker :
       IVsPackageDefinedTextMarkerType, IVsMergeableUIItem
   {
      public int DrawGlyphWithColors(
         IntPtr hdc,
         RECT[] pRect,
         int iMarkerType,
         IVsTextMarkerColorSet pMarkerColors,
         uint dwGlyphDrawFlags,
         int iLineHeight)
      {
         return 0;
      }

      public int GetBehaviorFlags(out uint pdwFlags)
      {
         pdwFlags = (uint)MARKERBEHAVIORFLAGS.MB_DEFAULT;
         return 0;
      }

      public int GetDefaultColors(
         COLORINDEX[] piForeground,
         COLORINDEX[] piBackground)
      {
         piForeground[0] = COLORINDEX.CI_WHITE;
         piBackground[0] = COLORINDEX.CI_DARKGREEN;
         return 0;
      }

      public int GetDefaultFontFlags(out uint pdwFontFlags)
      {
         pdwFontFlags = (uint)FONTFLAGS.FF_BOLD;
         return 0;
      }

      public int GetDefaultLineStyle(
         COLORINDEX[] piLineColor,
         LINESTYLE[] piLineIndex)
      {
         piLineColor[0] = COLORINDEX.CI_GREEN;
         piLineIndex[0] = LINESTYLE.LI_SOLID;
         return 0;
      }

      public int GetPriorityIndex(out int piPriorityIndex)
      {
         piPriorityIndex = 10002;
         return 0;
      }

      public int GetVisualStyle(out uint pdwVisualFlags)
      {
         pdwVisualFlags = (uint)(MARKERVISUAL.MV_COLOR_ALWAYS);
         return 0;
      }

      public int GetCanonicalName(out string pbstrNonLocalizeName)
      {
         pbstrNonLocalizeName = "Slice Variable Marker";
         return 0;
      }

      public int GetDescription(out string pbstrDesc)
      {
         pbstrDesc = "Slice Variable Marker";
         return 0;
      }

      public int GetDisplayName(out string pbstrDisplayName)
      {
         pbstrDisplayName = "Slice Variable Marker";
         return 0;
      }

      public int GetMergingPriority(out int piMergingPriority)
      {
         piMergingPriority = 0x401;
         return 0;
      }

   }   
}
