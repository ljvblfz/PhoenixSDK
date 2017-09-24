//--------------------------------------------------------------------------
//
// Description:
//
//    BBCount: A sample program that instruments an Msil assembly/module
//    with basic block counting routines which is in a DLL.
//
// Usage:
//
//    BBCount [/outdir <output-file-name>] [/pdb] [/map <map-name>]
//            [/cnt <count-name>] [/v] [/d] <src-assembly-list>
//
//      Perform basic-block instrumentation on an Msil image.
//      Arguments in [] are optional.
//
//      /v:   verbose
//      /d:   dump per-block disassembly
//      /outdir: directory to hold the instrumented Msil image
//            Default: current directory plus \instrumented
//      /pdb: output an updated pdb to the output directory
//      /map: name of map where you want to keep the mapping information.
//            Default: bbcount.map
//      /cnt: specify the file where you want to keep the total number
//            of basic blocks.
//            Default: bbcount.cnt
//
//      <src-assembly-list>
//            A semicolon-separated list of assemblies to instrument.
//            For each input assembly foo.exe an output assembly foo-new.exe is
//            created.
//
// Remarks:
//
//    This sample provides some more general interfaces that intend to
//    simplify instrumenting Msil modules. BBCount is just one
//    example that shows how to use these interfaces.
//
//--------------------------------------------------------------------------

#pragma region Using directives

using namespace System;
using namespace System::IO;
using namespace System::Collections;
using namespace System::Xml;

#pragma endregion

#include "bbcount.h"

// Not using Phx namespace to show explicit class hierarchy

//------------------------------------------------------------------------
//
//Description:
//
//    Initialization of the Phoenix infrastructure:
//       (1) set the appropriate code generation targets and runtime
//           for Phoenix. Here we assume targets is x86+msil
//       (2) Initialize the Phx framework (Phx::Initialize::All())
//
// Returns:
//
//    Nothing
//
// Remarks:
//
//    A more general way is to read in the target from command line.
//    Right now, we just assume it is x86+msil
//
//------------------------------------------------------------------------

void
Instrumentor::InitializePhx()
{
   // Setup X86/Msil Target and architecture

   Phx::Targets::Architectures::Architecture ^       x86Arch =
      Phx::Targets::Architectures::X86::Architecture::New();
   Phx::Targets::Runtimes::Runtime ^ win32x86Runtime =
      Phx::Targets::Runtimes::Vccrt::Win32::X86::Runtime::New(x86Arch);
   Phx::GlobalData::RegisterTargetArchitecture(x86Arch);
   Phx::GlobalData::RegisterTargetRuntime(win32x86Runtime);

   Phx::Targets::Architectures::Architecture ^       msilArch =
      Phx::Targets::Architectures::Msil::Architecture::New();
   Phx::Targets::Runtimes::Runtime ^ winMSILRuntime =
      Phx::Targets::Runtimes::Vccrt::Win::Msil::Runtime::New(msilArch);

   Phx::GlobalData::RegisterTargetArchitecture(msilArch);
   Phx::GlobalData::RegisterTargetRuntime(winMSILRuntime);

   // Initialize the Phx infrastructure

   Phx::Initialize::BeginInitialization();

   // And our phase

   InstrumentPhase::Initialize();
}

//------------------------------------------------------------------------
//
// Description:
//
//    The initialization of the Instrumentor.
//    For now, we only do some initialization for the IntrumentPhase
//
//------------------------------------------------------------------------

void
Instrumentor::Initialize
(
   Logger ^ l
)
{
   logger = l;
}

// The main entry for processing one assembly

void
Instrumentor::Process
(
   String ^ inFile
)
{
   currentAssembly = inFile;

   if (CmdLineParser::verbose)
   {
      Console::WriteLine("Processing " + currentAssembly + " ...");
   }

   // write out the map info for the assembly processing

   logger->StartAssemblyMap(currentAssembly);

   // Lookup the architecture and runtime.

   Phx::Targets::Architectures::Architecture ^       architecture =
      Phx::GlobalData::GetTargetArchitecture("X86");
   Phx::Targets::Runtimes::Runtime ^ runtime =
      Phx::GlobalData::GetTargetRuntime("Vccrt-Win32-X86");

   // Create an empty program unit in which to place the module unit to be read.

   Phx::Lifetime ^ lifetime = Phx::Lifetime::New(Phx::LifetimeKind::Global,
      nullptr);

   Phx::ProgramUnit ^ programUnit = Phx::ProgramUnit::New(lifetime, nullptr,
      Phx::GlobalData::TypeTable, architecture, runtime);

   // Make an empty moduleUnit.
   //   The PEModuleUnit class is used as the top level Unit when a PE
   //   executable is read.  This extends Phx::ModuleUnit with properties
   //   for the PE specific attributes read from the source executable
   //   that have no other representation in the Phoenix IR.

   Phx::PEModuleUnit ^ moduleUnit = Phx::PEModuleUnit::New(lifetime,
      Phx::Name::New(lifetime, currentAssembly), programUnit,
      Phx::GlobalData::TypeTable, architecture, runtime);

   // Prepare a phase list that we want to do with the input module

   Phx::Phases::PhaseConfiguration ^ config = PreparePhaseList(lifetime);

   // Execute the phase list

   config->PhaseList->DoPhaseList(moduleUnit);

   // Make a closure for the assemlby in the map file

   logger->EndAssemblyMap();

}

