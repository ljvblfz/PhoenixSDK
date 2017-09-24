using System;
using System.Collections.Generic;
using System.Text;
using LispKit.Secd;
using System.Diagnostics;

namespace LispKit.Sexp
{
   class SExpException : Exception
   {
      public SExpException()
         : this("The method or operation is undefined.")
      {
      }

      public SExpException(string message)
         : base(message)
      {
      }
   }

   class StopException : Exception
   {
      public StopException()       
      {
      }
   }

   abstract class SExp
   {
      private int line; 
      private int column;
      
      private uint debugCounter;
      private static uint nextDebugCounter = 0;
      
      protected SExp(int line, int column)
      {
         this.line = line;
         this.column = column;

         this.debugCounter = SExp.nextDebugCounter;
         SExp.nextDebugCounter++;
      }

      public virtual void SetCar(SExp car)
      {
         throw new Exception("The method or operation is not implemented.");
      }

      public virtual void SetCdr(SExp cdr)
      {
         throw new Exception("The method or operation is not implemented.");
      }

      public abstract string Format();
      
      public int Line
      {
         get { return this.line; }
      }

      public int Column
      {
         get { return this.column; }
      }
      
      public abstract SExp Car
      {
         get;
      }

      public abstract SExp Cdr
      {
         get;
      }

      public virtual int IntValue
      {
         get { throw new SExpException(); }
      }

      public virtual string Symbol
      {
         get { throw new SExpException(); }
      }

      // Composite selectors (188)


      public SExp Cadr
      {
         get { return Cdr.Car; } 
      }

      public SExp Cddr
      {
         get { return Cdr.Cdr; }
      }

      public SExp Cdddr
      {
         get { return Cdr.Cdr.Cdr; }
      }

      public SExp Cdar
      {
         get { return Car.Cdr; }
      }

      public SExp Caar
      {
         get { return Car.Car; }
      }

      public SExp Caddr
      {
         get { return Cdr.Cdr.Car; }
      }

      public SExp Cadddr
      {
         get { return Cdr.Cdr.Cdr.Car; }
      }

      /*public static SExp Flatten(SExp x)
      {
         if (x is Atom)
         {
            return new Cons(x, SymAtom.NIL);
         }
         return Append(Flatten(x.Car), Flatten(x.Cdr));
      }

      public static SExp Append(SExp x, SExp y)
      {
         if (x == SymAtom.NIL)
         {
            return y;
         }
         return new Cons(x.Car, Append(x.Cdr, y));
      }*/      
   }

   class Cons : SExp
   {
      private SExp car;
      private SExp cdr;
      private Instructions instruction;

      public override SExp Car
      {
         get { return this.car; }
      }

      public override SExp Cdr
      {
         get { return this.cdr; }
      }

      public Instructions Instruction
      {
         get { return this.instruction; }
         set { this.instruction = value; }
      }

      public Cons(int line, int column, SExp car, SExp cdr)
         : base(line, column)         
      {
         this.instruction = Instructions.None;
         this.car = car;
         this.cdr = cdr;
      }

      public Cons(Instructions instruction, SExp cdr)
         : base(0, 0)
      {
         this.instruction = instruction;
         this.car = SymAtom.FromInstruction(instruction);
         this.cdr = cdr;
      }

      public override void SetCar(SExp car)
      {
         this.car = car;
      }

      public override void SetCdr(SExp cdr)
      {
         this.cdr = cdr;
      }

      public override bool Equals(object obj)
      {
         if (obj is Cons)
         {
            Cons other = obj as Cons;
            return other.Car.Equals(this.Car) &&
                   other.Cdr.Equals(this.Cdr);
         }
         return base.Equals(obj);
      }

      public override int GetHashCode()
      {
         return Car.GetHashCode() + Cdr.GetHashCode();
      }

