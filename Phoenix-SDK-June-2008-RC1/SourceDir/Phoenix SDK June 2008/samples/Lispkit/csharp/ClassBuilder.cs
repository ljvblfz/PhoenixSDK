using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Lispkit.CodeGen
{
   /// <summary>
   /// Describes the symbolic name and the type of a method argument.
   /// </summary>
   struct Argument
   {
      public string Name;
      public Phx.Types.Type Type;

      public Argument(string name, Phx.Types.Type type)
      {
         this.Name = name;
         this.Type = type;
      }
   }

   /// <summary>
   /// Base class for building class information.
   /// You can extend this class to hold elements such as fields and events.
   /// </summary>
   abstract class ClassBuilder
   {
      // The module unit that is associated with the class builder.
      protected Phx.ModuleUnit moduleUnit;

      // The class type.
      protected Phx.Types.AggregateType classType;

      // The pointer-to-class type.
      protected Phx.Types.PointerType classPointerType;

      // The type symbol of the class.
      protected Phx.Symbols.TypeSymbol classTypeSymbol;

      // Mapping of method names to lists of function symbols.
      // The list of function symbols allows us to map method overloads
      // to a single dictionary entry.
      protected Dictionary<string, 
         List<Phx.Symbols.FunctionSymbol>> methods;
      
      // Retrieves the type of the class.
      public Phx.Types.AggregateType ClassType
      {
         get { return this.classType; }
      }

      // Retrieves the pointer-to-class type.
      public Phx.Types.PointerType ClassPointerType
      {
         get { return this.classPointerType; }
      }

      // Retrieves the type symbol of the class.
      public Phx.Symbols.TypeSymbol ClassTypeSymbol
      {
         get { return this.classTypeSymbol; }
      }

      // Constructs a new ClassBuilder instance.
      protected ClassBuilder(Phx.ModuleUnit moduleUnit, string className,
         Phx.Types.AggregateType baseType, Phx.Symbols.Access access, 
         bool isDefinition, bool isExternal)
      {
         this.moduleUnit = moduleUnit;
         this.methods = 
            new Dictionary<string, List<Phx.Symbols.FunctionSymbol>>();

         this.BuildClassType(className, baseType, access, 
            isDefinition, isExternal);
         Debug.Assert(this.classType != null);
         Debug.Assert(this.classPointerType != null);
         Debug.Assert(this.classTypeSymbol != null);
      }

      // Builds the type, pointer-to-class, and type symbols for the class.
      // Derived classes must implement this method for a specific
      // environment.
      protected abstract void BuildClassType(string className,
         Phx.Types.AggregateType baseType, Phx.Symbols.Access access,
         bool isDefinition, bool isExternal);

      // Adds the provided function symbol to the method table.
      public void AddMethod(Phx.Symbols.FunctionSymbol functionSymbol)
      {
         this.InternalAddMethod(functionSymbol.NameString, functionSymbol);
      }

      // Creates a function symbol from the provided parameters and adds 
      // it to the method table.
      // Derived classes must implement this method for a specific
      // environment.
      public abstract Phx.Symbols.FunctionSymbol AddMethod(
         Phx.Symbols.Access access, Phx.Symbols.Visibility visibility,
         bool isInstanceMethod, bool isVirtual, Phx.Types.Type returnType, 
         string methodName, Argument[] arguments);

      // Retrieves the function symbol with the given name, or null if the
      // method does not exist.
      // If there is more than one function symbol associated with the given
      // name, then this method returns the first one in the list.
      public Phx.Symbols.FunctionSymbol GetMethod(string methodName)
      {
         List<Phx.Symbols.FunctionSymbol> methodList;
         if (this.methods.TryGetValue(methodName, out methodList))
         {
            return methodList.Count > 0 ? methodList[0] : null;
         }
         return null;     
      }

      // Adds the provided function symbol to the method table.
      protected void InternalAddMethod(string methodName, 
         Phx.Symbols.FunctionSymbol functionSymbol)
      {
         if (!this.methods.ContainsKey(methodName))
         {
            this.methods.Add(methodName, 
               new List<Phx.Symbols.FunctionSymbol>());
         }
         this.methods[methodName].Add(functionSymbol);
      }     
   }
}

namespace Lispkit.CodeGen.Msil
{
   /// <summary>
   /// Concrete implementation of the ClassBuilder class.
   /// This class builds class information for the MSIL runtime.
   /// </summary>
   class MsilClassBuilder : ClassBuilder
   {
      // Constructs a new MsilClassBuilder instance.
      public MsilClassBuilder(Phx.ModuleUnit moduleUnit, string className,
         Phx.Types.AggregateType baseType, Phx.Symbols.Access access, 
         bool isDefinition, bool isExternal)
         : base(moduleUnit, className, baseType, access, 
            isDefinition, isExternal)
      {
      }