Phx::Phases::PhaseConfiguration ^
Instrumentor::PreparePhaseList
(
   Phx::Lifetime ^ lifetime
)
{
   Phx::Phases::PhaseConfiguration ^ config = Phx::Phases::PhaseConfiguration::New(lifetime,
      "BBCount Phases");

   // Add phases...

   config->PhaseList->AppendPhase(
      Phx::PE::ReaderPhase::New(config));

   {
      // Create the per-function phase list....

      Phx::Phases::PhaseList ^ unitList =
         Phx::PE::UnitListPhaseList::New(config,
            Phx::PE::UnitListWalkOrder::PrePass);

      unitList->AppendPhase(
         Phx::PE::RaiseIRPhase::New(config,
            Phx::FunctionUnit::LowLevelIRBeforeLayoutFunctionUnitState));

      // HERE: change this phase if you want to instrument at function
      // or basic block level

      unitList->AppendPhase(InstrumentPhase::New(config, logger));

      unitList->AppendPhase(EncodePhase::New(config));
      unitList->AppendPhase(Phx::PE::DiscardIRPhase::New(config));

      config->PhaseList->AppendPhase(unitList);
   }

   // HERE: change this phase if you want to instrument program start point
   // Right now there is no instrumentation for program start point
   //lastPhase = InstrumentPhase::New(config, lastPhase);

   config->PhaseList->AppendPhase(EmitPhase::New(config));

   // Add any plugins or target-specific phases.

   Phx::GlobalData::BuildPlugInPhases(config);

   return config;
}

void
Instrumentor::InstrumentPhase::Initialize()
{

#if (PHX_DEBUG_SUPPORT)

   InstrumentPhaseControl = Phx::Controls::ComponentControl::New("bbcount",
      "Inject bb counting into each method", "bbcount.cs");

#endif
   currentId = 0;
   funcProtoFactory = gcnew Hashtable();
   funcDisassembly = gcnew Queue();
}

//------------------------------------------------------------------------
//
// Description:
//
//    The static constructor of InstrumentPhase
//
//-----------------------------------------------------------------------

Instrumentor::InstrumentPhase ^
Instrumentor::InstrumentPhase::New
(
   Phx::Phases::PhaseConfiguration ^ config,
   Logger ^                   logger
)
{
   InstrumentPhase ^ instrumentPhase = gcnew InstrumentPhase();

   instrumentPhase->Initialize(config, "BBCount Instrumentation");

#if (PHX_DEBUG_SUPPORT)

   instrumentPhase->PhaseControl = InstrumentPhaseControl;

#endif

   instrumentPhase->logger = logger;

   return instrumentPhase;
}

//-------------------------------------------------------------------
//
// Description:
//
//    Construct a new instance of FuncPrototype based on the given
//    method signature, create a functionSymbol for the imported method
//
// Remarks:
//
//    Method signature is in format:
//
//       [AssemblyName]ClassName::MethodName(argumentType1,[argumentType2,..])
//
//       where argumentType is one of ParamType defined above
//
//    For example, the signature used in this sample:
//
//       [RtWrapper]RtWrapper::BBCount(zUINT)
//
//-------------------------------------------------------------------

Instrumentor::InstrumentPhase::FuncPrototype::FuncPrototype
(
   Phx::PEModuleUnit ^ module,
   String ^            funcSig
)
{

   // parsing the function signature
   // Format: [assembly]class.method(param1type,paramt2ype,...)

   int idx1 = funcSig->IndexOf('[');
   int idx2 = funcSig->IndexOf(']');

   String ^ assemblyName = funcSig->Substring(idx1 + 1, idx2 - idx1 - 1);

   int      idx3 = funcSig->LastIndexOf('.');
   String ^ className = funcSig->Substring(idx2 + 1, idx3 - idx2 - 1);

   int      idx4 = funcSig->IndexOf('(');
   String ^ methodName = funcSig->Substring(idx3 + 1, idx4 - idx3 - 1);

   // parse the parameter type list

   int idx5 = funcSig->LastIndexOf(')');
   array<wchar_t> ^  dem = gcnew array<wchar_t>{','};
   array<String ^> ^ splits =
      (funcSig->Substring(idx4 + 1, idx5 - idx4 - 1))->Split(dem);

   int argc = splits->Length;

   argumentTypes = gcnew array<ParamType ^>(argc);

   for (int i = 0; i < argc; i++)
   {
      argumentTypes[i] = dynamic_cast<ParamType ^>(Enum::Parse(ParamType::typeid,
         splits[i]->Trim()));
   }

   // translate the ParamType to internal phx Type

   array<Phx::Types::Type ^> ^ internalArgTypes =
      gcnew array<Phx::Types::Type ^>(argc);

   for (int i = 0; i < argc; i++)
   {
      switch(*argumentTypes[i])
      {
         case ParamType::zCHAR:

            internalArgTypes[i] = module->TypeTable->Character16Type;
            break;

         case ParamType::zINT:

            internalArgTypes[i] = module->TypeTable->Int32Type;
            break;

         case ParamType::zUINT:

            internalArgTypes[i] = module->TypeTable->UInt32Type;
            break;

         case ParamType::zLONG:

            internalArgTypes[i] = module->TypeTable->Int64Type;
            break;

         case ParamType::zULONG:

            internalArgTypes[i] = module->TypeTable->UInt64Type;
            break;

         case ParamType::zCHARPTR:

            internalArgTypes[i] = module->TypeTable->GetObjectPointerType(
               module->TypeTable->SystemStringAggregateType);
            break;

         default:

            Console::WriteLine(
               "Unsupported type for parameters of callback function");
            throw(gcnew Exception());
      }
   }

   // Import the related Symbols of this function prototype

   functionSymbol = CreateFunctionSymbol(module, assemblyName, className, methodName,
      internalArgTypes);
}

//------------------------------------------------------------------
//
// Description:
//
//    A helper function that creates a functionSymbol for the inserted call
//
//------------------------------------------------------------------

