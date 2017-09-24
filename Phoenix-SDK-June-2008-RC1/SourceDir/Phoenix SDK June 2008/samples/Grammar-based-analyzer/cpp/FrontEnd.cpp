//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    Top-level of front-end.exe.
//
// Remarks:
//
//    Successively invokes parsing, dumping of the Abstract Syntax Tree,
//    and evaluation of this Ast, i.e. ilasm generation.
//
// Usage:
//
//    FrontEnd.exe <compiland>
//
//-----------------------------------------------------------------------------


#include "Scanner.h"
#include "Parser.h"
#include "Ast.h"
#include "RankedSymExtensionObject.h"

static void InitializePhx();
static void AddStringTypeToTypeTable();
static bool CheckCommandLine(int argc, char* argv[]);

//-----------------------------------------------------------------------------
//
// Description:
//
//    Entry-point to the sample.
//
//    Usage:
//
//      FrontEnd input-file
//
//      The input file is in the Minim language.
//      The output (on stdout) is in Msil.
//    
// Remarks:
//
//    The ildasm is generated on stdout, which makes it convenient for
//    debugging in VS, but requires redirection to use the front-end in
//    a complete analyzer.
//
// Returns:
//
//    0 for OK. Nonzero for error.
//
//-----------------------------------------------------------------------------

int
main
(
   int   argc,
   char* argv[]
)
{
   Node ^ astRoot;
   Node ^ Parse();

   // Check command usage.

   if (!CheckCommandLine(argc, argv))
   {
      Console::Error->WriteLine("Usage: FrontEnd input-file");
      return 1;
   }

   // Point yyin to the input stream.

   InitScanner(argc, argv);

   // Initialize the phx lib for using types and symbol tables.

   InitializePhx();

   // Parse the input.

   astRoot = Parse();

   if (astRoot != nullptr)
   {
      astRoot->Dump(System::Console::Error);
      astRoot->Evaluate(System::Console::Out);
      if (Node::ErrorCount > 0)
      {
         Console::Error->WriteLine("{0} errors.", Node::ErrorCount);
         return 2;
      }
      return 0;
   }
   else
   {
      return 1;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Perform minimal framework initialization prior to parsing.
//
// Remarks:
//
//    What the parser needs for building the Ast is a global type table
//    populated with types corresponding to the input language's types.
//
//    This notably requires building a module unit, which is later used
//    during ilasm generation.
//
//-----------------------------------------------------------------------------

void InitializePhx()
{
   // Initialize architecture and target. Even though FrontEnd does not use Phoenix's
   // code generation, this information is necessary to properly initialize
   // the type table.

   Phx::Targets::Architectures::Architecture ^ architecture =
      Phx::Targets::Architectures::Msil::Architecture::New();
   Phx::Targets::Runtimes::Runtime ^ runtime =
      Phx::Targets::Runtimes::Vccrt::Win::Msil::Runtime::New(architecture);
   Phx::GlobalData::RegisterTargetArchitecture(architecture);
   Phx::GlobalData::RegisterTargetRuntime(runtime);

   // Initialize the infrastructure.
   // The call to BeginInitialization will initialize GlobalData::TypeTable

   Phx::Initialize::BeginInitialization();

    // EndInitialization completes control initialization and plugin registration.
    // It is necessary to complete control initialization to be able to create
    // module units.

   Phx::Initialize::EndInitialization("PHX|*|_PHX_|", gcnew array<String ^>(0));

    // Create a module (with an arbitrary name) and a symbol table for this
    // module.
    // This symbol table will be needed to add the build-in System.String type
    // to the type table.

   Phx::Types::Table ^ typeTable = Phx::GlobalData::TypeTable;
   Phx::Lifetime ^     lifetime = Phx::Lifetime::New(Phx::LifetimeKind::Module,
      nullptr);
   Phx::Name           moduleName = Phx::Name::New(lifetime, "compiland.obj");
   Phx::ModuleUnit ^   module = Phx::ModuleUnit::New(lifetime, moduleName,
      nullptr,
      typeTable, architecture, runtime);
   Phx::Symbols::Table::New(module, 64, true);

    // BeginInitialization has populated the type table with primitive type definitions.
    // We also need to define the Msil String type.

   AddStringTypeToTypeTable();
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Initialize the SystemStringAggregateType property of the global type table.
//
// Remarks:
//
//    Phx::Initialize places primitive types into the type table. Aggregate types
//    have to be initialized by client code. E.g. the PEReader does it.
//
//-----------------------------------------------------------------------------

void AddStringTypeToTypeTable()
{
   Phx::Types::Table ^ typeTable = Phx::GlobalData::TypeTable;

   // The last unit created (the module unit) can be retrieved as the current
   // unit.

   Phx::ModuleUnit ^  module =
      Phx::Threading::Context::GetCurrent()->Unit->AsModuleUnit;
   Phx::Symbols::Table ^ symbolTable = module->SymbolTable;

   // First create the base type, which is System.Object.

   // Create a symbol for the object type.

   Phx::Name                name = Phx::Name::New(symbolTable->Lifetime,
      "System.Object");
   Phx::Symbols::MsilTypeSymbol ^ typeSymbol = Phx::Symbols::MsilTypeSymbol::New(
      symbolTable, name, 0);

   // Create an aggregate type with variable-sized data.

   Phx::Types::AggregateType ^ aggregateType = Phx::Types::AggregateType::NewDynamicSize(
      typeTable, typeSymbol);
   aggregateType->IsRuntimeBuiltInType = true;
   aggregateType->IsSelfDescribing = true;
   aggregateType->IsPrimary = true;

   // Make it accessible through property SystemObjectAggregateType.

   typeTable->SystemObjectAggregateType = aggregateType;

   // Create a symbol for the String type.

   name = Phx::Name::New(symbolTable->Lifetime, "System.String");
   typeSymbol = Phx::Symbols::MsilTypeSymbol::New(symbolTable, name, 0);

   // Create an aggregate type with variable-sized data.

   aggregateType = Phx::Types::AggregateType::NewDynamicSize(typeTable, typeSymbol);
   aggregateType->IsRuntimeBuiltInType = true;
   aggregateType->IsSelfDescribing = true;
   aggregateType->IsPrimary = true;

   // Derive it from Object.

   Phx::Types::BaseTypeLink ^ baseTypeLink =
      Phx::Types::BaseTypeLink::New(typeTable->SystemObjectAggregateType->AsAggregateType,
         false);
   aggregateType->AppendBaseTypeLink(baseTypeLink);

   // Make it accessible through property SystemStringAggregateType.

   typeTable->SystemStringAggregateType = aggregateType;

   // Create a type for String reference.

   typeTable->ObjectPointerSystemStringType = typeTable->GetObjectPointerType(aggregateType);
}

//-----------------------------------------------------------------------------
//
// Description:
//
//     Check syntax of command line.
//
// Returns:
//
//     true if and only if the command line is correct.
//
//-----------------------------------------------------------------------------

bool 
CheckCommandLine
(
   int argc,
   char**
)
{
   return argc == 2;
}
