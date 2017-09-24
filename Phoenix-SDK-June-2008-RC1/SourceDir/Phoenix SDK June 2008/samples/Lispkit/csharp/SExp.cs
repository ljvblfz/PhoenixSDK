using System;
using System.Diagnostics;

// Note that this source file is shared between the compiler and 
// compiled Lispkit programs. The LISPKIT_COMPILER compilation symbol
// is defined by the compiler but not by individual programs. 
// We use the LISPKIT_COMPILER symbol to add data members, properties,
// and code that are used by the compiler, but are not needed by
// actual Lispkit programs.

namespace Lispkit.Sexp
{
   /// <summary>
   /// Represents an error that occurs when attempting to access
   /// undefined S-expression elements.
   /// </summary>
   class SExpException : Exception
   {
      // Creates a new instance of the SExpException class.
      public SExpException()
         : this("The method or operation is undefined.")
      {
      }

      // Creates a new instance of the SExpException class
      // with the given message string.
      public SExpException(string message)
         : base(message)
      {
      }
   }
   
   /// <summary>
   /// The base S-expression class.
   /// </summary>
   abstract class SExp
   {
      // For the compiler, we introduce a numeric identifier for
      // each S-expression object. This identifier is used to associate
      // each object with a location (source line and column numbers).
#if LISPKIT_COMPILER
      public int ID;
      private static int NextID = 0;
#endif
     
      // Creates a new instance of the SExp class.
      protected SExp()
      {
#if LISPKIT_COMPILER
         // Assign next available ID.
         this.ID = SExp.NextID++;
#endif
      }
            
      // Sets or retrieves the head of an S-expression list.      
      public virtual SExp Car
      {
#if LISPKIT_COMPILER
         set { throw new SExpException("car is undefined."); }
#endif
         get { throw new SExpException("car is undefined."); }
      }

      // Sets or retrieves the rest of an S-expression list.
      public virtual SExp Cdr
      {
#if LISPKIT_COMPILER
         set { throw new SExpException("cdr is undefined."); }
#endif
         get { throw new SExpException("cdr is undefined."); }
      }
            
      // Retrieves the integer value that is associated with 
      // the S-expression.      
      public virtual int IntValue
      {
         get { throw new SExpException("int value is undefined."); }
      }
      
      // Retrieves the symbolic value that is associated with 
      // the S-expression.      
      public virtual string Symbol
      {
         get { throw new SExpException("symbol is undefined."); }
      }

      // Retrieves the function pointer (0 arguments) that is associated with 
      // the S-expression.      
      public virtual FuncDelegate0 FuncPtr0
      {
         get { throw new SExpException("funcptr(0) is undefined."); }
      }
            
      // Retrieves the function pointer (1 argument) that is associated with 
      // the S-expression.      
      public virtual FuncDelegate1 FuncPtr1
      {
         get { throw new SExpException("funcptr(1) is undefined."); }
      }
      
      // Retrieves the function pointer (2 arguments) that is associated with 
      // the S-expression.      
      public virtual FuncDelegate2 FuncPtr2
      {
         get { throw new SExpException("funcptr(2) is undefined."); }
      }

      // Retrieves the function pointer (3 arguments) that is associated with 
      // the S-expression.      
      public virtual FuncDelegate3 FuncPtr3
      {
         get { throw new SExpException("funcptr(3) is undefined."); }
      }
      
      // Retrieves the function pointer (4 arguments) that is associated with 
      // the S-expression.      
      public virtual FuncDelegate4 FuncPtr4
      {
         get { throw new SExpException("funcptr(4) is undefined."); }
      }

      //
      // Composite selectors
      //
            
      // Composite selector for car(cdr(s)).      
      public SExp Cadr
      {
         get { return Cdr.Car; } 
      }
      
      // Composite selector for cdr(cdr(s)).      
      public SExp Cddr
      {
         get { return Cdr.Cdr; }
      }
      
      // Composite selector for cdr(cdr(cdr(s))).      
      public SExp Cdddr
      {
         get { return Cdr.Cdr.Cdr; }
      }
      
      // Composite selector for cdr(car(s)).      
      public SExp Cdar
      {
         get { return Car.Cdr; }
      }
      
