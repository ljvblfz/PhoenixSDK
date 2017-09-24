using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;
using System.IO;

namespace PEExplorer
{
   /// <summary>
   /// Drives the processing of PE module units.
   /// </summary>
   class Driver
   {
      // The PE module unit managed by the driver.
      private Phx.PEModuleUnit moduleUnit;
      
      // Builds manifest-related information.
      private ManifestBuilder manifestBuilder;

      // Flags cancellation is requested by a worker thread.
      private bool cancellationPending;

      // Maps global variable symbols to their respective RVAs.
      private Dictionary<uint, Phx.Symbols.GlobalVariableSymbol> 
         globalDataSymbols;
         
      // Map of source file name to source listing.
      private Dictionary<string, string[]> sourceFileMap;
      
      // Map of source file names as they appear in the .pdb to 
      // actual disk locations (resolved by the user).
      private Dictionary<string, string> sourceFileAliasMap;
      
      // Provider to browse for files.
      private IFileBrowser fileBrowser;
      
      /// <summary>
      /// Gets the PE module unit managed by the driver.
      /// </summary>
      internal Phx.PEModuleUnit ModuleUnit
      {
         get { return this.moduleUnit; }
      }

      /// <summary>
      /// Gets or sets whether cancellation is requested by a worker thread.
      /// </summary>      
      internal bool CancellationPending
      {
         get { return this.cancellationPending; }
         set { this.cancellationPending = value; }
      }
            
      /// <summary>
      /// Constructs a new Driver object.
      /// </summary>
      internal Driver(IFileBrowser fileBrowser)
      {
         this.cancellationPending = false;
         this.manifestBuilder = new ManifestBuilder();
         this.globalDataSymbols =
            new Dictionary<uint, Phx.Symbols.GlobalVariableSymbol>();
         this.sourceFileMap = new Dictionary<string,string[]>();
         this.sourceFileAliasMap = new Dictionary<string,string>();
         this.fileBrowser = fileBrowser;
      }

      /// <summary>
      /// Closes the PE module unit managed by the driver.
      /// </summary>
      internal void CloseModuleUnit()
      {
         if (this.moduleUnit != null)
         {
            // Close the module unit.
            this.moduleUnit.Close();

            // Release our handle to the PEModuleUnit object.
            this.moduleUnit = null;
         }
      }

      /// <summary>
      /// Loads module information contained at the provided file location.
      /// </summary>
      internal ElementNode ProcessModule(string fileName)
      {
         // Process the module in a try/catch block so that we
         // can invalidate the PEModuleUnit handle on error.
         try
         {
            return this.InternalProcessModule(fileName);
         }
         catch (Exception e)
         {
            this.moduleUnit = null;
            throw e;
         }
      }

      /// <summary>
      /// Loads module information contained at the provided file location.
      /// </summary>
      private ElementNode InternalProcessModule(string fileName)
      {
         // Open the PEModuleUnit.
         this.moduleUnit = Phx.PEModuleUnit.Open(fileName);
        
         // Load visible code/data entries into the module symbol table.
         this.moduleUnit.LoadGlobalSymbols();

         // Build the call graph.
         Driver.BuildCallGraph(this.moduleUnit);
         
         // The BuildCallGraph method builds encoded IR for each
         // function in the unit. Because we don't need this 
         // information right now, release each function unit
         // in the module unit.
         foreach (Phx.Unit unit in this.moduleUnit.ChildUnits)
         {
            if (unit.IsFunctionUnit)
            {
               this.ReleaseFunctionUnit(unit.AsFunctionUnit.FunctionSymbol);
            }
         }
         
         // Add the module unit to the manifest builder.
         this.manifestBuilder.AddModuleUnit(this.moduleUnit);
         
         // Throw an exception if the module unit does not contain metadata.
         if (!this.moduleUnit.HasMetadata)
         {
            throw Phx.FatalError.New(
               string.Format("'{0}' contains no metadata and cannot be processed.",
               fileName));
         }
         
         // Set the metadata version.
         this.manifestBuilder.MetadataVersion = 
            this.moduleUnit.ClrVersionString;

         // Add information for each assembly unit reference to the 
         // manifest builder.
         if (this.moduleUnit.AssemblyUnitReferenceList != null)
         {
            foreach (Phx.AssemblyUnit assemblyUnitReference in
               this.moduleUnit.AssemblyUnitReferenceList)
            {
               this.ProcessAssemblyUnitReference(assemblyUnitReference);
            }
         }

         // Keep a list of the import module symbols contained in the 
         // module unit. 
         // To obtain the list of external module references, we will remove 
         // from this list all import module symbols that are explicitly
         // listed in the module's import module symbol list.
         List<Phx.Symbols.ImportModuleSymbol> importModuleSymbols = 
            new List<Phx.Symbols.ImportModuleSymbol>();

         // Iterate through the symbol table associated with the module unit 
         // and check for three types of symbols:
         // (1) Add global variable symbols with data instructions to the the 
         //     global variable map.
         // (2) Add import module symbols to the import module symbol list.
         // (3) Add file and resource symbols to the manifest builder.
         foreach (Phx.Symbols.Symbol symbol 
            in moduleUnit.SymbolTable.AllSymbols)
         {
            // Check for global variable symbol with an associated
            // data instruction.
            if (symbol.IsGlobalVariableSymbol &&
                symbol.Location != null &&
                symbol.Location.IsDataLocation &&
                symbol.Location.AsDataLocation.DataInstruction != null)
            {
               if (!globalDataSymbols.ContainsKey(symbol.Rva))
               {
                  globalDataSymbols.Add(symbol.Rva, 
                     symbol.AsGlobalVariableSymbol);
               }
            }

            // Check for import module symbols.
            else if (symbol.IsImportModuleSymbol)
            {
               importModuleSymbols.Add(symbol.AsImportModuleSymbol);
            }
                        
            // Check for other interesting symbols.            
            else if (symbol.IsFileSymbol)
            {
               this.manifestBuilder.AddFileSymbol(symbol.AsFileSymbol);
            }
            else if (symbol.IsResourceSymbol)
            {
               this.manifestBuilder.AddResourceSymbol(symbol.AsResourceSymbol);
            }            
         }
         
         // Remove all import module symbols referenced by the 
         // ImportModuleSymbols property from the import module symbol list.
         // This will give us the list of external module references.
         foreach (Phx.Symbols.ImportModuleSymbol importModuleSymbol 
            in moduleUnit.ImportModuleSymbols)
         {
            importModuleSymbols.Remove(importModuleSymbol);
         }
         // Add the remaining, named, import module symbols to the 
         // manifest builder's list of external module references.
         string moduleName = this.moduleUnit.NameString;
         foreach (Phx.Symbols.ImportModuleSymbol importModuleSymbol 
            in importModuleSymbols)
         {
            if (importModuleSymbol.NameString.Length > 0 &&
               !importModuleSymbol.NameString.Equals(moduleName))
            {
               this.manifestBuilder.AddExternModuleReference(
                  importModuleSymbol);
            }
         }

         // Parse the v-table fixups symbol.
         if (this.moduleUnit.VTableFixupsSymbol != null)
         {
            this.ParseVTableFixupsSymbol(this.moduleUnit.VTableFixupsSymbol);
         }

         // Build the hierarchy of ElementNode objects for the loaded 
         // PE module unit and return the root node.
         return Collect(this.moduleUnit);
      }

      /// <summary>
      /// Builds a call graph for the given PEModuleUnit object.
      /// </summary>
      internal static Phx.Graphs.CallGraph 
      BuildCallGraph(Phx.PEModuleUnit moduleUnit)
      {
         Phx.Lifetime lifetime = moduleUnit.Lifetime;

         // We build the call graph by running a phase list that
         // contains a BuildCallGraphPhase object. 
         // This particular phase list acts on a ProgramUnit object.

         // Build a program unit that contains the module unit.
         
         Phx.ProgramUnit programUnit = Phx.ProgramUnit.New(
            lifetime, null, Phx.GlobalData.TypeTable, 
            moduleUnit.Architecture, moduleUnit.Runtime);

         programUnit.AddModuleUnit(moduleUnit);
         
         // Create the phase list.
         Phx.Phases.PhaseConfiguration config =
            Phx.Phases.PhaseConfiguration.New(lifetime, 
               "CallGraph Phases");

         // Add phase to list.
         config.PhaseList.AppendPhase(Phx.PE.BuildCallGraphPhase.New(config));

         // Execute the phases.
         config.PhaseList.DoPhaseList(programUnit);

         // Return the newly-built call graph.
         return moduleUnit.CallGraph;
      }

      /// <summary>
      /// Adds information related to the given AssemblyUnit object 
      /// to the manifest builder.
      /// </summary>
      private void ProcessAssemblyUnitReference(
         Phx.AssemblyUnit assemblyUnitReference)
      {
         Phx.Manifest manifest = assemblyUnitReference.Manifest;
         
         // Add the manifest to the builder.
         this.manifestBuilder.AddManifest(manifest, 
            assemblyUnitReference.IsExtern);

         // Retrieve all attribute and permission symbols associated
         // with the assembly symbol.
         Phx.Symbols.AssemblySymbol assemblySymbol =
            assemblyUnitReference.AssemblySymbol;
            
         Phx.Symbols.AttributeSymbol[] attributeSymbols =
            this.GetCustomAttributes(assemblySymbol);
         Phx.Symbols.PermissionSymbol[] permissionSymbols =
            this.GetPermissionAttributes(assemblySymbol);

         // Add the name of each attribute and permission symbol in 
         // the above arrays to the manifest builder.         
         List<string> attributeStrings = new List<string>();
         if (attributeSymbols != null)
         {
            foreach (Phx.Symbols.AttributeSymbol symbol in attributeSymbols)
            {
               attributeStrings.Add(this.BuildElementNode(symbol).Name);
            }
         }
         if (permissionSymbols != null)
         {
            foreach (Phx.Symbols.PermissionSymbol symbol in permissionSymbols)
            {
               attributeStrings.Add(this.BuildElementNode(symbol).Name);
            }
         }
         foreach (string attributeString in attributeStrings)
         {
            this.manifestBuilder.AddAttributeString(manifest, attributeString);
         }
      }

      /// <summary>
      /// Adds v-table fixup information to the manifest builder.
      /// </summary>
      private unsafe void
      ParseVTableFixupsSymbol(
         Phx.Symbols.GlobalVariableSymbol vtableFixupsSymbol)
      {
         // Get the data instruction associated with the fixups symbol.
         Phx.IR.DataInstruction dataInstruction =
            vtableFixupsSymbol.Location.AsDataLocation.DataInstruction;

         // Obtain a pointer to the data. 
         byte* dataPointer = dataInstruction.GetDataPointer(0);
       
         // Calculate the number of fixups in the data blob.
         // Each fixup entry is 8 bytes wide.
         int fixupCount = (int)dataInstruction.ByteSize / 8;

         // Generate a .vtfixup entry for each fixup in the         
         // data instruction.
         for (int i = 0; i < fixupCount; i++)
         {
            StringBuilder stringBuilder = new StringBuilder();
            
            stringBuilder.Append(".vtfixup ");

            // Each fixup entry consists of an RVA, element count,
            // and data type.
            int fixupRva = Utility.ReadInt32(ref dataPointer);
            short count = Utility.ReadInt16(ref dataPointer);
            short type = Utility.ReadInt16(ref dataPointer);

            int j;
            long[] values = new long[count];
            
            // Read values.
            for (j = 0; j < count; j++)
            {
               try
               {
                  // type 0x01 is 4-byte integer.
                  if ((type & 0x01) == 0x01)
                  {
                     byte* p = this.GetPointerFromRva((uint)fixupRva);
                     if (p != null)
                        values[j] = *((int*)(p));
                     else
                        values[j] = 0;
                  }
                  // type 0x02 is 8-byte integer.
                  if ((type & 0x02) == 0x02)
                  {
                     byte* p = this.GetPointerFromRva((uint)fixupRva);
                     if (p != null)
                        values[j] = *((long*)(p));
                     else
                        values[j] = 0;
                  }
               }
               catch (AccessViolationException)
               {
                  // Set a default value in the case where the binary is 
                  // malformed or other error occurred.
                  values[j] = 0x0;
               }
            }

            // Append slot number.
            stringBuilder.AppendFormat("[{0}] ", j.ToString("X"));

            // Append flags.
            if ((type & 0x01) == 0x01)
               stringBuilder.Append("int32 ");

            if ((type & 0x02) == 0x02)
               stringBuilder.Append("int64 ");

            if ((type & 0x04) == 0x04)
               stringBuilder.Append("fromunmanaged ");

            if ((type & 0x08) == 0x08)
               stringBuilder.Append("retainappdomain ");

            // Append RVA. 
            // We use the prefix 'D' to indicate that the fixup RVA
            // resides in the .data section. The prefix 'T' indicates that
            // the fixup resides in the .tls (thread local storage) section.
            char prefix = 'D';
            Phx.Symbols.GlobalVariableSymbol symbol;
            if (globalDataSymbols.TryGetValue((uint)fixupRva, out symbol))
            {
               if (symbol.AllocationBaseSectionSymbol != null &&
                   symbol.AllocationBaseSectionSymbol.IsTls)
               {
                  prefix = 'T';
               }
            }
            stringBuilder.AppendFormat("at {0}_{1} //", 
               prefix, fixupRva.ToString("X8"));

            // Append value. We used the value 0x0 above to indicate that
            // the value could not be obtained.
            for (j = 0; j < count; j++)
            {
               if ((type & 0x01) == 0x01)
                  if (values[j] == 0x0)
                     stringBuilder.Append(" ????????");
                  else
                     stringBuilder.AppendFormat(" {0}", 
                        values[j].ToString("X8"));

               if ((type & 0x02) == 0x02)
                  if (values[j] == 0x0)
                     stringBuilder.Append(" ????????????????");
                  else
                     stringBuilder.AppendFormat(" {0}", 
                        values[j].ToString("X16"));
            }

            // Add the entry to the manifest builder.
            this.manifestBuilder.AddVTableFixupString(stringBuilder.ToString());
         }
      }