Phx::Symbols::FunctionSymbol ^
Instrumentor::InstrumentPhase::FuncPrototype::CreateFunctionSymbol
(
   Phx::PEModuleUnit ^         moduleUnit,
   String ^                    assemblyName,
   String ^                    className,
   String ^                    methodName,
   array<Phx::Types::Type ^> ^ argumentTypeList
)
{
   // (1) -- get a Symbol reference to the Assembly.

   Phx::Symbols::AssemblySymbol ^ assemblySymbol =
      FindOrCreateAssemblySym(moduleUnit, assemblyName);

   // (2) -- get a Symbol reference to the class

   Phx::Symbols::MsilTypeSymbol ^ classTypeSym =
      FindOrCreateClassSym(moduleUnit, className, assemblySymbol);

   // (3) -- create a type for the class

   Phx::Types::AggregateType ^ classType =
      Phx::Types::AggregateType::NewDynamicSize(moduleUnit->TypeTable, classTypeSym);

   classType->IsDefinition = false;

   // (4) -- Create the symbol reference for the inserted method.

   Phx::Types::FunctionTypeBuilder ^ functionTypeBuilder =
      Phx::Types::FunctionTypeBuilder::New(moduleUnit->TypeTable);

   functionTypeBuilder->Begin();

   // Set the calling convention

   functionTypeBuilder->CallingConventionKind =
      Phx::Types::CallingConventionKind::ClrCall;

   // Set the return type - instrumented call always return void

   functionTypeBuilder->AppendReturnParameter(moduleUnit->TypeTable->VoidType);

   // Set the param types..

   for (int i = 0; i < argumentTypeList->Length; i++)
   {
      functionTypeBuilder->AppendParameter(argumentTypeList[i]);
   }

   Phx::Types::FunctionType ^ functionType = functionTypeBuilder->GetFunctionType();

   Phx::Name functionName = Phx::Name::New(moduleUnit->Lifetime, methodName);

   functionSymbol = Phx::Symbols::FunctionSymbol::New(moduleUnit->SymbolTable, 0, functionName,
      functionType, Phx::Symbols::Visibility::GlobalReference);
   classTypeSym->InsertInLexicalScope(functionSymbol, functionName);

   // Add it as a method of classType.

   classType->AppendMethodSymbol(functionSymbol);

   return functionSymbol;
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Find or create a sym reference to a named assembly.
//
//--------------------------------------------------------------------------

Phx::Symbols::AssemblySymbol ^
Instrumentor::InstrumentPhase::FuncPrototype::FindOrCreateAssemblySym
(
   Phx::ModuleUnit ^ moduleUnit,
   String ^          assemblyName
)
{
   // A more general API that creates a Symbol given the path name of an assembly
   // or module should be used.

   // For now, we assume the assembly is created in default way
   // by csc without publictoken in it. BCL assemblies are not
   // supported yet.

   // We first look through all the existing assembly
   // references to see if this assembly is already there.


   Phx::Symbols::AssemblySymbol ^ referenceSymbol = nullptr;

   for each(Phx::Symbols::Symbol ^ symbol in moduleUnit->SymbolTable->AllSymbols)
   {
      Phx::Symbols::AssemblySymbol ^ assemblySymbol =
         dynamic_cast<Phx::Symbols::AssemblySymbol ^>(symbol);

      if (assemblySymbol != nullptr)
      {
         if (assemblySymbol->NameString->Equals(assemblyName))
         {
            referenceSymbol = assemblySymbol;
            break;
         }
      }
   }

   // The assemblySymbol is not created yet

   if (referenceSymbol == nullptr)
   {
      // TODO: Hard coded creation of a manifest. Should be other way. Add
      // more general way once API for manifest is available

      Phx::Name       dllName =
         Phx::Name::New(moduleUnit->Lifetime, assemblyName);
      Phx::Manifest ^ dllManifest = Phx::Manifest::New(moduleUnit->Lifetime);

      dllManifest->Name = dllName;
      dllManifest->HashAlgorithm = 0x00008004;

      Phx::Version ^ version = Phx::Version::New(moduleUnit->Lifetime);
      version->BuildNumber = 0;
      version->MajorVersion = 0;
      version->MinorVersion = 0;
      version->RevisionNumber = 0;
      dllManifest->Version = version;

      referenceSymbol = Phx::Symbols::AssemblySymbol::New(nullptr,
         dllManifest, dllName, moduleUnit->SymbolTable);
   }

   return referenceSymbol;
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Find or create a sym reference to a named class.
//
//--------------------------------------------------------------------------

Phx::Symbols::MsilTypeSymbol ^
Instrumentor::InstrumentPhase::FuncPrototype::FindOrCreateClassSym
(
   Phx::ModuleUnit ^        moduleUnit,
   String ^                 className,
   Phx::Symbols::AssemblySymbol ^ assemblySymbol
)
{
   Phx::Symbols::MsilTypeSymbol ^ classTypeSym = nullptr;

   for each(Phx::Symbols::Symbol ^ symbol in moduleUnit->SymbolTable->AllSymbols)
   {
      Phx::Symbols::MsilTypeSymbol ^ tmpSym =
         dynamic_cast<Phx::Symbols::MsilTypeSymbol ^>(symbol);

      if (tmpSym != nullptr)
      {
         if (tmpSym->NameString->Equals(className))
         {
            classTypeSym = tmpSym;
            break;
         }
      }
   }

   // the msil type symbol has not been created yet

   if (classTypeSym == nullptr)
   {

      Phx::Name classTypeName = Phx::Name::New(
         moduleUnit->Lifetime, className);

      classTypeSym = Phx::Symbols::MsilTypeSymbol::New(
         moduleUnit->SymbolTable, classTypeName, 0, nullptr, nullptr);

      // Now, insert the class type to the scope of system assembly.

      assemblySymbol->InsertInLexicalScope(classTypeSym, classTypeName);

   }

   return classTypeSym;

}

//---------------------------------------------------------------------
//
// Description:
//
//    If the FuncPrototype is already created in the hash table,
//    the hashed FuncPrototype is returned. Otherwise, a new instance
//    of FuncPrototype is created according to the given function
//    signature.
//
// Returns:
//
//    The FuncPrototype object that is according to the given function
//    signature.
//
//---------------------------------------------------------------------

Instrumentor::InstrumentPhase::FuncPrototype ^
Instrumentor::InstrumentPhase::FindOrCreateFuncProto
(
   Phx::PEModuleUnit ^ moduleUnit,
   String ^            funcSig
)
{
   String ^ key = moduleUnit->NameString + funcSig;

   FuncPrototype ^ fproto = (FuncPrototype ^) funcProtoFactory[key];

   if (fproto == nullptr)
   {
      fproto = gcnew FuncPrototype(moduleUnit, key);
      funcProtoFactory[key] = fproto;
   }

   return fproto;
}

#if defined(PHX_API_NYI)
//---------------------------------------------------------------------
//
// Description:
//
//    A helper function to insert instructions that implement a call
//    to the method represented by the given FuncPrototype
//
//---------------------------------------------------------------------

void
Instrumentor::InstrumentPhase::InsertCallInstruction
(
   Phx::FunctionUnit ^       functionUnit,
   FuncPrototype ^           fproto, // Prototype for call to insert
   array<System::Object ^> ^ arguments, // Call arguments
   Phx::IR::Instruction ^    startInstruction // Insert before this
)
{
// This snippet is used in ~doc\guides\spec_template\ScenarioTemplate.doc
// It is programmed against a theoretical, easier Phoenix API.
   // Begin Snippet InsertCallAfter
   Phx::Types::Table ^ typeTable =
      functionUnit->ParentUnit->TypeTable;

   // create instructions that will push arguments onto stack

   array<ParamType ^> ^ paramTypeList = fproto->argTypes;

   Phx::Lifetime tmpLifetime;

   // Create a list of operands to be passed as arguments to the call.

   Phx::Collections::OperandList ^ operandList = 
      Phx::Collections::OperandList::New(tmpLifetime);

   for (int i = 0; i < arguments->Length; i++)
   {
      Phx::IR::Operand ^ argumentOperand;

      switch (*paramTypeList[i])
      {
         case ParamType::zCHAR:
         {
            argumentOperand = Phx::IR::ImmediateOperand::New(functionUnit,
               typeTable->Int32Type, (char)arguments[i]);
            operandList->Append(argumentOperand);

            break;
         }

         case ParamType::zUINT:
         {
            argumentOperand = Phx::IR::ImmediateOperand::New(functionUnit,
               typeTable->UInt32Type, (unsigned int)arguments[i]);
            operandList->Append(argumentOperand);

            break;
         }

         // <snipped for brevity> Handle zINT, zLONG, zULONG ... 

         case ParamType::zCHARPTR:
         {
            String ^ newString = (String ^) arguments[i];

            Phx::Syms::ConstantSymbol ^ stringSymbol = 
               Phx::Syms::ConstantSymbol::NewString(
                  functionUnit->ParentUnit->SymTable, newString);

            // Construct an address of a string in the string pool.

            argumentOperand = Phx::IR::MemoryOperand::NewAddressReadOnly(
               functionUnit, stringSymbol);
            operandList->Append(argumentOperand);

            break;
         }

         default:

            Console::WriteLine(
               "Unsupported primitive type for parameters of callback function")
               ;
            throw gcnew Exception();
      }
   }

   // Create a call and the instructions to load its operands.

   Phx::IR::InstructionRange callInstructions = 
      IR::InstructionRange::NewCall(functionUnit, fproto->funcSym, operandList);

   IR::InstructionRange::InsertBefore(startInstruction, callInstructions);
   // End Snippet InsertCallAfter
}


#endif

//---------------------------------------------------------------------
//
// Description:
//
//    A helper function to insert instructions that implement a call
//    to the method represented by the given FuncPrototype
//
//---------------------------------------------------------------------
// Begin Snippet InsertCallBefore
void
Instrumentor::InstrumentPhase::InsertCallInstr
(
   Phx::FunctionUnit ^           functionUnit,
   FuncPrototype ^           fproto,    // FuncPrototype for the inserted call
   array<System::Object ^> ^ arguments,      // the real argument, might be boxed
   Phx::IR::Instruction ^          startInstruction // [AGH0] Types should not be abbrev.

   // Insert new instruction before this one

)
{
   Phx::Types::Table ^ typeTable =
      functionUnit->ParentUnit->TypeTable;

   if (functionUnit->FlowGraph == nullptr)
   {
      functionUnit->BuildFlowGraph();
   }

   // create the call instruction
   // ISSUE-API-AHoskins-2007/2/20-#199274 It is unnatural to create a call without its arguments and then
   // append them.

   Phx::IR::Instruction ^ call = Phx::IR::CallInstruction::New(functionUnit,
      Phx::Targets::Architectures::Msil::Opcode::call, fproto->functionSymbol);

   Phx::IR::Instruction ^ ldInstr;

   // create instructions that will push arguments onto stack

   array<ParamType ^> ^ paramTypeList = fproto->argumentTypes;

   for (int i = 0; i < arguments->Length; i++)
   {
      Phx::IR::Operand ^ argumentOperand;

      switch (*paramTypeList[i])
      {
         case ParamType::zCHAR:
         {
            argumentOperand = Phx::IR::ImmediateOperand::New(functionUnit, typeTable->Int32Type,
               (char)arguments[i]);

            // ISSUE-API-AHoskins-2007/2/20-#199274 Can we avoid making the user do this work?  We should
            // provide architecture-agnostic ways of passing arguments to 
            // calls.

            ldInstr = Phx::IR::ValueInstruction::NewUnaryExpression(functionUnit,
               Phx::Targets::Architectures::Msil::Opcode::ldc, typeTable->Int32Type,
               argumentOperand);

            break;
         }

         case ParamType::zINT:
         {
            argumentOperand = Phx::IR::ImmediateOperand::New(functionUnit, typeTable->Int32Type,
               (int)arguments[i]);

            ldInstr = Phx::IR::ValueInstruction::NewUnaryExpression(functionUnit,
               Phx::Targets::Architectures::Msil::Opcode::ldc, typeTable->Int32Type,
               argumentOperand);

            break;

         }

         case ParamType::zUINT:
         {
            argumentOperand = Phx::IR::ImmediateOperand::New(functionUnit, typeTable->UInt32Type,
               (unsigned int)arguments[i]);

            ldInstr = Phx::IR::ValueInstruction::NewUnaryExpression(functionUnit,
               Phx::Targets::Architectures::Msil::Opcode::ldc, typeTable->UInt32Type,
               argumentOperand);

            break;

         }

         case ParamType::zLONG:
         {
            argumentOperand = Phx::IR::ImmediateOperand::New(functionUnit, typeTable->Int64Type,
               (long)arguments[i]);

            ldInstr = Phx::IR::ValueInstruction::NewUnaryExpression(functionUnit,
               Phx::Targets::Architectures::Msil::Opcode::ldc, typeTable->Int64Type,
               argumentOperand);

            break;
         }

         case ParamType::zULONG:
         {
            argumentOperand = Phx::IR::ImmediateOperand::New(functionUnit, typeTable->UInt64Type,
               (unsigned __int64)arguments[i]);

            ldInstr = Phx::IR::ValueInstruction::NewUnaryExpression(functionUnit,
               Phx::Targets::Architectures::Msil::Opcode::ldc, typeTable->UInt64Type,
               argumentOperand);

            break;
         }

         case ParamType::zCHARPTR:
         {
            String ^ newString = (String ^) arguments[i];

            // ISSUE-API-AHoskins-2007/2/20-#199274 This is a lot of abstractions just to create a string!

            Phx::Types::Type ^ stringType = typeTable->GetObjectPointerType(
               typeTable->SystemStringAggregateType);

            Phx::Name newStringName = Phx::Name::New(
               Phx::GlobalData::GlobalLifetime, newString);

            Phx::Symbols::ConstantSymbol ^ stringSym = Phx::Symbols::ConstantSymbol::New(
               functionUnit->ParentUnit->SymbolTable,
               0,
               newStringName,
               typeTable->SystemStringAggregateType,
               newString);

            // ISSUE-API-AHoskins-2007/2/20-#199274 This is our most complicated constructor.  
            // It is not readable.  Is the memory in heap, stack, or static?

            argumentOperand = Phx::IR::MemoryOperand::NewAddress(functionUnit,
               stringType, stringSym, nullptr, 0,
               Phx::Alignment::NaturalAlignment(stringType),
               functionUnit->AliasInfo->NotAliasedMemoryTag);

            ldInstr = Phx::IR::ValueInstruction::NewUnaryExpression(functionUnit,
               Phx::Targets::Architectures::Msil::Opcode::ldstr, stringType, argumentOperand);

            break;
         }

         default:

            Console::WriteLine(
               "Unsupported primitive type for parameters of callback function")
               ;
            throw gcnew Exception();
      }

      // ISSUE-API-AHoskins-2007/2/20-#199274 The user should not need to specify the DstOpnd link.

      ldInstr->DestinationOperand->Register = Phx::Targets::Architectures::Msil::Register::SR0;

      // ISSUE-API-AHoskins-2007/2/20-#199274 Insert into what?  Means insert ldInstr before 
      // startInstr, but that is not how it reads in English.

      startInstruction->InsertBefore(ldInstr);
      call->AppendSource(ldInstr->DestinationOperand);

  
      ldInstr->DestinationOperand->BreakExpressionTemporary();

      // ISSUE-API-AHoskins-2007/2/20-#199274 The user is having to maintain an invariant manually.

      ldInstr->DebugTag = startInstruction->DebugTag;
   }

   startInstruction->InsertBefore(call);
}
// End Snippet InsertCallBefore

void
Instrumentor::InstrumentPhase::Execute
(
   Phx::Unit ^ unit
)
{
   // unit->Dump();

   if (!(unit->IsFunctionUnit) && !(unit->IsPEModuleUnit))
   {
      return;
   }

   if (unit->IsPEModuleUnit)
   {
      // HERE: Insert your instrumentation at the PEModule level

      // For example: instrument some initialization of the profiling
      // runtime. For this sample, no instrumentation at this point

      return;
   }

   Phx::FunctionUnit ^ functionUnit = unit->AsFunctionUnit;

   // Only handle Msil functions.

   if (!functionUnit->Architecture->NameString->Equals("Msil"))
   {
      return;
   }

   Phx::PEModuleUnit ^ moduleUnit = functionUnit->ParentUnit->AsPEModuleUnit;

   // HERE: insert your instrumentation at the function level
   // For this sample, nothing is instrumented at this point

   // Build a less ambiguous signature to use instead of the
   // bare method name.

   Phx::Symbols::FunctionSymbol ^   functionSymbol = functionUnit->FunctionSymbol;
   Phx::Types::FunctionType ^ functionType = functionSymbol->FunctionType;

   String ^ methodName = functionSymbol->NameString + "(";
   bool first = true;
   for each (Phx::Types::Parameter parameter in functionType->UserDefinedParameters)
   {
      if (first)
      {
         first = false;
      }
      else
      {
         methodName += ", ";
      }

      methodName += parameter.Type->ToString();
   }
   methodName += ")";

   // Build the control flow graph for the current function

   functionUnit->BuildFlowGraph();
   Phx::Graphs::FlowGraph ^ fg = functionUnit->FlowGraph;

   // dump method info to the map file

   String ^ className = "<Module>";

   if (functionSymbol->EnclosingAggregateType != nullptr)
   {
      className = functionSymbol->EnclosingAggregateType->TypeSymbol->NameString;
   }

   logger->StartMethodMap(
      methodName, className,
      0, // we can't get the size of locals, yet.
      0, // we can't get the size of stack, yet.
      functionUnit->InstructionId, // the size of the method (in instructions)
      fg->StartBlock->FirstInstruction->GetFileName(),
      fg->StartBlock->FirstInstruction->GetLineNumber());

   // Instrument and dump the blocks for the current function.

   logger->StartBlocksMap();

   // design_template.doc
   // Begin Snippet MainLoop

   for each (Phx::Graphs::BasicBlock ^ block in fg->BasicBlocks)
   {
      // Find the first real instruction in the block

      Phx::IR::Instruction ^ firstInstruction = nullptr;

      for each (Phx::IR::Instruction ^ instruction in block->Instructions)
      {
         // Set asside each instruction for later disassembly.

         funcDisassembly->Enqueue(instruction);

         // Remember the first real instruction in the block.

         if ((firstInstruction == nullptr) && instruction->IsReal)
         {
            Phx::Common::Opcode ^ opcode =
               dynamic_cast<Phx::Common::Opcode ^>(instruction->Opcode);

            // Some real instructions aren't really real.

            if ((opcode == Phx::Common::Opcode::ReturnFinally)
               || (opcode == Phx::Common::Opcode::Leave)
               || (opcode == Phx::Common::Opcode::Unreached)
               || (opcode == Phx::Common::Opcode::ExitTypeFilter))
            {
               continue;
            }

            firstInstruction = instruction;
         }
      }

      if (firstInstruction != nullptr)
      {
         // HERE: insert your instrumentation at BB level
         // For this sample, we insert basic block counting routines

         // dump mapping information of current basic block

         logger->DumpBlockMap(currentId, firstInstruction->GetMsilOffset(),
            firstInstruction->GetFileName(), firstInstruction->GetLineNumber());

         // creae the FuncPrototype of the inserted call

         FuncPrototype ^ fproto = FindOrCreateFuncProto(moduleUnit,
            "[RtWrapper]RtWrapper.BBCount(zUINT)");

         // create the call instruction

         // set the parameter list

         array<System::Object ^> ^ arguments =
            gcnew array<System::Object ^>{currentId};

         InsertCallInstr(functionUnit, fproto, arguments, firstInstruction);

         // Dump the disassembly for the current block.

         if (CmdLineParser::dumpPerBlockDisassembly)
         {
            logger->StartBlockDisassemblyMap();

            for each (Phx::IR::Instruction ^ instruction in block->Instructions)
            {
               logger->DumpDisasmMap(instruction->GetMsilOffset(),
                  instruction->ToString(), instruction->GetFileName(),
                  instruction->GetLineNumber());
            }

            logger->EndBlockDisassemblyMap();
         }

         logger->EndBlockMap();

         // Increment the currentId of basic blocks

         currentId++;
      }
   }

   // End Snippet MainLoop

   functionUnit->DeleteFlowGraph();

   logger->EndBlocksMap();

   // Dump the disassembly for the current function.

   logger->StartMethodDisassemblyMap();

   while (0 < funcDisassembly->Count)
   {
      Phx::IR::Instruction ^ instruction =
         safe_cast<Phx::IR::Instruction ^>(funcDisassembly->Dequeue());
      logger->DumpDisasmMap(instruction->GetMsilOffset(), instruction->ToString(),
         instruction->GetFileName(), instruction->GetLineNumber());
   }

   logger->EndMethodDisassemblyMap();
   logger->EndMethodMap();
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Encode the modified function.
//
//--------------------------------------------------------------------------

Instrumentor::EncodePhase ^
Instrumentor::EncodePhase::New
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   EncodePhase ^ encodePhase = gcnew EncodePhase();

   encodePhase->Initialize(config, "Encode");

   return encodePhase;
}

void
Instrumentor::EncodePhase::Execute
(
   Phx::Unit ^ unit
)
{
   Phx::PE::Writer::EncodeUnit(unit);
}

Instrumentor::EmitPhase ^
Instrumentor::EmitPhase::New
(
   Phx::Phases::PhaseConfiguration ^ config
)
{
   EmitPhase ^ emitPhase = gcnew EmitPhase();

   emitPhase->Initialize(config, "Emit");

   return emitPhase;
}

void
Instrumentor::EmitPhase::Execute
(
   Phx::Unit ^ unit
)
{
   if (!unit->IsPEModuleUnit)
   {
      return;
   }

   Phx::PEModuleUnit ^ peModule = unit->AsPEModuleUnit;

   String ^ inputFileNameStr = peModule->NameString;

   String ^ outputFileNameString = CmdLineParser::outDir +
      Path::GetFileName(inputFileNameStr);

   String ^ outputPDBNameStr = nullptr;

   if (CmdLineParser::pdbOut)
   {
      outputPDBNameStr = CmdLineParser::outDir +
         Path::GetFileNameWithoutExtension(inputFileNameStr) + ".pdb";
   }

   Phx::PE::Writer ^ writer = Phx::PE::Writer::New(
      Phx::GlobalData::GlobalLifetime,
      outputFileNameString,
      outputPDBNameStr,
      peModule,
      peModule->SymbolTable,
      peModule->Architecture,
      peModule->Runtime);

   writer->Write();
}

Logger::Logger
(
   String ^ mapFile,
   String ^ cntFile
)
{
   mapWriter = gcnew XmlTextWriter(mapFile, nullptr);
   cntWriter = gcnew StreamWriter(File::OpenWrite(cntFile));
   methodSrcFile = "";
   methodSrcLine = 0;
   srcFile = "";
   srcLine = 0;
}

void
Logger::Close()
{
   mapWriter->Close();
   cntWriter->Close();
}

void
Logger::StartMap()
{
   // write out the xml header

   mapWriter->WriteStartDocument(true);
   mapWriter->Formatting = Formatting::Indented;
   mapWriter->Indentation = 3;

   mapWriter->WriteStartElement("assemblies");
}

void
Logger::EndMap()
{
   mapWriter->WriteEndElement();  // for the <assemblies> tag
   mapWriter->WriteEndDocument();
}

void
Logger::StartAssemblyMap
(
   String ^ assemblyName
)
{
   mapWriter->WriteStartElement("assembly");
   mapWriter->WriteAttributeString("name", assemblyName);
}

void
Logger::EndAssemblyMap()
{
   mapWriter->WriteEndElement(); // for the <assembly> tag
}

void
Logger::StartMethodMap
(
   String ^     methodName,
   String ^     className,
   unsigned int nLocal,
   unsigned int nStack,
   unsigned int nInstr,
   String ^     file,
   unsigned int line
)
{
   SetMethodSrcFileAndLine(file, line);

   mapWriter->WriteStartElement("method");

   mapWriter->WriteAttributeString("name", methodName);

   mapWriter->WriteAttributeString("class", className);

   mapWriter->WriteAttributeString("locals",
      String::Format("{0}", nLocal));

   mapWriter->WriteAttributeString("stack",
      String::Format("{0}", nStack));

   mapWriter->WriteAttributeString("size",
      String::Format("{0}", nInstr));

   WriteMethodSrcFileAndLine();
}

void
Logger::EndMethodMap()
{
   mapWriter->WriteEndElement(); //for the <method> tag
}

void
Logger::StartBlocksMap()
{
   mapWriter->WriteStartElement("blocks");
   WriteMethodSrcFileAndLine();
}

void
Logger::EndBlocksMap()
{
   mapWriter->WriteEndElement(); //for the <blocks> tag
}

void
Logger::DumpBlockMap
(
   unsigned int id,
   unsigned int offset,
   String ^     file,
   unsigned int line
)
{
   mapWriter->WriteStartElement("block");
   mapWriter->WriteAttributeString("id", String::Format("{0}", id));
   mapWriter->WriteAttributeString("count", "0");
   mapWriter->WriteAttributeString("offset", String::Format("{0}", offset));
   WriteSrcFileAndLine(file, line);
}

void
Logger::EndBlockMap()
{
   mapWriter->WriteEndElement(); //for the <block> tag
}

void
Logger::StartBlockDisassemblyMap()
{
   mapWriter->WriteStartElement("blk-disassembler");
}

void
Logger::EndBlockDisassemblyMap()
{
   mapWriter->WriteEndElement(); //for the <blk-disassembler> tag
}

void
Logger::StartMethodDisassemblyMap()
{
   mapWriter->WriteStartElement("disassembly");
   WriteMethodSrcFileAndLine();
}

void
Logger::EndMethodDisassemblyMap()
{
   mapWriter->WriteEndElement(); //for the <disassembly> tag
}

void
Logger::DumpDisasmMap
(
   unsigned int offset,
   String ^     disassembler,
   String ^     file,
   unsigned int line
)
{
   array<wchar_t> ^  nl = gcnew array<wchar_t>{'\n'};
   array<String ^> ^ disasmLines = disassembler->Split(nl);

   for (int i = 0; i < disasmLines->Length; i += 1)
   {
      mapWriter->WriteStartElement("disassembler");
      mapWriter->WriteAttributeString("offset", String::Format("{0,5}",
            offset));
      ConditionallyWriteSrcFileAndOrLine(file, line);
      mapWriter->WriteString(disasmLines[i]->Trim());
      mapWriter->WriteEndElement();
   }
}

// Reset the [current] method's source file and line number.

void
Logger::SetMethodSrcFileAndLine
(
   String ^     file,
   unsigned int line
)
{
   methodSrcFile = file;
   methodSrcLine = line;

   srcFile = "";
   srcLine = 0;
}

// Write the source file and line number attributes.

void
Logger::WriteMethodSrcFileAndLine()
{
   WriteSrcFileAndLine(methodSrcFile, methodSrcLine);
}

void
Logger::WriteSrcFileAndLine
(
   String ^     file,
   unsigned int line
)
{
   srcFile = file;
   srcLine = line;

   mapWriter->WriteAttributeString("srcFile", String::Format("{0}", srcFile));
   mapWriter->WriteAttributeString("srcLine", String::Format("{0}", srcLine));
}

// Conditionally write the [current] source file and/or line number
// attributes.

void
Logger::ConditionallyWriteSrcFileAndOrLine
(
   String ^     file,
   unsigned int line
)
{
   if (srcFile != file)
   {
      mapWriter->WriteAttributeString("srcFile", String::Format("{0}", file));
      mapWriter->WriteAttributeString("srcLine", String::Format("{0}", line));
      srcFile = file;
      srcLine = line;
   }
   else if (srcLine != line)
   {
      mapWriter->WriteAttributeString("srcLine", String::Format("{0}", line));
      srcLine = line;
   }
}

// Report the total number of basic blocks we visited during
// instrumentation and put it in a file so that the profiling
// runtime could use that for initialization

void
Logger::ReportBBCnt
(
   unsigned int totalBBNumber
)
{
   cntWriter->Write(totalBBNumber.ToString());
}

//--------------------------------------------------------------------------
//
// Description:
//
//    A control that capture the command line arguments that do not
//    match any other controls. Here we use it to capture the list
//    of assemblies to be instrumented.
//
//--------------------------------------------------------------------------

void
CmdLineParser::DefaultControl::Process
(
   String ^ string
)
{
   if (inFiles == nullptr)
   {
      inFiles = string;
   }
   else
   {
      inFiles = inFiles + ";" + string;
   }
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Parse and check the commandline variables
//
// Returns:
//
//    Nothing
//
//-----------------------------------------------------------------------------

Phx::Term::Mode
CmdLineParser::ParseCmdLine
(
   array<String ^> ^ argv
)
{
   DefaultControl ^ defaultControl = gcnew DefaultControl();
   Phx::Controls::Parser::RegisterDefaultControl(defaultControl);

   Phx::Controls::StringControl ^ outdirCtrl = Phx::Controls::StringControl::New(
      "outdir", "instrumented\\",
      "the directory to put the instrumented assemblies", "bbcount.cpp");

   Phx::Controls::StringControl ^ mapCtrl = Phx::Controls::StringControl::New(
      "map", "bbcount.map", "intrument map file", "bbcount.cpp");

   Phx::Controls::StringControl ^ cntCtrl = Phx::Controls::StringControl::New(
      "cnt", "bbcount.cnt", "basic block count file", "bbcount.cpp");

   Phx::Controls::SetBooleanControl ^ verboseCtrl = Phx::Controls::SetBooleanControl::New(
      "v", "verbose flag", "bbcount.cpp");

   Phx::Controls::SetBooleanControl ^ dumpPerBlockDisassemblyCtrl =
      Phx::Controls::SetBooleanControl::New("d", "dump per-block disassembly flag",
         "bbcount.cpp");

   Phx::Controls::SetBooleanControl ^ pdbOutCtrl = Phx::Controls::SetBooleanControl::New(
      "pdb", "output updated pdb", "bbcount.cpp");

   // Check for Phoenix wide options first

   Phx::Initialize::EndInitialization("PHX|*|_PHX_|", argv);

   if (inFiles == nullptr)
   {
      Usage();
      return Phx::Term::Mode::Fatal;
   }

   outDir = Path::GetFullPath(outdirCtrl->GetValue(nullptr));
   if (!outDir->EndsWith("\\"))
   {
      outDir = outDir + "\\";
   }

   DirectoryInfo ^ outDirInfo = gcnew DirectoryInfo(outDir);
   if (!outDirInfo->Exists)
   {
      outDirInfo->Create();
   }

   mapFile = mapCtrl->GetValue(nullptr);
   cntFile = cntCtrl->GetValue(nullptr);
   verbose = verboseCtrl->GetValue(nullptr);
   pdbOut  = pdbOutCtrl->GetValue(nullptr);
   dumpPerBlockDisassembly = dumpPerBlockDisassemblyCtrl->IsEnabled(nullptr);

   return Phx::Term::Mode::Normal;
}

//-----------------------------------------------------------------------------
//
// Description:
//
//    Print the usage String ^ to the console
//
// Returns:
//
//    Nothing
//
//-----------------------------------------------------------------------------

void
CmdLineParser::Usage()
{
   String ^ usage =
      L"Usage: BBCount [/outdir <output-file-name>] [/pdb] [/map <map-name>]\n"
      L"               [/cnt <count-name>] [/v] [/d] <src-assembly-list>\n"
      L"\n"
      L"       Perform basic-block instrumentation on an Msil image.\n"
      L"       Arguments in [] are optional.\n"
      L"\n"
      L"       /v:   verbose\n"
      L"       /d:   dump per-block disassembly\n"
      L"       /outdir: directory to hold the instrumented Msil image\n"
      L"             Default: current directory plus \\instrumented\n"
      L"       /pdb: output an updated pdb to the output directory\n"
      L"       /map: name of map where you want to keep the mapping information.\n"
      L"             Default: bbcount.map\n"
      L"       /cnt: specify the file where you want to keep the total number\n"
      L"             of basic blocks.\n"
      L"             Default: bbcount.cnt";

   Console::WriteLine(usage);
}

//--------------------------------------------------------------------------
//
// Description:
//
//    Entry point for application
//
// Remarks:
//
//    The order of initialization matters. Please initializing
//    Phoenix first, then parsing command line, then initializing
//    the Instrumentor, and finally do the real work
//
//--------------------------------------------------------------------------

int
main
(
   array<String ^> ^ argv      // command line argument strings
)
{
   Phx::Term::Mode termMode = Phx::Term::Mode::Normal;

   try
   {
      // (1) -- Initializing the Phoenix framework.
      // Always do this before anything else !!

      Instrumentor::InitializePhx();

      // (2) -- Parsing command line options

      termMode = CmdLineParser::ParseCmdLine(argv);

      if (termMode == Phx::Term::Mode::Fatal)
      {
         Phx::Term::All(termMode);
         return ((termMode == Phx::Term::Mode::Normal) ? 0 : 1);
      }

      // (3) -- Create an instance of Logger class that will be used
      //        for dumping out mapping information

      Logger ^ logger = gcnew Logger(CmdLineParser::mapFile,
         CmdLineParser::cntFile);

      // (4) -- Initialize the Intrumentor

      Instrumentor::Initialize(logger);

      // (5) -- Do the real intrumentation work

      // process the input assemblies one by one

      logger->StartMap();

      array<wchar_t> ^  delim = gcnew array<wchar_t>{';'};
      array<String ^> ^ images = CmdLineParser::inFiles->Split(delim);
      for (int i = 0; i < images->Length; i++)
      {
         Instrumentor::Process(images[i]);
      }

      logger->EndMap();

      // (6) -- Report the total number of basic blocks processed

      unsigned int totalBBNumber = Instrumentor::InstrumentPhase::currentId;
      if (CmdLineParser::verbose)
      {
         Console::WriteLine("There are {0} basic blocks total.",
            totalBBNumber);
      }

      logger->ReportBBCnt(totalBBNumber);

      logger->Close();

   }
   catch (Exception ^ e)
   {
      Console::WriteLine("Unhandled Exception: {0}", e);
      termMode = Phx::Term::Mode::Fatal;
   }

   // clean up Phx and terminate

   Phx::Term::All(termMode);
   return ((termMode == Phx::Term::Mode::Normal) ? 0 : 1);
}
