using System;
using System.Collections.Generic;
using System.Text;

namespace PEExplorer
{
   // These map to the images that are associated with the main 
   // form's tree view icons. 
   // TODO: Update this enumeration if you change the image list.
   enum ElementType
   {
      Blank = 0,
      Field,
      StaticField,
      Interface,
      GenericInterface,
      Class,
      GenericClass,
      ValueType,
      GenericValueType,
      Enum,
      GenericEnum,
      InstanceMethod,
      GenericInstanceMethod,
      NativeMethod,
      StaticMethod,
      GenericStaticMethod,
      Property,
      Info,
      Attribute,
      Override,
      Manifest,
      Event,
      Namespace,
      Assembly,
      Local,
      Global,
      Bad,      
   }

   /// <summary>
   /// Describes a single Msil element.
   /// </summary>
   internal class ElementNode
   {
      // The symbol that is associated with the element.
      private Phx.Symbols.Symbol symbol;

      // The name of the element.
      private string name;
      
      // The full signature of the element.
      private string[] signature;
      
      // The child nodes of the element.
      private List<ElementNode> children;
      
      // The disassembly associated with the element.
      private Disassembly disassembly;
      
      // The type of the element.
      private ElementType elementType;

      /// <summary>
      /// Constructs a new ElementNode object.
      /// </summary>
      internal ElementNode(Phx.Symbols.Symbol symbol, string name, 
         string signature, ElementType elementType)
      {
         this.symbol = symbol;
         this.name = name;
         
         this.signature = signature.Split(
            new string[] { Environment.NewLine }, 
            StringSplitOptions.RemoveEmptyEntries);

         this.elementType = elementType;
         this.disassembly = null;
         this.children = new List<ElementNode>();         
      }

      /// <summary>
      /// Gets or sets the disassembly associated with the element.
      /// </summary>
      internal Disassembly Disassembly
      {
         get { return this.disassembly; }
         set { this.disassembly = value; }
      }

      /// <summary>
      /// Gets the symbol associated with the element.
      /// </summary>
      internal Phx.Symbols.Symbol Symbol
      {
         get { return this.symbol; }
      }

      /// <summary>
      /// Gets the name associated with the element.
      /// </summary>
      internal string Name
      {
         get { return Driver.GetFormattedName(this.name); }
      }

      /// <summary>
      /// Gets the signature associated with the element.
      /// </summary>
      internal string[] Signature
      {
         get { return this.signature; }
      }

      /// <summary>
      /// Gets the list of child elements associated with the element.
      /// </summary>
      internal List<ElementNode> Children
      {
         get { return this.children; }
      }

      /// <summary>
      /// Gets the type of the element.
      /// </summary>
      internal ElementType ElementType
      {
         get { return this.elementType; }
      }
   }
}