//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Implementation of the Node class and derived classes.
//
// Remarks:
//
//    The Node class represents a node of the abstract syntax tree that
//    is built at parse time (Grammar.y).
//    It performs typechecking (through Node::TypeCheck) and code generation
//    (through the recursive virtual method Node::Evaluate).
//
//-----------------------------------------------------------------------------


#include <assert.h>

// Include definition of foreach_... macros.

#include "..\..\common\samples.h"

#include "Ast.h"
#include "YaccDeclarations.h"
#include "RankedSymExtensionObject.h"

//-----------------------------------------------------------------------------
// Static Node methods.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Description:
//
//    Helper function to implement properties SourceTypeString and
//    TargetTypeString.
//
//-----------------------------------------------------------------------------

void Node::InitializeTypeMaps()
{
   // Populate the maps from Phx::Types::Type to string representations.

   _sourceTypeString = gcnew Dictionary<Phx::Types::Type^ , String^> ();
   _sourceTypeString[Phx::GlobalData::TypeTable->Int32Type] = "int";
   _sourceTypeString[Phx::GlobalData::TypeTable->BooleanType] = "bool";
   _sourceTypeString[Phx::GlobalData::TypeTable->SystemStringAggregateType] =
      "string";
   _sourceTypeString[Phx::GlobalData::TypeTable->UnknownType] = "ERROR";

   _targetTypeString = gcnew Dictionary<Phx::Types::Type^ , String^> ();
   _targetTypeString[Phx::GlobalData::TypeTable->Int32Type] = "int32";
   _targetTypeString[Phx::GlobalData::TypeTable->BooleanType] = "bool";
   _targetTypeString[Phx::GlobalData::TypeTable->SystemStringAggregateType] =
      "string";
   _targetTypeString[Phx::GlobalData::TypeTable->UnknownType] = "ERROR";
}

