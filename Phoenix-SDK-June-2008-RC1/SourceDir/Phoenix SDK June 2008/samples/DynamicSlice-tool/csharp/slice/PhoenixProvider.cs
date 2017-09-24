using System;
using System.Collections.Generic;
using System.Text;

using SliceAnalysis.Utilities;

namespace SliceAnalysis
{
    /// <summary>
    /// Main interface to slicing tool and Phoenix.
    /// </summary>
    public class PhoenixProvider
    {
        /// <summary>
        /// Cache for the PEModuleUnits and FuncUnits we load.
        /// </summary>
        private Dictionary<string,
                           Pair<Phx.PEModuleUnit,
                                Dictionary<string, FunctionUnit>>> _moduleMap =
            new Dictionary<string, 
                           Pair<Phx.PEModuleUnit,
                                Dictionary<string, FunctionUnit>>>();

        /// <summary>
        /// Initialize Phoenix.
        /// </summary>
        public PhoenixProvider()
        {
            if (!Phx.GlobalData.PhoenixIsInitialized)
            {
                Phx.Targets.Architectures.Architecture x86Arch =
                   Phx.Targets.Architectures.X86.Architecture.New();   
                Phx.Targets.Runtimes.Runtime win32x86Runtime =
                   Phx.Targets.Runtimes.Vccrt.Win32.X86.Runtime.New(x86Arch);

                Phx.GlobalData.RegisterTargetArchitecture(x86Arch);
                Phx.GlobalData.RegisterTargetRuntime(win32x86Runtime);

                Phx.Targets.Architectures.Architecture msilArch =
                   Phx.Targets.Architectures.Msil.Architecture.New();
                Phx.Targets.Runtimes.Runtime winMSILRuntime =
                   Phx.Targets.Runtimes.Vccrt.Win.Msil.Runtime.New(msilArch);
                Phx.GlobalData.RegisterTargetArchitecture(msilArch);
                Phx.GlobalData.RegisterTargetRuntime(winMSILRuntime);
               
                Phx.Initialize.BeginInitialization();

                // Require this switch to be able to build SSA.
                string[] arguments = { "/raise" };
                
                Phx.Initialize.EndInitialization("PHX|*|_PHX_|", arguments);
            }
        }

        public SliceInfo Slice(SliceSpec s)
        {
            // Get the funcunit with the given function name.
            SliceAnalysis.FunctionUnit functionUnit = MakeFuncUnit(s);
            if (functionUnit != null)
            {
                return s.Slice(functionUnit);
            }

            // Return an empty SliceInfo object if functionUnit wasn't created
            // successfully.
            return new SliceInfo();
        }

       private Pair<Phx.PEModuleUnit, Dictionary<string, FunctionUnit>> 
            FetchPEModuleUnit(string peFile, string pdbFile)
        {
            if (_moduleMap.ContainsKey(peFile))
            {
                return _moduleMap[peFile];
            }
            else
            {
                // Load and raise the binary
                Phx.PEModuleUnit peModuleUnit = Phx.PEModuleUnit.Open(peFile,
                    pdbFile,     
                    Phx.Symbols.SymbolicInfoLevel.HasAllSymbolicInfo, null);
                peModuleUnit.LoadEncodedIRUnitList();

                Pair<Phx.PEModuleUnit, Dictionary<string, FunctionUnit>> pair =
                    new Pair<Phx.PEModuleUnit, Dictionary<string, FunctionUnit>>
                    (peModuleUnit, new Dictionary<string, FunctionUnit>());

                _moduleMap.Add(peFile, pair);
                return pair;
            }
        }

        /// <summary>
        /// Free up all open PE files and resources.
        /// </summary>
        public void Deinitialize()
        {
            foreach (Pair<Phx.PEModuleUnit,
                          Dictionary<string, SliceAnalysis.FunctionUnit>> p 
                     in _moduleMap.Values)
            {
                Phx.PEModuleUnit modUnit = p.First;
                if (modUnit != null)
                {
                    modUnit.Close();
                }
            }
        }
       