      // Composite selector for car(car(s)).      
      public SExp Caar
      {
         get { return Car.Car; }
      }
      
      // Composite selector for car(cdr(cdr(s))).      
      public SExp Caddr
      {
         get { return Cdr.Cdr.Car; }
      }
            
      // Composite selector for car(cdr(cdr(cdr(s)))).      
      public SExp Cadddr
      {
         get { return Cdr.Cdr.Cdr.Car; }
      }      
      
      // Determines whether the S-expression is an atom.      
      public SExp IsAtom()
      {
         if (this is Atom)
         {
            return SymAtom.TRUE;
         }
         return SymAtom.FALSE;
      }
      
      // Determines whether the S-expression represents
      // the TRUE atom.      
      public int IsTrue()
      {
         return this.Equals(SymAtom.TRUE) ? 1 : 0;
      }
      
      // Determines whether the S-expression is logically 
      // less than or equal to the provided S-expression.      
      public SExp Leq(SExp sexp)
      {
         if (this is IntAtom && sexp is IntAtom)
         {
            if (this.IntValue <= sexp.IntValue)
            {
               return SymAtom.TRUE;
            }
            else
            {
               return SymAtom.FALSE;
            }
         }
         return SymAtom.FALSE;
      }
      
      // Determines whether the S-expression is logically 
      // equal to the provided S-expression.      
      public SExp Eq(SExp sexp)
      {
         if (this.Equals(sexp))
         {
            return SymAtom.TRUE;
         }
         return SymAtom.FALSE;         
      }
   }

   /// <summary>
   /// Represents a CONS expression.
   /// </summary>
   class Cons : SExp
   {
      // The head of the list.
      private SExp car;
      // The rest of the list.
      private SExp cdr;
      
      // Sets or retrieves the head of an S-expression list.      
      public override SExp Car
      {
#if LISPKIT_COMPILER
         [DebuggerNonUserCode]
         set { this.car = value; }
#endif
         [DebuggerNonUserCode]
         get { return this.car; }
      }
            
      /// Sets or retrieves the rest of an S-expression list.      
      public override SExp Cdr
      {
#if LISPKIT_COMPILER
         [DebuggerNonUserCode]
         set { this.cdr = value; }
#endif
         [DebuggerNonUserCode]
         get { return this.cdr; }
      }
            
      // Creates a new instance of the Cons class.
      public Cons(SExp car, SExp cdr)        
      {
         this.car = car;
         this.cdr = cdr;
      }

      // Determines whether two Cons instances are equal.
      public override bool Equals(object obj)
      {
         if (this == obj)
            return true;
         
         if (obj is Cons)
         {
            Cons other = obj as Cons;
            return other.Car.Equals(this.Car) &&
                   other.Cdr.Equals(this.Cdr);
         }
         return base.Equals(obj);
      }

      // Serves as a hash function for the Cons type.
      public override int GetHashCode()
      {
         return Car.GetHashCode() ^ Cdr.GetHashCode();
      }

      // Determines whether this instance is the head of an 
      // infinite (circular) list.
      private bool IsInfinite()
      {
         SExp p = this;
         while (p is Cons)
         {            
            SExp q = p;
            p = p.Cdr;
            if (p == q)
            {
               return true;
            }
         }         
         return false;
      }

      // Used to print circular lists.
      private int recurseCount = 0;

      // Returns a String that represents the Cons object. 
      public override string ToString()
      {
         // If this is the head of a circular list, print out
         // only the first few elements.
         bool inRecurse = false;
         if (IsInfinite())
         {
            inRecurse = true;
            recurseCount = 1;
         }

         string s = "(";
         SExp p = this;
         while (p is Cons)
         {
            s += p.Car.ToString() + " ";           
            p = p.Cdr;

            if (inRecurse)
            {
               recurseCount--;
               if (recurseCount == 0)
               {
                  s += "[...]";
                  p = SymAtom.NIL;
                  break;
               }
            }
         }
         s = s.Trim();

         if (!p.Equals(SymAtom.NIL))
         {            
            s += "." + p.ToString();
         }
         s += ")";

         return s;
      }
   }

   /// <summary>
   /// Base class for ATOM expressions.
   /// </summary>
   abstract class Atom : SExp
   {
   }

