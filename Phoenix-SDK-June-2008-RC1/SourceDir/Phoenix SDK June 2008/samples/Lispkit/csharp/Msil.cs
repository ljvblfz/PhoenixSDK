using System;
using System.Collections.Generic;
using System.Text;
using System.CodeDom.Compiler;
using System.IO;
using System.Diagnostics;
using Lispkit.Sexp;
using Lispkit.Secd;

namespace Lispkit.CodeGen.Msil
{
   /// <summary>
   /// Lispkit MSIL compiler implementation.
   /// </summary>
   class Compiler : Lispkit.Compiler
   {
      // The input source file name.
      private string sourceFileName;

      // The module unit for the application.
      private Phx.PEModuleUnit moduleUnit;

      // The phase configuration.
      private Phx.Phases.PhaseConfiguration phaseConfiguration;

      // The list of function units that were built during
      // compilation.
      private List<Phx.FunctionUnit> functionUnits;

      // The number of lambda functions that were created.
      // This member is used to give unique names to anonymous functions.
      private int lambdaCount;

      // The function unit that is currently being built.
      private Phx.FunctionUnit currentFunctionUnit;

      // The logical last instruction in the current function unit.
      // We insert new instructions after this one.
      private Phx.IR.Instruction currentInstruction;

      // The stack of operands that forms the context of the application.
      private Stack<Phx.IR.Operand> operandStack;

      // Whether to generate a program debug database (PDB) for the module.
      private bool generatePdb;

      //
      // Class builders that hold information about classes (such as their
      // methods).
      //

      // Describes the program class $Lispkit.
      private ClassBuilder programClassBuilder;

      // Describes the class SExp.
      private ClassBuilder sexpClassBuilder;

      // Describes the class IntAtom.
      private ClassBuilder intAtomClassBuilder;

      // Describes the class SymAtom.
      private ClassBuilder symAtomClassBuilder;

      // Describes the class Cons.
      private ClassBuilder consClassBuilder;

      // Describes the classes FuncDelegate0, FuncDelegate1, 
      // FuncDelegate2, FuncDelegate3, and FuncDelegate4.            
      private ClassBuilder[] delegateClassBuilders;

      // Describes the classes FuncAtom0, FuncAtom1, 
      // FuncAtom2, FuncAtom3, and FuncAtom4.
      private ClassBuilder[] funcAtomClassBuilders;      

      // Creates a new instance of the Compiler class.
      public Compiler(string sourceFileName, 
         Phx.Targets.Runtimes.Runtime msilRuntime, bool generatePdb)
      {
         this.generatePdb = generatePdb;
         this.sourceFileName = sourceFileName;
         this.phaseConfiguration = BuildPhaseList(msilRuntime);
         this.lambdaCount = 0;
         this.functionUnits = new List<Phx.FunctionUnit>();
         this.currentInstruction = null;
         this.operandStack = new Stack<Phx.IR.Operand>();
      }

      // Pushes the given Operand object onto the operand stack.
      private void PushOperand(Phx.IR.Operand operand)
      {
         this.operandStack.Push(operand);
      }

      // Pops the top Operand object from the operand stack.
      private Phx.IR.Operand PopOperand()
      {
         return this.operandStack.Pop();
      }

      // Retrieves the Operand object that is at the top of the
      // operand stack.
      private Phx.IR.Operand TopOperand
      {
         get { return this.operandStack.Peek(); }
      }

      #region Compiler Members

      // Compiles the given S-expression.
      public override SExp Compile(Lispkit.Sexp.SExp e)
      {
         // Generate the output file name by changing the extension of
         // the source file name to ".exe".
         string outputFileName = Path.ChangeExtension(
            this.sourceFileName, ".exe");

         // Generate the base assembly.
         if (!this.GenerateBaseAssembly(outputFileName))
         {
            return SymAtom.ERROR;
         }
         try
         {
            // Raise the base assembly into memory. 
            this.LoadBaseAssembly(outputFileName);

            // Build type information.
            this.BuildClassTypes();
         }
         catch (Exception ex)
         {
            Output.ReportError(0, Error.InternalError, ex.Message);
            this.moduleUnit = null;
            return SymAtom.ERROR;
         }

         // Build the program, starting from the entry method.
         SExp sexp = this.BuildUserProgram(e);

         // If no errors were reported, execute the phase list for 
         // each FunctionUnit in the current module.
         if (Output.ErrorCount == 0)
         {                              
            this.ExecutePhases();
            this.moduleUnit.Close();
         }

         return sexp;
      }

      #endregion

      // Builds the user portion of the Lispkit program.
      private SExp BuildUserProgram(SExp e)
      {
         // Remember the previous state.
         Phx.FunctionUnit previousFunctionUnit = this.currentFunctionUnit;
         Phx.IR.Instruction previousInstruction = this.currentInstruction;
         
         // Generate the entry-point method $Main.
         Phx.FunctionUnit entryPointFunctionUnit = 
            this.CreateFunctionUnit(null, "$Main", 
               this.programClassBuilder.ClassType, 
               this.GetArgumentTypes(new Cons(SymAtom.NIL, SymAtom.NIL)),
               true);

         // Create a variable to hold the user argument list.

         Phx.Symbols.LocalVariableSymbol argListSymbol =
            Phx.Symbols.LocalVariableSymbol.New(
               this.currentFunctionUnit.SymbolTable, 0,
               Phx.Name.New(this.currentFunctionUnit.Lifetime, "argList"),
               this.sexpClassBuilder.ClassPointerType,
               Phx.Symbols.StorageClass.Parameter
            );

         // Append the operand to the destination list of
         // the enter instruction.

         this.currentFunctionUnit.FirstEnterInstruction.AppendDestination(
            Phx.IR.VariableOperand.New(this.currentFunctionUnit,
               argListSymbol.Type, argListSymbol));         

         // Create an operand for the argument list.
         Phx.IR.Operand argListOperand = Phx.IR.VariableOperand.New(
            this.currentFunctionUnit, argListSymbol.Type, argListSymbol);

         // Push it onto the operand stack.
         this.PushOperand(argListOperand);

         // Establish the 'anchor' instruction to append future instructions
         // after.
         this.currentInstruction = 
            entryPointFunctionUnit.LastInstruction.Previous;

         // Assuming the entry method is named 'E', generate:
         /* private static Sexp $Main(Sexp argument)
          * {
          *    $Lispkit $this = new $Lispkit();
          *    $this.E(argument);
          * }
          */

         // Create symbol for local variable "$this".
         
         Phx.Symbols.LocalVariableSymbol programInstanceSymbol = 
            Phx.Symbols.LocalVariableSymbol.New(
               this.currentFunctionUnit.SymbolTable, 0,
               Phx.Name.New(this.currentFunctionUnit.Lifetime, "$this"), 
               this.programClassBuilder.ClassPointerType, 
               Phx.Symbols.StorageClass.Auto
            );

         // Append the operand to the destination list of
         // the enter instruction.

         this.currentFunctionUnit.FirstEnterInstruction.AppendDestination(
            Phx.IR.VariableOperand.New(this.currentFunctionUnit,
               programInstanceSymbol.Type, programInstanceSymbol));   

         // Call $Lispkit class constructor.

         Phx.IR.Operand programInstanceOperand = 
            Phx.IR.VariableOperand.New(
               this.currentFunctionUnit, 
               programInstanceSymbol.Type,
               programInstanceSymbol);

         Phx.IR.Instruction newInstruction = 
            Phx.IR.CallInstruction.New(
               this.currentFunctionUnit, 
               Phx.Common.Opcode.NewObj,
               this.programClassBuilder.GetMethod(".ctor"));

         newInstruction.AppendDestination(programInstanceOperand);

         // Append the instruction to the instruction stream.
         this.AppendInstruction(e.ID, newInstruction);
          
         // Compile the remainder of the program.

         SExp sexp = Comp(e, SymAtom.NIL, new Cons(Instructions.AP, 
             new Cons(Instructions.STOP, SymAtom.NIL)));

         // Generate a call instruction if the first element of the program
         // is a LAMBDA expression.
         // Otherwise, we insert the instructions for the initial LET or 
         // LETREC expression directly into the entry-point function.

         if (e.Car.Equals(SymAtom.LAMBDA))
         {
            // The corresponding entry function is the last function in
            // the list.
            Phx.FunctionUnit entryMethod = 
               this.functionUnits[this.functionUnits.Count - 1];
             
            // Load arguments.
            this.LoadArgumentList(e.Cdr, this.PopOperand());

            // Call the entry function.
            this.CallMethod(e, entryMethod.FunctionSymbol, 
               programInstanceOperand);
         }
          
         // 'Apply' the item at the top of the operand stack.
         // If the top item is a function operand, call it and push the result
         // onto the stack.
         Phx.IR.Operand returnOperand;
         Phx.IR.Operand top = this.PopOperand();
         if (top is Phx.IR.FunctionOperand)
         {
            // Load arguments.
            this.LoadArgumentList(e.Cddr.Car.Cddr, this.PopOperand());

            // Call the function.
            this.CallMethod(e, top.AsFunctionOperand.FunctionSymbol, 
               programInstanceOperand);

            // Get the return value.
            returnOperand = this.PopOperand();
         }
         else
         {           
            returnOperand = top;
         }
         // Finish creation of the entry point method.
         this.FinishFunctionUnit(entryPointFunctionUnit, returnOperand);
         
         // Restore previous state.        
         this.currentFunctionUnit = previousFunctionUnit;
         this.currentInstruction = previousInstruction;
          
         return sexp;
      }

      // Builds the phase list for the compiler.
      private static Phx.Phases.PhaseConfiguration BuildPhaseList(
         Phx.Targets.Runtimes.Runtime runtime)
      {
         // Create the phase configuration.
         Phx.Phases.PhaseConfiguration phaseConfiguration =
            Phx.Phases.PhaseConfiguration.New(
               Phx.GlobalData.GlobalLifetime, "phases");

         // We raised an existing assembly into memory. These phases will
         // lower any changes we make back out to the assembly.
         Phx.Phases.Phase[] phases = {
            Lispkit.MarkTailCallPhase.New(phaseConfiguration),
            Phx.MirLowerPhase.New(phaseConfiguration),            
            Phx.Targets.Runtimes.CanonicalizePhase.New(phaseConfiguration),                
            Phx.Targets.Runtimes.LowerPhase.New(phaseConfiguration),            
            Phx.Targets.Runtimes.SwitchLowerPhase.New(phaseConfiguration),
            Phx.StackAllocatePhase.New(phaseConfiguration),
            Phx.Targets.Runtimes.FrameGenerationPhase.New(phaseConfiguration),
            Phx.Graphs.BlockLayoutPhase.New(phaseConfiguration),
            Phx.FlowOptimizer.Phase.New(phaseConfiguration),
            Lispkit.ApplyTailCallPhase.New(phaseConfiguration),       
         };
         Phx.Phases.PhaseList phaseList = phaseConfiguration.PhaseList;
         foreach (Phx.Phases.Phase phase in phases)
         {
            phaseList.AppendPhase(phase);
         }

         // Add any runtime-specific phases.
         Phx.GlobalData.GetFirstTargetRuntime().AddPhases(phaseConfiguration);

         // Add any plug-in phases.
         Phx.GlobalData.BuildPlugInPhases(phaseConfiguration);

         return phaseConfiguration;
      }

      // Emits instructions to create a delegate object instance.
      private Phx.IR.Operand BuildDelegate(SExp sexp, 
         Phx.Symbols.FunctionSymbol functionSymbol, int argCount)
      {
         // Emit a ldftn instruction that loads a pointer to the given
         // function. We will pass the loaded function pointer to the 
         // constructor of the delegate class.

         Phx.IR.Instruction ldFtn = Phx.IR.ValueInstruction.NewUnaryExpression(
            this.currentFunctionUnit,
            Phx.Targets.Architectures.Msil.Opcode.ldftn,
            this.currentFunctionUnit.TypeTable.GetMsilNativeIntType(false),
            Phx.IR.FunctionOperand.New(this.currentFunctionUnit,
               functionSymbol));

         // Because we're working in LIR, assign registers.
         ldFtn.DestinationOperand.Register = 
            Phx.Targets.Architectures.Msil.Register.SR0;

         // Append the instruction to the instruction stream.
         this.AppendInstruction(sexp.ID, ldFtn);

         // Generate delegate object instance.
                  
         // Get the ClassBuilder object that corresponds to the provided
         // argument count.
         ClassBuilder delegateClassBuilder = 
            this.delegateClassBuilders[argCount];

         // Get the pointer type to the delegate type.
         Phx.Types.Type delegatePointerType = 
            delegateClassBuilder.ClassPointerType;

         // Create an Operand object for the delegate instance.
         Phx.IR.VariableOperand delegateOperand =
            Phx.IR.VariableOperand.NewExpressionTemporary(
               this.currentFunctionUnit,
               delegatePointerType);

         // Call the constructor for the delegate class. The constructor
         // takes two arguments -- the class instance on which the delegate 
         // invokes the method and the pointer to the instance method that 
         // the delegate represents. 

         Phx.IR.Instruction callCtorInstruction = Phx.IR.CallInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.NewObj,
            delegateClassBuilder.GetMethod(".ctor"));

