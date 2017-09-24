using System;
using System.IO;

namespace Lispkit.Sexp
{
   /// <summary>
   /// Utility class that translates a text string into an S-expression.
   /// </summary>
   class SExpReader
   {
      // The type of a string token.
      private enum Type
      {
         AlphaNumeric,
         Numeric,
         Delimiter,
         EndFile
      }
      
      // The current token in the stream and its type.
      private string token;
      private Type type;

      // The input stream.
      private string input;
      // StringReader for the input stream.
      private StringReader reader;
            
      // Creates a new instance of the SExpReader class.
      public SExpReader(string input)
      {
         this.input = input.Trim();
      }

      // Translates the input string into an S-expression.
      public SExp Process()
      {         
         this.reader = new StringReader(this.input);

         this.Scan();
         return this.GetExp();
      }
      
      // Scans the next token in the stream.
      private void Scan()
      {
         this.GetToken();
         if (this.type == Type.EndFile)
         {
            token = ")";
         }
      }
      
      // Converts the current token into an S-expression.
      private SExp GetExp()
      {
         SExp e;
         
         if (this.token == "(")
         {
            this.Scan();
            e = this.GetExpList();
         }
         else if (this.type == Type.Numeric)
         {
            e = new IntAtom(int.Parse(this.token));
         }
         else
         {
            e = new SymAtom(token);
         }
         
         this.Scan();
         
         return e;
      }

      // Creates an S-expression list from the token stream.
      private SExp GetExpList()
      {
         SExp car = GetExp();
         SExp cdr;
         
         if (this.token == ".")
         {
            this.Scan(); 
            cdr = this.GetExp();
         }
         else if (this.token == ")")
         {
            cdr = SymAtom.NIL;
         }
         else
         {
            cdr = this.GetExpList();
         }
         
         return new Cons(car, cdr);
      }

      private bool eof = false;
      private char ch = ' ';
      
      // Retrieves the next token in the stream.
      private void GetToken()
      {
         this.token = string.Empty;
       
         while (!this.eof && (ch != ')' && ch != '(' 
            && char.IsWhiteSpace(this.ch)))
         {
            this.GetChar();
         }  
         
         if (this.eof)
         {
            this.type = Type.EndFile;
         }
         else if (char.IsDigit(this.ch) || this.ch == '-')
         {
            this.type = Type.Numeric;
            
            this.token += this.ch;
            this.GetChar();
            while (!this.eof && char.IsDigit(this.ch))
            {
               token += this.ch;
               this.GetChar();
            }
         }
         else if (char.IsLetter(this.ch))
         {
            this.type = Type.AlphaNumeric;

            this.token += this.ch;
            this.GetChar();
            while (!this.eof && char.IsLetterOrDigit(this.ch))
            {
               token += this.ch;
               this.GetChar();
            }
         }
         else
         {
            this.type = Type.Delimiter;
            this.token += ch;
            this.GetChar();
         }
      }

      // Retrieves the next character in the stream.
      private void GetChar()
      {
         int i = (char)this.reader.Read();
         if (i == 0xffff)
         {
            this.eof = true;         
         }
         else
         {
            this.ch = Convert.ToChar(i);
         }         
      }
   }
}