      // Builds the type, pointer-to-class, and type symbols for the class.
      protected override void BuildClassType(string className,
         Phx.Types.AggregateType baseType, Phx.Symbols.Access access, 
         bool isDefinition, bool isExternal)
      {
         Phx.Name classTypeName = 
            Phx.Name.New(this.moduleUnit.Lifetime, className);

         // Create a new MSIL type symbol for the class and add it to the 
         // symbol table of the module.
         this.classTypeSymbol = Phx.Symbols.MsilTypeSymbol.New(
            this.moduleUnit.SymbolTable, classTypeName, 0);

         // Create the AggregateType object for the class.
         // In MSIL, we use the NewDynamicSize method to create a 
         // type with variable size.
         this.classType = Phx.Types.AggregateType.NewDynamicSize(
            this.moduleUnit.TypeTable, this.classTypeSymbol);

         // Set properties of the type.
         this.classType.IsPrimary = true;
         this.classType.IsSelfDescribing = true;
         this.classType.Access = access;
         this.classType.IsDefinition = isDefinition;
         this.classType.IsReference = !isDefinition;
         
         // If the class has a base type (which is true for all classes except
         // for System.Object), append a base type link to the class type.
         if (baseType != null)
         {
            this.classType.AppendBaseTypeLink(
               Phx.Types.BaseTypeLink.New(baseType, false));
         }

         // Add the class type symbol to the lexical scope of the 
         // module's assembly if it is defined within this assembly.
         if (!isExternal)
         {            
            Phx.AssemblyUnit assemblyUnit = 
               this.moduleUnit.AsPEModuleUnit.AssemblyUnit;

            Phx.Symbols.AssemblySymbol assemblySymbol = 
               assemblyUnit.AssemblySymbol;

            assemblySymbol.InsertInLexicalScope(
               this.classTypeSymbol, classTypeName);
         }

         // Create a reference to the "object pointer to class" type.
         this.classPointerType = 
            this.moduleUnit.TypeTable.GetObjectPointerType(this.classType);
      }

      // Creates a function symbol from the provided parameters and adds 
      // it to the method table.
      public override Phx.Symbols.FunctionSymbol AddMethod(
         Phx.Symbols.Access access, Phx.Symbols.Visibility visibility,
         bool isInstanceMethod, bool isVirtual, Phx.Types.Type returnType, 
         string methodName, Argument[] arguments)
      {
         Phx.Lifetime lifetime = this.classTypeSymbol.Lifetime;

         // Create the function type by using a FunctionTypeBuilder object.

         Phx.Types.FunctionTypeBuilder typeBuilder =
            Phx.Types.FunctionTypeBuilder.New(this.moduleUnit.TypeTable);

         // Managed methods use the __clrcall calling convention.
         typeBuilder.CallingConventionKind = 
            Phx.Types.CallingConventionKind.ClrCall;

         // Append return type.
         typeBuilder.AppendReturnParameter(returnType);
         
         // Append argument types.

         // Instance methods pass the 'this' pointer as the first argument.
         if (isInstanceMethod)
         {
            typeBuilder.AppendParameter(this.classPointerType,
               Phx.Types.ParameterKind.ThisPointer);
         }
         // Append user-defined argument types.
         if (arguments != null)
         {
            foreach (Argument argument in arguments)
            {
               typeBuilder.AppendParameter(argument.Type);
            }
         }         

         // Extract the function type from the type builder.
         Phx.Types.FunctionType functionType = typeBuilder.GetFunctionType();

         // Create a function symbol for the method and add it to the symbol
         // table of the module.

         Phx.Name functionName = Phx.Name.New(lifetime, methodName);
                  
         Phx.Symbols.FunctionSymbol functionSymbol = 
            Phx.Symbols.FunctionSymbol.New(this.moduleUnit.SymbolTable,
            0, functionName, functionType, visibility);
         
         // Set virtual and access properties.
         functionSymbol.IsVirtual = isVirtual;
         functionSymbol.Access = access;

         // Set the Static modifier if the method is non-instance.
         if (!isInstanceMethod)
         {
            functionSymbol.MethodSpecifier = 
               Phx.Symbols.MethodSpecifier.Static;
         }

         // Append argument symbolic names.
         functionSymbol.CreateParameterSymbols();

         // Instance methods pass the 'this' pointer as the first argument.
         // By using the name '$this', Lispkit programs can use identifiers
         // that are named 'this'.
         if (isInstanceMethod)
         {
            functionSymbol.ThisPointerParameterSymbol.Name =
               Phx.Name.New(lifetime, "$this");
         }
         // Append user-defined argument names.
         if (arguments != null)
         {
            for (int argumentIndex = 0; argumentIndex < arguments.Length;
               ++argumentIndex)
            {
               functionSymbol.UserDefinedParameterSymbols[argumentIndex].Name =
                  Phx.Name.New(lifetime, arguments[argumentIndex].Name);
            }
         }

         // Add the function symbol to the class type.
         this.classType.AppendMethodSymbol(functionSymbol);

         // Add the function symbol to the name table of the parent
         // class symbol.
         this.classTypeSymbol.InsertInLexicalScope(
            functionSymbol, functionName);

         // Add to method mapping.
         this.InternalAddMethod(methodName, functionSymbol);

         return functionSymbol;
      }    
   }
}