      public override string Format()
      {
         string s = ToString();

         string t = string.Empty;

         int removeRParen = -1;
         int nestCount = 0;
         for (int i = 0; i < s.Length - 1; i++)
         {
            if (s[i] == '(')
            {
               ++nestCount;
            }
            else if (s[i] == ')')
            {
               --nestCount;
            }

            if (s[i] == '.')
            {
               if (s[i + 1] == '(')
               {
                  t += ' ';
                  i++;
                  removeRParen = nestCount - 1;
               }
               else if (s.IndexOf("NIL", i) == i + 1)
               {
                  i += 4;
               }
               else
               {
                  t += '.';
               }
            }
            else if (s[i] == ')' && removeRParen == nestCount)
            {
               //t += ' ';
               removeRParen = -1;
            }
            else
            {
               t += s[i];
            }
         }
         t += s[s.Length - 1];

         return t;
      }

      public override string ToString()
      {         
         bool hasInstruction = this.instruction != Instructions.None;
         bool hasCar = true;// this.car != SymAtom.NIL;
         bool hasCdr = true;//this.cdr != SymAtom.NIL;
         bool carIsInt = this.car is IntAtom;
         bool cdrIsInt = this.cdr is IntAtom;

         string a;
         string b;
         if (hasInstruction)
         {
            a = ((int)instruction).ToString();
            //a = instruction.ToString();
            b = this.cdr.Format();
         }
         else if (hasCdr)
         {
            a = this.car.Format();
            b = this.cdr.Format();
         }
         else
         {
            a = this.car.Format();
            b = string.Empty;
         }

         //Debug.Assert(!a.Contains("2 NIL 2 NIL"));
         //Debug.Assert(!b.Contains("2 NIL 2 NIL"));

         string s;
         if (a.Length > 0 && b.Length > 0)
            s = string.Format("({0}.{1})", a, b);
         else if (a.Length > 0)
            s = a;
         else
            s = b;

         return s;

      }      
   }

   abstract class Atom : SExp
   {
      protected Atom(int line, int column)
         : base(line, column)
      {
      }
         
      /// <summary>
      /// TODO
      /// </summary>
      public override SExp Car
      {
         get { throw new SExpException("car of an atom is undefined."); }
      }

      /// <summary>
      /// TODO
      /// </summary>
      public override SExp Cdr
      {
         get { throw new SExpException("cdr of an atom is undefined."); }
      }
   }

   class SymAtom : Atom
   {
      public static readonly SymAtom NIL = new SymAtom("NIL");
      public static readonly SymAtom QUOTE = new SymAtom("QUOTE");
      public static readonly SymAtom ADD = new SymAtom("ADD");
      public static readonly SymAtom SUB = new SymAtom("SUB");
      public static readonly SymAtom MUL = new SymAtom("MUL");
      public static readonly SymAtom DIV = new SymAtom("DIV");
      public static readonly SymAtom REM = new SymAtom("REM");
      public static readonly SymAtom EQ = new SymAtom("EQ");
      public static readonly SymAtom LEQ = new SymAtom("LEQ");
      public static readonly SymAtom CAR = new SymAtom("CAR");
      public static readonly SymAtom CDR = new SymAtom("CDR");
      public static readonly SymAtom CONS = new SymAtom("CONS");
      public static readonly SymAtom ATOM = new SymAtom("ATOM");
      public static readonly SymAtom IF = new SymAtom("IF");
      public static readonly SymAtom LAMBDA = new SymAtom("LAMBDA");
      public static readonly SymAtom LET = new SymAtom("LET");
      public static readonly SymAtom LETREC = new SymAtom("LETREC");
      public static readonly SymAtom LD = new SymAtom("LD");
      public static readonly SymAtom LDC = new SymAtom("LDC");
      public static readonly SymAtom LDF = new SymAtom("LDF");
      public static readonly SymAtom AP = new SymAtom("AP");
      public static readonly SymAtom RTN = new SymAtom("RTN");
      public static readonly SymAtom DUM = new SymAtom("DUM");
      public static readonly SymAtom RAP = new SymAtom("RAP");
      public static readonly SymAtom SEL = new SymAtom("SEL");
      public static readonly SymAtom JOIN = new SymAtom("JOIN");
      public static readonly SymAtom STOP = new SymAtom("STOP");
      