   /// <summary>
   /// Represents a symbolic ATOM expression.
   /// </summary>
   class SymAtom : Atom
   {
      //
      // Constant symbols that are shared between the compiler
      // and Lispkit programs.
      //

      public static readonly SymAtom NIL = new SymAtom("NIL");
      public static readonly SymAtom TRUE = new SymAtom("T");
      public static readonly SymAtom FALSE = new SymAtom("F");
      public static readonly SymAtom ERROR = new SymAtom("ERROR");
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

      // Symbolic data for the atom.
      private string symbol;

      // Retrieves the symbolic value that is associated with 
      // the S-expression.      
      public override string Symbol
      {
         get { return this.symbol; }
      }

      // Creates a new instance of the SymAtom class.
      public SymAtom(string symbol)
      {
         this.symbol = symbol;
      }

      // Class constructor method.
      static SymAtom()
      {
         // When invoked from the compiler, add each static field
         // to the Location object.
#if LISPKIT_COMPILER
         Location.Add(NIL.ID, 0, 0);
         Location.Add(TRUE.ID, 0, 0);
         Location.Add(FALSE.ID, 0, 0);
         Location.Add(ERROR.ID, 0, 0);
         Location.Add(QUOTE.ID, 0, 0);
         Location.Add(ADD.ID, 0, 0);
         Location.Add(SUB.ID, 0, 0);
         Location.Add(MUL.ID, 0, 0);
         Location.Add(DIV.ID, 0, 0);
         Location.Add(REM.ID, 0, 0);
         Location.Add(EQ.ID, 0, 0);
         Location.Add(LEQ.ID, 0, 0);
         Location.Add(CAR.ID, 0, 0);
         Location.Add(CDR.ID, 0, 0);
         Location.Add(CONS.ID, 0, 0);
         Location.Add(ATOM.ID, 0, 0);
         Location.Add(IF.ID, 0, 0);
         Location.Add(LAMBDA.ID, 0, 0);
         Location.Add(LET.ID, 0, 0);
         Location.Add(LETREC.ID, 0, 0);
         Location.Add(LD.ID, 0, 0);
         Location.Add(LDC.ID, 0, 0);
         Location.Add(LDF.ID, 0, 0);
         Location.Add(AP.ID, 0, 0);
         Location.Add(RTN.ID, 0, 0);
         Location.Add(DUM.ID, 0, 0);
         Location.Add(RAP.ID, 0, 0);
         Location.Add(SEL.ID, 0, 0);
         Location.Add(JOIN.ID, 0, 0);
         Location.Add(STOP.ID, 0, 0);
#endif
      }

      // Determines whether two SymAtom instances are equal.
      public override bool Equals(object obj)
      {
         if (obj is SymAtom)
         {
            return (obj as SymAtom).Symbol.Equals(this.Symbol);
         }
         return false;
      }

      // Serves as a hash function for the SymAtom type.
      public override int GetHashCode()
      {
         return this.Symbol.GetHashCode();
      }

      // Returns a String that represents the SymAtom object. 
      public override string ToString()
      {
         return Symbol.ToString();
      }
   }

   /// <summary>
   /// Base class for numeric atom expressions.
   /// </summary>
   abstract class NumAtom : Atom
   {
   }

   /// <summary>
   /// Represents a numeric (integer) ATOM expression.
   /// </summary>
   class IntAtom : NumAtom
   {
      // Numeric data for the atom.
      private int value;
            
      // Retrieves the integer value that is associated with 
      // the S-expression.
      public override int IntValue
      {  
         get { return this.value; }
      }

      // Creates a new instance of the IntAtom class.
      public IntAtom(int value)
      {
         this.value = value;
      }

      // Determines whether two IntAtom instances are equal.
      public override bool Equals(object obj)
      {
         if (obj is IntAtom)
         {
            return (obj as IntAtom).IntValue == this.IntValue;
         }
         return false;
      }

      // Serves as a hash function for the IntAtom type.
      public override int GetHashCode()
      {
         return this.IntValue.GetHashCode();
      }

      // Returns a String that represents the IntAtom object. 
      public override string ToString()
      {
         return IntValue.ToString();
      }
   }

   //
   // Delegate declarations for higher-order atom expressions.
   //

