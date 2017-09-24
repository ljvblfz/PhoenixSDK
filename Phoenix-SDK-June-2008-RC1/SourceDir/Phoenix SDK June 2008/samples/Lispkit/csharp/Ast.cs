using System.Diagnostics;

namespace Lispkit.Ast
{
   /// <summary>
   /// Base class for any node in the AST.
   /// </summary>
   abstract class Node 
   {
      private int line;
      private int column;

      protected Node(int line, int column)
      {
         this.line = line;
         this.column = column;
      }
          
      // Retrieves the source code line number that corresponds to 
      // this instance.
      internal int Line
      {
         get { return this.line; }
      }

      // Retrieves the source code column number that corresponds to 
      // this instance.
      internal int Column
      {
         get { return this.column; }
      }
             
      // Accepts the given AST visitor object.
      public abstract void Accept(IAstVisitor visitor);
   }

   /// <summary>
   /// An AST node that has one child.
   /// </summary>
   abstract class UnaryNode : Node
   {
      protected Node first;

      protected UnaryNode(int line, int column, Node first)
         : base(line, column)
      {
         this.first = first;
      }

      // A unary node has one child.   
      internal Node First
      {
         [DebuggerNonUserCode]
         get { return this.first; }
      }
      
      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {          
          visitor.Visit(this);
      }      
   }

   /// <summary>
   /// An AST node that has two children.
   /// </summary>
   abstract class BinaryNode : UnaryNode
   {
      protected Node second;

      protected BinaryNode(int line, int column,
         Node first, Node second)
         : base(line, column, first)
      {
         this.second = second;
      }

      // A binary node has two children.
      internal Node Second
      {
         [DebuggerNonUserCode]
         get { return this.second; }
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {
          
          visitor.Visit(this);
      }
   }

   /// <summary>
   /// An AST node that has three children.
   /// </summary>
   abstract class TernaryNode : BinaryNode
   {
      protected Node third;

      protected TernaryNode(int line, int column,
         Node first, Node second, Node third)
         : base(line, column, first, second)
      {
         this.third = third;
      }

      // A ternary node has three children.

      internal Node Third
      {
         [DebuggerNonUserCode]
         get { return this.third; }
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {   
          visitor.Visit(this);
      }      
   }

