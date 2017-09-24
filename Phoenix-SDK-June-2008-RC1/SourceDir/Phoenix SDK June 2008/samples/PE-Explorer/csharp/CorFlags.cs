using System;   

namespace PEExplorer
{  
   static class CorFlags
   {
      /// <summary>
      /// COM+ Header entry point flags.
      /// Taken from corhdr.h.
      /// </summary>
      [FlagsAttribute]
      public enum CorImageFlags
      {
         COMIMAGE_FLAGS_ILONLY = 0x00000001,
         COMIMAGE_FLAGS_32BITREQUIRED = 0x00000002,
         COMIMAGE_FLAGS_IL_LIBRARY = 0x00000004,
         COMIMAGE_FLAGS_STRONGNAMESIGNED = 0x00000008,
         COMIMAGE_FLAGS_NATIVE_ENTRYPOINT = 0x00000010,
         COMIMAGE_FLAGS_TRACKDEBUGDATA = 0x00010000,
      }

      /// <summary>
      /// Converts the provided CorImageFlags value to a
      /// string representation.
      /// </summary>
      public static string ParseCorImageFlags(CorImageFlags corFlags)
      {
         string flags = string.Empty;
         
         if ((corFlags & CorImageFlags.COMIMAGE_FLAGS_ILONLY) == 
            CorImageFlags.COMIMAGE_FLAGS_ILONLY)
         {
            flags = string.Concat(flags, "ILONLY ");
         }   
         if ((corFlags & CorImageFlags.COMIMAGE_FLAGS_32BITREQUIRED) == 
            CorImageFlags.COMIMAGE_FLAGS_32BITREQUIRED)
         {
            flags = string.Concat(flags, "32BITREQUIRED ");
         }
         if ((corFlags & CorImageFlags.COMIMAGE_FLAGS_IL_LIBRARY) == 
            CorImageFlags.COMIMAGE_FLAGS_IL_LIBRARY)
         {
            flags = string.Concat(flags, "IL_LIBRARY ");
         }
         if ((corFlags & CorImageFlags.COMIMAGE_FLAGS_STRONGNAMESIGNED) == 
            CorImageFlags.COMIMAGE_FLAGS_STRONGNAMESIGNED)
         {
            flags = string.Concat(flags, "STRONGNAMESIGNED ");
         }
         if ((corFlags & CorImageFlags.COMIMAGE_FLAGS_NATIVE_ENTRYPOINT) == 
            CorImageFlags.COMIMAGE_FLAGS_NATIVE_ENTRYPOINT)
         {
            flags = string.Concat(flags, "NATIVE_ENTRYPOINT ");
         }
         if ((corFlags & CorImageFlags.COMIMAGE_FLAGS_TRACKDEBUGDATA) == 
            CorImageFlags.COMIMAGE_FLAGS_TRACKDEBUGDATA)
         {
            flags = string.Concat(flags, "TRACKDEBUGDATA ");
         }
         
         return flags.Trim();
      }

      /// <summary>
      /// MethodImpl attribute bits.
      /// Taken from corhdr.h.
      /// </summary>
      [FlagsAttribute]
      public enum CorMethodImpl : uint
      {           
         miCodeTypeMask      =   0x0003,
         miIL                =   0x0000,
         miNative            =   0x0001,
         miOPTIL             =   0x0002,
         miRuntime           =   0x0003,

         miManagedMask       =   0x0004,
         miUnmanaged         =   0x0004,
         miManaged           =   0x0000,

         miForwardRef        =   0x0010,
         miPreserveSig       =   0x0080,

         miInternalCall      =   0x1000,
         miSynchronized      =   0x0020,
         miNoInlining        =   0x0008,
         miMaxMethodImplVal  =   0xffff
      }

