//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Implementation of the Evaluator class.
//
// Remarks:
//
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include <memory.h>

using namespace System;

#include "AstClasses.h"
#include "Ast.h"
#include "Evaluator.h"
#include "YaccDeclarations.h"

namespace Pascal 
{

//-----------------------------------------------------------------------------
//
// Description: Container class for holding VariableAccessNode 
//              and FieldDesignatorNode objects.
//
// Remarks:     Because Pascal With statements can contain a combination
//              of variable access and field designators, this class
//              serves as a single container type for both.
//
//-----------------------------------------------------------------------------

ref class VariableOrFieldAccess
{
public:
   VariableOrFieldAccess(VariableAccessNode ^ variableAccess)
      : variableAccess(variableAccess)
      , fieldDesignator(nullptr)
   {
   }
   VariableOrFieldAccess(FieldDesignatorNode ^ fieldDesignator)
      : variableAccess(nullptr)
      , fieldDesignator(fieldDesignator)
   {
   }

   property VariableAccessNode ^ VariableAccess
   {
      VariableAccessNode ^ get() { return this->variableAccess; }
   }
   property FieldDesignatorNode ^ FieldDesignator
   {
      FieldDesignatorNode ^ get() { return this->fieldDesignator; }
   }

private:
   VariableAccessNode ^ variableAccess;
   FieldDesignatorNode ^ fieldDesignator;
};

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given Node object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   Node^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given UnaryNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   UnaryNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given BinaryNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   BinaryNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given TernaryNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   TernaryNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given PolyadicNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   PolyadicNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ActualParameterListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ActualParameterListNode^ node
)
{
   // Visit each actual parameter and add it to the current
   // argument list.

   for each (ActualParameterNode ^ actualParameter in node->ActualParameters)
   {
      int errorCount = Output::ErrorCount;
      
      actualParameter->Accept(this);
      
      // Add current operand if no error occurred while processing
      // the parameter.
      
      if (errorCount == Output::ErrorCount)
      {
         IRBuilder::ArgumentList->Add(CurrentOperand);
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ActualParameterNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ActualParameterNode^ node
)
{
   node->FirstExpression->Accept(this);

   // This is custom handling for the built-in write and writeln routines.

   if (node->SecondExpression)
   {
      // Cache the current operand object. The child expressions will
      // write over it.

      Phx::IR::Operand ^ current = CurrentOperand;

      // Visit the child expressions and save their operands.

      Phx::IR::Operand ^ first = nullptr;
      Phx::IR::Operand ^ second = nullptr;
      
      node->SecondExpression->Accept(this);
      first = CurrentOperand;

      if (node->ThirdExpression)
      {
         node->ThirdExpression->Accept(this);
         second = CurrentOperand;
      }

      // Create a new ArgumentModifier object to hold the width specifiers 
      // and append it to the argument modifier list.

      IRBuilder::ArgumentModifierList->Add(gcnew ArgumentModifier(
         current,
         first,
         second
         )
      );

      // Restore the current operand.

      CurrentOperand = current;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ArrayTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ArrayTypeNode^ node
)
{
   // Grammar:
   // ARRAY LBRAC index_list RBRAC OF component_type

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = ModuleBuilder::CurrentFunctionUnit;

   // Get the element type of the array type.

   Phx::Types::Type ^ elementType = node->ComponentType->Type;
   
   // If the element type is unknown, then an anonymous
   // type was provided, such as:
   //    A : array[1..2] of 1..30;
   // In this case, create a new element type.

   if (elementType->IsUnknown)
   {
      elementType = ResolveNewType(node->ComponentType->TypeDenoter);
   }

   // Array of files is currently unsupported.

   if (TypeBuilder::IsFileType(elementType))
   {
      Output::ReportError(
         node->SourceLineNumber,
         Error::FileArrayUnsupported);
      return;
   }

   int errorCount = Output::ErrorCount;
   
   // Because Pascal allows multidimensional arrays, build a list
   // of allowable ranges for each array rank.

   int indexTypeNameSuffix = 1;
   List<TypedValueRange ^> ^ indextypeRanges = 
      gcnew List<TypedValueRange ^>();
   for each (IndexTypeNode ^ indexTypeNode in 
      node->IndexList->IndexTypes)
   {
      Phx::Types::Type ^ indexType = 
         functionUnit->TypeTable->UnknownType;

      // Similar to array element types, build a new type if an 
      // anonymous one was provided.

      OrdinalTypeNode ^ ordinalTypeNode = indexTypeNode->OrdinalType;
      if (ordinalTypeNode->Identifier)
      {
         indexType = ordinalTypeNode->Identifier->Type;         
      }
      else
      {
         // User is building a new enumerated or subrange type.
         
         String ^ currentName = NewTypeName;

         // Append a unique suffix to the type name.
         // We use the '`' character so that the type is inaccessible
         // from the user program.

         NewTypeName += "`" + indexTypeNameSuffix.ToString();

         // Build the subrange or enumerated type.

         NewOrdinalTypeNode ^ newOrdinalTypeNode = 
            ordinalTypeNode->NewOrdinalType;
         if (newOrdinalTypeNode->SubrangeType)
         {
            newOrdinalTypeNode->SubrangeType->Accept(this);
         }
         else
         {
            newOrdinalTypeNode->EnumeratedType->Accept(this);
         }
         
         indexType = TypeBuilder::GetTargetType(NewTypeName);
         
         NewTypeName = currentName;
         ++indexTypeNameSuffix;
      }

      // Each index type must be an enum or subrange type.

      if (!indexType->IsEnumType && 
          !TypeBuilder::IsSubrangeType(indexType))
      {
         Output::ReportError(
            ordinalTypeNode->SourceLineNumber,
            Error::UnknownIndexType, 
            ordinalTypeNode->First->Name
         );
      }
      else
      {
         // Retrieve the upper- and lower-bounds of the index type
         // for range checking.

         int lowerBound = 0;
         int upperBound = 0;

         if (indexType->IsEnumType)
         {
            upperBound = 
               TypeBuilder::GetEnumUpperBound(indexType->AsEnumType);
         }
         else 
         {
            Debug::Assert(TypeBuilder::IsSubrangeType(indexType));

            ValueRange ^ range = 
               TypeBuilder::GetSubrangeTypeRange(indexType);
            lowerBound = range->Lower;
            upperBound = range->Upper;
         }

         TypedValueRange ^ valueRange = 
            gcnew TypedValueRange(indexType, lowerBound, upperBound);

         // Range must be greater than zero.

         if (valueRange->Range == 0)
         {
            Output::ReportError(
               node->SourceLineNumber,
               Error::ZeroIndexRange
            );
         }
         else if (valueRange->Range < 0)
         {
            Output::ReportError(
               node->SourceLineNumber, 
               Error::NegativeIndexRange
            );
         }

         indextypeRanges->Add(valueRange);         
      }
   }

   if (errorCount != Output::ErrorCount)
   {
      // We encountered at least one error; return.
      return;
   }

   // Calculate the number of elements in the array.

   unsigned int elementCount = static_cast<unsigned int>(
      indextypeRanges[indextypeRanges->Count - 1]->Range
   ); 
   for (int i = indextypeRanges->Count - 2; i >= 0; --i)
   {
      elementCount *= static_cast<unsigned int>(
         indextypeRanges[i]->Range);
   }

   // Calculate the overall size of the array.

   unsigned int bitSize = (elementType->BitSize * elementCount);

   // Create a new type symbol and type for the array type.

   Phx::Symbols::TypeSymbol ^ typeSymbol = 
      Phx::Symbols::TypeSymbol::New(
         functionUnit->SymbolTable, 0, 
         Phx::Name::New(functionUnit->Lifetime, NewTypeName));

   Phx::Types::UnmanagedArrayType ^ arrayType = 
      Phx::Types::UnmanagedArrayType::New(
         functionUnit->TypeTable, bitSize, 
         typeSymbol, elementType);

   // Add the new type to our internal type mapping.

   TypeBuilder::RegisterArrayType(
      arrayType, indextypeRanges->ToArray(),
      node->SourceLineNumber);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given AssignmentStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   AssignmentStatementNode^ node
)
{
   this->nonLocalSymbol = nullptr;

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;
   
   Phx::Types::Type ^ variableType = nullptr;

   // Remember the current error count when we entered this method.

   int errorCount = Output::ErrorCount;

   // Process the variable access. This will produce the current
   // operand if no errors occurred.

   node->VariableAccess->Accept(this);
   
   if (errorCount != Output::ErrorCount)
   {
      CurrentOperand = nullptr;
      return;
   }

   Phx::IR::Operand ^ varAccessOperand = CurrentOperand;

   Phx::Symbols::Symbol ^ varSymbol = varAccessOperand->Symbol;
   if (this->nonLocalSymbol)
   {
      varSymbol = this->nonLocalSymbol;
   }

   Phx::Types::Type ^ varType = varAccessOperand->Type;
      
   // The right-hand side of the assignment statement must either be a 
   // constant or a declared identifier.
   // The call to TypeCheck will report any errors.

   node->Expression->TypeCheck();

   // Ensure both left-hand and right-hand sides have the same type.

   Phx::Types::Type ^ expressionType = node->Expression->Type;
  
   if (! varType->Equals(expressionType) && 
       ! TypeBuilder::AreEquivalentTypes(varType, expressionType))
   {
      Output::ReportError(
         node->SourceLineNumber, 
         Error::BadVariableAssignment,
         node->VariableAccess->Name,
         TypeBuilder::GetSourceType(varType),
         TypeBuilder::GetSourceType(expressionType)
      );         
   }
   
   // Ensure assignment is not performed on a constant variable.

   if (varSymbol &&
      (varSymbol->IsConstantSymbol || 
       ModuleBuilder::IsLocalConstant(functionUnit, varSymbol)))
   {
      // Attempt to assign to constant value.

      Output::ReportError(
         node->SourceLineNumber, 
         Error::ConstantAssignment,
         node->VariableAccess->Name
      );
   }

   // Ensure assignment is not performed on a file variable.

   if (varSymbol && TypeBuilder::IsFileType(varSymbol->Type))
   {
      if (TypeBuilder::IsFileType(varAccessOperand->Field->Type))
      {
         // Attempt to assign to file value.

         Output::ReportError(
            node->SourceLineNumber, 
            Error::FileAssignment,
            node->VariableAccess->Name
         );
      }
   }

   // Check for assignment to procedure or functions, for example:
   // f := x;

   if (varSymbol && varSymbol->Type->IsFunctionType)
   {
      Output::ReportError(
         node->SourceLineNumber, 
         Error::FunctionAssignment,
         node->VariableAccess->Name
      );
   }

   // Only generate IR for the assignment if no errors were encountered.

   if (errorCount != Output::ErrorCount)
   {
      return;
   }

   // Generate IR for the assignment statement.

   errorCount = Output::ErrorCount;

   node->Expression->Accept(this);
   Phx::IR::Operand ^ exprOperand = CurrentOperand;

   if (errorCount != Output::ErrorCount)
   {
      return;
   }
   
   // Generate assignment statement.

   // We special case here for set assignment. In this case, 
   // call the runtime library function set_assign to copy the
   // set values.

   if (TypeBuilder::IsSetType(varAccessOperand->Type))
   {      
      // set_assign has the following prototype:
      //    set_assign(set_node * dst, set_node * src, 
      //               int type_width, int fileindex, 
      //               int sourceLineNumber);

      List<Phx::IR::Operand ^> ^ arguments = gcnew List<Phx::IR::Operand ^>();

      //dst
      arguments->Add(varAccessOperand);

      //src
      arguments->Add(exprOperand);

      //type_width
      arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) varAccessOperand->Type->AsPointerType->ReferentType->ByteSize)
      );

      //fileindex
      arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) ModuleBuilder::SourceFileIndex)
      );

      //sourceLineNumber
      arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) node->SourceLineNumber)
      );

      ModuleBuilder::Runtime->CallSetFunction(
         functionUnit,
         "set_assign",
         arguments,
         node->SourceLineNumber
      );     

      // Because set assignment will allocate memory from the heap,
      // mark the created set for release after the function exits.

      ModuleBuilder::AddSetSymbolToReleaseList(
         functionUnit,
         varAccessOperand->Symbol
         );

      CurrentOperand = varAccessOperand;
   }

   // Otherwise, call the helper function EmitAssign.

   else
   {
      CurrentOperand = IRBuilder::EmitAssign(
         functionUnit,
         varAccessOperand, 
         exprOperand, 
         node->SourceLineNumber
      );
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given BaseTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   BaseTypeNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given BlockNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   BlockNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given BooleanConstantNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   BooleanConstantNode^ node
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;
   
   // The user is accessing the 'true' or 'false' Boolean constant.
   // Create a variable operand object from the true or false
   // symbol we created upon program entry.

   Phx::Symbols::Symbol ^ booleanSymbol = ModuleBuilder::LookupSymbol(
      functionUnit,
      node->Value ? "true" : "false"
   );
   
   Phx::IR::VariableOperand ^ varOpnd = IRBuilder::CreateVariableOperand(
      functionUnit,
      booleanSymbol
   );

   CurrentOperand = varOpnd;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given BooleanExpressionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   BooleanExpressionNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given CaseConstantListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   CaseConstantListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given CaseConstantNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   CaseConstantNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given CaseIndexNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   CaseIndexNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given CaseListElementListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   CaseListElementListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given CaseListElementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   CaseListElementNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given CaseStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   CaseStatementNode^ node
)
{
   // Grammar:
   // CASE case_index OF case_list_element_list END
   // CASE case_index OF case_list_element_list SEMICOLON END
   // CASE case_index OF case_list_element_list SEMICOLON 
   //    otherwisepart statement END
   // CASE case_index OF case_list_element_list SEMICOLON 
   //    otherwisepart statement SEMICOLON END
   
   // Note the above grammar allows for the statement 
   // section to end with a semicolon.
   // Since this is mainly a front-end notion, we only need 
   // to focus on the first and third variations.

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   int sourceLineNumber = node->SourceLineNumber;

   // Ensure the case index is of ordinal type.

   Phx::Types::Type ^ caseIndexType = node->CaseIndex->Expression->Type;
   if (! TypeBuilder::IsOrdinalType(caseIndexType))
   {
      Output::ReportError(
         sourceLineNumber,
         Error::GeneralTypeExpected,
         "ordinal"
      );
      return;
   }

   // Create the default label instruction for the case statement.

   Phx::IR::LabelInstruction ^ defaultLabelInstruction = 
      Phx::IR::LabelInstruction::New(functionUnit);

   // Create a target label to exit the case statement.

   Phx::IR::LabelInstruction ^ exitCaseLabelInstruction = 
      Phx::IR::LabelInstruction::New(functionUnit);

   // Generate the switch expression operand.

   node->CaseIndex->Expression->Accept(this);
   Phx::IR::Operand ^ switchExpressionOperand = CurrentOperand;

   // Convert the switch expression operand to integer type if it is of
   // non-integer ordinal type.

   if (caseIndexType->IsEnumType)
   {
      switchExpressionOperand = IRBuilder::ExtractPrimaryEnumField(
         functionUnit,
         switchExpressionOperand
      );     
   }

   // Create a switch instruction object.

   Phx::IR::SwitchInstruction ^ switchInstructon = 
      Phx::IR::SwitchInstruction::New(
         functionUnit,
         Phx::Common::Opcode::Switch,
         switchExpressionOperand,
         defaultLabelInstruction
      );    

   // Create parallel lists of LabelInstruction and StatementNode objects.
   // We collect the information first so we can generate case label
   // instructions and add them to the switch instruction before we
   // emit the actual code for them.

   List<Phx::IR::LabelInstruction ^> ^ labelInstructions = 
      gcnew List<Phx::IR::LabelInstruction ^>();
   List<StatementNode ^> ^ statements = 
      gcnew List<StatementNode ^>();

   // Case selectors must be exclusive to the Case statement.
   // Remember each range we added.

   List<ValueRange ^> ^ ranges = gcnew List<ValueRange ^>();      
   
   // Process the case list elements.

   for each (CaseListElementNode ^ caseListElementNode in 
      node->CaseListElementList->CaseListElements)
   {
      Phx::IR::LabelInstruction ^ labelInstruction = 
         Phx::IR::LabelInstruction::New(functionUnit);

      labelInstructions->Add(labelInstruction);

      statements->Add(caseListElementNode->Statement);
      
      for each (CaseConstantNode ^ caseConstantNode in 
         caseListElementNode->CaseConstantList->CaseConstants)
      {
         // A case constant can consist of a single value or a range of values.
         // We always have at least the first constant.
         
         // Obtain the ordinal value for the lower bound.

         bool error = false;
         int lowerValue = GetOrdinalValue(
            functionUnit,
            caseConstantNode->FirstConstant, 
            error
         );

         if (error)
         {
            Output::ReportError(
               caseConstantNode->SourceLineNumber,
               Error::GeneralTypeExpected,
               "constant ordinal"
            );
            continue;
         }

         // Verify type of lower bound is compatible with the case index.

         Phx::Types::Type ^ lowerType = caseConstantNode->FirstConstant->Type;
         if (! caseIndexType->Equals(lowerType))
         {
            Output::ReportError(
               caseConstantNode->FirstConstant->SourceLineNumber,
               Error::IncompatibleCaseIndexType,
               TypeBuilder::GetSourceType(lowerType),
               TypeBuilder::GetSourceType(caseIndexType)
            );
            continue;
         }

         // Create an upper limit value equal to the current lower limit value.

         int upperValue = lowerValue;

         if (caseConstantNode->SecondConstant)
         {
            // This is a range; update upperValue.

            error = false;
            upperValue = GetOrdinalValue(
               functionUnit,
               caseConstantNode->SecondConstant, 
               error
            );
            if (error)
            {
               Output::ReportError(
                  caseConstantNode->SourceLineNumber,
                  Error::GeneralTypeExpected,
                  "constant ordinal"
               );
               continue;
            }

            // Verify type of upper bound is compatible with the case index.

            Phx::Types::Type ^ upperType = 
               caseConstantNode->SecondConstant->Type;
            if (! caseIndexType->Equals(upperType))
            {
               Output::ReportError(
                  caseConstantNode->SecondConstant->SourceLineNumber,
                  Error::IncompatibleCaseIndexType,
                  TypeBuilder::GetSourceType(upperType),
                  TypeBuilder::GetSourceType(caseIndexType)
               );
               continue;
            }
         }

         // Verify that no ranges in the case statement overlap.

         for each (ValueRange ^ range in ranges)
         {
            bool lowerError = false;
            bool upperError = false;
            
            if (range->Contains(lowerValue))
            {
               lowerError = true;
            }
            if (range->Contains(upperValue))
            {
               upperError = true;
            }

            if (lowerError || upperError)
            {
               if (lowerError && upperError)
               {
                  Output::ReportError(                 
                     caseConstantNode->SourceLineNumber,
                     Error::CaseStatementRange,
                     lowerValue, 
                     upperValue
                  );
               }
               else if (lowerError)
               {
                  Output::ReportError(                 
                     caseConstantNode->SourceLineNumber,
                     Error::CaseStatementValue,
                     lowerValue
                  );
               }
               else
               {
                  Output::ReportError(                 
                     caseConstantNode->SourceLineNumber,
                     Error::CaseStatementValue,
                     upperValue
                  );
               }
               break;
            }
         }

         // Append range value to range list.

         ranges->Add(gcnew ValueRange(lowerValue, upperValue));
         
         // Append the current range to the switch instruction.

         switchInstructon->AppendCaseLabel(
            labelInstruction, 
            lowerValue, 
            upperValue
         );
      }      
   }

   // Now that we've filled in the switch instruction, emit it.

   CurrentOperand = IRBuilder::EmitSwitch(
      functionUnit,
      switchInstructon,
      sourceLineNumber
   );

   // Now emit all of the case labels, along with their statement parts.
   
   Debug::Assert(statements->Count == labelInstructions->Count);
   int count = statements->Count;
   for (int i = 0; i < count; ++i)
   {
      Phx::IR::LabelInstruction ^ labelInstruction = labelInstructions[i];
      StatementNode ^ statementNode = statements[i];

      IRBuilder::EmitLabel(functionUnit, labelInstruction);

      statementNode->Accept(this);
      
      // Since control flow is explicit, jump to the exit label.

      IRBuilder::EmitGoto(
         functionUnit, 
         functionUnit->LastInstruction->Previous, 
         exitCaseLabelInstruction, 
         statementNode->SourceLineNumber
      );
   }

   // Emit the default case label.

   IRBuilder::EmitLabel(functionUnit, defaultLabelInstruction);

   // If there was a provided otherwise_part, append the statement now.
   
   if (node->OtherwisePart)
   {
      node->Statement->Accept(this);

      sourceLineNumber = node->Statement->SourceLineNumber;
   }


   // Since Phoenix control flow is explicit, jump to the 
   // exit label (regardless whether there was an 
   // otherwise_part or not).

   IRBuilder::EmitGoto(
      functionUnit, 
      functionUnit->LastInstruction->Previous, 
      exitCaseLabelInstruction,       
      sourceLineNumber
   );
   
   // Emit the exit case label.

   IRBuilder::EmitLabel(functionUnit, exitCaseLabelInstruction);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given CharacterStringNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   CharacterStringNode^ node
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // For single character strings, we can just build
   // an immediate operand of type 'char' with the 
   // character literal.

   if (node->IsCharacterType)
   {
      CurrentOperand = Phx::IR::ImmediateOperand::New(
         functionUnit,
         TypeBuilder::GetTargetType(NativeType::Char),
         node->CharValue
      );
   }

   // Otherwise, get the string from the global string table,
   // create a proxy in the current function unit,
   // and create a variable operand.

   else
   {
      // GetStringSymbol will create a new entry in the
      // string table if a matching entry does not exist;
      // otherwise, it returns the shared string.

      Phx::Symbols::GlobalVariableSymbol ^ globalSymbol = 
         ModuleBuilder::GetStringSymbol(
            functionUnit->ParentModuleUnit, 
            node->StringValue
         );

      Phx::Symbols::NonLocalVariableSymbol ^ localStringSymbol =
         ModuleBuilder::MakeProxyInFunctionSymbolTable(
            globalSymbol, 
            functionUnit->SymbolTable
         );

      Phx::IR::VariableOperand ^ stringOperand = 
         Phx::IR::VariableOperand::New(
            functionUnit,
            TypeBuilder::GetTargetType(NativeType::Char),
            localStringSymbol
         );

      // Change to address operand.

      stringOperand->ChangeToAddress();

      CurrentOperand = stringOperand;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ClosedForStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ClosedForStatementNode^ node
)
{
   BuildForLoop(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ClosedIfStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ClosedIfStatementNode^ node
)
{
   // Get the current function unit.

   Phx::FunctionUnit^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   int sourceLineNumber = node->SourceLineNumber;

   // Evaluate the Boolean expression branch of the AST. 
   // This evaluation will reduce any compound Boolean expressions
   // into a single IR operand.

   int errorCount = Output::ErrorCount;
   node->BooleanExpression->Accept(this);
   if (errorCount != Output::ErrorCount)
   {
      return;
   }

   // Evaluation of the Boolean expression sets the
   // current operand to a true/false condition operand.

   Phx::IR::Operand ^ conditionOperand = CurrentOperand;

   // Generate label target instructions for the true 
   // and false arms of the if statement.

   Phx::IR::LabelInstruction ^ trueLabelInstruction  =
      Phx::IR::LabelInstruction::New(functionUnit);

   Phx::IR::LabelInstruction ^ falseLabelInstruction =
      Phx::IR::LabelInstruction::New(functionUnit);

   // Because Phoenix control flow is explicit, generate a 
   // shared target label to which both true and false arms 
   // will jump after execution.

   Phx::IR::LabelInstruction ^ endifLabelInstruction =
      Phx::IR::LabelInstruction::New(functionUnit);

   // Append a conditional branch instruction to 
   // the IR instruction stream.

   IRBuilder::EmitConditionalBranch(
      functionUnit,
      conditionOperand,
      trueLabelInstruction, 
      falseLabelInstruction
   );

   // Construct the 'true' arm of the conditional statement. 
   // This process includes three parts:
   // 1. A target label for the branch.
   // 2. Evaluation of the 'true' statement part. This is 
   //    part of the AST.
   // 3. An unconditional branch to the end of the if statement.

   IRBuilder::EmitLabel(functionUnit, trueLabelInstruction);

   node->FirstClosedStatement->Accept(this);

   IRBuilder::EmitGoto(
      functionUnit, 
      endifLabelInstruction, 
      sourceLineNumber
   );

   // Now construct the 'false' arm of the conditional statement. 
   // This includes the same three component parts as the 
   // 'true' arm of the if statement.

   IRBuilder::EmitLabel(functionUnit, falseLabelInstruction);

   node->SecondClosedStatement->Accept(this);

   IRBuilder::EmitGoto(
      functionUnit, 
      endifLabelInstruction, 
      sourceLineNumber
   );

   // Now emit the final label that exits the conditional statement.

   IRBuilder::EmitLabel(functionUnit, endifLabelInstruction);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ClosedStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ClosedStatementNode^ node
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // If the statement starts with a label, build it now.

   if (node->Label)
   {
      ModuleBuilder::BuildLabel(
         functionUnit,
         node->Label->Label,
         node->SourceLineNumber
      );
   }

   // Process the closed statement part.

   node->NonLabeledClosedStatement->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ClosedWhileStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ClosedWhileStatementNode^ node
)
{
   BuildWhileLoop(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ClosedWithStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ClosedWithStatementNode^ node
)
{
   BuildWithStatement(node, node->ClosedStatement);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ComponentTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ComponentTypeNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given CompoundStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   CompoundStatementNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ConstantDefinitionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ConstantDefinitionNode^ node
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // The type of each constant is defined by the 
   // node's right-hand-side expression.

   Phx::Types::Type ^ constantType = node->ConstantExpression->Type;

   // Create a normal local variable symbol for the constant.
   // We don't create a ConstantSymbol because we don't want the constant to be
   // declared globally.

   Phx::Symbols::Symbol ^ constantSymbol = 
      ModuleBuilder::AddVariableDeclarationSymbol(
         functionUnit,
         node->Identifier->Name, 
         constantType,
         Phx::Symbols::StorageClass::Auto,
         node->SourceLineNumber
      );

   Phx::IR::VariableOperand ^ destinationOperand = 
      Phx::IR::VariableOperand::New(
         functionUnit,
         constantType,
         constantSymbol
      );

   node->ConstantExpression->Accept(this);
   Phx::IR::Operand ^ sourceOperand = CurrentOperand;
  
   // Generate assignment statement.
   
   IRBuilder::EmitAssign(
      functionUnit,
      destinationOperand, 
      sourceOperand, 
      node->SourceLineNumber
   );

   // Add a new ConstantValue object to our mapping 
   // of constant -> symbol values.

   ConstantValue ^ constantValue;
   if (sourceOperand->IsImmediateOperand)
   {
      constantValue = 
         ConstantValue::FromImmediateOperand(
            sourceOperand->AsImmediateOperand);
   }
   else if (sourceOperand->Type->Equals(
      TypeBuilder::GetTargetType(NativeType::Boolean)))
   {
      constantValue = 
         ConstantValue::FromBoolean(sourceOperand->Symbol);
   }
   else
   {
      constantValue = 
         ConstantValue::FromString(sourceOperand->Symbol);
   }

   ModuleBuilder::AddLocalConstant(
      functionUnit, 
      constantSymbol,
      constantValue
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ConstantDefinitionPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ConstantDefinitionPartNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ConstantExponentiationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ConstantExponentiationNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ConstantExpressionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ConstantExpressionNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ConstantFactorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ConstantFactorNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ConstantListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ConstantListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ConstantNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ConstantNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ConstantPrimaryNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ConstantPrimaryNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ConstantSimpleExpressionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ConstantSimpleExpressionNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ConstantTermNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ConstantTermNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ControlVariableNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ControlVariableNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given DirectionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   DirectionNode^ node
)
{   
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given DirectiveNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   DirectiveNode^ node
)
{   
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given DomainTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   DomainTypeNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given EnumeratedTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   EnumeratedTypeNode^ node
)
{
   int sourceLineNumber = node->SourceLineNumber;
   
   // Get the current function unit and its symbol table.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;
   
   Phx::Symbols::Table ^ symbolTable = functionUnit->SymbolTable;

   // Check whether the type was already defined.

   // Note: The EnumeratedTypeNode class contains onlythe 
   // identifier list of the new enumerated type.
   // The NewTypeName property is the program name of the 
   // enumerated type and is set by a parent of this node.

   String ^ typeName = NewTypeName;

   Phx::Symbols::TypeSymbol ^ typeSymbol = 
      ModuleBuilder::LookupTypeSymbol(
         functionUnit,
         typeName
      );
   if (typeSymbol)
   {
      // The type has already been defined.
      // Report the error and return.

      Output::ReportError(
         sourceLineNumber,
         Error::TypeRedefinition,
         typeName
      );
      return;
   }

   // Create a new EnumType object to describe the type.
   // The EnumType class derives from AggregateType. In our
   // case, enum types contain a single integer field.

   Phx::Types::PrimitiveType ^ underlyingEnumType = 
      TypeBuilder::GetTargetType(NativeType::Integer)->AsPrimitiveType;

   // First, create a TypeSymbol object for the type.

   typeSymbol = Phx::Symbols::TypeSymbol::New(
      symbolTable,
      0,
      Phx::Name::New(symbolTable->Lifetime, typeName)
   );

   // Now create the EnumType object.

   Phx::Types::EnumType ^ enumType = Phx::Types::EnumType::New(
      functionUnit->TypeTable,
      typeSymbol,
      underlyingEnumType
   );

   // Append the integer field to the enum type.
   // Prepend the symbol name with characters that are not valid 
   // in a Pascal program to prevent user programs from accessing
   // this field.

   String ^ fieldName = "$$data";

   Phx::Symbols::FieldSymbol ^ fieldSymbol = 
      Phx::Symbols::FieldSymbol::New(
         symbolTable,
         0,
         Phx::Name::New(functionUnit->Lifetime, fieldName),
      underlyingEnumType
   );

   enumType->AppendFieldSymbol(fieldSymbol);

   // Set metaproperties for the enum type.

   enumType->IsOnlyData = true;
   enumType->IsSealed = true;
   enumType->HasCompleteInheritanceInfo = true;
   enumType->HasCompleteOverrideInfo = true;
   enumType->IsSelfDescribing = true;

   // Generate a symbol for each identifier that is defined within
   // this enum.

   List<Phx::Symbols::Symbol ^> ^ scalarSymbols = 
      gcnew List<Phx::Symbols::Symbol ^>();
   
   int n = 0; // the contant value of the current enum.
   
   for each (FormalParameterNode ^ formalParamNode in 
      node->IdentifierList->FormalParameters)
   {
      String ^ scalarName = formalParamNode->Identifier->Name;

      int errors = Output::ErrorCount;

      Phx::Symbols::Symbol ^ scalarSymbol = 
         ModuleBuilder::AddVariableDeclarationSymbol(      
            functionUnit,
            scalarName,
            enumType,
            Phx::Symbols::StorageClass::Auto,
            formalParamNode->SourceLineNumber
         );

      // If an error occurred when we added the variable declaration
      // symbol, (e.g. the variable name was already used), 
      // generate a new unique symbol placeholder so that we may 
      // continue with the rest of the enum.

      if (errors != Output::ErrorCount)
      {
         scalarName = ModuleBuilder::GenerateUniqueSymbolName();

         scalarSymbol = 
            ModuleBuilder::AddVariableDeclarationSymbol(      
               functionUnit,
               scalarName,
               enumType,
               Phx::Symbols::StorageClass::Auto,
               formalParamNode->SourceLineNumber
            );
      }

      // Add the new symbol to the symbol list.
      
      scalarSymbols->Add(scalarSymbol);
           
      // Emit an assignment statement for the current enum value.
     
      // First, create the base operand.

      Phx::IR::VariableOperand ^ baseOperand = 
         Phx::IR::VariableOperand::New(
            functionUnit,
            enumType,
            scalarSymbol
         );

      // Change it to an address operand.

      baseOperand->ChangeToAddress();

      // Now load the primary integer field from the 
      // address operand.

      Phx::IR::MemoryOperand ^ destinationOperand = 
         Phx::IR::MemoryOperand::New(
            functionUnit,
            enumType->GetField(underlyingEnumType, 0),
            nullptr,
            baseOperand,
            0,
            Phx::Alignment::NaturalAlignment(underlyingEnumType),
            functionUnit->AliasInfo->IndirectAliasedMemoryTag,
            functionUnit->SafetyInfo->SafeTag
         );

      // Now generate the source operand for the assignment.
      // In this case, it is a constant immediate value.

      Phx::IR::ImmediateOperand ^ sourceOperand = 
         Phx::IR::ImmediateOperand::New(
            functionUnit,
            underlyingEnumType,
            (int) n
         );

      // Generate assignment statement by using the EmitAssign
      // helper function.

      IRBuilder::EmitAssign(
         functionUnit,
         destinationOperand,
         sourceOperand, 
         sourceLineNumber
      );

      // Add the new symbol to the local constant table to
      // prevent the symbol from being re-assigned within the program.

      ModuleBuilder::AddLocalConstant(
         functionUnit, 
         scalarSymbol,
         gcnew ConstantValue((int)n)
      );

      // Increment current enum value.

      ++n;
   }

   // Register the new enum type with the type builder.

   TypeBuilder::RegisterEnumType(enumType, scalarSymbols);

   // Register the valid range for the field symbol.
   // The ValueRange class is an internal classed used 
   // to perform compile- and run-time range checking.

   ValueRange ^ enumRange = gcnew ValueRange(
      0,
      node->IdentifierList->FormalParameters->Count - 1
   );

   // Register the type range with the type builder.

   TypeBuilder::RegisterTypeRange(
      enumType,
      enumRange,
      sourceLineNumber
   );   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ExponentiationBaseNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ExponentiationBaseNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ExponentiationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ExponentiationNode^ node
)
{
   // An exponentiation takes the following form:
   // primary |
   // primary STARSTAR exponentiation

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // Process the primary part of the expression.

   node->Primary->Accept(this);

   // If an exponentiation was provided, process it now.

   if (node->Exponentiation)
   {      
      // Perform the x**y part here.
      // There is no intrinsic exponentiation instruction,
      // so we will make a call to the 'pow' runtime routine.

      List<Phx::IR::Operand ^> ^ arguments = 
         gcnew List<Phx::IR::Operand ^>();

      // The current operand was set by the Primary node.

      arguments->Add(CurrentOperand);

      // Process the exponentiation part and append the result
      // to the argument list.

      node->Exponentiation->Accept(this);
      arguments->Add(CurrentOperand);

      Phx::IR::Instruction ^ callInstruction = 
         ModuleBuilder::Runtime->CallMathFunction(
            functionUnit,
            "pow",
            arguments,
            node->SourceLineNumber
         );

      CurrentOperand = callInstruction->DestinationOperand;
   }  
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ExpressionBaseNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ExpressionBaseNode^ node
)
{
   throw gcnew Exception("Visit() not implemented for ExpressionBaseNode^");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ExpressionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ExpressionNode^ node
)
{
   // An expression takes the following form:
   // simple_expression |
   // simple_expression relop simple_expression
     
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // Process the left- and right-hand sides of the expression
   // if it is a compound expression.

   if (node->SecondSimpleExpression)
   {      
      Phx::Types::Type ^ leftType = node->FirstSimpleExpression->Type;
      Phx::Types::Type ^ rightType = node->SecondSimpleExpression->Type;

      // Ensure both side are type-compatible.

      if (! TypeBuilder::AreRelationalTypesCompatible(
         leftType,
         rightType,
         node->RelationalOperator) )
      {
         Output::ReportError(node->SourceLineNumber,
            Error::IncompatibleOperandTypes,
            TypeBuilder::GetSourceType(leftType),
            Utility::GetOperatorString(node->RelationalOperator),
            TypeBuilder::GetSourceType(rightType)
         );
         return;
      }

      // Process both sides of the expression. Each evaluation
      // produces an operand.

      node->FirstSimpleExpression->Accept(this);
      Phx::IR::Operand ^ left = CurrentOperand;
      node->SecondSimpleExpression->Accept(this);
      Phx::IR::Operand ^ right = CurrentOperand;
    
      if (left && right)
      {
         // Emit IR for the relational expression.

         CurrentOperand = IRBuilder::EmitBinaryRelationalOp(
            functionUnit,
            node->RelationalOperator, 
            left, 
            right,
            node->SourceLineNumber
         );
      }
      else
      {
         // Some error occurred; invalidate the current operand.

         CurrentOperand = nullptr;
      }
   }

   // Otherwise, process the single expression.

   else
   {      
      node->FirstSimpleExpression->Accept(this);
   } 
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FactorBaseNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FactorBaseNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FactorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FactorNode^ node
)
{
   // A factor takes the following form:
   // factor |
   // exponentiation

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // We use the '`' character so that the type is inaccessible
         // from the user program.

   if (node->Factor)
   {      
      node->Factor->Accept(this);

      // A factor can have an optional sign. Resolve
      // the value now.

      switch (node->Sign)
      {            
      case MINUS: // negation.
         {
            Phx::IR::Operand ^ current = CurrentOperand;

            // If the current value is an immediate, we can
            // statically resolve the negation.

            bool handled = false;
            if (current->IsImmediateOperand)
            {
               Phx::IR::ImmediateOperand ^ immediateOperand = 
                  current->AsImmediateOperand;

               if (immediateOperand->IsIntImmediate)
               {
                  immediateOperand->IntValue = -immediateOperand->IntValue;
                  handled = true;
               }
               else if(immediateOperand->IsFloatImmediate)
               {
                  immediateOperand->FloatValue = -immediateOperand->FloatValue;
                  handled = true;
               }
               else
               {
                  // Unexpected immediate type.

                  Debug::Assert(false);
               }               
            }
            
            if (! handled)
            {
               // Handle the conversion at runtime.

               CurrentOperand = IRBuilder::EmitUnaryOp(
                  functionUnit,
                  node->Sign,
                  current->Type,
                  current,
                  node->Factor->SourceLineNumber
                  );
            }
         }
         break;

      case PLUS: // identity.
      default:
         break;
      }
   }
   else
   {
      node->Exponentiation->Accept(this);
   }  
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FieldDesignatorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FieldDesignatorNode^ node
)
{
   // Grammar:
   // variable_access DOT identifier

   ++this->fieldDesignatorCount;
   
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // Process the variable access part and obtain the
   // resulting operand and access type.

   node->VariableAccess->Accept(this);
   Phx::IR::Operand ^ accessOperand = CurrentOperand;
   Phx::Types::Type ^ accessType = accessOperand->Type;

   // Generate operand for right-hand side.

   Phx::Name fieldName = Phx::Name::New(
      functionUnit->Lifetime,
      node->Identifier->Name
   );

   // Convert the access type to an aggregate type.

   Phx::Types::AggregateType ^ aggregateType;

   // If the access is a pointer type, enforce syntax rules.

   if (accessType->IsPointerType)
   {
      if (node->VariableAccess->UpArrow->Equals(String::Empty))
      {
         // The lack of a user-provided pointer dereference sybmol
         // would not impact execution of the program (because we know the
         // type from its symbol). However, we report the error to enforce 
         // syntax rules.

         Output::ReportError(
            node->VariableAccess->SourceLineNumber,
            Error::InvalidPointerAccess
         );
      }
      aggregateType = 
         accessType->AsPointerType->ReferentType->AsAggregateType;
   }
   else
   {
      aggregateType = accessType->AsAggregateType;
   }

   // Obtain the symbol table for the aggregate type.

   Phx::Symbols::Table ^ symbolTable = 
      TypeBuilder::GetSymbolTable(aggregateType);

   // Now lookup the symbol representing the field access.

   Phx::Symbols::Symbol ^ symbol = symbolTable->NameMap->Lookup(fieldName);

   // If lookup failed, this is an invalid field access.

   if (symbol == nullptr)
   {
      Output::ReportError(
         node->SourceLineNumber, 
         Error::InvalidFieldAccess,
         fieldName.NameString,
         node->VariableAccess->Name
      );
      
      CurrentOperand = nullptr;
      --this->fieldDesignatorCount;
      return;
   }
   
   // Convert the symbol to a field symbol and retrieve its type.

   Debug::Assert(symbol->IsFieldSymbol);
   Phx::Symbols::FieldSymbol ^ fieldSymbol = symbol->AsFieldSymbol;
   Phx::Types::Type ^ fieldType = fieldSymbol->Type;

   // Change the access operand to an address operand.

   if (! accessOperand->IsAddress && ! accessOperand->IsPointer)
      accessOperand->ChangeToAddress();

   // If the access operand is not a variable operand, assign
   // its value to an expression temporary (variable operand),
   // so we can load its field access.

   if (! accessOperand->IsVariableOperand)
   {
      Phx::IR::VariableOperand ^ t1 = 
         Phx::IR::VariableOperand::NewExpressionTemporary(
            functionUnit,
            functionUnit->TypeTable->GetUnmanagedPointerType(aggregateType)
         );

      Phx::IR::Instruction ^ assignInstruction = 
         Phx::IR::ValueInstruction::NewUnary(
            functionUnit,
            Phx::Common::Opcode::Assign,
            t1,
            accessOperand
         );

      functionUnit->LastInstruction->InsertBefore(assignInstruction);

      accessOperand = t1;
   }

   // Load the field from the aggregate.

   CurrentOperand = Phx::IR::MemoryOperand::New(
      functionUnit,
      fieldType,
      nullptr,
      accessOperand->AsVariableOperand,
      Phx::Utility::BitsToBytes(fieldSymbol->BitOffset),
      Phx::Alignment::NaturalAlignment(fieldType),
      functionUnit->AliasInfo->IndirectAliasedMemoryTag,
      functionUnit->SafetyInfo->SafeTag
   );
  
   --this->fieldDesignatorCount;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FileNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FileNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FileTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FileTypeNode^ node
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // Create a new file type based on the provided component type.
   // The NewTypeName property will have been set by a parent
   // of the FileTypeNode node. 

   Phx::Types::Type ^ componentType = node->ComponentType->Type;
   
   TypeBuilder::RegisterFileType(
      functionUnit->TypeTable,
      functionUnit->SymbolTable,
      NewTypeName,
      componentType,
      node->SourceLineNumber
   );   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FinalValueNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FinalValueNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FormalParameterListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FormalParameterListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FormalParameterNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FormalParameterNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FormalParameterSectionListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FormalParameterSectionListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FormalParameterSectionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FormalParameterSectionNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ForStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ForStatementNode^ node
)
{
   throw gcnew Exception("Visit() not implemented for ForStatementNode^");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FunctionalParameterSpecificationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FunctionalParameterSpecificationNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FunctionBlockNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FunctionBlockNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FunctionDeclarationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FunctionDeclarationNode^ node
)
{
   // A function declaration takes the following form:
   // function_heading semicolon directive |
   // function_identification semicolon function_block |
   // function_heading semicolon function_block

   // Obtain the identifier and formal parameter list, and
   // result type associated with the declaration.

   IdentifierNode ^ identifierNode = 
      node->FunctionHeading ? node->FunctionHeading->Identifier :
      node->FunctionIdentification->Identifier;

   FormalParameterListNode ^ formalParameterListNode =
      node->FunctionHeading ? node->FunctionHeading->FormalParameterList :
      nullptr;

   Phx::Types::Type ^ resultType = 
      node->FunctionHeading ? 
         TypeBuilder::GetTargetType(
            node->FunctionHeading->ResultType->Identifier->Name) :
         nullptr;

   // If there is a directive associated with the function
   // declaration, build the forward declaration now.

   if (node->Directive)
   {
      BuildProcedureOrFunctionDirective(
         identifierNode,
         formalParameterListNode,
         resultType,
         node->Directive,
         node->SourceLineNumber
         );
   }

   // Otherwise, build the function and its function block.

   else
   {
      Debug::Assert(node->FunctionBlock != nullptr);

      BuildProcedureOrFunctionBlock(
         identifierNode,
         formalParameterListNode,
         resultType,
         node->FunctionBlock->Block,
         node->SourceLineNumber
         );
   }   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FunctionDesignatorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FunctionDesignatorNode^ node
)
{
   // Grammar:
   // identifier params

   // Build the function call.

   BuildProcedureCall(
      node->Identifier->Name,
      node->Params,
      node->SourceLineNumber
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FunctionHeadingNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FunctionHeadingNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given FunctionIdentificationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   FunctionIdentificationNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given GotoStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   GotoStatementNode^ node
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;
   
   int sourceLineNumber = node->SourceLineNumber;

   int label = node->Label->Label;

   // Either the target label of the goto 
   //  1. has already been defined,
   //  2. has not yet been defined, but was declared (e.g. this is
   //     a forward reference), or
   //  3. the label was never declared.

   // We'll handle the last case first. For this case, report an error.
   
   if (! ModuleBuilder::IsLabelDeclared(functionUnit, label))
   {
      Output::ReportError(
         sourceLineNumber,
         Error::UndeclaredLabel,
         label
      );
      return;
   }

   // For the first case, emit the goto instruction now.
   
   Phx::IR::LabelInstruction ^ labelTarget = 
      ModuleBuilder::GetLabelInstruction(
         functionUnit,
         label,
         sourceLineNumber
      );

   if (labelTarget)
   {
      IRBuilder::EmitGoto(functionUnit, labelTarget, sourceLineNumber);

      // Mark that we've referenced this label.

      ModuleBuilder::AddLabelReference(functionUnit, label);
   }

   // Otherwise, add the reference to the current functionUnitNode's
   // label fixup list and we'll add it later.

   else
   {
      labelTarget = ModuleBuilder::GetLabelInstruction(
         functionUnit,
         -1,
         sourceLineNumber
      );

      Phx::IR::BranchInstruction ^ gotoInstruction = IRBuilder::EmitGoto(
         functionUnit, 
         labelTarget, 
         sourceLineNumber
      );

      ModuleBuilder::AddGotoLabelFixup(
         functionUnit,
         label, 
         gotoInstruction,
         sourceLineNumber
      );
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given IdentifierListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   IdentifierListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given IdentifierNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   IdentifierNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given IfStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   IfStatementNode^ node
)
{
   throw gcnew Exception("Visit() not implemented for IfStatementNode^");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given IndexedVariableNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   IndexedVariableNode^ node
)
{
   // Grammar:
   // variable_access LBRAC index_expression_list RBRAC

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // Process the variable access part.

   int errorCount = Output::ErrorCount;
   node->VariableAccess->Accept(this);   
   if (errorCount != Output::ErrorCount)
   {
      // An error occurred. Invalidate the current operand
      // and return.

      CurrentOperand = nullptr;
      return;
   }
   Phx::IR::Operand ^ operand = CurrentOperand;

   // Obtain the type associated with the access. We expect it
   // to be of array type.

   Phx::Types::Type ^ arrayType = node->Type;
   Debug::Assert(arrayType->IsArray);
      
   // Now obtain the element type of the array.

   Phx::Types::Type ^ elementType = 
      arrayType->AsUnmanagedArrayType->ElementType;
   
   // Build the index operand for the array access.

   // If the operand is a variable operand, load it into
   // a memory operand then build the array index operand.

   if (operand->IsVariableOperand)
   {
      Phx::IR::VariableOperand ^ baseOperand =
         operand->AsVariableOperand;

      if (! baseOperand->IsAddress)
         baseOperand->ChangeToAddress();

      operand = Phx::IR::MemoryOperand::New(
         functionUnit,
         elementType,
         nullptr,
         baseOperand,
         0,
         Phx::Alignment::NaturalAlignment(elementType),
         functionUnit->AliasInfo->IndirectAliasedMemoryTag,
         functionUnit->SafetyInfo->SafeTag
      );
         
      Phx::IR::VariableOperand ^ indexOperand = 
         BuildArrayIndexOperand(
            arrayType->AsUnmanagedArrayType,
            node
         );      

      operand->IndexOperand = indexOperand;
      CurrentOperand = operand;
   }

   // Otherwise, if the operand is already a memory operand,
   // simply build its index operand.

   else if (operand->IsMemoryOperand)
   {
      operand->AsMemoryOperand->IndexOperand = 
         BuildArrayIndexOperand(
            arrayType->AsUnmanagedArrayType,
            node
         );
      operand->Type = elementType;

      CurrentOperand = operand;      
   }

   else
   {
      // Unexpected operand type.

      Debug::Assert(false);
   }  
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given IndexExpressionListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   IndexExpressionListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given IndexExpressionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   IndexExpressionNode^ node
)
{      
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given IndexListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   IndexListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given IndexTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   IndexTypeNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given InitialValueNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   InitialValueNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given LabelDeclarationPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   LabelDeclarationPartNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given LabelListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   LabelListNode^ node
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // Add each label declaration to the label declaration table
   // for the current functionUnitNode.

   for each (LabelNode ^ labelNode in node->Labels)
   {
      if (! ModuleBuilder::AddLabelDeclaration(functionUnit, labelNode->Label))
      {
         Output::ReportError(
            node->SourceLineNumber,
            Error::LabelRedeclaration, 
            labelNode->Label
         );
      }
      
      // Report an error if the label string length is greater than 4 digits.

      if (labelNode->Label.ToString()->Length > 4)
      {
         Output::ReportError(
            node->SourceLineNumber,
            Error::InvalidLabelLength,
            labelNode->Label
         );
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given LabelNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   LabelNode^ node
)
{   
   return;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given MemberDesignatorListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   MemberDesignatorListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given MemberDesignatorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   MemberDesignatorNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ModuleNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ModuleNode^ node
)
{
   // Create a ModuleUnit object for the current source file.

   this->moduleUnit = ModuleBuilder::CreateModuleUnit(this->sourceFileName);

   if (this->moduleUnit == nullptr)
   {
      // An error occurred; halt processing.

      return;
   }

   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given NewOrdinalTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   NewOrdinalTypeNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given NewPointerTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   NewPointerTypeNode^ node
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   int sourceLineNumber = node->SourceLineNumber;

   // Get the name of domain type.

   String ^ domainTypeName = node->DomainType->Identifier->Name;

   // Lookup the type symbol associated with the domain type.

   Phx::Symbols::TypeSymbol ^ domainTypeSymbol = 
      ModuleBuilder::LookupTypeSymbol(functionUnit, domainTypeName);

   // If the domain type has already been defined, we can 
   // register its pointer type now.

   if (domainTypeSymbol)
   {
      TypeBuilder::RegisterPointerType(
         functionUnit, NewTypeName, domainTypeSymbol->Type,
         sourceLineNumber);
   }

   // Otherwise, the domain type has not yet been defined. This
   // is a forward-declaration to a pointer type. For example,
   //   type : link = ^person;
	//          person = record
	//             next : link;
   //          end;	
   // Create a temporary type for now, and we'll fix it up at
   // the end of the type definition part.

   else
   {
      // Register the type as 'pointer to void' for now and 
      // we'll fixup the underlying type later.

      Phx::Types::PointerType ^ pointerType = 
         TypeBuilder::RegisterPointerType(
            functionUnit, NewTypeName,
            functionUnit->TypeTable->VoidType,
            sourceLineNumber);

      // Ensure the pointer type has not already been defined.

      if (this->pointerTypeFixups->ContainsKey(NewTypeName))
      {
         Output::ReportFatalError(
            sourceLineNumber, Error::SymbolRedeclaration,
            NewTypeName, String::Concat(
               "pointer to ", this->pointerTypeFixups[NewTypeName]));
         return;
      }

      // Add the type to the fixup map.

      this->pointerTypeFixups[NewTypeName] = domainTypeName;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given NewStructuredTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   NewStructuredTypeNode^ node
)
{
   // Note that we do nothing with the Packed specification.

   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given NewTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   NewTypeNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given NilNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   NilNode^ node
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // Create a new immediate operand of type 'null pointer'
   // with the value 0.

   CurrentOperand = Phx::IR::ImmediateOperand::New(
      functionUnit,
      functionUnit->TypeTable->NullPointerType,
      (int) 0
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given NonLabeledClosedStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   NonLabeledClosedStatementNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given NonLabeledOpenStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   NonLabeledOpenStatementNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given NonStringNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   NonStringNode^ node
)
{
   // Delegate processing to the child nodes. The non-string
   // reference is either symbol or a literal numeic constant.

   if (node->Identifier)
      node->Identifier->Accept(this);
   else
      VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given OpenForStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   OpenForStatementNode^ node
)
{
   BuildForLoop(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given OpenIfStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   OpenIfStatementNode^ node
)
{
   // Grammar:
   // IF boolean_expression THEN statement
   // IF boolean_expression THEN closed_statement ELSE open_statement
  
   // Get the current function unit.

   Phx::FunctionUnit^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   int sourceLineNumber = node->SourceLineNumber;

   // Evaluate the Boolean expression branch of the AST. 
   // This evaluation will reduce any compound Boolean expressions 
   // into a single IR operand.

   int errorCount = Output::ErrorCount;
   node->BooleanExpression->Accept(this);
   if (errorCount != Output::ErrorCount)
   {
      return;
   }

   // Generate label target instructions for the true 
   // and false arms of the if statement.

   Phx::IR::LabelInstruction ^ trueLabelInstruction  =
      Phx::IR::LabelInstruction::New(functionUnit);

   Phx::IR::LabelInstruction ^ falseLabelInstruction =
      Phx::IR::LabelInstruction::New(functionUnit);

   // Because Phoenix control flow is explicit, generate a 
   // shared target label to which both true and false arms 
   // will jump after execution.

   Phx::IR::LabelInstruction ^ endifLabelInstruction =
      falseLabelInstruction;

   // Append a conditional branch instruction to 
   // the IR instruction stream.

   IRBuilder::EmitConditionalBranch(
      functionUnit,
      CurrentOperand,
      trueLabelInstruction, 
      falseLabelInstruction
   );

   // Construct the 'true' arm of the conditional statement. 
   // This process includes three parts:
   // 1. A target label for the branch.
   // 2. Evaluation of the 'true' statement part. This is 
   //    part of the AST.
   // 3. An unconditional branch to the end of the if statement.

   IRBuilder::EmitLabel(functionUnit, trueLabelInstruction);

   if (node->Statement)
      node->Statement->Accept(this);
   else
      node->ClosedStatement->Accept(this);

   if (node->OpenStatement)
   {
      endifLabelInstruction = Phx::IR::LabelInstruction::New(functionUnit);

      IRBuilder::EmitGoto(functionUnit, endifLabelInstruction,
         sourceLineNumber);
      IRBuilder::EmitLabel(functionUnit, falseLabelInstruction);

      node->OpenStatement->Accept(this);
   }

   // Now emit the final unconditional jump and jump label that 
   // exits the conditional statement.

   IRBuilder::EmitGoto(functionUnit, endifLabelInstruction, sourceLineNumber);
   IRBuilder::EmitLabel(functionUnit, endifLabelInstruction);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given OpenStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   OpenStatementNode^ node
)
{  
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // If the statement starts with a label, build it now.

   if (node->Label)
   {
      ModuleBuilder::BuildLabel(
         functionUnit,
         node->Label->Label,
         node->SourceLineNumber
      );
   }

   // Process the open statement part.

   node->NonLabeledOpenStatement->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given OpenWhileStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   OpenWhileStatementNode^ node
)
{
   BuildWhileLoop(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given OpenWithStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   OpenWithStatementNode^ node
)
{
   BuildWithStatement(node, node->OpenStatement);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given OrdinalTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   OrdinalTypeNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given OtherwisePartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   OtherwisePartNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ParamsNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ParamsNode^ node
)
{
   node->ActualParameterList->Accept(this);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given PrimaryBaseNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   PrimaryBaseNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given PrimaryNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   PrimaryNode^ node
)
{
   // A primary takes the following form:
   // variable_access		                                 
   // unsigned_constant
   // function_designator
   // set_constructor
   // expression
   // NOT primary

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // Delegate processing to the child node. The Primary
   // case requires additional processing.

   if (node->VariableAccess)
      node->VariableAccess->Accept(this);

   else if (node->UnsignedConstant)
      node->UnsignedConstant->Accept(this);

   else if (node->FunctionDesignator)
      node->FunctionDesignator->Accept(this);

   else if (node->SetConstructor)
      node->SetConstructor->Accept(this);

   else if (node->Expression)
      node->Expression->Accept(this);

   else if (node->Primary)
   {      
      node->Primary->Accept(this);

      // If no errors occurred while processing the Primary
      // part, emit a unary NOT instruction now.

      if (CurrentOperand != nullptr)
      {
         Phx::IR::Operand ^ operand = CurrentOperand;

         CurrentOperand = IRBuilder::EmitUnaryOp(
            functionUnit,
            NOT, 
            operand->Type, 
            operand,
            node->SourceLineNumber
         );
      }
   }
   else
   {
      // Should never get here.

      __assume(0);
      Debug::Assert(false);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ProceduralParameterSpecificationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ProceduralParameterSpecificationNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ProcedureAndFunctionDeclarationPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ProcedureAndFunctionDeclarationPartNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ProcedureBlockNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ProcedureBlockNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ProcedureDeclarationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ProcedureDeclarationNode^ node
)
{
   // If there is a directive associated with the procedure
   // declaration, build the forward declaration now.

   if (node->Directive)
   {
      BuildProcedureOrFunctionDirective(
         node->ProcedureHeading->ProcedureIdentification->Identifier,
         node->ProcedureHeading->FormalParameterList,
         Phx::GlobalData::TypeTable->VoidType,
         node->Directive,
         node->SourceLineNumber
         );
   }
   
   // Otherwise, build the procedure and its procedure block.

   else
   {
      Debug::Assert(node->ProcedureBlock != nullptr);

      BuildProcedureOrFunctionBlock(
         node->ProcedureHeading->ProcedureIdentification->Identifier,
         node->ProcedureHeading->FormalParameterList,
         Phx::GlobalData::TypeTable->VoidType, // all procedures return 'void'
         node->ProcedureBlock->Block,
         node->SourceLineNumber
         );
   }   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ProcedureHeadingNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ProcedureHeadingNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ProcedureIdentificationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ProcedureIdentificationNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ProcedureOrFunctionDeclarationListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ProcedureOrFunctionDeclarationListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ProcedureOrFunctionDeclarationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ProcedureOrFunctionDeclarationNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ProcedureStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ProcedureStatementNode^ node
)
{
   // Grammar:
   // identifier params

   // Build the procedure call.

   BuildProcedureCall(
      node->Identifier->Name,
      node->Params,
      node->SourceLineNumber
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ProgramHeadingNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ProgramHeadingNode^ node
)
{   
   // Grammar:
   // PROGRAM identifier |
   // PROGRAM identifier LPAREN identifier_list RPAREN

   // The identifier list to a program is the list of all
   // persistent files that will be used. The identifier
   // list may also contain forward declarations to the 
   // standard input and output files.
   // The identifier list is optional. If one was provided,
   // add their names to the module builder's program
   // parameter list.

   if (node->IdentifierList)
   {
      for each (FormalParameterNode ^ parameterNode in 
         node->IdentifierList->FormalParameters)
      {
         ModuleBuilder::AddProgramParameter(
            parameterNode->Identifier->Name,
            parameterNode->Identifier->SourceLineNumber
         );
      }
   }
   return;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ProgramNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ProgramNode^ node
)
{
   // Establish the program name for the executable.

   Configuration::ProgramName = 
      node->ProgramHeading->Identifier->Name;

   // The compilation unit is a program and not a module.

   Pascal::Configuration::IsProgram = true;

   // Create a ModuleUnit object for the current source file.

   this->moduleUnit = ModuleBuilder::CreateModuleUnit(this->sourceFileName);

   if (this->moduleUnit == nullptr)
   {
      // An error occurred; halt processing.

      return;
   }
  
   // Build program entry point to call the main user function.
   
   ModuleBuilder::MainFunctionUnit = 
      ModuleBuilder::Runtime->BuildEntryFunction(
         this->moduleUnit,
         node->SourceLineNumber
      );

   // Build the beginning part of the function. We will then
   // fill-in the body of the function before we build the
   // end part of the function.

   Phx::FunctionUnit ^ functionUnit = ModuleBuilder::BuildBeginFunction(
      this->moduleUnit,
      node->ProgramHeading->Identifier->Name,
      Phx::Types::CallingConventionKind::CDecl,
      Phx::Symbols::Visibility::GlobalDefinition,
      nullptr,
      Phx::GlobalData::TypeTable->VoidType,
      DirectiveType::None,
      node->ProgramHeading->SourceLineNumber
   );
  
   // Call the runtime_init and main user function directly 
   // before the return instruction. The sequence looks like:
   // runtime_init()
   // user_main_function()
   // runtime_exit()
   
   // Find the return instruction from the main function and insert
   // the calls directly before it.

   Phx::IR::Instruction ^ returnInstruction = 
      ModuleBuilder::MainFunctionUnit->LastInstruction;
   while (!returnInstruction->IsReturn)
      returnInstruction = returnInstruction->Previous;
   
   Phx::IR::Instruction ^ callInstruction;
   Phx::IR::Instruction ^ insertAfterInstruction;

   // Generate call to the runtime_init runtime function to initialize the
   // Pascal runtime.

   callInstruction = ModuleBuilder::Runtime->CallUtilityFunction(
      ModuleBuilder::MainFunctionUnit,
      "runtime_init",
      gcnew List<Phx::IR::Operand ^ >(),
      returnInstruction->Previous,    // insert before return statement
      node->SourceLineNumber      
   );
   insertAfterInstruction = callInstruction;

   // Generate call into user code.
   
   callInstruction = Phx::IR::CallInstruction::New(
      ModuleBuilder::MainFunctionUnit,
      Phx::Common::Opcode::Call,
      functionUnit->FunctionSymbol
   );

   insertAfterInstruction->InsertAfter(callInstruction);
   insertAfterInstruction = callInstruction;
   
   // Call the runtime_exit runtime function to clean-up any allocated memory.

   callInstruction = ModuleBuilder::Runtime->CallUtilityFunction(
      ModuleBuilder::MainFunctionUnit,
      "runtime_exit",
      gcnew List<Phx::IR::Operand ^ >(),
      insertAfterInstruction,
      node->SourceLineNumber      
   );

   // Visit the body of the program (the 'program' body is the main
   // user function body.
   
   VisitChildren(node);

   // Now call BuildEndFunction to build the end part of the main user
   // function.

   ModuleBuilder::BuildEndFunction(functionUnit, node->SourceLineNumber);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given RecordSectionListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   RecordSectionListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given RecordSectionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   RecordSectionNode^ node
)
{
   return;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given RecordTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   RecordTypeNode^ node
)
{
   // Grammar:
   // RECORD record_section_list END
   // RECORD record_section_list semicolon variant_part END
   // RECORD variant_part END

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // Before we can create an AggregateType object to hold 
   // the record type, we need to precalculate its overall size.

   unsigned int bitSize = 0;

   // First visit the record section list part.

   if (node->RecordSectionList)
   {
      bitSize += CalculateBitSize(node->RecordSectionList);
   }

   // Now visit the variant part.

   // We create any variant parts as an overlapping 
   // union of data members. In addition, no static or 
   // run-time checking is performed to ensure that only the 
   // data associated with the current variant selector is
   // accessed.

   if (node->VariantPart)
   {
      bitSize += CalculateBitSize(node->VariantPart);
   }

   // Disallow zero-sized (empty) records

   if (bitSize == 0)
   {
      Output::ReportError(node->SourceLineNumber, 
         Error::ZeroSizeRecord);

      // Continue for the sake of completeness. 
      // The error will prevent the compiler from 
      // generating code.
   }

   // Create a type symbol and type object for the 
   // record type.
   
   Phx::Symbols::TypeSymbol ^ typeSymbol = 
      Phx::Symbols::TypeSymbol::New(
         functionUnit->SymbolTable, 0,
         Phx::Name::New(functionUnit->Lifetime, NewTypeName));

   Phx::Types::AggregateType ^ aggregateType = 
      Phx::Types::AggregateType::New(
         functionUnit->TypeTable, bitSize, typeSymbol);

   // Register the new aggregate type with the type builder.

   Phx::Symbols::Table ^ symbolTable = 
      TypeBuilder::RegisterAggregateType(aggregateType);

   ModuleBuilder::PushScope(symbolTable);

   // Set metaproperties for type.
   // In Pascal, structured types will always contain plain data.

   aggregateType->IsOnlyData = true;
   aggregateType->IsSealed = true;
   aggregateType->HasCompleteInheritanceInfo = true;
   aggregateType->HasCompleteOverrideInfo = true;

   // Add fields to type.
   
   // First visit the record section list part.

   if (node->RecordSectionList)
   {
      AddFieldsToType(aggregateType, node->RecordSectionList);
   }

   // Now visit variant_part.

   if (node->VariantPart)
   {
      AddFieldsToType(aggregateType, node->VariantPart);
   }
   
   // We've filled in all members for the aggregate.

   aggregateType->HasCompleteMemberInfo = true;

   // Pop the symbol table from the symbol stack.

   ModuleBuilder::PopScope();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given RecordVariableListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   RecordVariableListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given RepeatStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   RepeatStatementNode^ node
)
{
   // Grammar:
   // REPEAT statement_sequence UNTIL boolean_expression
  
   // Get the current function unit.

   Phx::FunctionUnit^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;   
   
   int sourceLineNumber = node->SourceLineNumber;

   // Create the entry-point of the loop.

   Phx::IR::LabelInstruction ^ loopBodyLabelInstruction = 
      Phx::IR::LabelInstruction::New(functionUnit);

   // Jump to loop body.

   IRBuilder::EmitGoto(
      functionUnit, 
      loopBodyLabelInstruction, 
      sourceLineNumber
      );

   IRBuilder::EmitLabel(
      functionUnit, 
      loopBodyLabelInstruction
   );

   // Process the loop body.
   
   node->StatementSequence->Accept(this);

   // Process the conditional statement.

   int errorCount = Output::ErrorCount;
   node->BooleanExpression->Accept(this);
   if (errorCount != Output::ErrorCount)
   {
      // An error occurred; return.

      return;
   }

   // Emit conditional branch to continue processessing or
   // exit the loop.

   Phx::IR::LabelInstruction ^ exitLoopLabelInstruction = 
      Phx::IR::LabelInstruction::New(functionUnit);

   IRBuilder::EmitConditionalBranch(
      functionUnit,
      CurrentOperand,
      exitLoopLabelInstruction, // Exit body if condition is true.
      loopBodyLabelInstruction  // Repeat body if condition is false.     
   );

   // Emit the loop exit label.
   
   IRBuilder::EmitLabel(functionUnit, exitLoopLabelInstruction);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ResultTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ResultTypeNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given SetConstructorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   SetConstructorNode^ node
)
{
   // Grammar:
   // LBRAC member_designator_list RBRAC |
   // LBRAC RBRAC

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;
   
   // This list collects a list of operands that specify the initial
   // value ranges.

   List<Phx::IR::Operand ^> ^ memberRanges = gcnew List<Phx::IR::Operand ^>();
   
   // We will collect the base (member) and set (program name) types
   // for the set constructor.

   Phx::Types::Type ^ baseType = nullptr;
   Phx::Types::Type ^ setType = nullptr;
      TypeBuilder::GetTargetType(NativeType::Epsilon);

   // If a member list was provided, obtain the base and set
   // types.

   if (node->MemberDesignatorList)
   {
      int errorCount = Output::ErrorCount;
      
      baseType = node->MemberDesignatorList->Type;
      
      if (baseType->Equals(functionUnit->TypeTable->UnknownType))
      {
         Output::ReportError(
            node->SourceLineNumber,
            Error::UnknownSetBaseType
         );
      }

      if (errorCount != Output::ErrorCount)
      {
         // A type mismatch occurred or the base type could not
         // be established; return.

         return;
      }

      setType = TypeBuilder::RegisterSetType(
         functionUnit,
         TypeBuilder::GetUniqueTypeName(),
         baseType
      );
   }

   // Otherwise, this is the empty set.

   else
   {
      setType = TypeBuilder::GetTargetType(NativeType::Epsilon);
   }

   // Obtain the upper and lower bounds of the set.

   Phx::IR::Operand ^ lowerValueOperand;
   Phx::IR::Operand ^ upperValueOperand;

   if (node->MemberDesignatorList)
   {
      // The runtime expects 32-bit integer values for the bounds.
      // Create a Resolve object to widen the operands if they
      // are smaller than 32 bits.

      Phx::Types::Resolve ^ resolve = nullptr;
      if (baseType->ByteSize < functionUnit->TypeTable->Int32Type->ByteSize)
      {
         resolve = ModuleBuilder::Resolve;
      }

      // This list collects all sub-ranges of values
      // in the set.

      List<Phx::IR::Operand ^> ^ rangeValueOperands = 
         gcnew List<Phx::IR::Operand ^>();

      // Collect the ranges.

      for each (MemberDesignatorNode ^ designatorNode in 
         node->MemberDesignatorList->MemberDesignators)
      {         
         // Each member designator can have one or two parts.

         array<ExpressionNode ^> ^ expressionNodes = 
            { nullptr, nullptr };

         // The first part is optional.

         if (designatorNode->FirstExpression)
         {
            expressionNodes[0] = designatorNode->FirstExpression;
         }
         expressionNodes[1] = designatorNode->SecondExpression;
         
         // If we have both expressoins, process both nodes and 
         // add the two results to the range list.

         if (expressionNodes[0])
         {
            expressionNodes[0]->Accept(this);

            // Apply resolver if the members are less
            // than 32 bits.

            if (resolve)
               CurrentOperand = resolve->Convert(
                  CurrentOperand, 
                  functionUnit->LastInstruction->Previous,
                  functionUnit->TypeTable->Int32Type
               );

            memberRanges->Add(CurrentOperand);
            rangeValueOperands->Add(CurrentOperand);
            
            expressionNodes[1]->Accept(this);

            // Apply resolver if the members are less
            // than 32 bits.

            if (resolve)
               CurrentOperand = resolve->Convert(
                  CurrentOperand, 
                  functionUnit->LastInstruction->Previous,
                  functionUnit->TypeTable->Int32Type
               );

            memberRanges->Add(CurrentOperand);
            rangeValueOperands->Add(CurrentOperand);
         }

         // Otherwise, process the single expression and 
         // add its result twice to generate a pair.

         else
         {
            expressionNodes[1]->Accept(this);

            // Apply resolver if the members are less
            // than 32 bits.

            if (resolve)
               CurrentOperand = resolve->Convert(
                  CurrentOperand, 
                  functionUnit->LastInstruction->Previous,
                  functionUnit->TypeTable->Int32Type
               );

            rangeValueOperands->Add(CurrentOperand);

            memberRanges->Add(CurrentOperand);
            memberRanges->Add(CurrentOperand);
         }
      }

      // Call the runtime get_lower_bound and get_upper_bound
      // functions to get the low and high values of the set.

      // Insert the number of operands as the first argument
      // to the function call.

      rangeValueOperands->Insert(0, Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) rangeValueOperands->Count
         )
      );

      lowerValueOperand = ModuleBuilder::Runtime->CallUtilityFunction(
         functionUnit,
         "get_lower_bound",
         rangeValueOperands,
         functionUnit->LastInstruction->Previous,
         node->SourceLineNumber
      )->DestinationOperand;

      upperValueOperand = ModuleBuilder::Runtime->CallUtilityFunction(
         functionUnit,
         "get_upper_bound",
         rangeValueOperands,
         functionUnit->LastInstruction->Previous,
         node->SourceLineNumber
      )->DestinationOperand;
   }

   // This is the empty set. Initialize the upper and lower 
   // bounds to 0.

   else
   {
      upperValueOperand = lowerValueOperand = Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) 0
      );
   } 

   // Now call new_set to construct a new set.

   List<Phx::IR::Operand ^> ^ arguments = gcnew List<Phx::IR::Operand ^>();

   //lower
   arguments->Add(lowerValueOperand);

   //upper
   arguments->Add(upperValueOperand);

   //fileindex
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) ModuleBuilder::SourceFileIndex
      )
   );

   //sourceLineNumber   
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) node->SourceLineNumber
      )
   );

   Phx::IR::Instruction ^ callInstruction = 
      ModuleBuilder::Runtime->CallSetFunction(
         functionUnit,
         "new_set",
         arguments,
         node->SourceLineNumber
      );
   
   // Assign the new set to a temporary operand
   // so we can submit it for set release when the function exits.

   Phx::Symbols::Symbol ^ setSymbol = 
      ModuleBuilder::AddInternalVariableDeclarationSymbol(
         functionUnit,
         setType,
         Phx::Symbols::StorageClass::Auto,
         node->SourceLineNumber
      );

   Phx::IR::Operand ^ destinationOperand = Phx::IR::VariableOperand::New(
      functionUnit,
      setSymbol->Type,
      setSymbol
   );

   Phx::IR::Operand ^ sourceOperand = callInstruction->DestinationOperand;
   if (! setSymbol->Type->Equals(functionUnit->TypeTable->UnknownType))
   {
      Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

      sourceOperand = resolve->Convert(
         callInstruction->DestinationOperand,
         functionUnit->LastInstruction->Previous,
         setSymbol->Type
      );
   }
   Debug::Assert(!sourceOperand->Type->Equals(
      functionUnit->TypeTable->UnknownType));

   Phx::IR::Instruction ^ assignInstruction = 
      Phx::IR::ValueInstruction::NewUnary(
         functionUnit,
         Phx::Common::Opcode::Assign,
         destinationOperand,
         sourceOperand
      );
   
   functionUnit->LastInstruction->InsertBefore(assignInstruction);

   CurrentOperand = destinationOperand;
   
   // Add the set symbol to the list of set symbols mapped
   // to the current function unit.

   ModuleBuilder::AddSetDefinitionSymbol(
      functionUnit,
      setSymbol,
      setType
   );

   // Now mark the created set for release after the function exits.

   ModuleBuilder::AddSetSymbolToReleaseList(
      functionUnit,
      setSymbol
      );
   
   // Now call set_set_values to fill the set with values.

   arguments->Clear();

   //fileindex
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) ModuleBuilder::SourceFileIndex
      )
   );

   //sourceLineNumber   
   arguments->Add(Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) node->SourceLineNumber
      )
   );

   //set
   arguments->Add(CurrentOperand);

   //type_width
   if (baseType)
   {
      arguments->Add(Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) baseType->ByteSize
         )
      );
   }
   else
   {
      arguments->Add(Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) 0
         )
      );
   }

   //range_count
   if (node->MemberDesignatorList)
   {
      arguments->Add(Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) node->MemberDesignatorList->ChildList->Count
         )
      );
   }
   else
   {
      arguments->Add(Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) 0
         )
      );
   }   

   //ellipsis
   arguments->AddRange(memberRanges);

   // Call the new_set runtime function

   callInstruction = ModuleBuilder::Runtime->CallSetFunction(
      functionUnit,
      "set_set_values",
      arguments,
      node->SourceLineNumber
      );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given SetTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   SetTypeNode^ node
)
{
   // Grammar:
   // SET OF base_type

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;
   
   int sourceLineNumber = node->SourceLineNumber;

   int errorCount = Output::ErrorCount;
      
   // Get the base (member) type for the new set type.

   Phx::Types::Type ^ baseType = nullptr;

   // If a named identifier was provided for the set, 
   // establish the base type.
   
   OrdinalTypeNode ^ ordinalTypeNode = node->BaseType->OrdinalType;
   if (ordinalTypeNode->Identifier)
   {
      baseType = ordinalTypeNode->Identifier->Type;        
   }

   // Otherwise, the user is building a new (anonymous)
   // enumerated or subrange type.

   else
   {  
      // Save the current type name.

      String ^ currentName = NewTypeName;

      // Append a string to make the type name unique. 
      // We use the '`' character so that the type is inaccessible
      // from the user program.

      NewTypeName += "`1";

      // Process the subrange or enumerated type.
      // This will generate a new type.

      NewOrdinalTypeNode ^ newOrdinalTypeNode = ordinalTypeNode->NewOrdinalType;
      if (newOrdinalTypeNode->SubrangeType)
      {
         newOrdinalTypeNode->SubrangeType->Accept(this);
      }
      else
      {
         newOrdinalTypeNode->EnumeratedType->Accept(this);
      }

      // Retrieve the newly created type.

      baseType = TypeBuilder::GetTargetType(NewTypeName);
      
      // Set the new type name back to its original value.

      NewTypeName = currentName;
   }

   // Ensure that the user provided a valid (ordinal or subrange)
   // set base type. 
  
   if (! TypeBuilder::IsValidSetBaseType(baseType))
   {
      Output::ReportError(
         ordinalTypeNode->SourceLineNumber,
         Error::InvalidSetType, 
         ordinalTypeNode->First->Name
      );
   }
   
   if (errorCount != Output::ErrorCount)
   {
      // Return now if any errors occurred.

      return;
   }

   // Register the set type with the type system.
   // Although the base "set" type is predefined as 
   // an opaque pointer type, we will register it here 
   // as a new type for type-checking purposes.

   TypeBuilder::RegisterSetType(
      functionUnit,
      NewTypeName,
      baseType
   );   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given SimpleExpressionBaseNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   SimpleExpressionBaseNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given SimpleExpressionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   SimpleExpressionNode^ node
)
{
   // A simple_expression takes the following form:
   // term |
   // simple_expression addop term

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // If both parts were provided, process each part and then
   // reduce them to a single operand by applying the 
   // provided arithmetic operator.

   if (node->SecondTerm)
   {      
      node->SimpleExpression->Accept(this);
      Phx::IR::Operand ^ left = CurrentOperand;
      node->SecondTerm->Accept(this);
      Phx::IR::Operand ^ right = CurrentOperand;

      CurrentOperand = IRBuilder::EmitBinaryArithmeticOp(
         functionUnit,
         node->AddOperator, 
         node->Type, 
         left, 
         right,
         node->SourceLineNumber
      );      
   }
   
   // Otherwise, just process the term part.

   else
   {      
      node->FirstTerm->Accept(this);
   }  
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given StatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   StatementNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given StatementPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   StatementPartNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given StatementSequenceNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   StatementSequenceNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given StringNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   StringNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given StructuredTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   StructuredTypeNode^ node
)
{	
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given SubrangeTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   SubrangeTypeNode^ node
)
{
   // Grammar:
   // constant DOTDOT constant

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   int sourceLineNumber = node->SourceLineNumber;

   // Ensure that the lower and upper bounds are of Ordinal type.

   Phx::Types::Type ^ lowerBoundType = node->FirstConstant->Type;
   Phx::Types::Type ^ upperBoundType = node->SecondConstant->Type;

   int errorCount = Output::ErrorCount;

   if (! TypeBuilder::IsOrdinalType(lowerBoundType))
   {
      Output::ReportError(
         node->FirstConstant->SourceLineNumber,
         Error::GeneralTypeExpected,
         "ordinal"
      );
   }
   if (! TypeBuilder::IsOrdinalType(upperBoundType))
   {
      Output::ReportError(
         node->SecondConstant->SourceLineNumber,
         Error::GeneralTypeExpected,
         "ordinal"
      );
   }

   if (errorCount != Output::ErrorCount)
   {
      return;
   }

   // Verify lower and upper bounds are of the same type.

   if (! lowerBoundType->Equals(upperBoundType))
   {
      Output::ReportError(
         sourceLineNumber,
         Error::MismatchedSubrangeLimit,
         TypeBuilder::GetSourceType(lowerBoundType),
         TypeBuilder::GetSourceType(upperBoundType)
      );
      return;
   }

   // Obtain the ordinal value for the lower bound.

   bool error = false;
   int lowerValue = GetOrdinalValue(
      functionUnit, node->FirstConstant, error);

   // We already confirmed that this is an ordinal type.
   Debug::Assert(!error);  

   int upperValue = GetOrdinalValue(
      functionUnit, node->SecondConstant, error);

   // We already confirmed that this is an ordinal type.
   Debug::Assert(!error);

   // Ensure that the lower bound is less than the upper bound.

   if (lowerValue >= upperValue)
   {
      Output::ReportError(
         sourceLineNumber, Error::InvalidSubrange,
         lowerValue.ToString(), upperValue.ToString()
      );
      return;
   }

   // Create a ValueRange object to store the range.

   ValueRange ^ range = gcnew ValueRange(lowerValue, upperValue);

   // Create a new type object to represent the subrange type.

   Phx::Types::Type ^ baseType = lowerBoundType;

   Phx::Symbols::TypeSymbol ^ typeSymbol = 
      Phx::Symbols::TypeSymbol::New(
         functionUnit->SymbolTable, 0,
         Phx::Name::New(functionUnit->Lifetime, NewTypeName));

   // Use the NewRenamed method to create a type alias for the 
   // base type, but with a different type symbol.

   Phx::Types::Type ^ subrangeType = 
      baseType->NewRenamed(typeSymbol);

   // Register the new type.

   TypeBuilder::RegisterSubrangeType(
      subrangeType, range, node->SourceLineNumber);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given TagFieldNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   TagFieldNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given TagTypeNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   TagTypeNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given TermBaseNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   TermBaseNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given TermNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   TermNode^ node
)
{
   // A term takes the following form:
   // factor |
   // term mulop factor

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // If both parts were provided, process each part and then
   // reduce them to a single operand by applying the 
   // provided arithmetic operator.

   if (node->SecondFactor)
   {      
      node->Term->Accept(this);
      Phx::IR::Operand ^ left = CurrentOperand;
      node->SecondFactor->Accept(this);
      Phx::IR::Operand ^ right = CurrentOperand;

      CurrentOperand = IRBuilder::EmitBinaryArithmeticOp(
         functionUnit,
         node->MulOperator, 
         node->Type, 
         left, 
         right,
         node->SourceLineNumber);
   }

   // Otherwise, just process the factor part.

   else
   {
      node->FirstFactor->Accept(this);
   }  
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given TypeDefinitionListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   TypeDefinitionListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given TypeDefinitionNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   TypeDefinitionNode^ node
)
{
   // Begin a new type definition.
   
   NewTypeName = node->Identifier->Name;

   // Visit child nodes to define the actual type.
   
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given TypeDefinitionPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   TypeDefinitionPartNode^ node
)
{
   // First visit the body of the type definition part.

   VisitChildren(node);

   // Now fixup any forward-declared pointer types.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;
   if (functionUnit != nullptr)
   {
      FixupPointerTypes(functionUnit, node->SourceLineNumber);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given TypeDenoterNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   TypeDenoterNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given UnsignedConstantNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   UnsignedConstantNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given UnsignedIntegerNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   UnsignedIntegerNode^ node
)
{
   // Generate an operand that contains the 
   // immediate integer value.

   CurrentOperand = Phx::IR::ImmediateOperand::New(
      ModuleBuilder::CurrentFunctionUnit,
      TypeBuilder::GetTargetType(NativeType::Integer),
      node->Value
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given UnsignedNumberNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   UnsignedNumberNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given UnsignedRealNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   UnsignedRealNode^ node
)
{
   // Generate an operand that contains the 
   // immediate real value.

   CurrentOperand = Phx::IR::ImmediateOperand::New(
      ModuleBuilder::CurrentFunctionUnit,
      TypeBuilder::GetTargetType(NativeType::Real),
      node->Value
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given ValueParameterSpecificationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   ValueParameterSpecificationNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given VariableAccessNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   VariableAccessNode^ node
)
{   
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // Resolve the variable access by generating an operand.

   // If the access is symbolic, retrieve its symbol and 
   // generate an operand.

   if (node->Identifier)
   {
      Phx::Symbols::Symbol ^ variableSymbol = 
         ModuleBuilder::LookupSymbol(
            functionUnit,
            node->Identifier->Name
         );
      
      // The left-hand side of the assignment statement must
      // be declared in the var section of the procedure 
      // (or a parent procedure).

      if (variableSymbol == nullptr)
      {
         Output::ReportError(
            node->SourceLineNumber, 
            Error::UndeclaredIdentifier, 
            node->Identifier->Name
         );
         
         CurrentOperand = nullptr;
         return;
      }

      Phx::IR::Operand ^ operand = nullptr;

      // For nested procedures, check whether the symbol 
      // was defined in a parent function unit. 

      if (variableSymbol->Table->Unit &&
          variableSymbol->Table->Unit->IsFunctionUnit &&
          variableSymbol->Table->Unit != functionUnit)
      {
         operand = ModuleBuilder::GetNonLocalSymbolAddress(
            functionUnit,
            variableSymbol,
            node->SourceLineNumber
         );

         // Because the operand is likely not to have
         // an associated symbol, remember the non local
         // symbol so we can perform checking for things like
         // 'assignment to const'

         this->nonLocalSymbol = variableSymbol;
      }      

      // If we encountered a field symbol, we are in a with 
      // statement. In this case, expand the symbol to a
      // full operand.

      else if (variableSymbol->IsFieldSymbol)
      {
         Debug::Assert(WithNestingCount > 0);

         operand = ExpandWithSymbol(
            functionUnit,
            variableSymbol->AsFieldSymbol,
            node->SourceLineNumber
         );
      }

      // Although we don't create true constant symbols, 
      // this is provided for completeness.

      else if (variableSymbol->IsConstantSymbol)
      {
         operand = Phx::IR::ImmediateOperand::New(
            functionUnit,
            variableSymbol->AsConstantSymbol->Type,
            variableSymbol->AsConstantSymbol
         );
      }   

      // Otherwise, this is a plain-vanilla variable access.

      else
      {
         operand = IRBuilder::CreateVariableOperand(
            functionUnit,
            variableSymbol
         );
      }
      
      // If the symbol refers to a parameter that is passed
      // by reference, load it from its address.

      if (ModuleBuilder::IsVariableParameterSymbol(variableSymbol))
      {
         Phx::Types::Type ^ referentType = 
            variableSymbol->Type->AsPointerType->ReferentType;
         Debug::Assert(operand->IsAddress || operand->IsPointer);

         operand = Phx::IR::MemoryOperand::New(
            functionUnit,
            referentType,
            nullptr,
            operand->AsVariableOperand,
            0,
            Phx::Alignment::NaturalAlignment(referentType),
            functionUnit->AliasInfo->IndirectAliasedMemoryTag,
            functionUnit->SafetyInfo->SafeTag
         );
      }

      CurrentOperand = operand;
   }
   else if (node->FieldDesignator)
   {
      node->FieldDesignator->Accept(this);      
   }
   else if (node->IndexedVariable)
   {
      node->IndexedVariable->Accept(this);      
   }
   else
   {
      // This is variable_access^.
      // Attempt to dereference the variable access.

      node->VariableAccess->Accept(this); 

      Phx::IR::Operand ^ operand = CurrentOperand;

      // If the operand is of file type, load the $current_value
      // field. This is for code statements such as:
      // n := f^

      if (TypeBuilder::IsFileType(operand->Type))
      {
         Phx::Symbols::FieldSymbol ^ fieldSymbol;

         if (! ModuleBuilder::Runtime->IsRuntimeFileSymbol(operand->Symbol))
         {
            fieldSymbol = TypeBuilder::FindFieldSymbol(
               operand->Type->AsAggregateType,
               "$current_value"
            );

            if (operand->Symbol)
            {
               CurrentOperand = Phx::IR::VariableOperand::New(
                  functionUnit,
                  fieldSymbol->Field,
                  operand->Symbol
               );
            }
            else
            {
               CurrentOperand = Phx::IR::MemoryOperand::New(
                  functionUnit,
                  fieldSymbol->Field,
                  nullptr,
                  operand->BaseOperand,
                  0,
                  Phx::Alignment::NaturalAlignment(fieldSymbol->Field->Type),
                  functionUnit->AliasInfo->IndirectAliasedMemoryTag,
                  functionUnit->SafetyInfo->SafeTag
               );      
            }
         }
         else
         {
            Output::ReportError(
               node->VariableAccess->SourceLineNumber,
               Error::AmbiguousFileAccess,
               node->VariableAccess->Name
            );
            CurrentOperand = nullptr;
            return;
         }
      }

      // Check for pointer dereferences.

      else if (TypeBuilder::IsPointerType(operand->Type))
      {
         if (!node->UpArrow->Equals(String::Empty) && 
             this->fieldDesignatorCount == 0)
         {
            Phx::Types::PointerType ^ pointerType = 
               operand->Type->AsPointerType;

            Phx::Types::Type ^ domainType = 
               pointerType->ReferentType;

            // Load the memory location of the pointer into the
            // current operand.

            CurrentOperand = Phx::IR::MemoryOperand::New(
               functionUnit,
               domainType,
               nullptr,
               operand->AsVariableOperand,
               0,
               Phx::Alignment::NaturalAlignment(domainType),
               functionUnit->AliasInfo->IndirectAliasedMemoryTag,
               functionUnit->SafetyInfo->SafeTag
            );
         }
      }

      // The user performed an invalid operation, such as attempting
      // to dereference an integer value.

      else
      {
         Output::ReportError(
            node->SourceLineNumber,
            Error::GeneralTypeExpected,
            "file or pointer"
         );
         CurrentOperand = nullptr;
      }
   } 
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given VariableDeclarationListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   VariableDeclarationListNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given VariableDeclarationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   VariableDeclarationNode^ node
)
{
   // Grammar:
   // identifier COLON type_denoter |
   // identifier_list COLON type_denoter

   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // The type of each identifier defined by this node is defined 
   // by the TypeDenoter of the node.

   // Obtain the static type of the type denoter.
   
   Phx::Types::Type ^ staticType = node->TypeDenoter->Type;

   // If the static type is unknown, this is a new anonymous
   // type. Resolve the new type now.

   if (staticType->IsUnknown)
   {
      staticType = ResolveNewType(node->TypeDenoter);
   }

   // Track the run-time type of the variable.
   // This can differ from the static type in cases such as 
   // file and set declarations. This is because we track
   // the backing types of file and set types for type-checking
   // purposes, but to the user, these variables are simply
   // of type 'file' or 'set'.

   Phx::Types::Type ^ runtimeType = staticType;

   bool isSetType  = TypeBuilder::IsSetType(staticType);
   bool isFileType = isSetType ? false : TypeBuilder::IsFileType(staticType);
      
   // The first node is either an identifier or identifier_list.
   // Consolidate everything into a single list to simply processing.

   List<String ^> ^ identifiers = gcnew List<String ^>();
   List<int> ^ lineNumbers = gcnew List<int>();

   if (node->Identifier)
   {
      identifiers->Add(node->Identifier->Name);
      lineNumbers->Add(node->Identifier->SourceLineNumber);
   }
   else
   {
      Debug::Assert(node->IdentifierList != nullptr);

      for each(FormalParameterNode^ formalParamNode in 
         node->IdentifierList->FormalParameters)
      {
         identifiers->Add(formalParamNode->Identifier->Name);
         lineNumbers->Add(formalParamNode->Identifier->SourceLineNumber);
      }
   }

   // Add a new variable declaration symbol to the table of 
   // variable symbols mapped to the current function unit for
   // each member of the identifier list.

   Debug::Assert(identifiers->Count == lineNumbers->Count);
   for (int i = 0; i < identifiers->Count; ++i)
   {
      Phx::Symbols::Symbol ^ variableSymbol = 
         ModuleBuilder::AddVariableDeclarationSymbol(      
            functionUnit,
            identifiers[i], 
            runtimeType,
            Phx::Symbols::StorageClass::Auto,
            lineNumbers[i]
         );

      if (variableSymbol->Type->IsArray)
      {
         // If the symbol is an array of sets, we must allocate 
         // each element explicitly.

         Phx::Types::UnmanagedArrayType ^ arrayType = 
            variableSymbol->Type->AsUnmanagedArrayType;

         if (TypeBuilder::IsSetType(arrayType->ElementType))
         {
            ModuleBuilder::AllocateSetArray(
               functionUnit,
               variableSymbol,
               lineNumbers[i]
            );  
         }
      }

      // Perform type-specific allocation for sets and files.

      if (isSetType)
      {
         ModuleBuilder::AllocateSet(
            functionUnit,
            variableSymbol,
            staticType->AsPointerType->ReferentType,
            lineNumbers[i]
         );     
      }
      else if (isFileType)
      {
         ModuleBuilder::AllocateFile(
            functionUnit,
            variableSymbol,
            staticType,
            lineNumbers[i]
         );     
      }
   } 
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given VariableDeclarationPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   VariableDeclarationPartNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given VariableParameterSpecificationNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   VariableParameterSpecificationNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given VariantListNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   VariantListNode^ node
)
{	
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given VariantNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   VariantNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given VariantPartNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   VariantPartNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given VariantSelectorNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   VariantSelectorNode^ node
)
{
   VisitChildren(node);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given WhileStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   WhileStatementNode^ node
)
{
   throw gcnew Exception("Visit() not implemented for WhileStatementNode^");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Evaluates the given WithStatementNode object
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::Visit
(
   WithStatementNode^ node
)
{   
   throw gcnew Exception("Visit() not implemented for WithStatementNode^");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Creates a new Phx::Types::Type ^ object from the 
//    given TypeDenoterNode object.
//
// Remarks:
//
//
// Returns:
//
//    The newly created type.
//
//-----------------------------------------------------------------------------

Phx::Types::Type ^ 
Evaluator::ResolveNewType
(
   TypeDenoterNode ^ node
)
{
   // Save the current new type name.

   String ^ currentTypeName = NewTypeName;
   
   // Generate a new unique type name.

   String ^ structureName = TypeBuilder::GetUniqueTypeName();
   NewTypeName = structureName;

   // Process the new type node. This will generate
   // a new ordinal, structured, or pointer type.
      
   node->NewType->Accept(this);

   // Establish an identifier for the node for the next 
   // time we need to evalulate its type.

   node->ResolveType(NewTypeName);

   // Restore the previous new type name value.

   NewTypeName = currentTypeName;     

   // Return the newly created type.

   return TypeBuilder::GetTargetType(structureName);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds the index operand for an array variable access.
//
// Remarks:
//
//
// Returns:
//
//    The index operand for the array access.
//
//-----------------------------------------------------------------------------

Phx::IR::VariableOperand ^ 
Evaluator::BuildArrayIndexOperand
(
   Phx::Types::UnmanagedArrayType ^ arrayType,
   IndexedVariableNode ^ node
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // Get the value ranges associated with each rank
   // of the array type.

   array<TypedValueRange ^> ^ ranges = 
      TypeBuilder::GetArrayIndexRanges(arrayType);
   
   if (node->IndexExpressionList->IndexExpressions->Count != ranges->Length)
   {
      // User-provided indices did not match actual index count.

      Output::ReportError(
         node->SourceLineNumber,
         Error::IndexListCount,
         node->IndexExpressionList->IndexExpressions->Count,
         ranges->Length
      );

      return nullptr;
   }

   // Create an initial operand to represent the byte offset.
   // Because the resulting offset can be a combination of symbolic
   // and literal offsets (e.g. A[i,3+j]), several intermediate
   // calculations may be necessary. 
   
   // This method could probably be modified to make fewer
   // intermediate calculations.

   Phx::IR::Operand ^ offsetOperand = 
      Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) 0
      );

   int M = 1;

   IndexExpressionNode ^ expressionNode = nullptr; 
   int count = node->IndexExpressionList->IndexExpressions->Count;

   for (int i = count - 1; i >= 0; --i)
   {
      expressionNode = node->IndexExpressionList->IndexExpressions[i];

      // Evaluate raw index expression. This will generate an operand.

      expressionNode->Expression->Accept(this);
      Phx::IR::Operand ^ operand = CurrentOperand;
      if (operand == nullptr)
         continue;

      // Verify that the provided index type matches the 
      // declared index type.

      Phx::Types::Type ^ expectedIndexType = ranges[i]->Type;
      if (! operand->Type->Equals(expectedIndexType))
      {
         String ^ expectedTypeString;
         try
         {
            if (expectedIndexType->IsEnumType)
            {  expectedTypeString = "enum";
            }
            else if (TypeBuilder::IsSubrangeType(expectedIndexType))
            {  expectedTypeString = "subrange";
            }
            else
            {  expectedTypeString = expectedIndexType->TypeSymbol->NameString;
            }
         }
         catch (NullReferenceException ^)
         {
            expectedTypeString = "(unknown)";
         }

         Output::ReportError(
            expressionNode->SourceLineNumber,
            Error::MismatchedIndexType,
            expectedTypeString
         );
      }

      // If the array is indexed by an enumerated type,
      // extract the primary field from the enum.

      if (operand->Type->IsEnumType)
      {
         Debug::Assert(operand->IsVariableOperand);

         operand = IRBuilder::ExtractPrimaryEnumField(
            functionUnit,
            operand
         );
      }

      // Verify the index falls within the allowable range.

      // If the operand is an immediate, perform a static
      // (compile-time) range check.

      if (operand->IsImmediateOperand)
      {
         TypeBuilder::StaticCheckRange(
            functionUnit,
            ranges[i]->Type,
            operand->AsImmediateOperand,
            expressionNode->SourceLineNumber
         );
      }

      // Otherwise, perform the check at run time.

      else
      {
         TypeBuilder::DynamicCheckRange(
            functionUnit,
            ranges[i]->Type,
            operand,
            expressionNode->SourceLineNumber
            );
      }

      // Normalize to [0..(Max-Min)] by subtracting the "lower" bound value.

      Phx::IR::ImmediateOperand ^ indexStartOperand = 
         Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) ranges[i]->Lower
         );

      operand = IRBuilder::EmitBinaryArithmeticOp(
         functionUnit,
         MINUS,
         functionUnit->TypeTable->Int32Type,
         operand,
         indexStartOperand,
         expressionNode->SourceLineNumber
         );

      // Offset += I(i) * M

      Phx::IR::Operand ^ temporaryOperand = 
         IRBuilder::EmitBinaryArithmeticOp(
            functionUnit,
            STAR,
            functionUnit->TypeTable->Int32Type,
            Phx::IR::ImmediateOperand::New(
               functionUnit,
               functionUnit->TypeTable->Int32Type,
               (int) M
               ),
            operand,
            expressionNode->SourceLineNumber
         );

      offsetOperand = IRBuilder::EmitBinaryArithmeticOp(
         functionUnit,
         PLUS,
         functionUnit->TypeTable->Int32Type,
         temporaryOperand,
         offsetOperand,
         expressionNode->SourceLineNumber
      );
      
      //  M *= Range(i)

      M *= static_cast<int>(ranges[i]->Range);  
   }
    
   // Offset *= ArrayElementSize

   offsetOperand = IRBuilder::EmitBinaryArithmeticOp(
      functionUnit,
      STAR,
      functionUnit->TypeTable->Int32Type,
      Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) arrayType->ElementType->ByteSize 
         ),
      offsetOperand,
      expressionNode->SourceLineNumber
   );

   // Sanity-check that we ended up with a variable operand
   // object (index operands must be variable operands).
   // If not, create an expression temporary and assign to that.

   if (! offsetOperand->IsVariableOperand)
   {
      Phx::IR::VariableOperand ^ expressionTemporary =
         Phx::IR::VariableOperand::NewExpressionTemporary(
            functionUnit,
            functionUnit->TypeTable->Int32Type
         );

      offsetOperand = IRBuilder::EmitAssign(
         functionUnit,
         expressionTemporary,
         offsetOperand,
         expressionNode->SourceLineNumber
      );
   }

   // Perform a run-time check that the final index
   // value falls within the allowed range.

   TypeBuilder::DynamicCheckRange(
      functionUnit,
      offsetOperand,
      gcnew ValueRange(
         0, 
         arrayType->ByteSize - arrayType->ElementType->ByteSize
      ),
      expressionNode->SourceLineNumber
   );

   return offsetOperand->AsVariableOperand;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Extracts the ordinal value from the given ConstantNode object.
//
// Remarks:
//
//
// Returns:
//
//    The ordinal value of the constant.
//
//-----------------------------------------------------------------------------

int
Evaluator::GetOrdinalValue
(
   Phx::FunctionUnit ^ functionUnit,
   ConstantNode ^ node,
   bool % error
)
{
   error = false;

   // Grammar:
   // sign non_string
   // non_string
   // CHARACTER_STRING

   // If the node is a character string, return the single
   // character value if the string is one character long.
   // Otherwise, return an error.

   if (node->CharacterString)
   {
      if (node->CharacterString->IsCharacterType)
      {
         return node->CharacterString->CharValue;
      }
      else
      {
         error = true;
         return 0;
      }
   }

   // Try to extract the ordinal from the non-string type.

   else
   {
      // Extract the sign from the node.

      int mulFactor;
      switch (node->Sign)
      {
      case MINUS:
         mulFactor = -1;
         break;
      default:
         mulFactor = +1;
         break;
      }

      Phx::Symbols::Symbol ^ symbol = nullptr;

      switch (node->NonString->Kind)
      {
         // If the non-string node kind is an identifier,
         // extract the constant value from the symbol 
         // associated with the identifier.

      case NonStringNode::ValueKind::Identifier:
         
         symbol = ModuleBuilder::LookupSymbol(
            functionUnit,
            node->NonString->Identifier->Name
         );

         if (symbol != nullptr && 
             ModuleBuilder::IsLocalConstant(functionUnit, symbol))
         {
            // The symbol is a local constant; extract the ordinal value 
            // from the symbol.

            if (symbol->Type->IsEnumType)
            {
               return TypeBuilder::GetEnumOrdinalValue(
                  symbol->Type->AsEnumType,
                  symbol
               );
            }
            else
            {
               ConstantValue ^ constantValue = 
                  ModuleBuilder::GetLocalConstantValue(
                     functionUnit,
                     symbol
                  );

               switch (constantValue->GetType())
               {            
               case ConstantValue::Type::Character:
               case ConstantValue::Type::Integer:

                  // Return the constant character or integer value.

                  return int::Parse(constantValue->ToString());

               case ConstantValue::Type::Real:
               case ConstantValue::Type::String:
               default:

                  // Error; real and string are not ordinal types.

                  error = true;
                  return 0;
               }
            }           

            Debug::Assert(false);
         }

         else if (symbol != nullptr && 
                  symbol->Type->IsEnumType)
         {
            return TypeBuilder::GetEnumOrdinalValue(
               symbol->Type->AsEnumType,
               symbol
            );
         }
         error = true;
         return 0;
         
      case NonStringNode::ValueKind::RealValue:

         // Error; real is not an ordinal type.

         error = true;
         return 0;

      case NonStringNode::ValueKind::IntValue:

         // Return the constant integer value.

         return mulFactor * node->NonString->IntegerValue;

      default:
         Debug::Assert(false);
         return 0;
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds a procedure call from the provided procedure name and
//    parameters.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::BuildProcedureCall
(
   String ^ procedureName,
   ParamsNode ^ paramsNode,
   int sourceLineNumber
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;
      
   // Create an argument list for the call.
   // We maintain a stack of argument lists because some of
   // the arguments might be function/procedure calls themselves.
   
   IRBuilder::PushArgumentList();

   // Process the parameter list. This will add entries to the
   // current argument list.

   int errorCount = Output::ErrorCount;
   if (paramsNode)
      paramsNode->Accept(this);
   
   // If no errors occurred, emit the call.

   Phx::IR::Instruction ^ callInstruction = nullptr;
   if (errorCount == Output::ErrorCount)
   {
      callInstruction = IRBuilder::EmitCall(
         functionUnit,      
         procedureName,
         IRBuilder::ArgumentList,
         sourceLineNumber
      );
   }

   // Pop the argument list.

   IRBuilder::PopArgumentList();

   // Set the current operand.

   if (callInstruction != nullptr && 
       callInstruction->DestinationOperand != nullptr)
      CurrentOperand = callInstruction->DestinationOperand;
   else
      CurrentOperand = nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Helper function for building For loops.
//
// Remarks:
//
//    Used by OpenForStatementNode, ClosedForStatementNode.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::BuildForLoop
(
   ForStatementNode ^ node
)
{
   // Grammar:
   // FOR control_variable ASSIGNMENT initial_value direction
   //  final_value DO {open_statement | closed_statement}

   // Get the current function unit.

   Phx::FunctionUnit^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;   
   
   int sourceLineNumber = node->SourceLineNumber;

   // Verify that initial_value and final_value:
   //  1. Are ordinal types.
   //  2. Are compatible with each other.

   Phx::Types::Type ^ initialType = node->InitialValue->Expression->Type;
   Phx::Types::Type ^ finalType = node->FinalValue->Expression->Type;

   int errorCount = Output::ErrorCount;
   if (! TypeBuilder::IsOrdinalType(initialType))
   {
      Output::ReportError(
         sourceLineNumber,
         Error::GeneralTypeExpected,
         "ordinal"
      );
   }
   if (! TypeBuilder::IsOrdinalType(finalType))
   {
      Output::ReportError(
         sourceLineNumber,
         Error::GeneralTypeExpected,
         "ordinal"
      );
   }
   if (! TypeBuilder::AreEquivalentTypes(initialType, finalType))
   {
      Output::ReportError(
         sourceLineNumber,
         Error::IncompatibleTypes
      );
   }

   // Exit if type validation failed.

   if (errorCount != Output::ErrorCount)
   {
      return;
   }

   // Make reuse of the AssignmentStatementNode to generate the initial value.

   AssignmentStatementNode ^ assignmentNode = gcnew AssignmentStatementNode(
      sourceLineNumber,
      gcnew VariableAccessNode(
         sourceLineNumber,
         0,
         node->ControlVariable->Identifier
      ),
      node->InitialValue->Expression
   );
     
   // Remember the current error count before evaluating
   // the assignment statement.
   
   errorCount = Output::ErrorCount;
   
   // Process the assignment statement.

   assignmentNode->Accept(this);

   // Only generate IR if the assignment succeeded.

   if (errorCount != Output::ErrorCount)
   {
      return;
   }

   // Obtain the operand established when we processed the 
   // above AssignmentStatementNode.

   Phx::IR::Operand ^ controlOperand = CurrentOperand;

   Phx::Types::Type ^ controlTempOperandType = controlOperand->Type;
   if (TypeBuilder::IsSubrangeType(controlTempOperandType))
   {
      controlTempOperandType = 
         TypeBuilder::GetOrdinalType(controlTempOperandType->ByteSize);
   }

   Phx::IR::Operand ^ controlTempOperand = 
      Phx::IR::VariableOperand::NewExpressionTemporary(     
         functionUnit,
         controlTempOperandType
      );

   IRBuilder::EmitAssign(
      functionUnit,
      controlTempOperand,
      controlOperand,
      assignmentNode->SourceLineNumber
   );
      
   // Obtain the operand that represents the terminal value
   // of the loop iterator.

   node->FinalValue->Accept(this);
   Phx::IR::Operand ^ terminalOperand = CurrentOperand;

   // Create increment operand and appropriate arithmetic/relational operators.

   Phx::IR::ImmediateOperand ^ incrementOperand = 
      Phx::IR::ImmediateOperand::New(
         functionUnit,
         controlTempOperandType->IsEnumType ? 
            functionUnit->TypeTable->Int32Type : controlTempOperandType,
         (int) 1
      );
   unsigned arithmeticOp = node->Direction->IsTo ? PLUS : MINUS;
   unsigned relationalOp = node->Direction->IsTo ? GT : LT;

   // Create the entry-point of the loop.

   Phx::IR::LabelInstruction ^ enterLoopLabelInstruction = 
      Phx::IR::LabelInstruction::New(functionUnit);

   IRBuilder::EmitGoto(
      functionUnit, 
      enterLoopLabelInstruction, 
      sourceLineNumber
   );

   IRBuilder::EmitLabel(
      functionUnit, 
      enterLoopLabelInstruction
   );
   
   // Process the conditional statement.
   
   CurrentOperand = IRBuilder::EmitBinaryRelationalOp(
      functionUnit,
      relationalOp,
      controlTempOperand,
      terminalOperand,
      sourceLineNumber
   );

   Phx::IR::LabelInstruction ^ exitLoopLabelInstruction = 
      Phx::IR::LabelInstruction::New(functionUnit);

   Phx::IR::LabelInstruction ^ loopBodyLabelInstruction = 
      Phx::IR::LabelInstruction::New(functionUnit);

   IRBuilder::EmitConditionalBranch(
      functionUnit,
      CurrentOperand,
      exitLoopLabelInstruction,
      loopBodyLabelInstruction
   );

   // Emit loop body label instruction.

   IRBuilder::EmitLabel(functionUnit, loopBodyLabelInstruction);  

   // Copy control temp to actual control value.

   IRBuilder::EmitAssign(
      functionUnit,
      controlOperand,
      controlTempOperand,
      assignmentNode->SourceLineNumber
      );      

   // Process the statement body.

   node->Statement->Accept(this);

   // Increment/decrement the counter.

   Phx::IR::Operand ^ incrementResult = IRBuilder::EmitBinaryArithmeticOp(
      functionUnit,
      arithmeticOp,
      controlTempOperandType,
      controlOperand,
      incrementOperand,
      sourceLineNumber
   );

   IRBuilder::EmitAssign(
      functionUnit,
      controlTempOperand,
      incrementResult,
      sourceLineNumber      
   );

   // Loop back to top.

   IRBuilder::EmitGoto(
      functionUnit,
      functionUnit->LastInstruction->Previous,
      enterLoopLabelInstruction,
      sourceLineNumber
   );

   // Emit exit loop label instruction.

   IRBuilder::EmitLabel(functionUnit, exitLoopLabelInstruction);   
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Helper function for building While loops.
//
// Remarks:
//
//    Used by OpenWhileStatementNode, ClosedWhileStatementNode.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::BuildWhileLoop
(
   WhileStatementNode ^ node   
)
{  
   // Get the current function unit.

   Phx::FunctionUnit^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;   
 
   int sourceLineNumber = node->SourceLineNumber;

   // Create a label for the entry-point of the loop.

   Phx::IR::LabelInstruction ^ enterLoopLabelInstruction = 
      Phx::IR::LabelInstruction::New(functionUnit);

   // Because control flow in Phoenix is explicit, we must 
   // make an explicit jump into the loop.

   // First, emit the goto instruction.

   IRBuilder::EmitGoto(
      functionUnit, 
      enterLoopLabelInstruction, 
      sourceLineNumber
   );

   // Now emit the label target for entry point to the loop.

   IRBuilder::EmitLabel(
      functionUnit, 
      enterLoopLabelInstruction
   );

   // Evaluate the Boolean expression branch of the AST. 
   // This evaluation will reduce any compound Boolean expressions 
   // into a single IR operand.

   int errorCount = Output::ErrorCount;
   node->BooleanExpression->Accept(this);
   if (errorCount != Output::ErrorCount)
   {
      return;
   }

   // Evaluation of the Boolean expression sets the
   // current operand to a true/false condition operand.

   Phx::IR::Operand ^ conditionOperand = CurrentOperand;

   // Generate label target instructions for the
   // true (process loop body) and false (exit loop)
   // conditions of the Boolean expression.

   Phx::IR::LabelInstruction ^ loopBodyLabelInstruction = 
      Phx::IR::LabelInstruction::New(functionUnit);

   Phx::IR::LabelInstruction ^ exitLoopLabelInstruction = 
      Phx::IR::LabelInstruction::New(functionUnit);

   // Append a conditional branch instruction to 
   // the IR instruction stream. 

   IRBuilder::EmitConditionalBranch(
      functionUnit,
      conditionOperand,
      loopBodyLabelInstruction, 
      exitLoopLabelInstruction
   );

   // Emit the label that marks the entrance of the loop body.

   IRBuilder::EmitLabel(functionUnit, loopBodyLabelInstruction);
   
   // Process the AST nodes that represent the statement part of the
   // while loop body.

   node->Statement->Accept(this);

   // Jump back to the top of the loop.

   IRBuilder::EmitGoto(
      functionUnit, 
      enterLoopLabelInstruction, 
      sourceLineNumber
   );

   // Emit the label that marks the exit of the loop.

   IRBuilder::EmitLabel(functionUnit, exitLoopLabelInstruction);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the AggregateType object associated with the given symbol.
//
// Remarks:
//
//    Used by BuildWithStatement.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

static Phx::Types::AggregateType ^
GetSymbolAggregateType
(
 Phx::Symbols::Symbol ^ symbol
)
{
   if (symbol == nullptr)
   {
      return nullptr;
   }
   Phx::Types::AggregateType ^ type = nullptr;
            
   if (symbol->Type->IsAggregateType)
   {
      type = symbol->Type->AsAggregateType;
   }
   else if (symbol->Type->IsArray && 
      symbol->Type->AsUnmanagedArrayType->ElementType->IsAggregateType)
   {
      type = symbol->Type->AsUnmanagedArrayType->ElementType->AsAggregateType;
   }
   else if (symbol->Type->IsPointerType && 
      symbol->Type->AsPointerType->ReferentType->IsAggregateType)
   {
      type = symbol->Type->AsPointerType->ReferentType->AsAggregateType;
   }

   if (type && type->IsEnumType)
   {
      // Invalidate enumerated types.

      type = nullptr;
   }
   return type;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Helper function for building With statements.
//
// Remarks:
//
//    Used by OpenWithStatementNode, ClosedWithStatementNode.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::BuildWithStatement
(
   WithStatementNode ^ node,
   StatementNodeBase ^ statementNode
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   ++WithNestingCount;

   List<VariableAccessNode ^> ^ variableAccessNodes = gcnew
      List<VariableAccessNode ^>(node->RecordVariableList->VariableAccesses);
     
   Dictionary<VariableAccessNode ^, List<VariableAccessNode ^> ^> ^ 
      insertionPoints = nullptr;
   for each (VariableAccessNode ^ variableAccessNode in variableAccessNodes)
   {  
      if (variableAccessNode->FieldDesignator)      
      {
         // Replace the syntax "s.x.y" to resemble "s,t,y".

         if (insertionPoints == nullptr)
         {
            insertionPoints = gcnew 
              Dictionary<VariableAccessNode ^, List<VariableAccessNode ^> ^>();
         }

         List<VariableAccessNode ^> ^ insertList =
            gcnew List<VariableAccessNode ^>();

         insertionPoints[variableAccessNode] = insertList;

         array<String ^> ^ fieldTokens = variableAccessNode->Name->Split('.');
         for each (String ^ fieldToken in fieldTokens)
         {
            insertList->Add(gcnew VariableAccessNode(
               variableAccessNode->SourceLineNumber,
               0,
               gcnew IdentifierNode(
                     variableAccessNode->SourceLineNumber,
                     fieldToken
                  )
               )
            );
         }
      }  
   }

   if (insertionPoints)
   {
      for each (VariableAccessNode ^ key in insertionPoints->Keys)
      {
         variableAccessNodes->InsertRange(
            variableAccessNodes->IndexOf(key),
            insertionPoints[key]
         );
         variableAccessNodes->Remove(key);
      }
   }

   // Widen scope to each record variable access.

   int pushCount = 0;

   for each (VariableAccessNode ^ variableAccessNode in variableAccessNodes)
   {        
      String ^ name = variableAccessNode->Name;
      Phx::Symbols::Symbol ^ symbol = 
         ModuleBuilder::LookupSymbol(functionUnit, name);
      
      Phx::Types::AggregateType ^ resolvedAggregateType = 
         GetSymbolAggregateType(symbol);

      if (resolvedAggregateType)
      {
         // Is the variable access a field of another 
         // variable access in the list?
         
         if (symbol->IsFieldSymbol)
         {
            Phx::Symbols::FieldSymbol ^ fieldSymbol = symbol->AsFieldSymbol;
            bool didInsert = false;

            for (int i = 0; i < this->withAccessNodes->Count; ++i)
            {
               VariableOrFieldAccess ^ variableOrFieldAccess = 
                  this->withAccessNodes[i];

               if (IsFieldOf(fieldSymbol, variableOrFieldAccess))
               {
                  int sourceLineNumber = variableAccessNode->SourceLineNumber;
                  this->withAccessNodes->Insert(0, 
                     gcnew VariableOrFieldAccess(
                        gcnew FieldDesignatorNode(
                           sourceLineNumber,
                           variableAccessNode, 
                           gcnew IdentifierNode(sourceLineNumber, name)
                           )
                        )
                     );

                  didInsert = true;
                  break;
               }
            }

            if (!didInsert)
            {
               Output::ReportError(
                  node->SourceLineNumber,
                  Error::InvalidWithField,
                  name
               );
            }
         }
         else
         {
            this->withAccessNodes->Insert(0, 
               gcnew VariableOrFieldAccess(variableAccessNode));
         }         
         
         Phx::Symbols::Table ^ symTable = 
            TypeBuilder::GetSymbolTable(resolvedAggregateType);
         ModuleBuilder::PushScope(symTable);
         ++pushCount;
      }
      else
      {
         Output::ReportError(
            node->SourceLineNumber,
            Error::InvalidWithField,
            name
         );
      }
   }

   // Process statement block.

   statementNode->Accept(this);

   // Unwind scope widened during execution of the With block.

   Debug::Assert(pushCount >= 0);
   while (pushCount)
   {
      ModuleBuilder::PopScope();
      this->withAccessNodes->RemoveAt(0);
      --pushCount;
   }

   --WithNestingCount;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Determines whether the given symbol is a field of the given
//    variable/field access.
//
// Remarks:
//
//
// Returns:
//
//    true if the symbol is a field of the variable/field access,
//    false otherwise.
//
//-----------------------------------------------------------------------------

bool 
Evaluator::IsFieldOf
(
   Phx::Symbols::FieldSymbol ^ fieldSymbol,
   VariableOrFieldAccess ^ variableOrFieldAccess   
)
{
   Phx::Types::Type ^ type;
   if (variableOrFieldAccess->VariableAccess)
      type = variableOrFieldAccess->VariableAccess->Type;
   else
      type = variableOrFieldAccess->FieldDesignator->Type;

   Phx::Types::AggregateType ^ enclosingType = 
      fieldSymbol->EnclosingAggregateType;
   return enclosingType->Equals(type);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Walks the list of record variable access specifiers and finds the 
//    first symbol that matches the symbol's enclosing type.
//
// Remarks:
//
//
// Returns:
//
//    An operand representing the expanded symbol reference.
//
//-----------------------------------------------------------------------------

Phx::IR::Operand ^
Evaluator::ExpandWithSymbol
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Symbols::FieldSymbol ^ fieldSymbol,
   int sourceLineNumber
)
{
   Debug::Assert(WithNestingCount > 0);

   // Remember the original field symbol name in case we encounter
   // an error.
   
   String ^ fieldName = fieldSymbol->NameString;

   // Walk the list of record variable access specifiers (from narrow
   // to wider scope) and find the first symbol that matches the symbol's
   // enclosing type.
   
   // Because with statements can be nested, we may need to generate 
   // a number of assignment statements to resolve the reference.

   // Maintain a list of symbols that represent each field access, and 
   // we'll piece them together after they are all collected.

   List<Phx::Symbols::Symbol ^> ^ symbolList = 
      gcnew List<Phx::Symbols::Symbol ^>();

   int j = 0;
   while (fieldSymbol)
   {
      for (int i = j; i < this->withAccessNodes->Count; ++i)
      {
         VariableOrFieldAccess ^ variableOrFieldAccess = 
            this->withAccessNodes[i];
         if (IsFieldOf(fieldSymbol, variableOrFieldAccess))
         {
            symbolList->Insert(0, fieldSymbol);
            
            if (variableOrFieldAccess->FieldDesignator)
            {
               // Prepare to resolve the next field symbol.

               fieldSymbol = ModuleBuilder::LookupSymbol(
                  functionUnit,
                  variableOrFieldAccess->FieldDesignator->Identifier->Name
               )->AsFieldSymbol;
            }
            else
            {
               // Insert the base variable access symbol.

               symbolList->Insert(0, ModuleBuilder::LookupSymbol(
                  functionUnit,
                  variableOrFieldAccess->VariableAccess->Name
                  )
               );

               // Terminate the loop.

               fieldSymbol = nullptr;
            }

            j = i + 1;
            break;
         }
      }
   }

   // The 'final' (first because we're using the operand list like a stack)
   // symbol in the list should not reference a field symbol.

   int count = symbolList->Count;
   if (symbolList[0]->IsFieldSymbol)
   {
      Output::ReportError(
         sourceLineNumber,
         Error::InvalidWithField,
         fieldName
      );
      return nullptr;
   }

   Phx::IR::VariableOperand ^ baseOperand = Phx::IR::VariableOperand::New(
      functionUnit,
      symbolList[0]->Type,
      symbolList[0]
   );

   if (!baseOperand->IsAddress && !baseOperand->IsPointer)
      baseOperand->ChangeToAddress();
   
   int bitOffset = 0;
   for (int i = 1; i < count; ++i)
   {
      bitOffset += symbolList[i]->AsFieldSymbol->BitOffset;
   }

   Phx::Types::Field ^ field = symbolList[count-1]->AsFieldSymbol->Field;
   
   Phx::IR::MemoryOperand ^ fieldAccessOperand = Phx::IR::MemoryOperand::New(
      functionUnit,
      field->Type,
      nullptr, 
      baseOperand,
      Phx::Utility::BitsToBytes(bitOffset),
      Phx::Alignment::NaturalAlignment(field->Type),
      functionUnit->AliasInfo->IndirectAliasedMemoryTag,
      functionUnit->SafetyInfo->SafeTag
   );

   return fieldAccessOperand;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Calculates the bit size of the given RecordSectionListNode object.
//
// Remarks:
//
//
// Returns:
//
//    The bit size of the given RecordSectionListNode object.
//
//-----------------------------------------------------------------------------

unsigned int
Evaluator::CalculateBitSize
(
   RecordSectionListNode ^ node
)
{
   unsigned int bitSize = 0;

   for each (RecordSectionNode ^ recordSectionNode
      in node->RecordSections)
         bitSize += CalculateBitSize(recordSectionNode);

   return bitSize;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Calculates the bit size of the given RecordSectionNode object.
//
// Remarks:
//
//
// Returns:
//
//    The bit size of the given RecordSectionNode object.
//
//-----------------------------------------------------------------------------

unsigned int
Evaluator::CalculateBitSize
(
   RecordSectionNode ^ node
)
{
   // The type of each identifier defined by this node 
   // is defined by the TypeDenoter of the node.
  
   Phx::Types::Type ^ type = node->TypeDenoter->Type;

   // Create a new anonymous type if the field does not have
   // have an associated type symbol.

   if (type->IsUnknown)
   {
      type = ResolveNewType(node->TypeDenoter);
   }

   // Calculate natural alignment for this type and return 
   // the overall size for all of the elements.

   Phx::Alignment ^ align = Phx::Alignment::NaturalAlignment(type);
      
   return align->BitSize * node->IdentifierList->FormalParameters->Count;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Calculates the bit size of the given VariantPartNode object.
//
// Remarks:
//
//
// Returns:
//
//    The bit size of the given VariantPartNode object.
//
//-----------------------------------------------------------------------------

unsigned int
Evaluator::CalculateBitSize
(
   VariantPartNode ^ node
)
{
   // Per the grammar, either both variant_selector and variant_list must be
   // valid, or both must be nil.

   if (node->VariantSelector || node->VariantList)
   {
      Debug::Assert(node->VariantSelector && node->VariantList);
   }
   if (node->VariantSelector == nullptr || node->VariantList == nullptr)
   {
      return 0;
   }

   unsigned int bitSize = 0;
   Phx::Types::Type ^ type;
   Phx::Alignment ^ align;

   // First add-in the variant_selector part. 
   // The type of the variant selector is denoted by its tag_type.
   
   type = node->VariantSelector->TagType->Identifier->Type;
   align = Phx::Alignment::NaturalAlignment(type);
   bitSize += align->BitSize;

   // Now add-in the variant_list part.

   int largestVariant = 0;

   for each (VariantNode ^ variantNode in node->VariantList->Variants)
   {
      int variantSize = 0;

      if (variantNode->RecordSectionList)
         variantSize += CalculateBitSize(variantNode->RecordSectionList);

      if (variantNode->VariantPart)
         variantSize += CalculateBitSize(variantNode->VariantPart);

      if (variantSize > largestVariant)
         largestVariant = variantSize;
   }

   bitSize += largestVariant;

   return bitSize;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds the elements of the given RecordSectionListNode object to the 
//    given aggregate type.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::AddFieldsToType
(
   Phx::Types::AggregateType ^ type,
   RecordSectionListNode ^ node
)
{
   // If we're creating a union of data, set the
   // absolute bit offset.

   int absoluteBitOffset = -1;
   if (this->unionBitOffset)
   {
      absoluteBitOffset = *this->unionBitOffset;
   }

   for each (RecordSectionNode ^ recordSectionNode
      in node->RecordSections)
   {
      AddFieldsToType(type, recordSectionNode, absoluteBitOffset);

      if (absoluteBitOffset >= 0)
      {
         absoluteBitOffset += CalculateBitSize(recordSectionNode);
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds the elements of the given RecordSectionNode object to the 
//    given aggregate type.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::AddFieldsToType
(
   Phx::Types::AggregateType ^ type,
   RecordSectionNode ^ node,
   int absoluteBitOffset
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // The type of each identifier defined by this node is defined 
   // by the node's TypeDenoter.
  
   Phx::Types::Type ^ fieldType = node->TypeDenoter->Type;
   if (fieldType->IsUnknown)
   {
      fieldType = ResolveNewType(node->TypeDenoter);
   }

   // Add each field element to the type.

   for each (FormalParameterNode ^ formalParameterNode 
      in node->IdentifierList->FormalParameters)
   {      
      TypeBuilder::AddFieldToType(
         functionUnit->Lifetime,
         type,
         fieldType,
         formalParameterNode->Identifier->Name,
         absoluteBitOffset,
         formalParameterNode->SourceLineNumber
      );

      if (absoluteBitOffset >= 0)
      {
         Phx::Alignment ^ align = Phx::Alignment::NaturalAlignment(fieldType);
         absoluteBitOffset += align->BitSize;
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds the elements of the given VariantPartNode object to the 
//    given aggregate type.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::AddFieldsToType
(
   Phx::Types::AggregateType ^ type,
   VariantPartNode ^ node
)
{
   // Get the current function unit.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::CurrentFunctionUnit;

   // Per the grammar, either both variant_selector and variant_list must be
   // valid, or both must be nil.

   if (node->VariantSelector || node->VariantList)
   {
      Debug::Assert(node->VariantSelector && node->VariantList);
   }
   if (node->VariantSelector == nullptr || node->VariantList == nullptr)
   {
      return;
   }

   // First add-in the variant_selector part. 
   // The type of the variant selector is denoted by its tag_type.
   
   Phx::Types::Field ^ variantSelectorField = TypeBuilder::AddFieldToType(
      functionUnit->Lifetime,
      type,
      node->VariantSelector->TagType->Identifier->Type,
      node->VariantSelector->TagField->Identifier->Name,
      -1,
      node->VariantSelector->SourceLineNumber
   );

   // Now add-in the variant_list part.

   // Add each variant part at the same offset (e.g. create a union of data)

   int * unionBitOffset0 = this->unionBitOffset;
   int unionBitOffset = 
      variantSelectorField->BitOffset + variantSelectorField->BitSize;
   this->unionBitOffset = &unionBitOffset;

   for each (VariantNode ^ variantNode in node->VariantList->Variants)
   {
      if (variantNode->RecordSectionList)
         AddFieldsToType(
            type, 
            variantNode->RecordSectionList
            );

      if (variantNode->VariantPart)
         AddFieldsToType(
            type, 
            variantNode->VariantPart
            );
   }

   this->unionBitOffset = unionBitOffset0;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Resolves forward declarations to undefined pointer types.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::FixupPointerTypes
(
   Phx::FunctionUnit ^ functionUnit, 
   int sourceLineNumber
)
{
   for each (String ^ pointerTypeName in this->pointerTypeFixups->Keys)
   {
      Phx::Types::PointerType ^ realType = nullptr;
      String ^ domainTypeName = this->pointerTypeFixups[pointerTypeName];

      Phx::Symbols::TypeSymbol ^ realSymbol = 
         ModuleBuilder::LookupTypeSymbol(
         functionUnit,
         domainTypeName
      );

      if (realSymbol == nullptr)
      {
         // We could try to deal with the error and continue, 
         // but since we're still fairly early in compilation, 
         // just bail altogether.

         Output::ReportFatalError(
            sourceLineNumber,
            Error::UnknownTypeSymbol,
            domainTypeName
         );
         return;
      }

      // Get the existing pointer type.

      Phx::Types::PointerType ^ temporaryPointerType =
         TypeBuilder::GetPointerType(
            functionUnit,
            pointerTypeName
         );

      // Unregister it.

      TypeBuilder::UnregisterPointerType(
         functionUnit,
         temporaryPointerType
      );

      // Register the new pointer type.

      realType = TypeBuilder::RegisterPointerType(
         functionUnit,
         pointerTypeName,
         realSymbol->Type,
         0
      );

      // Replace all references to the temporary pointer
      // type with the new one.

      TypeBuilder::ReplaceAggregatePointerReferences(
         functionUnit,
         temporaryPointerType,
         realType
      );
   }

   this->pointerTypeFixups->Clear();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds the given identifiers to the given formal parameter list.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::AddFormalParameters
(
   IdentifierListNode ^ identifierListNode,
   IdentifierNode ^ identifierNode,
   FormalParameterType parameterType,
   List<FormalParameter ^> ^ formalParameters,
   int sourceLineNumber
)
{
   for each (FormalParameterNode ^ formalParameterNode in 
      identifierListNode->FormalParameters)
   {
      String ^ name = formalParameterNode->Identifier->Name;

      // Ensure the parameter has a unique identifier name.

      for each (FormalParameter ^ parameter in formalParameters)
      {         
         if (parameter->Name->Equals(name))
         {
            Output::ReportError(
               identifierListNode->SourceLineNumber,
               Error::FormalParameterRedeclaration,
               name
            );
         }
      }

      // Add the parameter to the list.

      Phx::Types::Type ^ type = 
         TypeBuilder::GetTargetType(identifierNode->Name);

      formalParameters->Add(
         gcnew FormalParameter(
            name,
            type,
            parameterType,
            formalParameterNode->SourceLineNumber)
         );

      // File and array parameters must be passed by reference.

      if (TypeBuilder::IsFileType(type))
      {      
         if (parameterType != FormalParameterType::Variable)
         {
            Output::ReportError(
               sourceLineNumber,
               Error::MustBeVarParameter,
               name, 
               "file"
            );
         }
      }
      else if (type->IsArray)
      {
         if(parameterType != FormalParameterType::Variable)
         {
            String ^ typeString = TypeBuilder::IsStringType(type) ? 
               "string" : "array";

            Output::ReportError(
               sourceLineNumber,
               Error::MustBeVarParameter,
               name, 
               typeString
            );
         }
      }
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Adds the given identifiers as procedure parameters to the given 
//    formal parameter list.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::AddFormalParameter
(   
   IdentifierNode ^ identifierNode,
   FormalParameterListNode ^ formalParameterListNode,
   FormalParameterType parameterType,
   Phx::Types::Type ^ resultType,
   List<FormalParameter ^> ^ formalParameters,
   int sourceLineNumber
)
{
   // Build the function type by using a FunctionTypeBuilder object.

   Phx::Types::FunctionTypeBuilder ^ functionTypeBuilder =
      Phx::Types::FunctionTypeBuilder::New(Phx::GlobalData::TypeTable);

   functionTypeBuilder->Begin();

   // Use cdecl calling convention for all Pascal functions.

   functionTypeBuilder->CallingConventionKind =
      Phx::Types::CallingConventionKind::CDecl;

   // Establish return type.

   functionTypeBuilder->AppendReturnParameter(resultType);

   // Append arguments.
   
   if (formalParameterListNode)
   {
      for each (FormalParameterSectionNode ^ sectionNode in 
         formalParameterListNode->FormalParameterSectionList->
            FormalParameterSections)
      {
         if (sectionNode->ValueParameterSpecification)
         {
            List<FormalParameter ^> ^ tempList = 
               gcnew List<FormalParameter ^>();

            AddFormalParameters(
               sectionNode->ValueParameterSpecification->IdentifierList,
               sectionNode->ValueParameterSpecification->Identifier,
               FormalParameterType::Value,
               tempList,
               sourceLineNumber
            );

            for each (FormalParameter ^ tempParameter in tempList)
            {
               functionTypeBuilder->AppendParameter(tempParameter->Type);
            }
         }
         else
         {
            // Section contains variable, function, or procedure specification;
            // however, when a procedure or function is passed as a parameter 
            // to another procedure or function, it may only contain 
            // value parameters.

            Output::ReportError(
               sectionNode->SourceLineNumber,
               Error::InvalidProcedureArgument
            );            
         }
      }
   }

   Phx::Types::FunctionType ^ functionType = 
      functionTypeBuilder->GetFunctionType();
  
   formalParameters->Add(
      gcnew FormalParameter(
         identifierNode->Name,
         functionType,
         parameterType,
         sourceLineNumber
      )
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Creates an array of FormalParameter objects from the given 
//    FormalParameterListNode object.
//
// Remarks:
//
//
// Returns:
//
//    An array of FormalParameter objects.
//
//-----------------------------------------------------------------------------

array<FormalParameter ^> ^ 
Evaluator::BuildFormalParameterList
(
   FormalParameterListNode ^ node
)
{
   if (node == nullptr)
   {
      return nullptr;
   }

   List<FormalParameter ^> ^ formalParameters = 
      gcnew List<FormalParameter ^>();

   // Add each formal parameter section to the FormalParameter list.

   FormalParameterSectionListNode ^ sectionListNode = 
      node->FormalParameterSectionList;
   for each (FormalParameterSectionNode ^ sectionNode in 
      sectionListNode->FormalParameterSections)
   {  
      // Call the appropriate Add method based on the 
      // parameter type (value, variable, procedure, function).

      // Value parameter.

      if (sectionNode->ValueParameterSpecification)
      {
         AddFormalParameters(
            sectionNode->ValueParameterSpecification->IdentifierList,
            sectionNode->ValueParameterSpecification->Identifier,
            FormalParameterType::Value,
            formalParameters,
            node->SourceLineNumber
         );
      }

      // Variable parameter.

      else if (sectionNode->VariableParameterSpecification)
      {
         AddFormalParameters(
            sectionNode->VariableParameterSpecification->IdentifierList,
            sectionNode->VariableParameterSpecification->Identifier,
            FormalParameterType::Variable,
            formalParameters,
            node->SourceLineNumber
         );
      }

      // Procedural parameter.

		else if (sectionNode->ProceduralParameterSpecification)
      {
         AddFormalParameter(
            sectionNode->ProceduralParameterSpecification->
               ProcedureHeading->ProcedureIdentification->Identifier,
            sectionNode->ProceduralParameterSpecification->
               ProcedureHeading->FormalParameterList,
            FormalParameterType::Procedure,
            Phx::GlobalData::TypeTable->VoidType,
            formalParameters,
            node->SourceLineNumber
         );
      }

      // Functional parameter.

      else 
      {
         Debug::Assert(
            sectionNode->FunctionalParameterSpecification != nullptr);

         AddFormalParameter(
            sectionNode->FunctionalParameterSpecification->
               FunctionHeading->Identifier,
            sectionNode->FunctionalParameterSpecification->
               FunctionHeading->FormalParameterList,
            FormalParameterType::Function,
            TypeBuilder::GetTargetType(
               sectionNode->FunctionalParameterSpecification->
                  FunctionHeading->ResultType->Identifier->Name),
            formalParameters,
            node->SourceLineNumber
         );
      }      
   }

   return formalParameters->ToArray();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds a procedure/function forward declaration.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::BuildProcedureOrFunctionDirective
(
   IdentifierNode ^ identifierNode,
   FormalParameterListNode ^ formalParameterListNode,
   Phx::Types::Type ^ resultType,
   DirectiveNode ^ directiveNode,
   int sourceLineNumber
)
{
   String ^ procedureName = identifierNode->Name;

   // Report an error if the procedure has already been declared.

   Phx::Symbols::Symbol ^ symbol = 
      this->moduleUnit->SymbolTable->NameMap->Lookup(
         Phx::Name::New(this->moduleUnit->Lifetime, procedureName)
      );

   if (symbol)
   {
      Output::ReportError(
         sourceLineNumber, 
         Error::SymbolRedeclaration,
         procedureName,
         TypeBuilder::GetSourceType(symbol->Type)
      );
      return;
   }

   String ^ directive = directiveNode->StringValue;
   String ^ directiveLwr = directive->ToLower();

   // Validate the directive type (must be forward or external).

   if (! directiveLwr->Equals("forward") && 
       ! directiveLwr->Equals("extern")  && ! directiveLwr->Equals("external"))
   {
      Output::ReportError(
         sourceLineNumber,
         Error::UnknownDirective, 
         directive
      );
      
      // Assume "forward" so we can continue processing.

      directive = directiveLwr = "forward";
   }

   // Construct an array of FormalParameters from the formal 
   // parameter list node.

   array<FormalParameter ^> ^ formalParameters = BuildFormalParameterList(
      formalParameterListNode
   );

   // Generate a 'stub' for the procedure or function.

   Phx::Symbols::Visibility visibility;
   DirectiveType directiveType;

   // Establish visiblity and directive values based on the
   // directive types.

   if (directiveLwr->Equals("forward"))
   {
      visibility = Phx::Symbols::Visibility::GlobalDefinition;
      directiveType = DirectiveType::Forward;
   }

   else
   {
      Debug::Assert(directiveLwr->StartsWith("extern"));

      visibility = Phx::Symbols::Visibility::GlobalReference;
      directiveType = DirectiveType::External;
   }
    
   Phx::FunctionUnit ^ functionUnit = ModuleBuilder::BuildBeginFunction(
      this->moduleUnit,
      procedureName,
      Phx::Types::CallingConventionKind::CDecl,
      visibility,
      formalParameters,
      resultType,
      directiveType,
      sourceLineNumber
   );

   if (functionUnit != nullptr)
   {
      ModuleBuilder::BuildEndFunction(functionUnit, sourceLineNumber);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Builds a procedure/function block.
//
// Remarks:
//
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
Evaluator::BuildProcedureOrFunctionBlock
(
   IdentifierNode ^ identifierNode,
   FormalParameterListNode ^ formalParameterListNode,
   Phx::Types::Type ^ resultType,
   BlockNode ^ blockNode,
   int sourceLineNumber
)
{  
   String ^ procedureName = identifierNode->Name;

   // Check for a forward or external declaration to this function.

   Phx::FunctionUnit ^ functionUnit = 
      ModuleBuilder::FindFunctionUnit(procedureName);

   // Report an error if a function unit with the given 
   // name already exists.

   if (functionUnit)
   {      
      Phx::Types::Type ^ returnType = 
         functionUnit->UnitSymbol->Type->
            AsFunctionType->ReturnType;

      String ^ functionOrProcedure = 
         returnType->Equals(
            this->moduleUnit->TypeTable->VoidType) ?
               "procedure" : "function";

      // Process based on directive type.

      DirectiveType directive = 
         ModuleBuilder::GetDirectiveType(functionUnit);

      if (directive == DirectiveType::External)
      {
         // User implemented an external function in the same module.

         Output::ReportError(
            sourceLineNumber,
            Error::InvalidExternalFunctionDefinition,
            procedureName, 
            functionOrProcedure
         );
         return;
      }
      else if (directive == DirectiveType::Forward)
      {  
         if (formalParameterListNode != nullptr || resultType != nullptr)
         {
            //  User repeated the parameter list or return type.

            Output::ReportError(
               sourceLineNumber,
               Error::InvalidForwardFunctionDefinition,
               procedureName,
               functionOrProcedure
            );
         }

         // Edit the function unit.

         List<Phx::IR::Instruction ^> ^ unlinkInstructions = 
            ModuleBuilder::BeginFunctionEdit(
               functionUnit, 
               sourceLineNumber
            );

         // Process the block part.

         blockNode->Accept(this);
         
         // Finish the edit.

         ModuleBuilder::EndFunctionEdit(
            unlinkInstructions,
            sourceLineNumber
         );

         // Clear the directive flag from the function.

         ModuleBuilder::SetDirectiveType(functionUnit, DirectiveType::None);

         return;
      }
   }

   // Function was not forward declared. We can define it now.

   // Build the formal parameter list for the function.

   array<FormalParameter ^> ^ formalParameters = BuildFormalParameterList(
      formalParameterListNode
   );

   // Build the beginning part of the function. 

   functionUnit = ModuleBuilder::BuildBeginFunction(
      this->moduleUnit,
      procedureName,
      Phx::Types::CallingConventionKind::CDecl,
      Phx::Symbols::Visibility::GlobalDefinition,
      formalParameters,
      resultType,
      DirectiveType::None,
      identifierNode->SourceLineNumber
   );

   if (functionUnit != nullptr)
   {
      // Process the block part.

      blockNode->Accept(this);

      // Now call BuildEndFunction to build the end part of the user
      // function.

      ModuleBuilder::BuildEndFunction(functionUnit, sourceLineNumber);
   }
}

} // namespace Pascal