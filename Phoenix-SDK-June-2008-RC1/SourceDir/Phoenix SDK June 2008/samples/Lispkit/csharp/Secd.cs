using System;
using System.Collections.Generic;
using System.Diagnostics;
using Lispkit.Sexp;

namespace Lispkit.Secd
{
   /// <summary>
   /// Symbolic atoms for all of the instructions that are used 
   /// by the SECD interpreter.
   /// </summary>
   static class Instructions
   {
      public static readonly SymAtom LD = SymAtom.LD; // load
      public static readonly SymAtom LDC = SymAtom.LDC; // load constant
      public static readonly SymAtom LDF = SymAtom.LDF; // load function
      public static readonly SymAtom AP = SymAtom.AP; // apply function
      public static readonly SymAtom RTN = SymAtom.RTN; // return 
      public static readonly SymAtom DUM = SymAtom.DUM; // create dummy environment
      public static readonly SymAtom RAP = SymAtom.RAP; // recursive apply
      public static readonly SymAtom SEL = SymAtom.SEL; // select subcontrol
      public static readonly SymAtom JOIN = SymAtom.JOIN; // rejoin main control
      public static readonly SymAtom CAR = SymAtom.CAR; // take car of item on top of stack
      public static readonly SymAtom CDR = SymAtom.CDR; // take cdr of item on top of stack
      public static readonly SymAtom ATOM = SymAtom.ATOM; // apply atom predicate to top stack item
      public static readonly SymAtom CONS = SymAtom.CONS; // form cons of top two stack items
      public static readonly SymAtom EQ = SymAtom.EQ; // apply eq predicate of top two stack items
      public static readonly SymAtom ADD = SymAtom.ADD; // apply add operation to top two stack items
      public static readonly SymAtom SUB = SymAtom.SUB; // apply sub operation to top two stack items
      public static readonly SymAtom MUL = SymAtom.MUL; // apply mul operation to top two stack items
      public static readonly SymAtom DIV = SymAtom.DIV; // apply div operation to top two stack items
      public static readonly SymAtom REM = SymAtom.REM; // apply rem operation to top two stack items
      public static readonly SymAtom LEQ = SymAtom.LEQ; // apply leq predicate of top two stack items
      public static readonly SymAtom STOP = SymAtom.STOP; // stop
   }

   /// <summary>
   /// Exception class that indicates that program execution
   /// should be terminated.
   /// </summary>
   class StopException : Exception
   {
      // Creates a new instance of the StopException class.
      public StopException()
      {
      }
   }

   /// <summary>
   /// The root instruction class. 
   /// </summary>
   abstract class Instruction
   {
      // Executes an instruction by modifying the SECD state.
      public abstract void Execute();
   }

   /// <summary>
   /// The base instruction class. 
   /// </summary>
   abstract class BaseInstruction : Instruction
   {
      // Counts the number of times the instruction is called.
      private int callCount = 0;

      // Executes an instruction by modifying the SECD state.
      public override void Execute()
      {
         // Increment call count and print a trace message.
         this.callCount++;
         Output.TraceMessage(this.ToString() + "(" + callCount + ")");

         // Execute the actual instruction.
         this.InternalExecute();
      }

      // Executes an instruction by modifying the SECD state.
      protected abstract void InternalExecute();
   }

   /// <summary>
   /// Implements the LD instruction.
   /// </summary>
   class LdInstruction : BaseInstruction 
   {
      #region Instruction Members

      protected override void InternalExecute()
      {
         // s e (LD i.c) d     -> (x.s) e c d   where x=locate(i,e)

         SExp x = Compiler.Locate(Register.c.Cadr, Register.e);
         Register.s = new Cons(x, Register.s);
         Register.c = Register.c.Cddr;
      }

      #endregion
   }

   /// <summary>
   /// Implements the LDC instruction.
   /// </summary>
   class LdcInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // s e (LDC a.c) d     -> (a.s) e c d