      private string symbol;

      public override string Symbol
      {
         get { return this.symbol; }
      }

      protected SymAtom(string symbol)
         : this(0, 0, symbol)
      {        
      }

      public SymAtom(int line, int column, string symbol)
         : base(line, column)
      {
         this.symbol = symbol;
      }

      internal static SymAtom FromInstruction(Instructions instruction)
      {
         switch (instruction)
         {
            case Instructions.LD:
               return LD;
            case Instructions.LDC:
               return LDC;
            case Instructions.LDF:
               return LDF;
            case Instructions.AP:
               return AP;
            case Instructions.RTN:
               return RTN;
            case Instructions.DUM:
               return DUM;
            case Instructions.RAP:
               return RAP;
            case Instructions.SEL:
               return SEL;
            case Instructions.JOIN:
               return JOIN;
            case Instructions.CAR:
               return CAR;
            case Instructions.CDR:
               return CDR;
            case Instructions.ATOM:
               return ATOM;
            case Instructions.CONS:
               return CONS;
            case Instructions.EQ:
               return EQ;
            case Instructions.ADD:
               return ADD;
            case Instructions.SUB:
               return SUB;
            case Instructions.MUL:
               return MUL;
            case Instructions.DIV:
               return DIV;
            case Instructions.REM:
               return REM;
            case Instructions.LEQ:
               return LEQ;
            case Instructions.STOP:
               return STOP;
            default:
               throw new Exception(""); // TODO:
         }
      }

      internal Instructions ToInstruction()
      {
         if (this == LD)
            return Instructions.LD;
         if (this == LDC)
            return Instructions.LDC;
         if (this == LDF)
            return Instructions.LDF;
         if (this == AP)
            return Instructions.AP;
         if (this == RTN)
            return Instructions.RTN;
         if (this == DUM)
            return Instructions.DUM;
         if (this == RAP)
            return Instructions.RAP;
         if (this == SEL)
            return Instructions.SEL;
         if (this == JOIN)
            return Instructions.JOIN;
         if (this == CAR)
            return Instructions.CAR;
         if (this == CDR)
            return Instructions.CDR;
         if (this == ATOM)
            return Instructions.ATOM;
         if (this == CONS)
            return Instructions.CONS;
         if (this == EQ)
            return Instructions.EQ;
         if (this == ADD)
            return Instructions.ADD;
         if (this == SUB)
            return Instructions.SUB;
         if (this == MUL)
            return Instructions.MUL;
         if (this == DIV)
            return Instructions.DIV;
         if (this == REM)
            return Instructions.REM;
         if (this == LEQ)
            return Instructions.LEQ;
         if (this == STOP)
            return Instructions.STOP;

         return Instructions.None;
      }

      public override string Format()
      {
         return this.ToString();
      }

      public override bool Equals(object obj)
      {
         // TEST
         //return this.ToString().Equals(obj.ToString());

         if (obj is SymAtom)
         {
            return (obj as SymAtom).Symbol.Equals(this.Symbol);
         }
         return base.Equals(obj);
      }

      public override int GetHashCode()
      {
         return this.Symbol.GetHashCode();
      }

      public override string ToString()
      {
         return Symbol.ToString();
      }
   }

   abstract class NumAtom : Atom
   {
      protected NumAtom(int line, int column)
         : base(line, column)
      {
      }         
   }

   class IntAtom : NumAtom
   {
      private int value;

      public override int IntValue
      {
         get { return this.value; }
      }

      public IntAtom(int line, int column, int value)
         : base(line, column)
      {
         this.value = value;
      }

      public override string Format()
      {
         return this.ToString();
      }

      public override bool Equals(object obj)
      {
         if (obj is IntAtom)
         {
            return (obj as IntAtom).IntValue == this.IntValue;
         }
         return base.Equals(obj);
      }

      public override int GetHashCode()
      {
         return this.IntValue.GetHashCode();
      }

      public override string ToString()
      {
         return IntValue.ToString();
      }
   }
}
