using System;

namespace Lispkit.Ast
{
   class Evaluator : IAstVisitor
   {
      // The current S-expression in the parse tree.
      private Sexp.SExp sexp;

      // Retrieves the evaluated parse tree.
      public Sexp.SExp SExp
      {
         get { return this.sexp; }
      }
       
      #region IAstVisitor Members

      // Visits the given Node object.
      public void Visit(Node node)
      {
         throw new Exception("The method or operation is not implemented.");
      }

      // Visits the given UnaryNode object.
      public void Visit(UnaryNode node)
      {
        throw new Exception("The method or operation is not implemented.");
      }

      // Visits the given BinaryNode object.
      public void Visit(BinaryNode node)
      {
        throw new Exception("The method or operation is not implemented.");
      }

      // Visits the given TernaryNode object.
      public void Visit(TernaryNode node)
      {
        throw new Exception("The method or operation is not implemented.");
      }

      // Visits the given UnaryExpNode object.
      public void Visit(UnaryExpNode node)
      {
        throw new Exception("The method or operation is not implemented.");
      }

      // Visits the given BinaryExpNode object.
      public void Visit(BinaryExpNode node)
      {
        throw new Exception("The method or operation is not implemented.");
      }

      // Visits the given AritExpNode object.
      public void Visit(AritExpNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;

         node.Second.Accept(this);
         Sexp.SExp cdr = this.sexp;

         Sexp.SymAtom symOp;
         switch (node.Op)
         {
         case AritOp.Add:
            symOp = Sexp.SymAtom.ADD;
            break;
         case AritOp.Sub:
            symOp = Sexp.SymAtom.SUB;
            break;
         case AritOp.Mul:
            symOp = Sexp.SymAtom.MUL;
            break;
         case AritOp.Div:
            symOp = Sexp.SymAtom.DIV;
            break;
         case AritOp.Rem:
            symOp = Sexp.SymAtom.REM;
            break;
         default:
            throw new ArgumentException("The argument is invalid", "op");
         }
                 
         this.sexp = new Sexp.Cons(symOp, 
            new Sexp.Cons(car, new Sexp.Cons(cdr, Sexp.SymAtom.NIL)));

         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cdr.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cddr.ID, node.Line, node.Column);
      }

