using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

namespace PEExplorer
{  
   class PermissionProperty
   {  
      private string name;
      private string type;
      private string value;

      /// <summary>
      /// Retrieves the name associated with the permission 
      /// property.
      /// </summary>
      internal string Name
      {
         get { return this.name; }
      }

      /// <summary>
      /// Retrieves the type associated with the permission 
      /// property.
      /// </summary>
      internal string Type
      {
         get { return this.type; }
      }

      /// <summary>
      /// Retrieves the value associated with the permission 
      /// property.
      /// </summary>
      internal string Value
      {
         get { return this.value; }
      }

      /// <summary>
      /// Constructs a new PermissionProperty object from the provided
      /// byte array.
      /// </summary>
      internal static PermissionProperty FromBlob(byte[] blob, ref int blobIndex)
      {
         return new PermissionProperty(blob, ref blobIndex);
      }

      /// <summary>
      /// Constructs a new PermissionProperty object from the provided
      /// byte array.
      /// </summary>
      private PermissionProperty(byte[] blob, ref int blobIndex)
      {
         // The first byte of the blob is SERIALIZATION_TYPE_PROPERTY, 0x54.
         
         Debug.Assert(blob[blobIndex] == 0x54);
         blobIndex++;

         // Parse element type.
         
         CorTypes.CorElementType elementType =
            (CorTypes.CorElementType)Utility.UncompressUInt32(
               blob, ref blobIndex);

         this.type = CorTypes.GetPrimitiveTypeString(elementType);

         // Parse property name length and property name.

         int propertyNameLength = 
            (int)Utility.UncompressUInt32(blob, ref blobIndex);
         this.name = 
            Utility.FormatByteArrayAsText(blob, blobIndex, propertyNameLength);
         blobIndex += propertyNameLength;
         
         // Parse encoded value.

         this.value = CorTypes.GetTypeValueString(
            elementType, blob, ref blobIndex);
      }
   }

   class Permission
   {
      // The class name of the permission.
      private string className;
      // The permission properties.
      private PermissionProperty[] permissionProperties;

      /// <summary>
      /// Retrieves the class name assocaited with the permission.
      /// </summary>
      internal string ClassName
      {
         get { return this.className; }
      }

      /// <summary>
      /// Retrieves the PermissionProperty objects that are
      /// associated with the permission.
      /// </summary>
      internal PermissionProperty[] PermissionProperties
      {
         get { return this.permissionProperties; }
      }

      /// <summary>
      /// Constructs a new Permission object from the provided
      /// byte array.
      /// </summary>
      internal static Permission FromBlob(byte[] blob, ref int blobIndex)
      {
         return new Permission(blob, ref blobIndex);
      }

      /// <summary>
      /// Constructs a new Permission object from the provided
      /// byte array.
      /// </summary>
      private Permission(byte[] blob, ref int blobIndex)
      {
         // Parse class name length and class name.

         // Read length of name.
         int classNameLength = 
            (int)Utility.UncompressUInt32(blob, ref blobIndex);
            
         // Read name as a text string.
         string propertyName = 
            Utility.FormatByteArrayAsText(blob, blobIndex, classNameLength);
         
         // Split text string into its components parts.
         string[] tokens = propertyName.Split(new char[] { ',' });
         
         // The second token is the assembly reference; the first token
         // is the fully-qualified class name.
         this.className = string.Format("[{0}]{1}", 
            tokens[1].Trim(), tokens[0].Trim());
            
         // Advance pointer into byte array.
         blobIndex += classNameLength;

         // Read the length of the initialization blob. This value is unused.
         uint initializationBlobLength = 
            Utility.UncompressUInt32(blob, ref blobIndex);
            
         // Read the number of properties in the byte stream.
         uint propertyCount = Utility.UncompressUInt32(blob, ref blobIndex);
         // Read in each property.
         this.permissionProperties = new PermissionProperty[propertyCount];
         for (uint i = 0; i < propertyCount; i++)
         {
            this.permissionProperties[i] = 
               PermissionProperty.FromBlob(blob, ref blobIndex);
         }
      }
   }

   /// <summary>
   /// Describes a single permission set.
   /// </summary>
   class PermissionSet
   {
      // The permissions in the set.
      private Permission[] permissions;

      /// <summary>
      /// Retrieves the Permission objects that make up the permission set.
      /// </summary>
      internal Permission[] Permissions
      {
         get { return this.permissions; }
      }

      /// <summary>
      /// Constructs a new PermissionSet object from the provided
      /// byte array.
      /// </summary>
      internal static PermissionSet FromBlob(byte[] blob)
      {
         return new PermissionSet(blob);
      }

      /// <summary>
      /// Constructs a new PermissionSet object from the provided
      /// byte array.
      /// </summary>
      private PermissionSet(byte[] blob)
      {
         // The blob must contain at least the dot ('.') character
         // and the permission count.
         Debug.Assert(blob.Length >= 2);

         // The blob must begin with the dot ('.') character.
         Debug.Assert(blob[0] == 0x2e);

         // Get permission count.

         int index = 1;
         int permissionCount = (int)Utility.UncompressUInt32(blob, ref index);

         // Read each Permission object from the byte array.
         this.permissions = new Permission[permissionCount];
         for (int i = 0; i < permissionCount; i++)
         {
            permissions[i] = Permission.FromBlob(blob, ref index);
         }
      }      
   }
}
