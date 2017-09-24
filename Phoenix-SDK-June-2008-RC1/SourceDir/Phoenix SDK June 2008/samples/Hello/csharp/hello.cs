//-----------------------------------------------------------------------------
//
// Phoenix
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    A program that creates Phoenix IR for a simple C++ program
//    and produces a linkable object file.
//
// Usage:
//
//    Hello.exe
//
//    Produces file main.obj, suitable for linking.
//    Simply run 'link main.obj [/debug]'.
//
//    Resulting executable can be debugged at source level -- load it
//    in visual studio and see.
//
// Remarks:
//
//    This program simulates the compilation of a simple C++ program.
//    The program itself is modelled after the following input text:
//
//    ---- base.h ----
//
//    #include <cstdio>
//
//    class Base {
//    public:
//        Base(const char * msg) : message(msg) {}
//        virtual void Hello() { printf("%s from Base\n", message); }
//        const char * GetMessage() { return message; }
//    private:
//        const char * message;
//    };
//
//    ---- main.cpp ----
//
//    #include "base.h"
//
//    class Derived: public Base {
//    public:
//        Derived(const char * msg) : Base(msg) {}
//        virtual void Hello() { printf("%s from Derived\n", this->GetMessage()); }
//    };
//
//    int main()
//    {
//        Base * base1 = new Base("Hello");
//        Base * base2 = new Derived("Hello");
//        base1->Hello();
//        base2->Hello();
//    }
//
//    ----------------
//
//    As with any implementation of a high-level language there are
//    many implementation decisions that must be made. In this example
//    we will attempt to adhere to Microsoft conventions, so that if
//    desired, you can omit main() from the object file created by
//    this example, compile it separately (along with base.h) using a
//    standard compiler, and link the two object files together.
//
//    ----------------
//
//    Follow-on activities (from easier to harder)
//
//    (1) Modify the program so that it prints
//
//          "Hello from Base"
//          "Goodbye from Derived"
//
//    (2) You can turn on various phoenix controls by passing command
//    line arguments to this program. Experiment with dumping and look
//    at how the phases in the phase list transform the IR.
//
//    (3) Add an integer data member to Derived, and print it along
//    with the hello message.
//
//    (4) Change the program to read a single command line argument,
//    interpret it as an integer value N (use atoi), and add a loop to
//    main so that the two objects say hello N times.
//
//    (5) Change the example so it works with wide characters
//    (unicode).  Call wsprintf instead of printf, etc.
//
//    (6) Change the example so that it generates amd64 code instead
//    of x86 code. Will the program run properly? Are there any
//    x86 assumptions baked into the IR?
//
//    (7) Don't produce code for main or derived in this object file;
//    instead, use the regular C++ compiler to compile them in a
//    second object file.  Link the two objects together and make them
//    produce the same output. You may find that comdat comes in
//    handy!
//
//    (8) Teach the debugger to understand that main has two local
//    variables (requires understanding of microsoft codeview debug
//    record formats).
//
//    (9) Examine the assembly language listing (-listing on command
//    line). Look for places where the compiler could have generated
//    better code. Implement an optimization phase as a plug-in for
//    this sample and see if you can make things better.
//
//-----------------------------------------------------------------------------

using System;

// Not using Phx namespace to show explicit class hierarchy

public class Hello
{
   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Class Variables.
   //
   // Remarks:
   //
   //    A normal compiler would have more flexible means for holding
   //    on to all this information, but in our case it's simplest (if
   //    somewhat inelegant) if we just keep references to all the
   //    structures we'll need to access as class static variables.
   //
   //--------------------------------------------------------------------------

   static Phx.ModuleUnit               module;
   static Phx.Targets.Runtimes.Runtime runtime;
   static Phx.Targets.Architectures.Architecture       architecture;
   static Phx.Lifetime                 lifetime;
   static Phx.Types.Table              typeTable;
   static Phx.Symbols.Table               symbolTable;
   static Phx.Phases.PhaseConfiguration       phaseConfiguration;

   static Phx.Types.Type               charType;
   static Phx.Types.PointerType            ptrToCharType;
   static Phx.Types.PointerType            ptrToUnkType;
   static Phx.Types.AggregateType           baseClassType;
   static Phx.Types.PointerType            ptrToBaseType;
   static Phx.Types.AggregateType           derivedClassType;
   static Phx.Types.PointerType            ptrToDerivedType;

   static Phx.Symbols.FunctionSymbol             printfSym;
   static Phx.Symbols.FunctionSymbol             operatorNewSym;
   static Phx.Symbols.FunctionSymbol             baseCtorSym;
   static Phx.Symbols.FunctionSymbol             baseHelloSym;
   static Phx.Symbols.FunctionSymbol             baseGetMessageSym;
   static Phx.Symbols.FunctionSymbol             derivedCtorSym;
   static Phx.Symbols.FunctionSymbol             derivedHelloSym;
   static Phx.Symbols.FunctionSymbol             mainSymbol;

   static Phx.Symbols.FieldSymbol            vtableFieldSym;
   static Phx.Symbols.FieldSymbol            messageFieldSym;

   static Phx.Symbols.GlobalVariableSymbol        baseVtableSym;
   static Phx.Symbols.GlobalVariableSymbol        derivedVtableSym;
   static Phx.Symbols.GlobalVariableSymbol        baseFormatStringSym;
   static Phx.Symbols.GlobalVariableSymbol        derivedFormatStringSym;
   static Phx.Symbols.GlobalVariableSymbol        firstHelloStringSym;
   static Phx.Symbols.GlobalVariableSymbol        secondHelloStringSym;

   static uint                         externalIdCounter = 100;
   static uint                         funcCounter = 1;

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Entry point for application
   //
   // Returns:
   //
   //    Nothing
   //
   //--------------------------------------------------------------------------