      /// <summary>
      /// Retrieves the data pointer associated with the data instruction of the
      /// global variable symbol with the given RVA.
      /// </summary>
      private unsafe byte* GetPointerFromRva(uint rva)
      {
         Phx.Symbols.GlobalVariableSymbol symbol;
         if (this.globalDataSymbols.TryGetValue(rva, out symbol))
         {            
            return symbol.Location.AsDataLocation.
               DataInstruction.GetDataPointer(0);
         }
         return null;
      }
      
      /// <summary>
      /// Builds the hierarchy of ElementNode objects for the loaded PE
      /// module unit.
      /// </summary>
      private ElementNode Collect(Phx.ModuleUnit moduleUnit)
      {
         // Build the root node.
         ElementNode rootNode = this.BuildElementNode(moduleUnit.UnitSymbol);
         if (rootNode != null)
         {
            // Recursively walk the symbol tables of the module.
            this.WalkScopeAux(rootNode, rootNode.Symbol);
         }
         return rootNode;
      }

      /// <summary>
      /// Creates an ElementNode object for the given symbol and 
      /// adds it to the parent node. 
      /// </summary>
      private void WalkScope(ElementNode parentNode, Phx.Symbols.Symbol symbol)
      {
         // Exit if the application is requesting cancellation.
         if (this.cancellationPending)
         {
            return;
         }

         // Add assembly reference manifests to the manifest builder.

         if (symbol.IsAssemblySymbol && symbol.LexicalNestLevel > 0)
         {
            this.manifestBuilder.AddManifest(
               symbol.AsAssemblySymbol.Manifest, true);
               
            // Do not build node information for external assembly references. 
            return;
         }

         // Build a node for this symbol and add it to its parent.

         ElementNode elementNode = this.BuildElementNode(symbol);
         if (elementNode != null)
         {
            parentNode.Children.Add(elementNode);

            // Walk scope.
            
            if (symbol.IsLexicalScope)
            {
               this.WalkScopeAux(elementNode, symbol);
            }
         }
      }

      /// <summary>
      /// Creates ElementNode objects for the symbols that are contained 
      /// within the scope of the given symbol and adds them to the 
      /// provided parent node.
      /// </summary>
      private void WalkScopeAux(ElementNode parentNode, Phx.Symbols.Symbol symbol)
      {
         // Traverse the symbols contained within the scope of 
         // the provided symbol.
         
         Phx.Collections.SymbolIConstantIterator iterator =
            symbol.SymbolIConstantIterator;
         while (iterator.MoveNext())
         {
            // Get the current value.
            Phx.Symbols.Symbol nestedSymbol = iterator.CurrentValue;
            
            // Process the symbol only if it is properly contained in 
            // the the scope of the parent symbol.            

            if (nestedSymbol.LexicalParentSymbol == symbol)
            {
               this.WalkScope(parentNode, nestedSymbol);
            }                        
         }
      }

      // The default Layout if one is not provided.
      private static readonly Phx.Types.Layout DefaultLayout = 
         Phx.Types.Layout.Auto;

      // The default CharacterSet if one is not provided.
      private static readonly Phx.Types.CharacterSet DefaultCharacterSet = 
         Phx.Types.CharacterSet.Ansi;

      /// <summary>
      /// Builds an ElementNode object with information related to
      /// the given symbol.
      /// </summary>
      private ElementNode BuildElementNode(Phx.Symbols.Symbol symbol)
      {
         // Build a string that represents the name of the symbol.
         
         string symbolName = Driver.GetFullName(symbol);
         
         // Build an ElementNode object based on the kind of the symbol.
                  
         ElementNode node = null;

         switch (symbol.SymbolKind)
         {
            // Assembly or ImportModule
            case Phx.Symbols.SymbolKind.Assembly:
            case Phx.Symbols.SymbolKind.ImportModule:
            {
               node = new ElementNode(symbol, symbol.NameString, string.Empty, 
                  ElementType.Assembly);
            }
            break;

            // Attribute            
            case Phx.Symbols.SymbolKind.Attribute:
            {
               string attributeSignature = 
                  this.GetCustomAttributeSignature(symbol.AsAttributeSymbol);

               node = new ElementNode(symbol, attributeSignature, string.Empty, 
                  ElementType.Attribute);
            }
            break;

            // Event
            case Phx.Symbols.SymbolKind.Event:
            {
               Phx.Symbols.EventSymbol eventSymbol = symbol.AsEventSymbol;

               string eventName = 
                  Driver.GetFormattedName(eventSymbol.NameString);
               string eventType = 
                  Driver.GetFriendlyTypeName(eventSymbol.EventType, true);

               node = new ElementNode(symbol,
                  string.Format("event {0} : {1}", eventName, eventType),
                  string.Empty, ElementType.Event);
            }
            break;

            // Field
            case Phx.Symbols.SymbolKind.Field:
            {
               string fieldName = this.GetFieldDisplayString(
                  symbol.AsFieldSymbol);

               node = new ElementNode(symbol, fieldName, string.Empty,
                  ElementType.Field);
            }
            break;

            // Function
            case Phx.Symbols.SymbolKind.Function:
            {
               // Build the node by using the BuildFunctionSymbolNode 
               // helper method.
               node = this.BuildFunctionSymbolNode(symbol.AsFunctionSymbol);
            }
            break;

            // GlobalVariable
            case Phx.Symbols.SymbolKind.GlobalVariable:
            {
               string globalName = this.GetGlobalDisplayString(
                  symbol.AsGlobalVariableSymbol);

               node = new ElementNode(symbol, globalName, string.Empty,
                  ElementType.Global);
            }
            break;

            // Namespace
            case Phx.Symbols.SymbolKind.Namespace:
            {
               string namespaceName = string.Empty;
               
               // For the root namespace, show the name of the module.
               if (symbol.NameString.Equals("::"))
               {
                  namespaceName = symbol.Table.Unit.NameString;
               }
               // Otherwise, show the namespace name.
               else
               {
                  namespaceName = symbol.NameString;
               }

               // Build a new ElementNode object for the namespace.
               node = new ElementNode(symbol, namespaceName, string.Empty,
                  ElementType.Namespace);
            }
            break;
            
            // Permission
            case Phx.Symbols.SymbolKind.Permission:
            {
               string permissionSignature =
                  this.GetPermissionSignature(symbol.AsPermissionSymbol);

               node = new ElementNode(symbol, permissionSignature, 
                  string.Empty, ElementType.Attribute);
            }
            break;

            // Property
            case Phx.Symbols.SymbolKind.Property:
            {
               string propertyName = this.GetPropertyDisplayString(
                  symbol.AsPropertySymbol);

               node = new ElementNode(symbol, propertyName, string.Empty,
                  ElementType.Property);
            }
            break;

            // StaticField
            case Phx.Symbols.SymbolKind.StaticField:
            {
               string staticFieldName = this.GetStaticFieldDisplayString(
                  symbol.AsStaticFieldSymbol);

               node = new ElementNode(symbol, staticFieldName, string.Empty,
                  ElementType.StaticField);
            }
            break;
            
            // Type or MsilType
            case Phx.Symbols.SymbolKind.Type:
            case Phx.Symbols.SymbolKind.MsilType:
            {
               // Build the node by using the BuildTypeSymbolNode 
               // helper method.
               node = this.BuildTypeSymbolNode(symbol.AsTypeSymbol);
            }
            break;
         }
         
         // Add any custom attributes associated with the symbol 
         // to the node as child nodes.
         if (node != null)
         {
            this.AddCustomAttributes(node, this.GetCustomAttributes(node.Symbol));
         }         
         
         return node;
      }

      /// <summary>
      /// Retrieves the fully-qualified name of the given symbol.
      /// </summary>
      internal static string GetFullName(Phx.Symbols.Symbol symbol)
      {
         // Build a fully-qualified name for symbols that contain 
         // a non-import module lexical parent symbol.
         if (symbol.LexicalParentSymbol != null &&
            !symbol.LexicalParentSymbol.IsImportModuleSymbol)
         {
            return string.Format("{0}::{1}",
               symbol.LexicalParentSymbol.QualifiedName, symbol.NameString);
         }
         // Otherwise, use the name provided by the NameString property.
         else
         {
            return symbol.NameString;
         }
      }
      
      /// <summary>
      /// Creates a ElementNode object for the given function symbol.
      /// </summary>
      private ElementNode BuildFunctionSymbolNode(
         Phx.Symbols.FunctionSymbol functionSymbol)
      {
         // Generate the ElementType of the function symbol.        
         ElementType symbolType;

         // If the symbol was originally encoded as Msil or is abstract,
         // then we have an instance or static method.
         if (functionSymbol.WasEncodedAsMsil || functionSymbol.IsAbstract)
         {
            // This flag is used to distinguish the node type 
            // as a generic function/method.
            bool isGenericFunction = Driver.IsGenericFunction(functionSymbol);

            // Use the IsInstanceMethod property to determine whether the
            // method is instance or static.
            if (functionSymbol.FunctionType.IsInstanceMethod)
            {
               if (isGenericFunction)
               {
                  symbolType = ElementType.GenericInstanceMethod;
               }
               else
               {
                  symbolType = ElementType.InstanceMethod;
               }
            }
            else
            {
               if (isGenericFunction)
               {
                  symbolType = ElementType.GenericStaticMethod;
               }
               else
               {
                  symbolType = ElementType.StaticMethod;
               }               
            }
         }
         // Otherwise, flag the method as native.
         else
         {
            symbolType = ElementType.NativeMethod;
         }

         // Generate type variable information if the function symbol
         // has a type variable type list.
         string typeVariableTypeList = string.Empty;
         if (functionSymbol.TypeVariableTypeList != null)
         {
            typeVariableTypeList = string.Concat(" ",
               FormatTypeVariableTypeList(functionSymbol.TypeVariableTypeList));
         }

         Phx.Types.FunctionType functionType = functionSymbol.FunctionType;
         
         // Generate a formatted string that describes the method prototype.

         string methodName = string.Format("method {0} : {1}{2}{3}({4})",
            Driver.GetFormattedName(
               Driver.GetClassName(functionSymbol.NameString)),
            functionType.IsVariableArguments ? "vararg " : string.Empty,
            Driver.GetFriendlyTypeName(functionType.ReturnType, true),
            typeVariableTypeList,
            Driver.GetFunctionTypeArgumentList(functionType));

         // Create the ElementNode object for the symbol.
         
         ElementNode node = new ElementNode(functionSymbol, methodName, 
            string.Empty, symbolType);

         // Append override information for the method as child nodes.
         if (functionSymbol.EnclosingAggregateType != null)
         {
            // Get base type link.
            Phx.Types.BaseTypeLink link =
               functionSymbol.EnclosingAggregateType.BaseTypeLinkList;
            if (link != null)
            {
               // Look up parent symbol by name in the base type.
               Phx.Symbols.Symbol parentSymbol = 
                  link.BaseAggregateType.TypeSymbol.LookupInLexicalScope(
                     functionSymbol.Name);

               // If a parent symbol exists, generate a formatted 
               // information string and add a new child node to the 
               // parent node.
               if (parentSymbol != null)
               {
                  string overrideText = string.Format(
                     ".override {0}::{1}", 
                     Driver.GetFriendlyTypeName(link.BaseAggregateType, false),
                     parentSymbol.NameString);
                     
                  node.Children.Add(new ElementNode(parentSymbol, overrideText, 
                     string.Empty, ElementType.Override));
               }
            }
         }
         
         return node;
      }