      public static bool IsMiIL(CorMethodImpl x)
      {
         return (x & CorMethodImpl.miCodeTypeMask) == CorMethodImpl.miIL;
      }
      public static bool IsMiNative(CorMethodImpl x)
      {
         return (x & CorMethodImpl.miCodeTypeMask) == CorMethodImpl.miNative;
      }
      public static bool IsMiOPTIL(CorMethodImpl x)
      {
         return (x & CorMethodImpl.miCodeTypeMask) == CorMethodImpl.miOPTIL;
                              }
      public static bool IsMiRuntime(CorMethodImpl x)
      {
         return (x & CorMethodImpl.miCodeTypeMask) == CorMethodImpl.miRuntime;
      }
      public static bool IsMiUnmanaged(CorMethodImpl x)
      {
         return (x & CorMethodImpl.miManagedMask) == CorMethodImpl.miUnmanaged;
      }
      public static bool IsMiManaged(CorMethodImpl x)
      {
         return (x & CorMethodImpl.miManagedMask) == CorMethodImpl.miManaged;
      }

      public static bool IsMiForwardRef(CorMethodImpl x)
      {
         return (x & CorMethodImpl.miForwardRef) != 0;
      }
      public static bool IsMiPreserveSig(CorMethodImpl x)
      {
         return (x & CorMethodImpl.miPreserveSig) != 0;
      }

      public static bool IsMiInternalCall(CorMethodImpl x)
      {
         return (x & CorMethodImpl.miInternalCall) != 0;
      }

      public static bool IsMiSynchronized(CorMethodImpl x)
      {
         return (x & CorMethodImpl.miSynchronized) != 0;
      }
      public static bool IsMiNoInlining(CorMethodImpl x)
      {
         return (x & CorMethodImpl.miNoInlining) != 0;
      }

      /// <summary>
      /// Converts the provided CorMethodImpl value to a
      /// string representation.
      /// </summary>
      public static string GetCorMethodImplFlags(CorMethodImpl flags)
      {
         string s = string.Empty;
         if (IsMiIL(flags))
            s = string.Concat(s, " cil");
         if (IsMiNative(flags))
            s = string.Concat(s, " native");
         if (IsMiOPTIL(flags))
            s = string.Concat(s, " optil");
         if (IsMiRuntime(flags))
            s = string.Concat(s, " runtime");
         if (IsMiUnmanaged(flags))
            s = string.Concat(s, " unmanaged");
         if (IsMiManaged(flags))
            s = string.Concat(s, " managed");
         if (IsMiForwardRef(flags))
            s = string.Concat(s, " forwardref");
         if (IsMiPreserveSig(flags))
            s = string.Concat(s, " preservesig");
         if (IsMiInternalCall(flags))
            s = string.Concat(s, " internalcall");
         if (IsMiSynchronized(flags))
            s = string.Concat(s, " synchronized");
         if (IsMiNoInlining(flags))
            s = string.Concat(s, " noinlining");
            
         return s.Trim();
      }

      /// <summary>
      /// PE subsystem flags.
      /// </summary>
      public enum Subsystem : ushort
      {
         IMAGE_SUBSYSTEM_UNKNOWN = 0, 
         // Unknown subsystem. 
         IMAGE_SUBSYSTEM_NATIVE = 1, 
         // No subsystem required (device drivers and native system processes).
         IMAGE_SUBSYSTEM_WINDOWS_GUI = 2, 
         // Windows graphical user interface (GUI) subsystem. 
         IMAGE_SUBSYSTEM_WINDOWS_CUI = 3, 
         // Windows character-mode user interface (CUI) subsystem. 
         IMAGE_SUBSYSTEM_OS2_CUI = 5, 
         // OS/2 CUI subsystem. 
         IMAGE_SUBSYSTEM_POSIX_CUI = 7, 
         // POSIX CUI subsystem. 
         IMAGE_SUBSYSTEM_WINDOWS_CE_GUI = 9, 
         // Windows CE system. 
         IMAGE_SUBSYSTEM_EFI_APPLICATION = 10, 
         // Extensible Firmware Interface (EFI) application. 
         IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER = 11, 
         // EFI driver with boot services. 
         IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER = 12, 
         // EFI driver with run-time services. 
         IMAGE_SUBSYSTEM_EFI_ROM = 13, 
         // EFI ROM image. 
         IMAGE_SUBSYSTEM_XBOX = 14, 
         // Xbox system. 
         IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION = 16, 
         // Boot application. 
      }

      /// <summary>
      /// Converts the provided Subsystem value to a
      /// string representation.
      /// </summary>
      public static object FormatSubsystem(Subsystem subsystem)
      {
         // Skip leading "IMAGE_SUBSYSTEM_".
         return subsystem.ToString().Substring(16);
      }
   }
}