         this.PushThisOperand();
         callCtorInstruction.AppendSource(this.PopOperand());
         callCtorInstruction.AppendSource(ldFtn.DestinationOperand);
         callCtorInstruction.AppendDestination(delegateOperand);

         // Append the instruction to the instruction stream.
         this.AppendInstruction(sexp.ID, callCtorInstruction);

         return callCtorInstruction.DestinationOperand;
      }

      // Emits instructions to create a FuncSym object instance.
      private void BuildFuncSym(SExp sexp, 
         Phx.Symbols.FunctionSymbol functionSymbol)
      {
         // Count the number of user-defined arguments that the function takes.
         // Report an error if there is no built-in Atom type that takes the
         // same number of arguments.
         int argCount = 
            functionSymbol.Type.AsFunctionType.UserDefinedParameters.Count;
         if (argCount >= this.funcAtomClassBuilders.Length)
         {
            Output.ReportError(Lispkit.Location.GetLine(sexp.ID), 
               Error.TooManyArguments, this.funcAtomClassBuilders.Length - 1);
            
            // Reset argument count and continue so that we can find other 
            // errors. Because at least one error is reported,
            // no code will be generated.
            argCount = 0;
         }

         // Build the delegate object that we will pass to the constructor 
         // of the FuncSym object.
         Phx.IR.Operand delegateOperand = 
            this.BuildDelegate(sexp, functionSymbol, argCount);

         // Get the ClassBuilder object that corresponds to the provided
         // argument count.
         ClassBuilder funcAtomClassBuilder = 
            this.funcAtomClassBuilders[argCount];

         // Create an Operand object for the FuncSym instance.
         Phx.IR.VariableOperand funcSymOperand =
            Phx.IR.VariableOperand.NewExpressionTemporary(
            this.currentFunctionUnit,
            funcAtomClassBuilder.ClassPointerType);

         // Call the constructor for the delegate class.

         Phx.IR.Instruction callCtorInstruction = Phx.IR.CallInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.NewObj,
            funcAtomClassBuilder.GetMethod(".ctor"));

         callCtorInstruction.AppendSource(delegateOperand);
         callCtorInstruction.AppendDestination(funcSymOperand);

         // Append the instruction to the instruction stream.
         this.AppendInstruction(sexp.ID, callCtorInstruction);