      /// <summary>
      /// Creates an ElementNode object for the given type symbol.
      /// </summary>
      private ElementNode BuildTypeSymbolNode(
         Phx.Symbols.TypeSymbol typeSymbol)
      {
         ElementType symbolType;
         
         // Create a StringBuilder object to hold the formatted
         // information string.
         StringBuilder stringBuilder = new StringBuilder();

         Phx.Types.Type type = typeSymbol.Type;
         
         // For the usual case, we expect the type to be an aggregate.
         if (type.IsAggregateType)
         {
            Phx.Types.AggregateType aggregateType = type.AsAggregateType;

            // Generate a .class prefix and set the element type.
            // Here, we look for the following class Msil types:
            //  * Class
            //  * Enum
            //  * Value type
            //  * Interface            
            if (Driver.IsMsilClass(aggregateType))
            {
               stringBuilder.Append(".class");

               if (aggregateType.IsGeneric)
               {
                  symbolType = ElementType.GenericClass;
               }
               else
               {
                  symbolType = ElementType.Class;
               }
            }
            else if (Driver.IsMsilEnum(aggregateType))
            {
               stringBuilder.Append(".class enum");

               if (aggregateType.IsGeneric)
               {
                  symbolType = ElementType.GenericEnum;
               }
               else
               {
                  symbolType = ElementType.Enum;
               }
            }
            else if (Driver.IsMsilValueType(aggregateType))
            {
               stringBuilder.Append(".class value");

               if (aggregateType.IsGeneric)
               {
                  symbolType = ElementType.GenericValueType;
               }
               else
               {
                  symbolType = ElementType.ValueType;
               }
            }
            else if (Driver.IsMsilInterface(aggregateType))
            {
               stringBuilder.Append(".class interface");

               if (aggregateType.IsGeneric)
               {
                  symbolType = ElementType.GenericInterface;
               }
               else
               {
                  symbolType = ElementType.Interface;
               }
            }
            else
            {
               // This case is unexpected.
               stringBuilder.AppendLine(aggregateType.TypeSymbol.NameString);
               symbolType = ElementType.Bad;
            }

            // Append access level. 
            stringBuilder.AppendFormat(" {0}", 
               aggregateType.Access.ToString().ToLower());

            // Append 'abstract' token for interfaces and
            // abstract types.
            if (aggregateType.IsAbstract || aggregateType.IsOnlyInterface)
            {
               stringBuilder.Append(" abstract");
            }

            // Append layout.
            Phx.Types.Layout layout = aggregateType.Layout;
            if (layout == Phx.Types.Layout.IllegalSentinel)
               layout = DefaultLayout;
            stringBuilder.AppendFormat(" {0}", layout.ToString().ToLower());

            // Append character set.
            Phx.Types.CharacterSet characterSet = aggregateType.CharacterSet;
            if (characterSet == Phx.Types.CharacterSet.IllegalSentinel)
               characterSet = DefaultCharacterSet;
            stringBuilder.AppendFormat(" {0}", 
               characterSet.ToString().ToLower());

            // Append 'sealed' token for sealed types.
            if (aggregateType.IsSealed)
            {
               stringBuilder.Append(" sealed");
            }

            // Append 'beforefieldinit' token.
            if (aggregateType.BeforeFieldInitialize)
            {
               stringBuilder.Append(" beforefieldinit");
            }

            stringBuilder.AppendLine();

            // Append inheritance information.

            Phx.Types.BaseTypeLink baseTypeLink = 
               aggregateType.BaseTypeLinkList;
            while (baseTypeLink != null)
            {
               Phx.Types.AggregateType baseType = 
                  baseTypeLink.BaseAggregateType;

               // Because all classes implicitly extend System.Object,
               // only show inheritance information for classes 
               // lower than Object.

               if (!baseType.Equals(
                  baseType.TypeTable.SystemObjectAggregateType))
               {
                  string info = Driver.GetFriendlyTypeName(baseType, false);
                  if (baseType.IsPrimary)
                  {
                     info = string.Format("extends {0}", info);
                  }
                  else
                  {
                     info = string.Format("implements {0}", info);
                  }
                  stringBuilder.AppendLine(info);
               }
               baseTypeLink = baseTypeLink.Next;
            }
         }
         // Handle the case where the type is not an aggregate type.
         // This case is unexpected.
         else 
         {
            stringBuilder.AppendLine(type.TypeSymbol.Name);
            symbolType = ElementType.Bad;
         }

         // Create the ElementNode object for the type.
         ElementNode node = new ElementNode(typeSymbol,
            Driver.GetFriendlyTypeName(type, false), stringBuilder.ToString(), 
            symbolType);

         // Append each line in the Msil signature as a child node.
         foreach (string info in node.Signature)
         {
            node.Children.Add(new ElementNode(null, info, string.Empty, 
               ElementType.Info));
         }
         
         return node;
      }

      /// <summary>
      /// Determines whether the given type is a generic type or
      /// a container for a generic type.
      /// </summary>
      private static bool IsGenericType(Phx.Types.Type type)
      {
         if (type == null)
         {
            return false;
         }
         
         // Determine whether the type is a generic type based on 
         // its type kind.
         switch (type.TypeKind)
         {
            // For type variables, check whether the parent
            // aggregate type is generic.
            case Phx.Types.TypeKind.TypeVariable:
            {
               Phx.Types.AggregateType parentAggregateType = 
                  type.AsTypeVariableType.ParentAggregateType;
                  
               if (parentAggregateType != null)
                  return Driver.IsGenericType(parentAggregateType);
               return false;
            }
            
            // For aggregate types, check whether the IsGeneric flag is set
            // or its uninstantiated aggreate type is generic.
            case Phx.Types.TypeKind.Aggregate:
            {
               Phx.Types.AggregateType aggregateType = type.AsAggregateType;
               
               if (aggregateType.IsGeneric)
                  return true;
               return Driver.IsGenericType(
                  aggregateType.UninstantiatedAggregateType);
            }
            
            // For pointer types, check the referent type.
            case Phx.Types.TypeKind.Pointer:
            {
               return Driver.IsGenericType(type.AsPointerType.ReferentType);
            }

            // For managed array types, check the element type.
            case Phx.Types.TypeKind.ManagedArray:
            {
               return Driver.IsGenericType(type.AsManagedArrayType.ElementType);
            }               
         }

         return false;
      }

      /// <summary>
      /// Determines whether the given function symbol is generic or has 
      /// a generic return value.
      /// </summary>
      private static bool IsGenericFunction(
         Phx.Symbols.FunctionSymbol functionSymbol)
      {
         // Trivial case: the function symbol itself is generic.
         if (functionSymbol.IsGeneric)
            return true;

         if (functionSymbol.EnclosingAggregateType != null)
         {
            // Get base type link.
            Phx.Types.BaseTypeLink link =
               functionSymbol.EnclosingAggregateType.BaseTypeLinkList;
            if (link != null)
            {
               // Look up parent symbol by name in the base type.
               
               Phx.Name name = Phx.Name.New(functionSymbol.Lifetime, 
                  Driver.GetClassName(functionSymbol.NameString));
                                 
               Phx.Symbols.Symbol parentSymbol = 
                  link.BaseAggregateType.TypeSymbol.LookupInLexicalScope(
                     name);

               // If a parent symbol exists, return true if the parent 
               // symbol is generic.
               if (parentSymbol != null && parentSymbol.IsFunctionSymbol)
               {
                  if (IsGenericFunction(parentSymbol.AsFunctionSymbol))
                     return true;
               }

               // Perform the same process, this time for the 
               // uninstantiated version of the aggregate type.
               if (link.BaseAggregateType.UninstantiatedAggregateType != null)
               {
                  parentSymbol =
                     link.BaseAggregateType.UninstantiatedAggregateType.
                        TypeSymbol.LookupInLexicalScope(name);

                  if (parentSymbol != null && parentSymbol.IsFunctionSymbol)
                  {
                     if (IsGenericFunction(parentSymbol.AsFunctionSymbol))
                        return true;
                  }
               }
            }
         }
         
         // Return true if the function type has a generic return type.

         if (IsGenericType(functionSymbol.FunctionType.ReturnType))
            return true;         

            
         return false;
      }

      /// <summary>
      /// Retrieves the backing type associated with the given property symbol.
      /// </summary>
      private static Phx.Types.Type GetBackingType(
         Phx.Symbols.PropertySymbol propertySymbol)
      {
         Phx.Types.Type propertyType = null;
         
         // If the property has a 'getter' method, the backing property 
         // type is the return type of the method.
         if (propertySymbol.FunctionSymbolGetter != null)
         {
            propertyType = propertySymbol.FunctionSymbolGetter.
               FunctionType.ReturnType;
         }
         // If the property has a 'setter' method, the backing property
         // type is the type of the last argument in the argument
         // list of the method.
         else if (propertySymbol.FunctionSymbolSetter != null)
         {
            Phx.Types.FunctionType functionType = 
                propertySymbol.FunctionSymbolSetter.FunctionType;

            propertyType = functionType.UserDefinedParameters[functionType.UserDefinedParameters.Count-1].Type;
         }
         
         return propertyType;
      }
         
      /// <summary>
      /// Generates a formatted string that describes the given 
      /// property symbol.
      /// </summary>
      private string GetPropertyDisplayString(
         Phx.Symbols.PropertySymbol propertySymbol)
      {
         StringBuilder stringBuilder = new StringBuilder();

         // Append property token and name.
         stringBuilder.AppendFormat("prop {0} :",
            Driver.GetFormattedName(propertySymbol.NameString));

         // Append the 'instance' token if either the 'getter' or 
         // 'setter' property method is an instance method.

         Phx.Types.FunctionType functionType = null;
         if (propertySymbol.FunctionSymbolGetter != null)
         {
            functionType = propertySymbol.FunctionSymbolGetter.FunctionType;
         }
         else if (propertySymbol.FunctionSymbolSetter != null)
         {
            functionType = propertySymbol.FunctionSymbolSetter.FunctionType;
         }
         if (functionType == null || functionType.IsInstanceMethod)
         {
            stringBuilder.Append(" instance");
         }

         // Append the backing type of the property.
         string propertyType = Driver.GetFriendlyTypeName(
            Driver.GetBackingType(propertySymbol), true);
         stringBuilder.AppendFormat(" {0}", propertyType);

         // If the property has a 'getter' method, append its 
         // return type in parenthesis.
         if (propertySymbol.FunctionSymbolGetter != null)
         {
            stringBuilder.AppendFormat("({0})",
               Driver.GetFunctionTypeArgumentList(
                  propertySymbol.FunctionSymbolGetter.FunctionType));
         }
         else
         {
            stringBuilder.Append("()");
         }

         return stringBuilder.ToString();
      }

      /// <summary>
      /// Builds a Disassembly object that contains the IL listing
      /// for the given function symbol.
      /// </summary>
      private Disassembly GetIlListing(Phx.Symbols.FunctionSymbol functionSymbol)
      {
         string[] lines;
         int lineCount;
         
         string defaultMessage = "IL listing unavailable";
         
         // Read the contents of the IL listing file and record the 
         // current line count.
         lines = File.ReadAllLines(GlobalData.ListingFileName);
         lineCount = lines.Length;

         // Write IL to the listing file.
         
         // Raise the function to the symbolic function unit state
         // if it does not already exist.
         bool createdFunctionUnit = false;
         Phx.FunctionUnit functionUnit = functionSymbol.FunctionUnit;
         if (functionUnit == null)
         {
            this.GetMethodDisassembly(functionSymbol,
               Phx.FunctionUnit.SymbolicFunctionUnitState);
            createdFunctionUnit = true;            
         }

         functionUnit = functionSymbol.FunctionUnit;
         if (functionUnit != null && functionUnit.Lister != null)
         {
            functionUnit.Lister.Function();
            
            lines = File.ReadAllLines(GlobalData.ListingFileName);
            StringBuilder stringBuilder = new StringBuilder();
            for (int i = lineCount; i < lines.Length; ++i)
            {
               stringBuilder.AppendLine(lines[i]);       
            }
            string disassembly = stringBuilder.ToString();
            if (disassembly.Length > 0)
            {
               return new Disassembly(disassembly);
            }
         }

         if (createdFunctionUnit)
         {
            this.ReleaseFunctionUnit(functionSymbol);
         }
                  
         return new Disassembly(defaultMessage);
      }
      
      /// <summary>
      /// Builds a Disassembly object that contains the source listing
      /// for the given function symbol.
      /// </summary>
      internal Disassembly GetSourceListing(Phx.Symbols.FunctionSymbol functionSymbol)
      {
         string defaultMessage = "Source listing unavailable";

         if (functionSymbol == null)
         {
            return new Disassembly(defaultMessage);
         }
                  
         // Raise the function to the symbolic function unit state
         // if it does not already exist.
         bool createdFunctionUnit = false;
         Phx.FunctionUnit functionUnit = functionSymbol.FunctionUnit;
         if (functionUnit == null)
         {
            this.GetMethodDisassembly(functionSymbol,
               Phx.FunctionUnit.SymbolicFunctionUnitState);
            createdFunctionUnit = true;
         }

         // Get the source lines from the source file associated
         // with the function.
         functionUnit = functionSymbol.FunctionUnit;
         if (functionUnit != null && 
             functionUnit.FirstInstruction != null &&
             functionUnit.LastInstruction != null)
         {                  
            Phx.IR.Instruction firstInstruction = 
               functionUnit.FirstInstruction;
            Phx.IR.Instruction lastInstruction =
               functionUnit.LastInstruction;
             
            // Get source file name.
            string sourceFileName = 
               firstInstruction.GetFileName().NameString;
               
            // Get source code for the file.
            string[] sourceFileListing = this.GetCachedSourceListing(
               sourceFileName);

            // Read the source listing if the code is available.
            if (sourceFileListing != null)
            {  
               // Because line numbers are 1-based, but the 
               // string array is 0-based, subtract 1 from
               // each of the line numbers.
               return this.ReadSourceListing(sourceFileListing,
                  (int)(firstInstruction.GetLineNumber()) - 1,
                  (int)(lastInstruction.GetLineNumber()) - 1);
            }

            if (createdFunctionUnit)
            {
               this.ReleaseFunctionUnit(functionSymbol);
            }
         }

         return new Disassembly(defaultMessage);
      }