//-----------------------------------------------------------------------------
//  Implementations of the Msil-generation method Evaluate and helper functions.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for a whole "Minim" program.
//
// Remarks:
//
//    Recurses on the virtual Evaluate method in its children to generate
//    code for each of the procedures that make up a Minim program.
//
//    Allocates the global field ParamsArray that is instrumental in the
//    implementation of the Print intrinsic (see function EvalPrintCall).
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
ModuleNode::Evaluate
(
   TextWriter ^ out
)
{
   // A module unit exists, because it was needed at parse time.
   // We still have to make sure lookup of the symbol table by name is enabled.

   Phx::ModuleUnit ^ module =
      Phx::Threading::Context::GetCurrent()->Unit->AsModuleUnit;
   if (module->SymbolTable->NameMap == nullptr)
   {
      module->SymbolTable->AddMap(Phx::Symbols::NameMap::New(module->SymbolTable, 64));
   }

   // The version of mscorlib.dll for version 2.0 of the Clr is hard-coded.
   // It should ideally be retrieved from configuration data.

   out->WriteLine(".assembly extern mscorlib");
   out->WriteLine("{");
   out->WriteLine(".publickeytoken = (B7 7A 5C 56 19 34 E0 89 )");
   out->WriteLine(
      ".hash = (CB 46 AC 1D E2 80 6F A3 04 89 AB BC 78 7C 66 17 C1 03 B6 06 ) ")
      ;
   out->WriteLine("}");
   out->WriteLine();

   // The names HOST_ASSEMBLY and HOST_MODULE are arbitrary.

   out->WriteLine(".assembly HOST_ASSEMBLY { }");
   out->WriteLine();
   out->WriteLine(".module HOST_MODULE.exe");

   // No namespace or class declaration is needed for the toy language that is
   // compiled.
   // Methods will get defined at global level.

   for each (Node^ procedure in _childList)
   {
      procedure->Evaluate(out);
   }

   // Define a global field for the object array that is used to implement
   // the intrinsic Print.

   out->WriteLine();
   out->WriteLine(".field assembly static object[] ParamsArray");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for a function node.
//
// Remarks:
//
//    A function node is a polyadic node whose first child is a FunctionHeader
//    and the following children are the statements that make up the function
//    body.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
FunctionNode::Evaluate
(
   TextWriter ^ out
)
{
   // Generate header.

   _childList[0]->Evaluate(out);

   // Generate body.

   //   Generation of the body will take place in the context of the FunctionUnit
   //   that was created while evaluating the header.

   Phx::FunctionUnit ^ functionUnit
      = Phx::Threading::Context::GetCurrent()->Unit->AsFunctionUnit;

   out->WriteLine("{");

   // If the procedure's name is Main, then it is the entry point.

   if ((safe_cast<HeaderNode ^>(_childList[0])->FunctionName) == "Main")
   {
      out->WriteLine("  .entrypoint");
      out->WriteLine();
   }

   // Collect local variables and place them in the local symbol table.
   //   This pass is needed prior to generating code for the function body
   //   insofar as variable types are inferred from assignment statements.

   //   Initialize the order numbers to be assigned to symbols.

   RankedSymExtensionObject::LocalIndex = 0;

   CollectLocalVariables();

   // Generate declaration for locals found, if any.

   if (RankedSymExtensionObject::LocalIndex > 0)
   {
      array< Phx::Symbols::VariableSymbol ^> ^ locals =
         gcnew array< Phx::Symbols::VariableSymbol ^>(
            RankedSymExtensionObject::LocalIndex);

      // Sort variable symbols by order number into array 'locals', whose
      // size is the current value of LocalIndex.

      Phx::Symbols::Table ^ symbolTable = functionUnit->SymbolTable;

      for each (Phx::Symbols::Symbol ^ symbol in symbolTable->AllSymbols)
      {
         if (!symbol->IsVariableSymbol)
         {
            continue;
         }

         Phx::Symbols::VariableSymbol ^ variableSymbol = symbol->AsVariableSymbol;

         if (variableSymbol->StorageClass != Phx::Symbols::StorageClass::Auto)
         {
            continue;
         }

         RankedSymExtensionObject ^ extensionObject
            = RankedSymExtensionObject::Get(variableSymbol);
         locals[extensionObject->OrderNumber] = variableSymbol;
      }

      // Generate declaration for locals found.

      out->WriteLine(".locals init (");
      String ^                      separator = "";
      for each (Phx::Symbols::VariableSymbol ^ variableSymbol in locals)
      {
         out->WriteLine("{0}{1} {2}", separator, TargetTypeString[variableSymbol->Type],
            variableSymbol->Name);
         separator = ", ";
      }
      out->WriteLine(")");
   }

   // Generate statements.

   for (int i = 1; i < _childList->Count; ++i)
   {
      _childList[i]->Evaluate(out);
   }

   // Add a return statement in the end if none was present.

   ReturnNode ^ finalReturnStatement
      = dynamic_cast<ReturnNode^>(_childList[_childList->Count - 1]);
   if (finalReturnStatement == nullptr)
   {
      out->WriteLine("  ret");
   }

   // End the function body with a right bracket.

   out->WriteLine("}");

   // Delete the function unit. This pops it off the stack of current units.

   functionUnit->Delete();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Helper function of HeaderNode::Evaluate.
//
// Remarks:
//
//    InitSymbols creates a FunctionUnit and attaches a symbol table to it.
//    It then populates the new function table with the formal parameters.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void HeaderNode::InitSymbols()
{
   // Build the function type.

   Phx::Types::FunctionTypeBuilder ^ functionTypeBuilder =
      Phx::Types::FunctionTypeBuilder::New(Phx::GlobalData::TypeTable);

   functionTypeBuilder->Begin();

   functionTypeBuilder->CallingConventionKind =
      Phx::Types::CallingConventionKind::ClrCall;

   // All Minim "functions" are procedures, i.e. functions returning void.

   functionTypeBuilder->AppendReturnParameter(Phx::GlobalData::TypeTable->VoidType);

   for each (Node ^   formalParameter in _childList)
   {
      functionTypeBuilder->AppendParameter(
         safe_cast<FormalParameterNode^>(formalParameter)->Type);
   }

   Phx::Types::FunctionType ^ functionType = functionTypeBuilder->GetFunctionType();

   // Use the function type to create a function symbol, which -in turn- is
   // needed in order to create a function unit, to which a local symbol
   // table will be attached.

   // Create function symbol in module symbol table.

   // When processing a function header, any previous FunctionUnit created will have
   // been deleted in FunctionNode::Evaluate. Therefore, the current unit will be
   // the module unit.

   Phx::ModuleUnit ^ module =
      Phx::Threading::Context::GetCurrent()->Unit->AsModuleUnit;

   Phx::Name functionName = Phx::Name::New(module->Lifetime, _functionName);
   if (module->SymbolTable->NameMap->Lookup(functionName) != nullptr)
   {
      Console::Error->WriteLine(
         "Line {0}. There is already a procedure with name {1}.",
         _sourceLineNumber, _functionName);
      ++ErrorCount;
      return;
   }

   Phx::Symbols::FunctionSymbol ^ functionSymbol = Phx::Symbols::FunctionSymbol::New(
      module->SymbolTable, 0 /* extern Id: can be arbitrary here */,
      functionName, functionType, Phx::Symbols::Visibility::GlobalDefinition);

   // Create function unit.

   Phx::FunctionUnit ^ functionUnit = Phx::FunctionUnit::New(
      Phx::Lifetime::New(Phx::LifetimeKind::Function, nullptr),
      functionSymbol, Phx::CodeGenerationMode::IJW,
      Phx::GlobalData::TypeTable, module->Architecture, module->Runtime,
      module, ++FunctionCounter);

   // Attach local symbol table.

   Phx::Symbols::Table ^ funcSymTable = Phx::Symbols::Table::New(functionUnit, 64, true);

   // Make it accessible by symbol name.

   funcSymTable->AddMap(Phx::Symbols::NameMap::New(funcSymTable, 64));

   // Place the formal parameters in the local symbol table.

   for (int i = 0; i < _childList->Count; ++i)
   {
      String ^  paramNameString =
         safe_cast<FormalParameterNode^>(_childList[i])->ParameterName;
      Phx::Name parameterName = Phx::Name::New(functionUnit->Lifetime, paramNameString);

      if (funcSymTable->NameMap->Lookup(parameterName) != nullptr)
      {
         Console::Error->WriteLine(
            "Line {0}. More than one formal parameter with name {1}.",
            _sourceLineNumber, paramNameString);
         ++ErrorCount;
         return;
      }

      Phx::Symbols::LocalVariableSymbol ^ parameterSymbol =
         Phx::Symbols::LocalVariableSymbol::New(funcSymTable,
            0, parameterName, safe_cast<FormalParameterNode^>(_childList[i])->Type,
            Phx::Symbols::StorageClass::Parameter);

      // Associate the occurrence index with the symbol.

      parameterSymbol->AddExtensionObject(gcnew RankedSymExtensionObject(i));
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for a function header.
//
// Remarks:
//
//    Creates a function unit and a symbol table for the unit (see
//    Evaluate::InitSymbols); then generates a method header in ilasm.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
HeaderNode::Evaluate
(
   TextWriter ^ out
)
{
   // Add the function type to the global symbol table and create a local
   // symbol table.

   InitSymbols();

   // Generate a method header.

   out->Write(".method public static void {0}(", _functionName);

   // Generate parameter list as a comma-separated list.

   String^         separator = nullptr;
   for each (Node^ child in _childList)
   {
      if (separator == nullptr)
      {
         separator = ", ";
      }
      else
      {
         out->Write(separator);
      }
      child->Evaluate(out);
   }

   // Close method header.

   out->WriteLine(") cil managed");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//     Generate a formal parameter declaration in an ilasm method header.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
FormalParameterNode::Evaluate
(
   TextWriter ^ out
)
{
   // Generate parameter declaration for method header.

   out->Write("{0} {1}", TargetTypeString[_type], _parameter);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for a return statement.
//
// Remarks:
//
//    When a procedure does not explicitly end in a return statement,
//    FunctionNode::Evaluate takes care of supplying one in ilasm.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void 
ReturnNode::Evaluate
(
   TextWriter^ out
)
{
   out->WriteLine("  ret");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for an integer constant.
//
//-----------------------------------------------------------------------------

void 
NumberNode::Evaluate
(
   TextWriter^ out
)
{
   out->WriteLine("  ldc.i4 {0}", _number);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for an boolean constant.
//
//-----------------------------------------------------------------------------

void 
LogicalNode::Evaluate
(
   TextWriter^ out
)
{
   if (_value)
   {
      // Push 1 for nonzero, i.e. true.

      out->WriteLine("  ldc.i4.1");
   }
   else
   {
      // Push 0 for false.

      out->WriteLine("  ldc.i4.0");
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code to evaluate an occurrence of a string literal.
//
//-----------------------------------------------------------------------------

void
StringNode::Evaluate
(
   TextWriter^ out
)
{
   out->WriteLine("  ldstr \"{0}\"", _string);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for a negative arithmetic expression.
//
//-----------------------------------------------------------------------------

void 
UnaryMinusNode::Evaluate
(
   TextWriter^ out
)
{
   // Push operand.

   _first->Evaluate(out);

   // Take its 2's complement.

   out->WriteLine("  neg");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for a negative boolean expression.
//
//-----------------------------------------------------------------------------

void 
NegationNode::Evaluate
(
   TextWriter^ out
)
{
   // Push boolean operand.

   _first->Evaluate(out);

   // Take its 1's complement.

   out->WriteLine("  neg");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for an if-statement.
//
// Remarks:
//
//    A ConditionNode is a ternary node whose third child may be null
//    in case there is no else branch.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
ConditionNode::Evaluate
(
   TextWriter ^ out
)
{
   // Evaluate condition (an ordinary expression that will get tested
   // for zero/nonzero).

   if (!TypeCheck(_first, Phx::GlobalData::TypeTable->BooleanType,
         "Condition of branching statement"))
   {
      return;
   }
   _first->Evaluate(out);

   System::String^ label1 = NewLabel();
   System::String^ label2 = NewLabel();
   out->WriteLine("  brtrue.s {0}", label1);
   out->WriteLine("  br {0}", label2);

   // Then-branch is at label1.

   out->WriteLine("{0}:", label1);
   _second->Evaluate(out);

   // No else branch?

   if (_third == nullptr)
   {
      out->WriteLine("{0}:", label2);
      return;
   }

   // Exit is at label 3.

   System::String^ label3 = NewLabel();
   out->WriteLine("  br {0}", label3);

   // Else-branch is at label 2.

   out->WriteLine("{0}:", label2);
   _third->Evaluate(out);
   out->WriteLine("{0}:", label3);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for an equality comparison or an arithmetic comparison.
//
// Remarks:
//
//    Equality comparison is supported between pairs of equal types.
//    Order comparisons are supported only on integers.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
ComparisonNode::Evaluate
(
   TextWriter ^ out
)
{
   // Type-check operands.

   if ((_operator != EQ) && (_operator != NE))
   {
      // Order comparison: only applies between numbers.

      if (!TypeCheck(_first, Phx::GlobalData::TypeTable->Int32Type,
            "First comparison term")
         || !TypeCheck(_second, Phx::GlobalData::TypeTable->Int32Type,
            "Second comparison term"))
      {
         return;
      }
   }
   else
   {
      // Equality comparison: supported between 2 booleans or 2 integers
      // exclusively.

      Phx::Types::Type ^ type1 = ExpressionType(_first,
         "First comparison term");
      Phx::Types::Type ^ type2 = ExpressionType(_second,
         "Second comparison term");

      if ((type1 == Phx::GlobalData::TypeTable->UnknownType)
         || (type2 == Phx::GlobalData::TypeTable->UnknownType))
      {
         return;
      }

      if (type1 != type2)
      {
         Console::Error->WriteLine(
            "Line {0}. Comparison between different types is illegal.",
            _sourceLineNumber);
         ++ErrorCount;
         return;
      }

      if (type1 == Phx::GlobalData::TypeTable->SystemStringAggregateType)
      {
         Console::Error->WriteLine(
            "Line {0}. Unexpected type 'string' for comparison operator.",
            _sourceLineNumber);
         ++ErrorCount;
         return;
      }
   }

   // Generate branching code.

   System::String ^ branchInstruction;
   System::String ^ label1 = NewLabel();
   System::String ^ label2 = NewLabel();

   switch(_operator)
   {

      case EQ:

         branchInstruction = "beq";
         break;

      case NE:

         branchInstruction = "bne";
         break;

      case GT:

         branchInstruction = "bgt";
         break;

      case GE:

         branchInstruction = "bge";
         break;

      case LT:

         branchInstruction = "blt";
         break;

      case LE:

         branchInstruction = "ble";
         break;

      default:

         assert(false);
         break;
   }

   // Push operands.

   _first->Evaluate(out);
   _second->Evaluate(out);

   // Compare them.

   out->WriteLine("  {0}.s {1}", branchInstruction, label1);

   // Push zero if comparison failed.

   out->WriteLine("  ldc.i4.0");
   out->WriteLine("  br {0}", label2);

   // Push nonzero otherwise.

   out->WriteLine("{0}:", label1);
   out->WriteLine("  ldc.i4.1");
   out->WriteLine("{0}:", label2);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Invoke descendants to collect local variables in the symbol table.
//
// Remarks:
//
//    All nodes except AssignmentNode simply delegate this call to descendants.
//
//-----------------------------------------------------------------------------

void Node::CollectLocalVariables()
{
}

void UnaryNode::CollectLocalVariables()
{
   _first->CollectLocalVariables();
}

void BinaryNode::CollectLocalVariables()
{
   UnaryNode::CollectLocalVariables();
   _second->CollectLocalVariables();
}

void TernaryNode::CollectLocalVariables()
{
   BinaryNode::CollectLocalVariables();
   if (_third != nullptr)
   {
      _third->CollectLocalVariables();
   }
}

void PolyadicNode::CollectLocalVariables()
{
   for each (Node^ child in _childList)
   {
      child->CollectLocalVariables();
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Record the lvalue in the symbol table if it has not yet been recorded.
//
// Remarks:
//
//    The "Minim" language has no variable declarations. Variable types are
//    inferred from assignment statements. Therefore, this function gets
//    invoked via recursion launched in FunctionNode::Evaluate prior to launching
//    Evaluate on the body of a function. This makes it possible to generate
//    an ilasm .local declaration before code is generated for the body.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void AssignmentNode::CollectLocalVariables()
{
   // The current unit is a function unit.

   Phx::FunctionUnit ^ functionUnit =
      Phx::Threading::Context::GetCurrent()->Unit->AsFunctionUnit;

   Phx::Name        variableName = Phx::Name::New(functionUnit->Lifetime,
      _variable);
   Phx::Symbols::Symbol ^ variableSym =
      functionUnit->SymbolTable->NameMap->Lookup(variableName);

   // If _variable is a new symbol, record it.

   Phx::Types::Type ^ assignedType = (safe_cast<IExpression^>(_first))->Type;

   if (variableSym == nullptr)
   {
      variableSym = Phx::Symbols::LocalVariableSymbol::NewAuto(
         functionUnit->SymbolTable, 0, variableName, assignedType);
      variableSym->AddExtensionObject(gcnew RankedSymExtensionObject(
            RankedSymExtensionObject::LocalIndex++));

   }

   // Else, type-check it.

   else if (variableSym->Type != assignedType)
   {
      Console::Error->WriteLine("Line {0}. Variable {1} of type {2} is being "
         "assigned an expression of type {3}.",
         _sourceLineNumber, _variable,
         SourceTypeString[variableSym->Type],
         SourceTypeString[assignedType]);
      ++ErrorCount;
      return;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for an assignment.
//
// Remarks:
//
//    In order to generate the correct instruction, this function looks up
//    the function's symbol table to find out the lvalue's storage class
//    (local variable vs. parameter) and its declaration order.
//    This last piece of information is stored in an extension object
//    attached to VariableSymbol objects.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
AssignmentNode::Evaluate
(
   TextWriter ^ out
)
{
   // Push the rvalue on the stack.

   _first->Evaluate(out);

   // Pop it into the lvalue.

   //  This requires a symbol table access to retrieve the symbol's storage
   //  class (local vs. param) and its syntactic occurrence order.

   Phx::FunctionUnit ^ functionUnit =
      Phx::Threading::Context::GetCurrent()->Unit->AsFunctionUnit;
   Phx::Name variableName = Phx::Name::New(functionUnit->Lifetime, _variable);

   // One can assume the lvalue is recorded in the symbol table, even if there
   // is an error in the input (see AssignmentNode::CollectLocalVariables).

   Phx::Symbols::VariableSymbol ^ lvalueSym =
      functionUnit->SymbolTable->NameMap->Lookup(variableName)->AsVariableSymbol;

   // Retrieve syntactic order among local variables or parameters.

   RankedSymExtensionObject ^ symExtension
      = RankedSymExtensionObject::Get(lvalueSym);

   if (lvalueSym->StorageClass == Phx::Symbols::StorageClass::Parameter)
   {
      out->WriteLine("  starg {0}", symExtension->OrderNumber);
   }
   else
   {
      out->WriteLine(" stloc.{0}", symExtension->OrderNumber);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for a variable occurrence in an expression.
//
// Remarks:
//
//    When a variable occurs in an lvalue, it is handled by
//    AssignmentNode::Evaluate.
//    The declaration order of the variable is found through an extension object
//    that is attached to the symbol stored in the function's symbol table.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
VariableNode::Evaluate
(
   TextWriter ^ out
)
{
   Phx::FunctionUnit ^  functionUnit =
      Phx::Threading::Context::GetCurrent()->Unit->AsFunctionUnit;
   Phx::Name        variableName = Phx::Name::New(functionUnit->Lifetime,
      _variable);
   Phx::Symbols::Symbol ^ sym = functionUnit->SymbolTable->NameMap->Lookup(variableName);

   if (sym == nullptr)
   {
      Console::Error->WriteLine("Line {0}. Undefined variable {1}.",
         _sourceLineNumber, _variable);
      ++ErrorCount;
      return;
   }

   Phx::Symbols::VariableSymbol ^        variableSymbol = sym->AsVariableSymbol;
   RankedSymExtensionObject ^ indexExtension
      = RankedSymExtensionObject::Get(variableSymbol);
   if (variableSymbol->StorageClass == Phx::Symbols::StorageClass::Parameter)
   {
      out->WriteLine("  ldarg {0}", indexExtension->OrderNumber);
   }
   else
   {
      out->WriteLine("  ldloc {0}", indexExtension->OrderNumber);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Implementation of the property IExpression::Type.
//
// Remarks:
//
//    The type of the variable is retrieved from the symbol table of the
//    current FunctionUnit.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

Phx::Types::Type ^ VariableNode::Type::get()
{
   Phx::FunctionUnit ^  functionUnit =
      Phx::Threading::Context::GetCurrent()->Unit->AsFunctionUnit;
   Phx::Name        variableName = Phx::Name::New(functionUnit->Lifetime,
      _variable);
   Phx::Symbols::Symbol ^ sym = functionUnit->SymbolTable->NameMap->Lookup(variableName);

   if (sym == nullptr)
   {
      Console::Error->WriteLine("Line {0}. Variable {1} has not been defined.",
         _sourceLineNumber, _variable);
      ++ErrorCount;
      return Phx::GlobalData::TypeTable->UnknownType;
   }
   return sym->Type;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for a procedure call.
//
// Remarks:
//
//    The intrinsic procedure "Print" is special-cased (see function
//    FunctionNode::EvalPrintCall).
//
//    Generating an ilasm call involves retrieving parameter types from the
//    symbol table.
//
// Returns:
//
//   void
//
//-----------------------------------------------------------------------------

void
FunctionCallNode::Evaluate
(
   TextWriter^ out
)
{
   // Special-case the Print intrinsic.

   if (_function == "Print")
   {
      EvalPrintCall(out);
      return;
   }

   // Retrieve the function symbol.

   Phx::FunctionUnit ^      functionUnit =
      Phx::Threading::Context::GetCurrent()->Unit->AsFunctionUnit;
   Phx::Symbols::Table ^   moduleTable = functionUnit->SymbolTable->ParentTable;
   Phx::Name            functionName = Phx::Name::New(moduleTable->Lifetime,
      _function);
   Phx::Symbols::FunctionSymbol ^ functionSymbol =
      moduleTable->NameMap->Lookup(functionName)->AsFunctionSymbol;

   if (functionSymbol == nullptr)
   {
      Console::Error->WriteLine("Line {0}. Undefined function {1}.",
         _sourceLineNumber, _function);
      ++ErrorCount;
      return;
   }

   // Push actual parameters

   Phx::Types::FunctionType ^ functionType = functionSymbol->Type->AsFunctionType;

   for (int i = 0; i < _childList->Count; ++i)
   {
      TypeCheck(_childList[i], functionType->Parameters[i].Type,
         String::Format("Parameter {0} to function {1}", i + 1, _function));
      _childList[i]->Evaluate(out);
   }

   // Generate call instruction.
   // "Functions" in Minim are really procedures. They always return void.

   out->Write("  call void {0}(", _function);

   // Generate comma-separated list of types for formal parameters.

   bool                       needSeparator = false;

   for each (Phx::Types::Parameter parameter in functionType->Parameters)
   {
      Phx::Types::Type ^ formalType = parameter.Type;

      out->Write("{0}{1}",
         needSeparator ? ", " : "",
         TargetTypeString[formalType]);
      needSeparator = true;
   }

   out->WriteLine(")");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for a call to the Print intrinsic.
//
// Remarks:
//
//    The code that is generated invokes the override of Console.Writeline
//    that takes a format string and an array of format parameters.
//    This array is allocated as a global field in ModuleNode::Evaluate.
//
// Returns:
//
//    void
//
//-----------------------------------------------------------------------------

void
FunctionCallNode::EvalPrintCall
(
   TextWriter^ out
)
{
   // If there are no parameters, just print a newline.

   if (_childList->Count == 0)
   {
      out->WriteLine("  call void [mscorlib]System.Console::WriteLine()");
      return;
   }

   // Push format string.

   out->Write("  ldstr \"");
   for (int i = 0; i < _childList->Count; ++i)
   {
      out->Write("{{{0}}}", i);
   }
   out->WriteLine("\"");

   // Create an array to contain each expression to print.
   // The name ParamsArray is arbitrary and is necessarily unique insofar as
   // Minim does not have global variables.

   out->WriteLine("  ldc.i4 {0}", _childList->Count);
   out->WriteLine("  newarr [mscorlib]System.Object");
   out->WriteLine("  stsfld object[] ParamsArray");

   // Store each param value in ParamsArray.

   for (int i = 0; i < _childList->Count; ++i)
   {
      out->WriteLine("  ldsfld object[] ParamsArray");
      out->WriteLine("  ldc.i4 {0}", i);
      _childList[i]->Evaluate(out);
      Phx::Types::Type ^ parameterType;
      try
      {
         parameterType = safe_cast<IExpression^>(_childList[i])->Type;
      }
      catch (InvalidCastException^)
      {
         Console::Error->WriteLine(
            "Line {0}. Parameter #{1} to procedure {2} is not an expression.",
            i + 1, _function);
         ++ErrorCount;
         return;
      }

      // Box values prior to storing in ParamsArray.

      if (parameterType == Phx::GlobalData::TypeTable->Int32Type)
      {
         out->WriteLine(" box [mscorlib]System.Int32");
      }
      else if (parameterType == Phx::GlobalData::TypeTable->BooleanType)
      {
         out->WriteLine(" box [mscorlib]System.Boolean");
      }

      out->WriteLine("  stelem.ref");
   }

   // Push ParamsArray and invoke WriteLine.

   out->WriteLine("  ldsfld object[] ParamsArray");
   out->WriteLine(
      "  call void [mscorlib]System.Console::WriteLine(string, object[])");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate an ilasm instruction to implement an arithmetic binary operation.
//
//-----------------------------------------------------------------------------

void
BinaryArithmeticNode::Evaluate
(
   TextWriter^ out
)
{
   if (!TypeCheck(_first, Phx::GlobalData::TypeTable->Int32Type,
         String::Format("First '{0}' operand",
            static_cast<Char>(_operator)))
      || !TypeCheck(_second, Phx::GlobalData::TypeTable->Int32Type,
         String::Format("Second '{0}' operand", static_cast<Char>(_operator))))
   {
      return;
   }

   // Push operands on the stack.

   _first->Evaluate(out);
   _second->Evaluate(out);

   // Generate binary instruction.

   switch(_operator)
   {
      case '*':

         out->WriteLine("  mul");
         break;

      case '/':

         out->WriteLine("  div");
         break;

      case '+':

         out->WriteLine("  add");
         break;

      case '-':

         out->WriteLine("  sub");
         break;

      default:

         assert(false);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Generate code for an and- or or-clause.
//
//-----------------------------------------------------------------------------

void
BinaryLogicalNode::Evaluate
(
   TextWriter^ out
)
{
   if (!TypeCheck(_first, Phx::GlobalData::TypeTable->BooleanType,
         "Condition clause")
      || !TypeCheck(_second, Phx::GlobalData::TypeTable->BooleanType,
         "Condition clause"))
   {
      return;
   }

   // Push operands.

   _first->Evaluate(out);
   _second->Evaluate(out);

   // Generate AND or OR as needed.

   if (_operator == AND)
   {
      out->WriteLine("  and");
   }
   else
   {
      out->WriteLine("  or");
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//     Generate code for a polyadic node.
//
// Remarks:
//
//    Simply delegate to child nodes.
//
//-----------------------------------------------------------------------------

void 
PolyadicNode::Evaluate
(
   TextWriter^ out
)
{
   for each (Node^ child in _childList)
   {
      child->Evaluate(out);
   }
}

//-----------------------------------------------------------------------------
//           Implementations of the debugging method Dump
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
// Description:
//
//    Dump the tree rooted at a module node.
//
// Remarks:
//
//    Dump methods yield a crude display of the Ast.
//
//-----------------------------------------------------------------------------

void
ModuleNode::Dump
(
   TextWriter ^ out
)
{
   out->WriteLine("BEGIN");
   for each (Node^ child in _childList)
   {
      child->Dump(out);
   }
   out->WriteLine("\nEND.");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//     Dump a comparison, arithmetic or logical binary node and its descendants.
//
//-----------------------------------------------------------------------------

void
BinaryNodeWithOperatorToken::Dump
(
   System::IO::TextWriter ^ out
)
{
   System::String ^ operatorString = nullptr;

   switch(_operator)
   {

      case EQ:

         operatorString = "==";
         break;

      case NE:

         operatorString = "!=";
         break;

      case GT:

         operatorString = ">";
         break;

      case GE:

         operatorString = ">=";
         break;

      case LT:

         operatorString = "<";
         break;

      case LE:

         operatorString = "<=";
         break;

      case AND:

         operatorString = "AND";
         break;

      case OR:

         operatorString = "OR";
         break;

      default:

      // A literal token does not need an associated string for displaying it.

         operatorString = nullptr;
         break;
   }

   out->Write("( ");
   First()->Dump(out);
   out->Write(" ) ");
   if (operatorString == nullptr) // A literal token.
   {
      out->Write(static_cast<Char>(_operator));
   }
   else
   {
      out->Write(operatorString);
   }
   out->Write(" ( ");
   Second()->Dump(out);
   out->Write(" )");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Dump a return node.
//
//-----------------------------------------------------------------------------

void
ReturnNode::Dump
(
   TextWriter ^ out
)
{
   out->WriteLine("return;");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Dump a number node.
//
//-----------------------------------------------------------------------------

void
NumberNode::Dump
(
   TextWriter ^ out
)
{
   out->Write(_number);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Dump a variable node.
//
//-----------------------------------------------------------------------------

void
VariableNode::Dump
(
   TextWriter ^ out
)
{
   out->Write(_variable);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Dump a boolean constant ('true' or 'false').
//
//-----------------------------------------------------------------------------

void
LogicalNode::Dump
(
   TextWriter ^ out
)
{
   out->Write(_value);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Dump a string literal.
//
//-----------------------------------------------------------------------------

void
StringNode::Dump
(
   TextWriter ^ out
)
{
   out->Write("\"{0}\"", _string);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Dump a formal parameter node.
//
//-----------------------------------------------------------------------------

void
FormalParameterNode::Dump
(
   TextWriter ^ out
)
{
   out->Write(" [ {0}: {1} ]", _parameter, SourceTypeString[_type]);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Dump an assignment node.
//
//-----------------------------------------------------------------------------

void
AssignmentNode::Dump
(
   TextWriter ^ out
)
{
   out->Write("{0} <- ( ", _variable);
   First()->Dump(out);
   out->WriteLine(" );");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Dump a function header.
//
//-----------------------------------------------------------------------------

void
HeaderNode::Dump
(
   TextWriter ^ out
)
{
   out->Write("{0}(", _functionName);

   String^         separator = "";
   for each (Node^ child in _childList)
   {
      out->Write(separator);
      separator = ", ";
      child->Dump(out);
   }

   out->Write(" )");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Dump a procedure call.
//
//-----------------------------------------------------------------------------

void
FunctionCallNode::Dump
(
   TextWriter ^ out
)
{
   out->Write("{0}( ", _function);
   String^         separator = "";
   for each (Node^ child in _childList)
   {
      out->Write(separator);
      separator = ", ";
      child->Dump(out);
   }
   out->WriteLine(" );");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Dump an expression prefixed with a unary minus.
//
//-----------------------------------------------------------------------------

void
UnaryMinusNode::Dump
(
   TextWriter ^ out
)
{
   out->Write("-(");
   First()->Dump(out);
   out->Write(")");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Dump a logical expression prefixed with a negation.
//
//-----------------------------------------------------------------------------

void
NegationNode::Dump
(
   TextWriter ^ out
)
{
   out->Write("NOT(");
   First()->Dump(out);
   out->Write(")");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Dump a conditional statement.
//
//-----------------------------------------------------------------------------

void 
ConditionNode::Dump
(
   TextWriter^ out
)
{
   out->Write("IF (");
   First()->Dump(out);
   out->WriteLine(") THEN { ");
   Second()->Dump(out);
   out->WriteLine("}");
   if (Third() == nullptr)
   {
      return;
   }
   out->Write("ELSE {");
   Third()->Dump(out);
   out->WriteLine("}");
}

//-----------------------------------------------------------------------------
//
// Description:
//
//     Dump a polyadic node.
//
// Remarks:
//
//    Simply delegate to child nodes.
//
//-----------------------------------------------------------------------------

void 
PolyadicNode::Dump
(
   TextWriter^ out
)
{
   for each (Node^ child in _childList)
   {
      child->Dump(out);
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//     Dump function definition.
//
//-----------------------------------------------------------------------------

void 
FunctionNode::Dump
(
   TextWriter^ out
)
{
   out->WriteLine();

   // dump header

   _childList[0]->Dump(out);

   // dump body

   out->WriteLine("\n{");
   for (int i = 1; i < _childList->Count; ++i)
   {
      _childList[i]->Dump(out);
   }
   out->WriteLine("}");
}
