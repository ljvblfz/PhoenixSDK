using System;
using System.Collections.Generic;
using LispKit.Ast;

namespace LispKit.Msil
{
   // TODO: Just roll into Evaluator???

   class TypeEvaluator : Evaluator
   {
      private ITypeProvider typeProvider;
      private Dictionary<Sexp.SExp, Phx.Types.Type> typeMap;

      public TypeEvaluator(ITypeProvider typeProvider)
      {
         this.typeProvider = typeProvider;
         this.typeMap = new Dictionary<LispKit.Sexp.SExp, Phx.Types.Type>();
      }

      public Dictionary<Sexp.SExp, Phx.Types.Type> TypeMap
      {
         get { return this.typeMap; }
      }

      #region IAstVisitor Members

      public override void Visit(CallExpNode node)
      {
         base.Visit(node);
                  
         Sexp.SExp car = this.sexp.Car;
         Sexp.SExp cdr = this.sexp.Cdr;
      }

      public override void Visit(FunctionNode node)
      {
         base.Visit(node);
      }

      public override void Visit(NumericNode node)
      {
         base.Visit(node);
         //this.sexp = new Sexp.IntAtom(node.Line, node.Column, node.IntValue);
      }
      
      public override void Visit(SymbolicNode node)
      {
         base.Visit(node);
         this.typeMap[this.sexp] = this.typeProvider.SymAtomClassType;
      }

      #endregion      
   }
}
