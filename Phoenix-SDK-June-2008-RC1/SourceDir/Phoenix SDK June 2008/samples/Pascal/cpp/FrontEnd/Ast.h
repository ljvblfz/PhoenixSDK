//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Definition of the AST node classes.
//
//-----------------------------------------------------------------------------

#pragma once

#include "Scanner.h"
#include "AstVisitor.h"
#include "Expression.h"

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Diagnostics;

namespace Ast
{

//-----------------------------------------------------------------------------
//
// Description:
//
//     Abstract class to describe an AST node.
//
// Remarks: 
//
//    
//-----------------------------------------------------------------------------

ref class Node abstract
{
public:

   Node
   (
      int sourceLineNumber
   )
      : sourceLineNumber(sourceLineNumber)
   {
   }

   // Determines whether this instance has child nodes.
   property bool HasChildren
   {
      virtual bool get()
      {
         // The default implementation has no children.
         return false;
      }
   }

   // Retrieves the source code line number that corresponds to 
   // this instance.
   property int SourceLineNumber
   {
      [DebuggerNonUserCode]
      int get()
      {
         return this->sourceLineNumber;
      }
   }

   // Allows any Node class to be associated with a string name.
   property String^ Name
   {
      virtual String^ get()
      {
         return String::Empty;
      }
   }
   
   // Accepts the given AST visitor object.
   virtual void Accept(IAstVisitor^ visitor) = 0;

protected:

   // Sets the current debug tag for the current function unit.
   [DebuggerNonUserCode]
   virtual void SetCurrentDebugTag();

protected:

   // The source code line number associated with this instance.
   int sourceLineNumber;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     Abstract class to represent nodes with one or more children.
//
// Remarks: 
//
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
      : Node(lineNumber), first(first)
   {
   }

   // A unary node has one child.
   
   property Node^ First
   {
      [DebuggerNonUserCode]
      Node^ get() { return this->first; }
   }

   property bool HasChildren
   {
      virtual bool get() override
      {
         return (this->first != nullptr);
      }
   }

   virtual void Accept(IAstVisitor^ visitor) override = 0;

protected:
   Node^ first;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     Abstract class to represent nodes with two children.
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
      : UnaryNode(lineNumber, first), second(second)
   {
   }

   // A binary node has two children.
   
   property Node^ Second
   {
      [DebuggerNonUserCode]
      Node^ get() { return this->second; }
   }

   property bool HasChildren
   {
      virtual bool get() override
      {
         return (UnaryNode::HasChildren && this->second != nullptr);
      }
   }

   virtual void Accept(IAstVisitor^ visitor) override = 0;

protected:
   Node^ second;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//     Abstract class to represent nodes with three children.
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
      : BinaryNode(lineNumber, first, second), third(third)
   {
   }

   // A ternary node has three children.

   property Node^ Third
   {
      [DebuggerNonUserCode]
      Node^ get() { return this->third; }
   }

   property bool HasChildren
   {
      virtual bool get() override
      {
         return (BinaryNode::HasChildren && this->second != nullptr);
      }
   }

   virtual void Accept(IAstVisitor^ visitor) override = 0;

protected:
   Node^ third;
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
      : Node(lineNumber), childList(childList)
   {
   }
   
   // A polyadic node has any number of children.

   property List<Node^>^ ChildList
   {
      [DebuggerNonUserCode]
      List<Node^>^ get() { return this->childList; }
   }

   property bool HasChildren
   {
      virtual bool get() override
      {
         if (this->childList == nullptr)
            return false;
         for each (Node^ node in this->childList)
            if (node == nullptr)
               return false;
         return true;
      }
   }

   virtual void Accept(IAstVisitor^ visitor) override = 0;

protected:
   List<Node^>^ childList;
};


//-----------------------------------------------------------------------------
//
// The following classes correspond to the grammar elements of the Pascal
// language. See pascal.y for details.
//
//-----------------------------------------------------------------------------


ref class ActualParameterListNode : public PolyadicNode
{
public:
   ActualParameterListNode
   (
      int            lineNumber,
      List<Node^>^   childList          // actual_parameter
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(ActualParameters != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<ActualParameterNode ^> ^ ActualParameters
   {  [DebuggerNonUserCode]
      List<ActualParameterNode ^> ^ get();
   }
   
private:
};

ref class ActualParameterNode : public TernaryNode
{
public:
   ActualParameterNode
   (
      int            lineNumber,
      Node^          first,         // expression
      Node^          second,        // expression | epsilon
      Node^          third          // expression | epsilon
   )
      : TernaryNode(lineNumber, first, second, third)
   {
      Debug::Assert(FirstExpression != nullptr);
      Debug::Assert(this->second == nullptr || SecondExpression != nullptr);
      Debug::Assert(this->third == nullptr || ThirdExpression != nullptr);
   }   

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ExpressionNode^ FirstExpression
   {  [DebuggerNonUserCode]
      ExpressionNode^ get();
   }
   property ExpressionNode^ SecondExpression
   {  [DebuggerNonUserCode]
      ExpressionNode^ get();
   }
   property ExpressionNode^ ThirdExpression
   {  [DebuggerNonUserCode]
      ExpressionNode^ get();
   }

private:
};

ref class ArrayTypeNode : public BinaryNode
{
public:
   ArrayTypeNode
   (
      int               lineNumber,
      Node^             first,      // index_list
      Node^             second      // component_type
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(IndexList != nullptr);
      Debug::Assert(ComponentType != nullptr);
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IndexListNode^ IndexList
   {  [DebuggerNonUserCode]
      IndexListNode^ get();
   }
   property ComponentTypeNode^ ComponentType
   {  [DebuggerNonUserCode]
      ComponentTypeNode^ get();
   }

private:
};

ref class AssignmentStatementNode : public BinaryNode
{
public:
   AssignmentStatementNode
   (
      int         lineNumber,
      Node^       first,         // variable_access
      Node^       second         // expression
   )
      : BinaryNode(lineNumber, first, second)    
   {
      Debug::Assert(VariableAccess != nullptr);
      Debug::Assert(Expression != nullptr);
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property VariableAccessNode^ VariableAccess
   {  [DebuggerNonUserCode]
      VariableAccessNode^ get();
   }
   property ExpressionNode^ Expression
   {  [DebuggerNonUserCode]
      ExpressionNode^ get();
   }

   property String^ Name
   {
      virtual String^ get() override;
   }

private:  
};

ref class BaseTypeNode : public UnaryNode
{
public:
   BaseTypeNode
   (
      int               lineNumber,
      Node^             first       // ordinal_type
   )
      : UnaryNode(lineNumber, first)      
   {
      Debug::Assert(OrdinalType != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property OrdinalTypeNode^ OrdinalType
   {  [DebuggerNonUserCode]
      OrdinalTypeNode^ get();
   }

private:
};

ref class BlockNode : public PolyadicNode
{
public:
   BlockNode
   (
      int             lineNumber,
      List<Node^>^    childList     // label_declaration_part
                                    // constant_declaration_part
                                    // type_definition_part
                                    // variable_declaration_part
                                    // procedure_and_function_declaration_part
                                    // statement_part
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(LabelDeclarationPart != nullptr);
      Debug::Assert(ConstantDefinitionPart != nullptr);
      Debug::Assert(TypeDefinitionPart != nullptr);
      Debug::Assert(VariableDeclarationPart != nullptr);
      Debug::Assert(ProcedureAndFunctionDeclarationPart != nullptr);
      Debug::Assert(StatementPart != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property LabelDeclarationPartNode^ LabelDeclarationPart
   {  [DebuggerNonUserCode]
      LabelDeclarationPartNode^ get();
   }
   property ConstantDefinitionPartNode^ ConstantDefinitionPart
   {  [DebuggerNonUserCode]
      ConstantDefinitionPartNode^ get();
   }
   property TypeDefinitionPartNode^ TypeDefinitionPart
   {  [DebuggerNonUserCode]
      TypeDefinitionPartNode^ get();
   }
   property VariableDeclarationPartNode^ VariableDeclarationPart
   {  [DebuggerNonUserCode]
      VariableDeclarationPartNode^ get();
   }
   property ProcedureAndFunctionDeclarationPartNode^ 
      ProcedureAndFunctionDeclarationPart
   {  [DebuggerNonUserCode]
      ProcedureAndFunctionDeclarationPartNode^ get();
   }
   property StatementPartNode^ StatementPart
   {  [DebuggerNonUserCode]
      StatementPartNode^ get();
   }
  
private:
};

ref class BooleanConstantNode : public Node, IExpression
{
public:
   BooleanConstantNode
   (
      int             lineNumber,
      bool            booleanValue
   )
      : Node(lineNumber)
      , value(booleanValue)
   {
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property bool Value
   {
      bool get()
      {  
         return this->value; 
      }
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
   bool value;
};

ref class BooleanExpressionNode : public UnaryNode, IExpression
{
public:
   BooleanExpressionNode
   (
      int        lineNumber,
      Node^      first        // expression
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Expression != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ExpressionNode^ Expression
   {  [DebuggerNonUserCode]
      ExpressionNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
};

ref class CaseConstantListNode : public PolyadicNode
{
public:
   CaseConstantListNode
   (
      int            lineNumber,
      List<Node^>^   childList      // case_constant
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(CaseConstants != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<CaseConstantNode ^> ^ CaseConstants
   {  [DebuggerNonUserCode]
      List<CaseConstantNode ^> ^ get();
   }

private:
};

ref class CaseConstantNode : public BinaryNode
{
public:
   CaseConstantNode
   (
      int            lineNumber,
      Node^          first,         // constant
      Node^          second         // constant | epsilon
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(FirstConstant != nullptr);
      Debug::Assert(this->second == nullptr || SecondConstant != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ConstantNode^ FirstConstant
   {  [DebuggerNonUserCode]
      ConstantNode^ get();
   }
   property ConstantNode^ SecondConstant
   {  [DebuggerNonUserCode]
      ConstantNode^ get();
   }

private:
};

ref class CaseIndexNode : public UnaryNode
{
public:
   CaseIndexNode
   (
      int            lineNumber,
      Node^          first          // expression
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Expression != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ExpressionNode^ Expression
   {  [DebuggerNonUserCode]
      ExpressionNode^ get();
   }

private:
};

ref class CaseListElementListNode : public PolyadicNode
{
public:
   CaseListElementListNode
   (
      int            lineNumber,
      List<Node^>^   childList          // case_list_element
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(CaseListElements != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<CaseListElementNode ^> ^ CaseListElements
   {  [DebuggerNonUserCode]
      List<CaseListElementNode ^> ^ get();
   }

private:
};

ref class CaseListElementNode : public BinaryNode
{
public:
   CaseListElementNode
   (
      int            lineNumber,
      Node^          first,         // case_constant_list
      Node^          second         // statement
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(CaseConstantList != nullptr);
      Debug::Assert(Statement != nullptr);
   }
 
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property CaseConstantListNode^ CaseConstantList
   {  [DebuggerNonUserCode]
      CaseConstantListNode^ get();
   }
   property StatementNode^ Statement
   {  [DebuggerNonUserCode]
      StatementNode^ get();
   }

private:
};

ref class CaseStatementNode : public PolyadicNode
{
public:
   CaseStatementNode
   (
      int            lineNumber,
      Node^          first,         // case_index
      Node^          second,        // case_list_element_list
      bool           semi1,         // was semicolon provided after 
                                    //  case_list_element_list?
      Node^          third,         // otherwisepart | epsilon
      Node^          forth,         // statement | epsilon
      bool           semi2          // was semicolon provided after statement?
   )
      : PolyadicNode(lineNumber, nullptr)
      , semi1(semi1)
      , semi2(semi2)
   {
      List<Node^>^ list = gcnew List<Node^>();
      list->Add(first);
      list->Add(second);
      list->Add(third);
      list->Add(forth);
      this->childList = list;

      Debug::Assert(CaseIndex != nullptr);
      Debug::Assert(CaseListElementList != nullptr);
      Debug::Assert(this->childList[2] == nullptr || OtherwisePart != nullptr);
      Debug::Assert(this->childList[3] == nullptr || Statement != nullptr);
   }

   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property CaseIndexNode^ CaseIndex
   {  [DebuggerNonUserCode]
      CaseIndexNode^ get();
   }
   property CaseListElementListNode^ CaseListElementList
   {  [DebuggerNonUserCode]
      CaseListElementListNode^ get();
   }
   property OtherwisePartNode^ OtherwisePart
   {  [DebuggerNonUserCode]
      OtherwisePartNode^ get();
   }
   property StatementNode^ Statement
   {  [DebuggerNonUserCode]
      StatementNode^ get();
   }
   
   property bool HasFirstSemicolon
   {
      bool get() { return this->semi1; }
   }
   property bool HasSecondSemicolon
   {
      bool get() { return this->semi2; }
   }
      
private:
   bool semi1, semi2;
};

ref class StatementNodeBase abstract : public BinaryNode
{
public:
   StatementNodeBase
   (
      int    lineNumber,
      Node^  first,
      Node^  second
   )
      : BinaryNode(lineNumber, first, second)
   {
   }  
};

ref class ClosedStatementNode : public StatementNodeBase
{
public:
   ClosedStatementNode
   (
      int    lineNumber,
      Node^  first,           // label | epsilon
      Node^  second           // non_labeled_closed_statement
   )
      : StatementNodeBase(lineNumber, first, second)
   {
      Debug::Assert(this->first == nullptr || Label != nullptr);
      Debug::Assert(NonLabeledClosedStatement != nullptr);
   }  
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property LabelNode^ Label
   {  [DebuggerNonUserCode]
      LabelNode^ get();
   }
   property NonLabeledClosedStatementNode^ NonLabeledClosedStatement
   {  [DebuggerNonUserCode]
      NonLabeledClosedStatementNode^ get();
   }

private:
};


ref class ComponentTypeNode : public UnaryNode, IExpression
{
public:
   ComponentTypeNode
   (
      int               lineNumber,
      Node^             first       // type_denoter
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(TypeDenoter != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property TypeDenoterNode^ TypeDenoter
   {  [DebuggerNonUserCode]
      TypeDenoterNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
};

ref class CompoundStatementNode : public UnaryNode
{
public:
   CompoundStatementNode
   (
      int      lineNumber,
      Node^    first          // statement_sequence
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(StatementSequence != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   
   property StatementSequenceNode^ StatementSequence
   {  [DebuggerNonUserCode]
      StatementSequenceNode^ get();
   }

private:
};

ref class ConstantDefinitionNode : public BinaryNode
{
public:
   ConstantDefinitionNode
   (
      int             lineNumber,
      Node^           first,        // identifier
      Node^           second        // cexpression
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(Identifier != nullptr);
      Debug::Assert(ConstantExpression != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }
   property ConstantExpressionNode^ ConstantExpression
   {  [DebuggerNonUserCode]
      ConstantExpressionNode^ get();
   }

private:
};

ref class ConstantDefinitionPartNode : public UnaryNode
{
public:
   ConstantDefinitionPartNode
   (
      int             lineNumber,
      Node^           first          // constant_list | epsilon
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(this->first == nullptr || ConstantList != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ConstantListNode^ ConstantList
   {  [DebuggerNonUserCode]
      ConstantListNode^ get();
   }

private:
};

ref class ConstantListNode : public PolyadicNode
{
public:
   ConstantListNode
   (
      int             lineNumber,
      List<Node^>^    childList        // constant_definition
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(ConstantDefinitions != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<ConstantDefinitionNode ^> ^ ConstantDefinitions
   {  [DebuggerNonUserCode]
      List<ConstantDefinitionNode ^> ^ get();
   }

private:
};

ref class ConstantNode : public UnaryNode, IExpression
{
public:
   ConstantNode
   (
      int         lineNumber,
      unsigned    sign,
      Node^       first         // non_string
   )
      : UnaryNode(lineNumber, first)
      , sign(sign)
   {
      Debug::Assert(NonString != nullptr);
   }

   ConstantNode
   (
      int         lineNumber,
      Node^       first         // non_string |
                                // CHARACTER_STRING
   )
      : UnaryNode(lineNumber, first)
      , sign(-1)
   {
      Debug::Assert(NonString != nullptr || CharacterString != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property NonStringNode^ NonString
   {  [DebuggerNonUserCode]
      NonStringNode^ get();
   }
   property CharacterStringNode^ CharacterString
   {  [DebuggerNonUserCode]
      CharacterStringNode^ get();
   }

   property unsigned Sign
   {
      unsigned get() { return this->sign; }
   }
   
   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
   unsigned sign;
};

ref class ControlVariableNode : public UnaryNode
{
public:
   ControlVariableNode
   (
      int            lineNumber,
      Node^          first          // identifier
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Identifier != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }

private:
};

ref class DirectionNode : public Node
{
public:
   DirectionNode
   (
      int          lineNumber,
      char *       identifierStart,
      int          identifierLength
   )
      : Node(lineNumber)
      , direction(gcnew String(identifierStart, 0, identifierLength))
   {
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property String^ Direction
   {
      String^ get()
      {
         return this->direction;
      }
   }

   property bool IsTo
   {
      bool get() 
      {
         return Direction->ToLower()->Equals("to");
      }
   }

   property bool IsDownTo
   {
      bool get() 
      {
         return Direction->ToLower()->Equals("downto");
      }
   }
   
private:
   String^ direction;
};

ref class DomainTypeNode : public UnaryNode
{
public:
   DomainTypeNode
   (
      int          lineNumber,
      Node^        first      // identifier
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Identifier != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }
   
private:
};

ref class EnumeratedTypeNode : public UnaryNode
{
public:
   EnumeratedTypeNode
   (
      int         lineNumber,
      Node^       first      // identifier_list
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(IdentifierList != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierListNode^ IdentifierList
   {  [DebuggerNonUserCode]
      IdentifierListNode^ get();
   }

private:
};

ref class ExponentiationBaseNode abstract : public BinaryNode, IExpression
{
protected:   
   ExponentiationBaseNode
   (
      int             lineNumber,     
      Node^           first         // primary
   )
      : BinaryNode(lineNumber, first, nullptr)
   {
   }

   ExponentiationBaseNode
   (
      int             lineNumber,     
      Node^           first,        // primary
      Node^           second        // exponentiation
   )
      : BinaryNode(lineNumber, first, second)
   {
   }

public:

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property bool IsExponentiationNode
   { [DebuggerNonUserCode]
      bool get();
   }
   property ExponentiationNode^ AsExponentiationNode
   { [DebuggerNonUserCode]
      ExponentiationNode^ get();
   }
   
   property bool IsConstantExponentiationNode 
   { [DebuggerNonUserCode]
      bool get();
   }
   property ConstantExponentiationNode^ AsConstantExponentiationNode
   { [DebuggerNonUserCode]
      ConstantExponentiationNode^ get();
   }
   
   // Retrieves the Type associated with this expression.

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() { return nullptr; }
   }

   // Performs type checking on this expression.

   virtual bool TypeCheck() { return false; }

private:
};

ref class ExponentiationNode : public ExponentiationBaseNode
{
public:
   ExponentiationNode
   (
      int             lineNumber,
      Node^           first         // primary
   )
      : ExponentiationBaseNode(lineNumber, first)
   {
      Debug::Assert(Primary != nullptr);
   }

   ExponentiationNode
   (
      int             lineNumber,
      Node^           first,        // primary
      Node^           second        // exponentiation
   )
      : ExponentiationBaseNode(lineNumber, first, second)
   {
      Debug::Assert(Primary != nullptr);
      Debug::Assert(Exponentiation != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property PrimaryNode^ Primary
   {  [DebuggerNonUserCode]
      PrimaryNode^ get();
   }
   property ExponentiationNode^ Exponentiation
   {  [DebuggerNonUserCode]
      ExponentiationNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() override;
   }

   virtual bool TypeCheck() override;

private:
};

ref class ConstantExponentiationNode : public ExponentiationBaseNode, 
                                              IExpression
{
public:
   ConstantExponentiationNode
   (
      int             lineNumber,
      Node^           first         // cprimary
   )
      : ExponentiationBaseNode(lineNumber, first)
   {
      Debug::Assert(ConstantPrimary != nullptr);
   }

   ConstantExponentiationNode
   (
      int             lineNumber,
      Node^           first,        // cprimary
      Node^           second        // cexponentiation
   )
      : ExponentiationBaseNode(lineNumber, first, second)
   {
      Debug::Assert(ConstantPrimary != nullptr);
      Debug::Assert(ConstantExponentiation != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ConstantPrimaryNode^ ConstantPrimary
   {  [DebuggerNonUserCode]
      ConstantPrimaryNode^ get();
   }
   property ConstantExponentiationNode^ ConstantExponentiation
   {  [DebuggerNonUserCode]
      ConstantExponentiationNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() override;
   }

   virtual bool TypeCheck() override;

private:
};


ref class ExpressionBaseNode abstract : public BinaryNode
{
protected:
   // Constructor for single simple expression.
   ExpressionBaseNode
   (
      int             lineNumber,
      Node^           first         // simple_expression
   )
      : BinaryNode(lineNumber, first, nullptr)
      , op(-1)
   {
   }

   // Constructor for relational operation between two simple expressions.
   ExpressionBaseNode
   (
      int             lineNumber,
      unsigned        relop,        // relop
      Node^           first,        // simple_expression
      Node^           second        // simple_expression
   )
      : BinaryNode(lineNumber, first, second)
      , op(relop)
   {
   }

public:

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property unsigned RelationalOperator
   {
      unsigned get() { return this->op; }
   }

   property bool IsExpressionNode
   { [DebuggerNonUserCode]
      bool get();
   }
   property ExpressionNode^ AsExpressionNode
   { [DebuggerNonUserCode]
      ExpressionNode^ get();
   }
   
   property bool IsConstantExpressionNode
   { [DebuggerNonUserCode]
      bool get();
   }
   property ConstantExpressionNode^ AsConstantExpressionNode
   { [DebuggerNonUserCode]
      ConstantExpressionNode^ get();
   }

private:
   unsigned op;  // {EQUAL, NOTEQUAL, LT, GT, LE, GE, IN}
};

// Constant expression node implementation.
ref class ConstantExpressionNode : public ExpressionBaseNode, IExpression
{
public:
   // Constructor for single simple expression.
   ConstantExpressionNode
   (
      int             lineNumber,
      Node^           first         // csimple_expression
   )
      : ExpressionBaseNode(lineNumber, first)
   {
      Debug::Assert(FirstConstantSimpleExpression != nullptr);
   }

   // Constructor for relational operation between two simple expressions.
   ConstantExpressionNode
   (
      int             lineNumber,
      unsigned        relop,        // relop
      Node^           first,        // csimple_expression
      Node^           second        // csimple_expression
   )
      : ExpressionBaseNode(lineNumber, relop, first, second)
   {
      Debug::Assert(FirstConstantSimpleExpression != nullptr);
      Debug::Assert(SecondConstantSimpleExpression != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ConstantSimpleExpressionNode^ FirstConstantSimpleExpression
   {  [DebuggerNonUserCode]
      ConstantSimpleExpressionNode^ get();
   }
   property ConstantSimpleExpressionNode^ SecondConstantSimpleExpression
   {  [DebuggerNonUserCode]
      ConstantSimpleExpressionNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();
};

ref class ExpressionNode : public ExpressionBaseNode, IExpression
{
public:
   // Constructor for single simple expression.
   ExpressionNode
   (
      int             lineNumber,
      Node^           first         // simple_expression
   )
      : ExpressionBaseNode(lineNumber, first)
   {
      Debug::Assert(FirstSimpleExpression != nullptr);
   }

   // Constructor for relational operation between two simple expressions.
   ExpressionNode
   (
      int             lineNumber,
      unsigned        relop,        // relop
      Node^           first,        // simple_expression
      Node^           second        // simple_expression
   )
      : ExpressionBaseNode(lineNumber, relop, first, second)
   {
      Debug::Assert(FirstSimpleExpression != nullptr);
      Debug::Assert(SecondSimpleExpression != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property SimpleExpressionNode^ FirstSimpleExpression
   {  [DebuggerNonUserCode]
      SimpleExpressionNode^ get();
   }
   property SimpleExpressionNode^ SecondSimpleExpression
   {  [DebuggerNonUserCode]
      SimpleExpressionNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
};

ref class FactorBaseNode abstract : public UnaryNode, IExpression
{
protected:
   // Constructor for single factor.
   FactorBaseNode
   (
      int             lineNumber,
      unsigned        sign,
      Node^           first         // factor | exponentiation
   )
      : UnaryNode(lineNumber, first)
      , sign(sign)
   {
   }

public:

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property bool IsFactorNode
   { [DebuggerNonUserCode]
      bool get();
   }
   property FactorNode^ AsFactorNode
   { [DebuggerNonUserCode]
      FactorNode^ get();
   }
   
   property bool IsConstantFactorNode
   { [DebuggerNonUserCode]
      bool get();
   }
   property ConstantFactorNode^ AsConstantFactorNode
   { [DebuggerNonUserCode]
      ConstantFactorNode^ get();
   }
   
   property unsigned Sign
   {
      unsigned get() { return this->sign; }
   }

   // Retrieves the Type associated with this expression.

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() { return nullptr; }
   }

   // Performs type checking on this expression.

   virtual bool TypeCheck() { return false; }

private:
   unsigned sign;  // {PLUS, MINUS}
};

ref class FactorNode : public FactorBaseNode
{
public:
   FactorNode
   (
      int             lineNumber,
      unsigned        sign,
      Node^           first         // factor
   )
      : FactorBaseNode(lineNumber, sign, first)
   {
      Debug::Assert(Factor != nullptr);
   }

   FactorNode
   (
      int             lineNumber,      
      Node^           first         // exponentiation
   )
      : FactorBaseNode(lineNumber, -1, first)
   {
      Debug::Assert(Exponentiation != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property FactorNode^ Factor
   {  [DebuggerNonUserCode]
      FactorNode^ get();
   }
   property ExponentiationNode^ Exponentiation
   {  [DebuggerNonUserCode]
      ExponentiationNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() override;
   }

   virtual bool TypeCheck() override;

private:
};

ref class ConstantFactorNode : public FactorBaseNode
{
public:

   ConstantFactorNode
   (
      int             lineNumber,
      unsigned        sign,
      Node^           first         // cfactor 
   )
      : FactorBaseNode(lineNumber, sign, first)
   {
      Debug::Assert(ConstantFactor != nullptr);
   }

   ConstantFactorNode
   (
      int             lineNumber,
      Node^           first         // cexponentiation
   )
      : FactorBaseNode(lineNumber, -1, first)
   {
      Debug::Assert(ConstantExponentiation != nullptr);
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ConstantFactorNode^ ConstantFactor
   {  [DebuggerNonUserCode]
      ConstantFactorNode^ get();
   }
   property ConstantExponentiationNode^ ConstantExponentiation
   {  [DebuggerNonUserCode]
      ConstantExponentiationNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() override;
   }

   virtual bool TypeCheck() override;

private:
};

ref class FieldDesignatorNode : public BinaryNode, IExpression
{
public:
   FieldDesignatorNode
   (
      int      lineNumber,
      Node^    first,         // variable_access
      Node^    second         // identifier
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(VariableAccess != nullptr);
      Debug::Assert(Identifier != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property VariableAccessNode^ VariableAccess
   {  [DebuggerNonUserCode]
      VariableAccessNode^ get();
   }
   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }

   property String^ Name
   {
      virtual String^ get() override;
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
};

ref class FileNode : public UnaryNode
{
public:
   FileNode
   (
      int               lineNumber,
      Node^             first       // program | module
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Program != nullptr || Module != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ProgramNode^ Program
   {  [DebuggerNonUserCode]
      ProgramNode^ get();
   }
   property ModuleNode^ Module
   {  [DebuggerNonUserCode]
      ModuleNode^ get();
   }

private:
};

ref class FileTypeNode : public UnaryNode
{
public:
   FileTypeNode
   (
      int               lineNumber,
      Node^             first       // component_type
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(ComponentType != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ComponentTypeNode^ ComponentType
   {  [DebuggerNonUserCode]
      ComponentTypeNode^ get();
   }

private:
};

ref class FinalValueNode : public UnaryNode
{
public:
   FinalValueNode
   (
      int            lineNumber,
      Node^          first          // expression
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Expression != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ExpressionNode^ Expression
   {  [DebuggerNonUserCode]
      ExpressionNode^ get();
   }

private:
};

ref class FormalParameterListNode : public UnaryNode
{
public:
   FormalParameterListNode
   (
      int                lineNumber,
      Node^              first        // formal_parameter_section_list
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(FormalParameterSectionList != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property FormalParameterSectionListNode^ FormalParameterSectionList
   {  [DebuggerNonUserCode]
      FormalParameterSectionListNode^ get();
   }

private:
};

ref class FormalParameterNode : public UnaryNode
{
public:
   FormalParameterNode
   (
      int           lineNumber,
      Node^         first        // identifier
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Identifier != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }

   property String^ Name
   {
      virtual String^ get() override;
   }

private:
};

ref class FormalParameterSectionListNode : public PolyadicNode
{
public:

   FormalParameterSectionListNode
   (
      int                lineNumber,
      List<Node^>^       childList     // formal_parameter_section
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(FormalParameterSections != nullptr);
   }
    
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<FormalParameterSectionNode ^> ^ FormalParameterSections
   {  [DebuggerNonUserCode]
      List<FormalParameterSectionNode ^> ^ get();
   }

private:
};

ref class FormalParameterSectionNode : public UnaryNode
{
public:

   FormalParameterSectionNode
   (
      int                lineNumber,
      Node^              first         // value_parameter_specification |
                                       // variable_parameter_specification |
                                       // procedural_parameter_specification |
                                       // functional_parameter_specification
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(ValueParameterSpecification != nullptr || 
         VariableParameterSpecification != nullptr || 
         ProceduralParameterSpecification != nullptr || 
         FunctionalParameterSpecification != nullptr);
   }
 
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ValueParameterSpecificationNode^ 
      ValueParameterSpecification
   {  [DebuggerNonUserCode]
      ValueParameterSpecificationNode^ get();
   }
   property VariableParameterSpecificationNode^ 
      VariableParameterSpecification
   {  [DebuggerNonUserCode]
      VariableParameterSpecificationNode^ get();
   }
   property ProceduralParameterSpecificationNode^ 
      ProceduralParameterSpecification
   {  [DebuggerNonUserCode]
      ProceduralParameterSpecificationNode^ get();
   }
   property FunctionalParameterSpecificationNode^ 
      FunctionalParameterSpecification
   {  [DebuggerNonUserCode]
      FunctionalParameterSpecificationNode^ get();
   }

private:
};

ref class ForStatementNode abstract : public PolyadicNode
{
protected:
   ForStatementNode
   (
      int          lineNumber,
      List<Node^>^ childList        
         // control_variable, initial_value, direction, final_value, statement
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(ControlVariable != nullptr);
      Debug::Assert(InitialValue != nullptr);
      Debug::Assert(Direction != nullptr);
      Debug::Assert(FinalValue != nullptr);
   }

public:

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ControlVariableNode^ ControlVariable
   {  [DebuggerNonUserCode]
      ControlVariableNode^ get();
   }
   property InitialValueNode^ InitialValue
   {  [DebuggerNonUserCode]
      InitialValueNode^ get();
   }
   property DirectionNode^ Direction
   {  [DebuggerNonUserCode]
      DirectionNode^ get();
   }
   property FinalValueNode^ FinalValue
   {  [DebuggerNonUserCode]
      FinalValueNode^ get();
   }

   property StatementNodeBase ^ Statement
   {
      StatementNodeBase ^ get()
      {
         return safe_cast<StatementNodeBase^>(this->childList[4]);
      }
   }

private:
};

ref class OpenForStatementNode : public ForStatementNode
{
public:
   OpenForStatementNode
   (
      int          lineNumber,
      List<Node^>^ childList        
   )
      : ForStatementNode(lineNumber, childList)
   {
      Debug::Assert(OpenStatement != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   
   
   property OpenStatementNode^ OpenStatement
   {  [DebuggerNonUserCode]
      OpenStatementNode^ get();
   }

private:
};

ref class ClosedForStatementNode : public ForStatementNode
{
public:
   ClosedForStatementNode
   (
      int          lineNumber,
      List<Node^>^ childList        
   )
      : ForStatementNode(lineNumber, childList)
   {
      Debug::Assert(ClosedStatement != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ClosedStatementNode^ ClosedStatement
   {  [DebuggerNonUserCode]
      ClosedStatementNode^ get();
   }

private:
};

ref class FunctionalParameterSpecificationNode : public UnaryNode
{
public:
   FunctionalParameterSpecificationNode
   (
      int               lineNumber,
      Node^             first          // function_heading
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(FunctionHeading != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property FunctionHeadingNode^ FunctionHeading
   {  [DebuggerNonUserCode]
      FunctionHeadingNode^ get();
   }

private:
};

ref class FunctionBlockNode : public UnaryNode
{
public:

   FunctionBlockNode
   (
      int               lineNumber,
      Node^             first          // block
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Block != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property BlockNode^ Block
   {  [DebuggerNonUserCode]
      BlockNode^ get();
   }

private:
};

ref class FunctionDeclarationNode : public BinaryNode
{
public:
   FunctionDeclarationNode
   (
      int         lineNumber,
      Node^       first,      // function_heading | function_identification
      Node^       second      // directive | function_block
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(FunctionHeading != nullptr || 
         FunctionIdentification != nullptr);
      Debug::Assert(Directive != nullptr || FunctionBlock != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property FunctionHeadingNode^ FunctionHeading
   {  [DebuggerNonUserCode]
      FunctionHeadingNode^ get();
   }
   property FunctionIdentificationNode^ FunctionIdentification
   {  [DebuggerNonUserCode]
      FunctionIdentificationNode^ get();
   }
   property DirectiveNode^ Directive
   {  [DebuggerNonUserCode]
      DirectiveNode^ get();
   }
   property FunctionBlockNode^ FunctionBlock
   {  [DebuggerNonUserCode]
      FunctionBlockNode^ get();
   }

private:
};

ref class FunctionDesignatorNode : public BinaryNode, IExpression
{
public:
   FunctionDesignatorNode
   (
      int            lineNumber,
      Node^          first,         // identifier
      Node^          second         // params
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(Identifier != nullptr);
      Debug::Assert(Params != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }
   property ParamsNode^ Params
   {  [DebuggerNonUserCode]
      ParamsNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
};

ref class FunctionHeadingNode : public TernaryNode
{
public:
   FunctionHeadingNode
   (
      int         lineNumber,
      Node^       first,         // identifier
      Node^       second,        // formal_parameter_list | epsilon
      Node^       third          // result_type
   )
      : TernaryNode(lineNumber, first, second, third)
   {
      Debug::Assert(Identifier != nullptr);
      Debug::Assert(this->second == nullptr || FormalParameterList != nullptr);
      Debug::Assert(ResultType != nullptr);
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }
   property FormalParameterListNode^ FormalParameterList
   {  [DebuggerNonUserCode]
      FormalParameterListNode^ get();
   }
   property ResultTypeNode^ ResultType
   {  [DebuggerNonUserCode]
      ResultTypeNode^ get();
   }

private:  
};

ref class FunctionIdentificationNode : public UnaryNode
{
public:
   FunctionIdentificationNode
   (
      int               lineNumber,
      Node^             first          // identifier
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Identifier != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }

private:
};


ref class GotoStatementNode : public UnaryNode
{
public:
   GotoStatementNode
   (
      int      lineNumber,
      Node^    first          // label
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Label != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property LabelNode^ Label
   {  [DebuggerNonUserCode]
      LabelNode^ get();
   }

private:
};

ref class IdentifierListNode : public PolyadicNode
{
public:
   IdentifierListNode
   (
      int               lineNumber,
      List<Node^>^      childList      // formal_parameter
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(FormalParameters != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<FormalParameterNode ^> ^ FormalParameters
   {  [DebuggerNonUserCode]
      List<FormalParameterNode ^> ^ get();
   }

private:
};

ref class IdentifierNode : public Node, IExpression
{
public:
   IdentifierNode
   (
      int          lineNumber,
      char*        identifierStart,
      int          identifierLength
   )
      : Node(lineNumber)
      , identifier(
         gcnew String(identifierStart, 0, identifierLength))
   {
      Fixup();
   }

   IdentifierNode
   (
      int          lineNumber,
      String ^     name
   )
      : Node(lineNumber)
      , identifier(name)
   {
      Fixup();
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property String^ Name
   {
      virtual String^ get() override
      {
         return this->identifier;
      }
   }
   
   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
   void Fixup();

private:
   String^ identifier;
};

ref class IfStatementNode abstract : public TernaryNode
{
protected:
   IfStatementNode
   (
      int   lineNumber,
      Node^ first,         // boolean_expression
      Node^ second,        // statement
      Node^ third          // statement
   )
      : TernaryNode(lineNumber, first, second, third)
   {
      Debug::Assert(BooleanExpression != nullptr);
	   
   }

public:

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property BooleanExpressionNode^ BooleanExpression
   {  [DebuggerNonUserCode]
      BooleanExpressionNode^ get();
   }

   property bool IsOpenIfStatementNode
   { [DebuggerNonUserCode]
      bool get();
   }
   property OpenIfStatementNode^ AsOpenIfStatementNode
   { [DebuggerNonUserCode]
      OpenIfStatementNode^ get();
   }

   property bool IsClosedIfStatementNode
   { [DebuggerNonUserCode]
      bool get();
   }
   property ClosedIfStatementNode^ AsClosedIfStatementNode
   { [DebuggerNonUserCode]
      ClosedIfStatementNode^ get();
   }

private:     
};

ref class OpenIfStatementNode : public IfStatementNode
{
public:
   OpenIfStatementNode
   (
      int   lineNumber,
      Node^ first,         // boolean_expression
      Node^ second,        // statement | closed_statement
      Node^ third          // open_statement | epsilon
   )
      : IfStatementNode(lineNumber, first, second, third)
   {
      Debug::Assert(Statement != nullptr || ClosedStatement != nullptr);
      Debug::Assert(this->third == nullptr || OpenStatement != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property StatementNode^ Statement
   {  [DebuggerNonUserCode]
      StatementNode^ get();
   }
   property ClosedStatementNode^ ClosedStatement
   {  [DebuggerNonUserCode]
      ClosedStatementNode^ get();
   }
   property OpenStatementNode^ OpenStatement
   {  [DebuggerNonUserCode]
      OpenStatementNode^ get();
   }

private:
};

ref class ClosedIfStatementNode : public IfStatementNode
{
public:
   ClosedIfStatementNode
   (
      int   lineNumber,
      Node^ first,         // boolean_expression
      Node^ second,        // closed_statement
      Node^ third          // closed_statement
   )
      : IfStatementNode(lineNumber, first, second, third)
   {
      Debug::Assert(FirstClosedStatement != nullptr);
      Debug::Assert(SecondClosedStatement != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ClosedStatementNode^ FirstClosedStatement
   {  [DebuggerNonUserCode]
      ClosedStatementNode^ get();
   }
   property ClosedStatementNode^ SecondClosedStatement
   {  [DebuggerNonUserCode]
      ClosedStatementNode^ get();
   }

private:
};

ref class IndexedVariableNode : public BinaryNode, IExpression
{
public:
   IndexedVariableNode
   (
      int   lineNumber,
      Node^ first,            // variable_access
      Node^ second            // index_expression_list
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(VariableAccess != nullptr);
      Debug::Assert(IndexExpressionList != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property VariableAccessNode^ VariableAccess
   {  [DebuggerNonUserCode]
      VariableAccessNode^ get();
   }
   property IndexExpressionListNode^ IndexExpressionList
   {  [DebuggerNonUserCode]
      IndexExpressionListNode^ get();
   }

   property String^ Name
   {
      virtual String^ get() override;
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
};

ref class IndexExpressionListNode : public PolyadicNode
{
public:
   IndexExpressionListNode
   (
      int            lineNumber,
      List<Node^>^   childList       // index_expression
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(IndexExpressions != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<IndexExpressionNode ^> ^ IndexExpressions
   {  [DebuggerNonUserCode]
      List<IndexExpressionNode ^> ^ get();
   }

private:
};

ref class IndexExpressionNode : public UnaryNode
{
public:
   IndexExpressionNode
   (
      int      lineNumber,
      Node^    first          // expression
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Expression != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ExpressionNode^ Expression
   {  [DebuggerNonUserCode]
      ExpressionNode^ get();
   }

private:
};

ref class IndexListNode : public PolyadicNode
{
public:
   IndexListNode
   (
      int               lineNumber,
      List<Node^>^      childList       // index_type
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(IndexTypes != nullptr);
   }
    
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<IndexTypeNode ^> ^ IndexTypes
   {  [DebuggerNonUserCode]
      List<IndexTypeNode ^> ^ get();
   }

private:
};

ref class IndexTypeNode : public UnaryNode
{
public:
   IndexTypeNode
   (
      int         lineNumber,
      Node^       first          // ordinal_type
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(OrdinalType != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property OrdinalTypeNode^ OrdinalType
   {  [DebuggerNonUserCode]
      OrdinalTypeNode^ get();
   }

private:
};

ref class InitialValueNode : public UnaryNode
{
public:
   InitialValueNode
   (
      int            lineNumber,
      Node^          first          // expression
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Expression != nullptr);
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ExpressionNode^ Expression
   {  [DebuggerNonUserCode]
      ExpressionNode^ get();
   }

private:
};

ref class LabelDeclarationPartNode : public UnaryNode
{
public:
   LabelDeclarationPartNode
   (
      int          lineNumber,
      Node^        first          // label_list | epsilon
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(this->first == nullptr || LabelList != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property LabelListNode^ LabelList
   {  [DebuggerNonUserCode]
      LabelListNode^ get();
   }

private:   
};

ref class LabelListNode : public PolyadicNode
{
public:
   LabelListNode
   (
      int             lineNumber,
      List<Node^>^    childList          // label
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(Labels != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<LabelNode ^> ^ Labels
   {  [DebuggerNonUserCode]
      List<LabelNode ^> ^ get();
   }

private:
};

ref class LabelNode : public Node
{
public:
   LabelNode
   (
      int      lineNumber,
      int      label
   )
      : Node(lineNumber), label(label)
   {
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property int Label
   {
      int get()
      {
         return this->label;
      }
   }

private:
   int label;
};

ref class MemberDesignatorListNode : public PolyadicNode, IExpression
{
public:
   MemberDesignatorListNode
   (
      int            lineNumber,
      List<Node^>^   childList      // member_designator
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(MemberDesignators != nullptr);
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<MemberDesignatorNode ^> ^ MemberDesignators
   {  [DebuggerNonUserCode]
      List<MemberDesignatorNode ^> ^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
};

ref class MemberDesignatorNode : public BinaryNode, IExpression
{
public:
   MemberDesignatorNode
   (
      int            lineNumber,
      Node^          first,         // expression | epsilon
      Node^          second         // expression
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(this->first == nullptr || FirstExpression != nullptr);
      Debug::Assert(SecondExpression != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ExpressionNode^ FirstExpression
   {  [DebuggerNonUserCode]
      ExpressionNode^ get();
   }
   property ExpressionNode^ SecondExpression
   {  [DebuggerNonUserCode]
      ExpressionNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
};

ref class ModuleNode : public PolyadicNode
{
public:
   ModuleNode
   (
      int          lineNumber,
      List<Node^>^ moduleContent    // constant_definition_part
                                    // type_definition_part
                                    // variable_declaration_part
                                    // procedure_and_function_declaration_part
   )
      : PolyadicNode(lineNumber, moduleContent)
   {
      Debug::Assert(ConstantDefinitionPart != nullptr);
      Debug::Assert(TypeDefinitionPart != nullptr);
      Debug::Assert(VariableDeclarationPart != nullptr);
      Debug::Assert(ProcedureAndFunctionDeclarationPart != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ConstantDefinitionPartNode^ ConstantDefinitionPart
   {  [DebuggerNonUserCode]
      ConstantDefinitionPartNode^ get();
   }
   property TypeDefinitionPartNode^ TypeDefinitionPart
   {  [DebuggerNonUserCode]
      TypeDefinitionPartNode^ get();
   }
   property VariableDeclarationPartNode^ VariableDeclarationPart
   {  [DebuggerNonUserCode]
      VariableDeclarationPartNode^ get();
   }
   property ProcedureAndFunctionDeclarationPartNode^ 
      ProcedureAndFunctionDeclarationPart
   {  [DebuggerNonUserCode]
      ProcedureAndFunctionDeclarationPartNode^ get();
   }

private:
};

ref class NewOrdinalTypeNode : public UnaryNode
{
public:
   NewOrdinalTypeNode
   (
      int         lineNumber,
      Node^       first      // enumerated_type | subrange_type
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(EnumeratedType != nullptr || SubrangeType != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property EnumeratedTypeNode^ EnumeratedType
   {  [DebuggerNonUserCode]
      EnumeratedTypeNode^ get();
   }
   property SubrangeTypeNode^ SubrangeType
   {  [DebuggerNonUserCode]
      SubrangeTypeNode^ get();
   }

private:
};

ref class NewPointerTypeNode : public UnaryNode
{
public:
   NewPointerTypeNode
   (
      int         lineNumber,
      char        uparrow,    // symbol used for uparrow, either -> or ^
      Node^       first       // domain_type
   )
      : UnaryNode(lineNumber, first)
      , uparrow(uparrow)
   {
      Debug::Assert(DomainType != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property DomainTypeNode^ DomainType
   {  [DebuggerNonUserCode]
      DomainTypeNode^ get();
   }

   property String^ UpArrow
   {
      String^ get()
      {
         switch (this->uparrow)
         {
         case '-':
            return "->";
         case '^':
            return "^";
         default:
            return String::Empty;
         }
      }
   }

private:
   char uparrow;
};

ref class NewStructuredTypeNode : public UnaryNode
{
public:
   NewStructuredTypeNode
   (
      int         lineNumber,
      Node^       first,         // strutured_type
      bool        packed         // user specified PACKED keyword
   )
      : UnaryNode(lineNumber, first)
      , packed(packed)
   {
      Debug::Assert(StructuredType != nullptr);
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property StructuredTypeNode^ StructuredType
   {  [DebuggerNonUserCode]
      StructuredTypeNode^ get();
   }

   property bool IsPacked
   {
      bool get() { return this->packed; }
   }

private:
   bool packed;
};

ref class NewTypeNode : public UnaryNode
{
public:
   NewTypeNode
   (
      int         lineNumber,
      Node^       first      // new_ordinal_type |
                             // new_structured_type |
                             // new_pointer_type
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(NewOrdinalType != nullptr || 
         NewStructuredType != nullptr || NewPointerType != nullptr);
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property NewOrdinalTypeNode^ NewOrdinalType
   {  [DebuggerNonUserCode]
      NewOrdinalTypeNode^ get();
   }
   property NewStructuredTypeNode^ NewStructuredType
   {  [DebuggerNonUserCode]
      NewStructuredTypeNode^ get();
   }
   property NewPointerTypeNode^ NewPointerType
   {  [DebuggerNonUserCode]
      NewPointerTypeNode^ get();
   }

private:
};

ref class NilNode : public Node, IExpression
{
public:
   NilNode
   (
      int      lineNumber      
   )
      : Node(lineNumber)
   {
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property String^ Nil
   {
      String^ get() { return this->nil; }
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:   
   literal String ^ nil = "nil";
};

ref class NonLabeledClosedStatementNode : public UnaryNode
{
public:
   NonLabeledClosedStatementNode
   (
      int    lineNumber,
      Node^  first            // assignment_statement |
   )                          // procedure_statement |		
                              // goto_statement |
                              // compound_statement |
                              // case_statement |
                              // repeat_statement |
                              // closed_if_statement |
                              // closed_while_statement |
                              // closed_for_statement |
                              // closed_with_statement |
                              // epsilon
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(this->first == nullptr || 
         (AssignmentStatement != nullptr || ProcedureStatement != nullptr || 
          GotoStatement != nullptr || CompoundStatement != nullptr || 
          CaseStatement != nullptr || RepeatStatement != nullptr || 
          ClosedIfStatement != nullptr || ClosedWhileStatement != nullptr || 
          ClosedForStatement != nullptr || ClosedWithStatement != nullptr));
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property AssignmentStatementNode^ AssignmentStatement
   {  [DebuggerNonUserCode]
      AssignmentStatementNode^ get();
   }
   property ProcedureStatementNode^ ProcedureStatement
   {  [DebuggerNonUserCode]
      ProcedureStatementNode^ get();
   }
   property GotoStatementNode^ GotoStatement
   {  [DebuggerNonUserCode]
      GotoStatementNode^ get();
   }
   property CompoundStatementNode^ CompoundStatement
   {  [DebuggerNonUserCode]
      CompoundStatementNode^ get();
   }
   property CaseStatementNode^ CaseStatement
   {  [DebuggerNonUserCode]
      CaseStatementNode^ get();
   }
   property RepeatStatementNode^ RepeatStatement
   {  [DebuggerNonUserCode]
      RepeatStatementNode^ get();
   }
   property ClosedIfStatementNode^ ClosedIfStatement
   {  [DebuggerNonUserCode]
      ClosedIfStatementNode^ get();
   }
   property ClosedWhileStatementNode^ ClosedWhileStatement
   {  [DebuggerNonUserCode]
      ClosedWhileStatementNode^ get();
   }
   property ClosedForStatementNode^ ClosedForStatement
   {  [DebuggerNonUserCode]
      ClosedForStatementNode^ get();
   }
   property ClosedWithStatementNode^ ClosedWithStatement
   {  [DebuggerNonUserCode]
      ClosedWithStatementNode^ get();
   }

private:
};

ref class NonLabeledOpenStatementNode : public UnaryNode
{
public:
   NonLabeledOpenStatementNode
   (
      int    lineNumber,
      Node^  first            // open_with_statement |
   )                          // open_if_statement |
                              // open_while_statement |
                              // open_for_statement
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(OpenWithStatement != nullptr || 
         OpenIfStatement != nullptr || OpenWhileStatement != nullptr || 
         OpenForStatement != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property OpenWithStatementNode^ OpenWithStatement
   {  [DebuggerNonUserCode]
      OpenWithStatementNode^ get();
   }
   property OpenIfStatementNode^ OpenIfStatement
   {  [DebuggerNonUserCode]
      OpenIfStatementNode^ get();
   }
   property OpenWhileStatementNode^ OpenWhileStatement
   {  [DebuggerNonUserCode]
      OpenWhileStatementNode^ get();
   }
   property OpenForStatementNode^ OpenForStatement
   {  [DebuggerNonUserCode]
      OpenForStatementNode^ get();
   }

private:
};

ref class NonStringNode : public Node, IExpression
{
public:
   NonStringNode
   (
      int      lineNumber,
      Node^    identifier
   )
      : Node(lineNumber)
      , identifier(identifier)
      , intVal(0)
      , realVal(0.0)
      , valueKind(ValueKind::Identifier)
   {
      Debug::Assert(Identifier != nullptr);
   }

   NonStringNode
   (
      int               lineNumber,
      int               value
   )
      : Node(lineNumber)
      , identifier(nullptr)
      , intVal(value)
      , realVal(0.0)
      , valueKind(ValueKind::IntValue)
   {
   }

   NonStringNode
   (
      int               lineNumber,
      double            value
   )
      : Node(lineNumber)
      , identifier(nullptr)
      , intVal(0)
      , realVal(value)
      , valueKind(ValueKind::RealValue)
   {
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property bool HasChildren
   {
      virtual bool get() override
      {
         return (this->identifier != nullptr);
      }
   }

   enum class ValueKind
   {  
      Identifier, 
      IntValue, 
      RealValue 
   };

   property ValueKind Kind
   {
      ValueKind get() { return this->valueKind; }
   }

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }

   property int IntegerValue
   {
      int get() { return this->intVal; }
   }

   property double RealValue
   {
      double get() { return this->realVal; }
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
   Node^ identifier;
   int intVal;
   double realVal;
   ValueKind valueKind;   
};

ref class OpenStatementNode : public StatementNodeBase
{
public:
   OpenStatementNode
   (
      int    lineNumber,
      Node^  first,           // label | epsilon
      Node^  second           // non_labeled_open_statement
   )
      : StatementNodeBase(lineNumber, first, second)
   {
      Debug::Assert(this->first == nullptr || Label != nullptr);
      Debug::Assert(NonLabeledOpenStatement != nullptr);
   }
    
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property LabelNode^ Label
   {  [DebuggerNonUserCode]
      LabelNode^ get();
   }
   property NonLabeledOpenStatementNode^ NonLabeledOpenStatement
   {  [DebuggerNonUserCode]
      NonLabeledOpenStatementNode^ get();
   }

private:
};

ref class OrdinalTypeNode : public UnaryNode
{
public:
   OrdinalTypeNode
   (
      int         lineNumber,
      Node^       first      // new_ordinal_type |
                             // identifier
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(NewOrdinalType != nullptr || Identifier != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property NewOrdinalTypeNode^ NewOrdinalType
   {  [DebuggerNonUserCode]
      NewOrdinalTypeNode^ get();
   }
   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }

private:
};

ref class OtherwisePartNode : public Node
{
public:
   OtherwisePartNode
   (
      int            lineNumber,
      bool           colon          // whether user supplied a colon 
   )
      : Node(lineNumber)
      , colon(colon)
   {
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property bool HasColon 
   {
      bool get() { return this->colon; }
   }
   
private:
   bool colon;
};

ref class ParamsNode : public UnaryNode
{
public:
   ParamsNode
   (
      int      lineNumber,
      Node^    first          // actual_parameter_list
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(ActualParameterList != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ActualParameterListNode^ ActualParameterList
   {  [DebuggerNonUserCode]
      ActualParameterListNode^ get();
   }

private:
};

ref class PrimaryBaseNode abstract : public UnaryNode, IExpression
{
protected:   
   PrimaryBaseNode
   (
      int             lineNumber,     
      Node^           first         
   )
      : UnaryNode(lineNumber, first)
   {
   }

public:

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property bool IsPrimaryNode
   { [DebuggerNonUserCode]
      bool get();
   }
   property PrimaryNode^ AsPrimaryNode
   { [DebuggerNonUserCode]
      PrimaryNode^ get();
   }

   property bool IsConstantPrimaryNode
   { [DebuggerNonUserCode]
      bool get();
   }
   property ConstantPrimaryNode^ AsConstantPrimaryNode
   { [DebuggerNonUserCode]
      ConstantPrimaryNode^ get();
   }

   // Retrieves the Type associated with this expression.

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() { return nullptr; }
   }

   // Performs type checking on this expression.

   virtual bool TypeCheck() { return false; }

private:
};

ref class PrimaryNode : public PrimaryBaseNode
{
public:
   PrimaryNode
   (
      int             lineNumber,
      Node^           first         // variable_access
                                    // unsigned_constant
                                    // function_designator
                                    // set_constructor
                                    // expression
                                    // NOT primary
   )
      : PrimaryBaseNode(lineNumber, first)
   {
      Debug::Assert(VariableAccess != nullptr || Expression != nullptr || 
         UnsignedConstant != nullptr || Primary != nullptr || 
         FunctionDesignator != nullptr || SetConstructor != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property VariableAccessNode^ VariableAccess
   {  [DebuggerNonUserCode]
      VariableAccessNode^ get();
   }
   property ExpressionNode^ Expression
   {  [DebuggerNonUserCode]
      ExpressionNode^ get();
   }
   property UnsignedConstantNode^ UnsignedConstant
   {  [DebuggerNonUserCode]
      UnsignedConstantNode^ get();
   }
   property PrimaryNode^ Primary
   {  [DebuggerNonUserCode]
      PrimaryNode^ get();
   }
   property FunctionDesignatorNode^ FunctionDesignator
   {  [DebuggerNonUserCode]
      FunctionDesignatorNode^ get();
   }
   property SetConstructorNode^ SetConstructor
   {  [DebuggerNonUserCode]
      SetConstructorNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() override;
   }

   virtual bool TypeCheck() override;

private:
};

ref class ConstantPrimaryNode : public PrimaryBaseNode
{
public:
   ConstantPrimaryNode
   (
      int             lineNumber,
      Node^           first         // identifier
                                    // cexpression
                                    // unsigned_constant
                                    // NOT cprimary
   )
      : PrimaryBaseNode(lineNumber, first)
   {
      Debug::Assert(Identifier != nullptr || ConstantExpression != nullptr ||
         UnsignedConstant != nullptr || ConstantPrimary != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }
   property ConstantExpressionNode^ ConstantExpression
   {  [DebuggerNonUserCode]
      ConstantExpressionNode^ get();
   }
   property UnsignedConstantNode^ UnsignedConstant
   {  [DebuggerNonUserCode]
      UnsignedConstantNode^ get();
   }
   property ConstantPrimaryNode^ ConstantPrimary
   {  [DebuggerNonUserCode]
      ConstantPrimaryNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() override;
   }

   virtual bool TypeCheck() override;

private:
};

ref class ProceduralParameterSpecificationNode : public UnaryNode
{
public:

   ProceduralParameterSpecificationNode
   (
      int               lineNumber,
      Node^             first          // procedure_heading
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(ProcedureHeading != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ProcedureHeadingNode^ ProcedureHeading
   {  [DebuggerNonUserCode]
      ProcedureHeadingNode^ get();
   }

private:
};

ref class ProcedureAndFunctionDeclarationPartNode : public UnaryNode
{
public:
   ProcedureAndFunctionDeclarationPartNode
   (
      int             lineNumber,
      Node^           first          // proc_or_func_declaration_list | epsilon
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(this->first == nullptr || 
         ProcedureOrFunctionDeclarationList != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ProcedureOrFunctionDeclarationListNode^ 
      ProcedureOrFunctionDeclarationList
   {  [DebuggerNonUserCode]
      ProcedureOrFunctionDeclarationListNode^ get();
   }

private:
};

ref class ProcedureBlockNode : public UnaryNode
{
public:

   ProcedureBlockNode
   (
      int               lineNumber,
      Node^             first          // block
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Block != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property BlockNode^ Block
   {  [DebuggerNonUserCode]
      BlockNode^ get();
   }

private:
};

ref class ProcedureDeclarationNode : public BinaryNode
{
public:
   ProcedureDeclarationNode
   (
      int         lineNumber,
      Node^       first,      // procedure_heading
      Node^       second      // directive | procedure_block
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(ProcedureHeading != nullptr);
      Debug::Assert(Directive != nullptr || ProcedureBlock != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ProcedureHeadingNode^ ProcedureHeading
   {  [DebuggerNonUserCode]
      ProcedureHeadingNode^ get();
   }
   property DirectiveNode^ Directive
   {  [DebuggerNonUserCode]
      DirectiveNode^ get();
   }
   property ProcedureBlockNode^ ProcedureBlock
   {  [DebuggerNonUserCode]
      ProcedureBlockNode^ get();
   }

private:
};

ref class ProcedureHeadingNode : public BinaryNode
{
public:
   ProcedureHeadingNode
   (
      int          lineNumber,
      Node^        first,        // procedure_identification  
      Node^        second        // formal_parameter_list | epsilon
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(ProcedureIdentification != nullptr);
      Debug::Assert(this->second == nullptr || FormalParameterList != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ProcedureIdentificationNode^ ProcedureIdentification
   {  [DebuggerNonUserCode]
      ProcedureIdentificationNode^ get();
   }
   property FormalParameterListNode^ FormalParameterList
   {  [DebuggerNonUserCode]
      FormalParameterListNode^ get();
   }

private:
};

ref class ProcedureIdentificationNode : public UnaryNode
{
public:

   ProcedureIdentificationNode
   (
      int               lineNumber,
      Node^             first          // identifier
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Identifier != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }

private:
};

ref class ProcedureOrFunctionDeclarationListNode : public PolyadicNode
{
public:
   ProcedureOrFunctionDeclarationListNode
   (
      int             lineNumber,
      List<Node^>^    childList        // proc_or_func_declaration
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(ProcedureOrFunctionDeclarations != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<ProcedureOrFunctionDeclarationNode ^> ^ 
      ProcedureOrFunctionDeclarations
   {  [DebuggerNonUserCode]
      List<ProcedureOrFunctionDeclarationNode ^> ^ get();
   }
   
private:
};

ref class ProcedureOrFunctionDeclarationNode : public UnaryNode
{
public:
   ProcedureOrFunctionDeclarationNode
   (
      int         lineNumber,
      Node^       first      // procedure_declaration |
                             // function_declaration
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(ProcedureDeclaration != nullptr || 
         FunctionDeclaration != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ProcedureDeclarationNode^ ProcedureDeclaration
   {  [DebuggerNonUserCode]
      ProcedureDeclarationNode^ get();
   }
   property FunctionDeclarationNode^ FunctionDeclaration
   {  [DebuggerNonUserCode]
      FunctionDeclarationNode^ get();
   }

private:
};

ref class ProcedureStatementNode : public BinaryNode
{
public:
   ProcedureStatementNode
   (
      int      lineNumber,
      Node^    first,         // identifier
      Node^    second         // params | epsilon
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(Identifier != nullptr);
      Debug::Assert(this->second == nullptr || Params != nullptr);
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }
   property ParamsNode^ Params
   {  [DebuggerNonUserCode]
      ParamsNode^ get();
   }

private:
};

ref class ProgramHeadingNode : public BinaryNode
{
public:
   ProgramHeadingNode
   (
      int          lineNumber,
      Node^        first,      // identifier
      Node^        second      // identifier_list | epsilon
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(Identifier != nullptr);
      Debug::Assert(this->second == nullptr || IdentifierList != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }
   property IdentifierListNode^ IdentifierList
   {  [DebuggerNonUserCode]
      IdentifierListNode^ get();
   }

private:   
};

ref class ProgramNode : public BinaryNode
{
public:
   ProgramNode
   (
      int               lineNumber,
      Node^             first,       // program_heading
      Node^             second       // block
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(ProgramHeading != nullptr);
      Debug::Assert(Block != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ProgramHeadingNode^ ProgramHeading
   {  [DebuggerNonUserCode]
      ProgramHeadingNode^ get();
   }
   property BlockNode^ Block
   {  [DebuggerNonUserCode]
      BlockNode^ get();
   }
  
private:   
};

ref class RecordSectionListNode : public PolyadicNode
{
public:
   RecordSectionListNode
   (
      int               lineNumber,
      List<Node^>^      childList      // record_section
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(RecordSections != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<RecordSectionNode ^> ^ RecordSections
   {  [DebuggerNonUserCode]
      List<RecordSectionNode ^> ^ get();
   }  

private:
};

ref class RecordSectionNode : public BinaryNode
{
public:
   RecordSectionNode
   (
      int        lineNumber,
      Node^      first,      // identifier_list
      Node^      second      // type_denoter
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(IdentifierList != nullptr);
      Debug::Assert(TypeDenoter != nullptr);
   }
     
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierListNode^ IdentifierList
   {  [DebuggerNonUserCode]
      IdentifierListNode^ get();
   }
   property TypeDenoterNode^ TypeDenoter
   {  [DebuggerNonUserCode]
      TypeDenoterNode^ get();
   }

private:
};

ref class RecordTypeNode : public BinaryNode
{
public:
   RecordTypeNode
   (
      int               lineNumber,
      Node^             first,      // record_section_list | epsilon
      Node^             second      // variant_part | epsilon
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(this->first == nullptr || RecordSectionList != nullptr);
      Debug::Assert(this->second == nullptr || VariantPart != nullptr);

      // At least one of the two members must be valid.
      Debug::Assert(RecordSectionList != nullptr || VariantPart != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property RecordSectionListNode^ RecordSectionList
   {  [DebuggerNonUserCode]
      RecordSectionListNode^ get();
   }
   property VariantPartNode^ VariantPart
   {  [DebuggerNonUserCode]
      VariantPartNode^ get();
   }
  
private:
};

ref class RecordVariableListNode : public PolyadicNode
{
public:
   RecordVariableListNode
   (
      int               lineNumber,
      List<Node^>^      childList      // variable_access
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(VariableAccesses != nullptr);
   }
 
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<VariableAccessNode ^> ^ VariableAccesses
   {  [DebuggerNonUserCode]
      List<VariableAccessNode ^> ^ get();
   }

private:
};

ref class RepeatStatementNode : public BinaryNode
{
public:
   RepeatStatementNode
   (
      int   lineNumber,
      Node^ first,         // statement_sequence
      Node^ second         // boolean_expression
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(StatementSequence != nullptr);
      Debug::Assert(BooleanExpression != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property StatementSequenceNode^ StatementSequence
   {  [DebuggerNonUserCode]
      StatementSequenceNode^ get();
   }
   property BooleanExpressionNode^ BooleanExpression
   {  [DebuggerNonUserCode]
      BooleanExpressionNode^ get();
   }

private:
};

ref class ResultTypeNode : public UnaryNode
{
public:
   ResultTypeNode
   (
      int         lineNumber,
      Node^       first         // identifier
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Identifier != nullptr);
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }
   
private:
};

ref class SetConstructorNode : public UnaryNode, IExpression
{
public:
   SetConstructorNode
   (
      int            lineNumber,
      Node^          first          // member_designator_list | epsilon
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(this->first == nullptr || MemberDesignatorList != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property MemberDesignatorListNode^ MemberDesignatorList
   {  [DebuggerNonUserCode]
      MemberDesignatorListNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
};

ref class SetTypeNode : public UnaryNode
{
public:
   SetTypeNode
   (
      int               lineNumber,
      Node^             first       // base_type
   )
      : UnaryNode(lineNumber, first)      
   {
      Debug::Assert(BaseType != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property BaseTypeNode^ BaseType
   {  [DebuggerNonUserCode]
      BaseTypeNode^ get();
   }

private:
};

ref class SimpleExpressionBaseNode abstract : public BinaryNode, IExpression
{
protected:
   // Constructor for single simple expression
   SimpleExpressionBaseNode
   (
      int             lineNumber,
      Node^           first         // term
   )
      : BinaryNode(lineNumber, first, nullptr)
      , addOp(-1)
   {
   }

   // Constructor for add operation between two simple expressions
   SimpleExpressionBaseNode
   (
      int             lineNumber,
      unsigned        addOp,        // addop
      Node^           first,        // simple_expression
      Node^           second        // term
   )
      : BinaryNode(lineNumber, first, second)      
      , addOp(addOp)
   {
   }

public:

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property bool IsSimpleExpressionNode
   { [DebuggerNonUserCode]
      bool get();
   }
   property SimpleExpressionNode^ AsSimpleExpressionNode
   { [DebuggerNonUserCode]
      SimpleExpressionNode^ get();
   }

   property bool IsConstantSimpleExpressionNode
   { [DebuggerNonUserCode]
      bool get();
   }
   property ConstantSimpleExpressionNode^ AsConstantSimpleExpressionNode
   { [DebuggerNonUserCode]
      ConstantSimpleExpressionNode^ get();
   }
   
   property unsigned AddOperator
   {
      unsigned get() { return this->addOp; }
   }

   // Retrieves the Type associated with this expression.

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() { return nullptr; }
   }

   // Performs type checking on this expression.

   virtual bool TypeCheck() { return false; }

private:
   unsigned addOp;  // {PLUS, MINUS, OR}
};

ref class SimpleExpressionNode : public SimpleExpressionBaseNode
{
public:
   // Constructor for single simple expression.
   SimpleExpressionNode
   (
      int             lineNumber,
      Node^           first         // term
   )
      : SimpleExpressionBaseNode(lineNumber, first)
   {
      Debug::Assert(FirstTerm != nullptr);
   }

   // Constructor for add operation between two simple expressions.
   SimpleExpressionNode
   (
      int             lineNumber,
      unsigned        addOp,        // addop
      Node^           first,        // simple_expression
      Node^           second        // term
   )
      : SimpleExpressionBaseNode(lineNumber, addOp, first, second)      
   {
      Debug::Assert(SimpleExpression != nullptr);
      Debug::Assert(SecondTerm != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property TermNode^ FirstTerm
   {  [DebuggerNonUserCode]
      TermNode^ get();
   }
   property SimpleExpressionNode^ SimpleExpression
   {  [DebuggerNonUserCode]
      SimpleExpressionNode^ get();
   }
   property TermNode^ SecondTerm
   {  [DebuggerNonUserCode]
      TermNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() override;
   }

   virtual bool TypeCheck() override;

private:
};

ref class ConstantSimpleExpressionNode : public SimpleExpressionBaseNode
{
public:
   // Constructor for single simple expression
   ConstantSimpleExpressionNode
   (
      int             lineNumber,
      Node^           first         // cterm
   )
      : SimpleExpressionBaseNode(lineNumber, first)
   {
      Debug::Assert(FirstConstantTerm != nullptr);
   }

   // Constructor for add operation between two simple expressions
   ConstantSimpleExpressionNode
   (
      int             lineNumber,
      unsigned        addOp,        // addop
      Node^           first,        // csimple_expression
      Node^           second        // cterm
   )
      : SimpleExpressionBaseNode(lineNumber, addOp, first, second)      
   {
      Debug::Assert(ConstantSimpleExpression != nullptr);
      Debug::Assert(SecondConstantTerm != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ConstantTermNode^ FirstConstantTerm
   {  [DebuggerNonUserCode]
      ConstantTermNode^ get();
   }
   property ConstantSimpleExpressionNode^ ConstantSimpleExpression
   {  [DebuggerNonUserCode]
      ConstantSimpleExpressionNode^ get();
   }
   property ConstantTermNode^ SecondConstantTerm
   {  [DebuggerNonUserCode]
      ConstantTermNode^ get();
   }
   
   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() override;
   }

   virtual bool TypeCheck() override;

};

ref class StatementNode : public UnaryNode
{
public:
   StatementNode
   (
      int    lineNumber,
      Node^  first         // open_statement |
                           // closed_statement
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(OpenStatement != nullptr || ClosedStatement != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property OpenStatementNode^ OpenStatement
   {  [DebuggerNonUserCode]
      OpenStatementNode^ get();
   }
   property ClosedStatementNode^ ClosedStatement
   {  [DebuggerNonUserCode]
      ClosedStatementNode^ get();
   }

private:
};

ref class StatementPartNode : public UnaryNode
{
public:
   StatementPartNode
   (
      int      lineNumber,
      Node^    first          // compound_statement
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(CompoundStatement != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property CompoundStatementNode^ CompoundStatement
   {  [DebuggerNonUserCode]
      CompoundStatementNode^ get();
   }
private:
};

ref class StatementSequenceNode : public PolyadicNode
{
public:
   StatementSequenceNode
   (
      int             lineNumber,
      List<Node^>^    childList      // statement
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(Statements != nullptr);
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<StatementNode ^> ^ Statements
   {  [DebuggerNonUserCode]
      List<StatementNode ^> ^ get();
   }

private:
};

ref class StringNode : public Node
{
public:
   StringNode
   (
      int         lineNumber,
      char*       valStart,
      int         valLength
   )
      : Node(lineNumber)
      , string(gcnew String(valStart, 0, valLength))
   {      
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property String ^ StringValue
   {
      virtual String ^ get()
      {
         return this->string;
      }
   }

   property bool IsCharacterType
   {
      virtual bool get()
      {
         return StringValue->Length == 1;
      }
   }

   property Char CharValue
   {
      virtual Char get()
      {
         Debug::Assert(IsCharacterType);
         return StringValue[0];
      }
   }

protected:
   String^ string;
};

ref class CharacterStringNode : public StringNode, IExpression
{
public:
   CharacterStringNode
   (
      int         lineNumber,
      char*       valStart,
      int         valLength
   )
      : StringNode(lineNumber, 
         valStart+1, valLength-2) // strip ' characters
   {
      Fixup();      
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
   // Checks for special character sequences.
   void Fixup();
   // Translates the given control character sequence to 
   // its runtime representation.
   static wchar_t TranslateControlCharacter(wchar_t);

private:
   static Dictionary<wchar_t, wchar_t> ^ controlCharacterMap;   
};


ref class DirectiveNode : public StringNode
{
public:
   DirectiveNode
   (
      int         lineNumber,
      char*       valStart,
      int         valLength
   )
      : StringNode(lineNumber, valStart, valLength)
   {
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

private:
};

ref class StructuredTypeNode : public UnaryNode
{
public:
   StructuredTypeNode
   (
      int         lineNumber,
      Node^       first      // array_type |
                             // record_type |
                             // set_type |
                             // file_type
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(ArrayType != nullptr || RecordType != nullptr ||
         SetType != nullptr || FileType != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ArrayTypeNode^ ArrayType
   {  [DebuggerNonUserCode]
      ArrayTypeNode^ get();
   }
   property RecordTypeNode^ RecordType
   {  [DebuggerNonUserCode]
      RecordTypeNode^ get();
   }
   property SetTypeNode^ SetType
   {  [DebuggerNonUserCode]
      SetTypeNode^ get();
   }
   property FileTypeNode^ FileType
   {  [DebuggerNonUserCode]
      FileTypeNode^ get();
   }

private:
};

ref class SubrangeTypeNode : public BinaryNode
{
public:
   SubrangeTypeNode
   (
      int         lineNumber,
      Node^       first,      // constant 
      Node^       second      // constant
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(FirstConstant != nullptr);
      Debug::Assert(SecondConstant != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ConstantNode^ FirstConstant
   {  [DebuggerNonUserCode]
      ConstantNode^ get();
   }
   property ConstantNode^ SecondConstant
   {  [DebuggerNonUserCode]
      ConstantNode^ get();
   }
   
private:
};

ref class TagFieldNode : public UnaryNode
{
public:
   TagFieldNode
   (
      int          lineNumber,
      Node^        first      // identifier
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Identifier != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }

private:
};

ref class TagTypeNode : public UnaryNode
{
public:
   TagTypeNode
   (
      int          lineNumber,
      Node^        first      // identifier
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Identifier != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }
   
private:
};

ref class TermBaseNode abstract : public BinaryNode, IExpression
{
protected:
   // Constructor for single factor
   TermBaseNode
   (
      int             lineNumber,
      Node^           first         // factor
   )
      : BinaryNode(lineNumber, first, nullptr)
      , mulOp(-1)
   {
   }

   // Constructor for mul operation between a term and a factor
   TermBaseNode
   (
      int             lineNumber,
      unsigned        mulOp,        // mulop
      Node^           first,        // term
      Node^           second        // factor
   )
      : BinaryNode(lineNumber, first, second)      
      , mulOp(mulOp)
   {
   }

public:

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property bool IsTermNode
   { [DebuggerNonUserCode]
      bool get();
   }
   property TermNode^ AsTermNode
   { [DebuggerNonUserCode]
      TermNode^ get();
   }

   property bool IsConstantTermNode
   { [DebuggerNonUserCode]
      bool get();
   }
   property ConstantTermNode^ AsConstantTermNode
   { [DebuggerNonUserCode]
      ConstantTermNode^ get();
   }

   property unsigned MulOperator
   {
      unsigned get() { return this->mulOp; }
   }

   // Retrieves the Type associated with this expression.

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() { return nullptr; }
   }

   // Performs type checking on this expression.

   virtual bool TypeCheck() { return false; }
 
private:
   unsigned mulOp;  // {STAR, SLASH, DIV, MOD, AND}
};

ref class TermNode : public TermBaseNode
{
public:
   // Constructor for single simple expression
   TermNode
   (
      int             lineNumber,
      Node^           first         // factor
   )
      : TermBaseNode(lineNumber, first)
   {
      Debug::Assert(FirstFactor != nullptr);
   }

   // Constructor for add operation between two simple expressions
   TermNode
   (
      int             lineNumber,
      unsigned        mulOp,        // mulop
      Node^           first,        // term
      Node^           second        // factor
   )
      : TermBaseNode(lineNumber, mulOp, first, second)      
   {
      Debug::Assert(Term != nullptr);
      Debug::Assert(SecondFactor != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property FactorNode^ FirstFactor
   {  [DebuggerNonUserCode]
      FactorNode^ get();
   }
   property TermNode^ Term
   {  [DebuggerNonUserCode]
      TermNode^ get();
   }
   property FactorNode^ SecondFactor
   {  [DebuggerNonUserCode]
      FactorNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() override;
   }

   virtual bool TypeCheck() override;
};

ref class ConstantTermNode : public TermBaseNode
{
public:
   // Constructor for single term
   ConstantTermNode
   (
      int             lineNumber,
      Node^           first         // cfactor
   )
      : TermBaseNode(lineNumber, first)
   {
      Debug::Assert(FirstConstantFactor != nullptr);
   }

   // Constructor for mul operation between a simple expression and a term
   ConstantTermNode
   (
      int             lineNumber,
      unsigned        mulOp,        // mulop
      Node^           first,        // cterm
      Node^           second        // cfactor
   )
      : TermBaseNode(lineNumber, mulOp, first, second)      
   {
      Debug::Assert(ConstantTerm != nullptr);
      Debug::Assert(SecondConstantFactor != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ConstantFactorNode^ FirstConstantFactor
   {  [DebuggerNonUserCode]
      ConstantFactorNode^ get();
   }
   property ConstantTermNode^ ConstantTerm
   {  [DebuggerNonUserCode]
      ConstantTermNode^ get();
   }
   property ConstantFactorNode^ SecondConstantFactor
   {  [DebuggerNonUserCode]
      ConstantFactorNode^ get();
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get() override;
   }

   virtual bool TypeCheck() override;
};

ref class TypeDefinitionListNode : public PolyadicNode
{
public:
   TypeDefinitionListNode
   (
      int             lineNumber,
      List<Node^>^    childList       // type_definition
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(TypeDefinitions != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<TypeDefinitionNode ^> ^ TypeDefinitions
   {  [DebuggerNonUserCode]
      List<TypeDefinitionNode ^> ^ get();
   }

private:
};

ref class TypeDefinitionNode : public BinaryNode
{
public:
   TypeDefinitionNode
   (
      int             lineNumber,
      Node^           first,        // identifier
      Node^           second        // type_denoter
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(Identifier != nullptr);
      Debug::Assert(TypeDenoter != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }
   property TypeDenoterNode^ TypeDenoter
   {  [DebuggerNonUserCode]
      TypeDenoterNode^ get();
   }

private:
};

ref class TypeDefinitionPartNode : public UnaryNode
{
public:
   TypeDefinitionPartNode
   (
      int             lineNumber,
      Node^           first        // type_definition_list | epsilon
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(this->first == nullptr || TypeDefinitionList != nullptr);
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property TypeDefinitionListNode^ TypeDefinitionList
   {  [DebuggerNonUserCode]
      TypeDefinitionListNode^ get();
   }

private:
};

ref class TypeDenoterNode : public UnaryNode, IExpression
{
public:
   TypeDenoterNode
   (
      int         lineNumber,
      Node^       first      // identifier |
                             // new_type
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(Identifier != nullptr || NewType != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }
   property NewTypeNode^ NewType
   {  [DebuggerNonUserCode]
      NewTypeNode^ get();
   }

   void
   ResolveType(String ^ name)
   {
      Debug::Assert(Identifier == nullptr);
      this->first = gcnew IdentifierNode(SourceLineNumber, name);
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
};

ref class UnsignedConstantNode : public UnaryNode, IExpression
{
public:
   UnsignedConstantNode
   (
      int             lineNumber,
      Node^           first         // unsigned_number |
                                    // CHARACTER_STRING |
                                    // NIL |
                                    // boolean_constant
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(UnsignedNumber != nullptr || CharacterString != nullptr ||
         Nil != nullptr || BooleanConstant != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property UnsignedNumberNode^ UnsignedNumber
   {  [DebuggerNonUserCode]
      UnsignedNumberNode^ get();
   }
   property CharacterStringNode^ CharacterString
   {  [DebuggerNonUserCode]
      CharacterStringNode^ get();
   }
   property NilNode^ Nil
   {  [DebuggerNonUserCode]
      NilNode^ get();
   }
   property BooleanConstantNode^ BooleanConstant
   {  [DebuggerNonUserCode]
      BooleanConstantNode^ get();
   }
   
   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
};

ref class UnsignedIntegerNode : public Node, IExpression
{
public:
   UnsignedIntegerNode
   (
      int             lineNumber,
      unsigned int    value
   )
      : Node(lineNumber)
      , value(value)
   {
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property unsigned int Value
   {
      unsigned int get() { return this->value; }
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
   unsigned int value;
};

ref class UnsignedNumberNode : public UnaryNode, IExpression
{
public:
   UnsignedNumberNode
   (
      int             lineNumber,
      Node^           first         // unsigned_integer
                                    // unsigned_real
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(UnsignedInteger != nullptr || UnsignedReal != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property UnsignedIntegerNode^ UnsignedInteger
   {  [DebuggerNonUserCode]
      UnsignedIntegerNode^ get();
   }
   property UnsignedRealNode^ UnsignedReal
   {  [DebuggerNonUserCode]
      UnsignedRealNode^ get();
   }
   
   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
};

ref class UnsignedRealNode : public Node, IExpression
{
public:
   UnsignedRealNode
   (
      int             lineNumber,
      double          value
   )
      : Node(lineNumber)
      , value(value)
   {
   }
  
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property double Value
   {
      double get()
      {  
         return this->value; 
      }
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
   double value;
};

ref class ValueParameterSpecificationNode : public BinaryNode
{
public:

   ValueParameterSpecificationNode
   (
      int               lineNumber,
      Node^             first,         // identifier_list
      Node^             second         // identifier 
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(IdentifierList != nullptr);
      Debug::Assert(Identifier != nullptr);
   }

   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierListNode^ IdentifierList
   {  [DebuggerNonUserCode]
      IdentifierListNode^ get();
   }
   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }

private:
};

ref class VariableAccessNode : public UnaryNode, IExpression
{
public:
   VariableAccessNode
   (
      int      lineNumber,      
      char     uparrow,
      Node^    first          // identifier | 
                              // indexed_variable |
                              // field_designator |
                              // variable_access UPARROW
   )
      : UnaryNode(lineNumber, first)
      , uparrow(uparrow)
      , cachedName(nullptr)
   {
      Debug::Assert(Identifier != nullptr || IndexedVariable != nullptr || 
         FieldDesignator != nullptr || VariableAccess != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }
   property IndexedVariableNode^ IndexedVariable
   {  [DebuggerNonUserCode]
      IndexedVariableNode^ get();
   }
   property FieldDesignatorNode^ FieldDesignator
   {  [DebuggerNonUserCode]
      FieldDesignatorNode^ get();
   }
   property VariableAccessNode^ VariableAccess
   {  [DebuggerNonUserCode]
      VariableAccessNode^ get();
   }

   property String^ UpArrow
   {
      String^ get()
      {
         switch (this->uparrow)
         {
         case '-':
            return "->";
         case '^':
            return "^";
         default:
            return String::Empty;
         }
      }
   }

   property String^ Name
   {
      virtual String^ get() override;
   }

   property virtual Phx::Types::Type ^ Type
   {
      Phx::Types::Type ^ get();
   }

   virtual bool TypeCheck();

private:
   char uparrow;
   String ^ cachedName;
};

ref class VariableDeclarationListNode : public PolyadicNode
{
public:
   VariableDeclarationListNode
   (
      int             lineNumber,
      List<Node^>^    childList         // variable_declaration
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(VariableDeclarations != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<VariableDeclarationNode ^> ^ VariableDeclarations
   { [DebuggerNonUserCode]
      List<VariableDeclarationNode ^> ^ get();
   }
   
private:
};

ref class VariableDeclarationNode : public BinaryNode
{
public:
   VariableDeclarationNode
   (
      int      lineNumber,      
      Node^    first,         // identifier |
                              // identifier_list
      Node^    second         // type_denoter
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(Identifier != nullptr || IdentifierList != nullptr);
      Debug::Assert(TypeDenoter != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }
   property IdentifierListNode^ IdentifierList
   {  [DebuggerNonUserCode]
      IdentifierListNode^ get();
   }
   property TypeDenoterNode^ TypeDenoter
   {  [DebuggerNonUserCode]
      TypeDenoterNode^ get();
   }

private:
};

ref class VariableDeclarationPartNode : public UnaryNode
{
public:
   VariableDeclarationPartNode
   (
      int             lineNumber,
      Node^           first         // variable_declaration_list
   )
      : UnaryNode(lineNumber, first)
   {
      Debug::Assert(this->first == nullptr || 
         VariableDeclarationList != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property VariableDeclarationListNode^ VariableDeclarationList
   {  [DebuggerNonUserCode]
      VariableDeclarationListNode^ get();
   }

private:
};

ref class VariableParameterSpecificationNode : public BinaryNode
{
public:

   VariableParameterSpecificationNode
   (
      int               lineNumber,
      Node^             first,         // identifier_list
      Node^             second         // identifier 
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(IdentifierList != nullptr);
      Debug::Assert(Identifier != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property IdentifierListNode^ IdentifierList
   {  [DebuggerNonUserCode]
      IdentifierListNode^ get();
   }
   property IdentifierNode^ Identifier
   {  [DebuggerNonUserCode]
      IdentifierNode^ get();
   }

private:
};

ref class VariantListNode : public PolyadicNode
{
public:
   VariantListNode
   (
      int            lineNumber,
      List<Node^>^   childList      // variant
   )
      : PolyadicNode(lineNumber, childList)
   {
      Debug::Assert(Variants != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property List<VariantNode ^> ^ Variants
   { [DebuggerNonUserCode]
      List<VariantNode ^> ^ get();
   }

private:
};

ref class VariantNode : public TernaryNode
{
public:
   VariantNode
   (
      int            lineNumber,
      Node^          first,         // case_constant_list
      Node^          second,        // record_section_list | epsilon
      Node^          third          // variant_part | epsilon
   )
      : TernaryNode(lineNumber, first, second, third)
   {
      Debug::Assert(CaseConstantList != nullptr);
      Debug::Assert(this->second == nullptr || RecordSectionList != nullptr);
      Debug::Assert(this->third == nullptr || VariantPart != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property CaseConstantListNode^ CaseConstantList
   {  [DebuggerNonUserCode]
      CaseConstantListNode^ get();
   }
   property RecordSectionListNode^ RecordSectionList
   {  [DebuggerNonUserCode]
      RecordSectionListNode^ get();
   }
   property VariantPartNode^ VariantPart
   {  [DebuggerNonUserCode]
      VariantPartNode^ get();
   }

private:
};

ref class VariantPartNode : public BinaryNode
{
public:
   VariantPartNode
   (
      int        lineNumber,
      Node^      first,      // variant_selector | epsilon
      Node^      second,     // variant_list | epsilon
      bool       semicolon   // whether a semicolon ended the statement
   )
      : BinaryNode(lineNumber, first, second)
      , semicolon(semicolon)
   {
      Debug::Assert(this->first == nullptr || VariantSelector != nullptr);
      Debug::Assert(this->second == nullptr || VariantList != nullptr);
   }

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property VariantSelectorNode^ VariantSelector
   {  [DebuggerNonUserCode]
      VariantSelectorNode^ get();
   }
   property VariantListNode^ VariantList
   {  [DebuggerNonUserCode]
      VariantListNode^ get();
   }

   property bool HasSemicolon
   {
      bool get()
      {
         return this->semicolon;
      }
   }
   
private:
   bool semicolon;
};


ref class VariantSelectorNode : public BinaryNode
{
public:
   VariantSelectorNode
   (
      int        lineNumber,
      Node^      first,      // tag_field | epsilon
      Node^      second      // tag_type
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(this->first == nullptr || TagField != nullptr);
      Debug::Assert(TagType != nullptr);
   }
    
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property TagFieldNode^ TagField
   {  [DebuggerNonUserCode]
      TagFieldNode^ get();
   }
   property TagTypeNode^ TagType
   {  [DebuggerNonUserCode]
      TagTypeNode^ get();
   }

private:
};

ref class WhileStatementNode abstract : public BinaryNode
{
protected:
   WhileStatementNode
   (
      int   lineNumber,
      Node^ first,         // boolean_expression
      Node^ second         // statement
   )
      : BinaryNode(lineNumber, first, second)
   {	   
      Debug::Assert(BooleanExpression != nullptr);
   }

   // Sets the current debug tag for the current function unit.
   virtual void SetCurrentDebugTag() override;

public:

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property BooleanExpressionNode^ BooleanExpression
   {  [DebuggerNonUserCode]
      BooleanExpressionNode^ get();
   }

   property StatementNodeBase ^ Statement
   {
      StatementNodeBase ^ get()
      {
         return safe_cast<StatementNodeBase^>(this->second);
      }
   }

private:
};

ref class OpenWhileStatementNode : public WhileStatementNode
{
public:
   OpenWhileStatementNode
   (
      int   lineNumber,
      Node^ first,         // boolean_expression
      Node^ second         // open_statement
   )
      : WhileStatementNode(lineNumber, first, second)
   {	   
      Debug::Assert(OpenStatement != nullptr);
   }
      
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property OpenStatementNode^ OpenStatement
   {  [DebuggerNonUserCode]
      OpenStatementNode^ get();
   }

private:
};

ref class ClosedWhileStatementNode : public WhileStatementNode
{
public:
   ClosedWhileStatementNode
   (
      int   lineNumber,
      Node^ first,         // boolean_expression
      Node^ second         // closed_statement
   )
      : WhileStatementNode(lineNumber, first, second)
   {
      Debug::Assert(ClosedStatement != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   
   
   property ClosedStatementNode^ ClosedStatement
   {  [DebuggerNonUserCode]
      ClosedStatementNode^ get();
   }

private:
};

ref class WithStatementNode abstract : public BinaryNode
{
protected:
   WithStatementNode
   (
      int   lineNumber,
      Node^ first,         // record_variable_list
      Node^ second         // statement
   )
      : BinaryNode(lineNumber, first, second)
   {
      Debug::Assert(RecordVariableList != nullptr);
   }

public:

   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property RecordVariableListNode^ RecordVariableList
   {  [DebuggerNonUserCode]
      RecordVariableListNode^ get();
   }

private:
};

ref class OpenWithStatementNode : public WithStatementNode
{
public:
   OpenWithStatementNode
   (
      int   lineNumber,
      Node^ first,         // record_variable_list
      Node^ second         // open_statement
   )
      : WithStatementNode(lineNumber, first, second)
   {
      Debug::Assert(OpenStatement != nullptr);
   }
   
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property OpenStatementNode^ OpenStatement
   {  [DebuggerNonUserCode]
      OpenStatementNode^ get();
   }

private:
};

ref class ClosedWithStatementNode : public WithStatementNode
{
public:
   ClosedWithStatementNode
   (
      int   lineNumber,
      Node^ first,         // record_variable_list
      Node^ second         // closed_statement
   )
      : WithStatementNode(lineNumber, first, second)
   {
      Debug::Assert(ClosedStatement != nullptr);
   }
      
   [DebuggerNonUserCode]
   virtual void Accept(IAstVisitor ^ visitor) override
   {  // Set the current debug tag
      SetCurrentDebugTag();
      // Hook back to the visitor's Visit method
      visitor->Visit(this);
   }   

   property ClosedStatementNode^ ClosedStatement
   {  [DebuggerNonUserCode]
      ClosedStatementNode^ get();
   }

private:
};

}  // namespace Ast
