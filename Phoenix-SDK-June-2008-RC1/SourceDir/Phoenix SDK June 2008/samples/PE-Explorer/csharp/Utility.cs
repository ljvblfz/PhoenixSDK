using System;
using System.Collections.Generic;
using System.Text;
using System.Diagnostics;

namespace PEExplorer
{
   static class Utility
   {
      /// <summary>
      /// Reads the next short value from the provided byte pointer
      /// and advances the pointer to the next position.
      /// </summary>
      public unsafe static short ReadInt16(ref byte* pointer)
      {
         short int16 = *(short*)(pointer);
         pointer += 2;
         return int16;
      }

      /// <summary>
      /// Reads the next int value from the provided byte pointer
      /// and advances the pointer to the next position.
      /// </summary>
      public unsafe static int ReadInt32(ref byte* pointer)
      {
         int int32 = *(int*)(pointer);
         pointer += 4;
         return int32;
      }

      /// <summary>
      /// Determines whether the provided byte array contains any 
      /// printable characters.
      /// </summary>
      public static bool ContainsPrintableCharacters(byte[] byteArray)
      {
         for (int i = 0; i < byteArray.Length; i++)
         {
            byte byteValue = byteArray[i];
            if (byteValue >= 0x20 && byteValue <= 0x7E)
            {
               return true;
            }
         }
         return false;
      }

      /// <summary>
      /// Formats the given byte array to a string of 
      /// space-separated hexadecimal numbers.
      /// </summary>
      public static string FormatByteArray(byte[] byteArray)
      {
         StringBuilder stringBuilder = new StringBuilder();

         // Append each byte in hexadecimal format.
         for (int i = 0; i < byteArray.Length; i++)
         {
            stringBuilder.AppendFormat("{0} ",
               byteArray[i].ToString("X2"));
         }

         return stringBuilder.ToString();
      }

      /// <summary>
      /// Converts the given byte array to a string of 
      /// ASCII characters.
      /// </summary>
      public static string FormatByteArrayAsText(byte[] byteArray)
      {
         return FormatByteArrayAsText(byteArray, 0, byteArray.Length);
      }

      /// <summary>
      /// Converts the given byte array to a string of 
      /// ASCII characters.
      /// </summary>
      public static string FormatByteArrayAsText(
         byte[] byteArray, int byteIndex, int byteCount)
      {
         StringBuilder stringBuilder = new StringBuilder();

         // Append textual format.

         char[] charArray = Encoding.ASCII.GetChars(
            byteArray, byteIndex, byteCount);
         for (int i = 0; i < charArray.Length; i++)
         {
            // The GetChars method converts each unprintable character to 
            // the character '?'. To match Ildasm output, convert 
            // each unprintable character to the '.' character by using 
            // the original byte value.

            byte byteValue = byteArray[byteIndex + i];
            char charValue = charArray[i];
            if (byteValue < 0x20 || byteValue > 0x7E)
            {
               charValue = '.';
            }
            stringBuilder.Append(charValue);
         }

         return stringBuilder.ToString();
      }
      
      /// <summary>
      /// Uncompresses the next integer value from the provided
      /// byte array.
      /// </summary>
      public static uint UncompressUInt32(byte[] bytes, ref int index)
      {
         long value;

         // Check for 4-byte integer (0x4000 - 0x1FFFFFFF).

         if ((bytes[index] & 0xC0) == 0xC0)
         {
            Debug.Assert(index + 3 < bytes.Length);
            value = Driver.GetInt32Value(bytes, index) & ~0xC0000000;
            index += 4;
         }

         // Check for 2-byte integer (0x80 - 0x3FFF).

         else if ((bytes[index] & 0x80) == 0x80)
         {
            Debug.Assert(index + 1 < bytes.Length);
            value = Driver.GetInt16Value(bytes, index) & ~0x8000;
            index += 2;
         }

         // The integer fits within a single byte (0-0x7f)

         else
         {
            Debug.Assert(index < bytes.Length);
            value = bytes[index];
            index++;
         }

         return (uint)value;
      }

      /// <summary>
      /// Converts the given ulong to an 8 or 16-character
      /// hexadecimal string.
      /// </summary>
      public static string ToHexString(ulong number)
      {
         // Encode 16 bytes if any of the high 32 bits are set.
         // Otherwise, encode 8 bytes.
         int stringLength = 8;
         if (((number >> 32) & 0xffffffff) != 0)
            stringLength = 16;

         return number.ToString(string.Format("X{0}", stringLength));
      }
   }
}
