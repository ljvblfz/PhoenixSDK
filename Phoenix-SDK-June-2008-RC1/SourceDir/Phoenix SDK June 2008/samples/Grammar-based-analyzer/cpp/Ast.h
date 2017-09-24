#include "Scanner.h"

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;

namespace Phx
{
namespace Types
{
ref class Type;
}
}

//-----------------------------------------------------------------------------
//
// Description:
// 
//    Interface implemented by all expression nodes.
//
// Remarks:
//    IExpression is to be implemented by classes derived from Node.
//    This interface is used to provide all expressions with a data type.
//
//-----------------------------------------------------------------------------

interface class IExpression
{
   property Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }
};

//-----------------------------------------------------------------------------
// ABSTRACT Node classes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Description:
//
//     Abstract class to describe an Ast node.
//
// Remarks: 
//
//    Global code generation objects, such as the label counter, are static
//    members of this class, as are typechecking utilities invoked while
//    evaluating expression trees.
//    
//-----------------------------------------------------------------------------

ref class Node abstract
{
   //  Constructors.

public:
   Node
   (
      int sourceLineNumber
   )
      : _sourceLineNumber(sourceLineNumber)
   {
   }

private:
   static Node()
   {
      ErrorCount = 0;
      FunctionCounter = 0;
   }

   // Public methods.

public:

   // Generate Msil code for subtree rooted at receiver.

   virtual void Evaluate(TextWriter^ out) = 0;

   // Dump Ast (debugging function).

   virtual void Dump(TextWriter^ out) = 0;

   // Helper methods for implementing virtual Evaluate.

public:

   // Collect implicit variable declarations from assignments.
   // Only assignment nodes do something when this method is invoked.

   virtual void CollectLocalVariables();

protected:

   // Type checking utilities.

   static Phx::Types::Type ^
   ExpressionType
   (
      Node^   expressionNode,
      String^ expressionDescription
   )
   {
      Phx::Types::Type ^ expressionType;

      try
      {
         expressionType = safe_cast<IExpression^>(expressionNode)->Type;
      }
      catch (InvalidCastException^)
      {
         Console::Error->WriteLine("Line {0}. {1}. Expression expected.",
            expressionNode->_sourceLineNumber, expressionDescription);
         ++ErrorCount;
         return Phx::GlobalData::TypeTable->UnknownType;
      }

      return expressionType;
   }

   static bool
   TypeCheck
   (
      Node^              operand,
      Phx::Types::Type ^ expectedType,
      String^            operandDescription
   )
   {
      Phx::Types::Type ^ realType = ExpressionType(operand, operandDescription);
      if (realType == Phx::GlobalData::TypeTable->UnknownType)
      {
         return false;
      }

      if (realType != expectedType)
      {
         Console::Error->WriteLine(
            "Line {0}. {1}. Expected type '{2}'. Found '{3}'.",
            operand->_sourceLineNumber,
            operandDescription, SourceTypeString[expectedType],
            SourceTypeString[realType]);
         ++ErrorCount;
         return false;
      }

      return true;
   }

   // Label generation.

   static String^ NewLabel()
   {
      return String::Format("label{0}", _labelNumber++);
   }

   // Public static properties.

public:

   // Type-checking status.

   static property int ErrorCount;

   // Ast-internal static fields and properties.

protected:

   // FunctionUnit initialization parameter.

   static property int FunctionCounter;

   // Convert from Phx::Types::Type to respectively a high-level type name
   // and an Msil type name.

   static property Dictionary<Phx::Types::Type ^, String ^> ^ SourceTypeString
   {
      Dictionary<Phx::Types::Type ^, String ^> ^ get()
      {
      if (_sourceTypeString == nullptr)
      {
      InitializeTypeMaps();
      }
      return _sourceTypeString;
      }
   }

   static property Dictionary<Phx::Types::Type ^, String ^> ^ TargetTypeString
   {
      Dictionary<Phx::Types::Type ^, String ^> ^ get()
      {
      if (_targetTypeString == nullptr)
      {
      InitializeTypeMaps();
      }
      return _targetTypeString;
      }
   }

   // Private static functions.

private:
   static void InitializeTypeMaps();

   // Private static fields.

private:
   static int _labelNumber = 0;

   // Maps from types to string get initialized on demand rather than in
   // a static constructor to Node, which might be invoked before the Phoenix
   // type table gets initialized.

   static Dictionary<Phx::Types::Type ^, String ^> ^ _sourceTypeString =
      nullptr;
   static Dictionary<Phx::Types::Type ^, String ^> ^ _targetTypeString =
      nullptr;

   // Instance variables.

protected:
   int _sourceLineNumber;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     Abstract class to represent nodes with one or more children.
//
// Remarks: 
//
//    Abstract arity classes like UnaryNode or BinaryNode are useful for
//    delegating tasks recursively to descendants.
//    
//-----------------------------------------------------------------------------


ref class UnaryNode abstract: public Node
{
public:
   UnaryNode
   (
      int   lineNumber,
      Node^ first
   )
      : Node(lineNumber), _first(first)
   {
   }

   // A unary node has one or more children.

   Node^ First() { return _first; }

   // Unless overriden, this will simply forward the call
   // to descendant(s).

   virtual void CollectLocalVariables() override;

protected:
   Node^ _first;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     Abstract class to represent nodes with two or more children.
//    
//-----------------------------------------------------------------------------

ref class BinaryNode  abstract: public UnaryNode
{
public:
   BinaryNode
   (
      int   lineNumber,
      Node^ first,
      Node^ second
   )
      : UnaryNode(lineNumber, first), _second(second)
   {
   }

   Node^ Second() { return _second; }

   // Unless overriden, this will simply forward the call
   // to descendants.

   virtual void CollectLocalVariables() override;

protected:
   Node^ _second;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     Abstract class to represent nodes with 3 children.
//    
//-----------------------------------------------------------------------------

ref class TernaryNode  abstract: public BinaryNode
{
public:
   TernaryNode
   (
      int   lineNumber,
      Node^ first,
      Node^ second,
      Node^ third
   )
      : BinaryNode(lineNumber, first, second), _third(third)
   {
   }

   Node^ Third() { return _third; }

   // Unless overriden, this will simply forward the call
   // to descendants.

   virtual void CollectLocalVariables() override;

protected:
   Node^ _third;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    Abstract class to represent nodes with a variable number of children.
//    
//-----------------------------------------------------------------------------

ref class PolyadicNode  abstract: public Node
{
public:
   PolyadicNode
   (
      int          lineNumber,
      List<Node^>^ childList
   )
      : Node(lineNumber), _childList(childList)
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;

   List<Node^>^ ChildList() { return _childList; }

   virtual void CollectLocalVariables() override;

protected:
   List<Node^>^ _childList;
};

//-----------------------------------------------------------------------------
//                      NULLARY NODES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Description:
//
//    Return statement.
//    
//-----------------------------------------------------------------------------

ref class ReturnNode : public Node
{
public:
   ReturnNode
   (
      int lineNumber
   )
      : Node(lineNumber)
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     Integer constant.
//    
//-----------------------------------------------------------------------------

ref class NumberNode : public Node, public IExpression
{
public:
   NumberNode
   (
      int lineNumber,
      int number
   )
      : Node(lineNumber), _number(number)
   {
   }

   property Phx::Types::Type ^ Type
   {
      virtual Phx::Types::Type ^ get()
      {
      return Phx::GlobalData::TypeTable->Int32Type;
      }
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;

private:
   int _number;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    Variable occurrence inside an expression.
//
// Remarks: 
//
//    A variable occurrence as an lvalue is represented by a field in an
//    AssignmentNode.
//    
//-----------------------------------------------------------------------------

ref class VariableNode : public Node, public IExpression
{
public:
   VariableNode
   (
      int   lineNumber,
      char* variableStart,
      int   variableLength
   )
      : Node(lineNumber), _variable(gcnew String(variableStart, 0,
         variableLength))
   {
   }

   property Phx::Types::Type ^ Type
   {
      virtual Phx::Types::Type ^ get();
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;

private:
   String^ _variable;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     A boolean constant.
//    
//-----------------------------------------------------------------------------

ref class LogicalNode : public Node, public IExpression
{
public:
   LogicalNode
   (
      int  lineNumber,
      bool value
   )
      : Node(lineNumber), _value(value)
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;

   property Phx::Types::Type ^ Type
   {
      virtual Phx::Types::Type ^ get()
      {
      return Phx::GlobalData::TypeTable->BooleanType;
      }
   }

private:
   bool  _value;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     A string literal.
//    
//-----------------------------------------------------------------------------

ref class StringNode : public Node, public IExpression
{
public:
   StringNode
   (
      int   lineNumber,
      char* valStart,
      int   valLength
   )
      : Node(lineNumber), _string(gcnew String(valStart, 0, valLength))
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;

   property Phx::Types::Type ^ Type
   {
      virtual Phx::Types::Type ^ get()
      {
      return Phx::GlobalData::TypeTable->SystemStringAggregateType;
      }
   }

private:
   String^ _string;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     A formal parameter in a procedure definition.
//    
//-----------------------------------------------------------------------------

ref class FormalParameterNode : public Node
{
public:
   FormalParameterNode
   (
      int                lineNumber,
      Phx::Types::Type ^ symbolType,
      char*              paramNameStart,
      int                paramNameLength
   )
      : Node(lineNumber), _type(symbolType)
         , _parameter(gcnew String(paramNameStart, 0, paramNameLength))
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;

   property String ^ ParameterName
   {
      String ^ get()
      {
      return _parameter;
      }
   }

public:
   property Phx::Types::Type ^ Type { Phx::Types::Type ^ get() { return _type;
      } }

private:
   Phx::Types::Type ^ _type;
   String ^           _parameter;
};

//-----------------------------------------------------------------------------
//                               UNARY NODES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Description:
//
//     An assignment.
//    
//-----------------------------------------------------------------------------

ref class AssignmentNode : public UnaryNode
{
public:
   AssignmentNode
   (
      int   lineNumber,
      char* variableStart,
      int   variableLength,
      Node^ expression
   )
      : UnaryNode(lineNumber, expression), _variable(gcnew String(variableStart,
         0, variableLength))
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;

   // Record left-hand term in symbol table if not already there.

   virtual void CollectLocalVariables() override;

private:
   String^ _variable;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     An arithmetic negative expression.
//    
//-----------------------------------------------------------------------------

ref class UnaryMinusNode : public UnaryNode, public IExpression
{
public:
   UnaryMinusNode
   (
      int   lineNumber,
      Node^ first
   )
      : UnaryNode(lineNumber, first)
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;

   property Phx::Types::Type ^ Type
   {
      virtual Phx::Types::Type ^ get()
      {
      return Phx::GlobalData::TypeTable->Int32Type;
      }
   }
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     A logical negative expression.
//
//-----------------------------------------------------------------------------

ref class NegationNode : public UnaryNode, public IExpression
{
public:
   NegationNode
   (
      int   lineNumber,
      Node^ first
   )
      : UnaryNode(lineNumber, first)
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;

   property Phx::Types::Type ^ Type
   {
      virtual Phx::Types::Type ^ get()
      {
      return Phx::GlobalData::TypeTable->BooleanType;
      }
   }
};

//-----------------------------------------------------------------------------
//                            BINARY NODES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Description:
//
//     A binary operation of any type.
//
// Remarks: 
//
//    Abstract class whose derived classes differ with respect to code
//    generation and type checking.
//    
//-----------------------------------------------------------------------------

ref class BinaryNodeWithOperatorToken abstract : public BinaryNode
{
public:
   BinaryNodeWithOperatorToken
   (
      int      lineNumber,
      unsigned operatorToken,
      Node^    first,
      Node^    second
   )
      : _operator(operatorToken), BinaryNode(lineNumber, first, second)
   {
   }

   virtual void Dump(TextWriter^ out) override;

protected:
   unsigned _operator;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     A binary arithmetic operation.
//    
//-----------------------------------------------------------------------------

ref class BinaryArithmeticNode : public BinaryNodeWithOperatorToken,
   public IExpression
{
public:

   // operatorToken is one of { '*', '/', '+', '-' }

   BinaryArithmeticNode
   (
      int      lineNumber,
      unsigned operatorToken,
      Node^    first,
      Node^    second
   )
      : BinaryNodeWithOperatorToken(lineNumber, operatorToken, first, second)
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   property Phx::Types::Type ^ Type
   {
      virtual Phx::Types::Type ^ get()
      {
      return Phx::GlobalData::TypeTable->Int32Type;
      }
   }
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     A binary comparison between operands of any types.
//
// Remarks: 
//
//    Typechecking will reject meaningless comparisons.
//    
//-----------------------------------------------------------------------------

ref class ComparisonNode : public BinaryNodeWithOperatorToken,
   public IExpression
{
public:

   // operatorToken is one of { EQ, NE, LT, LE, GT, GE }

   ComparisonNode
   (
      int      lineNumber,
      unsigned operatorToken,
      Node^    first,
      Node^    second
   )
      : BinaryNodeWithOperatorToken(lineNumber, operatorToken, first, second)
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   property Phx::Types::Type ^ Type
   {
      virtual Phx::Types::Type ^ get()
      {
      return Phx::GlobalData::TypeTable->BooleanType;
      }
   }
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     And- or or-expression.
//    
//-----------------------------------------------------------------------------

ref class BinaryLogicalNode : public BinaryNodeWithOperatorToken,
   public IExpression
{
public:

   // operatorToken is one of { AND, OR }

   BinaryLogicalNode
   (
      int   lineNumber,
      short operatorToken,
      Node^ first,
      Node^ second
   )
      : BinaryNodeWithOperatorToken(lineNumber, operatorToken, first, second)
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   property Phx::Types::Type ^ Type
   {
      virtual Phx::Types::Type ^ get()
      {
      return Phx::GlobalData::TypeTable->BooleanType;
      }
   }
};

//-----------------------------------------------------------------------------
//                              TERNARY NODES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Description:
//
//     If-statement.
//    
//-----------------------------------------------------------------------------

ref class ConditionNode : public TernaryNode
{
public:
   ConditionNode
   (
      int   lineNumber,
      Node^ first,
      Node^ second,
      Node^ third
   )
      : TernaryNode(lineNumber, first, second, third)
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;
};

//-----------------------------------------------------------------------------
//                               POLYADIC NODES
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Description:
//
//     Ast node that represents a whole program.
//
// Remarks: 
//
//    A program is a list of procedure definitions.
//    
//-----------------------------------------------------------------------------

ref class ModuleNode : public PolyadicNode
{
public:
   ModuleNode
   (
      int          lineNumber,
      List<Node^>^ moduleContent
   )
      : PolyadicNode(lineNumber, moduleContent)
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     A procedure definition.
//
// Remarks: 
//
//    _childList[0] is the HeaderNode.
//    
//-----------------------------------------------------------------------------

ref class FunctionNode : public PolyadicNode
{
public:
   FunctionNode
   (
      int          lineNumber,
      List<Node^>^ childList
   )
      : PolyadicNode(lineNumber, childList)
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     The header of a procedure.
//    
//-----------------------------------------------------------------------------

ref class HeaderNode : public PolyadicNode
{
public:
   HeaderNode
   (
      int          lineNumber,
      char*        functionNameStart,
      int          functionNameLength,
      List<Node^>^ formalParameterList
   )
      : PolyadicNode(lineNumber, formalParameterList),
         _functionName(gcnew String(functionNameStart, 0, functionNameLength))
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;

   property String ^ FunctionName
   {
      String ^ get()
      {
      return _functionName;
      }
   }

   // Private methods.

private:
   void InitSymbols();

   // Private fields.

private:
   String^ _functionName;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     A block of statements.
//
// Remarks: 
//
//    All behavior is currently inherited from PolyadicNode. It still makes
//    sense to keep PolyadicNode an abstract class to allow for future
//    extensions.
//-----------------------------------------------------------------------------

ref class StatementBlockNode : public PolyadicNode
{
public:
   StatementBlockNode
   (
      int          lineNumber,
      List<Node^>^ statementList
   )
      : PolyadicNode(lineNumber, statementList)
   {
   }
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     A procedure call.
//
// Remarks: 
//
//    This can be a real procedure call or an invocation of the Print intrinsic.
//    
//-----------------------------------------------------------------------------

ref class FunctionCallNode : public PolyadicNode
{
public:
   FunctionCallNode
   (
      int          lineNumber,
      char*        functionNameStart,
      int          functionNameLength,
      List<Node^>^ actualParameterList
   )
      : PolyadicNode(lineNumber, actualParameterList),
         _function(gcnew String(functionNameStart, 0, functionNameLength))
   {
   }

   virtual void Evaluate(TextWriter^ out) override;

   virtual void Dump(TextWriter^ out) override;

private:
   void EvalPrintCall(TextWriter^ out);

   String^ _function;
};

