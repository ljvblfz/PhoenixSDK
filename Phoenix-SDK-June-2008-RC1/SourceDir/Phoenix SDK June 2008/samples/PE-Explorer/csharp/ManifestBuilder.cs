using System;
using System.Collections.Generic;
using System.Text;

namespace PEExplorer
{
   /// <summary>
   /// Builds manifest-related information about the executable.
   /// </summary>
   class ManifestBuilder
   {
      // External module references
      private List<Phx.Symbols.ImportModuleSymbol> externalModuleReferences;
      
      // Internal manifest references
      private List<Phx.Manifest> manifests;
      
      // External manifest references
      private List<Phx.Manifest> externalManifests;

      // Manifest resource symbols.
      private List<Phx.Symbols.ResourceSymbol> resourceSymbols;

      // Mapping of manifest objects to lists of custom 
      // attribute strings.
      private Dictionary<Phx.Manifest, List<string>> manifestAttributes;

      // Mapping of module unit objects to lists of custom 
      // attribute strings.
      private Dictionary<Phx.ModuleUnit, List<string>> customAttributes;
      
      // The module units associated with the assembly.
      private List<Phx.PEModuleUnit> moduleUnits;
      
      // The CLR version of the assembly.
      private string metadataVersion;
      
      // List of v-table fixup strings associated with the module.
      private List<string> vtableFixupStrings;

      // File references
      private List<Phx.Symbols.FileSymbol> fileReferences;

      /// <summary>
      /// Constructs a new ManifestBuilder object.
      /// </summary>
      internal ManifestBuilder()
      {
         this.externalModuleReferences = 
            new List<Phx.Symbols.ImportModuleSymbol>();
         this.manifests = new List<Phx.Manifest>();
         this.externalManifests = new List<Phx.Manifest>();
         this.resourceSymbols = new List<Phx.Symbols.ResourceSymbol>();
         this.moduleUnits = new List<Phx.PEModuleUnit>();
         this.fileReferences = new List<Phx.Symbols.FileSymbol>();
         this.vtableFixupStrings = new List<string>();
         this.customAttributes = 
            new Dictionary<Phx.ModuleUnit, List<string>>();
         this.manifestAttributes = 
            new Dictionary<Phx.Manifest, List<string>>();
      }
     
      /// <summary>
      /// Returns a string representation of all manifest-related
      /// information.
      /// </summary>
      public override string ToString()
      {
         StringBuilder stringBuilder = new StringBuilder();

         // First, append metadata version.         
         stringBuilder.AppendFormat("// Metadata version: {1}{0}",
            Environment.NewLine, this.MetadataVersion);
            
         // Next, append external module references.         
         foreach (Phx.Symbols.ImportModuleSymbol importModuleSymbol 
            in this.externalModuleReferences)
         {
            stringBuilder.AppendFormat(".module extern {0}", 
               importModuleSymbol.NameString);
            stringBuilder.AppendLine();
         }
         
         // Next, parse external assembly references.
         foreach (Phx.Manifest manifest in this.externalManifests)
         {
            this.ParseManifest(manifest, true, stringBuilder);
         }
         
         // Next, parse internal assembly references.
         foreach (Phx.Manifest manifest in this.manifests)
         {
            this.ParseManifest(manifest, false, stringBuilder);
         }

         // Parse file resources.
         foreach (Phx.Symbols.FileSymbol fileSymbol 
            in this.fileReferences)
         {
            this.ParseFileSymbol(fileSymbol, stringBuilder);
         }

         // Parse manifest resources.
         foreach (Phx.Symbols.ResourceSymbol resourceSymbol
            in this.resourceSymbols)
         {
            this.ParseResourceSymbol(resourceSymbol, stringBuilder);
         }
         
         // Parse module unit metadata.         
         foreach (Phx.PEModuleUnit moduleUnit in this.moduleUnits)
         {
            List<string> customAttributes = null;
            this.customAttributes.TryGetValue(moduleUnit, 
               out customAttributes);

            this.ParseModuleUnit(moduleUnit, customAttributes, stringBuilder);
         }
         
         // Finally, parse any v-table fixups.         
         foreach (string vtableFixup in this.vtableFixupStrings)
         {
            stringBuilder.AppendLine(vtableFixup);
         }         
         
         return stringBuilder.ToString();
      }

