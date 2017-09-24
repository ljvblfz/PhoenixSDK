using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

namespace PEExplorer
{
   static class CorTypes
   {
      /// <summary>
      /// Element type for Cor signature.
      /// Taken from corhdr.h.
      /// </summary>
      public enum CorElementType 
      {
          ELEMENT_TYPE_END             = 0x0,
          ELEMENT_TYPE_VOID            = 0x1,
          ELEMENT_TYPE_BOOLEAN         = 0x2,
          ELEMENT_TYPE_CHAR            = 0x3,
          ELEMENT_TYPE_I1              = 0x4,
          ELEMENT_TYPE_U1              = 0x5,
          ELEMENT_TYPE_I2              = 0x6,
          ELEMENT_TYPE_U2              = 0x7,
          ELEMENT_TYPE_I4              = 0x8,
          ELEMENT_TYPE_U4              = 0x9,
          ELEMENT_TYPE_I8              = 0xa,
          ELEMENT_TYPE_U8              = 0xb,
          ELEMENT_TYPE_R4              = 0xc,
          ELEMENT_TYPE_R8              = 0xd,
          ELEMENT_TYPE_STRING          = 0xe,
              
          ELEMENT_TYPE_PTR             = 0xf,
          ELEMENT_TYPE_BYREF           = 0x10,
              
          ELEMENT_TYPE_VALUETYPE       = 0x11,
          ELEMENT_TYPE_CLASS           = 0x12,
          ELEMENT_TYPE_VAR             = 0x13,
          ELEMENT_TYPE_ARRAY           = 0x14,
          ELEMENT_TYPE_GENERICINST     = 0x15,
          ELEMENT_TYPE_TYPEDBYREF      = 0x16,

          ELEMENT_TYPE_I               = 0x18,
          ELEMENT_TYPE_U               = 0x19,
          ELEMENT_TYPE_FNPTR           = 0x1B,
          ELEMENT_TYPE_OBJECT          = 0x1C,
          ELEMENT_TYPE_SZARRAY         = 0x1D,
          ELEMENT_TYPE_MVAR            = 0x1e,

          ELEMENT_TYPE_CMOD_REQD       = 0x1F,
          ELEMENT_TYPE_CMOD_OPT        = 0x20,

          ELEMENT_TYPE_INTERNAL        = 0x21,
          ELEMENT_TYPE_MAX             = 0x22,

          ELEMENT_TYPE_MODIFIER        = 0x40,
          ELEMENT_TYPE_SENTINEL        = 0x01 | ELEMENT_TYPE_MODIFIER,
          ELEMENT_TYPE_PINNED          = 0x05 | ELEMENT_TYPE_MODIFIER,
          ELEMENT_TYPE_R4_HFA          = 0x06 | ELEMENT_TYPE_MODIFIER,
          ELEMENT_TYPE_R8_HFA          = 0x07 | ELEMENT_TYPE_MODIFIER
      }

      /// <summary>
      /// Native type for N-Direct.
      /// Taken from corhdr.h.
      /// </summary>
      public enum CorNativeType
      {
         NATIVE_TYPE_END               = 0x0,
         NATIVE_TYPE_VOID              = 0x1,
         NATIVE_TYPE_BOOLEAN           = 0x2,
         NATIVE_TYPE_I1                = 0x3,
         NATIVE_TYPE_U1                = 0x4,
         NATIVE_TYPE_I2                = 0x5,
         NATIVE_TYPE_U2                = 0x6,
         NATIVE_TYPE_I4                = 0x7,
         NATIVE_TYPE_U4                = 0x8,
         NATIVE_TYPE_I8                = 0x9,
         NATIVE_TYPE_U8                = 0xa,
         NATIVE_TYPE_R4                = 0xb,
         NATIVE_TYPE_R8                = 0xc,
         NATIVE_TYPE_SYSCHAR           = 0xd,
         NATIVE_TYPE_VARIANT           = 0xe,
         NATIVE_TYPE_CURRENCY          = 0xf,
         NATIVE_TYPE_PTR               = 0x10,

         NATIVE_TYPE_DECIMAL           = 0x11,
         NATIVE_TYPE_DATE              = 0x12,
         NATIVE_TYPE_BSTR              = 0x13,
         NATIVE_TYPE_LPSTR             = 0x14,
         NATIVE_TYPE_LPWSTR            = 0x15,
         NATIVE_TYPE_LPTSTR            = 0x16,
         NATIVE_TYPE_FIXEDSYSSTRING    = 0x17,
         NATIVE_TYPE_OBJECTREF         = 0x18,
         NATIVE_TYPE_IUNKNOWN          = 0x19,
         NATIVE_TYPE_IDISPATCH         = 0x1a,
         NATIVE_TYPE_STRUCT            = 0x1b,
         NATIVE_TYPE_INTF              = 0x1c,
         NATIVE_TYPE_SAFEARRAY         = 0x1d,
         NATIVE_TYPE_FIXEDARRAY        = 0x1e,
         NATIVE_TYPE_INT               = 0x1f,
         NATIVE_TYPE_UINT              = 0x20,