      /// <summary>
      /// Retrieves the cached source listing at the provided file path.
      /// If a cached version does not exist, this method will read it 
      /// from file and add it to the cache.
      /// </summary>
      private string[] GetCachedSourceListing(string sourceFileName)
      {
         string[] sourceListing = null;
                           
         if (sourceFileName != null)
         {
            // Lookup file name alias.
            string sourceFileAlias;
            if (this.sourceFileAliasMap.TryGetValue(sourceFileName,
               out sourceFileAlias))
            {
               sourceFileName = sourceFileAlias;
            }
               
            // Try to obtain the sources from the cache.
            if (this.sourceFileMap.TryGetValue(
               sourceFileName, out sourceListing))
            {            
               return sourceListing;
            }

            // If the file does not exist, ask the user to browse
            // to its location on disk. This is common in the case where
            // the module was built on a different machine with a 
            // different source path.
            if (! File.Exists(sourceFileName))
            {
               // Allow the user to browse to the file on disk.
               if (this.fileBrowser != null)
               {
                  string result = this.fileBrowser.Browse(sourceFileName);
                  if (File.Exists(result))
                  {
                     // Add binding to new source file.
                     this.sourceFileAliasMap.Add(sourceFileName, result);
                     sourceFileName = result;
                  }
               }
            }

            // If the file exists, read the file and add it to the cache.
            if (File.Exists(sourceFileName))
            {
               sourceListing = File.ReadAllLines(sourceFileName);
               this.sourceFileMap.Add(sourceFileName, sourceListing);
            }
         }
         return sourceListing;
      }

      /// <summary>
      /// Reads all lines within the given line number range 
      /// from the given source listing.
      /// </summary>
      private Disassembly ReadSourceListing(string[] sourceListing, 
         int firstLineNumber, int lastLineNumber)
      {
         string csBeginScope = "{";
         string csEndScope = "}";
         string vbEndSub = "End Sub";
         
         // Because the framework provides the line number to the first
         // instruction, read back through the file until we find
         // the end of the previous method, the opening of an enclosing
         // scope, or the beginning of the file.
         // This technique will also give us any source comments or 
         // attributes associated with the method.
         int beginScopeCount = 0;
         while (firstLineNumber > 0)
         {
            string line = sourceListing[firstLineNumber];

            if (line.Contains(csBeginScope) && line.Contains(csEndScope))
            {
               --firstLineNumber;
               continue;
            }

            if (line.Contains(csBeginScope))
            {
               ++beginScopeCount;
               if (beginScopeCount == 2)
               {
                  line = csEndScope;
               }             
            }
            
            if (line.Contains(csEndScope) ||                 
                line.Contains(vbEndSub))
            {
               // Skip current line.
               ++firstLineNumber;
               
               // Now skip all empty lines.
               while (firstLineNumber < lastLineNumber)
               {
                  if (sourceListing[firstLineNumber].Trim().Length > 0)
                  {
                     break;                     
                  }
                  ++firstLineNumber;
               }
               break;
            }
            --firstLineNumber;
         }
         
         // The framework also provides the line number to the last
         // instruction. Therefore, read until we find the method closer.
         while (lastLineNumber < sourceListing.Length - 1)
         {
            string line = sourceListing[lastLineNumber];
            if (line.Contains(csEndScope) ||
                line.Contains(vbEndSub))
            {
               break;
            }
            ++lastLineNumber;
         }
         
         // Append all lines between first and last line number
         // into a string builder.

         StringBuilder stringBuilder = new StringBuilder();
         int lineCount = lastLineNumber - firstLineNumber + 1;
         int[] lineNumbers = new int[lineCount];
         int[] lineOffsets = new int[lineCount];
         for (int i = firstLineNumber; i <= lastLineNumber; i++)
         {
            stringBuilder.AppendLine(sourceListing[i]);
            
            // Track line numbers, and ignore offsets.            
            lineNumbers[i - firstLineNumber] = (int)i + 1;
            lineOffsets[i - firstLineNumber] = -1;
         }

         return new Disassembly(stringBuilder.ToString(), 
            lineNumbers, lineOffsets);
      }

      /// <summary>
      /// Builds a Disassembly object for the given ElementNode object.
      /// </summary>
      internal void BuildDisassembly(ElementNode elementNode, 
         Phx.FunctionUnitState raiseToState)
      {
         Disassembly disassembly = null;
         
         // The Disassembly object for Manifest and Namespace nodes
         // do have associated symbol objects.

         if (elementNode.ElementType == ElementType.Manifest)
         {
            elementNode.Disassembly =
               new Disassembly(this.manifestBuilder.ToString());
            return;
         }
         if (elementNode.ElementType == ElementType.Namespace)
         {
            elementNode.Disassembly =
               new Disassembly(string.Format(
                  "namespace {0}{1}{{{1}}} // end of namespace {0}",
                  elementNode.Name, Environment.NewLine));
            return;
         }

         Phx.Symbols.Symbol symbol = elementNode.Symbol;
         if (symbol == null)
         {
            return;
         }
         
         // Build a Disassembly object based on the SymbolKind of 
         // the symbol.

         switch (symbol.SymbolKind)
         {
            case Phx.Symbols.SymbolKind.Attribute:
               disassembly =
                  new Disassembly(this.GetCustomAttributeSignature(
                     symbol.AsAttributeSymbol));
               break;

            case Phx.Symbols.SymbolKind.Event:
               disassembly = this.GetEventDisassembly(symbol.AsEventSymbol);
               break;

            case Phx.Symbols.SymbolKind.Field:
               disassembly = this.GetFieldDisassembly(symbol.AsFieldSymbol);
               break;
               
            case Phx.Symbols.SymbolKind.Function:
               disassembly =
                  this.GetMethodDisassembly(symbol.AsFunctionSymbol,
                     raiseToState);
               break;

            case Phx.Symbols.SymbolKind.GlobalVariable:
               disassembly = this.GetGlobalDisassembly(
                  symbol.AsGlobalVariableSymbol);
               break;
               
            case Phx.Symbols.SymbolKind.Type:
            case Phx.Symbols.SymbolKind.MsilType:
               disassembly =
                  this.GetTypeDisassembly(symbol.AsTypeSymbol,
                     elementNode.Name, elementNode.Signature);
               break;

            case Phx.Symbols.SymbolKind.Property:
               disassembly = this.GetPropertyDisassembly(
                  symbol.AsPropertySymbol);
               break;
            
            case Phx.Symbols.SymbolKind.StaticField:
               disassembly = this.GetStaticFieldDisassembly(
                  symbol.AsStaticFieldSymbol);
               break;
         }

         elementNode.Disassembly = disassembly;
      }
            
      /// <summary>
      /// Creates a Disassembly object that describes the provided
      /// type symbol.
      /// </summary>
      private Disassembly GetTypeDisassembly(Phx.Symbols.TypeSymbol typeSymbol,
         string className, string[] msilSignature)
      {
         StringBuilder stringBuilder = new StringBuilder();
         string indent = string.Empty;

         foreach (string info in msilSignature)
         {
            stringBuilder.AppendFormat("{0}{1}", indent, info);
            if (indent.Length == 0)
            {
               stringBuilder.AppendFormat(" {0}", className);
               indent = "       ";
            }
            stringBuilder.AppendLine();
         }

         stringBuilder.AppendFormat("{{{0}}} // end of class {1}",
            Environment.NewLine, className);

         return new Disassembly(stringBuilder.ToString());
      }
         
      /// <summary>
      /// Creates a Disassembly object that describes the provided
      /// property symbol.
      /// </summary>
      private Disassembly GetPropertyDisassembly(
         Phx.Symbols.PropertySymbol propertySymbol)
      {
         StringBuilder stringBuilder = new StringBuilder();
         
         stringBuilder.Append(".property");
         
         string propertyName = Driver.GetFormattedName(
            propertySymbol.NameString);
            
         // Append the 'instance' token if either the 'getter' or 
         // 'setter' property method is an instance method.

         Phx.Types.FunctionType functionType = null;
         if (propertySymbol.FunctionSymbolGetter != null)
         {
            functionType = propertySymbol.FunctionSymbolGetter.FunctionType;
         }
         else if (propertySymbol.FunctionSymbolSetter != null)
         {
            functionType = propertySymbol.FunctionSymbolSetter.FunctionType;
         }
         if (functionType == null || functionType.IsInstanceMethod)
         {
            stringBuilder.Append(" instance");
         }

         // Get the backing type of the property.
         string propertyType = Driver.GetFriendlyTypeName(
            Driver.GetBackingType(propertySymbol), true);
            
         // Append the property type and name.
         stringBuilder.AppendFormat(" {0} {1}",
            propertyType, propertyName);

         // If the property has a 'getter' method, append its 
         // return type in parenthesis.
         if (propertySymbol.FunctionSymbolGetter != null)
         {
            stringBuilder.AppendFormat("({0})", 
               Driver.GetFunctionTypeArgumentList(propertySymbol.
                  FunctionSymbolGetter.FunctionType));
         }
         else
         {
            stringBuilder.Append("()");
         }
                           
         stringBuilder.AppendFormat("{0}{{{0}", Environment.NewLine);

         // Add .set, .get, and .other methods as child nodes.
         
         if (propertySymbol.FunctionSymbolSetter != null)
         {
            stringBuilder.AppendFormat("  .set {0}", 
               Driver.GetPropertyMethodSignature(
                  propertySymbol.FunctionSymbolSetter));
            stringBuilder.AppendLine();
         }
         if (propertySymbol.FunctionSymbolGetter != null)
         {
            stringBuilder.AppendFormat("  .get {0}",
               Driver.GetPropertyMethodSignature(
                  propertySymbol.FunctionSymbolGetter));
            stringBuilder.AppendLine();
         }         
         if (propertySymbol.OtherFunctionSymbolList != null)
         {
            foreach (Phx.Symbols.FunctionSymbol otherFunctionSymbol 
               in  propertySymbol.OtherFunctionSymbolList)
            {
               stringBuilder.AppendFormat("  .other {0}",
                  Driver.GetPropertyMethodSignature(otherFunctionSymbol));
               stringBuilder.AppendLine();
            }                     
         }
                  
         stringBuilder.AppendFormat("}} // end of property {0}::{1}",
            Driver.GetClassName(
               propertySymbol.LexicalParentSymbol.QualifiedName), 
               propertyName);

         return new Disassembly(stringBuilder.ToString());      
      }

      /// <summary>
      /// Creates a string that describes the signature of the 
      /// given property method.
      /// </summary>
      private static string GetPropertyMethodSignature(
         Phx.Symbols.FunctionSymbol functionSymbol)
      {
         StringBuilder stringBuilder = new StringBuilder();

         Phx.Types.FunctionType functionType = functionSymbol.FunctionType;

         // Append 'instance' token.
         if (functionType.IsInstanceMethod)
         {
            stringBuilder.Append("instance ");
         }

         // Append return type.
         stringBuilder.AppendFormat("{0} ", Driver.GetFriendlyTypeName(
            functionType.ReturnType, true));

         // Append fully-qualified name.
         stringBuilder.AppendFormat("{0}::{1}",
            functionSymbol.LexicalParentSymbol.QualifiedName,
            Driver.GetFormattedName(functionSymbol.NameString));

         // Append parameter types.
         stringBuilder.Append("(");
         stringBuilder.Append(Driver.GetFunctionTypeArgumentList(functionType));
         stringBuilder.Append(")");

         return stringBuilder.ToString();
      }

      /// <summary>
      /// Creates a string that describes the signature of the 
      /// given event method.
      /// </summary>
      private static string GetEventMethodSignature(
         Phx.Symbols.FunctionSymbol functionSymbol)
      {
         StringBuilder stringBuilder = new StringBuilder();

         Phx.Types.FunctionType functionType = functionSymbol.FunctionType;

         // Append 'instance' or 'static' token.
         if (functionType.IsInstanceMethod)
         {
            stringBuilder.Append("instance ");
         }
         else
         {
            stringBuilder.Append("static ");
         }
         
         // Append return type.
         stringBuilder.AppendFormat("{0} ", Driver.GetFriendlyTypeName(
            functionType.ReturnType, true));

         // Append method name and parameter list.
         stringBuilder.AppendFormat("{0}(", Driver.GetFormattedName(
            functionSymbol.QualifiedName));

         string comma = string.Empty;

         foreach (Phx.Types.Parameter parameter in functionType.Parameters)
         {
            string argumentName = string.Empty;

            // Skip 'this' pointer.
            if (parameter.IsThisPointer)
            {
            }           
            else if (!parameter.IsReturnPointer)
            {
               if (parameter.IsEllipsis)
               {
                  argumentName += "...";
               }
               else
               {
                  argumentName = string.Format("{0} {1}",
                     Driver.GetClassTypeString(parameter.Type),
                     Driver.GetFriendlyTypeName(parameter.Type, true));
               }
            }

            if (argumentName.Length > 0)
            {
               stringBuilder.AppendFormat("{0}{1}", comma, argumentName);

               if (comma.Length == 0)
                  comma = ", ";
            }
         }

         stringBuilder.Append(")");

         return stringBuilder.ToString();
      }