        private FunctionUnit MakeFuncUnit(SliceSpec specification)
        {
            Pair<Phx.PEModuleUnit, Dictionary<string, SliceAnalysis.FunctionUnit>> 
                pair = FetchPEModuleUnit(specification.PeFile, specification.PdbFile);

            Phx.PEModuleUnit peModuleUnit = pair.First;
            Dictionary<string, SliceAnalysis.FunctionUnit> funcMap = pair.Second;

            // Always replace "::" with "." because Phx and Visual Studio don't
            // seem to agree with the way they represent scopes.
            string functionName = specification.FuncName.Replace("::", ".");

            SliceAnalysis.FunctionUnit functionUnit = null;
            // If the function in question is already cached, get it directly
            string key = functionName;
            if (specification.ArgTypes != null)
            {
               foreach (string s in specification.ArgTypes)
                  key += "," + s;
            }
            if (funcMap.ContainsKey(key))
            {
               functionUnit = funcMap[key];
            }
            else
            {
                // In this case, we need to raise the function and cache it.
                // Iterate through all the function units in the binary, and 
                // see if the name matches.
                foreach (Phx.Unit unit in peModuleUnit.ChildUnits)
                {
                    if (!unit.IsFunctionUnit)
                    {
                        continue;
                    }
                    Phx.FunctionUnit phxFuncUnit = unit.AsFunctionUnit;
                    if (phxFuncUnit.Lifetime == null)
                       phxFuncUnit.AllocateLifetime();
                    string phxFuncName = 
                        phxFuncUnit.FunctionSymbol.QualifiedName.Replace("::", ".");
                    
                    if (phxFuncName.Equals(functionName))
                    {                                                                       
                        // Raise the functionUnit.
                        peModuleUnit.Context.PushUnit(phxFuncUnit);
                        phxFuncUnit = peModuleUnit.Raise(phxFuncUnit.FunctionSymbol,
                                           Phx.FunctionUnit.SymbolicFunctionUnitState);

                        // Make sure argument types match.
                        // This only makes sense for C++ because
                        // C# types do not easily match up with Phoenix types.
                        // also, don't do argument checking if argumentTypes is null
                        if (!peModuleUnit.IsPureMsil && specification.ArgTypes != null)
                        {
                           int argumentNumber = 0;
                           bool argConflict = false;
                           foreach (Phx.IR.Operand operand in phxFuncUnit.FirstEnterInstruction.ExplicitDestinationOperands)
                           {
                              if (operand.Symbol == null || operand.Symbol.NameString.Equals("this"))
                                 continue;

                              if (argumentNumber == specification.ArgTypes.Count)
                              {
                                 argConflict = true;
                                 break;
                              }

                              if (!MatchArgType(operand.Type, specification.ArgTypes[argumentNumber]))
                              {
                                 argConflict = true;
                                 break;
                              }

                              argumentNumber++;
                           }
                                                        
                           if (argConflict || argumentNumber != specification.ArgTypes.Count)
                              continue;
                        }

                        functionUnit = new SliceAnalysis.FunctionUnit(phxFuncUnit);
                        peModuleUnit.Context.PopUnit();
                        funcMap.Add(key, functionUnit);
                        // Verify that the function name in the debuginfo 
                        // matches with the given function name.
                        if (string.Compare(phxFuncUnit.DebugInfo
                                     .GetFileName(phxFuncUnit.FunctionSymbol.DebugTag)
                                     .NameString, specification.SrcFile, true) == 0)
                        {
                            break;
                        }
                    }
                }
            }
            return functionUnit;
        }

       // returns true if the argument types are equivalent, false otherwise
       private bool MatchArgType(Phx.Types.Type operandType, string dbgType)
       {
          int ptrCount = 0;
          while (operandType.IsPointerType)
          {
             ptrCount++;
             operandType = operandType.AsPointerType.ReferentType;
          }

          char[] sep = { ' ' };
          string[] parts = dbgType.Split(sep);
          dbgType = parts[0];
          if (ptrCount != parts.Length - 1)
             return false;

          if (operandType.TypeSymbol != null)
          {
             // this is a non basic type
             if (operandType.IsAggregateType && !operandType.TypeSymbol.NameString.Equals(dbgType))
             {
                return false;
             }
          }
          else
          {
             // this is a basic type
             string dbgPhxType = X86InsnLookupTable.DebuggerTypeToPhxType(dbgType);
             if (!operandType.ToString().Equals(dbgPhxType))
             {
                return false;
             }
          }
          return true;
       }
    }

    public abstract class SliceSpec
    {
       private string funcname;
       public string FuncName
       {
          get { return funcname; }
          set { funcname = value; }
       }

       private string srcfile;
       public string SrcFile
       {
          get { return srcfile; }
          set { srcfile = value; }
       }

       private string pdbfile;
       public string PdbFile
       {
          get { return pdbfile; }
          set { pdbfile = value; }
       }

       private string pefile;
       public string PeFile
       {
          get { return pefile; }
          set { pefile = value; }
       }

       private List<string> argumentTypes;
       public List<string> ArgTypes
       {
          get { return argumentTypes; }
          set { argumentTypes = value; }
       }

        internal abstract SliceInfo Slice(SliceAnalysis.FunctionUnit functionUnit);        
    }

    public class SourceSliceSpec : SliceSpec
    {
       private string varname;
       public string VarName
       {
          get { return varname; }
          set { varname = value; }
       }

       private uint varline;
       public uint VarLine
       {
          get { return varline; }
          set { varline = value; }
       }

       private uint execoffset;
       public uint ExecOffset
       {
          get { return execoffset; }
          set { execoffset = value; }
       }

       private IEvaluator evaluator;
       public IEvaluator Evaluator
       {
          get { return evaluator; }
          set { evaluator = value; }
       }

        internal override SliceInfo Slice(SliceAnalysis.FunctionUnit functionUnit)
        {
            X86InsnLookupTable.Evaluator = this.Evaluator;
            return new SourceSliceStrategy(functionUnit, this).Slice();
        }
    }

    public class AsmSliceSpec : SliceSpec
    {
       private uint operandIndex;
       public uint OpndIndex
       {
          get { return operandIndex; }
          set { operandIndex = value; }
       }

       private uint instroffset;
       public uint InstrOffset
       {
          get { return instroffset; }
          set { instroffset = value; }
       }

        internal override SliceInfo Slice(SliceAnalysis.FunctionUnit functionUnit)
        {
            return new AsmSliceStrategy(functionUnit, this).Slice();
        }
    }
}