   delegate SExp FuncDelegate0();
   delegate SExp FuncDelegate1(SExp arg0);
   delegate SExp FuncDelegate2(SExp arg0, SExp arg1);
   delegate SExp FuncDelegate3(SExp arg0, SExp arg1, SExp arg2);
   delegate SExp FuncDelegate4(SExp arg0, SExp arg1, SExp arg2, SExp arg3);

   /// <summary>
   /// Represents an atom expression that contains a pointer to a method
   /// that takes 0 arguments.
   /// </summary>
   class FuncAtom0 : Atom
   {
      // The function delegate.
      private FuncDelegate0 funcDelegate;

      // Retrieves the function pointer (0 arguments) that is associated with 
      // the S-expression.      
      public override FuncDelegate0 FuncPtr0
      {
         get { return this.funcDelegate; }
      }

      // Creates a new instance of the FuncAtom0 class.
      public FuncAtom0(FuncDelegate0 funcDelegate)
      {
         this.funcDelegate = funcDelegate;
      }

      // Returns a String that represents the FuncAtom0 object. 
      public override string ToString()
      {
         return "[ -> SExp]";
      }
   }

   /// <summary>
   /// Represents an atom expression that contains a pointer to a method
   /// that takes 1 argument.
   /// </summary>
   class FuncAtom1 : Atom
   {
      // The function delegate.
      private FuncDelegate1 funcDelegate;

      // Retrieves the function pointer (1 argument) that is associated with 
      // the S-expression.      
      public override FuncDelegate1 FuncPtr1
      {
         get { return this.funcDelegate; }         
      }

      // Creates a new instance of the FuncAtom1 class.
      public FuncAtom1(FuncDelegate1 funcDelegate)
      {         
         this.funcDelegate = funcDelegate;
      }

      // Returns a String that represents the FuncAtom1 object. 
      public override string ToString()
      {
         return "[SExp -> SExp]";
      }     
   }

   /// <summary>
   /// Represents an atom expression that contains a pointer to a method
   /// that takes 2 arguments.
   /// </summary>
   class FuncAtom2 : Atom
   {
      // The function delegate.
      private FuncDelegate2 funcDelegate;

      // Retrieves the function pointer (2 arguments) that is associated with 
      // the S-expression.      
      public override FuncDelegate2 FuncPtr2
      {
         get { return this.funcDelegate; }
      }

      // Creates a new instance of the FuncAtom2 class.
      public FuncAtom2(FuncDelegate2 funcDelegate)
      {
         this.funcDelegate = funcDelegate;
      }

      // Returns a String that represents the FuncAtom2 object. 
      public override string ToString()
      {
         return "[SExp, SExp -> SExp]";
      }
   }

   /// <summary>
   /// Represents an atom expression that contains a pointer to a method
   /// that takes 3 arguments.
   /// </summary>
   class FuncAtom3 : Atom
   {
      // The function delegate.
      private FuncDelegate3 funcDelegate;

      // Retrieves the function pointer (3 arguments) that is associated with 
      // the S-expression.      
      public override FuncDelegate3 FuncPtr3
      {
         get { return this.funcDelegate; }
      }

      // Creates a new instance of the FuncAtom3 class.
      public FuncAtom3(FuncDelegate3 funcDelegate)         
      {
         this.funcDelegate = funcDelegate;
      }

      // Returns a String that represents the FuncAtom3 object. 
      public override string ToString()
      {
         return "[SExp, SExp, SExp -> SExp]";
      }
   }

   /// <summary>
   /// Represents an atom expression that contains a pointer to a method
   /// that takes 4 arguments.
   /// </summary>
   class FuncAtom4 : Atom
   {
      // The function delegate.
      private FuncDelegate4 funcDelegate;

      // Retrieves the function pointer (4 arguments) that is associated with 
      // the S-expression.      
      public override FuncDelegate4 FuncPtr4
      {
         get { return this.funcDelegate; }
      }

      // Creates a new instance of the FuncAtom4 class.
      public FuncAtom4(FuncDelegate4 funcDelegate)
      {
         this.funcDelegate = funcDelegate;
      }

      // Returns a String that represents the FuncAtom4 object. 
      public override string ToString()
      {
         return "[SExp, SExp, SExp, SExp -> SExp]";
      }
   }   
}
