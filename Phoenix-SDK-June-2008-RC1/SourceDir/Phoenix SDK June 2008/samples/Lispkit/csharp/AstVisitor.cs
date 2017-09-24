namespace Lispkit.Ast
{
   /// <summary>
   /// Interface for visiting nodes in an AST.
   /// Rather than defining action in the AST itself, we use the
   /// visitor pattern so we can define AST actions externally.
   /// </summary>
   interface IAstVisitor
   {
       void Visit(Node node);
       void Visit(UnaryNode node);
       void Visit(BinaryNode node);
       void Visit(TernaryNode node);

       void Visit(UnaryExpNode node);
       void Visit(BinaryExpNode node);

       void Visit(AritExpNode node);
       void Visit(AtomExpNode node);
       void Visit(AtomNode node);
       void Visit(CallExpNode node);
       void Visit(CarExpNode node);
       void Visit(CdrExpNode node);
       void Visit(ConsExpNode node);
       void Visit(ConsPairListNode node);
       void Visit(ConsPairNode node);
       void Visit(ConsPairRecListNode node);
       void Visit(ConsPairRecNode node);
       void Visit(EqExpNode node);
       void Visit(ExpListNode node);
       void Visit(ExpNode node);
       void Visit(FunctionNode node);
       void Visit(IfExpNode node);
       void Visit(LambdaExpNode node);
       void Visit(LeqExpNode node);
       void Visit(LetExpNode node);
       void Visit(LetrecExpNode node);
       void Visit(NumericNode node);
       void Visit(ProgramNode node);
       void Visit(QuoteExpNode node);
       void Visit(SExpressionListNode node);
       void Visit(SExpressionNode node);
       void Visit(SymbolicListNode node);
       void Visit(SymbolicSequenceNode node);
       void Visit(SymbolicNode node);
   };

} // namespace Ast 