         NATIVE_TYPE_NESTEDSTRUCT      = 0x21,
         NATIVE_TYPE_BYVALSTR          = 0x22,
         NATIVE_TYPE_ANSIBSTR          = 0x23,
         NATIVE_TYPE_TBSTR             = 0x24,
         NATIVE_TYPE_VARIANTBOOL       = 0x25,
         NATIVE_TYPE_FUNC              = 0x26,

         NATIVE_TYPE_ASANY             = 0x28,
         NATIVE_TYPE_ARRAY             = 0x2a,
         NATIVE_TYPE_LPSTRUCT          = 0x2b,
         NATIVE_TYPE_CUSTOMMARSHALER   = 0x2c,

         NATIVE_TYPE_ERROR             = 0x2d,

         NATIVE_TYPE_MAX               = 0x50
      }

      /// <summary>
      /// Converts the provided CorElementType value to a
      /// string representation.
      /// </summary>
      public static string GetPrimitiveTypeString(CorElementType elementType)
      {
         switch (elementType)
         {
            case CorElementType.ELEMENT_TYPE_VOID:
               return "void";
            case CorElementType.ELEMENT_TYPE_BOOLEAN:
               return "bool";
            case CorElementType.ELEMENT_TYPE_CHAR:
               return "char";
            case CorElementType.ELEMENT_TYPE_I1:
               return "int8";
            case CorElementType.ELEMENT_TYPE_U1:
               return "unsigned int8";
            case CorElementType.ELEMENT_TYPE_I2:
               return "int16";
            case CorElementType.ELEMENT_TYPE_U2:
               return "unsigned int16";
            case CorElementType.ELEMENT_TYPE_I4:
               return "int32";
            case CorElementType.ELEMENT_TYPE_U4:
               return "unsigned int32";
            case CorElementType.ELEMENT_TYPE_I8:
               return "int64";
            case CorElementType.ELEMENT_TYPE_U8:
               return "unsigned int64";
            case CorElementType.ELEMENT_TYPE_R4:
               return "float32";
            case CorElementType.ELEMENT_TYPE_R8:
               return "float64";
            case CorElementType.ELEMENT_TYPE_STRING:
               return "string";
            case CorElementType.ELEMENT_TYPE_TYPEDBYREF:
               return "typedref";
            case CorElementType.ELEMENT_TYPE_I:
               return "native int";
            case CorElementType.ELEMENT_TYPE_U:
               return "native unsigned int";
            default:
               Debug.Assert(false, "Unhandled");
               return elementType.ToString();
         }
      }