      /// <summary>
      /// Creates a Disassembly object that describes the provided
      /// event symbol.
      /// </summary>
      private Disassembly GetEventDisassembly(
         Phx.Symbols.EventSymbol eventSymbol)
      {
         string eventName = Driver.GetFormattedName(eventSymbol.NameString);
         string eventType = Driver.GetFriendlyTypeName(eventSymbol.EventType,
            true);
         
         StringBuilder stringBuilder = new StringBuilder();
                  
         stringBuilder.AppendFormat(".event {1} {0}", eventName, eventType);
         stringBuilder.AppendFormat("{0}{{{0}", Environment.NewLine);

         // Append .removeon, .addon, .fire, and .other methods.
         
         if (eventSymbol.FunctionSymbolRemoveOn != null)
         {
            stringBuilder.AppendFormat("  .removeon {0}", 
               Driver.GetEventMethodSignature(
                  eventSymbol.FunctionSymbolRemoveOn));
            stringBuilder.AppendLine();
         }
         if (eventSymbol.FunctionSymbolAddOn != null)
         {
            stringBuilder.AppendFormat("  .addon {0}",
               Driver.GetEventMethodSignature(
                  eventSymbol.FunctionSymbolAddOn));
            stringBuilder.AppendLine();
         }         
         if (eventSymbol.FunctionSymbolFire != null)
         {
            stringBuilder.AppendFormat("  .fire {0}",
               Driver.GetEventMethodSignature(
                  eventSymbol.FunctionSymbolFire));
            stringBuilder.AppendLine();
         }
         if (eventSymbol.OtherFunctionSymbolList != null)
         {
            foreach (Phx.Symbols.FunctionSymbol otherFunctionSymbol
               in eventSymbol.OtherFunctionSymbolList)
            {
               stringBuilder.AppendFormat("  .other {0}",
                  Driver.GetEventMethodSignature(otherFunctionSymbol));
               stringBuilder.AppendLine();
            }
         }

         stringBuilder.AppendFormat("}} // end of event {0}::{1}",
            Driver.GetClassName(eventSymbol.LexicalParentSymbol.QualifiedName),
               eventName);

         return new Disassembly(stringBuilder.ToString());
      }
      
      /// <summary>
      /// Retrieves an array of custom attribute symbols associated 
      /// with the given symbol.
      /// </summary>
      private Phx.Symbols.AttributeSymbol[] GetCustomAttributes(
         Phx.Symbols.Symbol symbol)
      {        
         // If the symbol has an attribute symbol list, add 
         // each custom attribute symbol in the list to our own list.
         if (symbol.AttributeSymbolList != null)
         {
            List<Phx.Symbols.AttributeSymbol> symbols = 
               new List<Phx.Symbols.AttributeSymbol>();
               
            foreach (Phx.Symbols.Symbol attributeSymbol
               in symbol.AttributeSymbolList)
            {
               if (attributeSymbol.IsAttributeSymbol &&
                   attributeSymbol.AsAttributeSymbol.IsCustom)
               {
                  symbols.Add(attributeSymbol.AsAttributeSymbol);
               }
            }
            return symbols.ToArray();
         }
         return null;
      }

      /// <summary>
      /// Adds each attribute symbol in the provided symbol array 
      /// to the child list of the given ElementNode object.
      /// </summary>
      private void AddCustomAttributes(ElementNode node, 
         Phx.Symbols.AttributeSymbol[] symbols)
      {
         if (symbols != null)
         {
            foreach (Phx.Symbols.AttributeSymbol attributeSymbol in symbols)
            {
               node.Children.Add(this.BuildElementNode(attributeSymbol));
            }
         }
      }
            
      /// <summary>
      /// Retrieves an array of permission symbols associated 
      /// with the given symbol.
      /// </summary>
      private Phx.Symbols.PermissionSymbol[] GetPermissionAttributes(
         Phx.Symbols.Symbol symbol)
      {
         // If the symbol has an attribute symbol list, add 
         // each custom attribute symbol in the list to our own list.
         if (symbol.AttributeSymbolList != null)
         {
            List<Phx.Symbols.PermissionSymbol> symbols = 
               new List<Phx.Symbols.PermissionSymbol>();
               
            foreach (Phx.Symbols.Symbol attributeSymbol 
               in symbol.AttributeSymbolList)
            {
               if (attributeSymbol.IsPermissionSymbol)
               {
                  symbols.Add(attributeSymbol.AsPermissionSymbol);
               }
            }
            return symbols.ToArray();
         }         
         return null;
      }
      
      /// <summary>
      /// Creates a comma-separated string that contains each argument
      /// associated with the given function type.
      /// </summary>
      private static string GetFunctionTypeArgumentList(
         Phx.Types.FunctionType functionType)
      {
         StringBuilder stringBuilder = new StringBuilder();

         string comma = string.Empty;

         // Append each argument in the function argument list to the
         // string builder.
         foreach (Phx.Types.Parameter parameter in functionType.Parameters)
         {
            // Skip 'this' pointer.
            if (!parameter.IsThisPointer)
            {
               // Handle ellipsis arguments.
               if (parameter.IsEllipsis)
               {
                  stringBuilder.Append("...");
               }
               // Handle all other arguments.
               else
               {
                  string argumentType = Driver.GetFriendlyTypeName(
                     parameter.Type, true);

                  stringBuilder.AppendFormat("{0}{1}", comma, argumentType);
               }

               if (comma.Length == 0)
                  comma = ", ";
            }
         }

         return stringBuilder.ToString();
      }

      /// <summary>
      /// Creates a Disassembly object that describes the provided
      /// static field symbol.
      /// </summary>
      private Disassembly GetStaticFieldDisassembly(
         Phx.Symbols.StaticFieldSymbol fieldSymbol)
      {         
         StringBuilder stringBuilder = new StringBuilder();

         // Append field token and access level.
         stringBuilder.AppendFormat(".field {0} static", 
            fieldSymbol.Access.ToString().ToLower());

         // Append field-specific attribute tokens.
         
         if (fieldSymbol.IsInitializeOnly)
         {
            stringBuilder.Append(" initonly");
         }

         if (fieldSymbol.IsLiteral)
         {
            stringBuilder.Append(" literal");
         }

         if (fieldSymbol.IsNotSerialized)
         {
            stringBuilder.Append(" notserialized");
         }

         if (fieldSymbol.IsSpecialName)
         {
            stringBuilder.Append(" specialname");
         }

         if (fieldSymbol.IsRuntimeSpecialName)
         {
            stringBuilder.Append(" rtspecialname");
         }

         // Look for marshal attribute in extension object list.

         if (fieldSymbol.HasExtensionObjects)
         {
            string marshalString = Driver.GetMarshalString(
               Driver.GetMarshalAttribute(fieldSymbol));
            if (marshalString.Length > 0)
            {
               stringBuilder.AppendFormat(" {0}", marshalString);
            }
         }
         
         // Append class type string.        

         string classString = Driver.GetClassTypeString(fieldSymbol.Type);
         if (classString.Length > 0)
         {
            stringBuilder.AppendFormat(" {0}", classString);
         }

         stringBuilder.AppendFormat(" {0} {1}",
            Driver.GetFriendlyTypeName(fieldSymbol.Type, true), 
            Driver.GetFormattedName(fieldSymbol.NameString));

         // For mapped fields, display data location.

         if (fieldSymbol.AllocationBaseSectionSymbol != null)
         {
            char prefix = 'D';
            if (fieldSymbol.AllocationBaseSectionSymbol.IsTls)
            {
               prefix = 'T';
            }

            stringBuilder.AppendFormat(" at {0}_{1}", 
               prefix, fieldSymbol.Rva.ToString("X8"));
         }

         return new Disassembly(stringBuilder.ToString());
      }

      /// <summary>
      /// Generates a formatted string that describes the given 
      /// static field symbol.
      /// </summary>
      private string GetStaticFieldDisplayString(
         Phx.Symbols.StaticFieldSymbol fieldSymbol)
      {
         StringBuilder stringBuilder = new StringBuilder();

         // Append field token and access level.
         
         stringBuilder.AppendFormat("field {0} : {1} static", 
            Driver.GetFormattedName(fieldSymbol.NameString),
            fieldSymbol.Access.ToString().ToLower());

         // Append field-specific attribute tokens.
         
         if (fieldSymbol.IsInitializeOnly)
         {
            stringBuilder.Append(" initonly");
         }

         if (fieldSymbol.IsLiteral)
         {
            stringBuilder.Append(" literal");
         }

         if (fieldSymbol.IsNotSerialized)
         {
            stringBuilder.Append(" notserialized");
         }

         if (fieldSymbol.IsSpecialName)
         {
            stringBuilder.Append(" specialname");
         }

         if (fieldSymbol.IsRuntimeSpecialName)
         {
            stringBuilder.Append(" rtspecialname");
         }

         // Look for marshal attribute in extension object list.

         if (fieldSymbol.HasExtensionObjects)
         {
            string marshalString = Driver.GetMarshalString(
               Driver.GetMarshalAttribute(fieldSymbol));
            if (marshalString.Length > 0)
            {
               stringBuilder.AppendFormat(" {0}", marshalString);
            }
         }

         // Append the 'valuetype' token if the type of the symbol
         // is a Msil value type.
                          
         if (fieldSymbol.Type.IsAggregateType)
         {
            if (IsMsilValueType(fieldSymbol.Type.AsAggregateType))
            {
               stringBuilder.Append(" valuetype");
            }
         }
         
         // Append the type name.
         
         stringBuilder.AppendFormat(" {0}",            
            Driver.GetFriendlyTypeName(fieldSymbol.Type, true));
         
         // For mapped fields, display data location.

         if (fieldSymbol.AllocationBaseSectionSymbol != null)
         {
            char prefix = 'D';
            if (fieldSymbol.AllocationBaseSectionSymbol.IsTls)
            {
               prefix = 'T';
            }
            
            stringBuilder.AppendFormat(" at {0}_{1}", 
               prefix, fieldSymbol.Rva.ToString("X8"));
         }

         return stringBuilder.ToString();  
      }

      /// <summary>
      /// Generates a formatted string that describes the given 
      /// global variable symbol.
      /// </summary>
      private string GetGlobalDisplayString(
         Phx.Symbols.GlobalVariableSymbol globalVariableSymbol)
      {
         // Always attempt to display field token and module reference.

         Phx.Symbols.Symbol importModuleSymbol = 
            globalVariableSymbol.LexicalParentSymbol;
         while (importModuleSymbol != null &&
            !importModuleSymbol.IsImportModuleSymbol)
         {
            importModuleSymbol = importModuleSymbol.LexicalParentSymbol;
         }

         if (importModuleSymbol != null)
         {
            return string.Format("field {0} [.module {1}]::{2}",
               Driver.GetFriendlyTypeName(globalVariableSymbol.Type, true),
               Path.GetFileName(importModuleSymbol.NameString),
               Driver.GetFormattedName(globalVariableSymbol.NameString));            
         }
         else
         {
            return string.Format("field {0} {1}",
               Driver.GetFriendlyTypeName(globalVariableSymbol.Type, true),
               Driver.GetFormattedName(globalVariableSymbol.NameString));            
         }
      }

      /// <summary>
      /// Creates a Disassembly object that describes the provided
      /// global variable symbol.
      /// </summary>
      private Disassembly GetGlobalDisassembly(
         Phx.Symbols.GlobalVariableSymbol globalVariableSymbol)
      {
         StringBuilder stringBuilder = new StringBuilder();

         // Append field token and access level.

         stringBuilder.Append(".field public static");

         // Append the 'valuetype' token if the type of the symbol
         // is a Msil value type.

         if (globalVariableSymbol.Type.IsAggregateType)
         {
            if (IsMsilValueType(globalVariableSymbol.Type.AsAggregateType))
            {
               stringBuilder.Append(" valuetype");
            }
         }

         // Append the type name.

         stringBuilder.AppendFormat(" {0}",
            Driver.GetFriendlyTypeName(globalVariableSymbol.Type, true));

         // Append module reference if the global comes from another module.

         if (globalVariableSymbol.LexicalParentSymbol != null)
         {
         }

         // Append variable name.

         stringBuilder.AppendFormat(" {0}",
            Driver.GetFormattedName(globalVariableSymbol.NameString));

         // For mapped fields, display data location.

         if (globalVariableSymbol.AllocationBaseSectionSymbol != null)
         {
            char prefix = 'D';
            if (globalVariableSymbol.AllocationBaseSectionSymbol.IsTls)
            {
               prefix = 'T';
            }

            stringBuilder.AppendFormat(" at {0}_{1}",
               prefix, globalVariableSymbol.Rva.ToString("X8"));
         }

         return new Disassembly(stringBuilder.ToString());
      }

      /// <summary>
      /// Generates a formatted string that describes the given 
      /// field symbol.
      /// </summary>
      private string GetFieldDisplayString(
         Phx.Symbols.FieldSymbol fieldSymbol)
      {
         StringBuilder stringBuilder = new StringBuilder();

         // Append field token, name, and access level.

         stringBuilder.AppendFormat("field {0} : {1}", 
            Driver.GetFormattedName(fieldSymbol.NameString), 
            fieldSymbol.Access.ToString().ToLower());

         // Append field-specific attribute tokens.
         
         if (fieldSymbol.IsInitializeOnly)
         {
            stringBuilder.Append(" initonly");
         }

         if (fieldSymbol.IsLiteral)
         {
            stringBuilder.Append(" literal");
         }

         if (fieldSymbol.IsNotSerialized)
         {
            stringBuilder.Append(" notserialized");
         }

         if (fieldSymbol.IsSpecialName)
         {
            stringBuilder.Append(" specialname");
         }

         if (fieldSymbol.IsRuntimeSpecialName)
         {
            stringBuilder.Append(" rtspecialname");
         }

         // Look for marshal attribute in extension object list.

         if (fieldSymbol.HasExtensionObjects)
         {
            string marshalString = Driver.GetMarshalString(
               Driver.GetMarshalAttribute(fieldSymbol));
            if (marshalString.Length > 0)
            {
               stringBuilder.AppendFormat(" {0}", marshalString);
            }
         }

         // Append class type string.   
         
         string classString = Driver.GetClassTypeString(fieldSymbol.Type);
         if (classString.Length > 0)
         {
            stringBuilder.AppendFormat(" {0}", classString);
         }

         // Append the type name.

         stringBuilder.AppendFormat(" {0}",
            GetFriendlyTypeName(fieldSymbol.Type, true));

         return stringBuilder.ToString();
      }