         // Push the new FuncSym object onto the operand stack.
         this.PushOperand(callCtorInstruction.DestinationOperand);
      }

      // Emits instructions to invoke an indirect call.
      private void CallMethodIndirect(SExp sexp, 
         Phx.IR.Operand functionOperand, List<Phx.IR.Operand> argList)
      {
         // Count the number of user-defined arguments that the function takes.
         // Report an error if there is no built-in Atom type that takes the
         // same number of arguments.
         int argCount = argList.Count;
         if (argCount >= this.funcAtomClassBuilders.Length)
         {
            Output.ReportError(0, Error.TooManyArguments,
               this.funcAtomClassBuilders.Length - 1);

            // Reset argument count and continue so that we can find other 
            // errors. Because at least one error is reported,
            // no code will be generated.
            argCount = 0;
         }

         // Get the ClassBuilder object that corresponds to the provided
         // argument count.
         ClassBuilder delegateClassBuilder = this.delegateClassBuilders[argCount];

         // Get the pointer type to the delegate type.
         Phx.Types.Type fnPtrType = delegateClassBuilder.ClassPointerType;
         
         // Emit a virtual call to access the appropriate FuncPtr property 
         // of the SExp object.

         Phx.IR.Instruction callInstruction = Phx.IR.CallInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.CallVirt,
            this.sexpClassBuilder.GetMethod(
               string.Concat("get_FuncPtr", argCount)));

         // Append 'this' pointer.
         callInstruction.AppendSource(functionOperand);
         // Append destination operand.
         callInstruction.AppendDestination(
            Phx.IR.VariableOperand.NewExpressionTemporary(
               this.currentFunctionUnit, fnPtrType));

         // Append the instruction to the instruction stream.
         this.AppendInstruction(sexp.ID, callInstruction);

         // Call the FuncDelegate.Invoke method to invoke the indirect method.
         this.CallMethod(sexp, delegateClassBuilder.GetMethod("Invoke"),
            callInstruction.DestinationOperand, argList);
      }

      // Emits instructions to invoke a direct call with 
      // an explicit argument list.
      private void CallMethod(SExp sexp, 
         Phx.Symbols.FunctionSymbol methodSymbol, 
         Phx.IR.Operand thisPointerOperand, List<Phx.IR.Operand> argList)
      {
         // Create a CallInstruction object with opcode CallVirt.
         Phx.IR.Instruction callInstruction = Phx.IR.CallInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.CallVirt,
            methodSymbol);

         // Append the 'this' pointer operand, if one was provided (e.g. the
         // method is an instance method).
         if (thisPointerOperand != null)
         {
            callInstruction.AppendSource(thisPointerOperand);
         }

         // Append user-specified arguments.
         foreach (Phx.IR.Operand arg in argList)
         {
            // All Lispkit functions take SExp objects as their arguments.
            // If the argument if a function operand, create a wrapper 
            // FuncSym object.
            Phx.IR.Operand a = arg;
            if (a is Phx.IR.FunctionOperand)
            {            
               this.BuildFuncSym(sexp, arg.Symbol.AsFunctionSymbol);
               a = this.PopOperand();
            }

            callInstruction.AppendSource(a);
         }

         // Append the destination operand of the call (the return value).
         // This is always an object of type SExp, but we use the ReturnType
         // property to remain flexible.
         callInstruction.AppendDestination(
            Phx.IR.VariableOperand.NewExpressionTemporary(
               this.currentFunctionUnit, 
               methodSymbol.FunctionType.ReturnType));

         // Append the instruction to the instruction stream.
         this.AppendInstruction(sexp.ID, callInstruction);

         // Push the destination operand (the return value operand)
         // onto the operand stack.
         this.PushOperand(callInstruction.DestinationOperand);
      }

      // Emits instructions to invoke a direct call. The arguments to the 
      // call are taken from the operand stack.
      private void CallMethod(SExp sexp, 
         Phx.Symbols.FunctionSymbol methodSymbol, 
         Phx.IR.Operand thisPointerOperand)
      {
         if (sexp == null)
         {
            // Build a dummy S-expression and map it to line and 
            // column number 0.
            sexp = new IntAtom(0);
            Lispkit.Location.Add(sexp.ID, 0, 0);
         }

         // Create a CallInstruction object with opcode CallVirt.
         Phx.IR.Instruction callInstruction = Phx.IR.CallInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.CallVirt,
            methodSymbol);

         // Append the 'this' pointer operand, if one was provided (e.g. the
         // method is an instance method).
         if (thisPointerOperand != null)
         {
            callInstruction.AppendSource(thisPointerOperand);
         }

         // Append user-specified arguments.
         int argCount = methodSymbol.FunctionType.UserDefinedParameters.Count;
         for (int i = 0; i < argCount; i++)
         {
            // Take the next argument from the operand stack.
            callInstruction.AppendSource(this.PopOperand());
         }

         // Append the destination operand of the call (the return value).
         Phx.Types.Type returnType = methodSymbol.FunctionType.ReturnType;
         if (! returnType.IsVoid)
         {
            callInstruction.AppendDestination(
               Phx.IR.VariableOperand.NewExpressionTemporary(
                  this.currentFunctionUnit, returnType));
         }

         // Append the instruction to the instruction stream.
         this.AppendInstruction(sexp.ID, callInstruction);

         // Push the destination operand (the return value operand)
         // onto the operand stack.
         this.PushOperand(callInstruction.DestinationOperand);
      }
            
      // Extracts the arguments in the given S-expression from the provided
      // base operand.
      private void LoadArgumentList(SExp argList, Phx.IR.Operand baseOperand)
      {
         // Count the number of arguments in the head of the list.
         int argCount = this.CountOperands(argList.Car);
         if (argCount == 0)
         {
            // Empty argument list.
            return;
         }
         if (argCount == 1)
         {
            // A single argument; return the base operand.
            this.PushOperand(baseOperand);
            return;
         }

         // Push each element of the list onto the operand stack.
         // We emit calls to the Car and Cdr properties to advance
         // through the list.

         Phx.IR.Operand operand;
         Stack<Phx.IR.Operand> argStack = new Stack<Phx.IR.Operand>();

         this.EmitCallCar(argList, baseOperand);
         operand = this.PopOperand();
         argStack.Push(operand);

         for (int n = 1; n < argCount; n++)
         {
            operand = baseOperand;
            for (int m = 0; m < n; m++)
            {
               this.EmitCallCdr(argList, operand);
               operand = this.PopOperand();
            }
            this.EmitCallCar(argList, operand);
            operand = this.PopOperand();
            argStack.Push(operand);
         }

         while (argStack.Count > 0)
         {
            this.PushOperand(argStack.Pop());
         }
      }
            
      // Generates MSIL code of the form e*n|c.
      private SExp Comp(SExp e, SExp n, SExp c)
      {
         if (e is Atom)
         {
            // Load the operand for the symbol onto the operand stack.
            this.LoadOperand(e);
            return new Cons(Instructions.LD, new Cons(Location(e, n), c));
         }
         else if (e.Car.Equals(SymAtom.QUOTE))
         {
            // Load the constant operand for the symbol onto the operand stack.
            this.LoadConstantOperand(e.Cadr);
            return new Cons(Instructions.LDC, new Cons(e.Cadr, c));          
         }
         else if (e.Car.Equals(SymAtom.ADD) || e.Car.Equals(SymAtom.SUB) || 
            e.Car.Equals(SymAtom.MUL) || e.Car.Equals(SymAtom.DIV) || 
            e.Car.Equals(SymAtom.REM))
         {
            // Emit code for the arithmetic expression.
            return this.CompArit(e, n, c);            
         }         
         else if (e.Car.Equals(SymAtom.EQ))
         {
            // Emit code for the EQ expression.
            return this.CompEq(e, n, c);
         }
         else if (e.Car.Equals(SymAtom.LEQ))
         {
            // Emit code for the LEQ expression.
            return this.CompLeq(e, n, c);            
         }
         else if (e.Car.Equals(SymAtom.CAR))
         {
            // Emit code for the CAR expression.
            return this.CompCar(e, n, c);            
         }
         else if (e.Car.Equals(SymAtom.CDR))
         {
            // Emit code for the CDR expression.
            return this.CompCdr(e, n, c);            
         }
         else if (e.Car.Equals(SymAtom.ATOM))
         {
            // Emit code for the ATOM expression.
            return this.CompAtom(e, n, c);            
         }
         else if (e.Car.Equals(SymAtom.CONS))
         {
            // Emit code for the CONS expression.
            return this.CompCons(e, n, c);
         }
         else if (e.Car.Equals(SymAtom.IF))
         {
            // Emit code for the CONS expression.
            return this.CompIf(e, n, c);
         }
         else if (e.Car.Equals(SymAtom.LAMBDA))
         {
            // Emit code for the LAMBDA expression.
            return this.CompLambda(e, n, c);            
         }
         else if (e.Car.Equals(SymAtom.LET))
         {
            // Emit code for the LET expression.
            return CompLet(e, n, c);            
         }
         else if (e.Car.Equals(SymAtom.LETREC))
         {
            // Emit code for the LETREC expression.
            return CompLetRec(e, n, c);
         }
         else
         {
            // Apply the function that sits at the top of the 
            // operand stack.

            // Compile the result.
            SExp result = Complis(e.Cdr, n, Comp(e.Car, n, 
               new Cons(Instructions.AP, c)));

            // Count the number of operands for the associated
            // function call.
            int argCount = this.CountOperands(e.Cdr);
            List<Phx.IR.Operand> argList = new List<Phx.IR.Operand>();
            for (int i = 0; i < argCount; i++)
            {
               // The call to Complis loads the operand stack with
               // the appropriate operands. Pop each operand and add
               // it to the front of the argument list.
               argList.Insert(0, this.PopOperand());
            }
            
            // Now pop the actual function operand from the 
            // stack.
            Phx.IR.Operand functionOperand = this.PopOperand();

            // If the operand is an actual FunctionOperand object, 
            // then we make a direct call.
            if (functionOperand.IsFunctionOperand)
            {
               // Push the 'this' pointer onto the operand stack.
               this.PushThisOperand();

               // Generate the function call.
               this.CallMethod(e, 
                  functionOperand.AsFunctionOperand.FunctionSymbol, 
                  this.PopOperand(), // pop 'this' from the operand stack
                  argList);
            }
            // Otherwise, we make an indirect call to the function.
            else
            {
               this.CallMethodIndirect(e, functionOperand, argList);
            }

            return result;
         }      
      }

      // Loads the operand for the provided symbol and pushes it onto the
      // operand stack.
      private void LoadOperand(SExp e)
      {
         // Look for the associated symbol in the symbol table of the
         // current function unit.
         Phx.Name name = 
            Phx.Name.New(this.currentFunctionUnit.Lifetime, e.Symbol);
         Phx.Symbols.Symbol symbol =
            this.currentFunctionUnit.SymbolTable.NameMap.Lookup(name);

         // If the symbol was not found in the local symbol table,
         // then it might be a symbol for a Lispkit user function.
         // Look up the symbol in the ClassBuilder object for the
         // user program.
         if (symbol == null)
         {
            symbol = this.programClassBuilder.GetMethod(e.Symbol);
            if (symbol != null)
            {
               Phx.IR.Operand operand = Phx.IR.FunctionOperand.New(
                  this.currentFunctionUnit, symbol.AsFunctionSymbol);
               this.PushOperand(operand);
               return;
            }
         }

         // If the symbol was not found in the symbol table or the 
         // program class, report the error.
         if (symbol == null)
         {
            // It is difficult to continue because we have nothing to push
            // onto the operand stack. Reporting a fatal error will throw
            // an exception to the top-level exception handler.
            Output.ReportFatalError(Lispkit.Location.GetLine(e.ID),
               Error.UndeclaredIdentifier, e.Symbol);
         }

         // If the symbol is a function symbol, then create a wrapper
         // object. All Lispkit user objects derive from SExp.
         if (symbol.IsFunctionSymbol)
         {
            this.BuildFuncSym(e, symbol.AsFunctionSymbol);
         }
         // Otherwise, create a VariableOperand object for the local
         // symbol.
         else
         {
            Phx.IR.Operand operand = Phx.IR.VariableOperand.New(
               this.currentFunctionUnit, symbol.Type, symbol);
            this.PushOperand(operand);
         }
      }

      // Counts the number of operands that are associated
      // with the given expression.
      private int CountOperands(SExp sexp)
      {
         if (sexp.Equals(SymAtom.NIL))
         {
            return 0;
         }

         return 1 + CountOperands(sexp.Cdr);
      }

      // Loads the operand for the provided expression and pushes it onto the
      // operand stack.
      private void LoadConstantOperand(SExp e)
      {
         Phx.IR.Operand operand = null;

         // The expression can be a cons expression, or a symbolic or numeric
         // atom.

         // If the expression is a cons expression 
         // (for example, '(QUOTE (4 21))', load the cons elements 
         // (car and cdr), and then emit code to create a new Cons object.
         if (e is Cons)
         {
            // Load Car.
            LoadConstantOperand(e.Car);
            Phx.IR.Operand carOperand = this.PopOperand();

            // Load Cdr.
            LoadConstantOperand(e.Cdr);
            Phx.IR.Operand cdrOperand = this.PopOperand();

            // Create the destination operand of type 'Cons'.
            Phx.IR.Operand destinationOperand =
               Phx.IR.VariableOperand.NewExpressionTemporary(
                  this.currentFunctionUnit,
                  this.consClassBuilder.ClassPointerType);

            // Call the Cons class constructor.
            // The constructor takes two arguments: the head and rest of
            // the list.

            Phx.IR.Instruction newInstruction = Phx.IR.CallInstruction.New(
               this.currentFunctionUnit, Phx.Common.Opcode.NewObj,
               this.consClassBuilder.GetMethod(".ctor"));

            // Append arguments.
            newInstruction.AppendSource(carOperand);
            newInstruction.AppendSource(cdrOperand);
            newInstruction.AppendDestination(destinationOperand);

            // Append the instruction to the instruction stream.
            this.AppendInstruction(e.ID, newInstruction);

            // The result is the constructed Cons object.
            operand = newInstruction.DestinationOperand;
         }
         // If the expression is a symbolic atom, emit code to create a 
         // new SymAtom object.
         else if (e is SymAtom)
         {
            // Create the destination operand of type 'SymAtom'.
            Phx.IR.Operand destinationOperand =
               Phx.IR.VariableOperand.NewExpressionTemporary(
                  this.currentFunctionUnit,
                  this.symAtomClassBuilder.ClassPointerType);

            // Call the SymAtom class constructor.
            // The constructor takes as its argument the literal string value
            // of the expression.

            Phx.IR.Instruction newInstruction = Phx.IR.CallInstruction.New(
               this.currentFunctionUnit, Phx.Common.Opcode.NewObj,
               this.symAtomClassBuilder.GetMethod(".ctor"));

            // Load literal string value.
            this.LoadString(e);
            Phx.IR.Operand ldstr = this.PopOperand();

            // Append arguments.
            newInstruction.AppendSource(ldstr);
            newInstruction.AppendDestination(destinationOperand);

            // Append the instruction to the instruction stream.
            this.AppendInstruction(e.ID, newInstruction);

            // The result is the constructed SymAtom object.
            operand = newInstruction.DestinationOperand;

            // Explicitly break the expression temporary because the
            // LoadString method assigns registers.
            ldstr.BreakExpressionTemporary();
         }
         // If the expression is a numeric atom, emit code to create a 
         // new IntAtom object.
         else if (e is IntAtom)
         {
            // Create the destination operand of type 'IntAtom'.
            Phx.IR.Operand destinationOperand =
               Phx.IR.VariableOperand.NewExpressionTemporary(
                  this.currentFunctionUnit,
                  this.intAtomClassBuilder.ClassPointerType);

            // Call the IntAtom class constructor.
            // The constructor takes as its argument the constant integer value
            // of the expression.

            Phx.IR.Instruction newInstruction = Phx.IR.CallInstruction.New(
               this.currentFunctionUnit, Phx.Common.Opcode.NewObj,
               this.intAtomClassBuilder.GetMethod(".ctor"));

            // Append arguments.
            newInstruction.AppendSource(Phx.IR.ImmediateOperand.New(
               this.currentFunctionUnit,
               this.currentFunctionUnit.TypeTable.Int32Type,
               e.IntValue));
            newInstruction.AppendDestination(destinationOperand);

            // Append the instruction to the instruction stream.
            this.AppendInstruction(e.ID, newInstruction);

            // The result is the constructed IntAtom object.            
            operand = newInstruction.DestinationOperand;
         }
         // Invalid case.
         else
         {
            Debug.Assert(false,
               "LoadConstantOperand: expression must be atom or cons.");
         }

         // Push the operand onto the stack.
         this.PushOperand(operand);
      }

      // Emits instructions to load the literal string value that is associated
      // with the given S-expression.
      private void LoadString(SExp sexp)
      {
         // Create local references to the current function unit and its parent
         // module unit.
         Phx.FunctionUnit functionUnit = this.currentFunctionUnit;
         Phx.ModuleUnit moduleUnit = functionUnit.ParentUnit.AsModuleUnit;

         // Load the 'object pointer to string' type. The type table of the 
         // module unit already contains a reference to the System.String type.
         Phx.Types.Type stringType = moduleUnit.TypeTable.GetObjectPointerType(
            moduleUnit.TypeTable.SystemStringAggregateType);

         // We use the value of the symbol as its name.
         Phx.Name stringName = Phx.Name.New(Phx.GlobalData.GlobalLifetime,
            sexp.Symbol);

         // Create a ConstantSymbol object to hold the literal string value.
         Phx.Symbols.ConstantSymbol stringSymbol =
            Phx.Symbols.ConstantSymbol.New(moduleUnit.SymbolTable, 0,
               stringName,
               moduleUnit.TypeTable.ObjectPointerSystemStringType,
               sexp.Symbol);

         // Create a memory operand to hold the value.
         Phx.IR.Operand stringOperand = Phx.IR.MemoryOperand.NewAddress(
            functionUnit, stringType, stringSymbol, null, 0,
            Phx.Alignment.NaturalAlignment(stringType),
            functionUnit.AliasInfo.NotAliasedMemoryTag);

         // Emit the MSIL ldstr instruction.
         Phx.IR.Instruction ldStr = Phx.IR.ValueInstruction.NewUnaryExpression(
            functionUnit, Phx.Targets.Architectures.Msil.Opcode.ldstr,
            stringType, stringOperand);

         // Append the instruction to the instruction stream.
         this.AppendInstruction(sexp.ID, ldStr);

         // Because we're emitting MSIL directly, assign registers.

         ldStr.DestinationOperand.Register =
            Phx.Targets.Architectures.Msil.Register.SR0;

         // Push the operand onto the stack.
         this.PushOperand(ldStr.DestinationOperand);
      }

      // Generates MSIL code for the arithmetic expression of the form e*n|c.
      private SExp CompArit(SExp e, SExp n, SExp c)
      {
         SExp instruction;
         Phx.Opcode opcode;

         // Generate the appropriate S-expression and opcode for the 
         // arithmetic instruction.

         if (e.Car.Equals(SymAtom.ADD))
         {
            instruction = Instructions.ADD;
            opcode = Phx.Common.Opcode.Add;
         }
         else if (e.Car.Equals(SymAtom.SUB))
         {
            instruction = Instructions.SUB;
            opcode = Phx.Common.Opcode.Subtract;
         }
         else if (e.Car.Equals(SymAtom.MUL))
         {
            instruction = Instructions.MUL;
            opcode = Phx.Common.Opcode.Multiply;
         }
         else if (e.Car.Equals(SymAtom.DIV))
         {
            instruction = Instructions.DIV;
            opcode = Phx.Common.Opcode.Divide;
         }
         else if (e.Car.Equals(SymAtom.REM))
         {
            instruction = Instructions.REM;
            opcode = Phx.Common.Opcode.Remainder;
         }
         else
         {
            throw new InvalidOperationException();
         }

         // Compile the result.
         SExp result = Comp(e.Cadr, n, Comp(e.Caddr, n,
            new Cons(instruction, c)));

         // Apply the arithmetic operation to the top two operands 
         // on the stack.

         // The result is a 32-bit integer.
         Phx.IR.Operand destinationOperand =
            Phx.IR.VariableOperand.NewExpressionTemporary(
            this.currentFunctionUnit, this.moduleUnit.TypeTable.Int32Type);

         // Pop the top two operands from the stack.
         Phx.IR.Operand source1 = this.PopOperand();
         Phx.IR.Operand source2 = this.PopOperand();

         // If either operand is not an immediate operand (e.g. an SExp
         // object), use the SExp.IntValue property to retrieve the 
         // integer value.

         if (!source1.IsImmediateOperand)
         {
            // Emit code to retrieve the SExp.IntValue property.
            
            Phx.IR.Operand thisOperand = source1;

            Phx.IR.Instruction callInstruction = Phx.IR.CallInstruction.New(
               this.currentFunctionUnit, Phx.Common.Opcode.CallVirt,
               this.sexpClassBuilder.GetMethod("get_IntValue"));

            // Append arguments.
            callInstruction.AppendSource(thisOperand);
            callInstruction.AppendDestination(
               Phx.IR.VariableOperand.NewExpressionTemporary(
                  this.currentFunctionUnit,
                  this.moduleUnit.TypeTable.Int32Type));

            // Append the instruction to the instruction stream.
            this.AppendInstruction(e.ID, callInstruction);

            // The first source operand is now the result of calling the 
            // IntValue property.
            source1 = callInstruction.DestinationOperand;
         }
         if (!source2.IsImmediateOperand)
         {
            // Emit code to retrieve the SExp.IntValue property.

            Phx.IR.Operand thisOperand = source2;

            Phx.IR.Instruction callInstruction = Phx.IR.CallInstruction.New(
                this.currentFunctionUnit, Phx.Common.Opcode.CallVirt,
                this.sexpClassBuilder.GetMethod("get_IntValue"));

            // Append arguments.
            callInstruction.AppendSource(thisOperand);
            callInstruction.AppendDestination(
               Phx.IR.VariableOperand.NewExpressionTemporary(
                  this.currentFunctionUnit,
                  this.moduleUnit.TypeTable.Int32Type));

            // Append the instruction to the instruction stream.
            this.AppendInstruction(e.ID, callInstruction);

            // The second source operand is now the result of calling the 
            // IntValue property.
            source2 = callInstruction.DestinationOperand;
         }

         // Emit the arithmetic instruction.
         Phx.IR.Instruction aritInstruction =
            Phx.IR.ValueInstruction.NewBinary(this.currentFunctionUnit,
            opcode, destinationOperand, source1, source2);

         // Append the instruction to the instruction stream.
         this.AppendInstruction(e.ID, aritInstruction);

         // All Lispkit user variables derive from the SExp class.
         // Emit code to create a new IntAtom object.  
         // The constructor takes as its argument the integer value
         // of the expression.

         destinationOperand = 
            Phx.IR.VariableOperand.NewExpressionTemporary(
               this.currentFunctionUnit,
               this.intAtomClassBuilder.ClassPointerType);

         Phx.IR.Instruction newInstruction = 
            Phx.IR.CallInstruction.New(
               this.currentFunctionUnit, 
               Phx.Common.Opcode.NewObj,
               this.intAtomClassBuilder.GetMethod(".ctor"));

         // Append arguments.
         newInstruction.AppendSource(aritInstruction.DestinationOperand);
         newInstruction.AppendDestination(destinationOperand);

         // Append the instruction to the instruction stream.
         this.AppendInstruction(e.ID, newInstruction);

         // Push the resulting IntAtom object operand onto the stack.
         this.PushOperand(destinationOperand);

         return result;
      }

      // Generates MSIL code for the EQ expression of the form e*n|c.
      private SExp CompEq(SExp e, SExp n, SExp c)
      {
         // Compile the result.
         SExp result = Comp(e.Cadr, n, Comp(e.Caddr, n, 
            new Cons(Instructions.EQ, c)));

         // Apply the EQ operation to the top two operands 
         // on the stack. In MSIL, we generate a method call that resembles
         // the following:
         //  SExp result = first.Eq(second);

         Phx.IR.Operand firstOperand = this.PopOperand();
         Phx.IR.Operand secondOperand = this.PopOperand();

         // Generate a call instruction for the SExp.Eq method.
         Phx.IR.Instruction callInstruction = Phx.IR.CallInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.CallVirt,
            this.sexpClassBuilder.GetMethod("Eq"));

         // Append arguments.
         callInstruction.AppendSource(firstOperand);  // 'this'
         callInstruction.AppendSource(secondOperand); // method argument
         callInstruction.AppendDestination(
            Phx.IR.VariableOperand.NewExpressionTemporary(
               this.currentFunctionUnit, 
               this.sexpClassBuilder.ClassPointerType));

         // Append the instruction to the instruction stream.
         this.AppendInstruction(e.ID, callInstruction);

         // Push the operand for the resulting SExp object onto the stack.
         this.PushOperand(callInstruction.DestinationOperand);
         
         return result;
      }

      // Generates MSIL code for the EQ expression of the form e*n|c.
      private SExp CompLeq(SExp e, SExp n, SExp c)
      {
         // Compile the result.
         SExp result = Comp(e.Cadr, n, Comp(e.Caddr, n, 
            new Cons(Instructions.LEQ, c)));

         // Apply the LEQ operation to the top two operands 
         // on the stack. In MSIL, we generate a method call that resembles
         // the following:
         //  SExp result = first.Leq(second);

         Phx.IR.Operand firstOperand = this.PopOperand();
         Phx.IR.Operand secondOperand = this.PopOperand();

         // Generate a call instruction for the SExp.Leq method.
         Phx.IR.Instruction callInstruction = Phx.IR.CallInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.CallVirt,
            this.sexpClassBuilder.GetMethod("Leq"));

         // Append arguments.
         callInstruction.AppendSource(firstOperand);  // 'this'
         callInstruction.AppendSource(secondOperand); // method argument
         callInstruction.AppendDestination(
            Phx.IR.VariableOperand.NewExpressionTemporary(
               this.currentFunctionUnit, 
               this.sexpClassBuilder.ClassPointerType));

         // Append the instruction to the instruction stream.
         this.AppendInstruction(e.ID, callInstruction);

         // Push the operand for the resulting SExp object onto the stack.
         this.PushOperand(callInstruction.DestinationOperand);

         return result;
      }

      // Generates MSIL code for the CAR expression of the form e*n|c.
      private SExp CompCar(SExp e, SExp n, SExp c)
      {
         // Compile the result.
         SExp result = Comp(e.Cadr, n, new Cons(Instructions.CAR, c));

         // Push the result of calling the CAR operation onto the stack.
         this.EmitCallCar(e, this.PopOperand());

         return result;
      }

      // Generates MSIL code for the CDR expression of the form e*n|c.
      private SExp CompCdr(SExp e, SExp n, SExp c)
      {
         // Compile the result.
         SExp result = Comp(e.Cadr, n, new Cons(Instructions.CDR, c));

         // Push the result of calling the CDR operation onto the stack.
         this.EmitCallCdr(e, this.PopOperand());

         return result;
      }

      // Generates MSIL code for the ATOM expression of the form e*n|c.
      private SExp CompAtom(SExp e, SExp n, SExp c)
      {
         // Compile the result.
         SExp result = Comp(e.Cadr, n, new Cons(Instructions.ATOM, c));

         // Apply the ATOM operation to the top operand 
         // on the stack. In MSIL, we generate a method call that resembles
         // the following:
         //  SExp result = source.IsAtom();

         Phx.IR.Operand sourceOperand = this.PopOperand();
                  
         // Create a CallInstruction object with opcode CallVirt.

         Phx.IR.Instruction callInstruction = Phx.IR.CallInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.CallVirt,
            this.sexpClassBuilder.GetMethod("IsAtom"));

         // Append arguments.
         callInstruction.AppendSource(sourceOperand);
         callInstruction.AppendDestination(
            Phx.IR.VariableOperand.NewExpressionTemporary(
            this.currentFunctionUnit, this.sexpClassBuilder.ClassPointerType));

         // Append the instruction to the instruction stream.
         this.AppendInstruction(e.ID, callInstruction);

         // Push the result onto the operand stack.
         this.PushOperand(callInstruction.DestinationOperand);

         return result;
      }

      // Generates MSIL code for the CONS expression of the form e*n|c.
      private SExp CompCons(SExp e, SExp n, SExp c)
      {
         // Compile the result.
         SExp result = Comp(e.Caddr, n, Comp(e.Cadr, n, 
            new Cons(Instructions.CONS, c)));

         // Apply the CONS operation to the top two operands 
         // on the stack. In MSIL, we generate a method call that resembles
         // the following:
         //  SExp result = new Cons(first, second);

         // Pop the arguments to the Cons class constructor.
         Phx.IR.Operand second = this.PopOperand();
         Phx.IR.Operand first = this.PopOperand();

         // Create the destination operand of type 'Cons'.
         Phx.IR.Operand destinationOperand =
            Phx.IR.VariableOperand.NewExpressionTemporary(
               this.currentFunctionUnit,
               this.consClassBuilder.ClassPointerType);

         // Call the Cons class constructor.
         // The constructor takes two arguments: the head and rest of
         // the list.
                  
         Phx.IR.Instruction newInstruction = Phx.IR.CallInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.NewObj,
            this.consClassBuilder.GetMethod(".ctor"));

         // Append arguments.
         newInstruction.AppendSource(first);
         newInstruction.AppendSource(second);
         newInstruction.AppendDestination(destinationOperand);

         // Append the instruction to the instruction stream.
         this.AppendInstruction(e.ID, newInstruction);

         // Push the new object onto the operand stack.
         this.PushOperand(newInstruction.DestinationOperand);

         return result;
      }

      // Generates MSIL code for the IF expression of the form e*n|c.
      private SExp CompIf(SExp e, SExp n, SExp c)
      {
         // Apply the IF operation. In MSIL, we generate a method call that 
         // resembles the following:
         //  if (s.IsTrue()) {
         //     ... true branch
         //  } else {
         //     ... false branch
         //  }

         Phx.IR.Instruction previousInstruction;

         // Build a new expression temporary for the result of the 
         // call to the SExp.IsTrue method.
         Phx.IR.Operand sourceConditionCodeOperand =
            Phx.IR.VariableOperand.NewExpressionTemporary(
               this.currentFunctionUnit,
               this.currentFunctionUnit.TypeTable.ConditionType);

         // Build a new expression temporary for the result of the 
         // conditional statement. The operand is of type 'SExp'.
         Phx.IR.Operand ifResultOperand =
            Phx.IR.VariableOperand.NewExpressionTemporary(
               this.currentFunctionUnit,
               this.sexpClassBuilder.ClassPointerType);

         // Remember the current instruction.
         Phx.IR.Instruction beforeBranchInstruction = this.currentInstruction;

         // Build a skeleton conditional branch block by using the 
         // BuildConditionalBranch method. This method produces label 
         // instructions for the true, false, and end branch targets.

         Phx.IR.Instruction tBranchInstruction;   // true branch
         Phx.IR.Instruction fBranchInstruction;   // false branch
         Phx.IR.Instruction endBranchInstruction; // end branch
         
         this.BuildConditionalBranch(e, sourceConditionCodeOperand,
            out tBranchInstruction, out fBranchInstruction, 
            out endBranchInstruction);

         // Now fill in the code for the true branch of the conditional 
         // statement.

         previousInstruction = this.currentInstruction;
         this.currentInstruction = tBranchInstruction;

         // Compile the result.
         SExp thenpt = Comp(e.Caddr, n, 
            new Cons(Instructions.JOIN, SymAtom.NIL));

         // All Lispkit functions take SExp objects as their arguments.
         // If the call to Comp produces a call to a lambda function,         
         // create a wrapper FuncSym object.
         if (e.Caddr is Cons && e.Caddr.Car.Equals(SymAtom.LAMBDA))
         {
            this.BuildFuncSym(e, 
              this.functionUnits[this.functionUnits.Count - 1].FunctionSymbol);
         }
         // Assign the operand at the top of the stack to the if result operand.
         this.EmitAssignReference(e, ifResultOperand, this.PopOperand());

         // Now fill in the code for the false branch of the conditional 
         // statement.

         this.currentInstruction = fBranchInstruction;

         // Compile the result.
         SExp elsept = Comp(e.Cadddr, n, 
            new Cons(Instructions.JOIN, SymAtom.NIL));

         // All Lispkit functions take SExp objects as their arguments.
         // If the call to Comp produces a call to a lambda function,         
         // create a wrapper FuncSym object.
         if (e.Cadddr is Cons && e.Cadddr.Car.Equals(SymAtom.LAMBDA))
         {
            this.BuildFuncSym(e, 
              this.functionUnits[this.functionUnits.Count - 1].FunctionSymbol);
         }
         // Assign the operand at the top of the stack to the if result operand.
         this.EmitAssignReference(e, ifResultOperand, this.PopOperand());
         
         // Now emit instructions for the conditional part.

         this.currentInstruction = beforeBranchInstruction;

         // Compile the result.
         SExp result = Comp(e.Cadr, n, new Cons(Instructions.SEL, 
            new Cons(thenpt, new Cons(elsept, c))));

         // Create an instruction to call the SExp.IsTrue method.

         Phx.IR.Instruction callInstruction = 
            Phx.IR.CallInstruction.New(
               this.currentFunctionUnit, 
               Phx.Common.Opcode.Call,
               this.sexpClassBuilder.GetMethod("IsTrue"));

         // Append arguments.
         callInstruction.AppendSource(this.PopOperand()); // 'this'
         callInstruction.AppendDestination(sourceConditionCodeOperand);

         // Append the instruction to the instruction stream.
         this.AppendInstruction(e.ID, callInstruction);
         // Push the result onto the operand stack.         
         this.PushOperand(ifResultOperand);
         this.currentInstruction = endBranchInstruction;         

         return result;
      }
      
      // Builds a skeleton conditional branch with the provided 
      // condition code operand. This method produces Instruction 
      // objects for the true, false, and end branch targets of the
      // conditional statement.
      private void BuildConditionalBranch(
         SExp s,
         Phx.IR.Operand sourceConditionCodeOperand,
         out Phx.IR.Instruction tBranchInstruction,
         out Phx.IR.Instruction fBranchInstruction,
         out Phx.IR.Instruction endBranchInstruction)
      {
         Phx.IR.BranchInstruction gotoInstruction;

         // Create label instructions for the true and false
         // arms of the branch. 

         Phx.IR.LabelInstruction trueLabelInstruction =
            Phx.IR.LabelInstruction.New(this.currentFunctionUnit);

         Phx.IR.LabelInstruction falseLabelInstruction =
            Phx.IR.LabelInstruction.New(this.currentFunctionUnit);

         // Create a common target for both branches to meet at.

         Phx.IR.LabelInstruction endLabelInstruction =
            Phx.IR.LabelInstruction.New(this.currentFunctionUnit);

         // Emit the conditional branch instruction.

         Phx.IR.Instruction branchInstruction = Phx.IR.BranchInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.ConditionalBranch,
            (int)Phx.ConditionCode.True, sourceConditionCodeOperand,
            trueLabelInstruction, falseLabelInstruction);

         // Append the instruction to the instruction stream.
         this.AppendInstruction(s.ID, branchInstruction);

         // Emit the label and assignment code for the true arm (this
         // includes branching to the common end point).

         // Append the instruction to the instruction stream.
         this.AppendInstruction(s.ID, trueLabelInstruction);
         tBranchInstruction = trueLabelInstruction;

         gotoInstruction = Phx.IR.BranchInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.Goto, 
            endLabelInstruction);

         // Append the instruction to the instruction stream.
         this.AppendInstruction(s.ID, gotoInstruction);

         // Emit the label and assignment code for the false arm (this
         // includes branching to the common end point).

         // Append the instruction to the instruction stream.
         this.AppendInstruction(s.ID, falseLabelInstruction);
         fBranchInstruction = falseLabelInstruction;

         gotoInstruction = Phx.IR.BranchInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.Goto, 
            endLabelInstruction);

         // Append the instruction to the instruction stream.
         this.AppendInstruction(s.ID, gotoInstruction);

         // Emit the final common jump target for both branches.

         this.AppendInstruction(s.ID, endLabelInstruction);
         endBranchInstruction = endLabelInstruction;
      }

      // Generates MSIL code for the LAMBDA expression of the form e*n|c.
      private SExp CompLambda(SExp e, SExp n, SExp c)
      {
         // Remember the previous function unit and instruction.
         // After we generate code for the lambda function, we reset
         // the current values back to the previous ones.
         Phx.FunctionUnit previousFunctionUnit = 
            this.currentFunctionUnit;
         Phx.IR.Instruction previousCurrentInstruction = 
            this.currentInstruction;

         // Build a FunctionUnit object to hold the lambda function.
         // First, we look for an existing name binding to the function.
         // For example, consider the following simple Lispkit program:
         //   (LET ident (ident LAMBDA (n) n))
         // In this program, ident is bound to the LAMBDA expression
         // that returns its single argument. In this case, we use the
         // function symbol that is bound to ident to build the 
         // function unit. Otherwise, we generate a unique name for the
         // function unit.

         Phx.FunctionUnit functionUnit;
         
         Phx.Symbols.FunctionSymbol functionSymbol = null;        
         if (this.lambdaBindings.TryGetValue(e.Cdr, out functionSymbol))
         {
            // Create the function unit from the existing symbol.
            functionUnit = this.CreateFunctionUnit(e, functionSymbol);
            this.currentFunctionUnit = functionUnit;

            // Allocate storage for the function symbol.
            this.AllocateMethodSymbol(moduleUnit, functionSymbol);

            // The function is a class function (method). 
            // Load the 'this' parameter and append it to the destination list
            // of the enter instruction.

            Phx.Types.Type thisClassPointerType = 
               this.programClassBuilder.ClassPointerType;

            Phx.Symbols.LocalVariableSymbol thisSymbol =
               Phx.Symbols.LocalVariableSymbol.New(
                  this.currentFunctionUnit.SymbolTable, 0,
                  Phx.Name.New(this.currentFunctionUnit.Lifetime, "$this"),
                  thisClassPointerType,
                  Phx.Symbols.StorageClass.Parameter
               );
            thisSymbol.IsThisParameter = true;

            this.currentFunctionUnit.FirstEnterInstruction.AppendDestination(
               Phx.IR.VariableOperand.New(this.currentFunctionUnit,
                  thisClassPointerType, thisSymbol));
         }
         else
         {
            // The LAMBDA expression is not bound. Create a new anonymous
            // function.

            functionUnit = this.CreateFunctionUnit(e,
               string.Format("L_{0}", this.lambdaCount), 
               this.programClassBuilder.ClassType,
               this.GetArgumentTypes(e.Cadr), false);

            this.lambdaCount++;

            // Add the new method to the program class method table.
            this.programClassBuilder.AddMethod(functionUnit.FunctionSymbol);
         }

         this.currentInstruction = functionUnit.LastInstruction.Previous;

         // Load parameters into the local symbol table.
         this.LoadSymbols(e.Cadr);
         
         // Compile the body of the function.
         SExp body = Comp(e.Caddr, new Cons(e.Cadr, n), 
            new Cons(Instructions.RTN, SymAtom.NIL));

         // Compile the result.
         SExp result = new Cons(Instructions.LDF, new Cons(body, c));

         // All Lispkit functions take SExp objects as their arguments.
         // If the function produces a call to another lambda function,
         // create a wrapper FuncSym object.
         if (e.Caddr is Cons && e.Caddr.Car.Equals(SymAtom.LAMBDA))
         {
            this.BuildFuncSym(e, 
              this.functionUnits[this.functionUnits.Count - 1].FunctionSymbol);
         }
         
         // Finish creation of the function unit by appending return 
         // and exit instructions.
         this.FinishFunctionUnit(functionUnit, this.PopOperand());

         // Restore the previous state.
         this.currentInstruction = previousCurrentInstruction;
         this.currentFunctionUnit = previousFunctionUnit;

         return result;
      }

      // Generates local variable symbols for each symbolic element
      // in the provided S-expression list.
      private void LoadSymbols(SExp symbolList)
      {
         // This method is recursive.
         // Base case: we've reached the end of the list.
         if (symbolList.Equals(SymAtom.NIL))
         {
            return;
         }
         // If the expression is a symbolic atom, create a local variable
         // symbol of type 'SExp' with the value of the symbol.
         else if (symbolList is SymAtom)
         {
            string symbolName = symbolList.Symbol;

            // Create a new LocalVariableSymbol object with storage
            // class Parameter and add it to the symbol table of the 
            // current function unit.

            Phx.Symbols.LocalVariableSymbol symbol =
               Phx.Symbols.LocalVariableSymbol.New(
                  this.currentFunctionUnit.SymbolTable, 0,
                  Phx.Name.New(this.currentFunctionUnit.Lifetime, symbolName),
                  this.sexpClassBuilder.ClassPointerType,
                  Phx.Symbols.StorageClass.Parameter
               );

            // Append the operand to the destination list of
            // the enter instruction.

            this.currentFunctionUnit.FirstEnterInstruction.AppendDestination(
               Phx.IR.VariableOperand.New(this.currentFunctionUnit,
                  symbol.Type, symbol));

            // Append the symbol type to the type builder of the function.
            this.currentFunctionUnit.FunctionTypeBuilder.AppendParameter(
               symbol.Type);   
         }
         // It is invalid for the symbol list to contain a numeric constant.
         else if (symbolList is IntAtom)
         {
            throw new InvalidOperationException();
         }
         // Recursive case: if the symbol is a list, load the first element 
         // and then the remainder of the list.
         else
         {
            Debug.Assert(symbolList is Cons);

            this.LoadSymbols(symbolList.Car);
            this.LoadSymbols(symbolList.Cdr);          
         }     
      }

      // Creates an array of Type objects for the given S-expression
      // list.
      private Phx.Types.Type[] GetArgumentTypes(SExp sexp)
      {
         List<Phx.Types.Type> argumentTypes = new List<Phx.Types.Type>();

         // Traverse the expression list. Because all Lispkit functions
         // take SExp objects, append the 'pointer to SExp' type to the 
         // array for each expression element.
         while (sexp is Cons)
         {
            argumentTypes.Add(this.sexpClassBuilder.ClassPointerType);
            sexp = sexp.Cdr;
         }
         return argumentTypes.ToArray();
      }

      // Creates an array of names for the given S-expression
      // list.
      private string[] GetArgumentNames(SExp sexp)
      {
         List<string> argumentNames = new List<string>();

         // Traverse the expression list and add the value of each
         // symbolic element to the list.
         while (sexp is Cons)
         {
            argumentNames.Add(sexp.Car.Symbol);
            sexp = sexp.Cdr;
         }
         return argumentNames.ToArray();
      }

      // Builds an array of Argument objects from the given
      // S-expression list.
      private Argument[] GetArgumentList(SExp args)
      {
         // Retrieve parallel type/name arrays for the 
         // arguments of the LAMBDA expression.
         Phx.Types.Type[] argTypes = this.GetArgumentTypes(args.Cadr);
         string[] argNames = this.GetArgumentNames(args.Cadr);
         Debug.Assert(argTypes.Length == argNames.Length);

         // Generate an array of Argument objects from the 
         // parallel arrays.
         Argument[] argList = new Argument[argTypes.Length];
         for (int i = 0; i < argList.Length; i++)
         {
            argList[i] = new Argument(argNames[i], argTypes[i]);
         }

         return argList;
      }

      // Generates MSIL code for the LET expression of the form e*n|c.
      private SExp CompLet(SExp e, SExp n, SExp c)
      {
         SExp m = new Cons(Vars(e.Cddr), n);
         SExp args = Exprs(e.Cddr);

         // Bind local symbols.
         this.LoadLetSymbols(m.Car, args);

         // Lispkit LET expressions are written with the body followed by 
         // variable bindings. We follow this concept by first generating
         // code for the body of the expression, followed by variable 
         // binding.

         // Remember the initial instruction so that we can later insert
         // instructions for variable bindings.
         Phx.IR.Instruction beforeBodyInstruction = this.currentInstruction;

         // Compile the body.
         SExp body = Comp(e.Cadr, m, new Cons(Instructions.RTN, SymAtom.NIL));

         // Remember the last instruction of the body.
         Phx.IR.Instruction afterBodyInstruction = this.currentInstruction;

         // Go back to the start of the LET block and compile the 
         // variable bindings.
         this.currentInstruction = beforeBodyInstruction;

         // Compile the result.
         SExp result = Complis(args, n, new Cons(Instructions.LDF, 
            new Cons(body, new Cons(Instructions.AP, c))));

         // Loads each operand argument list.
         int operandCount = LoadOperands(m.Car);
         Phx.IR.Operand[] operands = new Phx.IR.Operand[operandCount];
         for (int i = 0; i < operandCount; i++)
         {
            // The call to Complis pushed an operand onto the stack
            // for each function argument. Pop the operand and add it
            // to our local array.
            operands[i] = this.PopOperand();
         }

         // Bind non-function operands to their values.
         for (int i = 0; i < operandCount; i++)
         {
            Phx.IR.Operand operand = operands[i];

            if (!operand.IsFunctionOperand)
            {
               this.EmitAssignReference(e, operand, this.PopOperand());
            }
         }

         // Advance to the end of the LET body.
         this.currentInstruction = afterBodyInstruction;

         return result;
      }

      // Loads each operand in the provided S-expression list and returns
      // the number of loaded operands.
      private int LoadOperands(SExp sexp)
      {
         if (sexp.Equals(SymAtom.NIL))
         {
            return 0;
         }

         this.LoadOperand(sexp.Car);
         return 1 + this.LoadOperands(sexp.Cdr);
      }

      // Generates MSIL code for the LETREC expression of the form e*n|c.
      private SExp CompLetRec(SExp e, SExp n, SExp c)
      {
         SExp m = new Cons(Vars(e.Cddr), n);
         SExp args = Exprs(e.Cddr);

         // Bind local symbols.
         this.LoadLetSymbols(m.Car, args);

         // Remember the initial instruction so that we can later insert
         // instructions for variable bindings.
         Phx.IR.Instruction beforeBodyInstruction = this.currentInstruction;

         // Compile the body.
         SExp body = Comp(e.Cadr, m, new Cons(Instructions.RTN, SymAtom.NIL));

         // Remember the last instruction of the body.
         Phx.IR.Instruction afterBodyInstruction = this.currentInstruction;

         // Go back to the start of the LETREC block and compile the 
         // variable bindings.
         this.currentInstruction = beforeBodyInstruction;

         // Compile the result.
         SExp result = new Cons(Instructions.DUM, Complis(args, m,
               new Cons(Instructions.LDF, new Cons(body, 
                  new Cons(Instructions.RAP, c)))));

         // LETREC expressions are bound to LAMBDA expressions.
         // Emit code to call the bound expressions.
         while (this.operandStack.Count > 0 && 
            this.TopOperand.IsFunctionOperand)
         {
            // Pop the function operand from the stack.
            Phx.IR.FunctionOperand functionOperand = 
               this.PopOperand().AsFunctionOperand;

            // Load arguments.
            this.LoadArgumentList(args.Cdar, this.PopOperand());
            
            // Call the function.
            this.PushThisOperand();
            this.CallMethod(e, functionOperand.FunctionSymbol, 
               this.PopOperand());
         }

         // Advance to the end of the LET body.
         this.currentInstruction = afterBodyInstruction;

         return result;
      }

      // Bindings from S-expresions to function symbols that 
      // hold LAMBDA expressions.
      Dictionary<SExp, Phx.Symbols.FunctionSymbol> lambdaBindings =
         new Dictionary<SExp, Phx.Symbols.FunctionSymbol>();

      // Generates local variable symbols for each symbolic element
      // in the provided S-expression list.
      private void LoadLetSymbols(SExp symbolList, SExp args)
      {
         // This method resembles the LoadSymbols method, except that this
         // method allows symbolic atoms to be bound to LAMBDA expressions.

         // This method is recursive.
         // Base case: we've reached the end of the list.
         if (symbolList.Equals(SymAtom.NIL))
         {
            return;
         }
         // If the expression is a symbolic atom, create a local variable
         // symbol of type 'SExp' with the value of the symbol.
         else if (symbolList is SymAtom)
         {
            string symbolName = symbolList.Symbol;

            // Create a function symbol for the expression if the symbol is
            // bound to a LAMBDA expression.
            if (args.Car.Equals(SymAtom.LAMBDA))
            {
               // Retrieve the name and type pairing of each argument.
               Argument[] argList = this.GetArgumentList(args);

               // Add the method to the method table of the program class.
               Phx.Symbols.FunctionSymbol functionSymbol = 
                  this.programClassBuilder.AddMethod(
                     Phx.Symbols.Access.Private, 
                     Phx.Symbols.Visibility.GlobalDefinition,
                     true, false,
                     this.sexpClassBuilder.ClassPointerType,
                     symbolName,
                     argList);

               // Bind the function symbol to the expression.
               this.lambdaBindings.Add(args.Cdr, functionSymbol);
            }
            // Otherwise, bind the expression to a local variable symbol.
            else
            {
               Phx.Symbols.LocalVariableSymbol.New(
                  this.currentFunctionUnit.SymbolTable, 0,
                  Phx.Name.New(this.currentFunctionUnit.Lifetime, symbolName),
                  this.sexpClassBuilder.ClassPointerType,
                  Phx.Symbols.StorageClass.Auto);
            }
         }
         // It is invalid for the symbol list to contain a numeric constant.
         else if (symbolList is IntAtom)
         {
            throw new InvalidOperationException();
         }
         // Recursive case: if the symbol is a list, load the first element 
         // and then the remainder of the list.
         else
         {
            Debug.Assert(symbolList is Cons);

            this.LoadLetSymbols(symbolList.Car, args.Car);
            this.LoadLetSymbols(symbolList.Cdr, args.Cdr);
         }         
      }
      
      // Pushes the operand for the 'this' symbol onto the operand stack.
      private void PushThisOperand()
      {
         // The compiler uses the name '$this' for the implicit this
         // parameter. This allows user programs to use the name
         // 'this' for local variables.
         Phx.Name thisName = 
            Phx.Name.New(this.currentFunctionUnit.Lifetime, "$this");

         // Look up the symbol in the local symbol table.
         Phx.Symbols.Symbol thisSymbol = 
            this.currentFunctionUnit.SymbolTable.LookupByName(thisName);                  
         Debug.Assert(thisSymbol != null);

         // Push a new variable operand for the symbol onto the stack.
         this.PushOperand(Phx.IR.VariableOperand.New(
            this.currentFunctionUnit, thisSymbol.Type, thisSymbol));
      }

      // Emits code to assign a referece to the provided source operand
      // to the provided destination operand.
      private void EmitAssignReference(SExp s, 
         Phx.IR.Operand destinationOperand,
         Phx.IR.Operand sourceOperand)
      {
         // Create an Assign instruction and append it to the instruction
         // stream.

         Phx.IR.Instruction assignInstruction = 
            Phx.IR.ValueInstruction.NewUnary(
               this.currentFunctionUnit, 
               Phx.Common.Opcode.Assign,
               destinationOperand, 
               sourceOperand);

         // Append the instruction to the instruction stream.
         this.AppendInstruction(s.ID, assignInstruction);         
      }

      // Extracts the variable parts from the provided S-expression list.
      private static SExp Vars(SExp d)
      {
         if (d.Equals(SymAtom.NIL))
         {
            return SymAtom.NIL;
         }
         else
         {
            return new Cons(d.Caar, Vars(d.Cdr));
         }
      }

      // Extracts the expression parts from the provided S-expression list.
      private static SExp Exprs(SExp d)
      {
         if (d.Equals(SymAtom.NIL))
         {
            return SymAtom.NIL;
         }
         else
         {
            return new Cons(d.Cdar, Exprs(d.Cdr));
         }
      }

      // Compiles a list of expressions in the form
      // (LDC NIL) | ek*n | (CONS) | ... | e1*n | (CONS) | c
      private SExp Complis(SExp e, SExp n, SExp c)
      {
         if (e.Equals(SymAtom.NIL))
         {
            return new Cons(Instructions.LDC, new Cons(SymAtom.NIL, c));
         }
         else
         {
            return Complis(e.Cdr, n, Comp(e.Car, n, 
               new Cons(Instructions.CONS, c)));
         }
      }

      // Appends the given instruction to the instruction stream.
      private void AppendInstruction(int ID, Phx.IR.Instruction instruction)
      {
         // Retrieve the line and column numbers that are associated with
         // the provided S-expression.
         int line = Lispkit.Location.GetLine(ID);
         Debug.Assert(line >= 0);
         int column = Lispkit.Location.GetColumn(ID);
         Debug.Assert(column >= 0);

         // Set the current debug tag by using the line and column 
         // numbers and the current source file name.
         this.currentFunctionUnit.CurrentDebugTag =
            this.currentFunctionUnit.DebugInfo.CreateTag(Phx.Name.New(
               this.currentFunctionUnit.Lifetime, this.sourceFileName), 
            (uint) line, (ushort) column);

         // Insert the instruction.
         this.currentInstruction.InsertAfter(instruction);
         // Update the instruction pointer.
         this.currentInstruction = instruction;         
      }
            
      // Utility method to emit calls to the SExp.Car property.
      private void EmitCallCar(SExp sexp, Phx.IR.Operand operand)
      {
         // Emit code to retrieve the SExp.Car property.

         // Create a CallInstruction object with opcode CallVirt.
         Phx.IR.Instruction callInstruction = Phx.IR.CallInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.CallVirt,
            this.sexpClassBuilder.GetMethod("get_Car"));

         // Append arguments.
         callInstruction.AppendSource(operand);
         callInstruction.AppendDestination(
            Phx.IR.VariableOperand.NewExpressionTemporary(
               this.currentFunctionUnit, operand.Type));

         // Append the instruction to the instruction stream.
         this.AppendInstruction(sexp.ID, callInstruction);
         
         // Push the result of the call onto the operand stack.
         this.PushOperand(callInstruction.DestinationOperand);
      }

      // Utility method to emit calls to the SExp.Cdr property.
      private void EmitCallCdr(SExp sexp, Phx.IR.Operand operand)
      {
         // Emit code to retrieve the SExp.Car property.

         // Create a CallInstruction object with opcode CallVirt.
         Phx.IR.Instruction callInstruction = Phx.IR.CallInstruction.New(
            this.currentFunctionUnit, Phx.Common.Opcode.CallVirt,
            this.sexpClassBuilder.GetMethod("get_Cdr"));

         // Append arguments.
         callInstruction.AppendSource(operand);
         callInstruction.AppendDestination(
            Phx.IR.VariableOperand.NewExpressionTemporary(
               this.currentFunctionUnit, operand.Type));

         // Append the instruction to the instruction stream.
         this.AppendInstruction(sexp.ID, callInstruction);
         
         // Push the result of the call onto the operand stack.
         this.PushOperand(callInstruction.DestinationOperand);
      }
      
      // Loads the assembly at the given file name into memory. 
      private void LoadBaseAssembly(string fileName)
      {
         // Open the module unit.
         this.moduleUnit = Phx.PEModuleUnit.Open(fileName);

         // Create the MetadataEmitter object for the module unit.
         this.moduleUnit.MetadataEmitter = Phx.Metadata.Emitter.New(
            this.moduleUnit.Lifetime, this.moduleUnit);

         // Because we're modifying an existing binary, set the 
         // output image path to the source path.
         this.moduleUnit.OutputImagePath = fileName;

         // Traverse each function unit that contributes to 
         // the module and raise from Encoded IR (EIR) to 
         // Lower-level IR (LIR).
         foreach (Phx.FunctionUnit functionUnit in
            this.moduleUnit.GetEnumerableContributionUnit(
               Phx.ContributionUnitEnumerationKind.WritableFunctionUnit))
         {
            functionUnit.DisassembleToBeforeLayout();
         }
      }

      // Builds type information for the class types that are used by
      // the compiler.
      private void BuildClassTypes()
      {
         // Retrieve a local copy of the type table that is associated with
         // the module unit.
         Phx.Types.Table typeTable = this.moduleUnit.TypeTable;
                                             
         // Retrieve the AssemblySymbol object that is associated with
         // the mscorlib assembly.
         Phx.Symbols.AssemblySymbol mscorlibSymbol =
            this.LookupAssembly("mscorlib");

         // Create a reference to System.Object. Although we can use the 
         // Phx.Types.Table.SystemObjectAggregateType property to access
         // the System.Object type object, we use a ClassBuilder object
         // so that we can associate the type and its methods with a single
         // object.

         ClassBuilder objectClassBuilder = 
            this.BuildObjectClass(mscorlibSymbol);
                     
         // Build references to the SExp type and its derived types.
         // Because we are building class references and not definitions,
         // we only need to build references to the methods that 
         // we need.
                     
         // Build class Lispkit.Sexp.SExp.
         this.BuildSExpClass(objectClassBuilder);

         // Build class Lispkit.Sexp.IntAtom.
         this.BuildIntAtomClass();
         
         // Build class Lispkit.Sexp.SymAtom.
         this.BuildSymAtomClass();

         // Build class Lispkit.Sexp.Cons.
         this.BuildConsClass();

         // Build a definition for the $Lispkit class.
         this.BuildProgramClass(objectClassBuilder);

         // Create a reference to System.Delegate.
         ClassBuilder delegateClassBuilder =
            this.BuildDelegateClass(objectClassBuilder, mscorlibSymbol);

         // Create a reference to System.MulticastDelegate.
         ClassBuilder multicastDelegateClassBuilder =
            this.BuildMulticastDelegateClass(objectClassBuilder,
               delegateClassBuilder, mscorlibSymbol);

         // Build the following types:
         //  delegate SExp FuncDelegate0();
         //  delegate SExp FuncDelegate1(SExp arg0);
         //  delegate SExp FuncDelegate2(SExp arg0, SExp arg1);
         //  delegate SExp FuncDelegate3(SExp arg0, ..., SExp arg2);
         //  delegate SExp FuncDelegate4(SExp arg0, ..., SExp arg3);
         this.BuildFuncDelegateClasses(multicastDelegateClassBuilder);

         // Build the following types:
         //  FuncAtom0
         //  FuncAtom1
         //  FuncAtom2
         //  FuncAtom3
         //  FuncAtom4
         this.BuildFuncAtomClasses();
      }
                                  
      // Constructs a ClassBuilder object to hold the System.Object type and
      // its constructor method.
      private ClassBuilder BuildObjectClass(
         Phx.Symbols.AssemblySymbol mscorlibSymbol)
      {
         // Create a ClassBuilder object to hold the System.Object type.
         ClassBuilder objectClassBuilder = new MsilClassBuilder(
            this.moduleUnit, 
            "System.Object",
            null, // Object has no base class.
            Phx.Symbols.Access.Public,
            false, true);

         // Build a reference to the constructor method of the class.

         Phx.Symbols.FunctionSymbol ctorFunctionSymbol =
            objectClassBuilder.AddMethod(Phx.Symbols.Access.Public,
            Phx.Symbols.Visibility.GlobalReference, true, true,
            this.moduleUnit.TypeTable.VoidType, ".ctor", null);
         ctorFunctionSymbol.IsConstructor = true;

         // Now attach the class type to the mscorlib assembly reference.

         mscorlibSymbol.InsertInLexicalScope(
            objectClassBuilder.ClassTypeSymbol,
            Phx.Name.New(Phx.GlobalData.GlobalLifetime, "System.Object"));
         
         return objectClassBuilder;
      }

      // Constructs a ClassBuilder object to hold the System.Delegate type and
      // its constructor method.
      private ClassBuilder BuildDelegateClass(ClassBuilder objectClassBuilder,
         Phx.Symbols.AssemblySymbol mscorlibSymbol)
      {
         // Get a local copy of the type table that is associated with
         // the module unit.
         Phx.Types.Table typeTable = this.moduleUnit.TypeTable;

         // Get a local reference to the System.String type.
         Phx.Types.Type stringType = typeTable.GetObjectPointerType(
            typeTable.SystemStringAggregateType);

         ClassBuilder delegateClassBuilder = new MsilClassBuilder(
            this.moduleUnit, 
            "System.Delegate",
            objectClassBuilder.ClassType,
            Phx.Symbols.Access.Public, 
            false, true);

         // Build a reference to the constructor method of the class.

         Phx.Symbols.FunctionSymbol ctorFunctionSymbol =
            delegateClassBuilder.AddMethod(Phx.Symbols.Access.Family,
            Phx.Symbols.Visibility.GlobalReference, true, true,
            this.moduleUnit.TypeTable.VoidType, ".ctor", 
            new Argument[] { 
               new Argument("target", objectClassBuilder.ClassPointerType),
               new Argument("method", stringType) }
            );
         ctorFunctionSymbol.IsConstructor = true;

         // Now attach the class type to the mscorlib assembly reference.

         mscorlibSymbol.InsertInLexicalScope(
            delegateClassBuilder.ClassTypeSymbol,
            Phx.Name.New(Phx.GlobalData.GlobalLifetime, "System.Delegate"));

         return delegateClassBuilder;
      }

      // Constructs a ClassBuilder object to hold the System.MulticastDelegate 
      // type and its constructor method.
      private ClassBuilder BuildMulticastDelegateClass(
         ClassBuilder objectClassBuilder,
         ClassBuilder delegateClassBuilder,
         Phx.Symbols.AssemblySymbol mscorlibSymbol)
      {
         // Get a local copy of the type table that is associated with
         // the module unit.
         Phx.Types.Table typeTable = this.moduleUnit.TypeTable;

         // Get a local reference to the System.String type.
         Phx.Types.Type stringType = typeTable.GetObjectPointerType(
            typeTable.SystemStringAggregateType);

         ClassBuilder multicastDelegateClassBuilder = new MsilClassBuilder(
            this.moduleUnit, 
            "System.MulticastDelegate", 
            delegateClassBuilder.ClassType,
            Phx.Symbols.Access.Public, 
            false, true);

         // Build a reference to the constructor method of the class.

         Phx.Symbols.FunctionSymbol multicastDelegateCtor =
            multicastDelegateClassBuilder.AddMethod(Phx.Symbols.Access.Family,
            Phx.Symbols.Visibility.GlobalReference, true, true,
            this.moduleUnit.TypeTable.VoidType, ".ctor", 
            new Argument[] { 
               new Argument("target", objectClassBuilder.ClassPointerType),
               new Argument("method", stringType) });
         multicastDelegateCtor.IsConstructor = true;

         // Now attach the class type to the mscorlib assembly reference.

         mscorlibSymbol.InsertInLexicalScope(
            multicastDelegateClassBuilder.ClassTypeSymbol,
            Phx.Name.New(Phx.GlobalData.GlobalLifetime, 
               "System.MulticastDelegate"));

         return multicastDelegateClassBuilder;
      }

      // Constructs a ClassBuilder object to hold the SExp type and
      // its methods.
      private void BuildSExpClass(ClassBuilder objectClassBuilder)
      {
         // Get a local copy of the type table that is associated with
         // the module unit.
         Phx.Types.Table typeTable = this.moduleUnit.TypeTable;

         // Build class Lispkit.Sexp.SExp.
         ClassBuilder classBuilder = new MsilClassBuilder(
            this.moduleUnit,
            "Lispkit.Sexp.SExp",
            objectClassBuilder.ClassType,
            Phx.Symbols.Access.Public,
            false, false);

         // Add references to methods.

         // Properties.

         classBuilder.AddMethod(Phx.Symbols.Access.Public,
            Phx.Symbols.Visibility.GlobalReference,
            true, true, typeTable.Int32Type,
            "get_IntValue", null);
         classBuilder.AddMethod(Phx.Symbols.Access.Public,
            Phx.Symbols.Visibility.GlobalReference,
            true, true, classBuilder.ClassPointerType,
            "get_Car", null);
         classBuilder.AddMethod(Phx.Symbols.Access.Public,
            Phx.Symbols.Visibility.GlobalReference,
            true, true, classBuilder.ClassPointerType,
            "get_Cdr", null);

         // Methods.

         classBuilder.AddMethod(Phx.Symbols.Access.Public,
            Phx.Symbols.Visibility.GlobalReference,
            true, false, typeTable.ConditionType, "IsTrue", null);
         classBuilder.AddMethod(Phx.Symbols.Access.Public,
            Phx.Symbols.Visibility.GlobalReference,
            true, true, classBuilder.ClassPointerType, "Eq",
            new Argument[] { new Argument("A_0", classBuilder.ClassPointerType) });
         classBuilder.AddMethod(Phx.Symbols.Access.Public,
            Phx.Symbols.Visibility.GlobalReference,
            true, true, classBuilder.ClassPointerType, "Leq",
            new Argument[] { new Argument("A_0", classBuilder.ClassPointerType) });
         classBuilder.AddMethod(Phx.Symbols.Access.Public,
            Phx.Symbols.Visibility.GlobalReference,
            true, true, classBuilder.ClassPointerType, "IsAtom", null);

         this.sexpClassBuilder = classBuilder;
      }

      // Constructs a ClassBuilder object to hold the IntAtom type and
      // its methods.
      private void BuildIntAtomClass()
      {
         // Get a local copy of the type table that is associated with
         // the module unit.
         Phx.Types.Table typeTable = this.moduleUnit.TypeTable;

         ClassBuilder classBuilder = new MsilClassBuilder(this.moduleUnit, 
            "Lispkit.Sexp.IntAtom",
            this.sexpClassBuilder.ClassType, 
            Phx.Symbols.Access.Public, 
            false, false);

         // Add references to methods.

         classBuilder.AddMethod(Phx.Symbols.Access.Public, 
            Phx.Symbols.Visibility.GlobalReference,
            true, true, typeTable.VoidType, ".ctor",
            new Argument[] { new Argument("A_0", typeTable.Int32Type) });

         this.intAtomClassBuilder = classBuilder;
      }

      // Constructs a ClassBuilder object to hold the SymAtom type and
      // its methods.
      private void BuildSymAtomClass()
      {
         // Get a local copy of the type table that is associated with
         // the module unit.
         Phx.Types.Table typeTable = this.moduleUnit.TypeTable;

         // Get a local reference to the System.String type.
         Phx.Types.Type stringType = typeTable.GetObjectPointerType(
            typeTable.SystemStringAggregateType);                

         ClassBuilder classBuilder = new MsilClassBuilder(this.moduleUnit, 
            "Lispkit.Sexp.SymAtom",
            this.sexpClassBuilder.ClassType, 
            Phx.Symbols.Access.Public, 
            false, false);

         // Add references to methods.

         classBuilder.AddMethod(Phx.Symbols.Access.Public, 
            Phx.Symbols.Visibility.GlobalReference,
            true, true, typeTable.VoidType, ".ctor",
            new Argument[] { new Argument("A_0", stringType) });

         this.symAtomClassBuilder = classBuilder;
      }

      // Constructs a ClassBuilder object to hold the Cons type and
      // its methods.
      private void BuildConsClass()
      {
         // Get a local copy of the type table that is associated with
         // the module unit.
         Phx.Types.Table typeTable = this.moduleUnit.TypeTable;
                  
         ClassBuilder classBuilder = new MsilClassBuilder(this.moduleUnit, 
            "Lispkit.Sexp.Cons",
            this.sexpClassBuilder.ClassType, 
            Phx.Symbols.Access.Public, 
            false, false);

         // Add references to methods.

         Phx.Types.Type sexpPointerType = 
            this.sexpClassBuilder.ClassPointerType;

         classBuilder.AddMethod(Phx.Symbols.Access.Public, 
            Phx.Symbols.Visibility.GlobalReference,
            true, true, typeTable.VoidType, ".ctor",
            new Argument[] { new Argument("A_0", sexpPointerType),
                             new Argument("A_1", sexpPointerType) });

         this.consClassBuilder = classBuilder;
      }

      // Constructs ClassBuilder objects to hold the FuncDelegate types and
      // their methods.
      private void BuildFuncDelegateClasses(
         ClassBuilder multicastDelegateClassBuilder)
      {
         // Get a local copy of the type table that is associated with
         // the module unit.
         Phx.Types.Table typeTable = this.moduleUnit.TypeTable;

         // The compiler currenly supports 0-4 argument delegate types.
         this.delegateClassBuilders = new ClassBuilder[5];

         // Create an empty list of Argument objects. Each iteration
         // through the loop will add the next argument to the list.
         List<Argument> delegateArguments = new List<Argument>();

         // Create a ClassBuilder object for each delegate type.
         for (int i = 0; i < this.delegateClassBuilders.Length; i++)
         {
            // Build reference to class Lispkit.Sexp.FuncDelegate(i).

            ClassBuilder classBuilder = new MsilClassBuilder(this.moduleUnit,
               string.Concat("Lispkit.Sexp.FuncDelegate", i),
               multicastDelegateClassBuilder.ClassType,
               Phx.Symbols.Access.Public, 
               false, false);

            // Add references to methods.
                        
            classBuilder.AddMethod(Phx.Symbols.Access.Family,
               Phx.Symbols.Visibility.GlobalReference, 
               true, true, typeTable.VoidType, ".ctor", 
               new Argument[] { 
                  new Argument("target", 
                     typeTable.ObjectPointerSystemObjectType),
                  new Argument("method", 
                     typeTable.GetMsilNativeIntType(false)) });

            classBuilder.AddMethod(Phx.Symbols.Access.Family,
               Phx.Symbols.Visibility.GlobalReference, true, false, 
               this.sexpClassBuilder.ClassPointerType,
               "Invoke", delegateArguments.ToArray());

            // Add the next argument to the list.
            delegateArguments.Add(new Argument("A_" + i, 
               this.sexpClassBuilder.ClassPointerType));

            this.delegateClassBuilders[i] = classBuilder;
         }
      }

      // Constructs ClassBuilder objects to hold the FuncAtom types and
      // their methods.
      private void BuildFuncAtomClasses()
      {
         // The compiler currenly supports 0-4 argument types.
         this.funcAtomClassBuilders = new ClassBuilder[5];

         // Create a ClassBuilder object for each atom type.
         for (int i = 0; i < this.funcAtomClassBuilders.Length; i++)
         {
            // Build reference to class Lispkit.Sexp.FuncAtom(i).

            ClassBuilder classBuilder = new MsilClassBuilder(this.moduleUnit,
               string.Concat("Lispkit.Sexp.FuncAtom", i),
               this.sexpClassBuilder.ClassType,
               Phx.Symbols.Access.Public,
               false, false);

            // Add references to methods.

            classBuilder.AddMethod(Phx.Symbols.Access.Public,
               Phx.Symbols.Visibility.GlobalReference,
               true, true, this.moduleUnit.TypeTable.VoidType, ".ctor",
               new Argument[] { new Argument("A_0", 
                  this.delegateClassBuilders[i].ClassPointerType) });

            this.funcAtomClassBuilders[i] = classBuilder;
         }

         // Modify the ClassBuilder object for the SExp class to 
         // reference the FuncPtr property.

         for (int i = 0; i < this.delegateClassBuilders.Length; i++)
         {
            this.sexpClassBuilder.AddMethod(
               Phx.Symbols.Access.Public,
               Phx.Symbols.Visibility.GlobalReference,
               true, true,
               this.delegateClassBuilders[i].ClassPointerType,
               string.Concat("get_FuncPtr", i), null);
         }
      }

      // Constructs a ClassBuilder object to hold the $Lispkit type and
      // its methods.
      private void BuildProgramClass(ClassBuilder objectClassBuilder)
      {
         ClassBuilder classBuilder = new MsilClassBuilder(this.moduleUnit,
            "$Lispkit",
            objectClassBuilder.ClassType,
            Phx.Symbols.Access.Public,
            true, false);

         // Because we're defining this class (rather than referencing it),
         // we need to set the appropriate metadata entries and 
         // create a constructor method.

         // Set class metaproperties.
         classBuilder.ClassType.BeforeFieldInitialize = true;
         
         // Create a function unit for the constructor method.

         Phx.FunctionUnit ctorFunctionUnit = this.CreateFunctionUnit(
            null, ".ctor", classBuilder.ClassType, 
            new Phx.Types.Type[0], false);
                  
         // Emit code to have the constructor method call the 
         // System.Object constructor.

         this.currentInstruction = ctorFunctionUnit.LastInstruction.Previous;

         this.PushThisOperand();
         this.CallMethod(null, objectClassBuilder.GetMethod(".ctor"),
            this.PopOperand());
         this.PopOperand();

         // Set method metaproperties.
         Phx.Symbols.FunctionSymbol ctorSymbol =
            ctorFunctionUnit.FunctionSymbol;         
         ctorSymbol.IsConstructor = true;
         ctorSymbol.IsHideBySignature = true;
         ctorSymbol.IsSpecialName = true;
         ctorSymbol.IsRuntimeSpecialName = true;

         // Add the method to the method table.
         classBuilder.AddMethod(ctorSymbol);

         // Finish creation of the function unit by appending return 
         // and exit instructions.
         this.FinishFunctionUnit(ctorFunctionUnit, null);

         this.programClassBuilder = classBuilder;
      }

      // Searches for the AssemblySymbol object with the provided 
      // name in the program module unit.
      private Phx.Symbols.AssemblySymbol LookupAssembly(
         string assemblyNameString)
      {         
         Phx.Symbols.Table symbolTable = this.moduleUnit.SymbolTable;         
         Phx.Symbols.NameMap nameMap = symbolTable.NameMap;

         // If the module was not compiled with debug information,
         // the module symbol table might not have a name map.
         if (nameMap == null)
         {
            nameMap = Phx.Symbols.NameMap.New(symbolTable, 64);
            symbolTable.AddMap(nameMap);
         }

         // Lookup the symbol from the name map.
         Phx.Name assemblyName =
            Phx.Name.New(this.moduleUnit.Lifetime, assemblyNameString);
         Phx.Symbols.Symbol symbol = nameMap.Lookup(assemblyName);
         
         // Note that there might be a number of symbols with
         // identical names, so search until we have an
         // assembly reference.

         while (symbol != null)         
         {
            if (symbol.IsAssemblySymbol)
            {
               // We found the AssemblySymbol object with the 
               // given name.
               return symbol.AsAssemblySymbol;
            }

            // Move to the next symbol in the lookup list.
            symbol = nameMap.LookupNext(symbol);
         }

         // Not found.
         return null;
      }
      
      // Generates a base assembly with the given file name.
      private bool GenerateBaseAssembly(string outputFileName)
      {
         // Retrieve the resource strings that contain the source code
         // for the base assembly. 
         // SExp and SExpReader are shared with the compiler and provide 
         // run-time support for S-expression types.
         // BaseListing contains the startup code for a Lispkit program.

         string[] sources = {            
            Properties.Resources.SExp,
            Properties.Resources.SExpReader,
            Properties.Resources.BaseListing,
         };

         // Compile the assembly by using CSharpCodeProvider.

         Microsoft.CSharp.CSharpCodeProvider provider =
            new Microsoft.CSharp.CSharpCodeProvider();

         CompilerParameters cp = new CompilerParameters();

         // Generate an executable instead of a class library.
         cp.GenerateExecutable = true;
                  
         // Specify the assembly file name to generate.
         cp.OutputAssembly = outputFileName;

         // Save the assembly as a physical file.
         cp.GenerateInMemory = false;

         // Treat all warnings as errors.
         cp.TreatWarningsAsErrors = true;

         // Generate a program debug database (.pdb) file if the 
         // user provided the /debug flag.
         if (this.generatePdb)
         {
            cp.CompilerOptions = "/debug:full";
         }

         // Compile the source files.
         CompilerResults cr = 
            provider.CompileAssemblyFromSource(cp, sources);

         if (cr.Errors.Count > 0)
         {
            // Display compilation errors.
            foreach (CompilerError ce in cr.Errors)
            {
               Output.ReportError(ce.Line, 
                  Error.InternalError, ce.ToString());
            }
            return false;
         }

         return true;
      }

      // Executes the phase list on each function unit that was 
      // created during compilation.
      private void ExecutePhases()
      {
         foreach (Phx.FunctionUnit functionUnit in this.functionUnits)
         {
            this.ExecutePhases(functionUnit);
         }
      }

      // Executes the phase list on the provided function unit.
      private void ExecutePhases(Phx.FunctionUnit functionUnit)
      {
         // Push the unit onto the current context.
         Phx.Threading.Context context = Phx.Threading.Context.GetCurrent();
         context.PushUnit(functionUnit);

         // Run the phase list.         
         phaseConfiguration.PhaseList.DoPhaseList(functionUnit);

         // Force creation of a new signature.
         Phx.Targets.Runtimes.Vccrt.Win.Msil.Frame frame = functionUnit.Frame
            as Phx.Targets.Runtimes.Vccrt.Win.Msil.Frame;
         frame.LocalVarSigTok = 0;

         // Assign byte offsets to local parameters.
         int i = 0;
         foreach (Phx.Symbols.Symbol localSymbol in 
            functionUnit.SymbolTable.AllSymbols)
         {
            if (!localSymbol.IsLocalVariableSymbol) continue;
            if (!localSymbol.AsLocalVariableSymbol.IsAuto) continue;
            localSymbol.Location.AsFrameLocation.ByteOffset = i++;
         }

         // Finish encoding and release the function unit.

         Phx.PE.Writer.UpdateEncodedIR(functionUnit);
         Phx.Metadata.SignatureEmitter.EncodeLocalSignature(functionUnit);
         functionUnit.ReleaseLifetime();         
      }

      private Phx.FunctionUnit CreateFunctionUnit(SExp e,
         string name, Phx.Types.AggregateType classType, 
         Phx.Types.Type[] argumentTypes, bool isStatic)
      {
         if (e == null)
         {
            // Build a dummy S-expression and map it to line and 
            // column number 0.
            e = new IntAtom(0);
            Lispkit.Location.Add(e.ID, 0, 0);
         }

         // Get a local copy of the type table that is associated with
         // the module unit.
         Phx.Types.Table typeTable = moduleUnit.TypeTable;

         // We generate the function unit slightly different for 
         // class constructors.
         // Record whether the function is a constructor method.
         bool isConstructor = name == ".ctor" || name == ".cctor";
         
         // Create the function type by using a FunctionTypeBuilder object.
         Phx.Types.FunctionTypeBuilder typeBuilder =
            Phx.Types.FunctionTypeBuilder.New(this.moduleUnit.TypeTable);

         // Managed methods use the __clrcall calling convention.
         typeBuilder.CallingConventionKind = 
            Phx.Types.CallingConventionKind.ClrCall;
         
         // Append return type.
         if (isConstructor)
         {
            // Constructor methods do not return a value.
            typeBuilder.AppendReturnParameter(typeTable.VoidType);
         }
         else
         {
            // In this compiler, non-constructors methods always 
            // return type 'SExp'.
            typeBuilder.AppendReturnParameter(
               this.sexpClassBuilder.ClassPointerType);
         }

         // Instance methods pass the 'this' pointer as the first argument.
         if (!isStatic)
         {
            typeBuilder.AppendParameter(
               typeTable.GetObjectPointerType(classType),
               Phx.Types.ParameterKind.ThisPointer);
         }
         foreach (Phx.Types.Type argumentType in argumentTypes)
         {
            Debug.Assert(!isConstructor);
            typeBuilder.AppendParameter(argumentType);
         }

         // Extract the function type from the type builder.
         Phx.Types.FunctionType methodType = typeBuilder.GetFunctionType();

         // Create a function symbol and add it to the symbol table
         // of the parent module unit.

         Phx.Name functionName =
            Phx.Name.New(moduleUnit.Lifetime, name);

         Phx.Symbols.FunctionSymbol functionSymbol =
            Phx.Symbols.FunctionSymbol.New(moduleUnit.SymbolTable,
               0, functionName, methodType,
               Phx.Symbols.Visibility.GlobalDefinition);

         // Add the symbol as a method of the enclosing type.
         classType.AppendMethodSymbol(functionSymbol);
         classType.TypeSymbol.InsertInLexicalScope(
            functionSymbol, functionName);

         // Allocate storage for the function symbol.
         this.AllocateMethodSymbol(moduleUnit, functionSymbol);

         // Establish access level and method specifier.

         functionSymbol.Access = Phx.Symbols.Access.Public;
         if (isStatic)
         {
            functionSymbol.MethodSpecifier = 
               Phx.Symbols.MethodSpecifier.Static;
         }

         // Create the actual FunctionUnit object and its 
         // skeleton code.

         Phx.FunctionUnit functionUnit = 
            this.CreateFunctionUnit(e, functionSymbol);
         this.currentFunctionUnit = functionUnit;

         // Instance methods pass the 'this' pointer as the first argument.
         // Appending the name '$this' allows our language to use identifiers
         // that are named 'this'.
         if (! isStatic)
         {
            Phx.Types.Type thisClassPointerType = 
               this.moduleUnit.TypeTable.GetObjectPointerType(classType);

            // Create symbol for local parameter "$this".

            Phx.Symbols.LocalVariableSymbol thisSymbol =
               Phx.Symbols.LocalVariableSymbol.New(
                  this.currentFunctionUnit.SymbolTable, 0,
                  Phx.Name.New(this.currentFunctionUnit.Lifetime, "$this"),
                  thisClassPointerType, 
                  Phx.Symbols.StorageClass.Parameter
               );
            thisSymbol.IsThisParameter = true;

            // Append the operand to the destination list of
            // the enter instruction.

            this.currentFunctionUnit.FirstEnterInstruction.AppendDestination(
               Phx.IR.VariableOperand.New(this.currentFunctionUnit,
                  thisClassPointerType, thisSymbol));
         }
         
         // Associate the function unit with line and column numbers.

         int line, column;
         if (e != null)
         {
            line = Lispkit.Location.GetLine(e.ID);
            column = Lispkit.Location.GetColumn(e.ID);
         }
         else
         {
            line = 0;
            column = 0;
         }
         Debug.Assert(line >= 0);
         Debug.Assert(column >= 0);

         functionUnit.CurrentDebugTag = 
            functionUnit.DebugInfo.CreateTag(Phx.Name.New(
               functionUnit.Lifetime, this.sourceFileName),
            (uint)line, (ushort)column);
         
         
         return functionUnit;
      }

      // Allocates storage for the given function symbol.
      private void AllocateMethodSymbol(Phx.PEModuleUnit moduleUnit, 
         Phx.Symbols.FunctionSymbol methodSymbol)
      {
         // We allocate methods in the .text section of the image.

         Phx.Name textName = Phx.Name.New(
            Phx.GlobalData.GlobalLifetime, ".text");

         Phx.Symbols.SectionSymbol textSymbol =
            moduleUnit.SymbolTable.LookupByName(textName).AsSectionSymbol;

         methodSymbol.AllocationBaseSectionSymbol = textSymbol;
      }

      private uint functionUnitNumber = 1;

      private Phx.FunctionUnit CreateFunctionUnit(SExp e,
         Phx.Symbols.FunctionSymbol methodSymbol)
      {
         if (e == null)
         {
            // Build a dummy S-expression and map it to line and 
            // column number 0.
            e = new IntAtom(0);
            Lispkit.Location.Add(e.ID, 0, 0);
         }

         // Create a FunctionUnit object. 
         Phx.FunctionUnit functionUnit =
            Phx.FunctionUnit.New(methodSymbol,
               Phx.CodeGenerationMode.IJW, 
               this.moduleUnit.TypeTable,
               this.moduleUnit.MsilRuntime.Architecture,
               this.moduleUnit.MsilRuntime,
               this.moduleUnit, 
               functionUnitNumber++);

         // Associate the function unit with the provided
         // method symbol.
         methodSymbol.FunctionUnit = functionUnit;

         // Add the function unit to the module unit.
         this.moduleUnit.AppendChildUnit(functionUnit);

         // Create a symbol table for the function.
         Phx.Symbols.Table functionSymbolTable =
            Phx.Symbols.Table.New(functionUnit, 64, false);

         // Make it accessible by symbol name.
         functionSymbolTable.AddMap(
            Phx.Symbols.NameMap.New(functionSymbolTable, 64));

         // Generate a token map.
         Phx.Targets.Runtimes.Vccrt.Win.Msil.Runtime runtime = 
            functionUnit.Runtime as 
               Phx.Targets.Runtimes.Vccrt.Win.Msil.Runtime;
         runtime.TokenMap =
            Phx.Symbols.TokenMap.New(Phx.GlobalData.SymbolTable, 64);

         // Initialize the lifetime of the function unit.
         functionUnit.AllocateLifetime();

         // Generate debug information.
         Phx.Debug.Info.New(functionUnit.Lifetime, functionUnit);
         
         // Associate the function unit with line and column numbers.

         int line, column;
         if (e != null)
         {
            line = Lispkit.Location.GetLine(e.ID);
            column = Lispkit.Location.GetColumn(e.ID);
         }
         else
         {
            line = 0;
            column = 0;
         }
         Debug.Assert(line >= 0);
         Debug.Assert(column >= 0);

         functionUnit.CurrentDebugTag =
            functionUnit.DebugInfo.CreateTag(Phx.Name.New(
               functionUnit.Lifetime, this.sourceFileName),
            (uint)line, (ushort)column);
         
         // Retrieve start and end instructions for the unit.

         Phx.IR.Instruction startInstruction = functionUnit.FirstInstruction;
         startInstruction.DebugTag = functionUnit.CurrentDebugTag;

         Phx.IR.Instruction endInstruction = functionUnit.LastInstruction;
         endInstruction.DebugTag = functionUnit.CurrentDebugTag;
                 
         // Append and enter instruction after the start instruction.
         Phx.IR.LabelInstruction enterInstruction = 
            Phx.IR.LabelInstruction.New(
               functionUnit, 
               Phx.Common.Opcode.EnterFunction, 
               methodSymbol);

         startInstruction.InsertAfter(enterInstruction);         
         startInstruction.AppendLabelSource(Phx.IR.LabelOperandKind.Technical,
            Phx.IR.LabelOperand.New(functionUnit, enterInstruction));

         return functionUnit;                  
      }

      // Finishes creation of the provided function unit by appending return 
      // and exit instructions.
      private void FinishFunctionUnit(Phx.FunctionUnit functionUnit, 
         Phx.IR.Operand returnValueOperand)
      {
         // Create a label target for the exit instruction.
         Phx.IR.LabelInstruction exitInstruction =
            Phx.IR.LabelInstruction.New(functionUnit, 
               Phx.Common.Opcode.ExitFunction);
         
         // Insert it into the instruction stream.
         functionUnit.LastInstruction.InsertBefore(exitInstruction);

         // Generate a return instruction.
         Phx.IR.BranchInstruction returnInstruction =
            Phx.IR.BranchInstruction.NewReturn(functionUnit, 
               Phx.Common.Opcode.Return, exitInstruction);

         // Append the provided return operand to the source operand list.
         if (returnValueOperand != null)         
         {            
            returnInstruction.AppendSource(returnValueOperand);
         }

         // Insert it into the instruction stream.
         exitInstruction.InsertBefore(returnInstruction);

         // Allow the framework to finalize creation of the unit.
         functionUnit.FinishCreation();

         // Now that we're done with IR for this unit, pop it off the context
         // stack (creating the unit pushed it on).
         functionUnit.Context.PopUnit();

         // Add the function unit to our list of units.
         this.functionUnits.Add(functionUnit);
         
         // If running in verbose mode, dump the IR for the function unit.

         if (Program.VerboseMode)
         {
            Output.TraceMessage(string.Format("IR for function '{0}':",
               functionUnit.NameString));
            Output.BeginVerboseOutput();
            functionUnit.Dump();
            Output.EndVerboseOutput();
            Output.TraceMessage();
         }
      }
   }
}