         Register.s = new Cons(Register.c.Cadr, Register.s);
         Register.c = Register.c.Cddr;
      }

      #endregion 
   }

   /// <summary>
   /// Implements the LDF instruction.
   /// </summary>
   class LdfInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // s e (LDF c'.c) d     -> ((c'.e).s) e c d

         Register.s = new Cons(
            new Cons(Register.c.Cadr, Register.e), Register.s);
         Register.c = Register.c.Cddr;
      }

      #endregion 
   }

   /// <summary>
   /// Implements the AP instruction.
   /// </summary>
   class ApInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // ((c'.e') v.s) e (AP.c) d     -> NIL (v.e') c' (s e c.d)

         Register.d = new Cons(Register.s.Cddr, 
            new Cons(Register.e, 
               new Cons(Register.c.Cdr, Register.d)));
         Register.e = new Cons(Register.s.Cadr, Register.s.Cdar);
         Register.c = Register.s.Caar;
         Register.s = Register.nil;
      }

      #endregion
   }

   /// <summary>
   /// Implements the RTN instruction.
   /// </summary>
   class RtnInstruction : BaseInstruction
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // (a) e' (RTN) (s e c.d)     -> (a.s) e c d

         Register.s = new Cons(Register.s.Car, Register.d.Car);
         Register.e = Register.d.Cadr;
         Register.c = Register.d.Caddr;
         Register.d = Register.d.Cdddr;
      }

      #endregion
   }

   /// <summary>
   /// Implements the DUM instruction.
   /// </summary>
   class DumInstruction : BaseInstruction
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // s e (DUM.c) d     -> s (OMEGA.e) c d

         Register.e = new Cons(Register.pending, Register.e);
         Register.c = Register.c.Cdr;
      }

      #endregion
   }

   /// <summary>
   /// Implements the RAP instruction.
   /// </summary>
   class RapInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {        
         // ((c'.e') v.s) (OMEGA.e) (RAP.c) d     -> NIL rplaca(e',v) c' (s e c.d)

         Debug.Assert(Register.s.Cdar.Equals(Register.e));

         Register.d = new Cons(Register.s.Cddr,
            new Cons(Register.e.Cdr,
               new Cons(Register.c.Cdr, Register.d)));
         Register.e = Register.s.Cdar;
         Register.e.Car = Register.s.Cadr;
         Register.c = Register.s.Caar;
         Register.s = Register.nil;
      }

      #endregion
   }

   /// <summary>
   /// Implements the SEL instruction.
   /// </summary>
   class SelInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // (x.s) e (SEL ct cf.c) d     -> s e cx (c.d)
          
         Register.d = new Cons(Register.c.Cdddr, Register.d); 
         if (Register.s.Car.Equals(Register.t))
         {
            Register.c = Register.c.Cadr;
         }
         else
         {
            Register.c = Register.c.Caddr;            
         }
         Register.s = Register.s.Cdr;
      }

      #endregion 
   }

   /// <summary>
   /// Implements the JOIN instruction.
   /// </summary>
   class JoinInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // s e (JOIN) (c.d)     -> s e c d

         Register.c = Register.d.Car;
         Register.d = Register.d.Cdr;
      }

      #endregion 
   }

   /// <summary>
   /// Implements the CAR instruction.
   /// </summary>
   class CarInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // ((a.b).s) e (CAR.c) d     -> (a.s) e c d

         Register.s = new Cons(Register.s.Caar, Register.s.Cdr);
         Register.c = Register.c.Cdr;
      }

      #endregion 
   }

   /// <summary>
   /// Implements the CDR instruction.
   /// </summary>
   class CdrInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // ((a.b).s) e (CDR.c) d     -> (b.s) e c d

         Register.s = new Cons(Register.s.Cdar, Register.s.Cdr);
         Register.c = Register.c.Cdr;
      }

      #endregion 
   }

   /// <summary>
   /// Implements the ATOM instruction.
   /// </summary>
   class AtomInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // (a.s) e (ATOM.c) d        ->  (T.s) e c d
         // ((a.b).s) e (ATOM.c) d    ->  (F.s) e c d

         Register.s = new Cons(
            (Register.s.Car is Atom) ? Register.t : Register.f, 
            Register.s.Cdr);
         Register.c = Register.c.Cdr;
      }

      #endregion 
   }

   /// <summary>
   /// Implements the CONS instruction.
   /// </summary>
   class ConsInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // (a b.s) e (CONS.c) d     -> ((a.b).s) e c d

         Register.s = new Cons(new Cons(Register.s.Car, Register.s.Cadr),
            Register.s.Cddr);
         Register.c = Register.c.Cdr;
      }

      #endregion 
   }

   /// <summary>
   /// Implements the EQ instruction.
   /// </summary>
   class EqInstruction : BaseInstruction 
   { 
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // (a b.s) e (EQ.c) d     -> (b = a.s) e c d

         SExp b;
         SExp s = Register.s;
         if (s.Car is SymAtom && s.Cadr is SymAtom &&
          s.Car.Symbol.Equals(s.Cadr.Symbol))
         {
            b = Register.t;
         }
         else if (s.Car is IntAtom && s.Cadr is IntAtom &&
          s.Car.IntValue == s.Cadr.IntValue)
         {
            b = Register.t;
         }
         else
         {
            b = Register.f;
         }

         Register.s = new Cons(b, s.Cddr);
         Register.c = Register.c.Cdr;
      }

      #endregion
   }

   /// <summary>
   /// Implements the ADD instruction.
   /// </summary>
   class AddInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // (a b.s) e (ADD.c) d     -> (b + a.s) e c d

         Register.s = new Cons(new IntAtom(
            Register.s.Cadr.IntValue + Register.s.Car.IntValue),
            Register.s.Cddr);
         Register.c = Register.c.Cdr;
      }

      #endregion 
   }

   /// <summary>
   /// Implements the SUB instruction.
   /// </summary>
   class SubInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // (a b.s) e (SUB.c) d     -> (b - a.s) e c d

         Register.s = new Cons(new IntAtom(
            Register.s.Cadr.IntValue - Register.s.Car.IntValue),
            Register.s.Cddr);
         Register.c = Register.c.Cdr;
      }

      #endregion 
   }

   /// <summary>
   /// Implements the MUL instruction.
   /// </summary>
   class MulInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // (a b.s) e (MUL.c) d     -> (b * a.s) e c d

         Register.s = new Cons(new IntAtom(
            Register.s.Cadr.IntValue * Register.s.Car.IntValue),
            Register.s.Cddr);
         Register.c = Register.c.Cdr;
      }

      #endregion 
   }

   /// <summary>
   /// Implements the DIV instruction.
   /// </summary>
   class DivInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // (a b.s) e (DIV.c) d     -> (b / a.s) e c d

         Register.s = new Cons(new IntAtom(
            Register.s.Cadr.IntValue / Register.s.Car.IntValue),
            Register.s.Cddr);
         Register.c = Register.c.Cdr;
      }

      #endregion 
   }

   /// <summary>
   /// Implements the REM instruction.
   /// </summary>
   class RemInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // (a b.s) e (REM.c) d     -> (b % a.s) e c d

         Register.s = new Cons(new IntAtom(
            Register.s.Cadr.IntValue % Register.s.Car.IntValue),
            Register.s.Cddr);
         Register.c = Register.c.Cdr;
      }

      #endregion 
   }

   /// <summary>
   /// Implements the LEQ instruction.
   /// </summary>
   class LeqInstruction : BaseInstruction 
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         // (a b.s) e (LEQ.c) d    -> (b <= a.s) e c d

         if (Register.s.Cadr.IntValue <= Register.s.Car.IntValue)
         {
            Register.s = new Cons(Register.t, Register.s.Cddr);
         }
         else
         {
            Register.s = new Cons(Register.f, Register.s.Cddr);
         }
         Register.c = Register.c.Cdr;
      }

      #endregion
   }

   /// <summary>
   /// Implements the STOP instruction.
   /// </summary>
   class StopInstruction : BaseInstruction
   {
      #region Instruction Members

      // Executes an instruction by modifying the SECD state.
      protected override void InternalExecute()
      {
         throw new StopException();
      }
      
      #endregion
   }

   /// <summary>
   /// SECD machine implementation.
   /// </summary>
   class Compiler : Lispkit.Compiler, Lispkit.Interpreter
   {
      // Maps symbolic instruction names to Instruction objects.
      Dictionary<SExp, Instruction> instrs = 
         new Dictionary<SExp, Instruction>();

      // Tracks the execution count of each instruction.
      Dictionary<Instruction, int> coverageMap;

      public Compiler()
      {
         // Initialize the instruction table.
         this.instrs.Add(Instructions.LD, new LdInstruction());
         this.instrs.Add(Instructions.LDC, new LdcInstruction());
         this.instrs.Add(Instructions.LDF, new LdfInstruction());
         this.instrs.Add(Instructions.AP, new ApInstruction());
         this.instrs.Add(Instructions.RTN, new RtnInstruction());
         this.instrs.Add(Instructions.DUM, new DumInstruction());
         this.instrs.Add(Instructions.RAP, new RapInstruction());
         this.instrs.Add(Instructions.SEL, new SelInstruction());
         this.instrs.Add(Instructions.JOIN, new JoinInstruction());
         this.instrs.Add(Instructions.CAR, new CarInstruction());
         this.instrs.Add(Instructions.CDR, new CdrInstruction());
         this.instrs.Add(Instructions.ATOM, new AtomInstruction());
         this.instrs.Add(Instructions.CONS, new ConsInstruction());
         this.instrs.Add(Instructions.EQ, new EqInstruction());
         this.instrs.Add(Instructions.ADD, new AddInstruction());
         this.instrs.Add(Instructions.SUB, new SubInstruction());
         this.instrs.Add(Instructions.MUL, new MulInstruction());
         this.instrs.Add(Instructions.DIV, new DivInstruction());
         this.instrs.Add(Instructions.REM, new RemInstruction());
         this.instrs.Add(Instructions.LEQ, new LeqInstruction());
         this.instrs.Add(Instructions.STOP, new StopInstruction());

         // Create the coverage map, if needed.
         if (Program.ReportCoverage)
         {
            this.coverageMap = new Dictionary<Instruction, int>();
         }
      }

      #region Compiler Members

      // Compiles the given S-expression.
      public override SExp Compile(SExp e)
      {
         return Comp(e, SymAtom.NIL, new Cons(Instructions.AP, 
            new Cons(Instructions.STOP, SymAtom.NIL)));
      }

      #endregion

      #region Interpreter Members

      // Interprets the given S-expression with the given 
      // argument list.
      public SExp Execute(SExp argList, SExp code)
      {
         Output.TraceMessage("Processing instruction stream...");

         // Initialize SECD registers.
         Register.Initialize(argList, code);

         // Run the fetch-execute cycle for the program.
         // The cycle should run until it reaches a STOP instruction.
         try
         {
            for (; ; )
            {
               // Fetch the instruction that maps to the 
               // top of the code register.
               Instruction instr = instrs[Register.c.Car];

               // Instrument code coverage.
               if (this.coverageMap != null)
               {
                  if (!this.coverageMap.ContainsKey(instr))
                  {
                     this.coverageMap.Add(instr, 1);
                  }
                  else
                  {
                     this.coverageMap[instr]++;
                  }
               }

               // Execute the instruction.
               instr.Execute();
            }
         }
         // We always expect to reach the STOP instruction.
         // The result of the program is the top of the stack register.
         catch (StopException)
         {
            return Register.s.Car;
         }
         finally
         {
            Output.TraceMessage("");
            
            // Print instruction coverage if tracking information is
            // available.
            if (this.coverageMap != null)
            {
               // Print all used instructions.
               Output.TraceMessage("Used instructions:");
               foreach (KeyValuePair<SExp, Instruction> kvp in instrs)
               {
                  Instruction instr = kvp.Value;
                  if (instr != null && this.coverageMap.ContainsKey(instr))
                     Output.TraceMessage("\t" + instr.ToString());
               }

               // Print all unused instructions.
               Output.TraceMessage("Unused instructions:");
               foreach (KeyValuePair<SExp, Instruction> kvp in instrs)
               {
                  Instruction instr = kvp.Value;
                  if (instr != null && !this.coverageMap.ContainsKey(instr))
                     Output.TraceMessage("\t" + instr.ToString());
               }
            }

            Output.TraceMessage("");
         }
      }

      #endregion      

      // Calculates the SECD code of the form e*n|c
      private static SExp Comp(SExp e, SExp n, SExp c)
      {
         if (e is Atom)
         {
            return new Cons(Instructions.LD, new Cons(Location(e, n), c));
         }
         else if (e.Car == SymAtom.QUOTE)
         {
            return new Cons(Instructions.LDC, new Cons(e.Cadr, c));
         }
         else if (e.Car == SymAtom.ADD)
         {
            return Comp(e.Cadr, n, Comp(e.Caddr, n, new Cons(Instructions.ADD, c)));
         }
         else if (e.Car == SymAtom.SUB)
         {
            return Comp(e.Cadr, n, Comp(e.Caddr, n, new Cons(Instructions.SUB, c)));
         }
         else if (e.Car == SymAtom.MUL)
         {
            return Comp(e.Cadr, n, Comp(e.Caddr, n, new Cons(Instructions.MUL, c)));
         }
         else if (e.Car == SymAtom.DIV)
         {
            return Comp(e.Cadr, n, Comp(e.Caddr, n, new Cons(Instructions.DIV, c)));
         }
         else if (e.Car == SymAtom.REM)
         {
            return Comp(e.Cadr, n, Comp(e.Caddr, n, new Cons(Instructions.REM, c)));
         }
         else if (e.Car == SymAtom.EQ)
         {
            return Comp(e.Cadr, n, Comp(e.Caddr, n, new Cons(Instructions.EQ, c)));
         }
         else if (e.Car == SymAtom.LEQ)
         {
            return Comp(e.Cadr, n, Comp(e.Caddr, n, new Cons(Instructions.LEQ, c)));
         }
         else if (e.Car == SymAtom.CAR)
         {
            return Comp(e.Cadr, n, new Cons(Instructions.CAR, c));
         }
         else if (e.Car == SymAtom.CDR)
         {            
            return Comp(e.Cadr, n, new Cons(Instructions.CDR, c));
         }
         else if (e.Car == SymAtom.ATOM)
         {
            return Comp(e.Cadr, n, new Cons(Instructions.ATOM, c));
         }
         else if (e.Car == SymAtom.CONS)
         {
            return Comp(e.Caddr, n, Comp(e.Cadr, n, new Cons(Instructions.CONS, c)));
         }
         else if (e.Car == SymAtom.IF)
         {
            SExp thenpt = Comp(e.Caddr, n, new Cons(Instructions.JOIN, SymAtom.NIL));
            SExp elsept = Comp(e.Cadddr, n, new Cons(Instructions.JOIN, SymAtom.NIL));
            return Comp(e.Cadr, n, 
               new Cons(Instructions.SEL, new Cons(thenpt, new Cons(elsept, c))));
         }
         else if (e.Car == SymAtom.LAMBDA)
         {
            SExp body = Comp(e.Caddr, new Cons(e.Cadr, n), 
               new Cons(Instructions.RTN, SymAtom.NIL));
            return new Cons(Instructions.LDF, new Cons(body, c));
         }
         else if (e.Car == SymAtom.LET)
         {
            SExp m = new Cons(Vars(e.Cddr), n);
            SExp body = Comp(e.Cadr, m, new Cons(Instructions.RTN, SymAtom.NIL));
            SExp args = Exprs(e.Cddr);
            return Complis(args, n, new Cons(Instructions.LDF, 
               new Cons(body, new Cons(Instructions.AP, c))));
         }
         else if (e.Car == SymAtom.LETREC)
         {
            SExp m = new Cons(Vars(e.Cddr), n);
            SExp body = Comp(e.Cadr, m, new Cons(Instructions.RTN, SymAtom.NIL));
            SExp args = Exprs(e.Cddr);
            return new Cons(Instructions.DUM, Complis(args, m,
               new Cons(Instructions.LDF, 
                  new Cons(body, new Cons(Instructions.RAP, c)))));
         }
         else
         {
            return Complis(e.Cdr, n, Comp(e.Car, n,
               new Cons(Instructions.AP, c)));
         }
      }

      // Extracts the list of variables (x1 ... xn) from the given list 
      // of variables and definitions ((x1.e1) ... (xn.en)).
      private static SExp Vars(SExp d)
      {
         if (d.Equals(SymAtom.NIL))
         {
            return SymAtom.NIL;
         }
         else
         {
            return new Cons(d.Caar, Vars(d.Cdr));
         }
      }

      // Extracts the list of definitions (e1 ... en) from the given list 
      // of variables and definitions ((x1.e1) ... (xn.en)).
      private static SExp Exprs(SExp d)
      {
         if (d.Equals(SymAtom.NIL))
         {
            return SymAtom.NIL;
         }
         else
         {
            return new Cons(d.Cdar, Exprs(d.Cdr));
         }
      }

      // Compiles a list of expressions in the form
      // (LDC NIL) | ek*n | (CONS) | ... | e1*n | (CONS) | c
      private static SExp Complis(SExp e, SExp n, SExp c)
      {
         if (e.Equals(SymAtom.NIL))
         {
            return new Cons(Instructions.LDC, new Cons(SymAtom.NIL, c));
         }
         else
         {            
            return Complis(e.Cdr, n, 
               Comp(e.Car, n, new Cons(Instructions.CONS, c)));            
         }
      }
            
      // Determines the location of the index pair i=(b.n) 
      // in the environment e.
      internal static SExp Locate(SExp i, SExp e)
      {
         SExp b = i.Car;
         SExp n = i.Cdr;
         SExp t = Index(b.IntValue, e);
         return Index(n.IntValue, t);
      }

      // Retrieves the S-expression at the provided 
      // 0-based index.
      private static SExp Index(int n, SExp s)
      {
         if (n == 0)
         {
            return s.Car;
         }
         else
         {
            return Index(n - 1, s.Cdr);
         }
      }
	}

   /// <summary>
   /// Holds the S-expressions that act as the registers for the 
   /// SECD machine.
   /// </summary>
   static class Register
   {
      public static SExp s;            // S
      public static SExp e;            // E
      public static SExp c;            // C
      public static SExp d;            // D

      public static SExp t;            // true
      public static SExp f;            // false
      
      public static SExp nil;          // nil

      public static SExp pending;      // omega

      // Initializes the SECD registers with the given argument list
      // and code expressions.
      public static void Initialize(SExp argList, SExp code)
      {            
         // The nil register is a circular list of dummy characters.
         Register.nil = new Cons(SymAtom.NIL, SymAtom.NIL);
         Register.nil.Car = Register.nil;
         Register.nil.Cdr = Register.nil;
         Register.nil.Car = new Cons(new SymAtom("N"), Register.nil);
         Register.nil.Car.Cdr = new Cons(new SymAtom("I"), Register.nil);
         Register.nil.Cdar.Cdr = new Cons(new SymAtom("L"), Register.nil);

         Register.s = new Cons(argList, SymAtom.NIL);
         Register.e = Register.nil;
         Register.c = code;
         Register.d = Register.nil;

         Register.t = SymAtom.TRUE;
         Register.f = SymAtom.FALSE;
         
         Register.pending = new Cons(new SymAtom("OMEGA"), SymAtom.NIL);
      }
   }
}