      /// <summary>
      /// Creates a Disassembly object that describes the provided
      /// field symbol.
      /// </summary>
      private Disassembly GetFieldDisassembly(
         Phx.Symbols.FieldSymbol fieldSymbol)
      {         
         StringBuilder stringBuilder = new StringBuilder();

         // Append field token and access level.
         stringBuilder.AppendFormat(".field {0}", 
            fieldSymbol.Access.ToString().ToLower());

         // Append field-specific attribute tokens.
         
         if (fieldSymbol.IsInitializeOnly)
         {
            stringBuilder.Append(" initonly");
         }

         if (fieldSymbol.IsLiteral)
         {
            stringBuilder.Append(" literal");
         }
         
         if (fieldSymbol.IsNotSerialized)
         {
            stringBuilder.Append(" notserialized");
         }

         if (fieldSymbol.IsSpecialName)
         {
            stringBuilder.Append(" specialname");
         }

         if (fieldSymbol.IsRuntimeSpecialName)
         {
            stringBuilder.Append(" rtspecialname");
         }

         // Look for marshal attribute in extension object list.

         if (fieldSymbol.HasExtensionObjects)
         {
            string marshalString = Driver.GetMarshalString(
               Driver.GetMarshalAttribute(fieldSymbol));
            if (marshalString.Length > 0)
            {
               stringBuilder.AppendFormat(" {0}", marshalString);
            }
         }
         
         // Append class type string.

         string classString = Driver.GetClassTypeString(fieldSymbol.Type);
         if (classString.Length > 0)
         {
            stringBuilder.AppendFormat(" {0}", classString);
         }
                  
         stringBuilder.AppendFormat(" {0} {1}",
            Driver.GetFriendlyTypeName(fieldSymbol.Type, true),
            Driver.GetFormattedName(fieldSymbol.NameString));

         return new Disassembly(stringBuilder.ToString());
      }

      /// <summary>
      /// Retrieves the class name from the provided fully-qualified
      /// name.
      /// </summary>
      private static string GetClassName(string fullName)
      {
         // Get the index of the last occurance of the dot character.
         int index = fullName.LastIndexOf('.');
         
         // Do not trim the name if the leading character is a dot.
         if (index > 0)
         {
            // Compensate for the method names '.ctor' and .cctor' 
            // (for example, 'A..ctor').
            // In these cases we want to preserve the dot character.
            if (fullName[index - 1] == '.')
               index--;

            return fullName.Substring(index + 1);
         }
         return fullName;
      }
      
      /// <summary>
      /// Retrieves the class category of the given type. 
      /// </summary>
      private static string GetClassTypeString(Phx.Types.Type type)
      {
         // Because System.Object and System.String are special types,
         // do not return the 'class' category.
         if (type.Equals(type.TypeTable.SystemObjectAggregateType) ||
             type.Equals(type.TypeTable.SystemStringAggregateType))
         {
            return string.Empty;
         }

         // For managed array types, return the category of the element type.
         if (type.IsManagedArrayType)
         {
            return Driver.GetClassTypeString(
               type.AsManagedArrayType.ElementType);
         }
         // For pointer types, return the category of the referent type.
         else if (type.IsPointerType)
         {
            return Driver.GetClassTypeString(
               type.AsPointerType.ReferentType);
         }
         // For aggregate types, return the 'class' or 'valuetype' category.
         else if (type.IsAggregateType)
         {
            Phx.Types.AggregateType aggregateType = type.AsAggregateType;
         
            if (IsMsilClass(aggregateType))
            {
               return "class";
            }
            return "valuetype";
         }
         
         // For primitive or other types, return the empty string.
         return string.Empty;
      }

      /// <summary>
      /// Determines whether the provided aggregate type is a 
      /// Msil class.
      /// </summary>
      private static bool IsMsilClass(Phx.Types.AggregateType type)
      {
         return type.IsSelfDescribing;
      }

      /// <summary>
      /// Determines whether the provided aggregate type is a 
      /// Msil valuetype.
      /// </summary>
      private static bool IsMsilValueType(Phx.Types.AggregateType type)
      {
         if (type.IsNonSelfDescribingAggregate)
         {
            return true;
         }
         
         return type.IsPrimary && type.IsSealed &&
                !type.IsSelfDescribing;
      }

      /// <summary>
      /// Determines whether the provided aggregate type is a 
      /// Msil interface.
      /// </summary>
      private static bool IsMsilInterface(Phx.Types.AggregateType type)
      {
         return type.IsOnlyInterface;
      }

      /// <summary>
      /// Determines whether the provided aggregate type is a 
      /// Msil enum.
      /// </summary>
      private static bool IsMsilEnum(Phx.Types.AggregateType type)
      {
         return type.IsPrimary && type.IsSealed &&
                type.IsEnum && !type.IsSelfDescribing;
      }

      /// <summary>
      /// Retrieves the "friendly" name of the given type.       
      /// </summary>
      internal static string GetFriendlyTypeName(
         Phx.Types.Type type, bool asLeafType)
      {         
         // Type names that the framework produces in the IR differ 
         // from what we see in MSIL. For example, this method 
         // might translate the type name
         // 'object32->MgdArr[object32->Aggregate?]' to 
         // 'string[]'
         
         string typeName = string.Empty;
         
         if (asLeafType)
         {
            // Return special strings for System.Object and System.String.

            Phx.Types.Table globalTypeTable = Phx.GlobalData.TypeTable;

            if (type.Equals(globalTypeTable.SystemObjectAggregateType))
            {
               return "object";
            }
            if (type.Equals(globalTypeTable.SystemStringAggregateType))
            {
               return "string";
            }
         }

         // For pointer types, get the name of the referent type.
         // Also append reference or pointer qualifier for 
         // managed and unmanaged pointer types.
         if (type.IsPointerType)
         {
            Phx.Types.Type referenceType = 
               type.AsPointerType.ReferentType;
               
            if (type.IsObjectPointer)
            {
               // Return the name of the referent type. 
               // No additional type decoration is required.
               typeName = Driver.GetFriendlyTypeName(referenceType, true);
            }
          
            else if (type.IsManagedPointer)
            {
               // We use "&" to designate a managed pointer type.
               typeName = string.Concat(
                  Driver.GetFriendlyTypeName(referenceType, true), "&");
            }
            else if (type.IsUnmanagedPointer)
            {
               // We use "*" to designate a native pointer type.
               typeName = string.Concat(
                  Driver.GetFriendlyTypeName(referenceType, true), "*");
            }
         }

         // For array types, get the name of the element type, followed
         // by square brackets.
         else if (type.IsManagedArrayType)
         {
            typeName = string.Concat(
               Driver.GetFriendlyTypeName(type.AsManagedArrayType.ElementType,
                  asLeafType), "[]");
         }
         // For array types, get the name of the element type, followed
         // by square brackets.
         else if (type.IsUnmanagedArrayType)
         {
            typeName = string.Concat(
               Driver.GetFriendlyTypeName(type.AsUnmanagedArrayType.ElementType,
                  asLeafType), "[]");
         }
         
         // For aggregate types, form a fully-qualified name string.
         else if (type.IsAggregateType)
         {
            // Get the short name of the type. If the type does not
            // have a corresponding type symbol, we use 
            // the ToString() representation of the type.
            if (type.TypeSymbol != null)
               typeName += type.TypeSymbol.NameString;
            else
               typeName += type.ToString();

            // If the type has a type symbol, get the fully-qualified
            // name of its parent by using the LexicalParentSymbol
            // property.
            if (type.TypeSymbol != null)
            {
               Phx.Symbols.Symbol referenceSymbol = 
                  type.TypeSymbol.LexicalParentSymbol;

               if (referenceSymbol != null)
               {
                  switch (referenceSymbol.SymbolKind)
                  {
                     // If the parent is a type symbol, join the names 
                     // together with the dot character.
                     case Phx.Symbols.SymbolKind.Type:
                     case Phx.Symbols.SymbolKind.MsilType:
                     {
                        typeName = Driver.GetFriendlyTypeName(
                           referenceSymbol.AsTypeSymbol.Type, false) +
                              "." + typeName;
                     }
                     break;
                     
                     // If the parent is an assembly symbol, prepend the name
                     // of the type with the name of the assembly.
                     case Phx.Symbols.SymbolKind.Assembly:
                     {
                        typeName = "[" + referenceSymbol.NameString + "]" + 
                           typeName;
                     }
                     break;
                     
                     // Do not annotate the name string if the parent is a 
                     // reference to the current assembly.
                     case Phx.Symbols.SymbolKind.ImportModule:
                     {
                     }
                     break;

                     // If the parent is a namespace symbol, concatenate
                     // the namespace name with the type name.
                     case Phx.Symbols.SymbolKind.Namespace:
                     {
                        typeName = string.Concat(referenceSymbol.NameString, 
                           typeName);
                     }
                     break;
                     
                     // We do no expect to reach the default case, but
                     // append a prefix string if we do reach this case.
                     default:
                     {
                        typeName = string.Concat("[???]", typeName);
                     }
                     break;
                  }
               }
            }

            // If the aggregate type is generic, append the formatted
            // type variable type list string.
            Phx.Types.AggregateType aggregateType = type.AsAggregateType;
            if (aggregateType.IsGeneric)
            {
               typeName = string.Concat(typeName, 
                  Driver.FormatTypeVariableTypeList(
                     aggregateType.TypeVariableTypeList));
            }
         }
         // For primitive types, translate the framework type 
         // to the appropriate keyword.
         else if (type.IsPrimitiveType)
         {
            string primitiveTypeName = string.Empty;

            // Generate the MSIL type name for the type by using
            // the PrimitiveTypeKind property.
            switch (type.PrimitiveTypeKind)
            {
               case Phx.Types.PrimitiveTypeKind.Boolean:
                  primitiveTypeName = "bool";
                  break;
               case Phx.Types.PrimitiveTypeKind.Character:
                  primitiveTypeName = "char";
                  break;
               case Phx.Types.PrimitiveTypeKind.Float:
                  primitiveTypeName = "float" + type.BitSize;
                  break;
                  
               // For Int and UInt, if the SizeKind is symbolic,
               // then the type refers to the native integer type
               // of the underlying platform.
               
               case Phx.Types.PrimitiveTypeKind.Int:
                  if (type.SizeKind == Phx.Types.SizeKind.Symbolic)
                  {
                     primitiveTypeName = "native int";
                  }
                  else
                  {
                     Debug.Assert(type.BitSize > 0);
                     primitiveTypeName = "int" + type.BitSize;
                  }
                  break;
               case Phx.Types.PrimitiveTypeKind.UInt:
                  if (type.SizeKind == Phx.Types.SizeKind.Symbolic)
                  {
                     primitiveTypeName = "native uint";
                  }
                  else
                  {
                     Debug.Assert(type.BitSize > 0);
                     primitiveTypeName = "uint" + type.BitSize;
                  }                  
                  break;

               case Phx.Types.PrimitiveTypeKind.Void:
                  primitiveTypeName = "void";
                  break;
                  
               case Phx.Types.PrimitiveTypeKind.Unknown:
                  primitiveTypeName = "[unk]";
                  break;
            }
            typeName += primitiveTypeName;
         }
         // For function types, generate a string with the format
         // 'method <return_type> *(<argument_list>)'.
         else if (type.IsFunctionType)
         {
            Phx.Types.FunctionType functionType = type.AsFunctionType;

            typeName = string.Format("method {0} *({1})",
               Driver.GetFriendlyTypeName(functionType.ReturnType, false),
               Driver.GetFunctionTypeArgumentList(functionType)
            );
         }
         // For type variable types, append the appropriate parent
         // specifier followed by the type name.
         else if (type.IsTypeVariableType)
         {
            Phx.Types.TypeVariableType typeVariableType =
               type.AsTypeVariableType;

            string parentSpecifier = string.Empty;
            Phx.Collections.TypeVariableTypeList typeList = null;
            if (typeVariableType.ParentAggregateType != null)
            {
               typeList = 
                  typeVariableType.ParentAggregateType.TypeVariableTypeList;
               parentSpecifier = "!";
            }
            else if (typeVariableType.ParentFunctionSymbol != null)
            {
               typeList =
                  typeVariableType.ParentFunctionSymbol.TypeVariableTypeList;
               parentSpecifier = "!!";
            }

            typeName = string.Format("{0}{1}", parentSpecifier,
               type.TypeSymbol != null ? 
                  type.TypeSymbol.NameString : type.ToString());
         }
         else
         {
            Debug.Assert(false, "Unhandled");
            typeName += string.Format("[{0}]", type.ToString());
         }

         // If the type has any custom modifiers, append them to 
         // the type name string.
         if (type.ModifierList != null)
         {
            string modifiers = string.Empty;
            foreach (Phx.Types.CustomModifier customModifier 
               in type.ModifierList)
            {
               string modifierType = 
                  Driver.GetFriendlyTypeName(customModifier.ModifierType, 
                     asLeafType);

               string modifier = string.Empty;
               switch (customModifier.ModifierKind)
               {
                  case Phx.Types.CustomModifierKind.Optional:
                     modifier = string.Format(" modopt({0})", modifierType);
                     break;
                  case Phx.Types.CustomModifierKind.Required:
                     modifier = string.Format(" modreq({0})", modifierType);
                     break;
                  default:
                     modifier = string.Format(" modunk({0})", modifierType);
                     break;
               }

               modifiers = string.Concat(modifiers, modifier);
            }

            typeName = string.Concat(typeName, modifiers);
         }
         
         return typeName;
      }