   /// <summary>
   /// A unary AST node whose child node derives from ExpNode.
   /// </summary>
   abstract class UnaryExpNode : UnaryNode
   {
      protected UnaryExpNode(int line, int column, Node first)
         : base(line, column, first)
      {
         Debug.Assert(this.first is ExpNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {          
          visitor.Visit(this);
      }
   }

   /// <summary>
   /// A binary AST node whose child nodes derive from ExpNode.
   /// </summary>
   abstract class BinaryExpNode : BinaryNode
   {
      protected BinaryExpNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
         Debug.Assert(this.first is ExpNode);
         Debug.Assert(this.second is ExpNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {          
          visitor.Visit(this);
      }
   }

   /// <summary>
   /// Arithmetic operators for use with the AritExpNode class.
   /// </summary>
   enum AritOp
   {
       Add,
       Sub,
       Mul,
       Div,
       Rem
   }

   /// <summary>
   /// AST node for arithmetic operations.
   /// </summary>
   class AritExpNode : BinaryExpNode
   {
      private AritOp op;

      internal AritExpNode(int line, int column,
         Node first, Node second, AritOp op)
         : base(line, column, first, second)
      {
         this.op = op;
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {           
         visitor.Visit(this);
      }

      // The artithmetic operator that is associated with this
      // class instance.
      internal AritOp Op
      {
         get { return this.op; }
      }       
   }

   /// <summary>
   /// AST node for the ATOM expression.
   /// </summary>
   class AtomExpNode : UnaryExpNode
   {
      internal AtomExpNode(int line, int column, Node first)
         : base(line, column, first)
      {
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for the atoms.
   /// </summary>
   class AtomNode : UnaryNode
   {
      internal AtomNode(int line, int column, Node first)
         : base(line, column, first)
      {
         Debug.Assert(this.first is SymbolicNode ||
            this.first is NumericNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for call expressions.
   /// </summary>
   class CallExpNode : BinaryNode
   {
      internal CallExpNode(int line, int column, Node first)
         : base(line, column, first, null)
      {
         Debug.Assert(this.first is FunctionNode);
      }

      internal CallExpNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
         Debug.Assert(this.first is FunctionNode);
         Debug.Assert(this.second is ExpListNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for the CAR expression.
   /// </summary>
   class CarExpNode : UnaryExpNode
   {
      internal CarExpNode(int line, int column, Node first)
         : base(line, column, first)
      {
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for the CDR expression.
   /// </summary>
   class CdrExpNode : UnaryExpNode
   {
      internal CdrExpNode(int line, int column, Node first)
         : base(line, column, first)
      {
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for the CONS expression.
   /// </summary>
   class ConsExpNode : BinaryExpNode
   {
      internal ConsExpNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for lists of CONS pairs.
   /// </summary>
   class ConsPairListNode : BinaryNode
   {
      internal ConsPairListNode(int line, int column, Node first)
         : base(line, column, first, null)
      {
         Debug.Assert(this.first is ConsPairNode);
      }

      internal ConsPairListNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
         Debug.Assert(this.first is ConsPairNode);
         Debug.Assert(this.second is ConsPairListNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for a single CONS pair.
   /// </summary>
   class ConsPairNode : BinaryNode
   {
      internal ConsPairNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
         Debug.Assert(this.first is SymbolicNode);
         Debug.Assert(this.second is ExpNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for recursive lists of CONS pairs.
   /// </summary>
   class ConsPairRecListNode : BinaryNode
   {
      internal ConsPairRecListNode(int line, int column, Node first)
         : base(line, column, first, null)
      {
         Debug.Assert(this.first is ConsPairRecNode);
      }

      internal ConsPairRecListNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
         Debug.Assert(this.first is ConsPairRecNode);
         Debug.Assert(this.second is ConsPairRecListNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for a single recursive CONS pair.
   /// </summary>
   class ConsPairRecNode : BinaryNode
   {
      internal ConsPairRecNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
         Debug.Assert(this.first is SymbolicNode);
         Debug.Assert(this.second is LambdaExpNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for the EQ expression.
   /// </summary>
   class EqExpNode : BinaryExpNode
   {
      internal EqExpNode(int line, int column, 
         Node first, Node second)
         : base(line, column, first, second)
      {
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {          
          visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for expression lists.
   /// </summary>
   class ExpListNode : BinaryNode
   {
      internal ExpListNode(int line, int column, Node first)
         : base(line, column, first, null)
      {
         Debug.Assert(this.first is ExpNode);
      }

      internal ExpListNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
         Debug.Assert(this.first is ExpNode);
         Debug.Assert(this.second is ExpListNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }    
   }

   /// <summary>
   /// AST node for a single expression.
   /// </summary>
   class ExpNode : UnaryNode
   {
      internal ExpNode(int line, int column, Node first)
         : base(line, column, first)
      {
         Debug.Assert(
            this.first is AritExpNode ||
            this.first is SymbolicNode ||
            this.first is QuoteExpNode ||
            this.first is EqExpNode ||
            this.first is LeqExpNode ||
            this.first is CarExpNode ||
            this.first is CdrExpNode ||
            this.first is AtomExpNode ||
            this.first is ConsExpNode ||            
            this.first is IfExpNode ||
            this.first is LambdaExpNode ||
            this.first is LetExpNode ||
            this.first is LetrecExpNode ||
            this.first is CallExpNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for a single functional element.
   /// </summary>
   class FunctionNode : UnaryNode
   {
      internal FunctionNode(int line, int column, Node first)
         : base(line, column, first)
      {
         Debug.Assert(this.first is SymbolicNode ||
            this.first is LambdaExpNode ||
            this.first is LetExpNode ||
            this.first is LetrecExpNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }      
   }

   /// <summary>
   /// AST node for the IF expression.
   /// </summary>
   class IfExpNode : TernaryNode
   {
      internal IfExpNode(int line, int column,
         Node first, Node second, Node third)
         : base(line, column, first, second, third)
      {
         Debug.Assert(this.first is ExpNode);
         Debug.Assert(this.second is ExpNode);
         Debug.Assert(this.third is ExpNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for the LAMBDA expression.
   /// </summary>
   class LambdaExpNode : BinaryNode
   {
      internal LambdaExpNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
         Debug.Assert(this.first is SymbolicListNode);
         Debug.Assert(this.second is ExpNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for the LEQ expression.
   /// </summary>
   class LeqExpNode : BinaryExpNode
   {
      internal LeqExpNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {          
          visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for the LET expression.
   /// </summary>
   class LetExpNode : BinaryNode
   {
      internal LetExpNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
         Debug.Assert(this.first is ExpNode);
         Debug.Assert(this.second is ConsPairListNode);
      }

      internal LetExpNode(int line, int column,
         Node first)
         : base(line, column, first, null)
      {
         Debug.Assert(this.first is ExpNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for the LETREC expression.
   /// </summary>
   class LetrecExpNode : BinaryNode
   {
      internal LetrecExpNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
         Debug.Assert(this.first is ExpNode);
         Debug.Assert(this.second is ConsPairRecListNode);
      }

      internal LetrecExpNode(int line, int column,
         Node first)
         : base(line, column, first, null)
      {
         Debug.Assert(this.first is ExpNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for a numeric element.
   /// </summary>
   class NumericNode : Node
   {
      private int intValue;

      internal NumericNode(int line, int column, int value)
         : base(line, column)
      {
         this.intValue = value;
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }

      // Retrieves the int value that is associated with
      // this class instance.
      internal int IntValue
      {
         get { return this.intValue; }
      }
   }

   /// <summary>
   /// Root AST node of any Lispkit program.
   /// </summary>
   class ProgramNode : UnaryNode
   {
      internal ProgramNode(int line, int column, Node first)
         : base(line, column, first)
      {
         Debug.Assert(this.first is LetrecExpNode ||
            this.first is LetExpNode ||
            this.first is LambdaExpNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for the QUOTE expression.
   /// </summary>
   class QuoteExpNode : UnaryNode
   {
      internal QuoteExpNode(int line, int column, Node first)
         : base(line, column, first)
      {
         Debug.Assert(this.first is SExpressionNode);
      }

      internal QuoteExpNode(int line, int column, LexParser.Tokens token)
         : base(line, column, null)
      {
         this.first = new AtomNode(line, column, new SymbolicNode(0,0, token.ToString()));
      }
      
      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {
          visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for an S-expression list.
   /// </summary>
   class SExpressionListNode : BinaryNode
   {
      internal SExpressionListNode(int line, int column, Node first)
         : base(line, column, first, null)
      {
         Debug.Assert(this.first is SExpressionNode);
      }

      internal SExpressionListNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
         Debug.Assert(this.first is SExpressionNode);
         Debug.Assert(this.second is SExpressionNode ||
            this.second is SExpressionListNode);              
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// AST node for a single S-expression.
   /// </summary>
   class SExpressionNode : UnaryNode
   {
      internal SExpressionNode(int line, int column, Node first)
         : base(line, column, first)
      {
         Debug.Assert(this.first is AtomNode ||
            this.first is SExpressionListNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }      
   }

   /// <summary>
   /// AST node for a symbolic sequence.
   /// </summary>
   class SymbolicListNode : UnaryNode
   {
      internal SymbolicListNode(int line, int column)
         : base(line, column, null)
      {
      }

      internal SymbolicListNode(int line, int column, Node first)
         : base(line, column, first)
      {
         Debug.Assert(this.first is SymbolicSequenceNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {         
         visitor.Visit(this);
      }
   }

   /// <summary>
   /// /// AST node for a symbolic element.
   /// </summary>
   class SymbolicNode : Node
   {
      private string symbol;

      internal SymbolicNode(int line, int column, string symbol)
         : base(line, column)
      {
         this.symbol = symbol;
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {          
          visitor.Visit(this);
      }

      // Retrieves the string value that is associated with 
      // this class instance.
      internal string Symbol
      {
         get { return this.symbol; }
      }
   }

   /// <summary>
   /// AST node for a sequence of symbolic elements.
   /// </summary>
   class SymbolicSequenceNode : BinaryNode
   {
      internal SymbolicSequenceNode(int line, int column, Node first)
         : base(line, column, first, null)
      {
         Debug.Assert(this.first is SymbolicNode);
      }

      internal SymbolicSequenceNode(int line, int column,
         Node first, Node second)
         : base(line, column, first, second)
      {
         Debug.Assert(this.first is SymbolicNode);
         Debug.Assert(this.second is SymbolicSequenceNode);
      }

      // Accepts the given AST visitor object.
      [System.Diagnostics.DebuggerNonUserCode]
      public override void Accept(IAstVisitor visitor)
      {  
         visitor.Visit(this);
      }
   }        
}