      /// <summary>
      /// Appends information related to the given file symbol
      /// to the provided string builder.
      /// </summary>
      private void ParseFileSymbol(Phx.Symbols.FileSymbol fileSymbol, 
         StringBuilder stringBuilder)      
      {
         stringBuilder.Append(".file ");

         // Append 'nometadata' token if the symbol is a resource 
         // file or other non-metadata-containing file.
         if (fileSymbol.ContainsNoMetadata)
         {
            stringBuilder.Append("nometadata ");
         }
         
         // Append name and newline.
         stringBuilder.AppendLine(fileSymbol.NameString);

         // Append both the raw hash key as well as its string
         // representation.

         byte[] hashKey = fileSymbol.HashKey;

         stringBuilder.AppendFormat("    .hash = ({0})   // {1}",
            Utility.FormatByteArray(hashKey),
            Utility.FormatByteArrayAsText(hashKey));


         stringBuilder.AppendLine();
      }

      /// <summary>
      /// Adds the given fixup string entry to the list of 
      /// v-table fixup strings.
      /// </summary>
      internal void AddVTableFixupString(string fixupString)
      {
         this.vtableFixupStrings.Add(fixupString);
      }

      /// <summary>
      /// Adds the given ModuleUnit object to the ModuleUnit list.
      /// </summary>
      internal void AddModuleUnit(Phx.PEModuleUnit moduleUnit)
      {
         this.moduleUnits.Add(moduleUnit);
      }

      /// <summary>
      /// Adds the given Manifest object to the manifest list.
      /// </summary>
      internal void AddManifest(Phx.Manifest manifest, bool isExternal)
      {
         if (isExternal)
         {
            this.externalManifests.Add(manifest);
         }
         else
         {
            this.manifests.Add(manifest);
         }
      }

      /// <summary>
      /// Appends information related to the given module unit
      /// to the provided string builder.
      /// </summary>
      private void ParseModuleUnit(Phx.PEModuleUnit moduleUnit, 
         List<string> customAttributes, StringBuilder stringBuilder)
      {
         stringBuilder.AppendFormat(".module {1}{0}",
            Environment.NewLine, 
            Driver.GetFormattedName(moduleUnit.NameString));
            
         // Append custom module attributes.
         if (customAttributes != null)
         {
            foreach (string customAttribute in customAttributes)
            {
               stringBuilder.AppendLine(customAttribute);
            }
         }

         // .imagebase
         stringBuilder.AppendFormat(".imagebase 0x{1}{0}",
            Environment.NewLine,
            Utility.ToHexString(moduleUnit.ImageBase));

         // .file alignment
         stringBuilder.AppendFormat(".file alignment 0x{1}{0}",
            Environment.NewLine,
            moduleUnit.FileAlignment.ToString("X8"));

         // .stackreserve
         stringBuilder.AppendFormat(".stackreserve 0x{1}{0}",
            Environment.NewLine,
            Utility.ToHexString(moduleUnit.SizeOfStackReserve));

         // .subsystem
         ushort subsystem = moduleUnit.Subsystem;
         stringBuilder.AppendFormat(".subsystem 0x{1}       // {2}{0}",
            Environment.NewLine, subsystem.ToString("x4"),
            CorFlags.FormatSubsystem((CorFlags.Subsystem)subsystem));

         // .corflags
         uint cor20Flags = moduleUnit.Cor20Flags;
         stringBuilder.AppendFormat(".corflags 0x{0}", 
            cor20Flags.ToString("x8"));
         if (cor20Flags != 0x0)
         {
            stringBuilder.AppendFormat("    // {0}", 
               CorFlags.ParseCorImageFlags((CorFlags.CorImageFlags)cor20Flags));
         }
         
         stringBuilder.AppendLine();
      }

      /// <summary>
      /// Appends information related to the given manifest
      /// to the provided string builder.
      /// </summary>
      private void ParseManifest(Phx.Manifest manifest, bool isExtern, 
         StringBuilder stringBuilder)
      {
         string indent = "  ";

         // .assembly
         stringBuilder.AppendFormat(".assembly {2}{1}{0}{{{0}",
            Environment.NewLine, manifest.Name,
            isExtern ? "extern " : string.Empty);

         // Append custom attributes.
         if (this.manifestAttributes != null &&
             this.manifestAttributes.ContainsKey(manifest))
         {
            foreach (string customAttribute in
               this.manifestAttributes[manifest])
            {
               stringBuilder.AppendFormat("{1}{2}{0}",
                  Environment.NewLine, indent, customAttribute);
            }
         }
         
         // .ver
         string version = string.Format("{0}:{1}:{2}:{3}",
            manifest.Version.MajorVersion, manifest.Version.MinorVersion,
            manifest.Version.BuildNumber, manifest.Version.RevisionNumber);
            
         stringBuilder.AppendFormat("{1}.ver {2}{0}",
            Environment.NewLine, indent, version);

         // .publickey or .publickeytoken
         if (manifest.PublicKey != null)
         {
            stringBuilder.AppendFormat("{0}.publickey{2} ({1})",
               indent,
               Utility.FormatByteArray(manifest.PublicKey.Key),
               manifest.PublicKey.IsPublicKeyToken ? "token" : string.Empty);
               
            // Append ASCII representation only if at least one byte
            // is printable.
            
            if (Utility.ContainsPrintableCharacters(
               manifest.PublicKey.Key))
            {
               stringBuilder.AppendFormat("   // {0}",
                  Utility.FormatByteArrayAsText(manifest.PublicKey.Key));
            }
               
            stringBuilder.AppendLine();
         }

         // .locale
         if (manifest.CultureString.Length > 0)
         {
            stringBuilder.AppendFormat("{1}.locale {2}{0}",
               Environment.NewLine, indent,
               manifest.CultureString);
         }

         // .hash algorithm
         if (manifest.HashAlgorithm != 0)
         {
            stringBuilder.AppendFormat("{1}.hash algorithm 0x{2}{0}",
               Environment.NewLine, indent, 
               manifest.HashAlgorithm.ToString("X8"));
         }


         stringBuilder.AppendFormat("}}{0}", Environment.NewLine);
      }
     