      /// <summary>
      /// Creates a formatted string that describes the given
      /// type variable type object.
      /// </summary>
      private static string FormatTypeVariableType(
         Phx.Types.TypeVariableType type)
      {
         StringBuilder stringBuilder = new StringBuilder();         

         // Append constraints.
         
         if (type.IsCovariant)
            stringBuilder.Append("+ ");
         if (type.IsContravariant)
            stringBuilder.Append("- ");
         if (type.IsReferenceType)
            stringBuilder.Append("class ");
         if (type.IsNotNullableValueType)
            stringBuilder.Append("valuetype ");
         if (type.HasDefaultConstructor)
            stringBuilder.Append(".ctor ");

         if (type.GenericConstraintTypeList != null)
         {
            stringBuilder.Append("(");
            string comma = string.Empty;
            
            foreach (Phx.Types.Type constraintType 
               in type.GenericConstraintTypeList)
            {            
               string classString = 
                  Driver.GetClassTypeString(constraintType);
               if (classString.Length > 0)
               {
                  classString = string.Concat(classString, " ");
               }
               
               stringBuilder.AppendFormat("{0}{1}{2}",
                  comma, classString,
                  Driver.GetFriendlyTypeName(constraintType, true));
                  
               if (comma.Length == 0)
                  comma = ", ";
            }
            stringBuilder.Append(") ");
         }

         // Append the short name of the type. If the type does not
         // have a corresponding type symbol, we use the ToString() 
         // representation of the type.         
         if (type.TypeSymbol != null)
            stringBuilder.Append(type.TypeSymbol.NameString);
         else
            stringBuilder.Append(type.ToString());
            
         return stringBuilder.ToString();
      }

      /// <summary>
      /// Creates a formatted string that describes constraints and names
      /// of the members of the provided TypeVariableTypeList object.
      /// </summary>
      private static string FormatTypeVariableTypeList(
         Phx.Collections.TypeVariableTypeList typeVariableTypeList)
      {
         StringBuilder stringBuilder = new StringBuilder();
         
         stringBuilder.Append("<");
         string comma = string.Empty;

         // Append each formatted type variable type string to 
         // the string builder. 
                  
         foreach (Phx.Types.TypeVariableType typeVariableType 
            in typeVariableTypeList)
         {
            stringBuilder.AppendFormat("{0}{1}", 
               comma, Driver.FormatTypeVariableType(typeVariableType));
            if (comma.Length == 0)
               comma = ", ";            
         }
         
         stringBuilder.Append(">");
         return stringBuilder.ToString();
      }

      /// <summary>
      /// Creates a formatted string that describes the signature of the
      /// provided function symbol.
      /// </summary>
      internal string GetMethodSignature(
         Phx.Symbols.FunctionSymbol functionSymbol)
      {
         StringBuilder stringBuilder = new StringBuilder();

         Phx.Types.FunctionType functionType = functionSymbol.FunctionType;

         // Append access level.
         if (functionSymbol.Access != Phx.Symbols.Access.IllegalSentinel)
            stringBuilder.AppendFormat("{0} ", 
               functionSymbol.Access.ToString().ToLower());

         // Append method flags.
         stringBuilder.Append(this.GetMethodFlags(functionSymbol));

         // Append class type string of the return type.
         string classString = 
            Driver.GetClassTypeString(functionType.ReturnType);
         if (classString.Length > 0)
         {
            stringBuilder.AppendFormat("{0} ", classString);
         }

         // Append the 'vararg' token if the method takes a variable 
         // number of arguments.
         if (functionSymbol.FunctionType != null &&
             functionSymbol.FunctionType.IsVariableArguments)
         {
            stringBuilder.Append("vararg ");
         }

         // Append return type.  
         stringBuilder.AppendFormat("{0} ", Driver.GetFriendlyTypeName(
            functionType.ReturnType, true));

         // Append marshal attribute information if the argument for the
         // return value has associated marshaling information.
         
         if (functionSymbol.HasParameterSymbols)
         {
            Phx.Symbols.ParameterSymbol returnSymbol = functionSymbol.ReturnParameterSymbols[0];

            if (returnSymbol.MarshalAttributeSymbol != null)
            {
               string marshalString = string.Concat(
                  Driver.GetMarshalString(
                     returnSymbol.MarshalAttributeSymbol),
                  " ");
   
               stringBuilder.AppendLine();
               stringBuilder.Append("        ");
               stringBuilder.Append(marshalString);
            }
         }

         stringBuilder.AppendLine();
         stringBuilder.Append("        ");

         // Get the formatted type variable type list if the function
         // has associated type variable information.
         
         string typeVariableTypeList = string.Empty;
         if (functionSymbol.TypeVariableTypeList != null)
         {
            typeVariableTypeList = Driver.FormatTypeVariableTypeList(
               functionSymbol.TypeVariableTypeList);
         }

         // Append method name and type variable list.
         stringBuilder.AppendFormat("{0}{1}(", 
            GetFormattedName(functionSymbol.NameString), typeVariableTypeList);

         // Append argument list.
         
         string comma = string.Empty;
         
         // Used to generate names for anonymous parameters.
         int anonymousArgumentNumber = 0; 

         foreach (Phx.Symbols.ParameterSymbol parameterSymbol in functionSymbol.ParameterSymbols)
         {
            string argumentName = string.Empty;

            switch (parameterSymbol.ParameterKind)
            {
               case Phx.Types.ParameterKind.ThisPointer:
               default:
                  break;

               case Phx.Types.ParameterKind.Ellipsis:
               case Phx.Types.ParameterKind.ClrNativeEllipsis:

                  argumentName += "...";
                  break;

               case Phx.Types.ParameterKind.UserDefined:
               {  
                  string inOutOpt = string.Empty;
                  string marshalString = string.Empty;                  
                  string nameString = string.Empty;

                  // Get the category string for the type.
                  classString = Driver.GetClassTypeString(parameterSymbol.Type);
                  if (classString.Length > 0)
                  {
                     classString = string.Concat(classString, " ");
                  }

                  // in, out, optional
                  
                  if (parameterSymbol.IsIn)
                  {
                     inOutOpt = "[in] ";
                  }
                  else if (parameterSymbol.IsOut)
                  {
                     inOutOpt = "[out] ";
                  }
                  if (parameterSymbol.IsOptional)
                  {
                     inOutOpt = "[opt] ";
                  }
                  
                  // Marshaling information
                                                            
                  if (parameterSymbol.MarshalAttributeSymbol != null)
                  {
                     marshalString = string.Concat(
                        Driver.GetMarshalString(
                           parameterSymbol.MarshalAttributeSymbol),
                        " ");
                  }
                  
                  // Name

                  nameString = Driver.GetFormattedName(
                     parameterSymbol.Name.ToString());

                  // If we did not have a FunctionArgument object for the 
                  // argument, create a unique name for the argument.
                  if (nameString.Length == 0)
                  {
                     nameString = string.Format("A_{0}", 
                        anonymousArgumentNumber);
                     anonymousArgumentNumber++;
                  }

                  // Create the formatted string for the argument.
                  argumentName = string.Format("{0}{1}{2} {3}{4}",
                     inOutOpt, classString,
                     Driver.GetFriendlyTypeName(parameterSymbol.Type, true),
                     marshalString, nameString);
               }
               break;
            }

            // Append formatted argument to the argument list.
            if (argumentName.Length > 0)
            {
               stringBuilder.AppendFormat("{0}{1}", comma, argumentName);

               if (comma.Length == 0)
                  comma = ", ";
            }
         }

         stringBuilder.Append(") ");

         // Append implementation flags. We should always see at least
         // "cil" and "managed" attributes.

         stringBuilder.Append(CorFlags.GetCorMethodImplFlags(
            (CorFlags.CorMethodImpl)functionSymbol.ImplementationFlags));

         return stringBuilder.ToString();
      }

      /// <summary>
      /// Creates a Disassembly object that describes the provided
      /// function symbol.
      /// </summary>
      internal Disassembly GetMethodDisassembly(
         Phx.Symbols.FunctionSymbol functionSymbol,
         Phx.FunctionUnitState raiseToState)
      {
         if (raiseToState.Equals(GlobalData.SourceFunctionUnitState))
         {
            return this.GetSourceListing(functionSymbol);
         }
         else if (raiseToState.Equals(GlobalData.IlFunctionUnitState))
         {
            return this.GetIlListing(functionSymbol);
         }
         
         // We don't want to raise a FunctionUnit object that has no 
         // OriginalRva. This includes external P/Invoke calls 
         // in managed applications.
                  
         if (functionSymbol.OriginalRva == 0)
         {
            string disassembly = string.Format(
               ".method {0}{2}{{{2}   // {1} has no body{2}}}",
               this.GetMethodSignature(functionSymbol),                
               Driver.GetFormattedName(functionSymbol.NameString),
               Environment.NewLine);
               
            return new Disassembly(disassembly);
         }
         else
         {
            Phx.FunctionUnit functionUnit = functionSymbol.FunctionUnit;

            if (functionUnit == null)
            {
               // Raise the IR to the provided function unit state.

               // Get the module unit that contains the function symbol.
               Phx.PEModuleUnit moduleUnit =
                  functionSymbol.Table.Unit.AsPEModuleUnit;

               // Raise the function to the provided state.
               functionUnit = moduleUnit.Raise(functionSymbol, raiseToState);
            }                    
            
            // Create a new PhaseList object.

            Phx.Phases.PhaseList phaseList =
               Phx.Phases.PhaseList.New(GlobalData.PhaseConfiguration,
               "PE-Explorer Phases");

            // Append the expression optimizer phase if the function was
            // encoded as MSIL.
            if (functionSymbol.WasEncodedAsMsil && !raiseToState.Equals(
                Phx.FunctionUnit.EncodedIROnlyFunctionUnitState))
            {
               phaseList.AppendPhase(
                  Phx.ExpressionOptimizer.ExpressionOptimizerPhase.New(
                     GlobalData.PhaseConfiguration));
            }

            // Provide IR dump

            IrDisplayPhase displayPhase =
               IrDisplayPhase.New(GlobalData.PhaseConfiguration);
            displayPhase.HeaderText = string.Format(
               ".method {0}{1}{{", 
               this.GetMethodSignature(functionSymbol), 
               Environment.NewLine);
            displayPhase.FooterText = "}";

            phaseList.AppendPhase(displayPhase);
            phaseList.DoPhaseList(functionUnit);
            
            return displayPhase.Disassembly;
         }
      }
      
      /// <summary>
      /// Creates a formatted string that describes the signature of the
      /// provided permission symbol.
      /// </summary>
      internal string GetPermissionSignature(
         Phx.Symbols.PermissionSymbol permissionSymbol)
      {
         StringBuilder stringBuilder = new StringBuilder();
         
         stringBuilder.Append(".permissionset ");

         // Convert the Action token to a string.
         switch (permissionSymbol.Action)
         {
            case 0x0001:
               stringBuilder.Append("request ");
               break;
            case 0x0002:
               stringBuilder.Append("demand ");
               break;
            case 0x0003:
               stringBuilder.Append("assert ");
               break;
            case 0x0004:
               stringBuilder.Append("deny ");
               break;
            case 0x0005:
               stringBuilder.Append("permitonly ");
               break;
            case 0x0006:
               stringBuilder.Append("linkcheck ");
               break;
            case 0x0007:
               stringBuilder.Append("inheritcheck ");
               break;
            case 0x0008:
               stringBuilder.Append("reqmin ");
               break;
            case 0x0009:
               stringBuilder.Append("reqopt ");
               break;
            case 0x000A:
               stringBuilder.Append("reqrefuse ");
               break;
            case 0x000B:
               stringBuilder.Append("prejitgrant ");
               break;
            case 0x000C:
               stringBuilder.Append("prejitdeny ");
               break;
            case 0x000D:
               stringBuilder.Append("noncasdemand ");
               break;
            case 0x000E:
               stringBuilder.Append("noncaslinkdemand ");
               break;
            case 0x000F:
               stringBuilder.Append("noncasinheritance ");
               break;
            default:
               stringBuilder.Append("<invalid> ");
               break;
         }

         // Convert the blob data associated with the permission symbol
         // to a PermissionSet object. Then, create formatted strings
         // for each permission in the set.
         if (permissionSymbol.BlobByteSize > 0)
         {
            byte[] blob = permissionSymbol.Blob;

            if (permissionSymbol.Blob[0] == '.')
            {
               PermissionSet permissionSet = PermissionSet.FromBlob(blob);

               stringBuilder.AppendLine();

               int first = 0;
               int permissionCount = permissionSet.Permissions.Length;
               int last = permissionCount - 1;
               for (int i = 0; i < permissionCount; i++)
               {
                  Permission permission = permissionSet.Permissions[i];

                  if (i == first)
                     stringBuilder.Append("             = {");
                  else
                     stringBuilder.Append("                ");

                  foreach (PermissionProperty property 
                     in permission.PermissionProperties)
                  {
                     stringBuilder.AppendFormat(
                        "{0} = {{property {1} '{2}' = {1}({3})}}",
                        permission.ClassName, property.Type, 
                        property.Name, property.Value);
                  }

                  if (i == last)
                     stringBuilder.Append("}");
                  else
                     stringBuilder.AppendLine(",");
               }
            }
            else
            {
               // The assembly was generated prior to .NET 2.0.
               // Just dump the permissionset byte stream.

               stringBuilder.AppendLine();
               stringBuilder.AppendFormat("             = ( {0})", 
                  Utility.FormatByteArray(blob));

               if (Utility.ContainsPrintableCharacters(blob))
               {
                  stringBuilder.AppendLine();
                  stringBuilder.AppendFormat("             // {0}",
                     Utility.FormatByteArrayAsText(blob));
               }
            }
         }

         return stringBuilder.ToString();
      }