      // Visits the given AtomExpNode object.
      public void Visit(AtomExpNode node)
      {
         node.First.Accept(this);
         this.sexp = new Sexp.Cons(Sexp.SymAtom.ATOM, 
            new Sexp.Cons(this.sexp, Sexp.SymAtom.NIL));

         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cdr.ID, node.Line, node.Column);
      }

      // Visits the given AtomNode object.
      public void Visit(AtomNode node)
      {
         node.First.Accept(this);
      }

      // Visits the given CallExpNode object.
      public void Visit(CallExpNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;

         Sexp.SExp cdr;
         if (node.Second != null)
         {
            node.Second.Accept(this);
            cdr = this.sexp;
         }
         else
         {
            cdr = Sexp.SymAtom.NIL;
         }

         this.sexp = new Sexp.Cons(car, cdr);
         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
      }

      // Visits the given CarExpNode object.
      public void Visit(CarExpNode node)
      {
         node.First.Accept(this);
         Sexp.SExp cdr = this.sexp;

         this.sexp = new Sexp.Cons(Sexp.SymAtom.CAR, 
            new Sexp.Cons(cdr, Sexp.SymAtom.NIL));

         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cdr.ID, node.Line, node.Column);
      }

      // Visits the given CdrExpNode object.
      public void Visit(CdrExpNode node)
      {
         node.First.Accept(this);
         Sexp.SExp cdr = this.sexp;

         this.sexp = new Sexp.Cons(Sexp.SymAtom.CDR, 
            new Sexp.Cons(cdr, Sexp.SymAtom.NIL));

         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cdr.ID, node.Line, node.Column);
      }

      // Visits the given ConsExpNode object.
      public void Visit(ConsExpNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;

         node.Second.Accept(this);
         Sexp.SExp cdr = this.sexp;

         this.sexp = new Sexp.Cons(Sexp.SymAtom.CONS, 
            new Sexp.Cons(car, new Sexp.Cons(cdr, Sexp.SymAtom.NIL)));

         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cdr.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cddr.ID, node.Line, node.Column);
      }

      // Visits the given ConsPairListNode object.
      public void Visit(ConsPairListNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;

         if (node.Second != null)
         {
            node.Second.Accept(this);
            Sexp.SExp cdr = this.sexp;
            this.sexp = new Sexp.Cons(car, cdr);
         }
         else
         {
            this.sexp = new Sexp.Cons(car, Sexp.SymAtom.NIL);
         }

         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
      }

      // Visits the given ConsPairNode object.
      public void Visit(ConsPairNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;

         node.Second.Accept(this);
         Sexp.SExp cdr = this.sexp;

         this.sexp = new Sexp.Cons(car, cdr);

         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);         
      }

      // Visits the given ConsPairRecListNode object.
      public void Visit(ConsPairRecListNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;

         if (node.Second != null)
         {
            node.Second.Accept(this);
            Sexp.SExp cdr = this.sexp;
            this.sexp = new Sexp.Cons(car, cdr);
         }
         else
         {
            this.sexp = new Sexp.Cons(car, Sexp.SymAtom.NIL);
         }

         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
      }

      // Visits the given ConsPairRecNode object.
      public void Visit(ConsPairRecNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;

         node.Second.Accept(this);
         Sexp.SExp cdr = this.sexp;

         this.sexp = new Sexp.Cons(car, cdr);
         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
      }

      // Visits the given EqExpNode object.
      public void Visit(EqExpNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;

         node.Second.Accept(this);
         Sexp.SExp cdr = this.sexp;

         this.sexp = new Sexp.Cons(Sexp.SymAtom.EQ, 
            new Sexp.Cons(car, new Sexp.Cons(cdr, Sexp.SymAtom.NIL)));

         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cdr.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cddr.ID, node.Line, node.Column);
      }

      // Visits the given ExpListNode object.
      public void Visit(ExpListNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;
         Sexp.SExp cdr;
         
         if (node.Second != null)
         {            
            node.Second.Accept(this);
            cdr = this.sexp;
         }
         else
         {
            cdr = Sexp.SymAtom.NIL;
         }

         this.sexp = new Sexp.Cons(car, cdr);
         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
      }

      // Visits the given ExpNode object.
      public void Visit(ExpNode node)
      {         
         node.First.Accept(this);
      }

      // Visits the given FunctionNode object.
      public void Visit(FunctionNode node)
      {
         node.First.Accept(this);
      }

      // Visits the given IfExpNode object.
      public void Visit(IfExpNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;

         node.Second.Accept(this);
         Sexp.SExp thenpt = this.sexp;

         node.Third.Accept(this);
         Sexp.SExp elsept = this.sexp;

         this.sexp = new Sexp.Cons(Sexp.SymAtom.IF,
            new Sexp.Cons(car, new Sexp.Cons(thenpt,
                  new Sexp.Cons(elsept, Sexp.SymAtom.NIL))));

         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cdr.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cddr.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cdddr.ID, node.Line, node.Column);
      }

      // Visits the given LambdaExpNode object.
      public void Visit(LambdaExpNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;

         node.Second.Accept(this);
         Sexp.SExp cdr = this.sexp;

         this.sexp = new Sexp.Cons(Sexp.SymAtom.LAMBDA, 
            new Sexp.Cons(car, new Sexp.Cons(cdr, Sexp.SymAtom.NIL)));

         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cdr.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cddr.ID, node.Line, node.Column);
      }

      // Visits the given LeqExpNode object.
      public void Visit(LeqExpNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;

         node.Second.Accept(this);
         Sexp.SExp cdr = this.sexp;

         this.sexp = new Sexp.Cons(Sexp.SymAtom.LEQ,
            new Sexp.Cons(car, new Sexp.Cons(cdr, Sexp.SymAtom.NIL)));

         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cdr.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cddr.ID, node.Line, node.Column);
      }

      // Visits the given LetExpNode object.
      public void Visit(LetExpNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;
         Sexp.SExp cdr;

         if (node.Second != null)
         {
            node.Second.Accept(this);
            cdr = this.sexp;
         }
         else
         {
            cdr = Sexp.SymAtom.NIL;
         }

         this.sexp = new Sexp.Cons(Sexp.SymAtom.LET, new Sexp.Cons(car, cdr));
         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cdr.ID, node.Line, node.Column);
      }

      // Visits the given LetrecExpNode object.
      public void Visit(LetrecExpNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;
         Sexp.SExp cdr;

         if (node.Second != null)
         {
            node.Second.Accept(this);
            cdr = this.sexp;
         }
         else
         {
            cdr = Sexp.SymAtom.NIL;
         }

         this.sexp = new Sexp.Cons(Sexp.SymAtom.LETREC, new Sexp.Cons(car, cdr));
         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cdr.ID, node.Line, node.Column);
      }

      // Visits the given NumericNode object.
      public void Visit(NumericNode node)
      {
         this.sexp = new Sexp.IntAtom(node.IntValue);
         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
      }

      // Visits the given ProgramNode object.
      public void Visit(ProgramNode node)
      {
         node.First.Accept(this);

         // Trace the compiled parse tree to the output.
         string parseTree = this.sexp.ToString();
         Output.TraceMessage(string.Empty);
         Output.TraceMessage("Compiled parse tree:\r\n" + parseTree + "\r\n");
      }

      // Visits the given QuoteExpNode object.
      public void Visit(QuoteExpNode node)
      {  
         node.First.Accept(this);
         this.sexp = new Sexp.Cons(Sexp.SymAtom.QUOTE,
            new Sexp.Cons(this.sexp, Sexp.SymAtom.NIL));

         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
         Location.Add(this.sexp.Cdr.ID, node.Line, node.Column);
      }

      // Visits the given SExpressionListNode object.
      public void Visit(SExpressionListNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;
         Sexp.SExp cdr;

         if (node.Second != null)
         {
            node.Second.Accept(this);
            cdr = this.sexp;
         }
         else
         {
            cdr = Sexp.SymAtom.NIL;
         }

         this.sexp = new Sexp.Cons(car, cdr);
         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
      }

      // Visits the given SExpressionNode object.
      public void Visit(SExpressionNode node)
      {
         node.First.Accept(this);
      }

      // Visits the given SymbolicListNode object.
      public void Visit(SymbolicListNode node)
      {
         if (node.First != null)
         {
            node.First.Accept(this);
         }
         else
         {
            this.sexp = Sexp.SymAtom.NIL;
         }
      }

      // Visits the given SymbolicNode object.
      public void Visit(SymbolicNode node)
      {
         Sexp.SymAtom symAtom = GetReservedAtom(node.Symbol);
         if (symAtom != null)
         {
            this.sexp = symAtom;
         }
         else
         {
            this.sexp = new Sexp.SymAtom(node.Symbol);
            // Add line and column numbers to the Location object.
            Location.Add(this.sexp.ID, node.Line, node.Column);
         }
      }

      // Visits the given SymbolicSequenceNode object.
      public void Visit(SymbolicSequenceNode node)
      {
         node.First.Accept(this);
         Sexp.SExp car = this.sexp;
         Sexp.SExp cdr;

         if (node.Second != null)
         {
            node.Second.Accept(this);
            cdr = this.sexp;
         }
         else
         {
            cdr = Sexp.SymAtom.NIL;
         }

         this.sexp = new Sexp.Cons(car, cdr);
         // Add line and column numbers to the Location object.
         Location.Add(this.sexp.ID, node.Line, node.Column);
      }

      #endregion

      // Retrieves the shared atom that corresponds to the given
      // string.
      private Sexp.SymAtom GetReservedAtom(string s)
      {
         switch (s.ToUpper())
         {
            case "ADD":
               return Sexp.SymAtom.ADD;
            case "SUB":
               return Sexp.SymAtom.SUB;
            case "MUL":
               return Sexp.SymAtom.MUL;
            case "DIV":
               return Sexp.SymAtom.DIV;
            case "REM":
               return Sexp.SymAtom.REM;
            case "NIL":
               return Sexp.SymAtom.NIL;
            case "QUOTE":
               return Sexp.SymAtom.QUOTE;
            case "EQ":
               return Sexp.SymAtom.EQ;
            case "LEQ":
               return Sexp.SymAtom.LEQ;
            case "CAR":
               return Sexp.SymAtom.CAR;
            case "CDR":
               return Sexp.SymAtom.CDR;
            case "ATOM":
               return Sexp.SymAtom.ATOM;
            case "CONS":
               return Sexp.SymAtom.CONS;
            case "IF":
               return Sexp.SymAtom.IF;
            case "LAMBDA":
               return Sexp.SymAtom.LAMBDA;
            case "LET":
               return Sexp.SymAtom.LET;
            case "LETREC":
               return Sexp.SymAtom.LETREC;
         }
         return null;
      }
   }
}