      /// <summary>
      /// Associates the given attribute string with the provided
      /// Manifest object.
      /// </summary>
      internal void AddAttributeString(Phx.Manifest manifest, 
         string attributeString)
      {
         // Create string list if this is the first value for 
         // the given manifest.
         if (!this.manifestAttributes.ContainsKey(manifest))
         {
            this.manifestAttributes.Add(manifest, new List<string>());
         }
         
         this.manifestAttributes[manifest].Add(attributeString);
      }

      /// <summary>
      /// Associates the given attribute string with the provided
      /// ModuleUnit object.
      /// </summary>
      internal void AddCustomAttribute(Phx.ModuleUnit moduleUnit, 
         string customAttribute)
      {
         // Create string list if this is the first value for 
         // the given unit.
         if (!this.customAttributes.ContainsKey(moduleUnit))
         {
            this.customAttributes.Add(moduleUnit, new List<string>());
         }
         
         this.customAttributes[moduleUnit].Add(customAttribute);
      }
      
      /// <summary>
      /// Sets or retrieves the CLR version string associated with
      /// the current assembly.
      /// </summary>
      internal string MetadataVersion
      {
         get { return this.metadataVersion; }
         set 
         { 
            // The version string can have trailing null characters.
            if (value != null)
            {
               this.metadataVersion = value.TrimEnd(new char[] { '\0' });
            }
            else
            {
               this.metadataVersion = string.Empty;
            }
         }
      }

      /// <summary>
      /// Adds the given resource symbol to the list of 
      /// manifest resource symbols.
      /// </summary>
      internal void AddResourceSymbol(
         Phx.Symbols.ResourceSymbol resourceSymbol)
      {
         // To match ILDasm output, keep the list sorted by
         // resource offset.
         uint offset = resourceSymbol.ExternId;
         for (int i = 0; i < this.resourceSymbols.Count; i++)
         {
            if (offset < this.resourceSymbols[i].ExternId)
            {
               this.resourceSymbols.Insert(i, resourceSymbol);
               return;
            }
         }
         this.resourceSymbols.Add(resourceSymbol);
      }

      /// <summary>
      /// Appends information related to the given resource symbol
      /// to the provided string builder.
      /// </summary>
      private void ParseResourceSymbol(
         Phx.Symbols.ResourceSymbol resourceSymbol, 
         StringBuilder stringBuilder)
      {
         string body;

         // .file
         Phx.Symbols.FileSymbol resourceFileSymbol = 
            resourceSymbol.ResourceFileSymbol;
         if (resourceFileSymbol != null)
         {            
            body = string.Format(".file {0} at 0x{1}", 
               resourceFileSymbol.NameString, 
               resourceFileSymbol.ExternId.ToString("X8"));
         }
         else
         {
            body = string.Format("// Offset: {0} Length: {1}",
               resourceSymbol.ExternId.ToString("X8"),
               "?"
            );
         }

         // .mresource
         stringBuilder.AppendFormat(".mresource {1} {2}{0}{{{0}  {3}{0}}}{0}",
            Environment.NewLine,
            "public",
            resourceSymbol.NameString,
            body            
         );
      }

      /// <summary>
      /// Adds the given import module symbol to the list of 
      /// external module references.
      /// </summary>
      internal void AddExternModuleReference(
         Phx.Symbols.ImportModuleSymbol importModuleSymbol)
      {
         this.externalModuleReferences.Add(importModuleSymbol);
      }

      /// <summary>
      /// Adds the given file symbol to the list of 
      /// file symbol references.
      /// </summary>
      internal void AddFileSymbol(Phx.Symbols.FileSymbol fileSymbol)
      {
         this.fileReferences.Add(fileSymbol);
      }
   }
}