      /// <summary>
      /// Creates a formatted string that describes the signature of the
      /// provided custom attribute symbol.
      /// </summary>
      internal string GetCustomAttributeSignature(
         Phx.Symbols.AttributeSymbol attributeSymbol)
      {
         StringBuilder stringBuilder = new StringBuilder();

         // We could probe the symbol for attributes such as whether it 
         // is an instance method, the name of the attribute, etc. 
         // However, currently ILAsm contrains custom attributes to be 
         // instance constructors.

         stringBuilder.AppendFormat(".custom instance void {0}::.ctor(",
            Driver.GetFriendlyTypeName(attributeSymbol.AttributeType, false));

         // Append argument list types. 

         string separator = string.Empty;
         
         foreach (Phx.Types.Parameter parameter in attributeSymbol.FunctionSymbol.FunctionType.Parameters)
         {
            if (!parameter.IsThisPointer)
            {
               stringBuilder.Append(separator);
               if (separator.Length == 0)
                  separator = ", ";

               Phx.Types.Type type = parameter.Type;
               if (type.IsAggregateType)
               {
                  stringBuilder.Append("valuetype ");
               }

               stringBuilder.Append(Driver.GetFriendlyTypeName(type, true));
            }
         }

         stringBuilder.Append(")");

         // Append the raw byte stream followed by the 
         // bytes converted to ASCII format.
         if (attributeSymbol.BlobByteSize > 0)
         {
            byte[] blob = attributeSymbol.Blob;

            stringBuilder.AppendFormat(" = ( {0})",
               Utility.FormatByteArray(blob));

            if (Utility.ContainsPrintableCharacters(blob))
            {
               stringBuilder.AppendFormat("   // {0}",
                  Utility.FormatByteArrayAsText(blob));
            }
         }

         string result = stringBuilder.ToString();

         if (attributeSymbol.OwnerSymbol.Equals(this.moduleUnit.UnitSymbol))
         {
            this.manifestBuilder.AddCustomAttribute(this.moduleUnit, result);
         }

         return result;
      }

      /// <summary>
      /// Retrieves the marshal attribute symbol associated 
      /// with the given symbol.
      /// </summary>
      private static Phx.Symbols.AttributeSymbol GetMarshalAttribute(
         Phx.Symbols.Symbol symbol)
      {
         // If the symbol has an attribute symbol list, add 
         // each custom attribute symbol in the list to our own list.
         if (symbol.AttributeSymbolList != null)
         {
            foreach (Phx.Symbols.Symbol attributeSymbol in 
               symbol.AttributeSymbolList)
            {
               if (attributeSymbol.IsAttributeSymbol &&
                   attributeSymbol.AsAttributeSymbol.IsMarshaling)
               {
                  return attributeSymbol.AsAttributeSymbol;
               }
            }
         }
         return null;
      }

      /// <summary>
      /// Creates a formatted string that represents the given
      /// marshal attribute symbol.
      /// </summary>
      private static string GetMarshalString(
         Phx.Symbols.AttributeSymbol attributeSymbol)
      {
         if (attributeSymbol == null)
         {
            return string.Empty;
         }
         
         string marshalString = string.Empty;

         if (attributeSymbol.BlobByteSize == 1)
         {
            // The first (and only) byte in the attribute symbol
            // blob describes the type. 
            marshalString = string.Format("marshal({0})",
               CorTypes.GetNativeTypeString(
                  (CorTypes.CorNativeType)attributeSymbol.Blob[0]));
         }
         else
         {
            // Handle malformed attribute blob.
            marshalString = "marshal(/* unknown */)";
         }

         return marshalString;
      }

      /// <summary>
      /// Searches for the import symbol and import module symbol
      /// associdated with the given name.
      /// </summary>
      private static bool FindImports(Phx.PEModuleUnit moduleUnit, 
         Phx.Name name,
         out Phx.Symbols.ImportModuleSymbol importModuleSymbol,
         out Phx.Symbols.ImportSymbol importSymbol)
      {
         foreach (Phx.Symbols.ImportModuleSymbol moduleSymbol
            in moduleUnit.ImportModuleSymbols)
         {
            importSymbol = moduleSymbol.FindImport(name);
            if (importSymbol != null)
            {
               importModuleSymbol = moduleSymbol;
               return true;
            }
         }

         importModuleSymbol = null;
         importSymbol = null;
         return false;
      }

      /// <summary>
      /// Converts the given ushort value to a list of P/Invoke
      /// attribute strings.
      /// </summary>
      private static string GetPInvokeAttributes(ushort attributes)
      {
         if (attributes == 0x0000)
            return "/* no map */";

         if (attributes == 0xffff)
            return "/* unknown */";

         string result = string.Empty;

         if ((attributes & 0x0001) == 0x0001)
            result = string.Concat(result, " nomangle");

         if ((attributes & 0x0002) == 0x0002)
            result = string.Concat(result, " ansi");

         if ((attributes & 0x0004) == 0x0004)
            result = string.Concat(result, " unicode");

         if ((attributes & 0x0006) == 0x0006)
            result = string.Concat(result, " autochar");
         
         if ((attributes & 0x0040) == 0x0040)
            result = string.Concat(result, " lasterr");

         if ((attributes & 0x0100) == 0x0100)
            result = string.Concat(result, " winapi");

         if ((attributes & 0x0200) == 0x0200)
            result = string.Concat(result, " cdecl");

         if ((attributes & 0x0300) == 0x0300)
            result = string.Concat(result, " stdcall");

         if ((attributes & 0x0400) == 0x0400)
            result = string.Concat(result, " thiscall");

         if ((attributes & 0x0500) == 0x0500)
            result = string.Concat(result, " fastcall");
            
         if ((attributes & 0x1000) == 0x1000)
            result = string.Concat(result, " charmaperror:on");
            
         if ((attributes & 0x2000) == 0x2000)
            result = string.Concat(result, " charmaperror:off");
            
         if ((attributes & 0x1000) == 0x0010)
            result = string.Concat(result, " bestfit:on");
            
         if ((attributes & 0x2000) == 0x0020)
            result = string.Concat(result, " bestfit:off");
            
         return result;
      }

      /// <summary>
      /// Retrieves the method flags associated with the provided
      /// function symbol as a formatted string.
      /// </summary>
      private string GetMethodFlags(
         Phx.Symbols.FunctionSymbol functionSymbol)
      {
         StringBuilder stringBuilder = new StringBuilder();

         if (functionSymbol.IsHideBySignature)
         {
            stringBuilder.Append("hidebysig ");
         }

         if (functionSymbol.IsNewSlot)
         {
            stringBuilder.Append("newslot ");
         }

         if (functionSymbol.IsCheckAccessOnOverride)
         {
            stringBuilder.Append("strict ");
         }

         if (functionSymbol.IsSpecialName)
         {
            stringBuilder.Append("specialname ");
         }

         if (functionSymbol.IsRuntimeSpecialName)
         {
            stringBuilder.Append("rtspecialname ");
         }

         if (functionSymbol.IsRequireSecurityObject)
         {
            stringBuilder.Append("reqsecobj ");
         }

         if (functionSymbol.IsAbstract)
         {
            stringBuilder.Append("abstract ");
         }

         if (functionSymbol.IsVirtual)
         {
            stringBuilder.Append("virtual ");
         }

         if (functionSymbol.IsFinal)
         {
            stringBuilder.Append("final ");
         }

         stringBuilder.Append(
            functionSymbol.FunctionType.IsInstanceMethod ? 
            "instance " : "static ");

         // Append P/Invoke attributes.
         if (functionSymbol.IsPinvokeImpl)
         {
            Phx.Symbols.ImportModuleSymbol importModuleSymbol;
            Phx.Symbols.ImportSymbol importSymbol;

            FindImports(this.moduleUnit, functionSymbol.Name,
               out importModuleSymbol, out importSymbol);
            
            ushort attributes = 0xffff;
            
            stringBuilder.AppendFormat("pinvokeimpl({0}) ", 
               Driver.GetPInvokeAttributes(attributes));
         }

         if (functionSymbol.IsUnmanagedExport)
         {
            stringBuilder.Append("unmanagedexp ");
         }

         return stringBuilder.ToString();
      }
      
      /// <summary>
      /// Determines whether the provided string is an ILAsm 
      /// reserved word.
      /// </summary>
      internal static bool IsReservedName(string name)
      {
         return Keywords.IsQuotable(name);
      }
      
      /// <summary>
      /// Determines whether the given string is in 
      /// quotation marks.
      /// </summary>
      internal static bool IsQuotedText(string text)
      {
         return text.Length > 0 && text[0] == '\'' && 
            text[text.Length-1] == '\'';
      }

      /// <summary>
      /// Strips quotation marks from the given string.
      /// </summary>
      internal static string UnquoteText(string text)
      {
         Debug.Assert(Driver.IsQuotedText(text));
         return text.Substring(1, text.Length - 2);
      }
      
      /// <summary>
      /// Formats the given string for display. 
      /// </summary>
      internal static string GetFormattedName(string name)
      {
         // Surround reserved words with single quotes.
         if (Driver.IsReservedName(name))
         {
            return string.Format("'{0}'", name);
         }
         return name; 
      }

      /// <summary>
      /// Interprets the first 4 bytes of the given byte array
      /// as a 32-bit integer.
      /// </summary>
      internal static int GetInt32Value(byte[] bytes, int index)
      {
         return bytes[index + 3] +
               ((int)bytes[index + 2] << 8) +
               ((int)bytes[index + 1] << 16) +
               ((int)bytes[index + 0] << 24);
      }

      /// <summary>
      /// Interprets the first 2 bytes of the given byte array
      /// as a 32-bit integer.
      /// </summary>
      internal static int GetInt16Value(byte[] bytes, int index)
      {
         return bytes[index + 1] + ((int)bytes[index] << 8);
      }

      /// <summary>
      /// Retrieves the array of symbols associated with the symbol table
      /// of the provided function symbol.
      /// </summary>
      internal Phx.Symbols.Symbol[] GetFunctionSymbols(
         Phx.Symbols.FunctionSymbol functionSymbol)
      {
         List<Phx.Symbols.Symbol> symbols = 
            new List<Phx.Symbols.Symbol>();

         if (functionSymbol.FunctionUnit != null &&
             functionSymbol.FunctionUnit.SymbolTable != null)
         {
            foreach (Phx.Symbols.Symbol symbol 
               in functionSymbol.FunctionUnit.SymbolTable.AllSymbols)
            {
               symbols.Add(symbol);
            }            
         }

         return symbols.ToArray();
      }

      /// <summary>
      /// Releases disassembly information associated with the provided
      /// ElementNode object.
      /// </summary>
      internal void EvictDisassembly(ElementNode elementNode)
      {
         // If the symbol associated with the element node is a 
         // function symbol, we free the associated 
         // function unit from the symbol and its parent unit.
         if (elementNode.Symbol != null &&
             elementNode.Symbol.IsFunctionSymbol)
         {
            Phx.Symbols.FunctionSymbol functionSymbol = 
               elementNode.Symbol.AsFunctionSymbol;
            this.ReleaseFunctionUnit(functionSymbol);            
         }
         
         // Release the handle to the element node's Disassembly 
         // property.
         elementNode.Disassembly = null;
      }

      /// <summary>
      /// Releases the function unit associated with the provided
      /// function symbol from memory.
      /// </summary>
      private void ReleaseFunctionUnit(
         Phx.Symbols.FunctionSymbol functionSymbol)
      {
         if (functionSymbol == null)
         {
            return;
         }

         Phx.FunctionUnit functionUnit = functionSymbol.FunctionUnit;
         if (functionUnit != null)
         {
            functionUnit.ReleaseLifetime();
            if (functionUnit.ParentUnit != null)
            {
               functionUnit.ParentUnit.RemoveChildUnit(functionUnit);
            }
         }
         functionSymbol.FunctionUnit = null;
      }
   }
}