   public static int
   Main
   (
      String[] argv      // command line argument strings
   )
   {
      Phx.Term.Mode termMode = Phx.Term.Mode.Normal;

      try {

         // Do some initializations, and select the code generation
         // target.  Note that after this point we are largely
         // architecture neutral (that is, we don't know what processor
         // we're targeting...).

         runtime = Initialize(argv);
         architecture = runtime.Architecture;

         // Build up a list of phases to apply to IR.

         phaseConfiguration = BuildPhaseList();

         // In this example we will make use of the global type table to
         // hold all types. This table is automatically created by
         // Phx.Initialize.BeginInitialization.

         typeTable = Phx.GlobalData.TypeTable;

         // We must create a ModuleUnit to represent the unit of
         // compilation. Before we can do this we need to create a module
         // lifetime and a name for the module.

         lifetime = Phx.Lifetime.New(Phx.LifetimeKind.Module, null);

         Phx.Name moduleName = Phx.Name.New(lifetime, "main.obj");

         module = Phx.ModuleUnit.New(lifetime, moduleName, null,
            typeTable, runtime.Architecture, runtime);

         // When Phoenix is processing there is a notion of a current
         // unit. The current unit is used to fill in important context in
         // certain apis. Creating a unit establishes it as the current unit.

         // Associated a symbol table with the module. This table will
         // hold both external symbols and module-scope symbols.

         symbolTable = Phx.Symbols.Table.New(module, 64, true);

         // Now build up symbols to represent the "compilation environment" --
         // namely, things referred to here but defined else where.

         BuildExternalSymbols();

         // That establishes our compilation environment. Now define the
         // symbols and types for Base, Derived, and main respectively.

         BuildBaseClass();
         BuildDerivedClass();
         BuildMain();

         // And a few more symbols and types for initialized data.

         BuildInitializedData();

         // Finally, we're ready to create some code! In this sample
         // we'll actually create all the IR up front and then run
         // each function one by one through the phase list to produce
         // the encoded x86 instructions. A real compiler might prefer
         // to completely finish off one function before building the
         // IR for the next to keep memory usage down, but Phoenix
         // allows us do things either way.

         Phx.Unit baseCtorFunc = BuildBaseCtor();
         Phx.Unit baseGetMessageFunc = BuildBaseGetMessage();
         Phx.Unit baseHelloFunc = BuildBaseHello();
         Phx.Unit derivedCtorFunc = BuildDerivedCtor();
         Phx.Unit derivedHelloFunc = BuildDerivedHello();
         Phx.Unit mainFunc = BuildMainFunc();

         // At this point the code is all represented, but it's in
         // Phoenix high-level IR (Hir). To turn that IR into
         // x86 instructions we apply the phase list to each function.

         ExecutePhases(baseCtorFunc);
         ExecutePhases(baseGetMessageFunc);
         ExecutePhases(baseHelloFunc);
         ExecutePhases(derivedCtorFunc);
         ExecutePhases(derivedHelloFunc);
         ExecutePhases(mainFunc);

         // Prepare to write the object file.

         Phx.Coff.ObjectWriter objectWriter =
            Phx.Coff.ObjectWriter.New(lifetime, module,
               "main.obj", "main.cpp", architecture, 0,
               0, false);

         // Add a comment for the linker, telling it where to look for
         // the definitions for the external functions.

         objectWriter.AddComment(Phx.Coff.CommentType.Linker,
            "/DEFAULTLIB:LIBCMT");

         // Write the object.

         Phx.Output.Write("Writing main.obj..");
         objectWriter.Write();
         Phx.Output.WriteLine("..done");
      }
      catch (Exception exception)
      {
         Phx.Output.Write("Unhandled Exception: {0}", exception);
         termMode = Phx.Term.Mode.Fatal;
      }

#if (PHX_DEBUG_CHECKS)
      // Make sure there were no asserts during the run.

      if (Phx.Asserts.AssertCount != 0)
      {
         termMode = Phx.Term.Mode.Fatal;
      }
#endif

      // Cleanup.

      Phx.Term.All(termMode);

      // Return exit status.

      return (termMode == Phx.Term.Mode.Normal ? 0 : 1);
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Initializations.
   //
   // Remarks:
   //
   //    Performs framework initializations. Process command line and
   //    environment arguments. Turn on a few controls by default.
   //
   // Returns:
   //
   //    Runtime for which we'll generate code.
   //
   //--------------------------------------------------------------------------

   public static Phx.Targets.Runtimes.Runtime
   Initialize
   (
      String[] argv      // command line argument strings
   )
   {
      // We will generate X86 code.

      Phx.Targets.Architectures.Architecture architecture =
          Phx.Targets.Architectures.X86.Architecture.New();
      Phx.Targets.Runtimes.Runtime runtime =
          Phx.Targets.Runtimes.Vccrt.Win32.X86.Runtime.New(architecture);
      Phx.GlobalData.RegisterTargetArchitecture(architecture);
      Phx.GlobalData.RegisterTargetRuntime(runtime);

      // Initialize the infrastructure.

      Phx.Initialize.BeginInitialization();

      // Initialize controls set by command line, register plugins,
      // etc.

      Phx.Initialize.EndInitialization("PHX|*|_PHX_|", argv);

      // Enable some controls. We want to see the names of types, types
      // in our IR dumps, linenumbers in our IR dumps, and linenumbers
      // in our object file.

      Phx.Controls.Parser.ParseArgumentString(null,
         "-dumptypesym -dump:types -dump:linenumbers -lineleveldebug");

      return runtime;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Build up a phase list for compilation.
   //
   // Returns:
   //
   //    Phase list to use in processing each function.
   //
   //--------------------------------------------------------------------------

   public static Phx.Phases.PhaseConfiguration
   BuildPhaseList()
   {
      // When we get around to generating IR, we'll need to gradually
      // transform it into x86 machine code. Phx provides a number of
      // phases to help in this task. The phases are strung together
      // in a list, which we'll now build.

      Phx.Phases.PhaseConfiguration config;
      Phx.Phases.PhaseList   phaseList;

      config = Phx.Phases.PhaseConfiguration.New(Phx.GlobalData.GlobalLifetime,
         "Hello Phases");
      phaseList = config.PhaseList;

      phaseList.AppendPhase(Phx.Types.TypeCheckPhase.New(config));
      phaseList.AppendPhase(Phx.MirLowerPhase.New(config));
      phaseList.AppendPhase(Phx.Targets.Runtimes.CanonicalizePhase.New(config));
      phaseList.AppendPhase(Phx.Targets.Runtimes.LowerPhase.New(config));
      phaseList.AppendPhase(Phx.RegisterAllocator.LinearScan.RegisterAllocatorPhase.New(config));
      phaseList.AppendPhase(Phx.StackAllocatePhase.New(config));
      phaseList.AppendPhase(Phx.Targets.Runtimes.FrameGenerationPhase.New(config));
      phaseList.AppendPhase(Phx.Targets.Runtimes.SwitchLowerPhase.New(config));
      phaseList.AppendPhase(Phx.Graphs.BlockLayoutPhase.New(config));
      phaseList.AppendPhase(Phx.FlowOptimizer.Phase.New(config));

      // These two are defined below.

      phaseList.AppendPhase(Encode.New(config));
      phaseList.AppendPhase(Lister.New(config));

      // Add in any target-specific phases (eg x87 register allocation).

      runtime.AddPhases(config);

      // Add in any plugin phases.

      Phx.GlobalData.BuildPlugInPhases(config);

      return config;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Phase for encoding ...
   //
   // Returns:
   //
   //    New encoding phase
   //
   //--------------------------------------------------------------------------

   class Encode : Phx.Phases.Phase
   {
      public static Encode
      New
      (
         Phx.Phases.PhaseConfiguration config
      )
      {
         Encode encode = new Encode();

         encode.Initialize(config, "Encoding");

#if (PHX_DEBUG_SUPPORT)

         encode.PhaseControl = Phx.Targets.Runtimes.Encode.DebugControl;

#endif
         return encode;
      }

      override protected unsafe void
      Execute
      (
         Phx.Unit unit
      )
      {
         if (!unit.IsFunctionUnit)
         {
            return;
         }

         Phx.FunctionUnit function = unit.AsFunctionUnit;
         Phx.Targets.Runtimes.Encode encode = function.Encode;
         Phx.Unit module = function.ParentUnit;
         Phx.Symbols.FunctionSymbol functionSymbol = function.FunctionSymbol;

         function.EncodedIRLifetime = module.Lifetime;
         uint binarySize = encode.FinalizeIR();

         Phx.IR.DataInstruction encodedInstruction =
            Phx.IR.DataInstruction.New(module, binarySize);

         // The function is this big.

         functionSymbol.ByteSize = binarySize;

         // The call to GetDatPtr return a pointer and so is unsafe.

         encode.Function(encodedInstruction.GetDataPointer(0));

         // Copy fixups and debug.

         encodedInstruction.FixupList = encode.FixupList;
         encodedInstruction.DebugOffsets = encode.DebugOffsets;

         // Set the function sym's location to the encoded IR.

         functionSymbol.Location = Phx.Symbols.DataLocation.New(encodedInstruction);

         // Add the encoded IR to the appropriate section.

         Phx.Symbols.SectionSymbol textSectionSymbol =
            module.SymbolTable.ExternIdMap.Lookup(
               (uint) Phx.Symbols.PredefinedCxxILId.Text) as Phx.Symbols.SectionSymbol;

         textSectionSymbol.Section.AppendInstruction(encodedInstruction);

         // And note that the function ended up here.

         functionSymbol.AllocationBaseSectionSymbol = textSectionSymbol;
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Phase for producing an assembly listing.
   //
   // Returns:
   //
   //    New listing phase
   //
   //--------------------------------------------------------------------------

   class Lister : Phx.Phases.Phase
   {
      public static Lister
      New
      (
         Phx.Phases.PhaseConfiguration config
      )
      {
         Lister lister = new Lister();

         lister.Initialize(config, "Encoding");

#if (PHX_DEBUG_SUPPORT)

         lister.PhaseControl = Phx.Targets.Runtimes.Lister.DebugControl;

#endif
         return lister;
      }

      override protected void
      Execute
      (
         Phx.Unit unit
      )
      {
         if (!unit.IsFunctionUnit)
         {
            return;
         }
         
         Phx.FunctionUnit function = unit.AsFunctionUnit;

         function.Lister.Function();
      }
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Creates symbols and types for externals -- things that are referred
   //    to within our program but defined elsewhere.
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   static public void
   BuildExternalSymbols()
   {
      // In this case the only apparent symbol is printf, with
      // signature
      //
      //    int __cdecl printf(const char *, ...);
      //
      // Here the __cdecl indicates the calling convention.
      //
      // However, there is another external function that lurks behind
      // the scenes: the 'new Base' and 'new Derived' statements turn
      // into calls to an allocation function with the following
      // signature:
      //
      //    void * __cdecl operator new(unsigned int);
      //
      // Let's first tackle printf. To begin with, we must create a
      // type object that represents the function signature.  In
      // Phoenix, function types are created with via the
      // FunctionTypeBuilder:

      Phx.Types.FunctionTypeBuilder functionTypeBuilder =
         Phx.Types.FunctionTypeBuilder.New(typeTable);

      functionTypeBuilder.Begin();

      // Now indicate that this function requires the cdecl calling convention.

      functionTypeBuilder.CallingConventionKind = Phx.Types.CallingConventionKind.CDecl;

      // Set the return value type. Here it is int. We'll size
      // 'int' to be the register size on the target machine, and
      // use a type to match.

      functionTypeBuilder.AppendReturnParameter(module.RegisterIntType);

      // Set the first argument type. We disregard 'const' here since
      // it is largely a frontend notion. We'll have our compiler use
      // 8 bit characters. Also, we'll assume we have standard-sized
      // pointers.

      charType = typeTable.Character8Type;

      ptrToCharType = Phx.Types.PointerType.New(typeTable,
         Phx.Types.PointerTypeKind.UnmanagedPointer, typeTable.NativePointerBitSize,
         charType, null);

      // As an alternative, we could have used
      //
      //    typeTable.GetUnmanagedPointerType(charType);
      //
      // which is what we'll do in the future when we create pointer types.

      functionTypeBuilder.AppendParameter(ptrToCharType);

      // Now for the ... parameter. Strictly speaking this argument
      // doesn't have a type. In Phoenix we use FuncArgs to represent
      // special arguments like ... . Now, ... doesn't have a type,
      // but the method requires one, so we use the UnknownType.

      functionTypeBuilder.AppendParameter(typeTable.UnknownType,
          Phx.Types.ParameterKind.Ellipsis);

      // Now the functionType is ready to be extracted.

      Phx.Types.FunctionType printfType = functionTypeBuilder.GetFunctionType();

      // Now we're ready to create a symbol to represent printf itself. First
      // we must give it a linker name. To follow convention we prepend an
      // underscore. printf is external, so we indicate its visibility as a
      // reference.

      Phx.Name printfName = Phx.Name.New(lifetime, "_printf");

      printfSym = Phx.Symbols.FunctionSymbol.New(symbolTable, 0, printfName, printfType,
         Phx.Symbols.Visibility.GlobalReference);

      // Now for operator new. As with many C++ names, the actual
      // function name is 'mangled' into something with no spaces;
      // here the name is ??2@YAPAXI@Z

      Phx.Name operatorNewName = Phx.Name.New(lifetime, "??2@YAPAXI@Z");

      // In cases where there are no funny parameters like ... we can
      // use a streamlined method for building function types:

      ptrToUnkType =
         typeTable.GetUnmanagedPointerType(typeTable.UnknownType);

      Phx.Types.FunctionType operatorNewType = typeTable.GetFunctionType(
         Phx.Types.CallingConventionKind.CDecl,	0,
         ptrToUnkType, module.RegisterUIntType, null, null, null);

      operatorNewSym = Phx.Symbols.FunctionSymbol.New(symbolTable, 0,
         operatorNewName, operatorNewType, Phx.Symbols.Visibility.GlobalReference);
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Creates symbols and types for class Base.
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   static public void
   BuildBaseClass()
   {
      // class Base has three user-defined member functions and a data
      // member. Since Base has a virtual function, we'll provide a
      // virtual function table (vtable) in each Base object. If
      // pointers are 4 bytes the Base object will look something like
      // the following in memory:
      //
      //      0: vtable
      //      4: message
      //
      // where vtable will be:
      //
      //      0: Base::Hello();
      //
      // Let's first build a class type for Base, and fill in the data
      // members. We'll actually fill things in with a bit more detail
      // than is absolutely necessary...

      Phx.Symbols.TypeSymbol baseClassSym = Phx.Symbols.TypeSymbol.New(symbolTable, 0,
         Phx.Name.New(lifetime, "Base"));

      baseClassType = Phx.Types.AggregateType.New(typeTable,
         2 * typeTable.NativePointerBitSize, baseClassSym);

      // This type has a vtable.

      baseClassType.IsSelfDescribing = true;

      // We won't describe the type of the vtable in any detail; we
      // can use the unknown type for this. So the field contains
      // a pointer to unk.
      //
      // Now to create a field within the class for the vtable. This
      // field is introduced by the compiler, and is at offset 0
      // within the object.

      vtableFieldSym = Phx.Symbols.FieldSymbol.New(symbolTable,
         0, Phx.Name.New(lifetime, "vtable"), ptrToUnkType);

      vtableFieldSym.IsToolGenerated = true;
      vtableFieldSym.BitOffset = 0;

      baseClassType.AppendFieldSymbol(vtableFieldSym);

      // Now a field for message. This field is of type pointer to
      // character. We can reuse this type since we built it up
      // earlier when describing printf. The field is one pointer
      // length inside the object.

      messageFieldSym = Phx.Symbols.FieldSymbol.New(symbolTable,
         0, Phx.Name.New(lifetime, "message"), ptrToCharType);

      messageFieldSym.BitOffset = (int) typeTable.NativePointerBitSize;

      baseClassType.AppendFieldSymbol(messageFieldSym);

      // Now for the member functions. First, the
      // constructor. Although the constructor appears to have only
      // one argument, in our conventions the 'this' pointer is passed
      // as an implicit first argument and is returned.  So the actual
      // signature is something like
      //
      //   Base * __thiscall Base(void * this, const char * message);
      //
      // In our code we'll use "unk" (unknown) for void.

      ptrToBaseType =
         typeTable.GetUnmanagedPointerType(baseClassType);

      Phx.Types.FunctionType baseCtorType = typeTable.GetFunctionType(
         Phx.Types.CallingConventionKind.ThisCall, 0, ptrToBaseType,
         ptrToUnkType, ptrToCharType, null, null);

      // Now for the function symbol. Because of linker limitations and
      // lack of binary compatibility between object models, most
      // compilers will "mangle" a name like Base::Base into something
      // else for use by the linker. We're no different, and the
      // resulting name is ??0Base@@QAE@PBD@Z.
      //
      // Indicate that we will be providing a definition in this
      // compilation unit by setting the visibility to GlobalDefinition.

      Phx.Name baseCtorName = Phx.Name.New(lifetime, "??0Base@@QAE@PBD@Z");

      baseCtorSym = Phx.Symbols.FunctionSymbol.New(symbolTable, 0,
         baseCtorName, baseCtorType, Phx.Symbols.Visibility.GlobalDefinition);

      // Add this as a member function.

      baseClassType.AppendMethodSymbol(baseCtorSym);

      // Onto the GetMessage() method. Once again the this pointer is
      // a hidden parameter.

      Phx.Types.FunctionType getMessageType = typeTable.GetFunctionType(
         Phx.Types.CallingConventionKind.ThisCall, 0, ptrToCharType,
         ptrToBaseType, null, null, null);

      Phx.Name getMessageName = Phx.Name.New(lifetime,
         "?GetMessage@Base@@QAEPBDXZ");

      baseGetMessageSym = Phx.Symbols.FunctionSymbol.New(symbolTable, 0,
         getMessageName, getMessageType, Phx.Symbols.Visibility.GlobalDefinition);

      // Add this as a member function.

      baseClassType.AppendMethodSymbol(baseGetMessageSym);

      // Finally, the Hello() method. Once again the this pointer is
      // a hidden parameter.

      Phx.Types.FunctionType helloType = typeTable.GetFunctionType(
         Phx.Types.CallingConventionKind.ThisCall, 0, typeTable.VoidType,
         ptrToBaseType, null, null, null);

      Phx.Name helloName = Phx.Name.New(lifetime,
         "?Hello@Base@@UAEXXZ");

      baseHelloSym = Phx.Symbols.FunctionSymbol.New(symbolTable, 0,
         helloName, helloType, Phx.Symbols.Visibility.GlobalDefinition);

      // Add this as a member function.

      baseClassType.AppendMethodSymbol(baseHelloSym);

      // Done with class Base.

      baseClassType.HasCompleteInheritanceInfo = true;
      baseClassType.HasCompleteMemberInfo = true;
      baseClassType.HasCompleteOverrideInfo = true;
      baseClassType.LayoutFinished = true;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Creates symbols and types for class Derived.
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   static public void
   BuildDerivedClass()
   {
      // The class Derived has two user-defined member functions and
      // no data members of its own. It does inherit from Base. So
      // a Derived will look like:
      //
      //      0: vtable
      //      4: message
      //
      // where vtable will be:
      //
      //      0: Derived::Hello();

      Phx.Symbols.TypeSymbol derivedClassSym = Phx.Symbols.TypeSymbol.New(symbolTable, 0,
         Phx.Name.New(lifetime, "Derived"));

      derivedClassType = Phx.Types.AggregateType.New(typeTable,
         2 * typeTable.NativePointerBitSize, derivedClassSym);

      // Establish derived as inheriting from base.

      Phx.Types.BaseTypeLink inheritanceLink = Phx.Types.BaseTypeLink.New(
         baseClassType, false);

      derivedClassType.AppendBaseTypeLink(inheritanceLink);
      derivedClassType.IsSelfDescribing = true;

      // Now for the member functions. As with the Base constructor,
      // we need to make:
      //
      //   Derived * __thiscall Derived(void * this, const char * message);

      ptrToDerivedType =
         typeTable.GetUnmanagedPointerType(derivedClassType);

      Phx.Types.FunctionType derivedCtorType = typeTable.GetFunctionType(
         Phx.Types.CallingConventionKind.ThisCall, 0, ptrToDerivedType,
         ptrToUnkType, ptrToCharType, null, null);

      // Now for the function symbol. This time the name is
      // ??0Derived@@QAE@PBD@Z.

      Phx.Name derivedCtorName = Phx.Name.New(lifetime, "??0Derived@@QAE@PBD@Z");

      derivedCtorSym = Phx.Symbols.FunctionSymbol.New(symbolTable, 0,
         derivedCtorName, derivedCtorType, Phx.Symbols.Visibility.GlobalDefinition);

      // Add this as a member function.

      derivedClassType.AppendMethodSymbol(derivedCtorSym);

      // And now for the Hello() method. Once again the this pointer
      // is a hidden parameter.

      Phx.Types.FunctionType derivedHelloType = typeTable.GetFunctionType(
         Phx.Types.CallingConventionKind.ThisCall, 0, typeTable.VoidType,
         ptrToDerivedType, null, null, null);

      Phx.Name derivedHelloName = Phx.Name.New(lifetime,
         "?Hello@Derived@@UAEXXZ");

      derivedHelloSym = Phx.Symbols.FunctionSymbol.New(symbolTable, 0,
         derivedHelloName, derivedHelloType, Phx.Symbols.Visibility.GlobalDefinition);

      // Add this as a member function.

      derivedClassType.AppendMethodSymbol(derivedHelloSym);

      // Done with class Derived.

      derivedClassType.HasCompleteInheritanceInfo = true;
      derivedClassType.HasCompleteMemberInfo = true;
      derivedClassType.HasCompleteOverrideInfo = true;
      derivedClassType.LayoutFinished = true;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Creates the symbols and types for main.
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   static public void
   BuildMain()
   {
      Phx.Types.FunctionType mainType = typeTable.GetFunctionType(
         Phx.Types.CallingConventionKind.CDecl, 0, module.RegisterIntType,
         typeTable.VoidType, null, null, null);

      Phx.Name mainName = Phx.Name.New(lifetime, "_main");

      mainSymbol = Phx.Symbols.FunctionSymbol.New(symbolTable, 0,
         mainName, mainType, Phx.Symbols.Visibility.GlobalDefinition);
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Creates the IR for the Base::Base method.
   //
   // Returns:
   //
   //    FunctionUnit for Base::Base
   //
   //--------------------------------------------------------------------------

   static public Phx.FunctionUnit
   BuildBaseCtor()
   {
      // First we'll need a lifetime just for this function.

      Phx.Lifetime functionLifetime =
         Phx.Lifetime.New(Phx.LifetimeKind.Function, null);

       // Next we'll create the functionUnit. There are a lot of
       // parameters here.

      Phx.FunctionUnit function = Phx.FunctionUnit.New(functionLifetime,
         baseCtorSym, Phx.CodeGenerationMode.Native,
         typeTable, module.Architecture, module.Runtime, module, funcCounter++);

      // Associate some debugInfo, so we can track source positions.

      Phx.Debug.Info.New(lifetime, function);

      // Let's think a bit about what code needs to appear in this
      // function.  We will be passed a pointer to an uninitialized
      // memory area which is storage for the object (the this
      // pointer) and a pointer to a string. We need to store the
      // address of the vtable for the object into the vtable slot,
      // and then store the msg parameter into the message data
      // member. Finally, we need to return the this pointer.
      //
      // When writing IR textually we use the following conventions.
      // An IR instruction has destination operands, an opcode, and
      // source operands. We write these left-to-right:
      //
      //        <dst opnds> = OPCODE <src opnds>
      //
      // If there are no destnation operands, the = sign is omitted as
      // well.
      //
      // So our IR for Base::Base(...) will be something like:
      //
      //        [this + vtable]  = ASSIGN &$Base$Vtable
      //        [this + message] = ASSIGN msg
      //                           RETURN this
      //
      // Here we use [] to indicate an access to memory, & as
      // address-of. ASSIGN is a high-level opcode for assigning a
      // destination operand the value of a source operand.
      //
      // Note that we're also using symbolic references for locations.
      // Some of these symbols have already been created -- namely
      // vtable, message, and $Base$Vtable. But 'this' and 'msg' are
      // new symbols that are specific to this function. To accomodate
      // this distinction we need to create a new symbol table for the
      // function, to hold its "local" symbols. So let's do that
      // first.

      Phx.Symbols.Table funcSymTable = Phx.Symbols.Table.New(function, 64, true);

      // this is a parameter symbol with type Base *

      Phx.Symbols.LocalVariableSymbol thisSym = Phx.Symbols.LocalVariableSymbol.New(funcSymTable, 0,
         Phx.Name.New(functionLifetime, "this"), ptrToBaseType,
         Phx.Symbols.StorageClass.Parameter);

      thisSym.Alignment = Phx.Alignment.NaturalAlignment(ptrToBaseType);

      // msg is a parameter symbol with type char *

      Phx.Symbols.LocalVariableSymbol msgSym = Phx.Symbols.LocalVariableSymbol.New(funcSymTable, 0,
         Phx.Name.New(functionLifetime, "msg"), ptrToCharType,
         Phx.Symbols.StorageClass.Parameter);

      msgSym.Alignment = Phx.Alignment.NaturalAlignment(ptrToCharType);

      // One more important step. Each instruction carries debug
      // information along with it. We'd like to have this reflect the
      // source locations.

      function.CurrentDebugTag = function.DebugInfo.CreateTag(
         Phx.Name.New(lifetime, "base.h"), 5);

      // Grumble. Update tags on START/END.

      function.FirstInstruction.DebugTag = function.CurrentDebugTag;
      function.LastInstruction.DebugTag = function.CurrentDebugTag;

      // We can now create IR that refers to these symbols. But before
      // we do this we need to establish an entry point in the
      // function. The entry point names the point of control transfer
      // and also provides for parameter passing. In textual IR this
      // is simply an ENTERFUNC opcode where the parameter symbols are
      // destinations. The name is supplied by passing in the functionSymbol.
      // (<functionSymbol> is technically not an operand here, but just some
      // extra information).
      //
      //     this, msg = ENTERFUNC <functionSymbol>
      //
      // The order of parameters is important here. When writing IR in
      // text, the "first" destination operand (the head of the
      // destination operand list) is listed leftmost. So we want this
      // as the first destionation operand, and msg as the second.

      Phx.IR.LabelInstruction enterInstruction = Phx.IR.LabelInstruction.New(function,
         Phx.Common.Opcode.EnterFunction, baseCtorSym);

      // There also needs to be a technical edge from start to enter
      // to keep the flow graph wired up properly. In this case having
      // both Start and EnterFunction is a bit redundant, but in general a
      // function body could have many different entry points.

      Phx.IR.LabelOperand labelOperand = Phx.IR.LabelOperand.New(function, enterInstruction);
      function.FirstInstruction.AppendLabelSource(Phx.IR.LabelOperandKind.Technical, labelOperand);

      // Now create the destination operands for the ENTERFUNC instruction;
      // these represent the parameters to the function.
      // We encapsulate symbol references with variable operands. Note
      // that we must again specify the type...

      Phx.IR.VariableOperand thisOperand = Phx.IR.VariableOperand.New(function,
         ptrToBaseType, thisSym);

      Phx.IR.VariableOperand msgOpnd = Phx.IR.VariableOperand.New(function,
         ptrToCharType, msgSym);

      // Build the destination operand list.

      enterInstruction.AppendDestination(thisOperand);
      enterInstruction.AppendDestination(msgOpnd);

      // Now append this instruction to the function's instruction
      // list.

      function.FirstInstruction.InsertAfter(enterInstruction);

      // Now for our first ASSIGN: [this + vtable] = ASSIGN &$Base$Vtable.
      //
      // For destination, we have a memory operand with base address
      // and field.

      Phx.IR.MemoryOperand vtableMemOpnd = Phx.IR.MemoryOperand.New(function,
         vtableFieldSym.Field, null, thisOperand, 0,
         Phx.Alignment.NaturalAlignment(ptrToBaseType),
         function.AliasInfo.IndirectAliasedMemoryTag,
         function.SafetyInfo.SafeTag);
      vtableMemOpnd.Type = typeTable.GetUnmanagedPointerType(baseVtableSym.Type);

      // For a source, we want to refer to $Base$Vtable's
      // address. Recall that $Base$Vtable is an initialized global
      // variable. To refer to it in IR we must first introduce it
      // into the local symbol table by building a non-local variable sym:

      Phx.Symbols.NonLocalVariableSymbol localBaseVtableSym = Phx.Symbols.NonLocalVariableSymbol.New(
         funcSymTable, baseVtableSym);

      Phx.IR.VariableOperand vtablePtrOpnd = Phx.IR.VariableOperand.New(function,
         baseVtableSym.Type, localBaseVtableSym);

      // Take the effective address rather than $Base$Vtable itself.

      vtablePtrOpnd.ChangeToAddress();

      Phx.IR.Instruction vtableAssignInstr = Phx.IR.ValueInstruction.NewUnary(function,
         Phx.Common.Opcode.Assign, vtableMemOpnd, vtablePtrOpnd);

      enterInstruction.InsertAfter(vtableAssignInstr);

      // Now for the second ASSIGN: [this + message] = ASSIGN msg
      //
      // For destination, we have a memory operand with base address
      // and field.

      Phx.IR.MemoryOperand messageMemOpnd = Phx.IR.MemoryOperand.New(function,
         messageFieldSym.Field, null, thisOperand, 0,
         Phx.Alignment.NaturalAlignment(ptrToCharType),
         function.AliasInfo.IndirectAliasedMemoryTag,
         function.SafetyInfo.SafeTag);

      // For a source, we want to refer to msg. We just use msgOpnd over again.

      Phx.IR.Instruction messageAssignInstr = Phx.IR.ValueInstruction.NewUnary(function,
         Phx.Common.Opcode.Assign, messageMemOpnd, msgOpnd);

      vtableAssignInstr.InsertAfter(messageAssignInstr);

      // Now for the return. Before we can build the return we need to
      // create a final exit point for the function. So we do that first.

      Phx.IR.LabelInstruction exitInstruction = Phx.IR.LabelInstruction.New(function,
         Phx.Common.Opcode.ExitFunction);

      function.LastInstruction.InsertBefore(exitInstruction);

      Phx.IR.Instruction returnInstruction = Phx.IR.BranchInstruction.NewReturn(function,
         Phx.Common.Opcode.Return, exitInstruction, thisOperand);

      messageAssignInstr.InsertAfter(returnInstruction);

      // Perform generic, one-time tasks that are taken after IR is in place.

      function.FinishCreation();

      // Now that we're done with IR for this unit, pop it off the context
      // stack (creating the unit pushed it on).

      function.Context.PopUnit();

      return function;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Creates the IR for the Base::GetMessage method.
   //
   // Returns:
   //
   //    FunctionUnit for Base::GetMessage
   //
   //--------------------------------------------------------------------------

   static public Phx.FunctionUnit
   BuildBaseGetMessage()
   {
      // First we'll need a lifetime just for this function.

      Phx.Lifetime functionLifetime =
         Phx.Lifetime.New(Phx.LifetimeKind.Function, null);

       // Next we'll create the functionUnit. There are a lot of
       // parameters here.

      Phx.FunctionUnit function = Phx.FunctionUnit.New(functionLifetime,
         baseGetMessageSym, Phx.CodeGenerationMode.Native,
         typeTable, module.Architecture, module.Runtime, module, funcCounter++);

      // Associate some debugInfo, so we can track source positions.

      Phx.Debug.Info.New(lifetime, function);

      // For GetMessage there are no explicit parameters; just
      // the implicit this pointer. So the IR is simply:
      //
      //          this = ENTERFUNC
      //          temporary  = ASSIGN [this + message]
      //                 RETURN temporary
      //                 EXITFUNC

      Phx.Symbols.Table funcSymTable = Phx.Symbols.Table.New(function, 64, true);

      // this is a parameter symbol with type Base *

      Phx.Symbols.LocalVariableSymbol thisSym = Phx.Symbols.LocalVariableSymbol.New(funcSymTable, 0,
         Phx.Name.New(functionLifetime, "this"), ptrToBaseType,
         Phx.Symbols.StorageClass.Parameter);

      thisSym.Alignment = Phx.Alignment.NaturalAlignment(ptrToBaseType);

      // One more important step. Each instruction carries debug
      // information along with it. We'd like to have this reflect the
      // source locations.

      function.CurrentDebugTag = function.DebugInfo.CreateTag(
         Phx.Name.New(lifetime, "base.h"), 7);

      // Grumble. Update tags on START/END.

      function.FirstInstruction.DebugTag = function.CurrentDebugTag;
      function.LastInstruction.DebugTag = function.CurrentDebugTag;

      // Build the ENTERFUNC

      Phx.IR.LabelInstruction enterInstruction = Phx.IR.LabelInstruction.New(function,
         Phx.Common.Opcode.EnterFunction, baseGetMessageSym);

      Phx.IR.VariableOperand thisOperand = Phx.IR.VariableOperand.New(function,
         ptrToBaseType, thisSym);

      enterInstruction.AppendDestination(thisOperand);

      function.FirstInstruction.InsertAfter(enterInstruction);

      // There needs to be an edge from start to enter

      Phx.IR.LabelOperand labelOperand = Phx.IR.LabelOperand.New(function, enterInstruction);
      function.FirstInstruction.AppendLabelSource(Phx.IR.LabelOperandKind.Technical, labelOperand);

      // temporary = ASSIGN [this + message]

      Phx.IR.MemoryOperand messageMemOpnd = Phx.IR.MemoryOperand.New(function,
         messageFieldSym.Field, null, thisOperand, 0,
         Phx.Alignment.NaturalAlignment(ptrToCharType),
         function.AliasInfo.IndirectAliasedMemoryTag,
         function.SafetyInfo.SafeTag);

      // NewUnaryExpression creates a destination operand of the specified type.

      Phx.IR.Instruction assignInstruction = Phx.IR.ValueInstruction.NewUnaryExpression(function,
         Phx.Common.Opcode.Assign, ptrToCharType, messageMemOpnd);

      enterInstruction.InsertAfter(assignInstruction);

      // Now for the return. Before we can build the return we need to
      // create a final exit point for the function. So we do that first.

      Phx.IR.LabelInstruction exitInstruction = Phx.IR.LabelInstruction.New(function,
         Phx.Common.Opcode.ExitFunction);

      function.LastInstruction.InsertBefore(exitInstruction);

      Phx.IR.Instruction returnInstruction = Phx.IR.BranchInstruction.NewReturn(function,
         Phx.Common.Opcode.Return, exitInstruction, assignInstruction.DestinationOperand);

      assignInstruction.InsertAfter(returnInstruction);


      // Now that we're done with IR for this unit, pop it off the context
      // stack (creating the unit pushed it on).

      function.Context.PopUnit();

      return function;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Creates the IR for the Base::Hello method.
   //
   // Returns:
   //
   //    FunctionUnit for Base::Hello
   //
   //--------------------------------------------------------------------------

   static public Phx.FunctionUnit
   BuildBaseHello()
   {
      // First we'll need a lifetime just for this function.

      Phx.Lifetime functionLifetime =
         Phx.Lifetime.New(Phx.LifetimeKind.Function, null);

       // Next we'll create the functionUnit. There are a lot of
       // parameters here.

      Phx.FunctionUnit function = Phx.FunctionUnit.New(functionLifetime,
         baseHelloSym, Phx.CodeGenerationMode.Native,
         typeTable, module.Architecture, module.Runtime, module, funcCounter++);

      // Associate some debugInfo, so we can track source positions.

      Phx.Debug.Info.New(lifetime, function);

      // For Hello there are no explicit parameters; just the implicit
      // this pointer. There is also no return value. The IR is then:
      //
      //          this = ENTERFUNC
      //          temporary  = ASSIGN [this + message]
      //                 CALL printf, $SG3, temporary
      //                 RETURN
      //                 EXITFUNC

      Phx.Symbols.Table funcSymTable = Phx.Symbols.Table.New(function, 64, true);

      // this is a parameter symbol with type Base *

      Phx.Symbols.LocalVariableSymbol thisSym = Phx.Symbols.LocalVariableSymbol.New(funcSymTable, 0,
         Phx.Name.New(functionLifetime, "this"), ptrToBaseType,
         Phx.Symbols.StorageClass.Parameter);

      thisSym.Alignment = Phx.Alignment.NaturalAlignment(ptrToBaseType);

      // One more important step. Each instruction carries debug
      // information along with it. We'd like to have this reflect the
      // source locations.

      function.CurrentDebugTag = function.DebugInfo.CreateTag(
         Phx.Name.New(lifetime, "base.h"), 6);

      // Grumble. Update tags on START/END.

      function.FirstInstruction.DebugTag = function.CurrentDebugTag;
      function.LastInstruction.DebugTag = function.CurrentDebugTag;

      // Build the ENTERFUNC

      Phx.IR.LabelInstruction enterInstruction = Phx.IR.LabelInstruction.New(function,
         Phx.Common.Opcode.EnterFunction, baseHelloSym);

      Phx.IR.VariableOperand thisOperand = Phx.IR.VariableOperand.New(function,
        ptrToBaseType, thisSym);

      enterInstruction.AppendDestination(thisOperand);

      function.FirstInstruction.InsertAfter(enterInstruction);

      // There needs to be an edge from start to enter

      Phx.IR.LabelOperand labelOperand = Phx.IR.LabelOperand.New(function, enterInstruction);
      function.FirstInstruction.AppendLabelSource(Phx.IR.LabelOperandKind.Technical, labelOperand);

      // temporary = ASSIGN [this + message]

      Phx.IR.MemoryOperand messageMemOpnd = Phx.IR.MemoryOperand.New(function,
         messageFieldSym.Field, null, thisOperand, 0,
         Phx.Alignment.NaturalAlignment(ptrToCharType),
         function.AliasInfo.IndirectAliasedMemoryTag,
         function.SafetyInfo.SafeTag);

      Phx.IR.Instruction assignInstruction = Phx.IR.ValueInstruction.NewUnaryExpression(function,
         Phx.Common.Opcode.Assign, ptrToCharType, messageMemOpnd);

      enterInstruction.InsertAfter(assignInstruction);

      // CALL printf, &$SG3, temporary

      Phx.Symbols.NonLocalVariableSymbol localBaseFormatStringSym =
         Phx.Symbols.NonLocalVariableSymbol.New(funcSymTable, baseFormatStringSym);

      Phx.IR.VariableOperand baseFormatStringOpnd = Phx.IR.VariableOperand.New(function,
         charType, localBaseFormatStringSym);

      baseFormatStringOpnd.ChangeToAddress();

      Phx.IR.Instruction callInstruction = Phx.IR.CallInstruction.New(function, Phx.Common.Opcode.Call,
         printfSym);

      callInstruction.AppendSource(baseFormatStringOpnd);
      callInstruction.AppendSource(assignInstruction.DestinationOperand);

      assignInstruction.InsertAfter(callInstruction);

      // Now for the return. Before we can build the return we need to
      // create a final exit point for the function. So we do that first.

      Phx.IR.LabelInstruction exitInstruction = Phx.IR.LabelInstruction.New(function,
         Phx.Common.Opcode.ExitFunction);

      function.LastInstruction.InsertBefore(exitInstruction);

      Phx.IR.Instruction returnInstruction = Phx.IR.BranchInstruction.NewReturn(function,
         Phx.Common.Opcode.Return, exitInstruction);

      callInstruction.InsertAfter(returnInstruction);


      // Now that we're done with IR for this unit, pop it off the context
      // stack (creating the unit pushed it on).

      function.Context.PopUnit();

      return function;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Creates the IR for the Derived::Derived method.
   //
   // Returns:
   //
   //    FunctionUnit for Derived::Derived
   //
   //--------------------------------------------------------------------------

   static public Phx.FunctionUnit
   BuildDerivedCtor()
   {
      // First we'll need a lifetime just for this function.

      Phx.Lifetime functionLifetime =
         Phx.Lifetime.New(Phx.LifetimeKind.Function, null);

       // Next we'll create the functionUnit. There are a lot of
       // parameters here.

      Phx.FunctionUnit function = Phx.FunctionUnit.New(functionLifetime,
         derivedCtorSym, Phx.CodeGenerationMode.Native,
         typeTable, module.Architecture, module.Runtime, module, funcCounter++);

      // Associate some debugInfo, so we can track source positions.

      Phx.Debug.Info.New(lifetime, function);

      // When a derived class is constructed, it must first construct
      // the base class portion. So schematically the code for this
      // method is somethign like:
      //
      //     call Base::Base(msg);
      //     set vtable to &$Derived$Vtable
      //     initialize data members.
      //
      // In this case there are no data members, so the IR becomes:
      //
      //          this, msg = ENTERFUNC
      //                      CALL Base::Base, this, msg
      //     [this + vtable = ASSIGN &$Derived$Vtable
      //                      RETURN this;

      Phx.Symbols.Table funcSymTable = Phx.Symbols.Table.New(function, 64, true);

      // this is a parameter symbol with type Base *

      Phx.Symbols.LocalVariableSymbol thisSym = Phx.Symbols.LocalVariableSymbol.New(funcSymTable, 0,
         Phx.Name.New(functionLifetime, "this"), ptrToDerivedType,
         Phx.Symbols.StorageClass.Parameter);

      thisSym.Alignment = Phx.Alignment.NaturalAlignment(ptrToDerivedType);

      // msg is a parameter symbol with type char *

      Phx.Symbols.LocalVariableSymbol msgSym = Phx.Symbols.LocalVariableSymbol.New(funcSymTable, 0,
         Phx.Name.New(functionLifetime, "msg"), ptrToCharType,
         Phx.Symbols.StorageClass.Parameter);

      msgSym.Alignment = Phx.Alignment.NaturalAlignment(ptrToCharType);

      // One more important step. Each instruction carries debug
      // information along with it. We'd like to have this reflect the
      // source locations.

      function.CurrentDebugTag = function.DebugInfo.CreateTag(
         Phx.Name.New(lifetime, "main.cpp"), 5);

      // Grumble. Update tags on START/END.

      function.FirstInstruction.DebugTag = function.CurrentDebugTag;
      function.LastInstruction.DebugTag = function.CurrentDebugTag;

      // We can now create IR that refers to these symbols. But before
      // we do this we need to establish an entry point in the
      // function. The entry point names the point of control transfer
      // and also provides for parameter passing. In textual IR this
      // is simply an ENTERFUNC opcode where the parameter symbols are
      // destinations. The name is supplied by passing in the functionSymbol.
      // (<functionSymbol> is technically not an operand here, but just some
      // extra information).
      //
      //     this, msg = ENTERFUNC <functionSymbol>
      //
      // The order of parameters is important here. When writing IR in
      // text, the "first" destination operand (the head of the
      // destination operand list) is listed leftmost. So we want this
      // as the first destionation operand, and msg as the second.

      Phx.IR.LabelInstruction enterInstruction = Phx.IR.LabelInstruction.New(function,
         Phx.Common.Opcode.EnterFunction, derivedCtorSym);

      // We encapsulate symbol references with variable operands. Note
      // that we must again specify the type...

      Phx.IR.VariableOperand thisOperand = Phx.IR.VariableOperand.New(function,
         ptrToDerivedType, thisSym);

      Phx.IR.VariableOperand msgOpnd = Phx.IR.VariableOperand.New(function,
         ptrToCharType, msgSym);

      // Build the destination operand list.

      enterInstruction.AppendDestination(thisOperand);
      enterInstruction.AppendDestination(msgOpnd);

      // Now append this instruction to the function's instruction
      // list.

      function.FirstInstruction.InsertAfter(enterInstruction);

      // There needs to be an edge from start to enter

      Phx.IR.LabelOperand labelOperand = Phx.IR.LabelOperand.New(function, enterInstruction);
      function.FirstInstruction.AppendLabelSource(Phx.IR.LabelOperandKind.Technical, labelOperand);

      //                      CALL Base::Base, this, msg

      Phx.IR.CallInstruction callInstruction = Phx.IR.CallInstruction.New(function,
         Phx.Common.Opcode.Call, baseCtorSym);

      callInstruction.AppendSource(thisOperand);
      callInstruction.AppendSource(msgOpnd);

      enterInstruction.InsertAfter(callInstruction);

      // Now for the ASSIGN: [this + vtable] = ASSIGN $Derived$Vtable.
      //
      // For destination, we have a memory operand with base address
      // and field.

      Phx.IR.MemoryOperand vtableMemOpnd = Phx.IR.MemoryOperand.New(function,
         vtableFieldSym.Field, null, thisOperand, 0,
         Phx.Alignment.NaturalAlignment(ptrToDerivedType),
         function.AliasInfo.IndirectAliasedMemoryTag,
         function.SafetyInfo.SafeTag);
      vtableMemOpnd.Type = typeTable.GetUnmanagedPointerType(derivedVtableSym.Type);

      // For a source, we want to refer to $Base$Vtable's
      // address. Recall that $Base$Vtable is an initialized global
      // variable. To refer to it in IR we must first introduce it
      // into the local symbol table by building a non-local variable sym:

      Phx.Symbols.NonLocalVariableSymbol localDerivedVtableSym = Phx.Symbols.NonLocalVariableSymbol.New(
        funcSymTable, derivedVtableSym);

      Phx.IR.VariableOperand vtablePtrOpnd = Phx.IR.VariableOperand.New(function,
         derivedVtableSym.Type, localDerivedVtableSym);

      vtablePtrOpnd.ChangeToAddress();

      Phx.IR.Instruction vtableAssignInstr = Phx.IR.ValueInstruction.NewUnary(function,
         Phx.Common.Opcode.Assign, vtableMemOpnd, vtablePtrOpnd);

      callInstruction.InsertAfter(vtableAssignInstr);

      // Now for the return. Before we can build the return we need to
      // create a final exit point for the function. So we do that first.

      Phx.IR.LabelInstruction exitInstruction = Phx.IR.LabelInstruction.New(function,
         Phx.Common.Opcode.ExitFunction);

      function.LastInstruction.InsertBefore(exitInstruction);

      Phx.IR.Instruction returnInstruction = Phx.IR.BranchInstruction.NewReturn(function,
         Phx.Common.Opcode.Return, exitInstruction, thisOperand);

      vtableAssignInstr.InsertAfter(returnInstruction);


      // Now that we're done with IR for this unit, pop it off the context
      // stack (creating the unit pushed it on).

      function.Context.PopUnit();

      return function;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Creates the IR for the Derived::Hello method.
   //
   // Returns:
   //
   //    FunctionUnit for Derived::Hello
   //
   //--------------------------------------------------------------------------

   static public Phx.FunctionUnit
   BuildDerivedHello()
   {
      // First we'll need a lifetime just for this function.

      Phx.Lifetime functionLifetime =
         Phx.Lifetime.New(Phx.LifetimeKind.Function, null);

       // Next we'll create the functionUnit. There are a lot of
       // parameters here.

      Phx.FunctionUnit function = Phx.FunctionUnit.New(functionLifetime,
         derivedHelloSym, Phx.CodeGenerationMode.Native,
         typeTable, module.Architecture, module.Runtime, module, funcCounter++);

      // Associate some debugInfo, so we can track source positions.

      Phx.Debug.Info.New(lifetime, function);

      // For Derived::Hello there are no explicit parameters; just the
      // implicit this pointer. So the IR is:
      //
      //          this = ENTERFUNC
      //          tmp0 = CALL "Base::GetMessage", this
      //                 CALL printf, &$SG4, tmp0
      //                 RETURN
      //                 EXITFUNC
      //
      // Note that we take advantage of the fact that it is trivial to
      // convert a Derived * this to a Base * this.

      Phx.Symbols.Table funcSymTable = Phx.Symbols.Table.New(function, 64, true);

      // this is a parameter symbol with type Derived *

      Phx.Symbols.LocalVariableSymbol thisSym = Phx.Symbols.LocalVariableSymbol.New(funcSymTable, 0,
         Phx.Name.New(functionLifetime, "this"), ptrToDerivedType,
         Phx.Symbols.StorageClass.Parameter);

      thisSym.Alignment = Phx.Alignment.NaturalAlignment(ptrToDerivedType);

      // One more important step. Each instruction carries debug
      // information along with it. We'd like to have this reflect the
      // source locations.

      function.CurrentDebugTag = function.DebugInfo.CreateTag(
         Phx.Name.New(lifetime, "main.cpp"), 6);

      // Grumble. Update tags on START/END.

      function.FirstInstruction.DebugTag = function.CurrentDebugTag;
      function.LastInstruction.DebugTag = function.CurrentDebugTag;

      // Build the ENTERFUNC

      Phx.IR.LabelInstruction enterInstruction = Phx.IR.LabelInstruction.New(function,
        Phx.Common.Opcode.EnterFunction, derivedHelloSym);

      Phx.IR.VariableOperand thisOperand = Phx.IR.VariableOperand.New(function,
        ptrToBaseType, thisSym);

      enterInstruction.AppendDestination(thisOperand);

      function.FirstInstruction.InsertAfter(enterInstruction);

      // There needs to be an edge from start to enter

      Phx.IR.LabelOperand labelOperand = Phx.IR.LabelOperand.New(function, enterInstruction);
      function.FirstInstruction.AppendLabelSource(Phx.IR.LabelOperandKind.Technical, labelOperand);

      // temporary = CALL "Base::GetMessage", this

      Phx.IR.Instruction callGetMessageInstr = Phx.IR.CallInstruction.NewExpression(function,
         Phx.Common.Opcode.Call, ptrToCharType, baseGetMessageSym);

      callGetMessageInstr.AppendSource(thisOperand);

      enterInstruction.InsertAfter(callGetMessageInstr);

      // CALL printf, &$SG4, temporary

      Phx.Symbols.NonLocalVariableSymbol localDerivedFormatStringSym =
         Phx.Symbols.NonLocalVariableSymbol.New(funcSymTable, derivedFormatStringSym);

      Phx.IR.VariableOperand derivedFormatStringOpnd = Phx.IR.VariableOperand.New(function,
         charType, localDerivedFormatStringSym);

      derivedFormatStringOpnd.ChangeToAddress();

      Phx.IR.Instruction callPrintfInstr = Phx.IR.CallInstruction.New(function,
         Phx.Common.Opcode.Call, printfSym);

      callPrintfInstr.AppendSource(derivedFormatStringOpnd);
      callPrintfInstr.AppendSource(callGetMessageInstr.DestinationOperand);

      callGetMessageInstr.InsertAfter(callPrintfInstr);

      // Now for the return. Before we can build the return we need to
      // create a final exit point for the function. So we do that first.

      Phx.IR.LabelInstruction exitInstruction = Phx.IR.LabelInstruction.New(function,
         Phx.Common.Opcode.ExitFunction);

      function.LastInstruction.InsertBefore(exitInstruction);

      Phx.IR.Instruction returnInstruction = Phx.IR.BranchInstruction.NewReturn(function,
         Phx.Common.Opcode.Return, exitInstruction);

      callPrintfInstr.InsertAfter(returnInstruction);

      // Now that we're done with IR for this unit, pop it off the context
      // stack (creating the unit pushed it on).

      function.Context.PopUnit();

      return function;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Creates the IR for the main method.
   //
   // Returns:
   //
   //    FunctionUnit for main.
   //
   //--------------------------------------------------------------------------

   static public Phx.FunctionUnit
   BuildMainFunc()
   {
      // First we'll need a lifetime just for this function.

      Phx.Lifetime functionLifetime =
         Phx.Lifetime.New(Phx.LifetimeKind.Function, null);

       // Next we'll create the functionUnit. There are a lot of
       // parameters here.

      Phx.FunctionUnit function = Phx.FunctionUnit.New(functionLifetime,
         mainSymbol, Phx.CodeGenerationMode.Native,
         typeTable, module.Architecture, module.Runtime, module, funcCounter++);

      // Associate some debugInfo, so we can track source positions.

      Phx.Debug.Info.New(lifetime, function);

      // For main there are no explicit or implicit parameters.
      // The code here is somewhat more complex than the other
      // functions:
      //
      //                 ENTERFUNC
      //          tmp0 = CALL "operator new", sizeof(Base)
      //         base1 = CALL "Base::Base", tmp0, &$SG1
      //          tmp1 = CALL "operator new", sizeof(Derived)
      //         base2 = CALL "Derived::Derived", tmp1, $SG2
      //          tmp2 = ASSIGN [base1 + vtable]
      //                 CALL [tmp2 + hello_offs], base1
      //          tmp3 = ASSIGN [base2 + vtable]
      //                 CALL [tmp3 + hello_offs], base2
      //                 RETURN 0
      //                 EXITFUNC
      //
      // Note that the compiler must explicitly call the memory
      // allocator with the size of each object, and then call
      // the constructor to fill in that object.
      //
      // The two final calls are perhaps the most interesting.  These
      // are virtual calls, so the compiler must locate the function
      // to call by going indirect through the vtable. This requires
      // two loads: one to get the vtable from the object, and a second
      // to get the function pointer from the vtable.

      Phx.Symbols.Table funcSymTable = Phx.Symbols.Table.New(function, 64, true);

      // main has two local variables; declare them now.

      Phx.Symbols.LocalVariableSymbol base1Sym = Phx.Symbols.LocalVariableSymbol.NewAuto(
         funcSymTable, 0, Phx.Name.New(functionLifetime, "base1"),
         ptrToBaseType);

      base1Sym.Alignment = Phx.Alignment.NaturalAlignment(ptrToBaseType);

      Phx.Symbols.LocalVariableSymbol base2Sym = Phx.Symbols.LocalVariableSymbol.NewAuto(
         funcSymTable, 0, Phx.Name.New(functionLifetime, "base2"),
         ptrToBaseType);

      base2Sym.Alignment = Phx.Alignment.NaturalAlignment(ptrToBaseType);

      // Each instruction carries debug information along with
      // it. This information is picked up from context when IR is
      // created. So seed things with the source location of the
      // first statement in the function.

      function.CurrentDebugTag = function.DebugInfo.CreateTag(
         Phx.Name.New(lifetime, "main.cpp"), 9);

      // Grumble. Update tag on START. Defer END until we reach
      // the last source line...

      function.FirstInstruction.DebugTag = function.CurrentDebugTag;

      // Build the ENTERFUNC

      Phx.IR.LabelInstruction enterInstruction = Phx.IR.LabelInstruction.New(function,
        Phx.Common.Opcode.EnterFunction, mainSymbol);

      function.FirstInstruction.InsertAfter(enterInstruction);

      // There needs to be an edge from start to enter

      Phx.IR.LabelOperand labelOperand = Phx.IR.LabelOperand.New(function, enterInstruction);
      function.FirstInstruction.AppendLabelSource(Phx.IR.LabelOperandKind.Technical, labelOperand);

      //          tmp0 = CALL "operator new", sizeof(Base)

      function.CurrentDebugTag = function.DebugInfo.CreateTag(
         Phx.Name.New(lifetime, "main.cpp"), 11);

      Phx.IR.ImmediateOperand sizeofBaseOpnd = Phx.IR.ImmediateOperand.New(function,
         function.RegisterUIntType, baseClassType.ByteSize);

      Phx.IR.CallInstruction callInstr1 = Phx.IR.CallInstruction.NewExpression(function,
         Phx.Common.Opcode.Call, ptrToUnkType, operatorNewSym);

      callInstr1.AppendSource(sizeofBaseOpnd);

      enterInstruction.InsertAfter(callInstr1);

      //         base1 = CALL "Base::Base", tmp0, &$SG1

      Phx.Symbols.NonLocalVariableSymbol localFirstHelloStringSym =
         Phx.Symbols.NonLocalVariableSymbol.New(funcSymTable, firstHelloStringSym);

      Phx.IR.VariableOperand firstHelloStringOpnd = Phx.IR.VariableOperand.New(function,
         charType, localFirstHelloStringSym);

      Phx.IR.VariableOperand base1Opnd = Phx.IR.VariableOperand.New(function,
         ptrToBaseType, base1Sym);

      firstHelloStringOpnd.ChangeToAddress();

      Phx.IR.CallInstruction callInstr2 = Phx.IR.CallInstruction.New(function,
         Phx.Common.Opcode.Call, baseCtorSym);

      callInstr2.AppendDestination(base1Opnd);
      callInstr2.AppendSource(callInstr1.DestinationOperand);
      callInstr2.AppendSource(firstHelloStringOpnd);

      callInstr1.InsertAfter(callInstr2);

      //          tmp1 = CALL "operator new", sizeof(Derived)

      function.CurrentDebugTag = function.DebugInfo.CreateTag(
         Phx.Name.New(lifetime, "main.cpp"), 12);

      Phx.IR.ImmediateOperand sizeofDerivedOpnd = Phx.IR.ImmediateOperand.New(function,
        function.RegisterUIntType, derivedClassType.ByteSize);

      Phx.IR.CallInstruction callInstr3 = Phx.IR.CallInstruction.NewExpression(function,
         Phx.Common.Opcode.Call, ptrToUnkType, operatorNewSym);

      callInstr3.AppendSource(sizeofDerivedOpnd);

      callInstr2.InsertAfter(callInstr3);

      //         base2 = CALL "Derived::Derived", tmp1, &$SG2

      Phx.Symbols.NonLocalVariableSymbol localSecondHelloStringSym =
         Phx.Symbols.NonLocalVariableSymbol.New(funcSymTable, secondHelloStringSym);

      Phx.IR.VariableOperand secondHelloStringOpnd = Phx.IR.VariableOperand.New(function,
         charType, localSecondHelloStringSym);

      Phx.IR.VariableOperand base2Opnd = Phx.IR.VariableOperand.New(function,
         ptrToBaseType, base2Sym);

      secondHelloStringOpnd.ChangeToAddress();

      Phx.IR.CallInstruction callInstr4 = Phx.IR.CallInstruction.New(function,
         Phx.Common.Opcode.Call, derivedCtorSym);

      callInstr4.AppendDestination(base2Opnd);
      callInstr4.AppendSource(callInstr3.DestinationOperand);
      callInstr4.AppendSource(secondHelloStringOpnd);

      // We must temporarily hammer the dst operand type to suppress
      // type check errors...

      callInstr4.DestinationOperand.Type = ptrToDerivedType;

      callInstr3.InsertAfter(callInstr4);

      //          tmp2 = ASSIGN [base1 + vtable]

      function.CurrentDebugTag = function.DebugInfo.CreateTag(
         Phx.Name.New(lifetime, "main.cpp"), 13);

      Phx.IR.MemoryOperand base1VtableMemOpnd = Phx.IR.MemoryOperand.New(function,
         vtableFieldSym.Field, null, base1Opnd, 0,
         Phx.Alignment.NaturalAlignment(ptrToBaseType),
         function.AliasInfo.IndirectAliasedMemoryTag,
         function.SafetyInfo.SafeTag);
      base1VtableMemOpnd.Type = ptrToUnkType;

      // We are limited here because we never fully described the type
      // of the vtable.

      Phx.IR.Instruction assignInstr1 = Phx.IR.ValueInstruction.NewUnaryExpression(function,
         Phx.Common.Opcode.Assign, ptrToUnkType, base1VtableMemOpnd);

      callInstr4.InsertAfter(assignInstr1);

      //                 CALL [tmp2 + hello_offs], base1

      //  align type and alias tag are bogus.

      Phx.IR.MemoryOperand fnPtrMemOpnd1 = Phx.IR.MemoryOperand.New(function,
         baseHelloSym.Type, null, assignInstr1.DestinationOperand.AsVariableOperand, 0,
         Phx.Alignment.NaturalAlignment(ptrToBaseType),
         function.AliasInfo.IndirectAliasedMemoryTag,
         function.SafetyInfo.SafeTag);

      Phx.IR.CallInstruction callInstr5 = Phx.IR.CallInstruction.New(function,
         Phx.Common.Opcode.Call, fnPtrMemOpnd1);

      callInstr5.AppendSource(base1Opnd);

      assignInstr1.InsertAfter(callInstr5);

      //          tmp3 = ASSIGN [base2 + vtable]

      function.CurrentDebugTag = function.DebugInfo.CreateTag(
         Phx.Name.New(lifetime, "main.cpp"), 14);

      Phx.IR.MemoryOperand base2VtableMemOpnd = Phx.IR.MemoryOperand.New(function,
         vtableFieldSym.Field, null, base2Opnd, 0,
         Phx.Alignment.NaturalAlignment(ptrToBaseType),
         function.AliasInfo.IndirectAliasedMemoryTag,
         function.SafetyInfo.SafeTag);
      base2VtableMemOpnd.Type = ptrToUnkType;

      Phx.IR.Instruction assignInstr2 = Phx.IR.ValueInstruction.NewUnaryExpression(function,
         Phx.Common.Opcode.Assign, ptrToUnkType, base2VtableMemOpnd);

      callInstr5.InsertAfter(assignInstr2);

      //                 CALL [tmp2 + hello_offs], base1

      //  align type and alias tag are bogus.

      Phx.IR.MemoryOperand fnPtrMemOpnd2 = Phx.IR.MemoryOperand.New(function,
         derivedHelloSym.Type, null, assignInstr2.DestinationOperand.AsVariableOperand, 0,
         Phx.Alignment.NaturalAlignment(ptrToBaseType),
         function.AliasInfo.IndirectAliasedMemoryTag,
         function.SafetyInfo.SafeTag);

      Phx.IR.CallInstruction callInstr6 = Phx.IR.CallInstruction.New(function,
         Phx.Common.Opcode.Call, fnPtrMemOpnd2);

      callInstr6.AppendSource(base2Opnd);

      assignInstr2.InsertAfter(callInstr6);

      // Now for the return. Before we can build the return we need to
      // create a final exit point for the function. So we do that first.

      function.CurrentDebugTag = function.DebugInfo.CreateTag(
         Phx.Name.New(lifetime, "main.cpp"), 15);

      Phx.IR.LabelInstruction exitInstruction = Phx.IR.LabelInstruction.New(function,
         Phx.Common.Opcode.ExitFunction);

      function.LastInstruction.InsertBefore(exitInstruction);

      Phx.IR.Instruction returnInstruction = Phx.IR.BranchInstruction.NewReturn(function,
         Phx.Common.Opcode.Return, exitInstruction);

      Phx.IR.ImmediateOperand zeroOperand = Phx.IR.ImmediateOperand.New(function,
        function.RegisterIntType, 0);

      returnInstruction.AppendSource(zeroOperand);

      callInstr6.InsertAfter(returnInstruction);

      // Update source location on END.

      function.LastInstruction.DebugTag = function.CurrentDebugTag;

      // Now that we're done with IR for this unit, pop it off the context
      // stack (creating the unit pushed it on).

      function.Context.PopUnit();

      return function;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Create symbols and types for initialized data. Also create
   //    the initalizer data IR.
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   static public void
   BuildInitializedData()
   {
      // There are a few more symbols that we will need. The four
      // literal strings will both need symbols, as will the two
      // vtables for the class objects. So we'll need at least six
      // more symbols for this. Since these symbols represent
      // initialized global data values, it's probabaly time to begin
      // thinking about a representation for the initial values as
      // well (note that the initial values for the vtables will
      // contain referencs to the member functions).
      //
      // In Phx, initial values are represented by
      // DataInstrs. DataInstrs must live in suitable sections; and
      // which sections they appear in depends on what the data instrs
      // represent. In this case all our global data is read only, and
      // so they will appear in a section called .rdata (for read-only
      // data). And to represent the .rdata section we need yet
      // another symbol...
      //
      // It turns out there is "builtin" support for creating the
      // "well known" sections (later on we'll also need .text to hold
      // the code we create). We simply need to call runtime.ModuleInitialize
      // and the proper sections will be added to the module's sym
      // table. So let's do that now. (It's instructive to call
      // symbolTable.Dump() before and after this to see what shows up --
      // lots of exotic seeming sections like .xdata$x).

      runtime.ModuleInitialize(module);

      // However, there's no harm in creating our own section for
      // data; any of the builtin sections created by ModuleInitialize that
      // end up empty will be omitted from the final object file, and
      // the name of the section, while important, takes a backseat to
      // the section properties. So we'll create a new section for our
      // strings, and then put the vtables in the normal .rdata
      // section.

      Phx.Symbols.SectionSymbol stringSectSym = Phx.Symbols.SectionSymbol.New(symbolTable, 0,
         Phx.Name.New(lifetime, ".strings"));

      // We want this section to hold initialized, read only data. We
      // don't care about alignment, really, but will use 4 byte
      // minimum.

      stringSectSym.IsReadable = true;
      stringSectSym.IsInitialize = true;
      stringSectSym.Alignment = new Phx.Alignment(Phx.Alignment.Kind.AlignTo4Bytes);

      // Along with the sectionSymbol we need a section. At this point we have
      // to divulge that we are creating output in COFF.

      Phx.Coff.Section stringSection =
         Phx.Coff.Section.New(stringSectSym);

      stringSectSym.Section = stringSection;

      // Now to create the initial values. Note that we have two
      // identical copies of "Hello" here; clever compilers will
      // combine these into just one string, but must guard against
      // programs that write into literal string constants!

      String helloString = "Hello";
      String baseFormatString = "%s from Base\n";
      String derivedFormatString = "%s from Derived\n";

      // Now we need some symbols so we can refer to these strings in
      // our program IR. The names of these symbols are unimportant,
      // so we'll just make something up. The type of each symbol is
      // an array of characters with the appropriate length.

      firstHelloStringSym  =
         CreateInitializedString(helloString, "$SG1", stringSectSym);
      secondHelloStringSym =
         CreateInitializedString(helloString, "$SG2", stringSectSym);
      baseFormatStringSym =
         CreateInitializedString(baseFormatString, "$SG3", stringSectSym);
      derivedFormatStringSym =
         CreateInitializedString(derivedFormatString, "$SG4", stringSectSym);

      // So much for the strings. Now for the vtables. As before, we will be vague
      // on their exact format, but we do need to give it proper size.

      // First, look up the standard .rdata section.

      Phx.Symbols.SectionSymbol rdataSectionSymbol =
         symbolTable.ExternIdMap.Lookup((uint) Phx.Symbols.PredefinedCxxILId.Constant)
         as Phx.Symbols.SectionSymbol;
      Phx.Section rdataSection = rdataSectionSymbol.Section;

      // Do the base vtable

      Phx.Name baseVtableName = Phx.Name.New(lifetime, "$Base$Vtable");
      Phx.Types.Type baseVtableType = Phx.Types.AggregateType.New(typeTable,
         typeTable.NativePointerBitSize, null);
      baseVtableSym = Phx.Symbols.GlobalVariableSymbol.New(symbolTable,
         externalIdCounter++, baseVtableName, baseVtableType,
         Phx.Symbols.Visibility.GlobalDefinition);
      baseVtableSym.AllocationBaseSectionSymbol = rdataSectionSymbol;
      baseVtableSym.Alignment = new Phx.Alignment(Phx.Alignment.Kind.AlignTo4Bytes);

      // C# is funny about this
      //baseVtableSym.NeedsToBeEmitted = true;

      // Now build the vtable initializer...

      Phx.IR.DataInstruction baseVtableInstr = Phx.IR.DataInstruction.New(rdataSection.DataUnit,
         Phx.Utility.BitsToBytes(baseVtableType.BitSize));

      // At offset 0, put a reference to Base::Hello().

      Phx.Targets.Architectures.Fixup baseHelloReference = architecture.NewFixup(lifetime,
         Phx.Targets.Architectures.Fixup.Kind.Ptr32, 0, baseHelloSym);

      baseVtableInstr.AddFixup(baseHelloReference);
      rdataSection.AppendInstruction(baseVtableInstr);
      baseVtableSym.Location = Phx.Symbols.DataLocation.New(baseVtableInstr);

      // Do the derived vtable

      Phx.Name derivedVtableName = Phx.Name.New(lifetime, "$Derived$Vtable");
      Phx.Types.Type derivedVtableType = Phx.Types.AggregateType.New(typeTable,
         typeTable.NativePointerBitSize, null);
      derivedVtableSym = Phx.Symbols.GlobalVariableSymbol.New(symbolTable,
         externalIdCounter++, derivedVtableName, derivedVtableType,
         Phx.Symbols.Visibility.GlobalDefinition);
      derivedVtableSym.AllocationBaseSectionSymbol = rdataSectionSymbol;
      derivedVtableSym.Alignment = new Phx.Alignment(Phx.Alignment.Kind.AlignTo4Bytes);
      // derivedVtableSym.NeedsToBeEmitted = true;

      // Now build the vtable initializer...

      Phx.IR.DataInstruction derivedVtableInstr = Phx.IR.DataInstruction.New(rdataSection.DataUnit,
         Phx.Utility.BitsToBytes(derivedVtableType.BitSize));

      // At offset 0, put a reference to Derived::Hello().

      Phx.Targets.Architectures.Fixup derivedHelloReference = architecture.NewFixup(lifetime,
         Phx.Targets.Architectures.Fixup.Kind.Ptr32, 0, derivedHelloSym);

      derivedVtableInstr.AddFixup(derivedHelloReference);
      rdataSection.AppendInstruction(derivedVtableInstr);
      derivedVtableSym.Location = Phx.Symbols.DataLocation.New(derivedVtableInstr);
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Create an initialized string.
   //
   // Returns:
   //
   //    Symbol representing the initialized string.
   //
   //--------------------------------------------------------------------------

   static public Phx.Symbols.GlobalVariableSymbol
   CreateInitializedString
   (
      String           value,
      String           symbolicName,
      Phx.Symbols.SectionSymbol stringSectSym
   )
   {
      uint length = (uint) value.Length + 1;

      Phx.Name stringName =
         Phx.Name.New(lifetime, symbolicName);

      Phx.Types.Type stringType =
         Phx.Types.UnmanagedArrayType.New(typeTable, Phx.Utility.BytesToBits(length),
            null, charType);

      Phx.Symbols.GlobalVariableSymbol stringSym =
         Phx.Symbols.GlobalVariableSymbol.New(symbolTable, externalIdCounter++,
            stringName, stringType, Phx.Symbols.Visibility.File);
      stringSym.AllocationBaseSectionSymbol = stringSectSym;
      stringSym.Alignment = new Phx.Alignment(Phx.Alignment.Kind.AlignTo1Byte);
      // stringSym.NeedsToBeEmitted = true;

      Phx.Section stringSection = stringSectSym.Section;

      // Data Instructions are attached to Data Units.

      Phx.IR.DataInstruction stringInstr =
         Phx.IR.DataInstruction.New(stringSection.DataUnit, length);

      // Copy value and place a terminating zero in the buffer.

      stringInstr.WriteString(0, value);

      // Add the DataInstruction to the section and define the string's location
      // to be the start of the DataInstruction.

      stringSection.AppendInstruction(stringInstr);
      stringSym.Location = Phx.Symbols.DataLocation.New(stringInstr);

      return stringSym;
   }

   //--------------------------------------------------------------------------
   //
   // Description:
   //
   //    Run the phases on a function unit.
   //
   // Returns:
   //
   //    Nothing.
   //
   //--------------------------------------------------------------------------

   static public void
   ExecutePhases
   (
      Phx.Unit functionUnit
   )
   {
      Phx.Threading.Context context = Phx.Threading.Context.GetCurrent();
      context.PushUnit(functionUnit);

      phaseConfiguration.PhaseList.DoPhaseList(functionUnit);

      context.PopUnit();
   }
}