      /// <summary>
      /// Converts the provided CorNativeType value to a
      /// string representation.
      /// </summary>
      public static string GetNativeTypeString(CorNativeType nativeType)
      {
         switch (nativeType)
         {
            case CorNativeType.NATIVE_TYPE_VOID:
               return "void";
            case CorNativeType.NATIVE_TYPE_BOOLEAN:
               return "bool";
            case CorNativeType.NATIVE_TYPE_I1:
               return "int8";
            case CorNativeType.NATIVE_TYPE_U1:
               return "unsigned int8";
            case CorNativeType.NATIVE_TYPE_I2:
               return "int16";
            case CorNativeType.NATIVE_TYPE_U2:
               return "unsigned int16";
            case CorNativeType.NATIVE_TYPE_I4:
               return "int32";
            case CorNativeType.NATIVE_TYPE_U4:
               return "unsigned int32";
            case CorNativeType.NATIVE_TYPE_I8:
               return "int64";
            case CorNativeType.NATIVE_TYPE_U8:
               return "unsigned int64";
            case CorNativeType.NATIVE_TYPE_R4:
               return "float32";
            case CorNativeType.NATIVE_TYPE_R8:
               return "float64";
            case CorNativeType.NATIVE_TYPE_SYSCHAR:
               return "syschar";
            case CorNativeType.NATIVE_TYPE_VARIANT:
               return "variant";
            case CorNativeType.NATIVE_TYPE_CURRENCY:
               return "currency";
            case CorNativeType.NATIVE_TYPE_PTR:
               return "*";
            case CorNativeType.NATIVE_TYPE_DECIMAL:
               return "decimal";
            case CorNativeType.NATIVE_TYPE_DATE:
               return "date";
            case CorNativeType.NATIVE_TYPE_BSTR:
               return "bstr";
            case CorNativeType.NATIVE_TYPE_LPSTR:
               return "lpstr";
            case CorNativeType.NATIVE_TYPE_LPWSTR:
               return "lpwstr";
            case CorNativeType.NATIVE_TYPE_LPTSTR:
               return "lptstr";
            case CorNativeType.NATIVE_TYPE_FIXEDSYSSTRING:
               return "fixed sysstring [{0}]";
            case CorNativeType.NATIVE_TYPE_OBJECTREF:
               return "objectref";
            case CorNativeType.NATIVE_TYPE_IUNKNOWN:
               return "iunknown";
            case CorNativeType.NATIVE_TYPE_IDISPATCH:
               return "idispatch";
            case CorNativeType.NATIVE_TYPE_STRUCT:
               return "struct";
            case CorNativeType.NATIVE_TYPE_INTF:
               return "interface";
            case CorNativeType.NATIVE_TYPE_SAFEARRAY:
               return "safearray {0}";
            case CorNativeType.NATIVE_TYPE_FIXEDARRAY:
               return "fixed array [{0}]";
            case CorNativeType.NATIVE_TYPE_INT:
               return "int";
            case CorNativeType.NATIVE_TYPE_UINT:
               return "unsigned int";
            case CorNativeType.NATIVE_TYPE_NESTEDSTRUCT:
               return "nested struct";
            case CorNativeType.NATIVE_TYPE_BYVALSTR: 
               return "byvalstr";
            case CorNativeType.NATIVE_TYPE_ANSIBSTR:
               return "ansi bstr";
            case CorNativeType.NATIVE_TYPE_TBSTR:
               return "tbstr";
            case CorNativeType.NATIVE_TYPE_VARIANTBOOL:
               return "variant bool";
            case CorNativeType.NATIVE_TYPE_FUNC:
               return "method";
            case CorNativeType.NATIVE_TYPE_ASANY:
               return "as any";
            case CorNativeType.NATIVE_TYPE_ARRAY:
               return "{0} [{1}]";
            case CorNativeType.NATIVE_TYPE_LPSTRUCT:
               return "lpstruct";
            case CorNativeType.NATIVE_TYPE_CUSTOMMARSHALER:
               return "custom ({0}, {1})";
            case CorNativeType.NATIVE_TYPE_ERROR:
               return "error";
            default:
               Debug.Assert(false, "Unhandled");
               return nativeType.ToString();
         }
      }

      /// <summary>
      /// Converts the provided CorElementType value to a
      /// string representation.
      /// </summary>
      public static string GetTypeValueString(CorElementType elementType, 
         byte[] encodedValue, ref int encodedValueStart)
      {
         string value = string.Empty;

         switch (elementType)
         {
            case CorElementType.ELEMENT_TYPE_VOID:
               value = "void";
               break;
            case CorElementType.ELEMENT_TYPE_BOOLEAN:
               Debug.Assert(encodedValue[encodedValueStart] == 0 ||
                            encodedValue[encodedValueStart] == 1);
               value = encodedValue[encodedValueStart] == 0 ? "false" : "true";
               encodedValueStart++;
               break;
            case CorElementType.ELEMENT_TYPE_CHAR:
               value = ((char)Driver.GetInt16Value(
                  encodedValue, encodedValueStart)).ToString();
               encodedValueStart += 2;
               break;
            case CorElementType.ELEMENT_TYPE_I1:
               value = ((sbyte)encodedValue[encodedValueStart]).ToString();
               encodedValueStart++;
               break;
            case CorElementType.ELEMENT_TYPE_U1:
               value = ((byte)encodedValue[encodedValueStart]).ToString();
               encodedValueStart++;
               break;
            case CorElementType.ELEMENT_TYPE_I2:
               value = ((short)Driver.GetInt16Value(
                  encodedValue, encodedValueStart)).ToString();
               encodedValueStart += 2;
               break;
            case CorElementType.ELEMENT_TYPE_U2:
               value = ((ushort)Driver.GetInt16Value(
                  encodedValue, encodedValueStart)).ToString();
               encodedValueStart += 2;
               break;
            case CorElementType.ELEMENT_TYPE_I4:
               value = ((int)Driver.GetInt32Value(
                  encodedValue, encodedValueStart)).ToString();
               encodedValueStart += 4;
               break;
            case CorElementType.ELEMENT_TYPE_U4:
               value = ((uint)Driver.GetInt32Value(
                  encodedValue, encodedValueStart)).ToString();
               encodedValueStart += 4;
               break;
            case CorElementType.ELEMENT_TYPE_STRING:
               int count = encodedValue.Length - encodedValueStart;
               value = string.Format("\"{0}\"",
                  Utility.FormatByteArrayAsText(
                     encodedValue, encodedValueStart, count));
               encodedValueStart += count;
               break;
            default:
               Debug.Assert(false, "Unhandled");
               value = elementType.ToString();
               break;
         }

         return value;
      }
   }
}
