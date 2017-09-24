using Lispkit.Sexp;

namespace Lispkit
{
   /// <summary>
   /// Base class for code compilers.
   /// </summary>
   abstract class Compiler
   {
      // Compiles the given S-expression.
      public abstract SExp Compile(SExp e);

      // Creates an index pair of the location of the first
      // S-expression in the second.
      protected static SExp Location(SExp x, SExp n)
      {
         if (Member(x, n.Car))
         {
            return new Cons(new IntAtom(0), Position(x, n.Car));
         }
         else
         {
            SExp z = Location(x, n.Cdr);
            return new Cons(new IntAtom(z.Car.IntValue + 1), z.Cdr);
         }
      }

      // Retrieves the position of of the first
      // S-expression in the second.
      protected static SExp Position(SExp x, SExp a)
      {
         if (x.Equals(a.Car))
         {
            return new IntAtom(0);
         }
         return new IntAtom(1 + Position(x, a.Cdr).IntValue);
      }

      // Determines whether the second parameter is a member
      // of the first.
      protected static bool Member(SExp a, SExp x)
      {
         if (x.Equals(SymAtom.NIL))
         {
            return false;
         }

         if (a.Equals(x.Car))
         {
            return true;
         }

         return Member(a, x.Cdr);
      }
   }

   /// <summary>
   /// Interface for code interpreters.
   /// </summary>
   interface Interpreter
   {
      // Interprets the given S-expression with the given 
      // argument list.
      SExp Execute(SExp argList, SExp code);
   }
}
