//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Implementation of the IRBuilder class.
//
// Remarks:
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "YaccDeclarations.h"

using namespace System;
using namespace System::Diagnostics;

namespace Pascal 
{

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pushes a new argument list onto the argument list stack.
//    A stack of argument lists is required because an argument
//    itself may invoke a function or procedure call.
//
// Remarks:
//
//
// Returns:
//
//    The topmost argument list on the argument list stack.
//
//-----------------------------------------------------------------------------
   
List<Phx::IR::Operand ^> ^
IRBuilder::PushArgumentList()
{
   argumentLists->Insert(0, gcnew List<Phx::IR::Operand ^>());
   return ArgumentList;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Pops the topmost argument list from the argument list stack.
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
IRBuilder::PopArgumentList()
{
    argumentLists->RemoveAt(0);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the topmost argument list on the argument list stack.
//
// Remarks:
//
//
// Returns:
//
//    The topmost argument list on the argument list stack.
//
//-----------------------------------------------------------------------------

List<Phx::IR::Operand ^> ^ 
IRBuilder::ArgumentList::get()
{
   return argumentLists[0];
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Retrieves the current argument modifier list.
//
// Remarks:
//
//
// Returns:
//
//    The current argument modifier list.
//
//-----------------------------------------------------------------------------

List<ArgumentModifier ^> ^ 
IRBuilder::ArgumentModifierList::get()
{
    return argumentModifierList;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Finds the argument modifier matching the given base operand.
//
// Remarks:
//
//
// Returns:
//
//    The argument modifier matching the given base operand, or nullptr
//    if no argument modifer matches the operand.
//
//-----------------------------------------------------------------------------

ArgumentModifier ^ 
IRBuilder::FindArgumentModifier
(
   Phx::IR::Operand ^ baseOperand
)
{
   for each (ArgumentModifier ^ modifier in argumentModifierList)
   {
      if (modifier->BaseOperand == baseOperand)
      {
         return modifier;
      }
   }
   return nullptr;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Appends a unary operation to the end of the IR stream.
//    The op parameter may be one of PLUS, MINUS, or NOT, as
//    defined in YaccDeclarations.h.
//
// Remarks:
//
//
// Returns:
//
//    The destination operand of the generated IR instruction.
//
//-----------------------------------------------------------------------------

Phx::IR::Operand ^ 
IRBuilder::EmitUnaryOp
(
   Phx::FunctionUnit ^ functionUnit,
   unsigned op, 
   Phx::Types::Type ^ type, 
   Phx::IR::Operand ^ sourceOperand,
   int sourceLineNumber
)
{
   // Get the Phoenix Opcode that matches the operator.
   // For unary plus, emit a no-op.

   Phx::Opcode ^ opcode = nullptr;
   switch (op)
   {
   case PLUS:      opcode = Phx::Common::Opcode::Nop;       break;
   case MINUS:     opcode = Phx::Common::Opcode::Negate;    break;
   case NOT:       opcode = Phx::Common::Opcode::Not;       break;

   default:
      Debug::Assert(false, "Invalid opcode");
      break;
   }

   Phx::IR::ValueInstruction ^ instruction;

   // If the operation is a nop, just return the source operand.

   if (opcode->Equals(Phx::Common::Opcode::Nop))
   {
      return sourceOperand;
   }

   // Otherwise, generate an instruction for the unary operation.

   else
   {
      // If the operand is of enum type, extract the primary 
      // field from the operand. We do this so we can work with 
      // an atomic value; enumerated types are implement as
      // aggregates.

      if (sourceOperand->Type->IsEnumType)
      {
         sourceOperand = ExtractPrimaryEnumField(functionUnit, sourceOperand);
         type = sourceOperand->Type;
      }

      // Create a value instruction for the operation.

      instruction = Phx::IR::ValueInstruction::NewUnaryExpression(
         functionUnit, 
         opcode, 
         type, 
         sourceOperand
      );
   }
 
   // Append the instruction to the IR stream of the function.

   functionUnit->LastInstruction->InsertBefore(instruction);

   // The NOT operation produces a Boolean result.
   
   if (op == NOT)
   {
      // Promote the result to a Boolean operand and return.

      return PromoteToBoolean(
         functionUnit,
         instruction->DestinationOperand, 
         sourceLineNumber
      );
   }

   return instruction->DestinationOperand;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Appends a binary arithmetic operation to the end of the IR stream.
//    This method supports set operations (union (PLUS), 
//    difference (MINUS), and intersection (STAR)) as well 
//    as the following operators defined in YaccDeclarations.h:
//    PLUS, MINUS, STAR, SLASH, DIV, MOD, AND, OR
//
// Remarks:
//
//
// Returns:
//
//    The destination operand of the generated IR instruction.
//
//-----------------------------------------------------------------------------

Phx::IR::Operand ^ 
IRBuilder::EmitBinaryArithmeticOp
(
   Phx::FunctionUnit ^ functionUnit,
   unsigned op, 
   Phx::Types::Type ^ type, 
   Phx::IR::Operand ^ sourceOperand1, 
   Phx::IR::Operand ^ sourceOperand2,
   int sourceLineNumber
)
{
   // Check for special cases for set types.

   if (TypeBuilder::IsSetType(sourceOperand1->Type) || 
       TypeBuilder::IsSetType(sourceOperand2->Type))
   {
      // Set operations are performed through the runtime.
      // Map the set operator to the appropriate runtime function.

      String ^ functionName;

      switch (op)
      {
         case PLUS:
            functionName = "set_union";
            break;
         case MINUS:
            functionName = "set_difference";
            break;
         case STAR:
            functionName = "set_intersection";
            break;

         default:
            Debug::Assert(false, "Invalid opcode");
            break;
      }    

      // Create an argument list for the function call and append
      // the arguments.

      List<Phx::IR::Operand ^> ^ arguments = 
         gcnew List<Phx::IR::Operand ^>();

      //set1
      arguments->Add(sourceOperand1);

      //set2
      arguments->Add(sourceOperand2);      

      //file_index
      arguments->Add(Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) ModuleBuilder::SourceFileIndex
         )
      );

      //source_line_number
      arguments->Add(Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->Int32Type,
            (int) sourceLineNumber
         )
      );

      // Generate the function call through the runtime.

      Phx::IR::Instruction ^ callInstruction = 
         ModuleBuilder::Runtime->CallSetFunction(
            functionUnit,
            functionName,
            arguments,
            sourceLineNumber
         );

      // Assign the new set to a temporary operand so 
      // we can submit it for set release when the function exits.

      Phx::Symbols::Symbol ^ setSymbol = 
         ModuleBuilder::AddInternalVariableDeclarationSymbol(
            functionUnit,
            sourceOperand1->Type,
            Phx::Symbols::StorageClass::Auto,
            sourceLineNumber
         );

      ModuleBuilder::AddSetDefinitionSymbol(
         functionUnit,
         setSymbol,
         setSymbol->Type
      );

      // The runtime will produce an opaque set type. 
      // Convert it to the runtime type by using a Resolve
      // object.

      Phx::IR::Operand ^ destinationOperand = 
         Phx::IR::VariableOperand::New(
            functionUnit,
            setSymbol->Type,
            setSymbol
         );

      Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

      Phx::IR::Operand ^ sourceOperand = resolve->Convert(
         callInstruction->DestinationOperand,
         functionUnit->LastInstruction->Previous,
         setSymbol->Type
      );

      // Assign the result to the destination operand.

      Phx::IR::Instruction ^ assignInstruction = 
         Phx::IR::ValueInstruction::NewUnary(
            functionUnit,
            Phx::Common::Opcode::Assign,
            destinationOperand,
            sourceOperand
         );
     
      functionUnit->LastInstruction->InsertBefore(assignInstruction);

      // Now mark the created set for release after the function exits.

      ModuleBuilder::AddSetSymbolToReleaseList(
         functionUnit,
         setSymbol
         );

      // Return the destination operand.

      return destinationOperand;
   }

   // Get the Phoenix Opcode that matches the operator.

   Phx::Opcode ^ opcode = nullptr;
   switch (op)
   {
      case PLUS:        opcode = Phx::Common::Opcode::Add;        break;
      case MINUS:       opcode = Phx::Common::Opcode::Subtract;   break;
      case STAR:        opcode = Phx::Common::Opcode::Multiply;   break;
      case SLASH:       
      case DIV:         opcode = Phx::Common::Opcode::Divide;     break;
      case MOD:         opcode = Phx::Common::Opcode::Nop;        break;
      case AND:         opcode = Phx::Common::Opcode::BitAnd;     break;
      case OR:          opcode = Phx::Common::Opcode::BitOr;      break;

      default:
         Debug::Assert(false, "Invalid opcode");
         break;
   }

   // Check for unparameterized procedure or function calls, for example:
   // x := f + f;
   // where f is of function type.

   // Check first operand.

   if (sourceOperand1->Type->IsFunctionType)
   {
      // Emit the function call.

      Phx::IR::Instruction ^ callInstruction = 
         IRBuilder::EmitCall(
            functionUnit,      
            sourceOperand1->Symbol->NameString,
            gcnew List<Phx::IR::Operand ^>(),
            sourceLineNumber
         );

      // Return now if an error occurred.

      if (callInstruction == nullptr)
         return nullptr;

      // The source operand is now the result of the function call.

      sourceOperand1 = callInstruction->DestinationOperand;
   }

   // Check second operand.

   if (sourceOperand2->Type->IsFunctionType)
   {
      // Emit the function call.

      Phx::IR::Instruction ^ callInstruction = 
         IRBuilder::EmitCall(
            functionUnit,      
            sourceOperand2->Symbol->NameString,
            gcnew List<Phx::IR::Operand ^>(),
            sourceLineNumber
         );

      // Return now if an error occurred.

      if (callInstruction == nullptr)
         return nullptr;

      // The source operand is now the result of the function call.

      sourceOperand2 = callInstruction->DestinationOperand;
   }

   // If either source operand is of enum type, we need to extract
   // the primary fields from the operand. Because we already
   // performed type checking, the enum type should be the same
   // for both operands.

   Phx::Types::Type ^ promotedEnumType = nullptr;

   if (sourceOperand1->Type->IsEnumType)
   {
      promotedEnumType = sourceOperand1->Type;

      sourceOperand1 = ExtractPrimaryEnumField(
         functionUnit,
         sourceOperand1
      );
   }
   if (sourceOperand2->Type->IsEnumType)
   {
      promotedEnumType = sourceOperand2->Type;

      sourceOperand2 = ExtractPrimaryEnumField(
         functionUnit,
         sourceOperand2
      );
   }
   if (promotedEnumType)
   {
      // The operation is now on integer type (the 
      // primary field of an enumerated type).

      type = functionUnit->TypeTable->Int32Type;
   }
  
   // Fixup any mis-matching operand types.
   // Because we already performed type checking, the conversion will be legal.
   // This is especially common when assigning real to integer and vice-versa.
   
   if (! sourceOperand1->Type->Equals(sourceOperand2->Type))
   {
      // Create a Resolve object to handle the conversion.

      Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

      // Get the expected result type of the operation. For example,
      // i DIV j always produces an integer value; i / j always produces
      // a real value.

      Phx::Types::Type ^ resultType = TypeBuilder::GetBinaryArithmeticResult(
         sourceOperand1->Type, 
         sourceOperand2->Type, 
         op
      );

      // Perform the necessary conversion.

      if (! sourceOperand1->Type->Equals(resultType))
         sourceOperand1 = resolve->Convert(
            sourceOperand1, 
            functionUnit->LastInstruction->Previous, 
            resultType
         );

      if (! sourceOperand2->Type->Equals(resultType))
         sourceOperand2 = resolve->Convert(
            sourceOperand2, 
            functionUnit->LastInstruction->Previous, 
            resultType
         );
   }
  
   // For AND or OR operations, generate a temporary condition code.

   if (op == AND || op == OR)
   {
      type = functionUnit->TypeTable->ConditionType;
   }

   // For the slash operator, ensure both operands are of real type.

   if (op == SLASH)
   {
      Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

      Phx::Types::Type ^ realType = 
         TypeBuilder::GetTargetType(NativeType::Real);

      if (! sourceOperand1->Type->Equals(realType))
         sourceOperand1 = resolve->Convert(
            sourceOperand1, 
            functionUnit->LastInstruction->Previous, 
            realType
         );

      if (! sourceOperand2->Type->Equals(realType))
         sourceOperand2 = resolve->Convert(
            sourceOperand2, 
            functionUnit->LastInstruction->Previous, 
            realType
         );
   }

   // Generate the binary arithmetic instruction.

   Phx::IR::Instruction ^ instruction;

   // For the MOD operator, call the 'mod' run-time function.

   if (op == MOD)
   {  
      List<Phx::IR::Operand ^> ^ arguments = 
         gcnew List<Phx::IR::Operand ^>();

      arguments->Add(sourceOperand1);
      arguments->Add(sourceOperand2);

      instruction = ModuleBuilder::Runtime->CallMathFunction(
         functionUnit,
         "mod",
         arguments,
         sourceLineNumber
      );
   }

   // Otherwise, create a value instruction for the operation.

   else
   {
      instruction = 
         Phx::IR::ValueInstruction::NewBinaryExpression(
            functionUnit, 
            opcode, 
            type, 
            sourceOperand1, 
            sourceOperand2
         );

      functionUnit->LastInstruction->InsertBefore(instruction);
   }
 
   // For AND or OR operations, convert result to Boolean.

   if (op == AND || op == OR)
   {
      return PromoteToBoolean(
         functionUnit,
         instruction->DestinationOperand, 
         sourceLineNumber
      );
   }

   // If we performed the operation on enumerated values, 
   // convert the result to the original enum type.

   if (promotedEnumType)
   {
      return PromoteToEnum(
         functionUnit,
         promotedEnumType,
         instruction->DestinationOperand, 
         sourceLineNumber
      );
   }

   // Otherwise, just return the result of the operation.
   
   return instruction->DestinationOperand;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Appends a binary relational operation to the end of the IR stream.
//    This method supports the set operations set membership (IN), 
//    set equlity (EQUAL/NOTEQUAL), subset of (LE), and superset of (GE).
//    This method also supports compare operations on strings, as well 
//    as the following operators defined in YaccDeclarations.h:
//    EQUAL, NOTEQUAL, LT, GT, LE, GE
//
// Remarks:
//
//
// Returns:
//
//    The destination operand of the generated IR instruction.
//
//-----------------------------------------------------------------------------

Phx::IR::Operand ^ 
IRBuilder::EmitBinaryRelationalOp
(
   Phx::FunctionUnit ^ functionUnit,
   unsigned op, 
   Phx::IR::Operand ^ sourceOperand1, 
   Phx::IR::Operand ^ sourceOperand2,  
   int sourceLineNumber
)
{
   // Check for special cases for set types.

   // Check for the set membership (IN) operation.

   if (op == IN)
   {
      List<Phx::IR::Operand ^> ^ arguments = 
         gcnew List<Phx::IR::Operand ^>();
      
      // Ensure the arguments are compatible.
     
      if (! sourceOperand2->Type->AsPointerType->
         ReferentType->Equals(sourceOperand1->Type))
      {
         Output::ReportError(
            sourceLineNumber,
            Error::IncompatibleSetTypesForMembership
         );

         return nullptr;
      }

      // The set_membership function expects the set parameter 
      // before the value parameter, so swap the operands.

      Phx::IR::Operand ^ swapOperand = sourceOperand1;
      sourceOperand1                 = sourceOperand2;
      sourceOperand2                 = swapOperand;
     
      // If the source operand we're testing for membership
      // is of enum type, extract the primary integer field
      // from the operand (the set_membership runtime function
      // expect an integer value). 

      if (sourceOperand2->Type->IsEnumType)
      {
         sourceOperand2 = IRBuilder::ExtractPrimaryEnumField(
            functionUnit,
            sourceOperand2
         );            
      }

      // Otherwise, if the source operand we're testing for membership
      // is smaller than integer type (such as char), widen 
      // the value to the size of integer by using a Resolve object.

      else if (sourceOperand2->Type->ByteSize < 
         functionUnit->TypeTable->Int32Type->ByteSize)
      {
         Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

         sourceOperand2 = resolve->Convert(
            sourceOperand2,
            functionUnit->LastInstruction->Previous,
            functionUnit->TypeTable->Int32Type
         );
      }
      
      // Append the arguments and generate the runtime 
      // function call.

      //set
      arguments->Add(sourceOperand1);
      //value
      arguments->Add(sourceOperand2);

      Phx::IR::Instruction ^ callSetInstruction = 
         ModuleBuilder::Runtime->CallSetFunction(
            functionUnit,
            "set_membership",
            arguments,
            sourceLineNumber
         );

      // The set_membership function returns non-zero to indicate
      // set membership. Because the IN operation returns Boolean,
      // continue this method as normal, but exchange the first
      // source operand for the result of the runtime function call
      // and the second source operand for the immediate value 0.
      // We also change the operator to NOTEQUAL to produce a 
      // Boolean result.

      sourceOperand1 = callSetInstruction->DestinationOperand;

      sourceOperand2 = Phx::IR::ImmediateOperand::New(
         functionUnit,
         sourceOperand1->Type,
         (int) 0
      );
      
      op = NOTEQUAL;
   }

   // Check for set equlity, subset of, and superset of operations.

   else if (TypeBuilder::IsSetType(sourceOperand1->Type) && 
            TypeBuilder::IsSetType(sourceOperand2->Type))
   {
      // Map the set operator to the appropriate runtime function.

      String ^ functionName;

      switch (op)
      {
         case EQUAL:
         case NOTEQUAL:
            functionName = "set_equality";
            break;
         case LE:
            functionName = "is_subset_of";
            op = EQUAL;
            break;
         case GE:
            functionName = "is_superset_of";
            op = EQUAL;
            break;

         default:
            Debug::Assert(false, "Invalid opcode");
            break;
      }

      // Create an argument list for the function call and append
      // the arguments.

      List<Phx::IR::Operand ^> ^ arguments = gcnew List<Phx::IR::Operand ^>();

      //set1
      arguments->Add(sourceOperand1);

      //set2
      arguments->Add(sourceOperand2);      
      
      Phx::IR::Instruction ^ callInstruction = 
         ModuleBuilder::Runtime->CallSetFunction(
            functionUnit,
            functionName,
            arguments,
            sourceLineNumber
         );

      // The runtime function returns non-zero to indicate
      // 'true'. Because we want to return Boolean,
      // continue this method as normal, but exchange the first
      // source operand for the result of the runtime function call
      // and the second source operand for the immediate value 1.
      
      sourceOperand1 = callInstruction->DestinationOperand;

      sourceOperand2 = Phx::IR::ImmediateOperand::New(
         functionUnit,
         sourceOperand1->Type,
         (int) 1
      );
   }
   
   // Check for string types.

   else if (TypeBuilder::IsStringType(sourceOperand1->Type) && 
            TypeBuilder::IsStringType(sourceOperand2->Type))
   {
      // Create an argument list for the function call and append
      // the arguments.

      List<Phx::IR::Operand ^> ^ arguments = 
         gcnew List<Phx::IR::Operand ^>();

      // Create an immediate operand containing the static 
      // length of the strings (both strings must be
      // the same length).

      Phx::IR::Operand ^ destinationSizeOperand = 
         Phx::IR::ImmediateOperand::New(
            functionUnit,
            functionUnit->TypeTable->UInt32Type,
            TypeBuilder::GetStringLength(sourceOperand1->Symbol, 
                                         sourceOperand1->Type)
         );


      //string1
      arguments->Add(sourceOperand1);

      //string2
      arguments->Add(sourceOperand2);

      //count
      arguments->Add(destinationSizeOperand);

      Phx::IR::Instruction ^ callInstruction = 
         ModuleBuilder::Runtime->CallStringFunction(
            functionUnit,
            "_strncmp",
            arguments,
            sourceLineNumber
         );

      // We can now perform equal, not equal, greater than,
      // less than, etc. By comparing the result of the call
      // with the immediate value 0.

      sourceOperand1 = callInstruction->DestinationOperand;

      sourceOperand2 = Phx::IR::ImmediateOperand::New(
         functionUnit,
         functionUnit->TypeTable->Int32Type,
         (int) 0
      );
   }

   // Translate the numeric condition code defined in the grammar
   // to a Phx::ConditionCode object.

   Phx::ConditionCode conditionCode = Utility::GetConditionCode(op);
      
   // Fixup any mis-matching operand types.
   // Because we already performed type checking, the conversion will be legal.
   // This is especially common when assigning real to integer and vice-versa.
   
   if (! sourceOperand1->Type->Equals(sourceOperand2->Type))
   {
      // Create a Resolve object to handle the conversion.

      Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

      // Test for conversion from integer to real.

      if ( (sourceOperand1->Type->Equals(
         TypeBuilder::GetTargetType(NativeType::Integer)) &&
            sourceOperand2->Type->Equals(
         TypeBuilder::GetTargetType(NativeType::Real))) )
      {
         sourceOperand1 = resolve->Convert(
            sourceOperand1, 
            functionUnit->LastInstruction->Previous, 
            sourceOperand2->Type
         );
      }
      else if 
         ( (sourceOperand2->Type->Equals(
         TypeBuilder::GetTargetType(NativeType::Integer)) &&
            sourceOperand1->Type->Equals(
         TypeBuilder::GetTargetType(NativeType::Real))) )
      {         
         sourceOperand2 = resolve->Convert(
            sourceOperand2, 
            functionUnit->LastInstruction->Previous, 
            sourceOperand1->Type
         );         
      }
   }

   // If either operand is of an enumerated type, extract its
   // underlying integer field.

   if (sourceOperand1->Type->IsEnumType)
   {
      sourceOperand1 = ExtractPrimaryEnumField(
         functionUnit,
         sourceOperand1
      );
   }
   if (sourceOperand2->Type->IsEnumType)
   {
      sourceOperand2 = ExtractPrimaryEnumField(
         functionUnit,
         sourceOperand2
      );
   }

   // Translate the condition code to its floating-point equivalent
   // if we're performing an operation on the real type.

   if (sourceOperand1->Type->Equals(
      TypeBuilder::GetTargetType(NativeType::Real)))
   {
      Debug::Assert(sourceOperand2->Type->Equals(sourceOperand1->Type));

      conditionCode = Utility::TranslateToFloatConditionCode(conditionCode);
   }
    
   // Create an expression temporary to hold the result of the 
   // compare.

   Phx::IR::Operand ^ destination = 
      Phx::IR::VariableOperand::NewExpressionTemporary(
         functionUnit, 
         functionUnit->TypeTable->ConditionType
      );

   // Create a compare instruction and append it to the end of the 
   // IR stream.

   Phx::IR::Instruction ^ compareInstruction = 
      Phx::IR::CompareInstruction::New(
         functionUnit,
         Phx::Common::Opcode::Compare,
         (int) conditionCode,
         destination,
         sourceOperand1,
         sourceOperand2
      );
   
   functionUnit->LastInstruction->InsertBefore(compareInstruction);
   Debug::Assert(destination == compareInstruction->DestinationOperand);

   // Promote the result to a Boolean operand and return.

   return PromoteToBoolean(
      functionUnit,
      destination, 
      sourceLineNumber
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a conditional branch statement and appends it
//    to the end of the IR stream.
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
IRBuilder::EmitConditionalBranch
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::IR::Operand ^ conditionOperand,
   Phx::IR::LabelInstruction ^ trueLabelInstruction,
   Phx::IR::LabelInstruction ^ falseLabelInstruction
)
{
   // Because Phoenix branch instructions expect an integer
   // condition operand, we need to extract the primary
   // integer value field from Boolean operands.

   Phx::Types::Type ^ booleanType = 
      TypeBuilder::GetTargetType(NativeType::Boolean);

   if (conditionOperand->Type->Equals(booleanType))
   {
      conditionOperand = ExtractPrimaryEnumField(
         functionUnit,
         conditionOperand
       );      
   }

   // Generate a conditional branch instruction and 
   // append it to the instruction stream.

   Phx::IR::Instruction ^ branchInstruction =
      Phx::IR::BranchInstruction::New(
         functionUnit,
         Phx::Common::Opcode::ConditionalBranch,
         (int) Phx::ConditionCode::True,
         conditionOperand,
         trueLabelInstruction,
         falseLabelInstruction
      );

   functionUnit->LastInstruction->InsertBefore(branchInstruction);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Appends the given label instruction to the end of the IR stream.
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
IRBuilder::EmitLabel
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::IR::LabelInstruction ^ labelInstruction
)
{
   functionUnit->LastInstruction->InsertBefore(labelInstruction);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Creates an unconditional branch instruction to the given label target
//    and appends it after the given instruction.
//
// Remarks:
//
//
// Returns:
//
//    The created unconditional branch instruction.
//
//-----------------------------------------------------------------------------

Phx::IR::BranchInstruction ^ 
IRBuilder::EmitGoto
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::IR::Instruction ^ insertAfterInstruction,
   Phx::IR::LabelInstruction ^ labelInstruction,
   int sourceLineNumber
)
{
   Phx::IR::BranchInstruction ^ gotoInstruction = 
      Phx::IR::BranchInstruction::New(
         functionUnit,
         Phx::Common::Opcode::Goto,
         labelInstruction
      );
   
   insertAfterInstruction->InsertAfter(gotoInstruction);
   return gotoInstruction;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Creates an unconditional branch instruction to the given label target
//    and appends it to the end of the IR stream.
//
// Remarks:
//
//
// Returns:
//
//    The created unconditional branch instruction.
//
//-----------------------------------------------------------------------------

Phx::IR::BranchInstruction ^ 
IRBuilder::EmitGoto
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::IR::LabelInstruction ^ labelInstruction,
   int sourceLineNumber
)
{
   return EmitGoto(
      functionUnit, 
      functionUnit->LastInstruction->Previous, 
      labelInstruction, 
      sourceLineNumber
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Creates an assignment instruction between the given source and 
//    destination operands and appends it to the end of the IR stream.
//
// Remarks:
//
//
// Returns:
//
//    The destination operand of the created assignment instruction.
//
//-----------------------------------------------------------------------------

Phx::IR::Operand ^
IRBuilder::EmitAssign
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::IR::Operand ^ destinationOperand,    
   Phx::IR::Operand ^ sourceOperand,
   int sourceLineNumber   
)
{
   Phx::Types::Type ^ integerType = 
      TypeBuilder::GetTargetType(NativeType::Integer);

   Phx::Types::Type ^ realType = 
      TypeBuilder::GetTargetType(NativeType::Real);

   // Fixup any mis-matching operand types.
   // Because we already performed type checking, the conversion will be legal.
   // We allow implicit conversion from integer to real and vice-versa.
   
   if (! destinationOperand->Type->Equals(sourceOperand->Type))
   {
      if ( (sourceOperand->Type->Equals(integerType) && 
            destinationOperand->Type->Equals(realType)) ||
           
           (destinationOperand->Type->Equals(integerType) && 
            sourceOperand->Type->Equals(realType)) 
         )
      {
         // Convert the source operand type to the destination operand type.

         sourceOperand = ConvertOperandType(
            functionUnit, 
            sourceOperand, 
            destinationOperand->Type
         );
      }
   }

   // For operands of set type, if one of the operands is of the 
   // 'empty set' type, convert it to the type of the other set operand
   // by using a Resolve object.

   if (TypeBuilder::IsSetType(sourceOperand->Type))
   {
      Debug::Assert(TypeBuilder::IsSetType(destinationOperand->Type));

      Phx::Types::Type ^ epsilonType = 
         TypeBuilder::GetTargetType(NativeType::Epsilon);

      if (sourceOperand->Equals(epsilonType) || 
          destinationOperand->Equals(epsilonType))
      {
         Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

         if (destinationOperand->Type->Equals(epsilonType))
            destinationOperand = resolve->Convert(
               destinationOperand, 
               functionUnit->LastInstruction->Previous, 
               sourceOperand->Type
            );

         else
            sourceOperand = resolve->Convert(
               sourceOperand, 
               functionUnit->LastInstruction->Previous, 
               destinationOperand->Type
            );
      }          
   }

   // For array/string types, we can perform a straight assignment 
   // (shallow copy) if both operands are string pointers. 
   // Otherwise, we want to perform a deep copy by using the 
   // memcpy runtime function.

   if (sourceOperand->Type->IsUnmanagedArrayType ||
       TypeBuilder::IsStringType(sourceOperand->Type))
   {
      //Debug::Assert(TypeBuilder::IsStringType(destinationOperand->Type));

      if (destinationOperand->Type->IsPointerType)
      {
         // Pointer assignment; fall-through.

         Debug::Assert(sourceOperand->Type->IsPointerType);
      }
      else
      {

         List<Phx::IR::Operand ^> ^ arguments = 
            gcnew List<Phx::IR::Operand ^>();

         // Create an immediate operand containing the static 
         // length of the strings (both strings must be
         // the same length).

         Phx::IR::Operand ^ destinationSizeOperand;

         if (TypeBuilder::IsStringType(sourceOperand->Type))
         {
            destinationSizeOperand = Phx::IR::ImmediateOperand::New(
               functionUnit,
               functionUnit->TypeTable->UInt32Type,
               TypeBuilder::GetStringLength(destinationOperand->Symbol, 
                                            destinationOperand->Type)
            );
         }
         else
         {
            destinationSizeOperand = Phx::IR::ImmediateOperand::New(
               functionUnit,
               functionUnit->TypeTable->UInt32Type,
               sourceOperand->Type->AsUnmanagedArrayType->ByteSize
            );
         }

         //dest
         arguments->Add(destinationOperand);

         //src
         arguments->Add(sourceOperand);

         //count
         arguments->Add(destinationSizeOperand);

         Phx::IR::Instruction ^ callInstruction =
            ModuleBuilder::Runtime->CallStringFunction(
               functionUnit,
               "_memcpy",
               arguments,
               sourceLineNumber
            );

         // Return the result of the call instruction, or nullptr
         // if an error occurred.

         if (callInstruction)
            return callInstruction->DestinationOperand;
         return nullptr;
      }
   }

   // Check for unparameterized procedure or function calls, for example:
   // x := f;
   // In this case, generate the function call and reassign the source
   // operand to the result of the call.

   if (sourceOperand->Type->IsFunctionType)
   {
      Phx::IR::Instruction ^ callInstruction = 
         IRBuilder::EmitCall(
            functionUnit,      
            sourceOperand->Symbol->NameString,
            gcnew List<Phx::IR::Operand ^>(),
            sourceLineNumber
         );

      if (callInstruction == nullptr)
      {
         // An error occurred; return.

         return nullptr;
      }

      sourceOperand = callInstruction->DestinationOperand;
   }    
       
   // Create an assign instruction and append it 
   // to the IR stream.

   Phx::IR::Instruction ^ assignInstruction = 
      Phx::IR::ValueInstruction::NewUnary(
         functionUnit, 
         Phx::Common::Opcode::Assign,
         destinationOperand,
         sourceOperand
      );

   functionUnit->LastInstruction->InsertBefore(assignInstruction);

   // Perform bounds checking if the destination operand is of 
   // enum or subrange type. 

   if (TypeBuilder::IsSubrangeType(destinationOperand->Type))
   {
      // Perform static bounds checking if the source 
      // operand is an immediate operand.

      if (sourceOperand->IsImmediateOperand)
      {
         TypeBuilder::StaticCheckRange(
            functionUnit,
            destinationOperand->Type,
            sourceOperand->AsImmediateOperand,
            sourceLineNumber
         );
      }

      // Otherwise, we'll need to perform runtime bounds checking.

      else
      {
         Phx::Symbols::Symbol ^ destinationSymbol = destinationOperand->Symbol;
         if (destinationSymbol == nullptr && 
             destinationOperand->IsMemoryOperand)
            destinationSymbol = 
               destinationOperand->AsMemoryOperand->BaseOperand->Symbol;

         TypeBuilder::DynamicCheckRange(
            functionUnit,
            destinationOperand->Type,
            destinationOperand,
            sourceLineNumber
         );
      }
   }

   // Return the result of the assignment instruction.

   return assignInstruction->DestinationOperand;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Appends the given switch instruction to the end of the IR stream.
//
// Remarks:
//
//
// Returns:
//
//    The destination operand of the switch instruction.
//
//-----------------------------------------------------------------------------

Phx::IR::Operand ^
IRBuilder::EmitSwitch
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::IR::SwitchInstruction ^ switchInstructon,
   int sourceLineNumber
)
{
   functionUnit->LastInstruction->InsertBefore(switchInstructon);

   return switchInstructon->DestinationOperand;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Validates the argument number and argument types 
//    for the given function type.
//
// Remarks:
//
//    This method will also print an error message if the call is invalid.
//    It also exits after it finds the first error.
//
// Returns:
//
//    true if the call is valid; false otherwise.
//
//-----------------------------------------------------------------------------

bool 
IRBuilder::ValidateCall
(
   String ^ functionName,
   Phx::Types::FunctionType ^ functionType, 
   List<Phx::IR::Operand ^> ^ argumentList,
   int sourceLineNumber
)
{
   // Validate parameter count.

   int expectedArgumentCount = functionType->ParametersForInstruction.Count;
   int actualArgumentCount   = argumentList->Count;

   if (expectedArgumentCount != actualArgumentCount)
   {
      Output::ReportError(
         sourceLineNumber,
         Error::InvalidArgumentCount,
         functionName,
         expectedArgumentCount,
         actualArgumentCount
      );
      return false;
   }

   // Validate parameter types.

   for (int i = 0; i < actualArgumentCount; ++i)
   {
      Phx::Types::Type ^ expectedArgumentType = 
         functionType->Parameters[i].Type;

      Phx::Types::Type ^ actualArgumentType = 
         argumentList[i]->Type;

      if (! expectedArgumentType->Equals(actualArgumentType) &&
          ! TypeBuilder::AreEquivalentTypes(
               expectedArgumentType, 
               actualArgumentType)
         )
      {
         Output::ReportError(
            sourceLineNumber,
            Error::InvalidArgument,
            functionName, 
            TypeBuilder::GetSourceType(expectedArgumentType), i+1
         );
         return false;
      }
   }
   return true;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generates a call to the given procedure or function name
//    and appends it to the end of the IR stream.
//
// Remarks:
//
//
// Returns:
//
//    The created call instruction.
//
//-----------------------------------------------------------------------------

Phx::IR::Instruction ^ 
IRBuilder::EmitCall
(
   Phx::FunctionUnit ^ functionUnit,
   String ^ procedureName,
   List<Phx::IR::Operand ^> ^ argumentList,
   int sourceLineNumber
)
{
   // Get the formal parameter list associated with the procedure.

   // GetFormalParameters returns nullptr if the 
   // provided procedure name refers to an external library 
   // call or a function/procedure parameter.

   array<FormalParameter ^> ^ formalParameters = 
      ModuleBuilder::GetFormalParameters(procedureName);

   Phx::IR::Instruction ^ callInstruction = nullptr;

   // Defer processing to the runtime if the procedure is a
   // runtime call. The Runtime helper class may need to
   // alter the argument list.

   if (ModuleBuilder::Runtime->IsRuntimeFunctionName(procedureName))
   {
      callInstruction = 
         ModuleBuilder::Runtime->BuildRuntimeFunctionCall(
            functionUnit,
            procedureName,
            argumentList,
            sourceLineNumber
         );      
   }
   else
   {
      // This is a call to program-defined procedure.
      // First, verify the function symbol exists.
   
      Phx::Symbols::Symbol ^ functionSymbol = 
         functionUnit->ParentModuleUnit->SymbolTable->NameMap->Lookup(
            Phx::Name::New(
               functionUnit->ParentModuleUnit->Lifetime,
               procedureName
            )
         );

      if (functionSymbol && functionSymbol->IsFunctionSymbol)
      {
         // Check for 'callability' of the procedure or function.
         // A function is non-callable if it exists but cannot be
         // reached from the calling scope.
            
         if (! ModuleBuilder::IsCallable(functionSymbol->AsFunctionSymbol))
         {
            String ^ functionOrProcedure = 
               functionSymbol->Type->AsFunctionType->HasVoidReturn ?
               "procedure" : "function";
            
            Output::ReportError(
               sourceLineNumber,
               Error::FunctionUnreachable,
               functionSymbol->NameString,
               functionOrProcedure
            );

            // Continue processing so we can catch further errors.
         }
      }

      if (functionSymbol == nullptr)
      {
         functionSymbol = ModuleBuilder::LookupSymbol(
            functionUnit, 
            procedureName
         );
      }

      // Process the argument list and check for invalid arguments,
      // such as constant values being passed by reference. Convert
      // valid operands to address operands.

      int n = 0;
      bool error = false;
      for each (Phx::IR::Operand ^ argument in argumentList)
      {                  
         // If the argument is variable (e.g. pass by reference), change
         // the operand to an address operand.

         if (formalParameters && n < formalParameters->Length &&
             formalParameters[n]->ParameterType != FormalParameterType::Value)
         {
            // Each argument must be symbolic and not marked as
            // a local constant to be passed by reference.

            if (!argument->IsMemoryOperand && argument->Symbol == nullptr)
            {
               error = true;
            }
            else if (ModuleBuilder::IsLocalConstant(
               functionUnit, argument->Symbol))
            {
               error = true;
            }

            if (error)
            {
               Output::ReportError(
                  sourceLineNumber,
                  Error::ConstantVarParameter,
                  procedureName, 
                  formalParameters[n]->Name
               );
               n++;
               continue;
            }

            argument->ChangeToAddress();
         }
         n++;
      }

      if (error)
         return callInstruction;

      // Get the function type associated with the function symbol.

      Phx::Types::FunctionType ^ functionType = nullptr;
      if (functionSymbol)
      {
         if (functionSymbol->Type->IsFunctionType)
         {
            // Direct function type.

            functionType = functionSymbol->Type->AsFunctionType;
         }
         else if (functionSymbol->Type->IsPointerType &&
                  functionSymbol->Type->AsPointerType->
                     ReferentType->IsFunctionType)
         {
            // Pointer (referece) to function type.

            functionType = functionSymbol->Type->AsPointerType->
               ReferentType->AsFunctionType;
         }
      }

      if (functionType)
      {
         // Validate the call. If it encounters any errors, it will report
         // them. In case of failure, return now.

         if (! ValidateCall(
            procedureName,
            functionType, 
            argumentList,
            sourceLineNumber))
         {
            return callInstruction;
         }
         
         // Generate the call instruction.

         // If the symbol is a function symbol, then we can make
         // a direct call to the function.
         if (functionSymbol->IsFunctionSymbol)
         {
            callInstruction = Phx::IR::CallInstruction::New(
               functionUnit,
               Phx::Common::Opcode::Call,
               functionSymbol->AsFunctionSymbol
            );
         }
         // Otherwise, this is an indirect function call. 
         // First create a variable operand by using the symbol,
         // and then create the call instruction from that
         // operand.
         else         
         {
            Debug::Assert(functionSymbol->IsFunction);

            Phx::IR::VariableOperand ^ functionOperand = 
               Phx::IR::VariableOperand::New(
                  functionUnit,
                  functionSymbol->Type,
                  functionSymbol
               );

            callInstruction = Phx::IR::CallInstruction::New(
               functionUnit,
               Phx::Common::Opcode::Call,
               functionOperand
            );
         }

         // Loop through the argument list again, this time convert
         // mismatching types (such as implicit conversions between integer
         // and real), and then append the final argument to the call
         // instruction's source list.

         int n = 0;
         for each (Phx::IR::Operand ^ argument in argumentList)
         {
            Phx::Types::Type ^ expectedArgumentType = 
               functionType->Parameters[n].Type;

            Phx::Types::Type ^ actualArgumentType = 
               argumentList[n]->Type;

            if (! expectedArgumentType->Equals(actualArgumentType))
            {
               // Resolve the argument type mismatch.

               argument = ConvertOperandType(
                  functionUnit,
                  argument,
                  expectedArgumentType
               );  
            }
            
            // Append the argument to the source list of the instruction.
            callInstruction->AppendSource(argument);

            n++;
         }

         // For functions, create an expression temporary to hold
         // the result (destination) of the function call.

         if (! functionType->ReturnType->IsVoid)
         {
            Phx::IR::Operand ^ result = 
               Phx::IR::VariableOperand::NewExpressionTemporary(
                  functionUnit,
                  functionType->ReturnType
               );

            callInstruction->AppendDestination(result);
         }
   
         // Finally, append the instruction to the IR stream.

         functionUnit->LastInstruction->InsertBefore(callInstruction);
      }
      else
      {
         // Could not resolve function symbol/type.

         Output::ReportError(
            sourceLineNumber,
            Error::UndeclaredFunction,
            procedureName
         );      
      }
   }

   // Return the call instruction.

   return callInstruction;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Creates a new variable operand from the provided symbol.
//
// Remarks:
//
//    This method also considers the origin of the symbol.
//    If the symbol references a global variable, this method
//    creates a proxy symbol for it.
//
// Returns:
//
//    A new variable operand.
//
//-----------------------------------------------------------------------------

Phx::IR::VariableOperand ^
IRBuilder::CreateVariableOperand
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Symbols::Symbol ^ symbol
)
{
   Debug::Assert(symbol != nullptr);
         
   // If the symbol references a global variable, 
   // create a proxy (NonLocalVariableSymbol) in the
   // symbol table of the function unit.

   if (symbol->IsGlobalVariableSymbol)
   {
      symbol = ModuleBuilder::MakeProxyInFunctionSymbolTable(
         symbol->AsGlobalVariableSymbol,
         functionUnit->SymbolTable
      );
   }  

   return Phx::IR::VariableOperand::New(
      functionUnit, 
      symbol->Type, 
      symbol
   );
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Promotes the provided operand to an operand of
//    the given enum type.
//
// Remarks:
//
//    Because enumerated types are implemented as aggregates,
//    this routine is useful for cases where we need to pass
//    the primary field of the enum value to a runtime function,
//    and then promote the result back to the original enum type.
//
// Returns:
//
//    The promoted enum operand.
//
//-----------------------------------------------------------------------------

Phx::IR::Operand ^ 
IRBuilder::PromoteToEnum
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::Types::Type ^ enumType,
   Phx::IR::Operand ^ operand,
   int sourceLineNumber
)
{
   Debug::Assert(! operand->Type->Equals(enumType));

   // Create a new operand to hold the result.

   Phx::IR::Operand ^ enumOperand = 
      Phx::IR::VariableOperand::NewExpressionTemporary(
         functionUnit,
         enumType
      );
  
   // Load the primary field of the operand into an 
   // expression temporary.

   Phx::IR::Operand ^ fieldAccessOperand = 
      Phx::IR::VariableOperand::NewExpressionTemporary(
         functionUnit,
         enumOperand->Field->Next,
         enumOperand->Id
      );
    
   // Assign into the primary field of the enum operand.

   EmitAssign(
      functionUnit,
      fieldAccessOperand,
      operand,
      sourceLineNumber
   );
   
   return enumOperand;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Promotes the provided operand to an operand of
//    the Boolean enum type.
//
// Remarks:
//
//    This method is conceptually similar to PromoteToEnum;
//    however, we want to allow runtime functions to return 
//    non-zero for 'true' and zero for 'false'. This method
//    creates a conditional branch that assigns the result
//    to the global 'false' value if the value is zero;
//    otherwise it assigns the result to the global 'true' value.
//
//    This implementation is provided for instructional
//    purposes only. Techniques such performing two logical
//    Not instructions should normalize the result to [0..1]
//    without introducing a branch.
//
// Returns:
//
//    The promoted enum operand.
//
//-----------------------------------------------------------------------------

Phx::IR::Operand ^
IRBuilder::PromoteToBoolean
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::IR::Operand ^ operand,
   int sourceLineNumber
)
{   
   Phx::Types::Type ^ booleanType = 
      TypeBuilder::GetTargetType("Boolean");

   // Get the global 'true' and 'false' symbols.
   // If they have not yet been proxied into the provided
   // function unit, do so now.

   Phx::Symbols::Symbol ^ trueSymbol = 
      ModuleBuilder::LookupSymbol(functionUnit, "true");

   if (trueSymbol->IsGlobalVariableSymbol)
   {
      trueSymbol = ModuleBuilder::MakeProxyInFunctionSymbolTable(
         trueSymbol->AsGlobalVariableSymbol, 
         functionUnit->SymbolTable
      );
   }
   
   Phx::Symbols::Symbol ^ falseSymbol = 
      ModuleBuilder::LookupSymbol(functionUnit, "false");

   if (falseSymbol->IsGlobalVariableSymbol)
   {
      falseSymbol = ModuleBuilder::MakeProxyInFunctionSymbolTable(
         falseSymbol->AsGlobalVariableSymbol, 
         functionUnit->SymbolTable
      );
   }

   // Generate operands for the true and false symbols.

   Phx::IR::VariableOperand ^ trueOperand = 
      Phx::IR::VariableOperand::New(
         functionUnit,
         booleanType,
         trueSymbol
      );

   Phx::IR::VariableOperand ^ falseOperand = 
      Phx::IR::VariableOperand::New(
         functionUnit,
         booleanType,
         falseSymbol
      );

   // Create an expression temporary to hold the result.

   Phx::IR::VariableOperand ^ resultOperand = 
      Phx::IR::VariableOperand::NewExpressionTemporary(
         functionUnit,
         booleanType
      );

   // Create label instructions for the true and false
   // arms of the branch. 

   Phx::IR::LabelInstruction ^ trueLabelInstruction  = 
      Phx::IR::LabelInstruction::New(functionUnit);
   
   Phx::IR::LabelInstruction ^ falseLabelInstruction = 
      Phx::IR::LabelInstruction::New(functionUnit);
  
   // Create a common target for both branches to meet at.

   Phx::IR::LabelInstruction ^ endLabelInstruction = 
      Phx::IR::LabelInstruction::New(functionUnit);

   // Emit the conditional branch statement.

   EmitConditionalBranch(
      functionUnit,
      operand,
      trueLabelInstruction,
      falseLabelInstruction
   );

   // Emit the label and assignment code for the true arm (this
   // includes branching to the common end point).

   EmitLabel(functionUnit, trueLabelInstruction);

   EmitAssign(
      functionUnit,
      resultOperand,
      trueOperand,
      sourceLineNumber
   );

   EmitGoto(functionUnit, endLabelInstruction, sourceLineNumber);

   // Emit the label and assignment code for the false arm (this
   // includes branching to the common end point).

   EmitLabel(functionUnit, falseLabelInstruction);

   EmitAssign(
      functionUnit,
      resultOperand,
      falseOperand,
      sourceLineNumber
   );

   EmitGoto(functionUnit, endLabelInstruction, sourceLineNumber);

   // Emit the final common jump target for both branches.

   EmitLabel(functionUnit, endLabelInstruction);

   // Return the resulting Boolean operand.

   return resultOperand;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Extracts the primary field from the given operand.
//
// Remarks:
//
//    Call this method to extract the primary integer field
//    from an operand. Enumerations are implemented as an aggregate
//    type with a single integer field.
//
// Returns:
//
//    The extracted field operand.
//
//-----------------------------------------------------------------------------

Phx::IR::Operand ^ 
IRBuilder::ExtractPrimaryEnumField
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::IR::Operand ^ enumOperand
)
{
   // If the existing operand is an expression temporary,
   // create a new expression temporary with the existing
   // identifier.

   if (enumOperand->IsTemporary)
   {
      enumOperand = Phx::IR::VariableOperand::NewExpressionTemporary(
         functionUnit,
         enumOperand->Field->Next,
         enumOperand->Id
      );
   }

   // Otherwise, if the operand is symbolic (e.g. a VariableOperand),
   // create a new variable operand with the existing symbol.

   else if (enumOperand->Symbol)
   {
      enumOperand = Phx::IR::VariableOperand::New(
         functionUnit,
         enumOperand->Field->Next,
         enumOperand->Symbol
      );
   }

   // Otherwise, we have a memory operand. Create a new memory operand,
   // using the base operand of the existing one.

   else
   {
      // The item may be a member of an array.
      // Remember the current index operand (it will often be nullptr).

      Phx::IR::VariableOperand ^ indexOperand = enumOperand->IndexOperand;

      enumOperand = Phx::IR::MemoryOperand::New(
         functionUnit,
         enumOperand->Field->Next,
         nullptr,
         enumOperand->BaseOperand,
         enumOperand->ByteOffset,
         Phx::Alignment::NaturalAlignment(functionUnit->TypeTable->Int32Type),
         functionUnit->AliasInfo->IndirectAliasedMemoryTag,
         functionUnit->SafetyInfo->SafeTag
      );

      // Apply the existing index operand to the new memory operand.

      enumOperand->IndexOperand = indexOperand;
   }

   return enumOperand;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Converts an array of character type to a character pointer type.
//
// Remarks:
//
//
// Returns:
//
//    A memory operand containing the string pointer.
//
//-----------------------------------------------------------------------------

Phx::IR::MemoryOperand ^
IRBuilder::ConvertStringToPointer
(
   Phx::FunctionUnit ^ functionUnit, 
   Phx::IR::Operand ^ operand
)
{
   Phx::IR::MemoryOperand ^ result = nullptr;

   // Get the char and char* types.

   Phx::Types::Type ^ charType = 
      TypeBuilder::GetTargetType(NativeType::Char);

   Phx::Types::Type ^ ptrToCharType = 
      functionUnit->TypeTable->GetUnmanagedPointerType(charType);

   // Change the operand to an address operand if it is not one already.

   if (! operand->IsAddress)
   {
      operand->ChangeToAddress();
   }

   // If the operand is already a memory operand, do nothing.

   if (operand->IsMemoryOperand)
   {
      result = operand->AsMemoryOperand;      
   }

   // Otherwise, create a new memory operand using the
   // current operand as the base.

   else if (operand->IsVariableOperand)
   {
      result = Phx::IR::MemoryOperand::New(
         functionUnit,
         charType,
         nullptr,
         operand->AsVariableOperand,
         0,
         Phx::Alignment::NaturalAlignment(charType),
         functionUnit->AliasInfo->IndirectAliasedMemoryTag,
         functionUnit->SafetyInfo->SafeTag
         );

      // Change operand to address operand.

      result->ChangeToAddress();
   }
   else
   {
      // Unexpected case.

      Debug::Assert(false);
   }

   // Set the type of the operand and return.

   result->Type = ptrToCharType;
   return result;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Converts the given operand to the provided new type.
//
// Remarks:
//
//    This method will attempt to perform the conversion at compile time if
//    the operand is an immediate. Otherwise, the conversion will be
//    performed at run time.
//
// Returns:
//
//    The converted operand.
//
//-----------------------------------------------------------------------------

Phx::IR::Operand ^
IRBuilder::ConvertOperandType
(
   Phx::FunctionUnit ^ functionUnit,
   Phx::IR::Operand ^ operand,
   Phx::Types::Type ^ newType   
)
{
   // If the source operand is an immediate, we can perform
   // the conversion at compile time.
   
   if (operand->IsImmediateOperand)
   {
	  Phx::IR::ImmediateOperand ^ immOp = operand->AsImmediateOperand;
      // Check for conversion from integer to real.

      if (immOp->IsIntImmediate && newType->IsFloat)
      {
         operand = Phx::IR::ImmediateOperand::New(
            functionUnit,
            newType,
            static_cast<double>(immOp->IntValue32)
         );
      }

      // Check for conversion from real to integer.

      else if (immOp->IsFloatImmediate && newType->IsInt)
      {
         operand = Phx::IR::ImmediateOperand::New(
            functionUnit,
            newType,
            static_cast<int>(immOp->FloatValue)
         );
      }
   }
   else
   {
      // Create a Resolve object to convert the value at runtime.

      Phx::Types::Resolve ^ resolve = ModuleBuilder::Resolve;

      operand = resolve->Convert(
         operand,
         functionUnit->LastInstruction->Previous,
         newType
      );
   }

   // Return the converted operand.

   return operand;
}

} // namespace Pascal