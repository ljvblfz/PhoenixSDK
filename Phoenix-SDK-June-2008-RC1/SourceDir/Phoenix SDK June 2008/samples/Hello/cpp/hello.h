//-----------------------------------------------------------------------------
//
// Copyright (C) Microsoft Corporation.  All Rights Reserved.
//
// Description:
//
//    A program that creates Phoenix IR for a simple C++ program
//    and produces a linkable object file.
//
//-----------------------------------------------------------------------------

public
ref class Hello
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
   //    structures we'll need to access as classstatic variables.
   //
   //--------------------------------------------------------------------------

   static Phx::ModuleUnit                 ^ module;
   static Phx::Targets::Runtimes::Runtime ^ runtime;
   static Phx::Targets::Architectures::Architecture       ^ architecture;
   static Phx::Lifetime                   ^ lifetime;
   static Phx::Types::Table               ^ typeTable;
   static Phx::Symbols::Table                ^ symbolTable;
   static Phx::Phases::PhaseConfiguration        ^ phaseConfiguration;

   static Phx::Types::Type                ^ charType;
   static Phx::Types::PointerType             ^ ptrToCharType;
   static Phx::Types::PointerType             ^ ptrToUnkType;
   static Phx::Types::AggregateType            ^ baseClassType;
   static Phx::Types::PointerType             ^ ptrToBaseType;
   static Phx::Types::AggregateType            ^ derivedClassType;
   static Phx::Types::PointerType             ^ ptrToDerivedType;

   static Phx::Symbols::FunctionSymbol              ^ printfSym;
   static Phx::Symbols::FunctionSymbol              ^ operatorNewSym;
   static Phx::Symbols::FunctionSymbol              ^ baseCtorSym;
   static Phx::Symbols::FunctionSymbol              ^ baseHelloSym;
   static Phx::Symbols::FunctionSymbol              ^ baseGetMessageSym;
   static Phx::Symbols::FunctionSymbol              ^ derivedCtorSym;
   static Phx::Symbols::FunctionSymbol              ^ derivedHelloSym;
   static Phx::Symbols::FunctionSymbol              ^ mainSymbol;

   static Phx::Symbols::FieldSymbol             ^ vtableFieldSym;
   static Phx::Symbols::FieldSymbol             ^ messageFieldSym;

   static Phx::Symbols::GlobalVariableSymbol         ^ baseVtableSym;
   static Phx::Symbols::GlobalVariableSymbol         ^ derivedVtableSym;
   static Phx::Symbols::GlobalVariableSymbol         ^ baseFormatStringSym;
   static Phx::Symbols::GlobalVariableSymbol         ^ derivedFormatStringSym;
   static Phx::Symbols::GlobalVariableSymbol         ^ firstHelloStringSym;
   static Phx::Symbols::GlobalVariableSymbol         ^ secondHelloStringSym;

   static UInt32                          externalIdCounter = 100;
   static UInt32                          funcCounter = 1;

public:

   static int
   Run
   (
      array<String ^> ^ arguments
   );

   static Phx::Targets::Runtimes::Runtime ^
   Initialize
   (
      array<String ^> ^ argv
   );

   static Phx::Phases::PhaseConfiguration ^
   BuildPhaseList();

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

   ref class Encode : Phx::Phases::Phase
   {
   public:

      static Encode ^
      New
      (
         Phx::Phases::PhaseConfiguration ^ config
      );

   protected:

      virtual void
      Execute
      (
         Phx::Unit ^ unit
      ) override;
   };

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

   ref class Lister : Phx::Phases::Phase
   {
   public:

      static Lister ^
      New
      (
         Phx::Phases::PhaseConfiguration ^ config
      );

   protected:

      virtual void
      Execute
      (
         Phx::Unit ^ unit
      ) override;
   };

   static void
   BuildExternalSymbols();

   static void
   BuildBaseClass();

   static void
   BuildDerivedClass();

   static void
   BuildMain();

   static Phx::FunctionUnit ^
   BuildBaseCtor();

   static Phx::FunctionUnit ^
   BuildBaseGetMessage();

   static Phx::FunctionUnit ^
   BuildBaseHello();

   static Phx::FunctionUnit ^
   BuildDerivedCtor();

   static Phx::FunctionUnit ^
   BuildDerivedHello();

   static Phx::FunctionUnit ^
   BuildMainFunc();

   static void
   BuildInitializedData();

   static Phx::Symbols::GlobalVariableSymbol ^
   CreateInitializedString
   (
      String ^             value,
      String ^             symbolicName,
      Phx::Symbols::SectionSymbol ^ stringSectSym
   );

   static void
   ExecutePhases
   (
      Phx::Unit ^ functionUnit
   );
};
